/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Pd side of the Pd/Pd-gui interface.  Also, some system interface routines
that didn't really belong anywhere. */

#define WATCHDOGTHREAD

#define PD_PLUSPLUS_FACE
#include "desire.h"
#include "pthread.h"
#include <sstream>
#ifdef UNISTD
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/resource.h>
#endif
#ifdef HAVE_BSTRING_H
#include <bstring.h>
#endif
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <winsock.h>
#include <windows.h>
#ifdef _MSC_VER
typedef int pid_t;
#endif
typedef int socklen_t;
#define EADDRINUSE WSAEADDRINUSE
#endif

#include <stdarg.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#ifndef SIGIOT
#define SIGIOT SIGABRT
#endif

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#else
#include <stdlib.h>
#endif

#define DEBUG_MESSUP 1      /* messages up from pd to pd-gui */
#define DEBUG_MESSDOWN 2    /* messages down from pd-gui to pd */

/* T.Grill - make it a _little_ more adaptable... */
#ifndef PDBINDIR
#define PDBINDIR "bin/"
#endif

#ifndef WISHAPP
#define WISHAPP "wish84.exe"
#endif

#ifdef __linux__
#define LOCALHOST "127.0.0.1"
#else
#define LOCALHOST "localhost"
#endif

struct t_fdpoll {
    int fdp_fd;
    t_fdpollfn fdp_fn;
    void *fdp_ptr;
};

#define INBUFSIZE 16384

extern int sys_guisetportnumber;
static int sys_nfdpoll;
static t_fdpoll *sys_fdpoll;
static int sys_maxfd;
t_text *sys_netreceive;
static t_binbuf *inbinbuf;
t_socketreceiver *sys_socketreceiver;
extern int sys_addhist(int phase);

/* ----------- functions for timing, signals, priorities, etc  --------- */

#ifdef _WIN32
static LARGE_INTEGER nt_inittime;
static double nt_freq = 0;

static void sys_initntclock() {
    LARGE_INTEGER f1;
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    if (!QueryPerformanceFrequency(&f1)) {
          fprintf(stderr, "pd: QueryPerformanceFrequency failed\n");
          f1.QuadPart = 1;
    }
    nt_freq = f1.QuadPart;
    nt_inittime = now;
}

#if 0
/* this is a version you can call if you did the QueryPerformanceCounter
   call yourself.  Necessary for time tagging incoming MIDI at interrupt
   level, for instance; but we're not doing that just now. */

double nt_tixtotime(LARGE_INTEGER *dumbass) {
    if (nt_freq == 0) sys_initntclock();
    return (((double)(dumbass->QuadPart - nt_inittime.QuadPart)) / nt_freq);
}
#endif
#endif /* _WIN32 */

    /* get "real time" in seconds; take the
    first time we get called as a reference time of zero. */
double sys_getrealtime() {
#ifndef _WIN32
    static struct timeval then;
    struct timeval now;
    gettimeofday(&now, 0);
    if (then.tv_sec == 0 && then.tv_usec == 0) then = now;
    return (now.tv_sec - then.tv_sec) + (1./1000000.) * (now.tv_usec - then.tv_usec);
#else
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    if (nt_freq == 0) sys_initntclock();
    return double(now.QuadPart - nt_inittime.QuadPart) / nt_freq;
#endif
}

int sys_pollsockets () {
    struct timeval timout;
    int didsomething = 0;
    fd_set readset, writeset, exceptset;
    timout.tv_sec = 0;
    timout.tv_usec = 0;
    FD_ZERO(&writeset);
    FD_ZERO(&readset);
    FD_ZERO(&exceptset);
    t_fdpoll *fp = sys_fdpoll;
    for (int i=sys_nfdpoll; i--; fp++) FD_SET(fp->fdp_fd, &readset);
    select(sys_maxfd+1, &readset, &writeset, &exceptset, &timout);
    for (int i=0; i<sys_nfdpoll; i++) if (FD_ISSET(sys_fdpoll[i].fdp_fd, &readset)) {
        sys_fdpoll[i].fdp_fn(sys_fdpoll[i].fdp_ptr, sys_fdpoll[i].fdp_fd);
        didsomething = 1;
    }
    return didsomething;
}

