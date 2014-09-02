#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define VERSION "v0.1"
#define LISTENQ 1

char CURRENT_DIR[256] = "/";
int INIT_SEED = 0;

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

char* version_msg()
{
    char msg[40] = "YAFTPd - Yet Another FTP daemon ";
    strncat(msg, VERSION, 5);
    return response_msg(200, msg);
}

char* parse_command(char* command)
{
    // Remove "\n" from command
    command = strtok(command, "\n");
    // Split string on " "
    char* token = strsep(&command, " ");
    // Print result
    printf("Token: %s\nArgument: %s\n", token, command);

    if(!strncmp(token, "USER", 4)) {
        return response_msg(331, "Whatever user ;)");
    } else if(!strncmp(token, "PASS", 4)) {
        return response_msg(231, "Whatever pass ;)");
    } else if(!strncmp(token, "PWD", 3) || !strncmp(token, "XPWD", 4)) {
        return response_msg(257, CURRENT_DIR);
    } else if(!strncmp(token, "CWD", 3)) {
        strncpy(CURRENT_DIR, command, 256);
        return response_msg(250, "OK");
    } else if(!strncmp(token, "PASV", 4)){
        
    } else {
        return response_msg(500, "Command not found");
    }
    return NULL;
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