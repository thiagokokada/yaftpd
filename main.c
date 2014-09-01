#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>

#include "ftp.c"

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

int main (int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servaddr, servconn;
    pid_t childpid;
    char recvline[MAXLINE + 1];
    ssize_t  n;

    if (argc != 2) {
        fprintf(stderr,"usage: %s PORT\n",argv[0]);
        fprintf(stderr,"Run FTP server in port PORT\n");
        exit(1);
    }

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket()");
        exit(2);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(atoi(argv[1]));
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind()");
        exit(3);
    }

    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen()");
        exit(4);
    }

    printf("YAFTPd is running in port %s\n",argv[1]);

    for (;;) {
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
            perror("accept()");
            exit(5);
        }

        socklen_t servconn_size = sizeof(servconn);
        if (getsockname(connfd, (struct sockaddr_in *) &servconn, &servconn_size) == -1) {
            perror("getsockname()");
            exit(6);
        }

        if ((childpid = fork()) == 0) { // Child proccess
            printf("Succesful connection at %s. New child PID: %d\n", inet_ntoa(servconn.sin_addr), getpid());
            close(listenfd);

            /* When the user connects show info message about server version */ 
            char* msg = version_msg();
            write(connfd, msg, strlen(msg));

            while ((n=read(connfd, recvline, MAXLINE)) > 0) {
                recvline[n]=0;
                printf("PID %d send: ", getpid());
                if ((fputs(recvline,stdout)) == EOF) {
                    perror("fputs()");
                    exit(7);
                }
                char* return_msg = parse_command(recvline);
                write(connfd, return_msg, strlen(return_msg));
            }

            printf("Finished connection with child PID: %d\n", getpid());
            exit(0);
        } else { // Parent proccess
            close(connfd);
        }
    }
    exit(0);
}
