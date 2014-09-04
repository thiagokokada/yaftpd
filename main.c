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
    int listenfd;
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
        if ((CONN_FD = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        socklen_t CURRENT_CONN_SIZE = sizeof(CURRENT_CONN);
        if (getsockname(CONN_FD, (struct sockaddr_in *) &CURRENT_CONN, &CURRENT_CONN_SIZE) == -1) {
            perror("getsockname");
            exit(EXIT_FAILURE);
        }

        if ((childpid = fork()) == 0) { // Child proccess
            printf("Succesful connection at %s. New child PID: %d\n", inet_ntoa(CURRENT_CONN.sin_addr), getpid());
            close(listenfd);

            /* When the user connects show info message about server version */ 
            char* msg = version_info();
            write(CONN_FD, msg, strlen(msg));

            while ((n=read(CONN_FD, recvline, MAXLINE)) > 0) {
                recvline[n]=0;
                printf("PID %d SEND: ", getpid());
                if ((fputs(recvline,stdout)) == EOF) {
                    perror("fputs");
                    exit(EXIT_FAILURE);
                }
                char* return_msg = parse_command(recvline);
                printf("SERVER RESPONSE: %s", return_msg);
                write(CONN_FD, return_msg, strlen(return_msg));
            }

            printf("Finished connection with child PID: %d\n", getpid());
            exit(EXIT_SUCCESS);
        } else { // Parent proccess
            close(CONN_FD);
        }
    }
    exit(0);
}
