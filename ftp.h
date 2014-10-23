/* The MIT License (MIT)
 *
 * Copyright (c) 2014 Thiago Kenji Okada
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */


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
#include <ctype.h>

#define LISTENQ 1
#define MAXDATASIZE 16384
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