void sys_microsleep(int microsec) {
	/* Tim says: sleep granularity on "modern" operating systems is only 1ms???
	   - linux: we might be better with the high precision posix timer kernel patches ...
	   - windows: win9x doesn't implement a SwitchToThread function, so we can't sleep for small timeslices
	   - osx: ???
	*/
	if (!sys_callbackscheduler && microsec < 1000) {
		if (500 < microsec) microsec = 1000; else return;
	}
#ifndef MSW
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = microsec;
	select(0,0,0,0,&timeout);

#else
	Sleep(microsec/1000);
#endif
	/* a solution for lower timeslices might be a busysleep but this might
	   block a low-priority thread and won't work for win9x
#define _WIN32_WINNT 0x0400
	double end = sys_getrealtime() + (double)microsec * 1e-6;
	do {
#ifdef MSW
		SwitchToThread();
#else 
		sched_yield();
#endif
	}
	while(sys_getrealtime() < end);
	*/
}

#ifdef UNISTD
typedef void (*sighandler_t)(int);

static void sys_signal(int signo, sighandler_t sigfun) {
    struct sigaction action;
    action.sa_flags = 0;
    action.sa_handler = sigfun;
    memset(&action.sa_mask, 0, sizeof(action.sa_mask));
#if 0  /* GG says: don't use that */
    action.sa_restorer = 0;
#endif
    if (sigaction(signo, &action, 0) < 0) perror("sigaction");

}

static void sys_exithandler(int n) {
    static int trouble = 0;
    if (!trouble) {
        trouble = 1;
        fprintf(stderr, "Pd: signal %d\n", n);
        sys_bail(1);
    } else sys_bail(0);
}

static void sys_alarmhandler(int n) {
    fprintf(stderr, "Pd: system call timed out\n");
}

/* what is this for?? */
static void sys_huphandler(int n) {
    struct timeval timout;
    timout.tv_sec = 0;
    timout.tv_usec = 30000;
    select(1, 0, 0, 0, &timout);
}

void sys_setalarm(int microsec) {
    struct itimerval it;
    it.it_interval.tv_sec  = 0;
    it.it_interval.tv_usec = 0;
    it.it_value.tv_sec  = microsec/1000000;
    it.it_value.tv_usec = microsec%1000000;
    sys_signal(SIGALRM, microsec ? sys_alarmhandler : SIG_IGN);
    setitimer(ITIMER_REAL, &it, 0);
}

#endif

#if defined(__linux) || defined(__APPLE__)
#if (_POSIX_PRIORITY_SCHEDULING - 0) >=  200112L || (_POSIX_MEMLOCK - 0) >=  200112L
#include <sched.h>
#endif
#if (_POSIX_MEMLOCK - 0) >=  200112L
#include <sys/resource.h>
#endif
void sys_set_priority(int higher) {
#if (_POSIX_PRIORITY_SCHEDULING - 0) >=  200112L
    struct sched_param par;
#ifdef USEAPI_JACK
    int p1 = sched_get_priority_min(SCHED_FIFO);
    int p3 = (higher ? p1 + 7 : p1 + 5);
#else
    int p2 = sched_get_priority_max(SCHED_FIFO);
    int p3 = (higher ? p2 - 1 : p2 - 3);
#endif
    par.sched_priority = p3;
    if (sched_setscheduler(0,SCHED_FIFO,&par) != -1)
       fprintf(stderr, "priority %d scheduling enabled.\n", p3);
#endif
#if (_POSIX_MEMLOCK - 0) >=  200112L
    /* tb: force memlock to physical memory { */
    struct rlimit mlock_limit;
    mlock_limit.rlim_cur=0;
    /* tb: only if we are really root we can set the hard limit */
    mlock_limit.rlim_max = getuid() ? 100 : 0;
    setrlimit(RLIMIT_MEMLOCK,&mlock_limit);
    /* } tb */
    if (mlockall(MCL_FUTURE) != -1) fprintf(stderr, "memory locking enabled.\n");
#endif
}
#endif /* __linux__ */

#ifdef IRIX             /* hack by <olaf.matthes@gmx.de> at 2003/09/21 */

#if (_POSIX_PRIORITY_SCHEDULING - 0) >=  200112L || (_POSIX_MEMLOCK - 0) >=  200112L
#include <sched.h>
#endif

