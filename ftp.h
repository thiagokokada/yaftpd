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
#include <arpa/inet.h>

#define VERSION_INFO "YAFTPd - Yet Another FTP daemon v0.1"
#define LISTENQ 1

extern int INIT_SEED;
extern char CURRENT_DIR[256];

int random_number(int min,int max);
int create_listener(uint32_t ip,uint16_t port,int reuse_addr);
char *parse_command(char *command);
char *version_info();
char *response_msg(int return_code,char *text_msg);

#endif