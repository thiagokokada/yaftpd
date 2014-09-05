# YAFTPd - Yet Another File Transfer Protocol daemon

### Introduction

YAFTPd is a small FTP server, born from a programming exercise to implement a FTP server in C language for MAC5910 course in [IME-USP][1]. This server only implements passive mode data transfers (since this was the only requirement of this exercise). The server implements the following FTP commands:

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

Some of them were not required by this exercise but was implemented anyway for the sake of completeness. Of course, this is still a small subset of the whole FTP protocol defined by [RFC959][2] (and subsequent RFCs).

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

I only tested this program in a single computer, using both localhost (127.0.0.1) and LAN (192.168.0.x). While this should work between multiple machines in the same LAN and even over the Internet this is not tested.

### Considerations

Instead of using the suggested ```proftpd``` program, that isn't available in Arch Linux repositories, I used the [```bftpd```][7]. So the response messages from ```yaftpd``` shouldn't be the same of ```proftpd```, but the response codes should be the same or at least similar. Anyway, I tried to be funny sometimes when creating the user response messages, so don't be surprise if you suddenly find a cow appearing in your screen ;) .

Another thing, while it has "daemon" in it's name, this program doesn't really have a daemon mode (at least yet). I only put the "d" there so it follows the traditional nomenclature of FTP servers (ftpd, proftpd, vsftpd, bftpd).

### Thanks

* [Daniel Macêdo Batista][3]: the teaching professor of this course and for giving an example code of a simple echo server in C;
* [Wireshark][4]: this packet sniffer allowed me to easily investigate the FTP protocol;
* [GNU][5]: I used ```ftp``` and ```telnet``` programs from GNU inetutils during my investigation of the FTP protocol and to validate my implementation. The GNU extensions of C library helped a lot in development too;
* [CLANG][6]: this awesome compiler helped a lot during the development of this program;
* [Bftpd][7]: like I said in *Considerations*, I used ```bftpd``` instead of ```proftpd``` to investigate the FTP protocol locally;
* [Mozilla FTP][8]: to confirm my findings I played with Mozilla's public FTP server too.

[1]: http://www.ime.usp.br/
[2]: http://tools.ietf.org/html/rfc959
[3]: http://www.ime.usp.br/~batista/
[4]: https://www.wireshark.org/
[5]: https://gnu.org/
[6]: http://clang.llvm.org/
[7]: http://bftpd.sourceforge.net/
[8]: ftp://ftp.mozilla.org/pub/