void sys_set_priority(int higher) {
#if (_POSIX_PRIORITY_SCHEDULING - 0) >=  200112L
    struct sched_param par;
        /* Bearing the table found in 'man realtime' in mind, I found it a */
        /* good idea to use 192 as the priority setting for Pd. Any thoughts? */
    if (higher) par.sched_priority = 250;       /* priority for watchdog */
    else        par.sched_priority = 192;       /* priority for pd (DSP) */
    if (sched_setscheduler(0, SCHED_FIFO, &par) != -1)
        fprintf(stderr, "priority %d scheduling enabled.\n", par.sched_priority);
#endif

#if (_POSIX_MEMLOCK - 0) >=  200112L
    if (mlockall(MCL_FUTURE) != -1) fprintf(stderr, "memory locking enabled.\n");
#endif
}
/* end of hack */
#endif /* IRIX */

/* ------------------ receiving incoming messages over sockets ------------- */

void sys_sockerror(char *s) {
#ifdef _WIN32
    int err = WSAGetLastError();
    if (err == 10054) return;
    else if (err == 10044) {
        fprintf(stderr, "Warning: you might not have TCP/IP \"networking\" turned on\n");
        fprintf(stderr, "which is needed for Pd to talk to its GUI layer.\n");
    }
#else
    int err = errno;
#endif /* _WIN32 */
    fprintf(stderr, "%s: %s (%d)\n", s, strerror(err), err);
}

void sys_addpollfn(int fd, t_fdpollfn fn, void *ptr) {
    int nfd = sys_nfdpoll;
    int size = nfd * sizeof(t_fdpoll);
    sys_fdpoll = (t_fdpoll *)t_resizebytes(sys_fdpoll, size, size + sizeof(t_fdpoll));
    t_fdpoll *fp = sys_fdpoll + nfd;
    fp->fdp_fd = fd;
    fp->fdp_fn = fn;
    fp->fdp_ptr = ptr;
    sys_nfdpoll = nfd + 1;
    if (fd >= sys_maxfd) sys_maxfd = fd + 1;
}

void sys_rmpollfn(int fd) {
    int nfd = sys_nfdpoll;
    int size = nfd * sizeof(t_fdpoll);
    t_fdpoll *fp = sys_fdpoll;
    for (int i=nfd; i--; fp++) {
        if (fp->fdp_fd == fd) {
            while (i--) {
                fp[0] = fp[1];
                fp++;
            }
            sys_fdpoll = (t_fdpoll *)t_resizebytes(sys_fdpoll, size, size - sizeof(t_fdpoll));
            sys_nfdpoll = nfd - 1;
            return;
        }
    }
    post("warning: %d removed from poll list but not found", fd);
}

t_socketreceiver *socketreceiver_new(t_pd *owner, int fd, t_socketnotifier notifier,
t_socketreceivefn socketreceivefn, int udp) {
    t_socketreceiver *x = (t_socketreceiver *)getbytes(sizeof(*x));
    x->inhead = x->intail = 0;
    x->owner = owner;
    x->notifier = notifier;
    x->socketreceivefn = socketreceivefn;
    x->udp = udp;
    x->fd = fd;
    x->obuf = 0;
    x->next = 0;
    x->inbuf = (char *)malloc(INBUFSIZE);
    if (!x->inbuf) bug("t_socketreceiver");
    return x;
}

void socketreceiver_free(t_socketreceiver *x) {free(x->inbuf); free(x);}

/* this is in a separately called subroutine so that the buffer isn't
   sitting on the stack while the messages are getting passed. */
static int socketreceiver_doread(t_socketreceiver *x) {
    char messbuf[INBUFSIZE], *bp = messbuf;
    int inhead = x->inhead;
    int intail = x->intail;
    char *inbuf = x->inbuf;
    if (intail == inhead) return 0;
    for (int i=intail; i!=inhead; i=(i+1)&(INBUFSIZE-1)) {
        /* ";" not preceded by "\" is a message boundary in current syntax.
           in future syntax it might become more complex. */
        char c = *bp++ = inbuf[i];
        if (c == ';' && (!i || inbuf[i-1] != '\\')) {
            intail = (i+1)&(INBUFSIZE-1);
            binbuf_text(inbinbuf, messbuf, bp - messbuf);
            x->inhead = inhead;
            x->intail = intail;
            return 1;
        }
    }
    return 0;
}

