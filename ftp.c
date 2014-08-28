#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VERSION "v0.1"

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
    char* token = strsep(&command, " ");
    if(!strncmp(token, "USER", 4)) {
        return response_msg(331, "Whatever user ;)");
    } else if(!strncmp(token, "PASS", 4)) {
        return response_msg(231, "Whatever pass ;)");
    } else {
        return response_msg(500, "Command not found");
    }
}
