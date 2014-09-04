#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef FTP_H
#define FTP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>        

#define VERSION_INFO "YAFTPd - Yet Another FTP daemon v0.1"
#define LISTENQ 1

typedef enum {
    LIST,
    ABOR,
    GET,
    PUT
} popt_t;

extern char CURRENT_DIR[256];
extern int INIT_SEED;
extern int PASSIVE_PIPE_FD[2];
extern int CONN_FD;
extern struct sockaddr_in CURRENT_CONN;
extern socklen_t CURRENT_CONN_SIZE;

int random_number(int min,int max);
int create_listener(uint32_t ip,uint16_t port,int reuse_addr);
char *parse_command(char *command);
char *version_info();
char *response_msg(int return_code,char *text_msg);
int start_passive_mode(uint32_t ip, uint16_t port);
void set_passive_mode_operation(popt_t type);

#endif