static void socketreceiver_getudp(t_socketreceiver *x, int fd) {
    char buf[INBUFSIZE+1];
    int ret = recv(fd, buf, INBUFSIZE, 0);
    if (ret < 0) {
        sys_sockerror("recv");
        sys_rmpollfn(fd);
        sys_closesocket(fd);
    } else if (ret > 0) {
        buf[ret] = 0;
        if (buf[ret-1] == '\n') {
            char *semi = strchr(buf, ';');
            if (semi) *semi = 0;
            binbuf_text(inbinbuf, buf, strlen(buf));
            outlet_setstacklim();
            x->socketreceivefn(x->owner, inbinbuf);
        } else {/*bad buffer ignored */}
    }
}

void sys_exit();

void socketreceiver_read(t_socketreceiver *x, int fd) {
    if (x->udp) {socketreceiver_getudp(x, fd); return;}
    /* else TCP */
    int readto = x->inhead >= x->intail ? INBUFSIZE : x->intail-1;
    /* the input buffer might be full.  If so, drop the whole thing */
    if (readto == x->inhead) {
	fprintf(stderr, "pd: dropped message from gui\n");
	x->inhead = x->intail = 0;
	readto = INBUFSIZE;
    } else {
	int ret = recv(fd, x->inbuf+x->inhead, readto-x->inhead, 0);
        if (ret<=0) {
	    if (ret<0) sys_sockerror("recv"); else post("EOF on socket %d", fd);
	    if (x->notifier) x->notifier(x->owner);
	    sys_rmpollfn(fd);
	    sys_closesocket(fd);
	    return;
	}
	x->inhead += ret;
	if (x->inhead >= INBUFSIZE) x->inhead = 0;
	while (socketreceiver_doread(x)) {
	    outlet_setstacklim();
	    x->socketreceivefn(x->owner, inbinbuf);
	}
    }
}

void sys_closesocket(int fd) {
#ifdef UNISTD
    close(fd);
#endif
#ifdef _WIN32
    closesocket(fd);
#endif /* _WIN32 */
}

/* ---------------------- sending messages to the GUI ------------------ */
#define GUI_ALLOCCHUNK 8192
#define GUI_UPDATESLICE 512 /* how much we try to do in one idle period */
#define GUI_BYTESPERPING 1024 /* how much we send up per ping */

static void sys_trytogetmoreguibuf(t_socketreceiver *self, int newsize) {
    self->osize = newsize;
    self->obuf = (char *)realloc(self->obuf, newsize);
}

#undef max /* for msys compat */
int max(int a, int b) { return ((a)>(b)?(a):(b)); }

std::ostringstream lost_posts;

void sys_vgui(char *fmt, ...) {
    t_socketreceiver *self = sys_socketreceiver;
    va_list ap;
    va_start(ap, fmt);
    if (!self) {voprintf(lost_posts,fmt,ap); va_end(ap); return;}
    if (!self->obuf) {
        self->obuf = (char *)malloc(GUI_ALLOCCHUNK);
        self->osize = GUI_ALLOCCHUNK;
        self->ohead = self->otail = 0;
    }
    if (self->ohead > self->osize - GUI_ALLOCCHUNK/2)
        sys_trytogetmoreguibuf(self,self->osize + GUI_ALLOCCHUNK);
    int msglen = vsnprintf(self->obuf+self->ohead, self->osize-self->ohead, fmt, ap);
    va_end(ap);
    if(msglen < 0) {fprintf(stderr, "Pd: buffer space wasn't sufficient for long GUI string\n"); return;}
    if (msglen >= self->osize - self->ohead) {
        int msglen2, newsize = self->osize+1+max(msglen,GUI_ALLOCCHUNK);
        sys_trytogetmoreguibuf(self,newsize);
        va_start(ap, fmt);
        msglen2 = vsnprintf(self->obuf+self->ohead, self->osize-self->ohead, fmt, ap);
        va_end(ap);
        if (msglen2 != msglen) bug("sys_vgui");
        if (msglen >= self->osize-self->ohead) msglen = self->osize-self->ohead;
    }
    self->ohead += msglen;
    self->bytessincelastping += msglen;
}

void sys_gui(char *s) {sys_vgui("%s", s);}

