#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "ftp.h"

int main (int argc, char **argv) {
    int listenfd;

    if (argc != 2) {
        fprintf(stderr,"usage: %s PORT\n",argv[0]);
        fprintf(stderr,"Run YAFTPd server in port PORT\n");
        exit(EXIT_FAILURE);
    }

    if ((listenfd = create_listener(INADDR_ANY, atoi(argv[1]), 1)) == -1) {
        perror("create_listener");
        exit(EXIT_FAILURE);
    }

    printf("YAFTPd is running in port %s\n",argv[1]);

    for (;;) {
        if (controller_conn(listenfd) == -1) {
            perror("controller_conn");
            exit(EXIT_FAILURE);
        }
    }

    exit(EXIT_SUCCESS);
}
