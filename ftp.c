#include "ftp.h"

int INIT_SEED = 0;
int PASSIVE_PIPE_FD[2] = {0, 0};
int CONN_FD = 0;

int parse_command(char* command)
{
    // Remove "\r\n" from command
    command = strtok(command, "\r\n");
    // Split string on " "
    char* token = strsep(&command, " ");
    // RFC says that both upper and lower chars are to be treated equally, so...
    int i = 0;
    while(token[i]) {
        token[i] = toupper(token[i]);
        i++;
    }
    // Print result
    //printf("Token: %s\nArgument: %s\n", token, command);

    char* return_msg = NULL;
    if(!strncmp(token, "USER", 4)) {
        return_msg = response_msg(331, "Whatever user ;)");
    } else if(!strncmp(token, "PASS", 4)) {
        return_msg = response_msg(230, "Whatever pass ;)");
    } else if(!strncmp(token, "PWD", 4) || !strncmp(token, "XPWD", 4)) {
        char cwd[1024];
        if(getcwd(cwd, sizeof(cwd)) != NULL) {
            asprintf(&return_msg, "\"%s\" is the current working directory", cwd);
            return_msg = response_msg(257, return_msg);
        } else {
            return_msg = response_msg(451, "Could not get current working directory");
        }
    } else if(!strncmp(token, "CWD", 4)) {
        if(chdir(command) == -1) {
            return_msg = response_msg(451, strerror(errno));
        } else {
            return_msg = response_msg(250, "OK");
        }
    } else if(!strncmp(token, "PASV", 4)){
        int childpid;
        int port = random_number(1024, 65535);
        char* current_ip = get_socket_ip(CONN_FD);
        
        if(current_ip == NULL) {
            return -1;
        }

        // The child will start a new process, to handle data connection
        // from passive mode
        pipe(PASSIVE_PIPE_FD);
        if ((childpid = fork()) == 0) {
            return start_passive_mode(INADDR_ANY, port);
        }

        asprintf(&return_msg, "Entering Passive Mode (%s,%s,%s,%s,%d,%d)",
                 strsep(&current_ip, "."), strsep(&current_ip, "."),
                 strsep(&current_ip, "."), strsep(&current_ip, "."),
                 port / 256, port % 256);
        
        return_msg = response_msg(227, return_msg);
    } else if(!strncmp(token, "LIST", 4)) {
        set_passive_mode_operation(LIST, NULL);
        return_msg = response_msg(150, "BINARY data connection established");
    } else if(!strncmp(token, "RETR", 4)) {
        if(access(command, R_OK) != -1) {
            set_passive_mode_operation(RETR, command);
            return_msg = response_msg(150, "BINARY data connection established");
        } else {
            return_msg = response_msg(553, "No such file or directory");
        }

    /* The commands bellow are the minimum required implementation by FTP
    (excluding the commands implemented above), to make FTP workable without
    needless error messages. We don't really implement them, just return the
    default value to make them workable. */
    } else if(!strncmp(token, "SYST", 4)) {
        return_msg = response_msg(215, "UNIX Type: L8");
    } else if(!strncmp(token, "PORT", 4)) {
        return_msg = response_msg(500, "This server doesn't support active mode");
    } else if(!strncmp(token, "NOOP", 4) || !strncmp(token, "MODE", 4) ||
        !strncmp(token, "TYPE", 4) || !strncmp(token, "STRU", 4)) {
        return_msg = response_msg(200, "OK");
    } else if(!strncmp(token, "QUIT", 4)) {
        return_msg = response_msg(221, "Bye bye T-T...");
        return 1;
    } else {
        return_msg = response_msg(500, "Command not found");
    }

    printf("SERVER RESPONSE: %s", return_msg);
    write(CONN_FD, return_msg, strlen(return_msg));
    
    return 0;
}

char* response_msg(int return_code, char* text_msg)
{
    int size;
    char* msg = NULL;

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

int start_passive_mode(uint32_t ip, uint16_t port) {
    int listenfd, connfd;

    if ((listenfd = create_listener(ip, port, 1)) == -1) {
        return -1;
    }
    if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
        return -1;
    }

    close(listenfd);
    printf("Successful connect in passive mode with PID: %d\n", getpid());
    close(PASSIVE_PIPE_FD[1]);

    popt_t operation;
    read(PASSIVE_PIPE_FD[0], &operation, sizeof(operation));

    char* return_msg = NULL;
    switch(operation.type) {
        case LIST: {
            //https://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html
            DIR *dp;
            struct dirent *ep;

            dp = opendir("./");
            if(dp != NULL) {
                char* dir_name;
                while((ep = readdir(dp))) {
                    asprintf(&dir_name, "%s\r\n", ep->d_name);
                    write(connfd, dir_name, (strlen(dir_name)+1));
                }
                closedir(dp);
                return_msg = response_msg(226, "Directory list has been submitted");
            } else {
                return_msg = response_msg(451, strerror(errno));
            }
            close(connfd);
            write(CONN_FD, return_msg, strlen(return_msg));
            return 1;
            break;
        }
        case RETR: {
            FILE *fp;
            char recvline[MAXDATASIZE];

            if((fp = fopen(operation.arg, "rb"))) {
                int bytes_read = 0;
                while((bytes_read = fread(recvline, 1, MAXDATASIZE, fp)) > 0) {
                    write(connfd, recvline, bytes_read);
                }
                fclose(fp);
                return_msg = response_msg(226, "File transmission successful");
            } else {
                return_msg = response_msg(553, "Error while trying to open file");
            }
            close(connfd);
            write(CONN_FD, return_msg, strlen(return_msg));
            return 0;
            break;
        }
        case STOR: {
            break;
        }
        default: {
            return_msg = response_msg(500, "Command not found");
            break;
        }
    }
    return 0;
}

char* get_socket_ip(int fd) {
    struct sockaddr_in conn_addr;
    socklen_t conn_addr_len = sizeof(conn_addr);
    
    if ((getsockname(fd, &conn_addr, &conn_addr_len)) == -1) {
        return NULL;
    }

    return inet_ntoa(conn_addr.sin_addr);
}

void set_passive_mode_operation(int type, char* arg) {
    popt_t operation;
    operation.type = type;
    if(arg) {
        strncpy(operation.arg, arg, 256);
    }
    close(PASSIVE_PIPE_FD[0]);
    write(PASSIVE_PIPE_FD[1], &operation, sizeof(operation));
}