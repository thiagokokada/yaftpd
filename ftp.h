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
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096
#define VERSION_INFO "YAFTPd - Yet Another FTP daemon v0.1"

extern char CURRENT_DIR[256];
extern int INIT_SEED;
extern socklen_t CURRENT_CONN_SIZE;
extern struct sockaddr_in CURRENT_CONN;

char *parse_command(char *command);
char *response_msg(int return_code,char *text_msg);
char *version_info();
int controller_conn(int listenfd);
int data_conn(int listenfd);
int create_listener(uint32_t ip,uint16_t port,int reuse_addr);
int random_number(int min,int max);

#endif
