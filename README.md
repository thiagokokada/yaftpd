# YAFTPd - Yet Another File Transfer Protocol daemon

### Introduction

YAFTPd is programming exercise to implement a FTP server in C language for MAC5910 course in IME-USP. This server only implements passive mode data transfers (since this was the only requirement of this exercise). The server implements the following FTP commands:

* USER
* PASS
* PWD
* CWD
* PASV
* LIST
* RETR
* STOR
* SYST
* PORT
* NOOP
* QUIT

Some of them were not required by this exercise but was implemented anyway for the sake of completeness. Of course, this is still a small subset of the whole FTP protocol defined by [RFC959][1] (and subsequent RFCs).

### How to run

You need a GNU environment to compile this source code, since it uses various GNU/POSIX extensions like asprintf, readdir, etc. A relatively recent GNU/Linux distribution should do.

To compile, you can use the included Makefile like this:

	$ make -f Makefile

To run the resulted file, just call the generated ```yaftpd``` binary like this:

	$ ./yaftpd 2121

Where 2121 is the port to run the service.

### Tested environment

This program was developed using ```Arch Linux``` distribution using the ```CLANG``` compiler, version ```3.4.2```. The program was compiled with ```GCC``` compiler, version ```4.9.1``` too, but only a quick test was made and during this test one bug specific to GCC version was found. This bug was fixed but I don't know if other problems may occur with the use of ```GCC``` instead of ```CLANG```.

To run the (more tested) CLANG version, you can install ```clang``` in your machine and modify the ```CC``` variable in ```Makefile``` to ```clang``` instead of ```gcc```.

### Thanks

* [Daniel Macêdo Batista][2]: the teaching professor of this course and for giving an example code of a simple echo server in C;
* [Wireshark][3]: this packet sniffer allowed me to investigate the FTP protocol;
* [GNU][4]: I used ```ftp``` program from GNU inetutils during the investigation of FTP protocol and to validate my implementation. The GNU extensions of C library helped a lot while development too.
* [CLANG][5]: this awesome compiler helped a lot during the development of this program.

[1]: http://tools.ietf.org/html/rfc959
[2]: http://www.ime.usp.br/~batista/
[3]: https://www.wireshark.org/
[4]: https://gnu.org/
[5]: http://clang.llvm.org/