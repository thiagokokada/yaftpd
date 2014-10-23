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


#include "ftp.h"


int INIT_SEED = 0;
int PASSIVE_PIPE_FD[2] = {0, 0};
int CONN_FD = 0;

/** Parse user commands, process and send result to the user.

Input:
command -- User command to parse.

Returns -1 in case of error, 0 in case of success and to maintain the current
connection active and 1 in case of success but to close the current connection.
*/
int parse_command(char* command)
{
    // Remove "\r\n" from command
    command = strtok(command, "\r\n");
    // Split string on " "
    char* token = strsep(&command, " ");
    // RFC says that both upper and lower chars are to be treated equally, so...
    int i = 0;
    while(token[i]) {
        token[i] = toupper(token[i]);
        i++;
    }

    char* return_msg = NULL;
    // We don't really authenticate the user, just return the correct code
    // with a random message to the user.
    if(!strncmp(token, "USER", 4)) {
        return_msg = response_msg(331, "Whatever user ;)");
    } else if(!strncmp(token, "PASS", 4)) {
        return_msg = response_msg(230, "Whatever pass ;)");
    } else if(!strncmp(token, "PWD", 4)) {
        char cwd[1024];
        // Using getcwd, we get the current working directory, so we can
        // inform the user.
        if(getcwd(cwd, sizeof(cwd)) != NULL) {
            asprintf(&return_msg, "\"%s\" is the current working directory", cwd);
            return_msg = response_msg(257, return_msg);
        } else {
            return_msg = response_msg(451, "Could not get current working directory");
        }
    } else if(!strncmp(token, "CWD", 4)) {
        // In the case it's impossible to change directory, return the error
        // message to the user
        if(chdir(command) == -1) {
            return_msg = response_msg(451, strerror(errno));
        } else {
            return_msg = response_msg(250, "OK");
        }
    } else if(!strncmp(token, "PASV", 4)){
        int childpid;
        int port = random_number(1024, 65535);
        char* current_ip = get_socket_ip(CONN_FD);
        
        // If the above function fails this probably there is something wrong.
        if(current_ip == NULL) {
            return -1;
        }

        // We need to create a new process, so we don't lock the current
        // process for user input. We create a pipe too, so we can send
        // the actual request from client to the child process.
        pipe(PASSIVE_PIPE_FD);
        if ((childpid = fork()) == 0) {
            return start_passive_mode(INADDR_ANY, port);
        }

        // In passive mode, the address that the client needs to connect is
        // defined by "(a,b,c,d,e,f)", where "a,b,c,d" is the server IP
        // (like "127.0.0.1"->"127,0,0,1") and "e,f" is the socket port openned
        // by the server, in the following representation: "e = port / 256" and
        // "f = port % 256".
        char* a = strsep(&current_ip, ".");
        char* b = strsep(&current_ip, ".");
        char* c = strsep(&current_ip, ".");
        char* d = strsep(&current_ip, ".");
        int e = port / 256;
        int f = port % 256;
        asprintf(&return_msg, "Entering Passive Mode (%s,%s,%s,%s,%d,%d)",
                 a, b, c, d, e, f);
        
        return_msg = response_msg(227, return_msg);

    // LIST, RETR, STOR are operations that happen in the data connection
    // created in the PASV request.
    } else if(!strncmp(token, "LIST", 4)) {
        set_passive_mode_operation(LIST, NULL);
        return_msg = response_msg(150, "BINARY data connection established");
    } else if(!strncmp(token, "RETR", 4)) {
        // We first check if we can actually read the file before trying to
        // send to the client.
        if(access(command, F_OK|R_OK) != -1) {
            set_passive_mode_operation(RETR, command);
            return_msg = response_msg(150, "BINARY data connection established");
        } else {
            return_msg = response_msg(553, "No such file or directory");
        }
    } else if(!strncmp(token, "STOR", 4)) {
        // This operation is basically the inverse of RETR.
        if(access(command, F_OK|W_OK) == -1) {
            set_passive_mode_operation(STOR, command);
            return_msg = response_msg(150, "BINARY data connection established");
        } else {
            return_msg = response_msg(553, "Remote file is write protected");
        }
    // The commands bellow are the minimum required implementation by FTP
    // (excluding the commands implemented above), to make FTP workable without
    // needless error messages. We don't really implement them, just return the
    // default value to make them workable.
    } else if(!strncmp(token, "SYST", 4)) {
        return_msg = response_msg(215, "UNIX Type: L8");
    } else if(!strncmp(token, "PORT", 4)) {
        return_msg = response_msg(500, "This server doesn't support active mode");
    } else if(!strncmp(token, "NOOP", 4) || !strncmp(token, "MODE", 4) ||
        !strncmp(token, "TYPE", 4) || !strncmp(token, "STRU", 4)) {
        return_msg = response_msg(200, "OK");
    } else if(!strncmp(token, "MOO", 4)) {
        // Why did I implemented this command? Well, because ;)...
        // Of course this is only accessible using telnet or a specially
        // craft client.
        char* moo = "\n"
                    " ______________________ \n"
                    "< Because I can ;) ... >\n"
                    " ---------------------- \n"
                    "        \\   ^__^        \n"
                    "         \\  (oo)\\_______\n"
                    "            (__)\\       )\\/\\\n"
                    "                ||----w |\n"
                    "                ||     ||\n";

        return_msg = response_msg(666, moo);
    } else if(!strncmp(token, "QUIT", 4)) {
        return_msg = response_msg(221, "Bye bye T-T...");
        // Need to write the message before exiting this function.
        write(CONN_FD, return_msg, strlen(return_msg));
        // Return 1 after QUIT, so the parent can close this connection.
        return 1;
    } else {
        return_msg = response_msg(500, "Command not found");
    }

    // For logging purposes.
    printf("SERVER RESPONSE: %s", return_msg);
    // Actually send message to the client.
    write(CONN_FD, return_msg, strlen(return_msg));
    
    // Except if overwrite above, the current connection is still alive.
    return 0;
}