static int sys_flushtogui(t_socketreceiver *self) {
    int writesize = self->ohead-self->otail;
    if (!writesize) return 0;
    int nwrote = send(self->fd, self->obuf+self->otail, writesize, 0);
    if (nwrote < 0) {
        perror("pd-to-gui socket");
        sys_bail(1);
    } else if (!nwrote) {
        return 0;
    } else if (nwrote >= self->ohead-self->otail) {
         self->ohead = self->otail = 0;
    } else if (nwrote) {
        self->otail += nwrote;
        if (self->otail > self->osize>>2) {
            memmove(self->obuf, self->obuf+self->otail, self->ohead-self->otail);
            self->ohead -= self->otail;
            self->otail = 0;
        }
    }
    return 1;
}

void glob_ping(t_pd *dummy) {t_socketreceiver *self = sys_socketreceiver; self->waitingforping = 0;}

int sys_pollgui() {
	if (sys_socketreceiver) sys_flushtogui(sys_socketreceiver);
	return sys_pollsockets();
}

/* --------------------- starting up the GUI connection ------------- */

#ifdef __linux__
void glob_watchdog(t_pd *dummy) {
#ifndef WATCHDOGTHREAD
    if (write(sys_watchfd, "\n", 1) < 1) {
        fprintf(stderr, "pd: watchdog process died\n");
        sys_bail(1);
    }
#endif
}
#endif

static void sys_setsignals() {
#ifdef UNISTD
    signal(SIGHUP, sys_huphandler);
    signal(SIGINT, sys_exithandler);
    signal(SIGQUIT, sys_exithandler);
    signal(SIGILL, sys_exithandler);
    signal(SIGIOT, sys_exithandler);
    signal(SIGFPE, SIG_IGN);
/*  signal(SIGILL, sys_exithandler);
    signal(SIGBUS, sys_exithandler);
    signal(SIGSEGV, sys_exithandler); */
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
/* GG says: don't set SIGSTKFLT */
#endif
}

//t_pd *pd_new3(const char *s);

extern "C" t_text *netreceive_new(t_symbol *compatflag, t_floatarg fportno, t_floatarg udpflag);
static int sys_start_watchdog_thread();
static void sys_setpriority();
extern t_text *manager;

int sys_startgui() {
#ifdef _WIN32
    short version = MAKEWORD(2, 0);
    WSADATA nobby;
    if (WSAStartup(version, &nobby)) sys_sockerror("WSAstartup");
#endif /* _WIN32 */
    /* create an empty FD poll list */
    sys_fdpoll = (t_fdpoll *)t_getbytes(0);
    sys_nfdpoll = 0;
    inbinbuf = binbuf_new();
    sys_setsignals();
    sys_netreceive = netreceive_new(&s_,sys_guisetportnumber,0);
//    fprintf(stderr,"sys_netreceive=%p\n",sys_netreceive);
    if (!sys_netreceive) return 0;
//    obj_connect(sys_netreceive,0,(t_text *)pd_new3("print left"),0);
//    obj_connect(sys_netreceive,1,(t_text *)pd_new3("print right"),0);
    obj_connect(sys_netreceive,0,manager,0);
#if defined(__linux__) || defined(IRIX)
/* now that we've spun off the child process we can promote our process's
   priority, if we can and want to.  If not specfied (-1), we check root
   status.  This misses the case where we might have permission from a
   "security module" (linux 2.6) -- I don't know how to test for that.
   The "-rt" flag must be set in that case. */
    if (sys_hipriority == -1) sys_hipriority = !getuid() || !geteuid();
#endif
    if (sys_hipriority) sys_setpriority();
    setuid(getuid());
    return 0;
}

/* To prevent lockup, we fork off a watchdog process with higher real-time
   priority than ours.  The GUI has to send a stream of ping messages to the
   watchdog THROUGH the Pd process which has to pick them up from the GUI
   and forward them.  If any of these things aren't happening the watchdog
   starts sending "stop" and "cont" signals to the Pd process to make it
   timeshare with the rest of the system.  (Version 0.33P2 : if there's no
   GUI, the watchdog pinging is done from the scheduler idle routine in this
   process instead.) */
