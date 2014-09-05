# Based from here: http://mrbook.org/tutorials/make/
#CC=clang
CC=gcc
CFLAGS=-c -Wall
OUTPUT=yaftpd

all: yaftpd

yaftpd: main.o ftp.o
	$(CC) main.o ftp.o -o $(OUTPUT)

main.o: main.c
	$(CC) $(CFLAGS) main.c

ftp.o: ftp.c ftp.h
	$(CC) $(CFLAGS) ftp.c

clean:
	rm -rf *.o $(OUTPUT)
