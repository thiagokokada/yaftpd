#include "ftp.h"

char CURRENT_DIR[256] = "/";
int INIT_SEED = 0;
int PASSIVE_PIPE_FD[2];
int CONN_FD = 0;
struct sockaddr_in CURRENT_CONN;
socklen_t CURRENT_CONN_SIZE;

char* parse_command(char* command)
{
    // Remove "\n" from command
    command = strtok(command, "\n");
    // Split string on " "
    char* token = strsep(&command, " ");
    // Print result
    //printf("Token: %s\nArgument: %s\n", token, command);

    if(!strncmp(token, "USER", 4)) {
        return response_msg(331, "Whatever user ;)");
    } else if(!strncmp(token, "PASS", 4)) {
        return response_msg(231, "Whatever pass ;)");
    } else if(!strncmp(token, "PWD", 3) || !strncmp(token, "XPWD", 4)) {
        char* msg;
        asprintf(&msg, "\"%s\" is the current working directory", CURRENT_DIR);
        return response_msg(257, msg);
    } else if(!strncmp(token, "CWD", 3)) {
        strncpy(CURRENT_DIR, command, 256);
        return response_msg(250, "OK");
    } else if(!strncmp(token, "PASV", 4)){
        int childpid, listenfd, connfd;
        int port = random_number(1024, 65535);
        char* current_ip = inet_ntoa(CURRENT_CONN.sin_addr);
        char* msg;

        pipe(PASSIVE_PIPE_FD);
        if ((childpid = fork()) == 0) {
            if(handle_passive_conn(INADDR_ANY, port, PASSIVE_PIPE_FD) == -1) {
                exit(EXIT_FAILURE);
            } else {
                exit(EXIT_SUCCESS);
            }
        }

        asprintf(&msg, "Entering Passive Mode (%s,%s,%s,%s,%d,%d)",
                 strsep(&current_ip, "."), strsep(&current_ip, "."),
                 strsep(&current_ip, "."), strsep(&current_ip, "."),
                 port / 256, port % 256);
        
        return response_msg(227, msg);
    } else if (!strncmp(token, "ABOR", 4)) {
        close(PASSIVE_PIPE_FD[1]);
        popt_t operation = ABOR;
        write(PASSIVE_PIPE_FD[0], &operation, sizeof(operation));
        return response_msg(226, "Aborted connection");
    } else if (!strncmp(token, "LIST", 4)) {
        close(PASSIVE_PIPE_FD[0]);
        popt_t operation = LIST;
        write(PASSIVE_PIPE_FD[1], &operation, sizeof(operation));
        return response_msg(150, "BINARY data connection established");
    } else if (!strncmp(token, "SYST", 4)) {
        return response_msg(215, "UNIX Type: L8");
    } else if(!strncmp(token, "QUIT", 4)) {
        return response_msg(221, "Bye bye T-T...");
    }
    else {
        return response_msg(500, "Command not found");
    }
    return NULL;
}

char* response_msg(int return_code, char* text_msg)
{
    int size;
    char* msg;

    /* Return codes <400 are positive replies, while return codes >=400 are
     * negative replies.
     *
     * See here: https://en.wikipedia.org/wiki/List_of_FTP_server_return_codes
     */
    if(return_code < 400) {
        size = asprintf(&msg, "%d %s\r\n", return_code, text_msg);
    } else {
        size = asprintf(&msg, "%d Error: %s\r\n", return_code, text_msg);
    }

    if(size == -1) {
        fprintf(stderr, "Malloc error: can't allocate memory\n");
        exit(EXIT_FAILURE);
    }
    
    return msg;
}

char* version_info()
{
    return response_msg(220, VERSION_INFO);
}

int create_listener(uint32_t ip, uint16_t port, int reuse_addr) {
    int listenfd;
    struct sockaddr_in servaddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return -1;
    }

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) == -1) { 
        return -1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(ip);
    servaddr.sin_port        = htons(port);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        return -1;
    }

    if (listen(listenfd, LISTENQ) == -1) {
        return -1;
    }

    return listenfd;
}

int random_number(int min, int max) {
    /* Generate a random number in [min, max] range, where min >= 0 and
    max < RAND_MAX.

    Returns a number between 0 and max, or -1 in case of error.
    */
    
    // Check if input is valid
    if(min < 0 || max >= RAND_MAX) {
        return -1;
    }

    // Initialize srand if it's not initialized yet
    if(!INIT_SEED) {
        INIT_SEED = 1;
        srand(time(NULL));
    }

    // Generate random number, discard it if it's not in inputted range
    int divisor = RAND_MAX/(max+1);
    int retval;
    do { 
        retval = rand() / divisor;
    } while (retval < min || retval > max);

    return retval;
}

int handle_passive_conn(uint32_t ip, uint16_t port, int* pipe_pid) {
    int listenfd, connfd;

    if ((listenfd = create_listener(ip, port, 1)) == -1) {
        return -1;
    }
    if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
        return -1;
    }

    close(listenfd);
    printf("Successful connect in passive mode with PID: %d\n", getpid());
    close(pipe_pid[1]);

    popt_t operation;
    read(pipe_pid[0], &operation, sizeof(operation));

    char* return_msg;
    switch(operation) {
        case LIST: {
                    // http://stackoverflow.com/a/646254
            FILE *fp;
            char path[1035];

            fp = popen("ls -l | sed 's/$/\\r/'", "r");
            while(fgets(path, sizeof(path) - 1, fp) != NULL) {
                write(connfd, path, (strlen(path)+1));
            }
            pclose(fp);
            close(connfd);
            return_msg = response_msg(226, "Directory list has been submitted");
            break;
        }
        case ABOR: {
            close(connfd);
            break;
        }
        case GET: {
            break;
        }
        case PUT: {
            break;
        }
        default: {
            return_msg = response_msg(500, "Command not found");
            break;
        }
    }

    write(CONN_FD, return_msg, strlen(return_msg));
    return 0;
}