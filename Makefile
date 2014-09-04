# Based from here: http://mrbook.org/tutorials/make/
CC=clang
CFLAGS=-c -Wall
OUTPUT=yaftpd

all: yaftpd

yaftpd: main.o ftp.o
	$(CC) main.o ftp.o -o $(OUTPUT)

main.o: main.c
	$(CC) $(CFLAGS) main.c

ftp.o: ftp.c
	$(CC) $(CFLAGS) ftp.c

clean:
	rm -rf *.o $(OUTPUT)