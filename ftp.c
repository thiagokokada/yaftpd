#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define VERSION "v0.1"

char CURRENT_DIR[256] = "/";

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
    } else {
        return response_msg(500, "Command not found");
    }
}
