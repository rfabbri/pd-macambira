/* Copyright (c) 2000 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in the Pd distribution.  */

/* the "pdsend" command.  This is a standalone program that forwards messages
from its standard input to Pd via the netsend/netreceive ("FUDI") protocol. */

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#ifdef MSW
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#define SOCKET_ERROR -1
#endif

void sockerror(const char *s);
void x_closesocket(int fd);
#define BUFSIZE 4096

int main(int argc, char **argv) {
    int sockfd, portno, protocol;
    struct sockaddr_in server;
    struct hostent *hp;
    const char *hostname;
#ifdef MSW
    short version = MAKEWORD(2, 0);
    WSADATA nobby;
#endif
    if (argc<2 || sscanf(argv[1], "%d", &portno)<1 || portno<=0) goto usage;
    if (argc>=3) hostname = argv[2]; else hostname = "127.0.0.1";
    if (argc >= 4) {
        if      (!strcmp(argv[3], "tcp")) protocol = SOCK_STREAM;
        else if (!strcmp(argv[3], "udp")) protocol = SOCK_DGRAM;
        else goto usage;
    } else protocol = SOCK_STREAM;
#ifdef MSW
    if (WSAStartup(version, &nobby)) sockerror("WSAstartup");
#endif
    sockfd = socket(AF_INET, protocol, 0);
    if (sockfd < 0) {sockerror("socket()"); return 1;}
    /* connect socket using hostname provided in command line */
    server.sin_family = AF_INET;
    hp = gethostbyname(hostname);
    if (!hp) {fprintf(stderr, "%s: unknown host\n", hostname); x_closesocket(sockfd); return 1;}
    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);
    server.sin_port = htons((unsigned short)portno);
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server))<0) {sockerror("connect"); x_closesocket(sockfd); return 1;}
    /* now loop reading stdin and sending  it to socket */
    while (1) {
        char buf[BUFSIZE], *bp;
	unsigned int nsent, nsend;
        if (!fgets(buf, BUFSIZE, stdin)) break;
        nsend = strlen(buf);
        for (bp = buf, nsent = 0; nsent < nsend;) {
            int res = send(sockfd, buf, nsend-nsent, 0);
            if (res<0) {sockerror("send"); goto done;}
            nsent += res;
            bp += res;
        }
    }
done:
    if (ferror(stdin)) perror("stdin");
    return 0;
usage:
    fprintf(stderr, "usage: pdsend <portnumber> [host] [udp|tcp]\n");
    fprintf(stderr, "(default is localhost and tcp)\n");
    return 1;
}

void sockerror(const char *s) {
#ifdef MSW
    int err = WSAGetLastError();
    if (err == 10054) return;
    else if (err == 10044) fprintf(stderr,"Warning: you might not have TCP/IP \"networking\" turned on\n");
#else
    int err = errno;
#endif
    fprintf(stderr, "%s: %s (%d)\n", s, strerror(err), err);
}

void x_closesocket(int fd) {
#ifdef MSW
    closesocket(fd);
#else
    close(fd);
#endif
}