/** Create a FTP friendly formated message, including return code, CRLF and
error tag when it's appropriate.

Input:
return_code -- Return code of the message. This is what the client process.
See https://en.wikipedia.org/wiki/List_of_FTP_server_return_codes for a list of
them.

text_msg -- User friendly message. This can be anything, since it isn't
processed by the client, but of course a nice message helps.

Return the formated message in case of success, or NULL in case of error. 
*/
char* response_msg(int return_code, char* text_msg)
{
    int size;
    char* msg = NULL;

    // Return codes <400 are positive replies, while return codes >=400 are
    // negative replies.
    if(return_code < 400) {
        size = asprintf(&msg, "%d %s\r\n", return_code, text_msg);
    } else {
        size = asprintf(&msg, "%d Error: %s\r\n", return_code, text_msg);
    }

    if(size == -1) {
        return NULL;
    }
    
    return msg;
}

/** Returns version info with correct return code when the users connects to
the server. Actual server information is defined by VERSION_INFO constant.
*/
char* version_info()
{
    return response_msg(220, VERSION_INFO);
}

/** Create a new listener socket.

Input:
ip -- IP to listen by. Needs to be correctly formatted. See "man 0 sys_socket.h"
for details.
port -- Port to listen by. Same considerations as above.
reuse_addr -- Set to 1 if it's possible to reuse this socket in a small amount
of time, otherwise set it to 0.

Returns the new socket file descriptor in case of success, or -1 otherwise. In
case of error, errno will be set to indicate the type of the error.
*/
int create_listener(uint32_t ip, uint16_t port, int reuse_addr) {
    int listenfd;
    struct sockaddr_in servaddr;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return -1;
    }

    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) == -1) { 
        return -1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(ip);
    servaddr.sin_port        = htons(port);

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        return -1;
    }

    if (listen(listenfd, LISTENQ) == -1) {
        return -1;
    }

    return listenfd;
}

