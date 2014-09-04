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
#include <dirent.h>

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096
#define VERSION_INFO "YAFTPd - Yet Another FTP daemon v0.1"

enum opt {
    LIST,
    RETR,
    STOR
};

typedef struct {
    enum opt type;
    char arg[256];
} popt_t;

extern int INIT_SEED;
extern int PASSIVE_PIPE_FD[2];
extern int CONN_FD;

int random_number(int min,int max);
int create_listener(uint32_t ip,uint16_t port,int reuse_addr);
int parse_command(char* command);
char* version_info();
char* response_msg(int return_code,char* text_msg);
char* get_socket_ip(int fd);
int start_passive_mode(uint32_t ip, uint16_t port);
void set_passive_mode_operation(int type, char* arg);

#endif