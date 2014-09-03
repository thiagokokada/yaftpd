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

#include "ftp.h"

#define MAXDATASIZE 100
#define MAXLINE 4096

int main (int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servconn;
    pid_t childpid;
    char recvline[MAXLINE + 1];
    ssize_t  n;

    if (argc != 2) {
        fprintf(stderr,"usage: %s PORT\n",argv[0]);
        fprintf(stderr,"Run FTP server in port PORT\n");
        exit(EXIT_FAILURE);
    }

    if ((listenfd = create_listener(INADDR_ANY, atoi(argv[1]), 1)) == -1) {
        perror("create_listener");
        exit(EXIT_FAILURE);
    }

    printf("YAFTPd is running in port %s\n",argv[1]);

    for (;;) {
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        socklen_t servconn_size = sizeof(servconn);
        if (getsockname(connfd, (struct sockaddr_in *) &servconn, &servconn_size) == -1) {
            perror("getsockname");
            exit(EXIT_FAILURE);
        }

        if ((childpid = fork()) == 0) { // Child proccess
            printf("Succesful connection at %s. New child PID: %d\n", inet_ntoa(servconn.sin_addr), getpid());
            close(listenfd);

            /* When the user connects show info message about server version */ 
            char* msg = version_info();
            write(connfd, msg, strlen(msg));

            while ((n=read(connfd, recvline, MAXLINE)) > 0) {
                recvline[n]=0;
                printf("PID %d send: ", getpid());
                if ((fputs(recvline,stdout)) == EOF) {
                    perror("fputs");
                    exit(EXIT_FAILURE);
                }
                char* return_msg = parse_command(recvline);
                write(connfd, return_msg, strlen(return_msg));
            }

            printf("Finished connection with child PID: %d\n", getpid());
            exit(EXIT_SUCCESS);
        } else { // Parent proccess
            close(connfd);
        }
    }
    exit(0);
}