/** Generate a random number in [min, max] range, where min >= 0 and
max < RAND_MAX.

Input:
min -- Minimum possible value.
max -- Maximum possible value.

Returns a number between 0 and max, or -1 in case of error.
*/
int random_number(int min, int max) {    
    // Check if input is valid
    if(min < 0 || max >= RAND_MAX) {
        return -1;
    }

    // Initialize srand if it's not initialized yet
    if(!INIT_SEED) {
        INIT_SEED = 1;
        srand(time(NULL));
    }

    // Generate random number, discard it if it's not in inputted range
    int divisor = RAND_MAX/(max+1);
    int retval;
    do { 
        retval = rand() / divisor;
    } while (retval < min || retval > max);

    return retval;
}

/** Starts a new listener socket for data in passive mode.

Input:
ip and port -- See create_listener for details.

Returns -1 in case of error, 1 otherwise. Why not 0? This is reserved for
future use (to use if you need to maintain this connection active, but my
investigation of FTP protocol didn't find a case where the data connection
was active after use).
*/
int start_passive_mode(uint32_t ip, uint16_t port) {
    int listenfd, connfd;

    if ((listenfd = create_listener(ip, port, 1)) == -1) {
        return -1;
    }
    if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
        return -1;
    }

    close(listenfd);
    printf("Successful connect in passive mode with PID: %d\n", getpid());
    close(PASSIVE_PIPE_FD[1]);

    popt_t operation;
    read(PASSIVE_PIPE_FD[0], &operation, sizeof(operation));

    char* return_msg = NULL;
    switch(operation.type) {
        case LIST: {
            //https://www.gnu.org/software/libc/manual/html_node/Simple-Directory-Lister.html
            DIR *dp;
            struct dirent *ep;

            dp = opendir("./");
            if(dp != NULL) {
                char* dir_name;
                while((ep = readdir(dp))) {
                    asprintf(&dir_name, "%s\r\n", ep->d_name);
                    write(connfd, dir_name, (strlen(dir_name)+1));
                }
                closedir(dp);
                return_msg = response_msg(226, "Directory list has been submitted");
            } else {
                return_msg = response_msg(451, strerror(errno));
            }
            close(connfd);
            write(CONN_FD, return_msg, strlen(return_msg));
            break;
        }
        case RETR: {
            FILE *fp;
            char recvline[MAXDATASIZE];

            if((fp = fopen(operation.arg, "rb"))) {
                int bytes_read = 0;
                while((bytes_read = fread(recvline, 1, MAXDATASIZE, fp)) > 0) {
                    write(connfd, recvline, bytes_read);
                }
                fclose(fp);
                return_msg = response_msg(226, "File transmission successful");
            } else {
                return_msg = response_msg(553, "Error while trying to open file");
            }
            close(connfd);
            write(CONN_FD, return_msg, strlen(return_msg));
            break;
        }
        case STOR: {
            FILE *fp;
            char recvline[MAXDATASIZE];

            if((fp = fopen(operation.arg, "wb"))) {
                int bytes_read = 0;
                while((bytes_read = read(connfd, recvline, MAXDATASIZE)) > 0) {
                    fwrite(recvline, 1, bytes_read, fp);
                }
                fclose(fp);
                return_msg = response_msg(226, "File transmission successful");
            } else {
                return_msg = response_msg(553, "Error while trying to open file");
            }
            close(connfd);
            write(CONN_FD, return_msg, strlen(return_msg));
            break;
        }
        default: {
            return_msg = response_msg(500, "Command not found");
            break;
        }
    }
    return 1;
}

/* Returns the socket IP in a user friendly format (like 127.0.0.1).

Input:
fd -- socket's file descriptor
*/
char* get_socket_ip(int fd) {
    struct sockaddr_in conn_addr;
    socklen_t conn_addr_len = sizeof(conn_addr);
    
    if ((getsockname(fd, &conn_addr, &conn_addr_len)) == -1) {
        return NULL;
    }

    return inet_ntoa(conn_addr.sin_addr);
}

/* Set passive mode operation

Input:
type -- Type of operation
arg -- Argument of the operation
*/
void set_passive_mode_operation(int type, char* arg) {
    popt_t operation;
    operation.type = type;
    if(arg) {
        strncpy(operation.arg, arg, 256);
    }
    close(PASSIVE_PIPE_FD[0]);
    write(PASSIVE_PIPE_FD[1], &operation, sizeof(operation));
}