void sys_setpriority() {
#if defined(__linux__) || defined(IRIX)
#ifndef WATCHDOGTHREAD
    int pipe9[2];
    if (pipe(pipe9) < 0) {
        setuid(getuid());
        sys_sockerror("pipe");
        return 1;
    }
    int watchpid = fork();
    if (watchpid < 0) {
        setuid(getuid());
        if (errno) perror("sys_startgui"); else fprintf(stderr, "sys_startgui failed\n");
        return 1;
    } else if (!watchpid) { /* we're the child */
        sys_set_priority(1);
        setuid(getuid());
        if (pipe9[1]) {
            dup2(pipe9[0], 0);
            close(pipe9[0]);
        }
        close(pipe9[1]);
        sprintf(cmdbuf, "%s/pd-watchdog\n", guidir);
        if (sys_verbose) fprintf(stderr, "%s", cmdbuf);
        execl("/bin/sh", "sh", "-c", cmdbuf, (char*)0);
        perror("pd: exec");
        _exit(1);
    } else { /* we're the parent */
        sys_set_priority(0);
        setuid(getuid());
        close(pipe9[0]);
        sys_watchfd = pipe9[1];
        /* We also have to start the ping loop in the GUI; this is done later when the socket is open. */
    }
#else
	sys_start_watchdog_thread();
	sys_set_priority(0);
#endif
#endif /* __linux__ */
#ifdef MSW
    if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
        fprintf(stderr, "pd: couldn't set high priority class\n");
#endif
#ifdef __APPLE__
    if (sys_hipriority) {
        struct sched_param param;
        int policy = SCHED_RR;
        param.sched_priority = 80; /* adjust 0 : 100 */
        int err = pthread_setschedparam(pthread_self(), policy, &param);
        if (err) post("warning: high priority scheduling failed");
    }
#endif /* __APPLE__ */
    if (!sys_nogui) {
        /* here is where we start the pinging. */
#if defined(__linux__) || defined(IRIX)
#ifndef WATCHDOGTHREAD
        if (sys_hipriority) sys_gui("pdtk_watchdog\n");
#endif
#endif
    }
}

/* This is called when something bad has happened, like a segfault.
Call glob_quit() below to exit cleanly.
LATER try to save dirty documents even in the bad case. */
void sys_bail(int n) {
    static int reentered = 0;
    if (reentered) _exit(1);
    reentered = 1;
    fprintf(stderr, "closing audio...\n");
    sys_close_audio();
    fprintf(stderr, "closing MIDI...\n");
    sys_close_midi();
    fprintf(stderr, "... done.\n");
    exit(n);
}

extern "C" void glob_closeall(void *dummy, t_floatarg fforce);

void glob_quit(void *dummy) {
    glob_closeall(0, 1);
    sys_bail(0);
}

static pthread_t watchdog_id;
static pthread_t main_pd_thread;
static pthread_mutex_t watchdog_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t watchdog_cond = PTHREAD_COND_INITIALIZER;
static void *watchdog_thread(void *);

/* start a high priority watchdog thread */
static int sys_start_watchdog_thread() {
	pthread_attr_t w_attr;
	main_pd_thread = pthread_self();
	pthread_attr_init(&w_attr);
	pthread_attr_setschedpolicy(&w_attr, SCHED_FIFO); /* use rt scheduling */
	int status = pthread_create(&watchdog_id, &w_attr, (void*(*)(void*)) watchdog_thread, NULL);
	return status; /* what is this function supposed to return anyway? it tried returning void though declared as int */
}

#ifdef MSW
int gettimeofday (struct timeval *tv, void *tz);
#endif

static t_int* watchdog_callback(t_int *dummy) {
	/* signal the condition */
	pthread_cond_signal(&watchdog_cond);
	return 0;
}

/* this watchdog thread registers an idle callback once a minute 
   if the idle callback isn't excecuted within one minute, we're probably
   blocking the system.
   kill the whole process!
*/

static void *watchdog_thread(void *dummy) {
	sys_set_priority(1);
	post("watchdog thread started");
	sys_microsleep(60*1000*1000); /* wait 60 seconds ... hoping that everything is set up */
	post("watchdog thread active");
	while (1) {
		struct timespec timeout;
		struct timeval now;
		int status;
		gettimeofday(&now,0);
		timeout.tv_sec = now.tv_sec + 15; /* timeout: 15 seconds */
		timeout.tv_nsec = now.tv_usec * 1000;
		sys_callback((t_int(*)(t_int*))watchdog_callback, 0, 0);
		status = pthread_cond_timedwait(&watchdog_cond, &watchdog_mutex, &timeout);
		if (status) {
#if defined(__linux__) || defined(IRIX)
			fprintf(stderr, "watchdog killing");
			kill(0,9); /* kill parent thread */
#endif
		}
		sys_microsleep(15*1000*1000); /* and sleep for another 15 seconds */
	}
	return 0; /* avoid warning */
}
