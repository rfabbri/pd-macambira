/* Copyright (c) 2000 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in the Pd distribution.  */

/* the "pdreceive" command.  This is a standalone program that receives messages
from Pd via the netsend/netreceive ("FUDI") protocol, and copies them to
standard output. */

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

typedef struct _fdpoll {
    int fd;
    char *inbuf;
    int inhead;
    int intail;
    int udp;
} t_fdpoll;

static int nfdpoll;
static t_fdpoll *fdpoll;
static int maxfd;
static int sockfd;
static int protocol;

static void sockerror(const char *s);
static void x_closesocket(int fd);
static void dopoll();
#define BUFSIZE 4096

int main(int argc, char **argv) {
    int portno;
    struct sockaddr_in server;
#ifdef MSW
    short version = MAKEWORD(2, 0);
    WSADATA nobby;
#endif
    if (argc < 2 || sscanf(argv[1],"%d",&portno)<1 || portno<=0) goto usage;
    if (argc >= 3) {
        if      (!strcmp(argv[2],"tcp")) protocol = SOCK_STREAM;
        else if (!strcmp(argv[2],"udp")) protocol = SOCK_DGRAM;
        else goto usage;
    } else protocol = SOCK_STREAM;
#ifdef MSW
    if (WSAStartup(version, &nobby)) sockerror("WSAstartup");
#endif
    sockfd = socket(AF_INET, protocol, 0);
    if (sockfd < 0) {
        sockerror("socket()");
        exit(1);
    }
    maxfd = sockfd + 1;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

#ifdef IRIX
    /* this seems to work only in IRIX but is unnecessary in Linux.  Not sure what MSW needs in place of this. */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 0, 0) < 0) fprintf(stderr, "setsockopt failed\n");
#endif

    /* assign client port number */
    server.sin_port = htons((unsigned short)portno);

    /* name the socket */
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        sockerror("bind");
        x_closesocket(sockfd);
        return 0;
    }
    if (protocol == SOCK_STREAM) {
        if (listen(sockfd, 5) < 0) {
            sockerror("listen");
            x_closesocket(sockfd);
            return 1;
        }
    }
    /* now loop forever selecting on sockets */
    while (1) dopoll();
usage:
    fprintf(stderr, "usage: pdreceive <portnumber> [udp|tcp]\n");
    fprintf(stderr, "(default is tcp)\n");
    return 1;
}

static void addport(int fd) {
    fdpoll = (t_fdpoll *)realloc(fdpoll, (nfdpoll+1)*sizeof(t_fdpoll));
    t_fdpoll *fp = fdpoll + nfdpoll;
    fp->fd = fd;
    nfdpoll++;
    if (fd >= maxfd) maxfd = fd + 1;
    fp->inhead = fp->intail = 0;
    if (!(fp->inbuf = (char *) malloc(BUFSIZE))) {fprintf(stderr, "out of memory"); exit(1);}
    printf("number_connected %d;\n", nfdpoll);
}

static void rmport(t_fdpoll *x) {
    t_fdpoll *fp = fdpoll;
    for (int i=nfdpoll; i--; fp++) {
        if (fp == x) {
            x_closesocket(fp->fd);
            free(fp->inbuf);
            while (i--) {
                fp[0] = fp[1];
                fp++;
            }
            fdpoll = (t_fdpoll *)realloc(fdpoll, (nfdpoll-1)*sizeof(t_fdpoll));
            nfdpoll--;
            printf("number_connected %d;\n", nfdpoll);
            return;
        }
    }
    fprintf(stderr, "warning: item removed from poll list but not found");
}

static void doconnect() {
    int fd = accept(sockfd, 0, 0);
    if (fd < 0) perror("accept");
    else addport(fd);
}

static void udpread() {
    char buf[BUFSIZE];
    int ret = recv(sockfd, buf, BUFSIZE, 0);
    if (ret<0) {
        sockerror("recv (udp)");
        x_closesocket(sockfd);
        exit(1);
    } else if (ret>0) {
#ifdef MSW
        for (int j=0; j<ret; j++) putchar(buf[j]);
#else
        if (write(1, buf, ret) < ret) {
            perror("write");
            exit(1);
        }
#endif
    }
}

static int tcpmakeoutput(t_fdpoll *x) {
    char messbuf[BUFSIZE+1], *bp = messbuf;
    int inhead = x->inhead;
    int intail = x->intail;
    char *inbuf = x->inbuf;
    if (intail == inhead) return 0;
    for (int indx = intail; indx != inhead; indx = (indx+1)&(BUFSIZE-1)) {
        /* search for a semicolon. */
        char c = *bp++ = inbuf[indx];
        if (c == ';') {
            intail = (indx+1)&(BUFSIZE-1);
            if (inbuf[intail] == '\n') intail = (intail+1)&(BUFSIZE-1);
            *bp++ = '\n';
#ifdef MSW
            for (int j=0; j<bp-messbuf; j++) putchar(messbuf[j]);
#else
            if (write(1, messbuf, bp-messbuf)<bp-messbuf) {perror("write"); exit(1);}
#endif
            x->inhead = inhead;
            x->intail = intail;
            return 1;
        }
    }
    return 0;
}

static void tcpread(t_fdpoll *x) {
    int readto = x->inhead >= x->intail ? BUFSIZE : x->intail-1;
    int ret;
    /* the input buffer might be full.  If so, drop the whole thing */
    if (readto == x->inhead) {
        fprintf(stderr, "pd: dropped message from gui\n");
        x->inhead = x->intail = 0;
        readto = BUFSIZE;
    } else {
        ret = recv(x->fd, x->inbuf + x->inhead, readto - x->inhead, 0);
        if (ret < 0) {
            sockerror("recv (tcp)");
            rmport(x);
        } else if (ret == 0) rmport(x);
        else {
            x->inhead += ret;
            if (x->inhead >= BUFSIZE) x->inhead = 0;
            while (tcpmakeoutput(x)) {}
        }
    }
}

static void dopoll() {
    t_fdpoll *fp;
    fd_set readset, writeset, exceptset;
    FD_ZERO(&writeset);
    FD_ZERO(&readset);
    FD_ZERO(&exceptset);
    FD_SET(sockfd, &readset);
    if (protocol == SOCK_STREAM) {
        fp = fdpoll;
        for (int i=nfdpoll; i--; fp++) FD_SET(fp->fd, &readset);
    }
    if (select(maxfd+1, &readset, &writeset, &exceptset, 0) < 0) {perror("select"); exit(1);}
    if (protocol == SOCK_STREAM) {
        for (int i=0; i<nfdpoll; i++) if (FD_ISSET(fdpoll[i].fd, &readset)) tcpread(&fdpoll[i]);
        if (FD_ISSET(sockfd, &readset)) doconnect();
    } else {
        if (FD_ISSET(sockfd, &readset)) udpread();
    }
}

static void sockerror(const char *s) {
#ifdef MSW
    int err = WSAGetLastError();
    if (err == 10054) return;
    else if (err == 10044) fprintf(stderr,"Warning: you might not have TCP/IP \"networking\" turned on\n");
#else
    int err = errno;
#endif
    fprintf(stderr, "%s: %s (%d)\n", s, strerror(err), err);
}

static void x_closesocket(int fd) {
#ifdef MSW
    closesocket(fd);
#else
    close(fd);
#endif
}
