#include "ftp.h"

char CURRENT_DIR[256] = "/";
int INIT_SEED = 0;
int NUM_CONN = 0;
socklen_t CURRENT_CONN_SIZE;
struct sockaddr_in CURRENT_CONN;

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
        int listenfd;
        int port = random_number(1024, 65535);
        char* msg;
        if ((listenfd = create_listener(INADDR_ANY, port, 0)) == -1) {
            return NULL;
        }
        if (data_conn(listenfd) == -1) {
            return NULL;
        } 
        asprintf(&msg, "Entering Passive Mode (127,0,0,1,%d,%d)", port / 256, port % 256);
        return response_msg(227, msg);
    } else {
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
    return response_msg(200, VERSION_INFO);
}

int controller_conn(int listenfd)
{
    int connfd;
    pid_t childpid;
    char recvline[MAXLINE + 1];
    ssize_t  n;

    if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1) {
        return -1;
    }

    if ((childpid = fork()) == 0) { // Child proccess
        close(listenfd);

        /* When the user connects show info message about server version */ 
        char* msg = version_info();
        write(connfd, msg, strlen(msg));

        while ((n=read(connfd, recvline, MAXLINE)) > 0) {
            recvline[n]=0;
            printf("PID %d send: ", getpid());
            if ((fputs(recvline,stdout)) == EOF) {
                return -1;
            }
            char* return_msg;
            if((return_msg = parse_command(recvline)) == NULL) {
                return -1;
            }
            write(connfd, return_msg, strlen(return_msg));
        }

        printf("Finished connection with child PID: %d\n", getpid());
    }

    close(connfd);
    return 0;
}

int data_conn(int listenfd)
{
    int connfd;
    pid_t childpid;
    char recvline[MAXLINE + 1];
    ssize_t  n;

    /* Limit the number of multiple connections an user can do simultaneously.
    This should protect the server from a possible DoS attack by issuing
    multiples PASV commands. */
    if (NUM_CONN < MAXCONN) {
        NUM_CONN++;
    } else {
        return -1;
    }

    if ((childpid = fork()) == 0) {
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1) {
            NUM_CONN --;
            return -1;
        }
        printf("Succesful connection in passive mode with PID: %d\n", getpid());
        close(listenfd);
    }

    close(connfd);
    NUM_CONN--;
    return 0;
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
