/* Copyright (c) 2007 Mathieu Bouchard
   Copyright (c) 1997-1999 Miller Puckette.
   For information on usage and redistribution,
   and for a DISCLAIMER OF ALL WARRANTIES,
   see the file "LICENSE.txt" in this distribution.  */

#define PD_PLUSPLUS_FACE
#include "desire.h"
using namespace desire;
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#ifdef UNISTD
#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/param.h>
#include <unistd.h>
#endif
#ifdef MSW
#include <wtypes.h>
#include <time.h>
#endif
#ifdef MSW
#include <winsock.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#define SOCKET_ERROR -1
#endif
#ifdef MSW
#include <io.h>
#endif
#if defined (__APPLE__) || defined (__FreeBSD__)
#define HZ CLK_TCK
#endif
#include "config.h"

#define a_float   a_w.w_float
#define a_symbol  a_w.w_symbol
#define a_pointer a_w.w_gpointer

#define LIST_NGETBYTE 100 /* bigger that this we use alloc, not alloca */

/* #include <string.h> */
#ifdef MSW
#include <malloc.h>
#else
#include <alloca.h>
#endif

#define LOGTEN 2.302585092994
#undef min
#undef max

//conflict with min,max
//using namespace std;

float mtof(float f) {return f>-1500 ? 8.17579891564 * exp(.0577622650 * min(f,1499.f)) : 0;}
float ftom(float f) {return f>0 ? 17.3123405046 * log(.12231220585 * f) : -1500;}
float powtodb(float f) {return f>0 ? max(100 + 10./LOGTEN * log(f),0.) : 0;}
float rmstodb(float f) {return f>0 ? max(100 + 20./LOGTEN * log(f),0.) : 0;}
float dbtopow(float f) {return f>0 ? exp((LOGTEN * 0.1 ) * (min(f,870.f)-100.)) : 0;}
float dbtorms(float f) {return f>0 ? exp((LOGTEN * 0.05) * (min(f,485.f)-100.)) : 0;}

/* ------------- corresponding objects ----------------------- */

#define FUNC1(C,EXPR) \
static t_class *C##_class; \
static void *C##_new() { \
    t_object *x = (t_object *)pd_new(C##_class); \
    outlet_new(x, &s_float); return x;} \
static void C##_float(t_object *x, t_float a) {x->outlet->send(EXPR);}
#define FUNC1DECL(C,SYM) \
    C##_class = class_new2(SYM,C##_new,0,sizeof(t_object),0,""); \
    class_addfloat(C##_class, (t_method)C##_float); \
    class_sethelpsymbol(C##_class,s);

FUNC1(mtof,   mtof(a))
FUNC1(ftom,   ftom(a))
FUNC1(powtodb,powtodb(a))
FUNC1(rmstodb,rmstodb(a))
FUNC1(dbtorms,dbtorms(a))
FUNC1(dbtopow,dbtopow(a))

/* -------------------------- openpanel ------------------------------ */
static t_class *openpanel_class;
struct t_openpanel : t_object {
    t_symbol *s;
    t_symbol *path;
};
static void *openpanel_new(t_symbol *s) {
    t_openpanel *x = (t_openpanel *)pd_new(openpanel_class);
    x->s = symprintf("d%lx",(t_int)x);
    x->path = s;
    pd_bind(x,x->s);
    outlet_new(x,&s_symbol);
    return x;
}
static void openpanel_bang(t_openpanel *x) {
    sys_vgui("pdtk_openpanel {%s} {%s}\n", x->s->name, (x->path&&x->path->name)?x->path->name:"\"\"");
}
static void openpanel_symbol(t_openpanel *x, t_symbol *s) {
    sys_vgui("pdtk_openpanel {%s} {%s}\n", x->s->name, (s && s->name) ? s->name : "\"\"");
}
static void openpanel_callback(t_openpanel *x, t_symbol *s) {x->outlet->send(s);}
static void openpanel_path(t_openpanel *x, t_symbol *s) {x->path=s;}
static void openpanel_free(t_openpanel *x) {pd_unbind(x, x->s);}
static void openpanel_setup() {
    t_class *c = openpanel_class = class_new2("openpanel",openpanel_new,openpanel_free,sizeof(t_openpanel),0,"S");
    class_addbang(c, openpanel_bang);
    class_addmethod2(c, openpanel_path, "path","s");
    class_addmethod2(c, openpanel_callback,"callback","s");
    class_addsymbol(c, openpanel_symbol);
}

/* -------------------------- savepanel ------------------------------ */
static t_class *savepanel_class;
struct t_savepanel : t_object {
    t_symbol *s;
    t_symbol *path;
};
static void *savepanel_new(t_symbol*s) {
    t_savepanel *x = (t_savepanel *)pd_new(savepanel_class);
    x->s = symprintf("d%lx",(t_int)x);
    x->path=s;
    pd_bind(x, x->s);
    outlet_new(x, &s_symbol);
    return x;
}
static void savepanel_bang(t_savepanel *x) {
    sys_vgui("pdtk_savepanel {%s} {%s}\n", x->s->name, (x->path&&x->path->name)?x->path->name:"\"\"");
}
static void savepanel_symbol(t_savepanel *x, t_symbol *s) {
    sys_vgui("pdtk_savepanel {%s} {%s}\n", x->s->name, (s && s->name) ? s->name : "\"\"");
}
static void savepanel_callback(t_savepanel *x, t_symbol *s) {x->outlet->send(s);}
static void savepanel_free(t_savepanel *x) {pd_unbind(x, x->s);}
static void savepanel_setup() {
    t_class *c = savepanel_class = class_new2("savepanel",savepanel_new,savepanel_free,sizeof(t_savepanel),0,"S");
    class_addbang(c, savepanel_bang);
    class_addmethod2(c, openpanel_path, "path","s");
    class_addmethod2(c, savepanel_callback, "callback","s");
    class_addsymbol(c, savepanel_symbol);
}

/* ---------------------- key and its relatives ------------------ */
static t_symbol *key_sym,   *keyup_sym,   *keyname_sym;
static t_class  *key_class, *keyup_class, *keyname_class;
struct t_key : t_object {};
static void *key_new() {
    t_key *x = (t_key *)pd_new(key_class);
    outlet_new(x, &s_float);
    pd_bind(x, key_sym);
    return x;
}
static void key_float(t_key *x, t_floatarg f) {x->outlet->send(f);}

struct t_keyup : t_object {};
static void *keyup_new() {
    t_keyup *x = (t_keyup *)pd_new(keyup_class);
    outlet_new(x, &s_float);
    pd_bind(x, keyup_sym);
    return x;
}
static void keyup_float(t_keyup *x, t_floatarg f) {x->outlet->send(f);}

struct t_keyname : t_object {};
static void *keyname_new() {
    t_keyname *x = (t_keyname *)pd_new(keyname_class);
    outlet_new(x, &s_float);
    outlet_new(x, &s_symbol);
    pd_bind(x, keyname_sym);
    return x;
}
static void keyname_list(t_keyname *x, t_symbol *s, int ac, t_atom *av) {
    x->out(1)->send(atom_getsymbolarg(1, ac, av));
    x->out(0)->send(atom_getfloatarg( 0, ac, av));
}
static void key_free(    t_key *x)     {pd_unbind(x, key_sym);}
static void keyup_free(  t_keyup *x)   {pd_unbind(x, keyup_sym);}
static void keyname_free(t_keyname *x) {pd_unbind(x, keyname_sym);}

static void key_setup() {
    key_class     = class_new2("key",    key_new,    key_free,    sizeof(t_key),    CLASS_NOINLET,"");
    keyup_class   = class_new2("keyup",  keyup_new,  keyup_free,  sizeof(t_keyup),  CLASS_NOINLET,"");
    keyname_class = class_new2("keyname",keyname_new,keyname_free,sizeof(t_keyname),CLASS_NOINLET,"");
    class_addfloat(key_class,     key_float);
    class_addfloat(keyup_class,   keyup_float);
    class_addlist( keyname_class, keyname_list);
    class_sethelpsymbol(keyup_class, gensym("key"));
    class_sethelpsymbol(keyname_class, gensym("key"));
    key_sym     = gensym("#key");
    keyup_sym   = gensym("#keyup");
    keyname_sym = gensym("#keyname");
}

static t_class *netsend_class;
struct t_netsend : t_object {
    int fd;
    int protocol;
};
static void *netsend_new(t_floatarg udpflag) {
    t_netsend *x = (t_netsend *)pd_new(netsend_class);
    outlet_new(x, &s_float);
    x->fd = -1;
    x->protocol = (udpflag != 0 ? SOCK_DGRAM : SOCK_STREAM);
    return x;
}
static void netsend_connect(t_netsend *x, t_symbol *hostname, t_floatarg fportno) {
    struct sockaddr_in server;
    int portno = (int)fportno;
    if (x->fd >= 0) {error("netsend_connect: already connected"); return;}
    /* create a socket */
    int sockfd = socket(AF_INET, x->protocol, 0);
    if (sockfd < 0) {sys_sockerror("socket"); return;}
    /* connect socket using hostname provided in command line */
    server.sin_family = AF_INET;
    struct hostent *hp = gethostbyname(hostname->name);
    if (!hp) {error("bad host?"); return;}
#if 0
    int intarg = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char*)&intarg, sizeof(intarg)) < 0)
            error("setsockopt (SO_RCVBUF) failed");
#endif
    /* for stream (TCP) sockets, specify "nodelay" */
    if (x->protocol == SOCK_STREAM) {
        int intarg = 1;
        if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&intarg, sizeof(intarg)) < 0)
                error("setsockopt (TCP_NODELAY) failed");
    }
    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);
    /* assign client port number */
    server.sin_port = htons((u_short)portno);
    post("connecting to port %d", portno);
    /* try to connect.  LATER make a separate thread to do this because it might block */
    if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0) {
        sys_sockerror("connecting stream socket");
        sys_closesocket(sockfd);
        return;
    }
    x->fd = sockfd;
    x->outlet->send(1.);
}

static void netsend_disconnect(t_netsend *x) {
    if (x->fd < 0) return;
    sys_closesocket(x->fd);
    x->fd = -1;
    x->outlet->send(0.);
}
static void netsend_send(t_netsend *x, t_symbol *s, int argc, t_atom *argv) {
    if (x->fd < 0) {error("netsend: not connected"); return;}
    t_binbuf *b = binbuf_new();
    t_atom at;
    binbuf_add(b, argc, argv);
    SETSEMI(&at);
    binbuf_add(b, 1, &at);
    char *buf;
    int length, sent=0;
    binbuf_gettext(b, &buf, &length);
    char *bp = buf;
    while (sent < length) {
        static double lastwarntime;
        static double pleasewarn;
        double timebefore = sys_getrealtime();
        int res = send(x->fd, buf, length-sent, 0);
        double timeafter = sys_getrealtime();
        int late = (timeafter - timebefore > 0.005);
        if (late || pleasewarn) {
            if (timeafter > lastwarntime + 2) {
                 post("netsend blocked %d msec", (int)(1000 * ((timeafter - timebefore) + pleasewarn)));
                 pleasewarn = 0;
                 lastwarntime = timeafter;
            } else if (late) pleasewarn += timeafter - timebefore;
        }
        if (res <= 0) {
            sys_sockerror("netsend");
            netsend_disconnect(x);
            break;
        } else {
            sent += res;
            bp += res;
        }
    }
    free(buf);
    binbuf_free(b);
}
static void netsend_free(t_netsend *x) {netsend_disconnect(x);}
static void netsend_setup() {
    netsend_class = class_new2("netsend",netsend_new,netsend_free,sizeof(t_netsend),0,"F");
    class_addmethod2(netsend_class, netsend_connect, "connect","sf");
    class_addmethod2(netsend_class, netsend_disconnect, "disconnect","");
    class_addmethod2(netsend_class, netsend_send, "send","*");
}

static t_class *netreceive_class;
struct t_netreceive : t_object {
    t_outlet *msgout;
    t_outlet *connectout;
    int connectsocket;
    int nconnections;
    int udp;
/* only used for sending (bidirectional socket to desire.tk) */
    t_socketreceiver *sr;
};
static void netreceive_notify(t_netreceive *x) {
    x->connectout->send(--x->nconnections);
}
static void netreceive_doit(t_netreceive *x, t_binbuf *b) {
    int natom = binbuf_getnatom(b);
    t_atom *at = binbuf_getvec(b);
    for (int msg = 0; msg < natom;) {
	int emsg;
        for (emsg = msg; emsg < natom && at[emsg].a_type != A_COMMA && at[emsg].a_type != A_SEMI; emsg++) {}
        if (emsg > msg) {
            for (int i = msg; i < emsg; i++) if (at[i].a_type == A_DOLLAR || at[i].a_type == A_DOLLSYM) {
                error("netreceive: got dollar sign in message");
                goto nodice;
            }
            if (at[msg].a_type == A_FLOAT) {
                if (emsg > msg + 1) x->msgout->send(emsg-msg, at + msg);
                else x->msgout->send(at[msg].a_float);
            } else if (at[msg].a_type == A_SYMBOL)
                x->msgout->send(at[msg].a_symbol,emsg-msg-1,at+msg+1);
        }
    nodice:
        msg = emsg + 1;
    }
}
static void netreceive_connectpoll(t_netreceive *x) {
    int fd = accept(x->connectsocket, 0, 0);
    if (fd < 0) post("netreceive: accept failed");
    else {
        t_socketreceiver *y = socketreceiver_new((t_pd *)x, fd,
            (t_socketnotifier)netreceive_notify, x->msgout?(t_socketreceivefn)netreceive_doit:0, 0);
        sys_addpollfn(fd, (t_fdpollfn)socketreceiver_read, y);
        x->connectout->send(++x->nconnections);
	y->next = x->sr;
	x->sr = y;
    }
}
extern "C" t_text *netreceive_new(t_symbol *compatflag, t_floatarg fportno, t_floatarg udpflag) {
    struct sockaddr_in server;
    int udp = !!udpflag;
    int old = !strcmp(compatflag->name , "old");
    int intarg;
    /* create a socket */
    int sockfd = socket(AF_INET, (udp ? SOCK_DGRAM : SOCK_STREAM), 0);
    if (sockfd < 0) {sys_sockerror("socket"); return 0;}
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
#if 1
    /* ask OS to allow another Pd to reopen this port after we close it. */
    intarg = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char*)&intarg, sizeof(intarg)) < 0)
            post("setsockopt (SO_REUSEADDR) failed");
#endif
#if 0
    intarg = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &intarg, sizeof(intarg)) < 0)
            post("setsockopt (SO_RCVBUF) failed");
#endif
    /* Stream (TCP) sockets are set NODELAY */
    if (!udp) {
        intarg = 1;
        if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&intarg, sizeof(intarg)) < 0)
                post("setsockopt (TCP_NODELAY) failed");
    }
    /* assign server port number */
    server.sin_port = htons((u_short)fportno);
    /* name the socket */
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        sys_sockerror("bind");
        sys_closesocket(sockfd);
        return 0;
    }
    t_netreceive *x = (t_netreceive *)pd_new(netreceive_class);
    if (old) {
        /* old style, nonsecure version */
        x->msgout = 0;
    } else x->msgout = outlet_new(x, &s_anything);
    if (udp) { /* datagram protocol */
        t_socketreceiver *y = socketreceiver_new((t_pd *)x, sockfd, (t_socketnotifier)netreceive_notify,
                x->msgout ? (t_socketreceivefn)netreceive_doit : 0, 1);
        sys_addpollfn(sockfd, (t_fdpollfn)socketreceiver_read, y);
        x->connectout = 0;
    } else { /* streaming protocol */
        if (listen(sockfd, 5) < 0) {
            sys_sockerror("listen");
            sys_closesocket(sockfd);
            sockfd = -1;
        } else {
            sys_addpollfn(sockfd, (t_fdpollfn)netreceive_connectpoll, x);
            x->connectout = outlet_new(x, &s_float);
        }
    }
    x->connectsocket = sockfd;
    x->nconnections = 0;
    x->udp = udp;
    x->sr = 0;
    return x;
}
static void netreceive_free(t_netreceive *x) {
    /* LATER make me clean up open connections */
    if (x->connectsocket >= 0) {
        sys_rmpollfn(x->connectsocket);
        sys_closesocket(x->connectsocket);
    }
}
static void netreceive_setup() {
    netreceive_class = class_new2("netreceive",netreceive_new,netreceive_free,sizeof(t_netreceive),CLASS_NOINLET,"FFS");
}
extern "C" t_socketreceiver *netreceive_newest_receiver(t_netreceive *x) {return x->sr;}

struct t_qlist : t_object {
    t_binbuf *binbuf;
    int onset;                /* playback position */
    t_clock *clock;
    float tempo;
    double whenclockset;
    float clockdelay;
    t_symbol *dir;
    t_canvas *canvas;
    int reentered;
};
static void qlist_tick(t_qlist *x);
static t_class *qlist_class;
static void *qlist_new() {
    t_qlist *x = (t_qlist *)pd_new(qlist_class);
    x->binbuf = binbuf_new();
    x->clock = clock_new(x, (t_method)qlist_tick);
    outlet_new(x, &s_list);
    outlet_new(x, &s_bang);
    x->onset = 0x7fffffff;
    x->tempo = 1;
    x->whenclockset = 0;
    x->clockdelay = 0;
    x->canvas = canvas_getcurrent();
    x->reentered = 0;
    return x;
}
static void qlist_rewind(t_qlist *x) {
    x->onset = 0;
    if (x->clock) clock_unset(x->clock);
    x->whenclockset = 0;
    x->reentered = 1;
}
static void qlist_donext(t_qlist *x, int drop, int automatic) {
    t_pd *target = 0;
    while (1) {
        int argc = binbuf_getnatom(x->binbuf), count, onset = x->onset, onset2, wasreentered;
        t_atom *argv = binbuf_getvec(x->binbuf);
        t_atom *ap = argv + onset, *ap2;
        if (onset >= argc) goto end;
        while (ap->a_type == A_SEMI || ap->a_type == A_COMMA) {
            if (ap->a_type == A_SEMI) target = 0;
            onset++, ap++;
            if (onset >= argc) goto end;
        }
        if (!target && ap->a_type == A_FLOAT) {
            ap2 = ap + 1;
            onset2 = onset + 1;
            while (onset2 < argc && ap2->a_type == A_FLOAT) onset2++, ap2++;
            x->onset = onset2;
            if (automatic) {
                clock_delay(x->clock, x->clockdelay = ap->a_float * x->tempo);
                x->whenclockset = clock_getsystime();
            } else x->outlet->send(onset2-onset,ap);
            return;
        }
        ap2 = ap + 1;
        onset2 = onset + 1;
        while (onset2 < argc && (ap2->a_type == A_FLOAT || ap2->a_type == A_SYMBOL)) onset2++, ap2++;
        x->onset = onset2;
        count = onset2 - onset;
        if (!target) {
            if (ap->a_type != A_SYMBOL) continue;
            target = ap->a_symbol->thing;
	    if (!target) {error("qlist: %s: no such object", ap->a_symbol->name); continue;}
            ap++;
            onset++;
            count--;
            if (!count) {x->onset = onset2; continue;}
        }
        wasreentered = x->reentered;
        x->reentered = 0;
        if (!drop) {
            if      (ap->a_type == A_FLOAT)  typedmess(target, &s_list, count, ap);
            else if (ap->a_type == A_SYMBOL) typedmess(target, ap->a_symbol, count-1, ap+1);
        }
        if (x->reentered) return;
        x->reentered = wasreentered;
    }  /* while (1); never falls through */

end:
    x->onset = 0x7fffffff;
    x->out(1)->send();
    x->whenclockset = 0;
}
static void qlist_next(t_qlist *x, t_floatarg drop) {qlist_donext(x, drop != 0, 0);}
static void qlist_bang(t_qlist *x) {qlist_rewind(x); qlist_donext(x, 0, 1);}
static void qlist_tick(t_qlist *x) {x->whenclockset=0; qlist_donext(x, 0, 1);}
static void qlist_add(t_qlist *x, t_symbol *s, int ac, t_atom *av) {
    t_atom a;
    SETSEMI(&a);
    binbuf_add(x->binbuf, ac, av);
    binbuf_add(x->binbuf, 1, &a);
}
static void qlist_add2(t_qlist *x, t_symbol *s, int ac, t_atom *av) {
    binbuf_add(x->binbuf, ac, av);
}
static void qlist_clear(t_qlist *x) {
    qlist_rewind(x);
    binbuf_clear(x->binbuf);
}
static void qlist_set(t_qlist *x, t_symbol *s, int ac, t_atom *av) {
    qlist_clear(x);
    qlist_add(x, s, ac, av);
}
static void qlist_read(t_qlist *x, t_symbol *filename, t_symbol *format) {
    int cr = 0;
    if (!strcmp(format->name, "cr")) cr = 1;
    else if (*format->name) error("qlist_read: unknown flag: %s", format->name);
    if (binbuf_read_via_canvas(x->binbuf, filename->name, x->canvas, cr))
	error("%s: read failed", filename->name);
    x->onset = 0x7fffffff;
    x->reentered = 1;
}
static void qlist_write(t_qlist *x, t_symbol *filename, t_symbol *format) {
    int cr = 0;
    char *buf = canvas_makefilename(x->canvas,filename->name,0,0);
    if (!strcmp(format->name, "cr")) cr = 1;
    else if (*format->name) error("qlist_read: unknown flag: %s", format->name);
    if (binbuf_write(x->binbuf,buf,"",cr)) error("%s: write failed", filename->name);
    free(buf);
}
static void qlist_print(t_qlist *x) {
    post("--------- textfile or qlist contents: -----------");
    binbuf_print(x->binbuf);
}
static void qlist_tempo(t_qlist *x, t_float f) {
    float newtempo;
    if (f < 1e-20) f = 1e-20;
    else if (f > 1e20) f = 1e20;
    newtempo = 1./f;
    if (x->whenclockset != 0) {
        float elapsed = clock_gettimesince(x->whenclockset);
        float left = x->clockdelay - elapsed;
        if (left < 0) left = 0;
        left *= newtempo / x->tempo;
        clock_delay(x->clock, left);
    }
    x->tempo = newtempo;
}
static void qlist_free(t_qlist *x) {
    binbuf_free(x->binbuf);
    if (x->clock) clock_free(x->clock);
}

static t_class *textfile_class;
typedef t_qlist t_textfile;
static void *textfile_new() {
    t_textfile *x = (t_textfile *)pd_new(textfile_class);
    x->binbuf = binbuf_new();
    outlet_new(x, &s_list);
    outlet_new(x, &s_bang);
    x->onset = 0x7fffffff;
    x->reentered = 0;
    x->tempo = 1;
    x->whenclockset = 0;
    x->clockdelay = 0;
    x->clock = NULL;
    x->canvas = canvas_getcurrent();
    return x;
}
static void textfile_bang(t_textfile *x) {
    int argc = binbuf_getnatom(x->binbuf), onset = x->onset, onset2;
    t_atom *argv = binbuf_getvec(x->binbuf);
    t_atom *ap = argv + onset, *ap2;
    while (onset < argc && (ap->a_type == A_SEMI || ap->a_type == A_COMMA)) onset++, ap++;
    onset2 = onset;
    ap2 = ap;
    while (onset2 < argc && (ap2->a_type != A_SEMI && ap2->a_type != A_COMMA)) onset2++, ap2++;
    if (onset2 > onset) {
        x->onset = onset2;
        if (ap->a_type == A_SYMBOL) x->outlet->send(ap->a_symbol,onset2-onset-1,ap+1);
        else                        x->outlet->send(             onset2-onset  ,ap  );
    } else {
        x->onset = 0x7fffffff;
        x->out(1)->send();
    }
}

static void textfile_rewind(t_qlist *x) {x->onset = 0;}
static void textfile_free(t_textfile *x) {binbuf_free(x->binbuf);}

extern t_pd *newest;

/* the "list" object family.

    list append - append a list to another
    list prepend - prepend a list to another
    list split - first n elements to first outlet, rest to second outlet
    list trim - trim off "list" selector
    list length - output number of items in list

Need to think more about:
    list foreach - spit out elements of a list one by one (also in reverse?)
    list array - get items from a named array as a list
    list reverse - permute elements of a list back to front
    list pack - synonym for 'pack'
    list unpack - synonym for 'unpack'
    list cat - build a list by accumulating elements

Probably don't need:
    list first - output first n elements.
    list last - output last n elements
    list nth - nth item in list, counting from zero
*/

/* -------------- utility functions: storage, copying  -------------- */

#if HAVE_ALLOCA
#define ATOMS_ALLOCA(x, n) ((x) = (t_atom *)((n) < LIST_NGETBYTE ?  \
        alloca((n) * sizeof(t_atom)) : getbytes((n) * sizeof(t_atom))))
#define ATOMS_FREEA(x, n) ( \
    ((n) < LIST_NGETBYTE || (free((x)), 0)))
#else
#define ATOMS_ALLOCA(x, n) ((x) = (t_atom *)getbytes((n) * sizeof(t_atom)))
#define ATOMS_FREEA(x, n) (free((x)))
#endif

static void atoms_copy(int argc, t_atom *from, t_atom *to) {
    for (int i = 0; i < argc; i++) to[i] = from[i];
}

/* ------------- fake class to divert inlets to ----------------- */
static void alist_list(t_binbuf *x, t_symbol *s, int argc, t_atom *argv) {
    binbuf_clear(x);
    x->v = (t_atom *)getbytes(argc * sizeof(*x->v));
    if (!x->v) {x->n = 0; error("list_alloc: out of memory"); return;}
    x->n = argc;
    for (int i = 0; i < argc; i++) x->v[i] = argv[i];
}
static void alist_anything(t_binbuf *x, t_symbol *s, int argc, t_atom *argv) {
    binbuf_clear(x);
    x->v = (t_atom *)getbytes((argc+1) * sizeof(*x->v));
    if (!x->v) {x->n = 0; error("list_alloc: out of memory"); return;}
    x->n = argc+1;
    SETSYMBOL(&x->v[0], s);
    for (int i = 0; i < argc; i++) x->v[i+1] = argv[i];
}
static void alist_toatoms(t_binbuf *x, t_atom *to) {for (size_t i=0; i<x->n; i++) to[i] = x->v[i];}

//t_class *list_any_class;     struct t_list_any     : t_object {t_binbuf *alist;};
t_class *list_append_class;  struct t_list_append  : t_object {t_binbuf *alist;};
t_class *list_prepend_class; struct t_list_prepend : t_object {t_binbuf *alist;};
t_class *list_split_class;   struct t_list_split : t_object {t_float f;};
t_class *list_trim_class;    struct t_list_trim : t_object {};
t_class *list_length_class;  struct t_list_length : t_object {};

static t_pd *list_append_new(t_symbol *s, int argc, t_atom *argv) {
    t_list_append *x = (t_list_append *)pd_new(list_append_class);
    x->alist = binbuf_new(); alist_list(x->alist, 0, argc, argv); outlet_new(x, &s_list); inlet_new(x,x->alist, 0, 0);
    return x;
}
static t_pd *list_prepend_new(t_symbol *s, int argc, t_atom *argv) {
    t_list_prepend *x = (t_list_prepend *)pd_new(list_prepend_class);
    x->alist = binbuf_new(); alist_list(x->alist, 0, argc, argv); outlet_new(x, &s_list); inlet_new(x,x->alist,0,0);
    return x;
}
static void list_append_free (t_list_append *x)  {binbuf_free(x->alist);}
static void list_prepend_free(t_list_prepend *x) {binbuf_free(x->alist);}

static void list_append_list(t_list_append *x, t_symbol *s, int argc, t_atom *argv) {
    t_atom *outv; int outc = x->alist->n + argc; ATOMS_ALLOCA(outv, outc);
    atoms_copy(argc, argv, outv);
    alist_toatoms(x->alist, outv+argc);
    x->outlet->send(outc,outv); ATOMS_FREEA(outv,outc);
}
static void list_append_anything(t_list_append *x, t_symbol *s, int argc, t_atom *argv) {
    t_atom *outv; int outc = x->alist->n+argc+1; ATOMS_ALLOCA(outv, outc);
    SETSYMBOL(outv, s);
    atoms_copy(argc, argv, outv + 1);
    alist_toatoms(x->alist, outv + 1 + argc);
    x->outlet->send(outc,outv); ATOMS_FREEA(outv,outc);
}
static void list_prepend_list(t_list_prepend *x, t_symbol *s, int argc, t_atom *argv) {
    t_atom *outv; int outc = x->alist->n + argc; ATOMS_ALLOCA(outv, outc);
    alist_toatoms(x->alist, outv);
    atoms_copy(argc, argv, outv + x->alist->n);
    x->outlet->send(outc,outv); ATOMS_FREEA(outv,outc);
}
static void list_prepend_anything(t_list_prepend *x, t_symbol *s, int argc, t_atom *argv) {
    t_atom *outv; int outc = x->alist->n+argc+1; ATOMS_ALLOCA(outv, outc);
    alist_toatoms(x->alist, outv);
    SETSYMBOL(outv + x->alist->n, s);
    atoms_copy(argc, argv, outv + x->alist->n + 1);
    x->outlet->send(outc,outv); ATOMS_FREEA(outv,outc);
}
static t_pd *list_split_new(t_floatarg f) {
    t_list_split *x = (t_list_split *)pd_new(list_split_class);
    outlet_new(x, &s_list);
    outlet_new(x, &s_list);
    outlet_new(x, &s_list);
    floatinlet_new(x, &x->f);
    x->f = f;
    return x;
}
static void list_split_list(t_list_split *x, t_symbol *s, int argc, t_atom *argv) {
    int n = (int)x->f;
    if (n<0) n=0;
    if (argc >= n) {
        x->out(1)->send(argc-n,argv+n);
        x->out(0)->send(n,argv);
    } else x->out(2)->send(argc,argv);
}
static void list_split_anything(t_list_split *x, t_symbol *s, int argc, t_atom *argv) {
    t_atom *outv;
    ATOMS_ALLOCA(outv, argc+1);
    SETSYMBOL(outv, s);
    atoms_copy(argc, argv, outv + 1);
    list_split_list(x, &s_list, argc+1, outv);
    ATOMS_FREEA(outv, argc+1);
}

static t_pd *list_trim_new() {
    t_list_trim *x = (t_list_trim *)pd_new(list_trim_class);
    outlet_new(x, &s_list);
    return x;
}
static void list_trim_list(t_list_trim *x, t_symbol *s, int argc, t_atom *argv) {
    if (argc < 1 || argv[0].a_type != A_SYMBOL) x->outlet->send(argc,argv);
    else x->outlet->send(argv[0].a_symbol,argc-1,argv+1);
}
static void list_trim_anything(t_list_trim *x, t_symbol *s, int argc, t_atom *argv) {x->outlet->send(s,argc,argv);}

static t_pd *list_length_new() {
    t_list_length *x = (t_list_length *)pd_new(list_length_class);
    outlet_new(x, &s_float);
    return x;
}
static void list_length_list(    t_list_length *x, t_symbol *s, int argc, t_atom *argv) {x->outlet->send(float(argc));}
static void list_length_anything(t_list_length *x, t_symbol *s, int argc, t_atom *argv) {x->outlet->send(float(argc+1));}

static void *list_new(t_pd *dummy, t_symbol *s, int argc, t_atom *argv) {
    t_pd *newest = 0; /* hide global var */
    if (!argc || argv[0].a_type != A_SYMBOL) newest = list_append_new(s, argc, argv);
    else {
        t_symbol *s2 = argv[0].a_symbol;
        if      (s2 == gensym("append"))  newest = list_append_new(s, argc-1, argv+1);
        else if (s2 == gensym("prepend")) newest = list_prepend_new(s, argc-1, argv+1);
        else if (s2 == gensym("split"))   newest = list_split_new(atom_getfloatarg(1, argc, argv));
        else if (s2 == gensym("trim"))    newest = list_trim_new();
        else if (s2 == gensym("length"))  newest = list_length_new();
        else error("list %s: unknown function", s2->name);
    }
    /* workaround for bug in kernel.c */
    if (newest) pd_set_newest(newest);
    return newest;
}

#define LISTOP(name,argspec,freer) \
  list_##name##_class = class_new2("list " #name,list_##name##_new,freer,sizeof(t_list_##name),0,argspec); \
    class_addlist(list_##name##_class, list_##name##_list); \
    class_addanything(list_##name##_class, list_##name##_anything); \
    class_sethelpsymbol(list_##name##_class, &s_list);

static void list_setup () {
    LISTOP(append,"*",list_append_free)
    LISTOP(prepend,"*",list_prepend_free)
    LISTOP(split,"F",0)
    LISTOP(trim,"",0)
    LISTOP(length,"",0)
    class_addcreator2("list",list_new,"*");
}

/* miller's "homebrew" linear-congruential algorithm */
static t_class *random_class;
struct t_random : t_object {
    t_float f;
    unsigned int state;
};
static int makeseed() {
    static unsigned int random_nextseed = 1489853723;
    random_nextseed = random_nextseed * 435898247 + 938284287;
    return random_nextseed & 0x7fffffff;
}
static void *random_new(t_floatarg f) {
    t_random *x = (t_random *)pd_new(random_class);
    x->f = f;
    x->state = makeseed();
    floatinlet_new(x,&x->f);
    outlet_new(x,&s_float);
    return x;
}
static void random_bang(t_random *x) {
    int n = (int)x->f, nval;
    int range = max(n,1);
    unsigned int randval = x->state;
    x->state = randval = randval * 472940017 + 832416023;
    nval = (int)((double)range * (double)randval * (1./4294967296.));
    if (nval >= range) nval = (int)(range-1);
    x->outlet->send(nval);
}
static void random_seed(t_random *x, float f, float glob) {x->state = (int)f;}
static void random_setup() {
    random_class = class_new2("random",random_new,0,sizeof(t_random),0,"F");
    class_addbang(random_class, random_bang);
    class_addmethod2(random_class, random_seed,"seed","f");
}

static t_class *loadbang_class;
struct t_loadbang : t_object {};
static void *loadbang_new() {
    t_loadbang *x = (t_loadbang *)pd_new(loadbang_class);
    outlet_new(x,&s_bang);
    return x;
}
static void loadbang_loadbang(t_loadbang *x) {
    if (!sys_noloadbang) x->outlet->send();
}
static void loadbang_setup() {
    loadbang_class = class_new2("loadbang",loadbang_new,0,sizeof(t_loadbang),CLASS_NOINLET,"");
    class_addmethod2(loadbang_class, loadbang_loadbang, "loadbang","");
}

static t_class *namecanvas_class;
struct t_namecanvas : t_object {
    t_symbol *sym;
    t_pd *owner;
};
static void *namecanvas_new(t_symbol *s) {
    t_namecanvas *x = (t_namecanvas *)pd_new(namecanvas_class);
    x->owner = (t_pd *)canvas_getcurrent();
    x->sym = s;
    if (*s->name) pd_bind(x->owner, s);
    return x;
}
static void namecanvas_free(t_namecanvas *x) {
    if (*x->sym->name) pd_unbind(x->owner, x->sym);
}
static void namecanvas_setup() {
    namecanvas_class = class_new2("namecanvas",namecanvas_new,namecanvas_free,sizeof(t_namecanvas),CLASS_NOINLET,"S");
}

static t_class *cputime_class;
struct t_cputime : t_object {
#ifdef UNISTD
    struct tms setcputime;
#endif
#ifdef MSW
    LARGE_INTEGER kerneltime;
    LARGE_INTEGER usertime;
    bool warned;
#endif
};
static t_class *realtime_class; struct t_realtime : t_object {double setrealtime;};
static t_class *timer_class;    struct t_timer    : t_object {double settime;};


static void cputime_bang(t_cputime *x) {
#ifdef UNISTD
    times(&x->setcputime);
#endif
#ifdef MSW
    FILETIME ignorethis, ignorethat;
    BOOL retval = GetProcessTimes(GetCurrentProcess(), &ignorethis, &ignorethat,
        (FILETIME *)&x->kerneltime, (FILETIME *)&x->usertime);
    if (!retval) {
        if (!x->warned) {error("cputime is apparently not supported on your platform"); return;}
        x->warned = 1;
        x->kerneltime.QuadPart = 0;
        x->usertime.QuadPart = 0;
    }
#endif
}
static void cputime_bang2(t_cputime *x) {
#ifdef UNISTD
    struct tms newcputime;
    times(&newcputime);
    float elapsedcpu = 1000 * (newcputime.tms_utime +    newcputime.tms_stime
                          - x->setcputime.tms_utime - x->setcputime.tms_stime) / HZ;
#endif
#ifdef MSW
    FILETIME ignorethis, ignorethat;
    LARGE_INTEGER usertime, kerneltime;
    BOOL retval = GetProcessTimes(GetCurrentProcess(), &ignorethis, &ignorethat, (FILETIME *)&kerneltime, (FILETIME *)&usertime);
    float elapsedcpu = retval ? 0.0001 *
            ((kerneltime.QuadPart - x->kerneltime.QuadPart) +
               (usertime.QuadPart -   x->usertime.QuadPart)) : 0;
#endif
    x->outlet->send(elapsedcpu);
}

static void realtime_bang(t_realtime *x) {x->setrealtime = sys_getrealtime();}
static void    timer_bang(t_timer *x   ) {x->settime     = clock_getsystime();}
static void realtime_bang2(t_realtime *x) {x->outlet->send((sys_getrealtime() - x->setrealtime) * 1000.);}
static void    timer_bang2(t_timer *x   ) {x->outlet->send(clock_gettimesince(x->settime));}

static void *cputime_new() {
    t_cputime *x = (t_cputime *)pd_new(cputime_class);
    outlet_new(x,gensym("float"));
    inlet_new(x,x,gensym("bang"),gensym("bang2"));
#ifdef MSW
    x->warned = 0;
#endif
    cputime_bang(x);
    return x;
}
static void *realtime_new() {
    t_realtime *x = (t_realtime *)pd_new(realtime_class);
    outlet_new(x,gensym("float"));
    inlet_new(x,x,gensym("bang"),gensym("bang2"));
    realtime_bang(x);
    return x;
}
static void *timer_new(t_floatarg f) {
    t_timer *x = (t_timer *)pd_new(timer_class);
    timer_bang(x);
    outlet_new(x, gensym("float"));
    inlet_new(x, x, gensym("bang"), gensym("bang2"));
    return x;
}
static void timer_setup() {
    realtime_class = class_new2("realtime",realtime_new,0,sizeof(t_realtime),0,"");
    cputime_class  = class_new2("cputime", cputime_new, 0,sizeof(t_cputime), 0,"");
    timer_class    = class_new2("timer",   timer_new,   0,sizeof(t_timer),   0,"F");
    class_addbang(realtime_class, realtime_bang);
    class_addbang(cputime_class,  cputime_bang);
    class_addbang(timer_class,    timer_bang);
    class_addmethod2(realtime_class,realtime_bang2,"bang2","");
    class_addmethod2(cputime_class, cputime_bang2, "bang2","");
    class_addmethod2(timer_class,   timer_bang2,   "bang2","");
}

static t_class *print_class;
struct t_print : t_object {
    t_symbol *sym;
};
static void *print_new(t_symbol *s) {
    t_print *x = (t_print *)pd_new(print_class);
    if (*s->name) x->sym = s;
    else x->sym = gensym("print");
    return x;
}
static void print_bang(t_print *x)                    {post("%s: bang", x->sym->name);}
static void print_pointer(t_print *x, t_gpointer *gp) {post("%s: (gpointer)", x->sym->name);}
static void print_float(t_print *x, t_float f)        {post("%s: %g", x->sym->name, f);}
static void print_list(t_print *x, t_symbol *s, int argc, t_atom *argv) {
    if (argc && argv->a_type != A_SYMBOL) startpost("%s:", x->sym->name);
    else startpost("%s: %s", x->sym->name, (argc>1 ? s_list : argc==1 ? s_symbol : s_bang).name);
    postatom(argc, argv);
    endpost();
}
static void print_anything(t_print *x, t_symbol *s, int argc, t_atom *argv) {
    startpost("%s: %s", x->sym->name, s->name);
    postatom(argc, argv);
    endpost();
}
static void print_setup() {
    t_class *c = print_class = class_new2("print",print_new,0,sizeof(t_print),0,"S");
    class_addbang(c, print_bang);
    class_addfloat(c, print_float);
    class_addpointer(c, print_pointer);
    class_addlist(c, print_list);
    class_addanything(c, print_anything);
}

/*---- Macro ----*/
static t_class *macro_class;
struct t_macro : t_object {
    t_symbol *sym;
    t_outlet *bangout;
};

static void *macro_new(t_symbol *s) {
    t_macro *x = (t_macro *)pd_new(macro_class);
    if (*s->name) x->sym = s;
    else x->sym = gensym("macro");
    x-> bangout = outlet_new(x, &s_bang);
    return x;
}

static void macro_bang(t_macro *x) {x->bangout->send();}

static void macro_send(t_macro *x, t_symbol *s, int argc, t_atom *argv) {
  std::ostringstream t;
  t << s->name;
  for (int i=0; i<argc; i++) {t << " " << &argv[i];}
  sys_mgui(x->dix->canvas, "macro_event_append", "Sp", t.str().data(), x);
}
static void macro_setup() {
    t_class *c = macro_class = class_new2("macro",macro_new,0,sizeof(t_macro),0,"S");
    class_addanything(c, macro_send);
    class_addmethod2(c, macro_bang, "mbang","");
}

/*---- Clipboard ----*/
static t_class *clipboard_class;
struct t_clipboard : t_object {
  t_binbuf *alist;
  t_symbol *sym;
  t_outlet *dump;
};

static void *clipboard_new(t_symbol *s) {
  t_clipboard *x = (t_clipboard *)pd_new(clipboard_class);
  if (*s->name) x->sym = s;
  else x->sym = gensym("clipboard");
  x->alist = binbuf_new();
  x->dump = outlet_new(x,&s_list);
  return x;
}

static void clipboard_bang(t_clipboard *x) {sys_mgui(x->dix->canvas, "get_clipboard", "p", x);}
static void clipboard_reply (t_clipboard *x, t_symbol *s, int argc, t_atom *argv) {x->dump->send(argc,argv);}

static void clipboard_setup() {
    t_class *c = clipboard_class = class_new2("clipboard",clipboard_new,0,sizeof(t_clipboard),0,"S");
    class_addbang(c, clipboard_bang);
    class_addmethod2(c, clipboard_reply,"clipboard_set","*");
}

/*---- Display ----*/
static t_class *display_class;
struct t_display : t_object {
  t_float height;
};

static void *display_new(t_floatarg f) {
    t_display *x = (t_display *)pd_new(display_class);
    x->height = f;
    if (!x->height) x->height = 1;
    return x;
}

static void display_height (t_display *x) {sys_mgui(x, "height=", "i", (int)x->height);}

static void display_send(t_display *x, t_symbol *s, int argc, t_atom *argv) {
  std::ostringstream t;
  t << s->name;
  for (int i=0; i<argc; i++) {t << " " << &argv[i];}
  sys_mgui(x, "dis", "S", t.str().data());
}

static void display_setup() {
    t_class *c = display_class = class_new2("display",display_new,0,sizeof(t_display),0,"F");
    class_addanything(c, display_send);
    class_addmethod2(c, display_height, "height","");
}

/*---- Any ----*/
static t_class *any_class;
struct t_any : t_object {t_binbuf *alist;};

static void *any_new(t_symbol *s,int argc, t_atom *argv) {
    t_any *x = (t_any *)pd_new(any_class);
    x->alist = binbuf_new();
    if (argc) {
      if (argv[0].a_type == A_FLOAT) {alist_list(x->alist, 0, argc, argv);}
      if (argv[0].a_type == A_SYMBOL) {alist_anything(x->alist, argv[0].a_symbol, argc-1, argv+1);} 
    }
    outlet_new(x, &s_anything);
    inlet_new(x,x->alist, 0, 0);
    return x;
}

static void any_anything(t_any *x, t_symbol *s, int argc, t_atom *argv) {
    t_atom *outv; int outc = x->alist->n+argc+1; ATOMS_ALLOCA(outv, outc);
    if ((argv[0].a_type == A_FLOAT  && s==&s_list) || s==&s_float) {
      alist_list(x->alist,0,argc,argv); x->outlet->send(argc,argv); return;
    }
    if ( argv[0].a_type == A_SYMBOL || s!=&s_list  || s!=&s_float) {
      alist_anything(x->alist,s,argc,argv); x->outlet->send(s,argc,argv);
    }
}

static void any_bang(t_any *x) {
    t_atom *outv; int outc = x->alist->n;
    ATOMS_ALLOCA(outv, outc);
    alist_toatoms(x->alist, outv);
    if (!binbuf_getnatom(x->alist)) {x->outlet->send();return;}
    if (outv[0].a_type == A_FLOAT) {x->outlet->send(outc,outv);}
    if (outv[0].a_type == A_SYMBOL) {x->outlet->send(outv[0].a_symbol,outc-1,outv+1);}
    ATOMS_FREEA(outv, outc);
}

static void any_setup() {
    post("DesireData iemlib2 [any] clone");
    t_class *c = any_class = class_new2("any",any_new,0,sizeof(t_any),0,"*");
    class_addanything(c, any_anything);
    class_addbang(c, any_bang);
}

/* MSW and OSX don't appear to have single-precision ANSI math */
#if defined(MSW) || defined(__APPLE__)
#define sinf sin
#define cosf cos
#define atanf atan
#define atan2f atan2
#define sqrtf sqrt
#define logf log
#define expf exp
#define fabsf fabs
#define powf pow
#endif

struct t_binop : t_object {
    t_float f1;
    t_float f2;
};
static void *binop_new(t_class *floatclass, t_floatarg f) {
    t_binop *x = (t_binop *)pd_new(floatclass);
    outlet_new(x, &s_float);
    floatinlet_new(x, &x->f2);
    x->f1 = 0;
    x->f2 = f;
    return x;
}

#define BINOP(NAME,EXPR) \
static t_class *NAME##_class; \
static void *NAME##_new(t_floatarg f) {return binop_new(NAME##_class, f);} \
static void NAME##_bang(t_binop *x) {float a=x->f1,b=x->f2; x->outlet->send(EXPR);} \
static void NAME##_float(t_binop *x, t_float f) {x->f1=f; NAME##_bang(x);}

BINOP(binop_plus,a+b)
BINOP(binop_minus,a-b)
BINOP(binop_times,a*b)
BINOP(binop_div,a/b)
BINOP(binop_pow, a>0?powf(a,b):0)
BINOP(binop_max, a>b?a:b)
BINOP(binop_min, a<b?a:b)
BINOP(binop_ee,a==b)
BINOP(binop_ne,a!=b)
BINOP(binop_gt,a>b)
BINOP(binop_lt,a<b)
BINOP(binop_ge,a>=b)
BINOP(binop_le,a<=b)
BINOP(binop_ba,(int)a&(int)b)
BINOP(binop_la,a&&b)
BINOP(binop_bo,(int)a|(int)b)
BINOP(binop_lo,a||b)
BINOP(binop_ls,(int)a<<(int)b)
BINOP(binop_rs,(int)a>>(int)b)
BINOP(binop_pc,(int)a % (b?(int)b:1))
static int mymod(int a, int b) {
    int n2 = (int)b;
    if (n2 < 0) n2 = -n2; else if (!n2) n2 = 1;
    int r = (int)a % n2;
    return r<0 ? r+n2 : r;
}
BINOP(binop_mymod, (float)mymod((int)a,(int)b))
static int mydiv(int a, int b) {
    if (b < 0) b=-b; else if (!b) b=1;
    if (a < 0) a -= b-1;
    return a/b;
}
BINOP(binop_mydiv, (float)mydiv((int)a,(int)b))

BINOP(binop_atan2, (a==0 && b==0 ? 0 : atan2f(a,b)));

FUNC1(sin,sinf(a))
FUNC1(cos,cosf(a))
FUNC1(tan,tanf(a))
FUNC1(atan,atanf(a))

FUNC1(sqrt,sqrtf(a))
FUNC1(log, a>0 ? logf(a) : -1000)
FUNC1(exp,expf(min(a,87.3365f)))
FUNC1(abs,fabsf(a))

static t_class *clip_class;
struct t_clip : t_object {
    float f1;
    float f2;
    float f3;
};
static void *clip_new(t_floatarg f2, t_floatarg f3) {
    t_clip *x = (t_clip *)pd_new(clip_class);
    floatinlet_new(x, &x->f2); x->f2 = f2;
    floatinlet_new(x, &x->f3); x->f3 = f3;
    outlet_new(x, &s_float);
    return x;
}
static void clip_bang( t_clip *x) {                      x->outlet->send(clip(x->f1,x->f2,x->f3));}
static void clip_float(t_clip *x, t_float f) {x->f1 = f; x->outlet->send(clip(x->f1,x->f2,x->f3));}

void arithmetic_setup() {
    t_symbol *s = gensym("operators");
#define BINOPDECL(NAME,SYM) \
    NAME##_class = class_new2(SYM,NAME##_new,0,sizeof(t_binop),0,"F"); \
    class_addbang(NAME##_class, NAME##_bang); \
    class_addfloat(NAME##_class, (t_method)NAME##_float); \
    class_sethelpsymbol(NAME##_class,s);

    BINOPDECL(binop_plus,"+")
    BINOPDECL(binop_minus,"-")
    BINOPDECL(binop_times,"*")
    BINOPDECL(binop_div,"/")
    BINOPDECL(binop_pow,"pow")
    BINOPDECL(binop_max,"max")
    BINOPDECL(binop_min,"min")

    s = gensym("otherbinops");

    BINOPDECL(binop_ee,"==")
    BINOPDECL(binop_ne,"!=")
    BINOPDECL(binop_gt,">")
    BINOPDECL(binop_lt,"<")
    BINOPDECL(binop_ge,">=")
    BINOPDECL(binop_le,"<=")

    BINOPDECL(binop_ba,"&")
    BINOPDECL(binop_la,"&&")
    BINOPDECL(binop_bo,"|")
    BINOPDECL(binop_lo,"||")
    BINOPDECL(binop_ls,"<<")
    BINOPDECL(binop_rs,">>")

    BINOPDECL(binop_pc,"%")
    BINOPDECL(binop_mymod,"mod")
    BINOPDECL(binop_mydiv,"div")
    BINOPDECL(binop_atan2,"atan2");

    s = gensym("math");

#define FUNCDECL(NAME,SYM) \
    NAME##_class = class_new2(SYM,NAME##_new,0,sizeof(t_object),0,""); \
    class_addfloat(NAME##_class, (t_method)NAME##_float); \
    class_sethelpsymbol(NAME##_class,s);

    FUNCDECL(sin,"sin");
    FUNCDECL(cos,"cos");
    FUNCDECL(tan,"tan");
    FUNCDECL(atan,"atan");
    FUNCDECL(sqrt,"sqrt");
    FUNCDECL(log,"log");
    FUNCDECL(exp,"exp");
    FUNCDECL(abs,"abs");

    clip_class = class_new2("clip",clip_new,0,sizeof(t_clip),0,"FF");
    class_addfloat(clip_class, clip_float);
    class_addbang(clip_class, clip_bang);
}

static t_class *pdint_class;    struct t_pdint    : t_object {t_float   v;};
static t_class *pdfloat_class;  struct t_pdfloat  : t_object {t_float   v;};
static t_class *pdsymbol_class; struct t_pdsymbol : t_object {t_symbol *v;};
static t_class *bang_class;     struct t_bang     : t_object {};

static void pdint_bang(t_pdint *x) {x->outlet->send(t_float(int(x->v)));}
static void pdint_float(t_pdint *x, t_float v) {x->v = v; pdint_bang(x);}

static void pdfloat_bang(t_pdfloat *x) {x->outlet->send(x->v);}
static void pdfloat_float(t_pdfloat *x, t_float v) {x->v=v; pdfloat_bang(x);}

static void pdsymbol_bang(t_pdsymbol *x) {x->outlet->send(x->v);}
static void pdsymbol_symbol(  t_pdsymbol *x, t_symbol *v                    ) {x->v=v; pdsymbol_bang(x);}
static void pdsymbol_anything(t_pdsymbol *x, t_symbol *v, int ac, t_atom *av) {x->v=v; pdsymbol_bang(x);}

/* For "list" message don't just output "list"; if empty, we want to bang the symbol and
   if it starts with a symbol, we output that. Otherwise it's not clear what we should do
   so we just go for the "anything" method.  LATER figure out if there are other places
   where empty lists aren't equivalent to "bang"???  Should Pd's message passer always check
   and call the more specific method, or should it be the object's responsibility?  Dunno... */
#if 0
static void pdsymbol_list(t_pdsymbol *x, t_symbol *s, int ac, t_atom *av) {
    if (!ac) pdsymbol_bang(x);
    else if (av->a_type == A_SYMBOL) pdsymbol_symbol(x, av->a_symbol);
    else pdsymbol_anything(x, s, ac, av);
}
#endif

static void *pdint_new(t_floatarg v) {
    t_pdint *x   = (t_pdint *)  pd_new(pdint_class  ); x->v=v; outlet_new(x,&s_float); floatinlet_new(x,&x->v);
    return x;
}
static void *pdfloat_new(t_pd *dummy, t_float v) {
    t_pdfloat *x = (t_pdfloat *)pd_new(pdfloat_class); x->v=v; outlet_new(x,&s_float); floatinlet_new(x,&x->v);
    pd_set_newest((t_pd *)x);
    return x;
}
static void *pdfloat_new2(t_floatarg f) {return pdfloat_new(0, f);}
static void *pdsymbol_new(t_pd *dummy, t_symbol *v) {
    t_pdsymbol *x = (t_pdsymbol *)pd_new(pdsymbol_class); x->v=v; outlet_new(x, &s_symbol); symbolinlet_new(x, &x->v);
    pd_set_newest((t_pd *)x);
    return x;
}
static void *bang_new(t_pd *dummy) {
    t_bang *x = (t_bang *)pd_new(bang_class);
    outlet_new(x, &s_bang);
    pd_set_newest((t_pd *)x);
    return x;
}
static void *bang_new2(t_bang f) {return bang_new(0);}
static void bang_bang(t_bang *x) {x->outlet->send();}

void misc_setup() {
    pdint_class = class_new2("int",pdint_new,0,sizeof(t_pdint),0,"F");
    class_addcreator2("i",pdint_new,"F");
    class_addbang(pdint_class, pdint_bang);
    class_addfloat(pdint_class, pdint_float);
    pdfloat_class = class_new2("float",pdfloat_new,0,sizeof(t_pdfloat),0,"F");
    class_addcreator2("f",pdfloat_new2,"F");
    class_addbang(pdfloat_class, pdfloat_bang);
    class_addfloat(pdfloat_class, pdfloat_float);
    pdsymbol_class = class_new2("symbol",pdsymbol_new,0,sizeof(t_pdsymbol),0,"S");
    class_addbang(pdsymbol_class, pdsymbol_bang);
    class_addsymbol(pdsymbol_class, pdsymbol_symbol);
    class_addanything(pdsymbol_class, pdsymbol_anything);
    t_class *c = bang_class = class_new2("bang",bang_new,0,sizeof(t_bang),0,"");
    class_addcreator2("b",bang_new2,"");
    class_addbang(c, bang_bang);
    class_addfloat(c, bang_bang);
    class_addsymbol(c, bang_bang);
    class_addlist(c, bang_bang);
    class_addanything(c, bang_bang);
}

/* -------------------- send & receive ------------------------------ */

static t_class *   send_class; struct t_send    : t_object {t_symbol *sym;};
static t_class *receive_class; struct t_receive : t_object {t_symbol *sym;};

static void send_bang(    t_send *x) {                                     if (x->sym->thing)    pd_bang(x->sym->thing);}
static void send_float(   t_send *x, t_float f) {                          if (x->sym->thing)   pd_float(x->sym->thing,f);}
static void send_symbol(  t_send *x, t_symbol *s) {                        if (x->sym->thing)  pd_symbol(x->sym->thing,s);}
static void send_pointer( t_send *x, t_gpointer *gp) {                     if (x->sym->thing) pd_pointer(x->sym->thing,gp);}
static void send_list(    t_send *x, t_symbol *s, int argc, t_atom *argv) {if (x->sym->thing)    pd_list(x->sym->thing,s,argc,argv);}
static void send_anything(t_send *x, t_symbol *s, int argc, t_atom *argv) {if (x->sym->thing)  typedmess(x->sym->thing,s,argc,argv);}

static void receive_bang(    t_receive *x) {                                     x->outlet->send();}
static void receive_float(   t_receive *x, t_float v) {                          x->outlet->send(v);}
static void receive_symbol(  t_receive *x, t_symbol *v) {                        x->outlet->send(v);}
static void receive_pointer( t_receive *x, t_gpointer *v) {                      x->outlet->send(v);}
static void receive_list(    t_receive *x, t_symbol *s, int argc, t_atom *argv) {x->outlet->send(  argc,argv);}
static void receive_anything(t_receive *x, t_symbol *s, int argc, t_atom *argv) {x->outlet->send(s,argc,argv);}
static void *receive_new(t_symbol *s) {
    t_receive *x = (t_receive *)pd_new(receive_class);
    x->sym = s;
    pd_bind(x, s);
    outlet_new(x, 0);
    return x;
}
static void receive_free(t_receive *x) {pd_unbind(x, x->sym);}

static void *send_new(t_symbol *s) {
    t_send *x = (t_send *)pd_new(send_class);
    if (!*s->name) symbolinlet_new(x, &x->sym);
    x->sym = s;
    return x;
}
static void sendreceive_setup() {
    t_class *c;
    c = send_class = class_new2("send",send_new,0,sizeof(t_send),0,"S");
    class_addcreator2("s",send_new,"S");
    class_addbang(c, send_bang);
    class_addfloat(c, send_float);
    class_addsymbol(c, send_symbol);
    class_addpointer(c, send_pointer);
    class_addlist(c, send_list);
    class_addanything(c, send_anything);
    c = receive_class = class_new2("receive",receive_new,receive_free,sizeof(t_receive),CLASS_NOINLET,"S");
    class_addcreator2("r",receive_new,"S");
    class_addbang(c, receive_bang);
    class_addfloat(c, receive_float);
    class_addsymbol(c, receive_symbol);
    class_addpointer(c, receive_pointer);
    class_addlist(c, receive_list);
    class_addanything(c, receive_anything);
}

/* -------------------------- select ------------------------------ */

static t_class *select_class;
struct t_selectelement {
    t_atom a;
    t_outlet *out;
};
struct t_select : t_object {
    t_int nelement;
    t_selectelement *vec;
    t_outlet *rejectout;
};
#define select_each(e,x) for (t_selectelement *e = x->vec;e;e=0) for (int nelement = x->nelement; nelement--; e++)

static void select_float(t_select *x, t_float f) {
    select_each(e,x) if (e->a.a_type==A_FLOAT  && e->a.a_float==f)  {e->out->send(); return;}
    x->rejectout->send(f);
}
static void select_symbol(t_select *x, t_symbol *s) {
    select_each(e,x) if (e->a.a_type==A_SYMBOL && e->a.a_symbol==s) {e->out->send(); return;}
    x->rejectout->send(s);
}
static void select_free(t_select *x) {free(x->vec);}
static void *select_new(t_symbol *s, int argc, t_atom *argv) {
    t_atom a;
    if (argc == 0) {
        argc = 1;
        SETFLOAT(&a, 0);
        argv = new t_atom[1];
    }
    t_select *x = (t_select *)pd_new(select_class);
    x->nelement = argc;
    x->vec = (t_selectelement *)getbytes(argc * sizeof(*x->vec));
    t_selectelement *e = x->vec;
    for (int n = 0; n < argc; n++, e++) {
        e->out = outlet_new(x, &s_bang);
        e->a = argv[n];
        if (e->a.a_type == A_FLOAT)
              floatinlet_new(x, &x->vec[n].a.a_float);
        else symbolinlet_new(x, &x->vec[n].a.a_symbol);
    }
    x->rejectout = outlet_new(x, &s_float);
    return x;
}
void select_setup() {
    select_class = class_new2("select",0,select_free,sizeof(t_select),0,"");
    class_addfloat(select_class, select_float);
    class_addsymbol(select_class, select_symbol);
    class_addcreator2("select",select_new,"*");
    class_addcreator2("sel",   select_new,"*");
}

/* -------------------------- route ------------------------------ */

static t_class *route_class;
struct t_routeelement {
    t_atom a;
    t_outlet *out;
};
struct t_route : t_object {
    t_int n;
    t_routeelement *vec;
    t_outlet *rejectout;
};
static void route_anything(t_route *x, t_symbol *sel, int argc, t_atom *argv) {
    t_routeelement *e = x->vec;
    post("1: sel=%s",sel->name);
    for (int n = x->n; n--; e++) if (e->a.a_type == A_SYMBOL) if (e->a.a_symbol == sel) {
        if (argc > 0 && argv[0].a_type == A_SYMBOL) e->out->send(argv[0].a_symbol,argc-1,argv+1);
        else { /* tb {: avoid 1 element lists */
	    if (argc > 1) e->out->send(argc,argv);
	    else if (argc == 0) e->out->send();
	    else e->out->send(&argv[0]);
	} /* tb } */
        return;
    }
    x->rejectout->send(sel,argc,argv);
}

#define route_eachr(E,L) for (t_routeelement *E = L->vec;E;E=0) for (int ROUTEN = x->n; ROUTEN--; E++)
static void route_list(t_route *x, t_symbol *sel, int argc, t_atom *argv) {
    if (argc && argv->a_type == A_FLOAT) {
        float f = atom_getfloat(argv);
        route_eachr(e,x) if (e->a.a_type == A_FLOAT && e->a.a_float == f) {
            if (argc > 1 && argv[1].a_type == A_SYMBOL) e->out->send(argv[1].a_symbol,argc-2,argv+2);
            else {
		argc--; argv++;
		if (argc>1) e->out->send(argc,argv); else if (argc==0) e->out->send(); else e->out->send(&argv[0]);
	    }
            return;
        } else if (e->a.a_type == A_SYMBOL && e->a.a_symbol == &s_float) {
	    e->out->send(argv[0].a_float);
	    return;
        }
    } else { /* symbol arguments */
        if (argc > 1) { /* 2 or more args: treat as "list" */
	    route_eachr(e,x) if (e->a.a_type == A_SYMBOL && e->a.a_symbol == &s_list) {
                if (argv[0].a_type==A_SYMBOL) e->out->send(argv[0].a_symbol,argc-1,argv+1);
                else e->out->send(argc,argv);
                return;
            }
        } else if (argc==0)                  {route_eachr(e,x) {if (e->a.a_symbol==&s_bang   ) {e->out->send(        ); return;}}
        } else if (argv[0].a_type==A_FLOAT)  {route_eachr(e,x) {if (e->a.a_symbol==&s_float  ) {e->out->send(&argv[0]); return;}}
        } else if (argv[0].a_type==A_SYMBOL) {route_eachr(e,x) {if (e->a.a_symbol==&s_symbol ) {e->out->send(&argv[0]); return;}}
        } else if (argv[0].a_type==A_POINTER){route_eachr(e,x) {if (e->a.a_symbol==&s_pointer) {e->out->send(&argv[0]); return;}}
        }
    }
    if (!argc) x->rejectout->send(); else x->rejectout->send(argc,argv);
}

static void route_free(t_route *x) {free(x->vec);}
static void *route_new(t_symbol *s, int argc, t_atom *argv) {
    t_route *x = (t_route *)pd_new(route_class);
    if (argc == 0) {
        t_atom a;
        argc = 1;
        SETFLOAT(&a, 0);
        argv = &a;
    }
    x->n = argc;
    x->vec = (t_routeelement *)getbytes(argc * sizeof(*x->vec));
    t_routeelement *e = x->vec;
    for (int n = 0; n < argc; n++, e++) {
        e->out = outlet_new(x, &s_list);
        e->a = argv[n];
    }
    x->rejectout = outlet_new(x, &s_list);
    return x;
}
void route_setup() {
    route_class = class_new2("route",route_new,route_free,sizeof(t_route),0,"*");
    class_addlist(route_class, route_list);
    class_addanything(route_class, route_anything);
}

static t_class *pack_class;
struct t_pack : t_object {
    t_int n;              /* number of args */
    t_atom *vec;          /* input values */
    t_atom *outvec;       /* space for output values */
};
static void *pack_new(t_symbol *s, int argc, t_atom *argv) {
    t_pack *x = (t_pack *)pd_new(pack_class);
    t_atom defarg[2], *ap, *vec, *vp;
    if (!argc) {
        argv = defarg;
        argc = 2;
        SETFLOAT(&defarg[0], 0);
        SETFLOAT(&defarg[1], 0);
    }
    x->n = argc;
    vec = x->vec = (t_atom *)getbytes(argc * sizeof(*x->vec));
    x->outvec = (t_atom *)getbytes(argc * sizeof(*x->outvec));
    vp = x->vec;
    ap = argv;
    for (int i = 0; i < argc; i++, ap++, vp++) {
        if (ap->a_type == A_FLOAT) {
            *vp = *ap;
            if (i) floatinlet_new(x, &vp->a_float);
        } else if (ap->a_type == A_SYMBOL) {
            char c = *ap->a_symbol->name;
            if      (c == 's') { SETSYMBOL(vp, &s_symbol); if (i) symbolinlet_new(x, &vp->a_symbol);}
            else if (c == 'p') { /* SETPOINTER(vp, gpointer_new()) if (i) pointerinlet_new(x, gp); */}
            else if (c == 'f') { SETFLOAT(vp, 0); if (i) floatinlet_new(x, &vp->a_float);}
            else error("pack: %s: bad type '%c'", ap->a_symbol->name, c);
        }
    }
    outlet_new(x, &s_list);
    return x;
}
static void pack_bang(t_pack *x) {
    int reentered = 0, size = x->n * sizeof (t_atom);
    t_atom *outvec;
    /* reentrancy protection.  The first time through use the pre-allocated outvec; if we're reentered we have to allocate new memory. */
    if (!x->outvec) {
        outvec = (t_atom *)t_getbytes(size);
        reentered = 1;
    } else {
        outvec = x->outvec;
        x->outvec = 0;
    }
    memcpy(outvec, x->vec, size);
    x->outlet->send(x->n,outvec);
    if (reentered) free(outvec); else x->outvec = outvec;
}

static void pack_pointer(t_pack *x, t_gpointer *gp) {
    if (x->vec->a_type == A_POINTER) {
        //gpointer_unset(x->gpointer);
        //*x->gpointer = *gp;
        //if (gp->o) gp->o->refcount++;
        pack_bang(x);
    } else error("pack_pointer: wrong type");
}
static void pack_float(t_pack *x, t_float f) {
    if (x->vec->a_type == A_FLOAT ) {x->vec->a_float = f;  pack_bang(x);} else error("pack_float: wrong type");
}
static void pack_symbol(t_pack *x, t_symbol *s) {
    if (x->vec->a_type == A_SYMBOL) {x->vec->a_symbol = s; pack_bang(x);} else error("pack_symbol: wrong type");
}
static void pack_list(t_pack *x, t_symbol *s, int ac, t_atom *av) {obj_list(x, 0, ac, av);}
static void pack_anything(t_pack *x, t_symbol *s, int ac, t_atom *av) {
    t_atom *av2 = (t_atom *)getbytes((ac + 1) * sizeof(t_atom));
    for (int i = 0; i < ac; i++) av2[i + 1] = av[i];
    SETSYMBOL(av2, s);
    obj_list(x, 0, ac+1, av2);
    free(av2);
}
static void pack_free(t_pack *x) {
    free(x->vec);
    free(x->outvec);
}
static void pack_setup() {
    t_class *c = pack_class = class_new2("pack",pack_new,pack_free,sizeof(t_pack),0,"*");
    class_addbang(c, pack_bang);
    class_addpointer(c, pack_pointer);
    class_addfloat(c, pack_float);
    class_addsymbol(c, pack_symbol);
    class_addlist(c, pack_list);
    class_addanything(c, pack_anything);
}

static t_class *unpack_class;
struct t_unpack : t_object {
    t_int n;
    t_outlet **vec;
    char *vat;
};

static t_atomtype atomtype_from_letter(char c) {
    switch (c) {
    case 'e': return A_ATOM;
    case 'f': return A_FLOAT;
    case 's': return A_SYMBOL;
    case 'p': return A_POINTER;
    default:  return A_CANT;
    }
}

static void *unpack_new(t_symbol *s, int argc, t_atom *argv) {
    t_unpack *x = (t_unpack *)pd_new(unpack_class);
    t_atom defarg[2];
    if (!argc) {
        argv = defarg;
        argc = 2;
        SETFLOAT(&defarg[0], 0);
        SETFLOAT(&defarg[1], 0);
    }
    x->n = argc;
    x->vec = (t_outlet **)getbytes(argc * sizeof(*x->vec));
    x->vat =      (char *)getbytes(argc);
    t_atom *ap=argv;
    for (int i=0; i<argc; ap++, i++) {
	x->vec[i] = outlet_new(x,0);
        switch (argv[i].a_type) {
	    case A_FLOAT: x->vat[i]=A_FLOAT; break;
	    case A_SYMBOL: {
		const char *s = atom_getstring(&argv[i]);
		if (strlen(s)<1 || (x->vat[i]=atomtype_from_letter(s[0]))==A_CANT) {error("%s: bad type", s); x->vat[i]=A_FLOAT;}
		break;
	    }
	    default: error("bad type");
	}
    }
    return x;
}
static void unpack_list(t_unpack *x, t_symbol *s, int argc, t_atom *argv) {
    if (argc > x->n) argc = x->n;
    //for (int i=argc-1; i>=0; i--) x->vec[i]->send(&argv[i]);
    for (int i=argc-1; i>=0; i--) {
	if (x->vat[i]==A_ATOM || x->vat[i]==argv[i].a_type) x->vec[i]->send(&argv[i]);
	else error("type mismatch");
    }
}
static void unpack_anything(t_unpack *x, t_symbol *s, int ac, t_atom *av) {
    t_atom *av2 = (t_atom *)getbytes((ac+1) * sizeof(t_atom));
    for (int i=0; i<ac; i++) av2[i+1] = av[i];
    SETSYMBOL(av2, s);
    unpack_list(x, 0, ac+1, av2);
    free(av2);
}
static void unpack_free(t_unpack *x) {free(x->vec); free(x->vat);}
static void unpack_setup() {
    unpack_class = class_new2("unpack",unpack_new,unpack_free,sizeof(t_unpack),0,"*");
    class_addlist(unpack_class, unpack_list);
    class_addanything(unpack_class, unpack_anything);
}

static t_class *trigger_class;
struct t_triggerout {
    int type;
    t_outlet *outlet;
};
struct t_trigger : t_object {
    t_int n;
    t_triggerout *vec;
};
static void *trigger_new(t_symbol *s, int argc, t_atom *argv) {
    t_trigger *x = (t_trigger *)pd_new(trigger_class);
    t_atom defarg[2];
    if (!argc) {
        argv = defarg;
        argc = 2;
        SETSYMBOL(&defarg[0], &s_bang);
        SETSYMBOL(&defarg[1], &s_bang);
    }
    x->n = argc;
    x->vec = (t_triggerout *)getbytes(argc * sizeof(*x->vec));
    t_triggerout *u = x->vec;
    t_atom *ap = argv;
    for (int i=0; i<argc; u++, ap++, i++) {
        t_atomtype thistype = ap->a_type;
        char c;
        if (thistype == A_SYMBOL) c = ap->a_symbol->name[0];
        else if (thistype == A_FLOAT) c = 'f';
        else c = 0;
        if      (c == 'p') u->outlet = outlet_new(x, &s_pointer);
        else if (c == 'f') u->outlet = outlet_new(x, &s_float);
        else if (c == 'b') u->outlet = outlet_new(x, &s_bang);
        else if (c == 'l') u->outlet = outlet_new(x, &s_list);
        else if (c == 's') u->outlet = outlet_new(x, &s_symbol);
        else if (c == 'a') u->outlet = outlet_new(x, &s_symbol);
        else {
            error("trigger: %s: bad type", ap->a_symbol->name);
            c='f'; u->outlet = outlet_new(x, &s_float);
        }
	u->type = c;
    }
    return x;
}
static void trigger_list(t_trigger *x, t_symbol *s, int argc, t_atom *argv) {
    t_triggerout *u = x->vec+x->n;
    for (int i = x->n; u--, i--;) {
        if      (u->type == 'f')  u->outlet->send(argc ? atom_getfloat(argv) : 0);
        else if (u->type == 'b')  u->outlet->send();
        else if (u->type == 's')  u->outlet->send(argc ? atom_getsymbol(argv) : &s_symbol);
        else if (u->type == 'p') {
            if (!argc || argv->a_type != 'p') error("unpack: bad pointer"); else u->outlet->send(argv->a_pointer);
        } else u->outlet->send(argc,argv);
    }
}
static void trigger_anything(t_trigger *x, t_symbol *s, int argc, t_atom *argv) {
    t_triggerout *u = x->vec+x->n;
    for (int i = x->n; u--, i--;) {
        if      (u->type == 'b') u->outlet->send();
        else if (u->type == 'a') u->outlet->send(s,argc,argv);
        else error("trigger: can only convert 's' to 'b' or 'a'; got %s", s->name);
    }
}
static void trigger_bang(t_trigger *x)                                                    {trigger_list(x,0,0,0  );}
static void trigger_pointer(t_trigger *x, t_gpointer *gp) {t_atom at; SETPOINTER(&at, gp); trigger_list(x,0,1,&at);}
static void trigger_float(t_trigger *x, t_float f) {       t_atom at; SETFLOAT(&at, f);    trigger_list(x,0,1,&at);}
static void trigger_symbol(t_trigger *x, t_symbol *s) {    t_atom at; SETSYMBOL(&at, s);   trigger_list(x,0,1,&at);}
static void trigger_free(t_trigger *x) {free(x->vec);}
static void trigger_setup() {
    t_class *c = trigger_class = class_new2("trigger",trigger_new,trigger_free,sizeof(t_trigger),0,"*");
    class_addcreator2("t",trigger_new,"*");
    class_addlist(c, trigger_list);
    class_addbang(c, trigger_bang);
    class_addpointer(c, trigger_pointer);
    class_addfloat(c, trigger_float);
    class_addsymbol(c, trigger_symbol);
    class_addanything(c, trigger_anything);
}

static t_class *spigot_class;
struct t_spigot : t_object {float state;};
static void *spigot_new(t_floatarg f) {
    t_spigot *x = (t_spigot *)pd_new(spigot_class);
    floatinlet_new(x, &x->state);
    outlet_new(x, 0);
    x->state = f;
    return x;
}
static void spigot_bang(    t_spigot *x) {                                     if (x->state) x->outlet->send();}
static void spigot_pointer( t_spigot *x, t_gpointer *v) {                      if (x->state) x->outlet->send(v);}
static void spigot_float(   t_spigot *x, t_float v) {                          if (x->state) x->outlet->send(v);}
static void spigot_symbol(  t_spigot *x, t_symbol *v) {                        if (x->state) x->outlet->send(v);}
static void spigot_list(    t_spigot *x, t_symbol *s, int argc, t_atom *argv) {if (x->state) x->outlet->send(  argc,argv);}
static void spigot_anything(t_spigot *x, t_symbol *s, int argc, t_atom *argv) {if (x->state) x->outlet->send(s,argc,argv);}
static void spigot_setup() {
    t_class *c = spigot_class = class_new2("spigot",spigot_new,0,sizeof(t_spigot),0,"F");
    class_addbang(c, spigot_bang);
    class_addpointer(c, spigot_pointer);
    class_addfloat(c, spigot_float);
    class_addsymbol(c, spigot_symbol);
    class_addlist(c, spigot_list);
    class_addanything(c, spigot_anything);
}

static t_class *moses_class;
struct t_moses : t_object {float y;};
static void *moses_new(t_floatarg f) {
    t_moses *x = (t_moses *)pd_new(moses_class);
    floatinlet_new(x, &x->y);
    outlet_new(x, &s_float);
    outlet_new(x, &s_float);
    x->y = f;
    return x;
}
static void moses_float(t_moses *x, t_float v) {x->out(v>=x->y)->send(v);}
static void moses_setup() {
    moses_class = class_new2("moses",moses_new,0,sizeof(t_moses),0,"F");
    class_addfloat(moses_class, moses_float);
}
static t_class *until_class;
struct t_until : t_object {
    int run;
    int count;
};
static void *until_new() {
    t_until *x = (t_until *)pd_new(until_class);
    inlet_new(x, x, gensym("bang"), gensym("bang2"));
    outlet_new(x, &s_bang);
    x->run = 0;
    return x;
}
static void until_bang(t_until *x) {            x->run=1; x->count=-1;     while (x->run && x->count) {x->count--; x->outlet->send();}}
static void until_float(t_until *x, t_float f) {x->run=1; x->count=(int)f; while (x->run && x->count) {x->count--; x->outlet->send();}}
static void until_bang2(t_until *x) {x->run = 0;}
static void until_setup() {
    until_class = class_new2("until",until_new,0,sizeof(t_until),0,"");
    class_addbang(until_class, until_bang);
    class_addfloat(until_class, until_float);
    class_addmethod2(until_class, until_bang2,"bang2","");
}

static t_class *makefilename_class;
struct t_makefilename : t_object {t_symbol *format;};
static void *makefilename_new(t_symbol *s) {
    t_makefilename *x = (t_makefilename *)pd_new(makefilename_class);
    if (!s->name) s = gensym("file.%d");
    outlet_new(x, &s_symbol);
    x->format = s;
    return x;
}

/* doesn't do any typechecking or even counting the % signs properly */
static void makefilename_float(t_makefilename *x, t_floatarg f) {x->outlet->send(symprintf(x->format->name,(int)f));}
static void makefilename_symbol(t_makefilename *x, t_symbol *s) {x->outlet->send(symprintf(x->format->name,s->name));}
static void makefilename_set(t_makefilename *x, t_symbol *s) {x->format = s;}

static void makefilename_setup() {
    t_class *c = makefilename_class = class_new2("makefilename",makefilename_new,0,sizeof(t_makefilename),0,"S");
    class_addfloat(c, makefilename_float);
    class_addsymbol(c, makefilename_symbol);
    class_addmethod2(c, makefilename_set, "set","s");
}

static t_class *swap_class;
struct t_swap : t_object {
    t_float f1;
    t_float f2;
};
static void *swap_new(t_floatarg f) {
    t_swap *x = (t_swap *)pd_new(swap_class);
    x->f2 = f;
    x->f1 = 0;
    outlet_new(x, &s_float);
    outlet_new(x, &s_float);
    floatinlet_new(x, &x->f2);
    return x;
}
static void swap_bang(t_swap *x) {
    x->out(1)->send(x->f1);
    x->out(0)->send(x->f2);
}
static void swap_float(t_swap *x, t_float f) {
    x->f1 = f;
    swap_bang(x);
}
void swap_setup() {
    swap_class = class_new2("swap",swap_new,0,sizeof(t_swap),0,"F");
    class_addcreator2("fswap",swap_new,"F");
    class_addbang(swap_class, swap_bang);
    class_addfloat(swap_class, swap_float);
}

static t_class *change_class;
struct t_change : t_object {t_float f;};
static void *change_new(t_floatarg f) {
    t_change *x = (t_change *)pd_new(change_class);
    x->f = f;
    outlet_new(x, &s_float);
    return x;
}
static void change_bang(t_change *x) {x->outlet->send(x->f);}
static void change_float(t_change *x, t_float f) {
    if (f != x->f) {
        x->f = f;
        x->outlet->send(x->f);
    }
}
static void change_set(t_change *x, t_float f) {x->f = f;}
void change_setup() {
    change_class = class_new2("change",change_new,0,sizeof(t_change),0,"F");
    class_addbang(change_class, change_bang);
    class_addfloat(change_class, change_float);
    class_addmethod2(change_class, change_set, "set","F");
}

static t_class *value_class, *vcommon_class;
struct t_vcommon : t_pd {
    int c_refcount;
    t_float f;
};
struct t_value : t_object {
    t_symbol *sym;
    t_float *floatstar;
};

/* get a pointer to a named floating-point variable.  The variable
   belongs to a "vcommon" object, which is created if necessary. */
t_float *value_get(t_symbol *s) {
    t_vcommon *c = (t_vcommon *)pd_findbyclass(s, vcommon_class);
    if (!c) {
        c = (t_vcommon *)pd_new(vcommon_class);
        c->f = 0;
        c->c_refcount = 0;
        pd_bind(c,s);
    }
    c->c_refcount++;
    return &c->f;
}
/* release a variable.  This only frees the "vcommon" resource when the last interested party releases it. */
void value_release(t_symbol *s) {
    t_vcommon *c = (t_vcommon *)pd_findbyclass(s, vcommon_class);
    if (c) {
        if (!--c->c_refcount) {
            pd_unbind(c,s);
            pd_free(c);
        }
    } else bug("value_release");
}
int value_getfloat(t_symbol *s, t_float *f) {t_vcommon *c=(t_vcommon *)pd_findbyclass(s,vcommon_class); if (!c) return 1; *f=c->f;return 0;}
int value_setfloat(t_symbol *s, t_float  f) {t_vcommon *c=(t_vcommon *)pd_findbyclass(s,vcommon_class); if (!c) return 1; c->f=f; return 0;}
static void *value_new(t_symbol *s) {
    t_value *x = (t_value *)pd_new(value_class);
    x->sym = s;
    x->floatstar = value_get(s);
    outlet_new(x, &s_float);
    return x;
}
static void value_bang(t_value *x) {x->outlet->send(*x->floatstar);}
static void value_float(t_value *x, t_float f) {*x->floatstar = f;}
static void value_ff(t_value *x) {value_release(x->sym);}
static void value_setup() {
    value_class = class_new2("value",value_new,value_ff,sizeof(t_value),0,"S");
    class_addcreator2("v",value_new,"S");
    class_addbang(value_class, value_bang);
    class_addfloat(value_class, value_float);
    vcommon_class = class_new2("value",0,0,sizeof(t_vcommon),CLASS_PD,"");
}

/* MIDI. */

void outmidi_noteon(int portno, int channel, int pitch, int velo);
void outmidi_controlchange(int portno, int channel, int ctlno, int value);
void outmidi_programchange(int portno, int channel, int value);
void outmidi_pitchbend(int portno, int channel, int value);
void outmidi_aftertouch(int portno, int channel, int value);
void outmidi_polyaftertouch(int portno, int channel, int pitch, int value);
void outmidi_mclk(int portno);

static t_symbol *midiin_sym,   *sysexin_sym,   *notein_sym,   *ctlin_sym;
static  t_class *midiin_class, *sysexin_class, *notein_class, *ctlin_class;
struct t_midiin : t_object {};
struct t_notein : t_object {t_float ch;};
struct t_ctlin  : t_object {t_float ch; t_float ctlno;};

static void midiin_list(t_midiin *x, t_symbol *s, int ac, t_atom *av) {
    x->out(1)->send(atom_getfloatarg(1,ac,av) + 1);
    x->out(0)->send(atom_getfloatarg(0,ac,av));
}
void inmidi_byte(int portno, int byte) {
    if ( midiin_sym->thing) {t_atom at[2]; SETFLOAT(at, byte); SETFLOAT(at+1, portno+1); pd_list( midiin_sym->thing, 0, 2, at);}
}
void inmidi_sysex(int portno, int byte) {
    if (sysexin_sym->thing) {t_atom at[2]; SETFLOAT(at, byte); SETFLOAT(at+1, portno+1); pd_list(sysexin_sym->thing, 0, 2, at);}
}

static void notein_list(t_notein *x, t_symbol *s, int argc, t_atom *argv) {
    float pitch   = atom_getfloatarg(0, argc, argv);
    float velo    = atom_getfloatarg(1, argc, argv);
    float channel = atom_getfloatarg(2, argc, argv);
    if (x->ch) {if (channel != x->ch) return;} else x->out(2)->send(channel);
    x->out(1)->send(velo);
    x->out(0)->send(pitch);
}

void inmidi_noteon(int portno, int channel, int pitch, int velo) {
    if (notein_sym->thing) {
        t_atom at[3];
        SETFLOAT(at, pitch);
        SETFLOAT(at+1, velo);
        SETFLOAT(at+2, (channel + (portno << 4) + 1));
        pd_list(notein_sym->thing, &s_list, 3, at);
    }
}

static void *midiin_new() {
    t_midiin *x = (t_midiin *)pd_new(midiin_class);
    outlet_new(x, &s_float);
    outlet_new(x, &s_float);
    pd_bind(x, midiin_sym);
    return x;
}
static void *sysexin_new() {
    t_midiin *x = (t_midiin *)pd_new(sysexin_class);
    outlet_new(x, &s_float);
    outlet_new(x, &s_float);
    pd_bind(x, sysexin_sym);
    return x;
}
static void *notein_new(t_floatarg f) {
    t_notein *x = (t_notein *)pd_new(notein_class);
    x->ch = f;
    outlet_new(x, &s_float);
    outlet_new(x, &s_float);
    if (f == 0) outlet_new(x, &s_float);
    pd_bind(x, notein_sym);
    return x;
}
static void *ctlin_new(t_symbol *s, int argc, t_atom *argv) {
    t_ctlin *x = (t_ctlin *)pd_new(ctlin_class);
    int ctlno = (int)(argc ? atom_getfloatarg(0, argc, argv) : -1);
    int channel = (int)atom_getfloatarg(1, argc, argv);
    x->ch = channel;
    x->ctlno = ctlno;
    outlet_new(x, &s_float);
    if (!channel) {
        if (x->ctlno < 0) outlet_new(x, &s_float);
        outlet_new(x, &s_float);
    }
    pd_bind(x, ctlin_sym);
    return x;
}
static void ctlin_list(t_ctlin *x, t_symbol *s, int argc, t_atom *argv) {
    t_float ctlnumber = atom_getfloatarg(0, argc, argv);
    t_float value     = atom_getfloatarg(1, argc, argv);
    t_float channel   = atom_getfloatarg(2, argc, argv);
    if (x->ctlno >= 0 && x->ctlno != ctlnumber) return;
    if (x->ch > 0  && x->ch != channel) return;
    if (x->ch == 0)   x->out(2)->send(channel);
    if (x->ctlno < 0) x->out(1)->send(ctlnumber);
    x->out(0)->send(value);
}

static void  midiin_free(t_midiin *x) {pd_unbind(x,  midiin_sym);}
static void sysexin_free(t_midiin *x) {pd_unbind(x, sysexin_sym);}
static void  notein_free(t_notein *x) {pd_unbind(x,  notein_sym);}
static void   ctlin_free(t_ctlin *x)  {pd_unbind(x,   ctlin_sym);}

void inmidi_controlchange(int portno, int channel, int ctlnumber, int value) {
    if (ctlin_sym->thing) {
        t_atom at[3];
        SETFLOAT(at, ctlnumber);
        SETFLOAT(at+1, value);
        SETFLOAT(at+2, (channel + (portno << 4) + 1));
        pd_list(ctlin_sym->thing, &s_list, 3, at);
    }
}

struct t_midi2 : t_object {t_float ch;};
static void *midi2_new(t_class *cl, t_floatarg ch) {
    t_midi2 *x = (t_midi2 *)pd_new(cl);
    x->ch = ch;
    outlet_new(x, &s_float);
    if (!ch) outlet_new(x, &s_float);
    return x;
}
static void midi2_list(t_midi2 *x, t_symbol *s, int argc, t_atom *argv) {
    float value   = atom_getfloatarg(0, argc, argv);
    float channel = atom_getfloatarg(1, argc, argv);
    if (x->ch) {if (channel != x->ch) return;} else x->out(1)->send(channel);
    x->out(0)->send(value);
}
static t_symbol *pgmin_sym, *bendin_sym, *touchin_sym;
static t_class *pgmin_class, *bendin_class, *touchin_class;
struct   t_pgmin : t_midi2 {};
struct  t_bendin : t_midi2 {};
struct t_touchin : t_midi2 {};
static void *pgmin_new(t_floatarg f) {  t_pgmin *x   = (t_pgmin *)  midi2_new(pgmin_class  ,f);pd_bind(x,  pgmin_sym);return x;}
static void *bendin_new(t_floatarg f) { t_bendin *x  = (t_bendin *) midi2_new(bendin_class ,f);pd_bind(x, bendin_sym);return x;}
static void *touchin_new(t_floatarg f) {t_touchin *x = (t_touchin *)midi2_new(touchin_class,f);pd_bind(x,touchin_sym);return x;}
static void   pgmin_free(  t_pgmin *x) {pd_unbind(x,   pgmin_sym);}
static void  bendin_free( t_bendin *x) {pd_unbind(x,  bendin_sym);}
static void touchin_free(t_touchin *x) {pd_unbind(x, touchin_sym);}
void inmidi_programchange(int portno, int channel, int value) {
    if (pgmin_sym->thing) {
        t_atom at[2]; SETFLOAT(at,value+1); SETFLOAT(at+1, channel+(portno<<4)+1); pd_list(  pgmin_sym->thing, &s_list, 2, at);
    }
}
void inmidi_pitchbend(int portno, int channel, int value) {
    if (bendin_sym->thing) {
        t_atom at[2]; SETFLOAT(at,value);   SETFLOAT(at+1, channel+(portno<<4)+1); pd_list( bendin_sym->thing, &s_list, 2, at);
    }
}
void inmidi_aftertouch(int portno, int channel, int value) {
    if (touchin_sym->thing) {
        t_atom at[2]; SETFLOAT(at,value);   SETFLOAT(at+1, channel+(portno<<4)+1); pd_list(touchin_sym->thing, &s_list, 2, at);
    }
}

/* ----------------------- polytouchin ------------------------- */
static t_symbol *polytouchin_sym;
static t_class *polytouchin_class;
struct t_polytouchin : t_object {
    t_float ch;
};
static void *polytouchin_new(t_floatarg f) {
    t_polytouchin *x = (t_polytouchin *)pd_new(polytouchin_class);
    x->ch = f;
    outlet_new(x, &s_float);
    outlet_new(x, &s_float);
    if (f == 0) outlet_new(x, &s_float);
    pd_bind(x, polytouchin_sym);
    return x;
}
static void polytouchin_list(t_polytouchin *x, t_symbol *s, int argc, t_atom *argv) {
    t_float channel = atom_getfloatarg(2, argc, argv);
    if (x->ch) {if (channel != x->ch) return;} else x->out(2)->send(channel);
    x->out(1)->send(atom_getfloatarg(0, argc, argv)); /*pitch*/
    x->out(0)->send(atom_getfloatarg(1, argc, argv)); /*value*/
}
static void polytouchin_free(t_polytouchin *x) {pd_unbind(x, polytouchin_sym);}
static void polytouchin_setup() {
    polytouchin_class = class_new2("polytouchin",polytouchin_new,polytouchin_free,
	sizeof(t_polytouchin), CLASS_NOINLET,"F");
    class_addlist(polytouchin_class, polytouchin_list);
    class_sethelpsymbol(polytouchin_class, gensym("midi"));
    polytouchin_sym = gensym("#polytouchin");
}
void inmidi_polyaftertouch(int portno, int channel, int pitch, int value) {
    if (polytouchin_sym->thing) {
        t_atom at[3];
        SETFLOAT(at, pitch);
        SETFLOAT(at+1, value);
        SETFLOAT(at+2, (channel + (portno << 4) + 1));
        pd_list(polytouchin_sym->thing, &s_list, 3, at);
    }
}

/*----------------------- midiclkin--(midi F8 message )---------------------*/
static t_symbol *midiclkin_sym;
static t_class *midiclkin_class;
struct t_midiclkin : t_object {};
static void *midiclkin_new(t_floatarg f) {
    t_midiclkin *x = (t_midiclkin *)pd_new(midiclkin_class);
    outlet_new(x, &s_float);
    outlet_new(x, &s_float);
    pd_bind(x, midiclkin_sym);
    return x;
}
static void midiclkin_list(t_midiclkin *x, t_symbol *s, int argc, t_atom *argv) {
    x->out(1)->send(atom_getfloatarg(1, argc, argv)); /*count*/
    x->out(0)->send(atom_getfloatarg(0, argc, argv)); /*value*/
}
static void midiclkin_free(t_midiclkin *x) {pd_unbind(x, midiclkin_sym);}
static void midiclkin_setup() {
    midiclkin_class = class_new2("midiclkin",midiclkin_new,midiclkin_free,sizeof(t_midiclkin),CLASS_NOINLET,"F");
    class_addlist(midiclkin_class, midiclkin_list);
        class_sethelpsymbol(midiclkin_class, gensym("midi"));
    midiclkin_sym = gensym("#midiclkin");
}
void inmidi_clk(double timing) {
    static float prev = 0;
    static float count = 0;
    if (midiclkin_sym->thing) {
        t_atom at[2];
        float diff = timing - prev;
        count++;
        /* 24 count per quarter note */
        if (count == 3) {SETFLOAT(at, 1); count = 0;} else SETFLOAT(at, 0);
        SETFLOAT(at+1, diff);
        pd_list(midiclkin_sym->thing, &s_list, 2, at);
        prev = timing;
    }
}

/*----------midirealtimein (midi FA,FB,FC,FF message)-----------------*/
static t_symbol *midirealtimein_sym;
static t_class *midirealtimein_class;
struct t_midirealtimein : t_object {};
static void *midirealtimein_new() {
    t_midirealtimein *x = (t_midirealtimein *)pd_new(midirealtimein_class);
    outlet_new(x, &s_float);
    outlet_new(x, &s_float);
    pd_bind(x, midirealtimein_sym);
    return x;
}
static void midirealtimein_list(t_midirealtimein *x, t_symbol *s, int argc, t_atom *argv) {
    x->out(1)->send(atom_getfloatarg(0, argc, argv)); /*portno*/
    x->out(0)->send(atom_getfloatarg(1, argc, argv)); /*byte*/
}
static void midirealtimein_free(t_midirealtimein *x) {pd_unbind(x, midirealtimein_sym);}
static void midirealtimein_setup() {
    midirealtimein_class = class_new2("midirealtimein",midirealtimein_new,midirealtimein_free,
	sizeof(t_midirealtimein),CLASS_NOINLET,"F");
    class_addlist(midirealtimein_class, midirealtimein_list);
        class_sethelpsymbol(midirealtimein_class, gensym("midi"));
    midirealtimein_sym = gensym("#midirealtimein");
}
void inmidi_realtimein(int portno, int SysMsg) {
    if (midirealtimein_sym->thing) {
        t_atom at[2];
        SETFLOAT(at, portno);
        SETFLOAT(at+1, SysMsg);
        pd_list(midirealtimein_sym->thing, &s_list, 2, at);
    }
}

void outmidi_byte(int portno, int byte);
static t_class *midiout_class; struct t_midiout : t_object {t_float portno;};
static t_class *noteout_class; struct t_noteout : t_object {t_float v; t_float ch;};
static t_class *ctlout_class;  struct t_ctlout  : t_object {t_float v; t_float ch;};

static void *noteout_new(t_floatarg channel) {
    t_noteout *x = (t_noteout *)pd_new(noteout_class);
    x->v = 0;
    x->ch = channel<1?channel:1;
    floatinlet_new(x, &x->v);
    floatinlet_new(x, &x->ch);
    return x;
}
static void *ctlout_new(t_floatarg v, t_floatarg channel) {
    t_ctlout *x = (t_ctlout *)pd_new(ctlout_class);
    x->v = v;
    x->ch = channel<1?channel:1;
    floatinlet_new(x, &x->v);
    floatinlet_new(x, &x->ch);
    return x;
}
static void *midiout_new(t_floatarg portno) {
    t_midiout *x = (t_midiout *)pd_new(midiout_class);
    if (portno <= 0) portno = 1;
    x->portno = portno;
    floatinlet_new(x, &x->portno);
#ifdef __irix__
    post("midiout: unimplemented in IRIX");
#endif
    return x;
}

static void midiout_float(t_midiout *x, t_floatarg f){
    outmidi_byte((int)x->portno-1, (int)f);
}
static void noteout_float(t_noteout *x, t_float f) {
    int binchan = max(0,(int)x->ch-1);
    outmidi_noteon((binchan >> 4), (binchan & 15), (int)f, (int)x->v);
}
static void ctlout_float(t_ctlout *x, t_float f) {
    int binchan = (int)x->ch - 1;
    if (binchan < 0) binchan = 0;
    outmidi_controlchange((binchan >> 4), (binchan & 15), (int)x->v, (int)f);
}

static t_class *pgmout_class, *bendout_class, *touchout_class;
struct t_mido2 : t_object {t_float ch;};
struct t_pgmout   : t_mido2 {};
struct t_bendout  : t_mido2 {};
struct t_touchout : t_mido2 {};
static void *mido2_new(t_class *cl, t_floatarg channel) {
    t_pgmout *x = (t_pgmout *)pd_new(cl);
    x->ch = channel<1?channel:1; floatinlet_new(x, &x->ch); return x;
}
static void *pgmout_new(  t_floatarg ch) {return mido2_new(pgmout_class,ch);}
static void *bendout_new( t_floatarg ch) {return mido2_new(bendout_class,ch);}
static void *touchout_new(t_floatarg ch) {return mido2_new(touchout_class,ch);}
static void pgmout_float(t_pgmout *x, t_floatarg f) {
    int binchan = max(0,(int)x->ch-1);
    outmidi_programchange(binchan>>4, binchan&15, min(127,max(0,(int)f-1)));
}
static void bendout_float(t_bendout *x, t_float f) {
    int binchan = max(0,(int)x->ch-1);
    outmidi_pitchbend(binchan>>4, binchan&15, (int)f+8192);
}
static void touchout_float(t_touchout *x, t_float f) {
    int binchan = max(0,(int)x->ch-1);
    outmidi_aftertouch(binchan>>4, binchan&15, (int)f);
}

static t_class *polytouchout_class;
struct t_polytouchout : t_object {
    t_float ch;
    t_float pitch;
};
static void *polytouchout_new(t_floatarg channel) {
    t_polytouchout *x = (t_polytouchout *)pd_new(polytouchout_class);
    x->ch = channel<1?channel:1;
    x->pitch = 0;
    floatinlet_new(x, &x->pitch);
    floatinlet_new(x, &x->ch);
    return x;
}
static void polytouchout_float(t_polytouchout *x, t_float n) {
    int binchan = max(0,(int)x->ch-1);
    outmidi_polyaftertouch((binchan >> 4), (binchan & 15), (int)x->pitch, (int)n);
}
static void polytouchout_setup() {
    polytouchout_class = class_new2("polytouchout",polytouchout_new,0,sizeof(t_polytouchout),0,"F");
    class_addfloat(polytouchout_class, polytouchout_float);
    class_sethelpsymbol(polytouchout_class, gensym("midi"));
}

static t_class *makenote_class;
struct t_hang {
    t_clock *clock;
    t_hang *next;
    t_float pitch;
    struct t_makenote *owner;
};
struct t_makenote : t_object {
    t_float velo;
    t_float dur;
    t_hang *hang;
};
static void *makenote_new(t_floatarg velo, t_floatarg dur) {
    t_makenote *x = (t_makenote *)pd_new(makenote_class);
    x->velo = velo;
    x->dur = dur;
    floatinlet_new(x, &x->velo);
    floatinlet_new(x, &x->dur);
    outlet_new(x, &s_float);
    outlet_new(x, &s_float);
    x->hang = 0;
    return x;
}
static void makenote_tick(t_hang *hang) {
    t_makenote *x = hang->owner;
    t_hang *h2, *h3;
    x->out(1)->send(0.);
    x->out(0)->send(hang->pitch);
    if (x->hang == hang) x->hang = hang->next;
    else for (h2 = x->hang; (h3 = h2->next); h2 = h3) {
        if (h3 == hang) {
            h2->next = h3->next;
            break;
        }
    }
    clock_free(hang->clock);
    free(hang);
}
static void makenote_float(t_makenote *x, t_float f) {
    if (!x->velo) return;
    x->out(1)->send(x->velo);
    x->out(0)->send(f);
    t_hang *hang = (t_hang *)getbytes(sizeof *hang);
    hang->next = x->hang;
    x->hang = hang;
    hang->pitch = f;
    hang->owner = x;
    hang->clock = clock_new(hang, (t_method)makenote_tick);
    clock_delay(hang->clock, (x->dur >= 0 ? x->dur : 0));
}
static void makenote_stop(t_makenote *x) {
    t_hang *hang;
    while ((hang = x->hang)) {
        x->out(1)->send(0.);
        x->out(0)->send(hang->pitch);
        x->hang = hang->next;
        clock_free(hang->clock);
        free(hang);
    }
}
static void makenote_clear(t_makenote *x) {
    t_hang *hang;
    while ((hang = x->hang)) {
        x->hang = hang->next;
        clock_free(hang->clock);
        free(hang);
    }
}
static void makenote_setup() {
    makenote_class = class_new2("makenote",makenote_new,makenote_clear,sizeof(t_makenote),0,"FF");
    class_addfloat(makenote_class, makenote_float);
    class_addmethod2(makenote_class, makenote_stop, "stop","");
    class_addmethod2(makenote_class, makenote_clear,"clear","");
}

static t_class *stripnote_class;
struct t_stripnote : t_object {
    t_float velo;
};
static void *stripnote_new() {
    t_stripnote *x = (t_stripnote *)pd_new(stripnote_class);
    floatinlet_new(x, &x->velo);
    outlet_new(x, &s_float);
    outlet_new(x, &s_float);
    return x;
}
static void stripnote_float(t_stripnote *x, t_float f) {
    if (!x->velo) return;
    x->out(1)->send(x->velo);
    x->out(0)->send(f);
}
static void stripnote_setup() {
    stripnote_class = class_new2("stripnote",stripnote_new,0,sizeof(t_stripnote),0,"");
    class_addfloat(stripnote_class, stripnote_float);
}

static t_class *poly_class;
struct t_voice {
    float pitch;
    int used;
    unsigned long serial;
};
struct t_poly : t_object {
    int n;
    t_voice *vec;
    float vel;
    unsigned long serial;
    int steal;
};
static void *poly_new(float fnvoice, float fsteal) {
    int i, n = (int)fnvoice;
    t_poly *x = (t_poly *)pd_new(poly_class);
    t_voice *v;
    if (n < 1) n = 1;
    x->n = n;
    x->vec = (t_voice *)getbytes(n * sizeof(*x->vec));
    for (v = x->vec, i = n; i--; v++) v->pitch = v->used = v->serial = 0;
    x->vel = 0;
    x->steal = (fsteal != 0);
    floatinlet_new(x, &x->vel);
    outlet_new(x, &s_float);
    outlet_new(x, &s_float);
    outlet_new(x, &s_float);
    x->serial = 0;
    return x;
}
static void poly_float(t_poly *x, t_float f) {
    int i;
    t_voice *v;
    t_voice *firston, *firstoff;
    unsigned int serialon, serialoff, onindex = 0, offindex = 0;
    if (x->vel > 0) {
        /* note on.  Look for a vacant voice */
        for (v=x->vec, i=0, firston=firstoff=0, serialon=serialoff=0xffffffff; i<x->n; v++, i++) {
            if (v->used && v->serial < serialon)
                    firston = v, serialon = v->serial, onindex = i;
            else if (!v->used && v->serial < serialoff)
                    firstoff = v, serialoff = v->serial, offindex = i;
        }
        if (firstoff) {
            x->out(2)->send(x->vel);
            x->out(1)->send(firstoff->pitch = f);
            x->out(0)->send(offindex+1);
            firstoff->used = 1;
            firstoff->serial = x->serial++;
        }
        /* if none, steal one */
        else if (firston && x->steal) {
            x->out(2)->send(0.);     x->out(1)->send(firston->pitch    ); x->out(0)->send(onindex+1);
            x->out(2)->send(x->vel); x->out(1)->send(firston->pitch = f); x->out(0)->send(onindex+1);
            firston->serial = x->serial++;
        }
    } else {    /* note off. Turn off oldest match */
        for (v = x->vec, i = 0, firston = 0, serialon = 0xffffffff; i < x->n; v++, i++)
                if (v->used && v->pitch == f && v->serial < serialon)
                    firston = v, serialon = v->serial, onindex = i;
        if (firston) {
            firston->used = 0;
            firston->serial = x->serial++;
            x->out(2)->send(0.);
            x->out(1)->send(firston->pitch);
            x->out(0)->send(onindex+1);
        }
    }
}
static void poly_stop(t_poly *x) {
    t_voice *v = x->vec;
    for (int i = 0; i < x->n; i++, v++) if (v->used) {
        x->out(2)->send(0.);
        x->out(1)->send(v->pitch);
        x->out(0)->send(i+1);
        v->used = 0;
        v->serial = x->serial++;
    }
}
static void poly_clear(t_poly *x) {
    t_voice *v = x->vec;
    for (int i = x->n; i--; v++) v->used = v->serial = 0;
}
static void poly_free(t_poly *x) {free(x->vec);}
static void poly_setup() {
    poly_class = class_new2("poly",poly_new,poly_free,sizeof(t_poly),0,"FF");
    class_addfloat(poly_class, poly_float);
    class_addmethod2(poly_class, poly_stop, "stop","");
    class_addmethod2(poly_class, poly_clear, "clear","");
}

static t_class *bag_class;
struct t_bagelem {
    struct t_bagelem *next;
    t_float value;
};
struct t_bag : t_object {
    t_float velo;
    t_bagelem *first;
};
static void *bag_new() {
    t_bag *x = (t_bag *)pd_new(bag_class);
    x->velo = 0;
    floatinlet_new(x, &x->velo);
    outlet_new(x, &s_float);
    x->first = 0;
    return x;
}
static void bag_float(t_bag *x, t_float f) {
    t_bagelem *bagelem, *e2, *e3;
    if (x->velo != 0) {
        bagelem = (t_bagelem *)getbytes(sizeof *bagelem);
        bagelem->next = 0;
        bagelem->value = f;
        if (!x->first) x->first = bagelem;
        else {    /* LATER replace with a faster algorithm */
            for (e2 = x->first; (e3 = e2->next); e2 = e3) {}
            e2->next = bagelem;
        }
    } else {
        if (!x->first) return;
        if (x->first->value == f) {
            bagelem = x->first;
            x->first = x->first->next;
            free(bagelem);
            return;
        }
        for (e2 = x->first; (e3 = e2->next); e2 = e3) if (e3->value == f) {
            e2->next = e3->next;
            free(e3);
            return;
        }
    }
}
static void bag_flush(t_bag *x) {
    t_bagelem *bagelem;
    while ((bagelem = x->first)) {
        x->outlet->send(bagelem->value);
        x->first = bagelem->next;
        free(bagelem);
    }
}
static void bag_clear(t_bag *x) {
    t_bagelem *bagelem;
    while ((bagelem = x->first)) {
        x->first = bagelem->next;
        free(bagelem);
    }
}
static void bag_setup() {
    bag_class = class_new2("bag",bag_new,bag_clear,sizeof(t_bag),0,"");
    class_addfloat(bag_class, bag_float);
    class_addmethod2(bag_class,bag_flush,"flush","");
    class_addmethod2(bag_class,bag_clear,"clear","");
}
void midi_setup() {
    midiin_class  = class_new2( "midiin", midiin_new, midiin_free,sizeof(t_midiin),CLASS_NOINLET,"F");
    sysexin_class = class_new2("sysexin",sysexin_new,sysexin_free,sizeof(t_midiin),CLASS_NOINLET,"F");
    notein_class  = class_new2( "notein", notein_new, notein_free,sizeof(t_notein),CLASS_NOINLET,"F");
    ctlin_class   = class_new2(  "ctlin",  ctlin_new,  ctlin_free,sizeof( t_ctlin),CLASS_NOINLET,"*");
    class_addlist( midiin_class, midiin_list);  midiin_sym = gensym( "#midiin"); class_sethelpsymbol( midiin_class, gensym("midi"));
    class_addlist(sysexin_class, midiin_list); sysexin_sym = gensym("#sysexin"); class_sethelpsymbol(sysexin_class, gensym("midi"));
    class_addlist( notein_class, notein_list);  notein_sym = gensym( "#notein"); class_sethelpsymbol( notein_class, gensym("midi"));
    class_addlist(  ctlin_class,  ctlin_list);   ctlin_sym = gensym(  "#ctlin"); class_sethelpsymbol(  ctlin_class, gensym("midi"));

    pgmin_class   = class_new2("pgmin",  pgmin_new,  pgmin_free,  sizeof(t_pgmin),  CLASS_NOINLET,"F");
    bendin_class  = class_new2("bendin", bendin_new, bendin_free, sizeof(t_bendin), CLASS_NOINLET,"F");
    touchin_class = class_new2("touchin",touchin_new,touchin_free,sizeof(t_touchin),CLASS_NOINLET,"F");
    class_addlist(  pgmin_class,midi2_list);
    class_addlist( bendin_class,midi2_list);
    class_addlist(touchin_class,midi2_list);
    class_sethelpsymbol(  pgmin_class, gensym("midi"));
    class_sethelpsymbol( bendin_class, gensym("midi"));
    class_sethelpsymbol(touchin_class, gensym("midi"));
    pgmin_sym   = gensym("#pgmin");
    bendin_sym  = gensym("#bendin");
    touchin_sym = gensym("#touchin");

    polytouchin_setup(); midirealtimein_setup(); midiclkin_setup();
    midiout_class  = class_new2("midiout",  midiout_new,  0, sizeof(t_midiout),  0,"FF");
    ctlout_class   = class_new2("ctlout",   ctlout_new,   0, sizeof(t_ctlout),   0,"FF");
    noteout_class  = class_new2("noteout",  noteout_new,  0, sizeof(t_noteout),  0,"F");
    pgmout_class   = class_new2("pgmout",   pgmout_new,   0, sizeof(t_pgmout),   0,"F");
    bendout_class  = class_new2("bendout",  bendout_new,  0, sizeof(t_bendout),  0,"F");
    touchout_class = class_new2("touchout", touchout_new, 0, sizeof(t_touchout), 0,"F");
    class_addfloat( midiout_class,  midiout_float); class_sethelpsymbol( midiout_class, gensym("midi"));
    class_addfloat(  ctlout_class,   ctlout_float); class_sethelpsymbol(  ctlout_class, gensym("midi"));
    class_addfloat( noteout_class,  noteout_float); class_sethelpsymbol( noteout_class, gensym("midi"));
    class_addfloat(  pgmout_class,   pgmout_float); class_sethelpsymbol(  pgmout_class, gensym("midi"));
    class_addfloat( bendout_class,  bendout_float); class_sethelpsymbol( bendout_class, gensym("midi"));
    class_addfloat(touchout_class, touchout_float); class_sethelpsymbol(touchout_class, gensym("midi"));
    polytouchout_setup(); makenote_setup(); stripnote_setup(); poly_setup(); bag_setup();
}

static t_class *delay_class;
struct t_delay : t_object {
    t_clock *clock;
    double deltime;
};
static void delay_bang(t_delay *x) {clock_delay(x->clock, x->deltime);}
static void delay_stop(t_delay *x) {clock_unset(x->clock);}
static void delay_ft1(t_delay *x, t_floatarg g) {x->deltime = max(0.f,g);}
static void delay_float(t_delay *x, t_float f) {delay_ft1(x, f); delay_bang(x);}
static void delay_tick(t_delay *x) {x->outlet->send();}
static void delay_free(t_delay *x) {clock_free(x->clock);}
static void *delay_new(t_floatarg f) {
    t_delay *x = (t_delay *)pd_new(delay_class);
    delay_ft1(x, f);
    x->clock = clock_new(x, (t_method)delay_tick);
    outlet_new(x, gensym("bang"));
    inlet_new(x, x, gensym("float"), gensym("ft1"));
    return x;
}
static void delay_setup() {
    t_class *c = delay_class = class_new2("delay",delay_new,delay_free,sizeof(t_delay),0,"F");
    class_addcreator2("del",delay_new,"F");
    class_addbang(c, delay_bang);
    class_addmethod2(c,delay_stop,"stop","");
    class_addmethod2(c,delay_ft1,"ft1","f");
    class_addfloat(c, delay_float);
}

static t_class *metro_class;
struct t_metro : t_object {
    t_clock *clock;
    double deltime;
    int hit;
};
static void metro_tick(t_metro *x) {
    x->hit = 0;
    x->outlet->send();
    if (!x->hit) clock_delay(x->clock, x->deltime);
}
static void metro_float(t_metro *x, t_float f) {
    if (f) metro_tick(x); else clock_unset(x->clock);
    x->hit = 1;
}
static void metro_bang(t_metro *x) {metro_float(x, 1);}
static void metro_stop(t_metro *x) {metro_float(x, 0);}
static void metro_ft1(t_metro *x, t_floatarg g) {x->deltime = max(1.f,g);}
static void metro_free(t_metro *x) {clock_free(x->clock);}
static void *metro_new(t_floatarg f) {
    t_metro *x = (t_metro *)pd_new(metro_class);
    metro_ft1(x, f);
    x->hit = 0;
    x->clock = clock_new(x, (t_method)metro_tick);
    outlet_new(x, gensym("bang"));
    inlet_new(x, x, gensym("float"), gensym("ft1"));
    return x;
}
static void metro_setup() {
    t_class *c = metro_class = class_new2("metro",metro_new,metro_free,sizeof(t_metro),0,"F");
    class_addbang(c, metro_bang);
    class_addmethod2(c,metro_stop,"stop","");
    class_addmethod2(c,metro_ft1,"ft1","f");
    class_addfloat(c, metro_float);
}

static t_class *line_class;
struct t_line : t_object {
    t_clock *clock;
    double targettime;
    t_float targetval;
    double prevtime;
    t_float setval;
    int gotinlet;
    t_float grain;
    double oneovertimediff;
    double in1val;
};
static void line_tick(t_line *x) {
    double timenow = clock_getsystime();
    double msectogo = - clock_gettimesince(x->targettime);
    if (msectogo < 1E-9) {
        x->outlet->send(x->targetval);
    } else {
        x->outlet->send(x->setval + x->oneovertimediff * (timenow-x->prevtime) * (x->targetval-x->setval));
        clock_delay(x->clock, (x->grain > msectogo ? msectogo : x->grain));
    }
}
static void line_float(t_line *x, t_float f) {
    double timenow = clock_getsystime();
    if (x->gotinlet && x->in1val > 0) {
        if (timenow > x->targettime) x->setval = x->targetval;
        else x->setval = x->setval + x->oneovertimediff * (timenow-x->prevtime) * (x->targetval-x->setval);
        x->prevtime = timenow;
        x->targettime = clock_getsystimeafter(x->in1val);
        x->targetval = f;
        line_tick(x);
        x->gotinlet = 0;
        x->oneovertimediff = 1./ (x->targettime - timenow);
        clock_delay(x->clock, (x->grain > x->in1val ? x->in1val : x->grain));
    } else {
        clock_unset(x->clock);
        x->targetval = x->setval = f;
        x->outlet->send(f);
    }
    x->gotinlet = 0;
}
static void line_ft1(t_line *x, t_floatarg g) {
    x->in1val = g;
    x->gotinlet = 1;
}
static void line_stop(t_line *x) {
    x->targetval = x->setval;
    clock_unset(x->clock);
}
static void line_set(t_line *x, t_floatarg f) {
    clock_unset(x->clock);
    x->targetval = x->setval = f;
}
static void line_set_granularity(t_line *x, t_floatarg grain) {
    if (grain <= 0) grain = 20;
    x->grain = grain;
}
static void line_free(t_line *x) {
    clock_free(x->clock);
}
static void *line_new(t_floatarg f, t_floatarg grain) {
    t_line *x = (t_line *)pd_new(line_class);
    x->targetval = x->setval = f;
    x->gotinlet = 0;
    x->oneovertimediff = 1;
    x->clock = clock_new(x, (t_method)line_tick);
    x->targettime = x->prevtime = clock_getsystime();
	line_set_granularity(x, grain);
    outlet_new(x, gensym("float"));
    inlet_new(x, x, gensym("float"), gensym("ft1"));
    return x;
}
static void line_setup() {
    t_class *c = line_class = class_new2("line",line_new,line_free,sizeof(t_line),0,"FF");
    class_addmethod2(c,line_ft1,"ft1","f");
    class_addmethod2(c,line_stop,"stop","");
    class_addmethod2(c,line_set,"set","f");
    class_addmethod2(c,line_set_granularity,"granularity","f");
    class_addfloat(c,line_float);
}

static t_class *pipe_class;
struct t_hang2 {
    t_clock *clock;
    t_hang2 *next;
    struct t_pipe *owner;
    union word vec[0]; /* not the actual number of elements */
};
struct t_pipe : t_object {
    int n;
    float deltime;
    t_atom *vec;
    t_hang2 *hang;
};
static void *pipe_new(t_symbol *s, int argc, t_atom *argv) {
    t_pipe *x = (t_pipe *)pd_new(pipe_class);
    t_atom defarg, *ap;
    t_atom *vec, *vp;
    float deltime=0;
    if (argc) {
        if (argv[argc-1].a_type != A_FLOAT) {
            std::ostringstream os;
            atom_ostream(&argv[argc-1], os);
            post("pipe: %s: bad time delay value", os.str().data());
        } else deltime = argv[argc-1].a_float;
        argc--;
    }
    if (!argc) {
        argv = &defarg;
        argc = 1;
        SETFLOAT(&defarg, 0);
    }
    x->n = argc;
    vec = x->vec = (t_atom *)getbytes(argc * sizeof(*x->vec));
    vp = vec;
    ap = argv;
    for (int i=0; i<argc; i++, ap++, vp++) {
        if (ap->a_type == A_FLOAT) {
            *vp = *ap;
            outlet_new(x, &s_float);
            if (i) floatinlet_new(x, &vp->a_float);
        } else if (ap->a_type == A_SYMBOL) {
            char c = *ap->a_symbol->name;
            if      (c=='s') {SETSYMBOL(vp, &s_symbol); outlet_new(x, &s_symbol); if (i) symbolinlet_new(x, &vp->a_symbol);}
            else if (c=='p') {vp->a_type = A_POINTER; outlet_new(x, &s_pointer); /*if (i) pointerinlet_new(x, gp);*/}
            else if (c=='f') {SETFLOAT(vp,0); outlet_new(x, &s_float); if (i) floatinlet_new(x, &vp->a_float);}
            else error("pack: %s: bad type", ap->a_symbol->name);
        }
    }
    floatinlet_new(x, &x->deltime);
    x->hang = 0;
    x->deltime = deltime;
    return x;
}
static void hang_free(t_hang2 *h) {
    clock_free(h->clock);
    free(h);
}
static void hang_tick(t_hang2 *h) {
    t_pipe *x = h->owner;
    t_hang2 *h2, *h3;
    if (x->hang == h) x->hang = h->next;
    else for (h2 = x->hang; (h3 = h2->next); h2 = h3) {
        if (h3 == h) {
            h2->next = h3->next;
            break;
        }
    }
    t_atom *p = x->vec + x->n-1;
    t_word *w = h->vec + x->n-1;
    for (int i = x->n; i--; p--, w--) {
        switch (p->a_type) {
        case A_FLOAT:  x->out(i)->send(w->w_float   ); break;
        case A_SYMBOL: x->out(i)->send(w->w_symbol  ); break;
        case A_POINTER:x->out(i)->send(w->w_gpointer); break;
        default:{}
        }
    }
    hang_free(h);
}
static void pipe_list(t_pipe *x, t_symbol *s, int ac, t_atom *av) {
    t_hang2 *h = (t_hang2 *)getbytes(sizeof(*h) + (x->n - 1) * sizeof(*h->vec));
    int n = x->n;
    if (ac > n) ac = n;
    t_atom *p = x->vec;
    t_atom *ap = av;
    for (int i=0; i<ac; i++, p++, ap++) *p = *ap;
    t_word *w = h->vec;
    p = x->vec;
    for (int i=0; i< n; i++, p++,  w++) *w = p->a_w;
    h->next = x->hang;
    x->hang = h;
    h->owner = x;
    h->clock = clock_new(h, (t_method)hang_tick);
    clock_delay(h->clock, (x->deltime >= 0 ? x->deltime : 0));
}
static void pipe_flush(t_pipe *x) {
    while (x->hang) hang_tick(x->hang);
}
static void pipe_clear(t_pipe *x) {
    t_hang2 *hang;
    while ((hang = x->hang)) {
        x->hang = hang->next;
        hang_free(hang);
    }
}
static void pipe_setup() {
    pipe_class = class_new2("pipe",pipe_new,pipe_clear,sizeof(t_pipe),0,"*");
    class_addlist(pipe_class, pipe_list);
    class_addmethod2(pipe_class,pipe_flush,"flush","");
    class_addmethod2(pipe_class,pipe_clear,"clear","");
}

/* ---------------------------------------------------------------- */
/* new desiredata classes are below this point. */

static t_class *unpost_class;
struct t_unpost : t_object {
	t_outlet *o0,*o1;
};
struct t_unpost_frame {
	t_unpost *self;
	std::ostringstream buf;
};
static t_unpost_frame *current_unpost;

void *unpost_new (t_symbol *s) {
    t_unpost *x = (t_unpost *)pd_new(unpost_class);
    x->o0 = outlet_new(x,&s_symbol);
    x->o1 = outlet_new(x,&s_symbol);
    return x;
}
extern t_printhook sys_printhook;
void unpost_printhook (const char *s) {
    std::ostringstream &b = current_unpost->buf;
    b << s;
    const char *p;
    const char *d=b.str().data(),*dd=d;
    for (;;) {
        p = strchr(d,'\n');
        if (!p) break;
        current_unpost->self->o1->send(gensym2(d,p-d));
	d=p+1;
    }
    if (d!=dd) {
        char *q = strdup(d); /* well i could use memmove, but i'm not supposed to use strcpy because of overlap */
        current_unpost->buf.clear();
	current_unpost->buf << q;
        free(q);
    }
}
void unpost_anything (t_unpost *x, t_symbol *s, int argc, t_atom *argv) {
    t_printhook backup1 = sys_printhook;
    t_unpost_frame *backup2 = current_unpost;
    sys_printhook = unpost_printhook;
    current_unpost = new t_unpost_frame;
    current_unpost->self = x;
    x->o0->send(s,argc,argv);
    sys_printhook = backup1;
    current_unpost = backup2;
}

struct t_unparse : t_object {};
static t_class *unparse_class;
void *unparse_new (t_symbol *s) {
    t_unparse *x = (t_unparse *)pd_new(unparse_class);
    outlet_new(x,&s_symbol);
    return x;
}
void unparse_list (t_unparse *x, t_symbol *s, int argc, t_atom *argv) {
    std::ostringstream o;
    for (int i=0; i<argc; i++) {
	o << ' ';
	atom_ostream(argv+i,o);
    }
    x->outlet->send(gensym(o.str().data()+1));
}

struct t_parse : t_object {};
static t_class *parse_class;
void *parse_new (t_symbol *s) {
    t_parse *x = (t_parse *)pd_new(parse_class);
    outlet_new(x,&s_list);
    return x;
}
void parse_symbol (t_unpost *x, t_symbol *s) {
    t_binbuf *b = binbuf_new();
    binbuf_text(b,s->name,s->n);
    x->outlet->send(&s_list,b->n,b->v);
    binbuf_free(b);
}

struct t_tracecall : t_object {};
static t_class *tracecall_class;
void *tracecall_new (t_symbol *s) {
    t_tracecall *x = (t_tracecall *)pd_new(tracecall_class);
    outlet_new(x,&s_list);
    return x;
}
void tracecall_anything (t_tracecall *x, t_symbol *dummy, int dum, t_atom *my) {
    t_atom a[2];
    for (int i=pd_stackn-1; i>=0; i--) {
	SETSYMBOL( &a[0],pd_stack[i].self->_class->name);
	SETSYMBOL( &a[1],pd_stack[i].s);
	//SETPOINTER(&a[2],pd_stack[i].self);
	x->outlet->send(2,a);
    }
}

static void matju_setup() {
    unpost_class = class_new2("unpost",unpost_new,0,sizeof(t_unpost),0,"");
    class_addanything(unpost_class, unpost_anything);
    unparse_class = class_new2("unparse",unparse_new,0,sizeof(t_unparse),0,"");
    class_addlist(unparse_class, unparse_list);
    parse_class = class_new2("parse",parse_new,0,sizeof(t_parse),0,"");
    class_addsymbol(parse_class, parse_symbol);
    tracecall_class = class_new2("tracecall",tracecall_new,0,sizeof(t_tracecall),0,"");
    class_addanything(tracecall_class,tracecall_anything);
}

/* end of new desiredata classes */
/* ---------------------------------------------------------------- */

void builtins_setup() {
    t_symbol *s = gensym("acoustics.pd");
    FUNC1DECL(mtof,   "mtof");
    FUNC1DECL(ftom,   "ftom");
    FUNC1DECL(powtodb,"powtodb");
    FUNC1DECL(rmstodb,"rmstodb");
    FUNC1DECL(dbtopow,"dbtopow");
    FUNC1DECL(dbtorms,"dbtorms");
    random_setup();
    loadbang_setup();
    namecanvas_setup();
    print_setup();
    macro_setup();
    display_setup();
    any_setup();
    clipboard_setup();
    delay_setup();
    metro_setup();
    line_setup();
    timer_setup();
    pipe_setup();
    misc_setup();
    sendreceive_setup();
    select_setup();
    route_setup();
    pack_setup();
    unpack_setup();
    trigger_setup();
    spigot_setup();
    moses_setup();
    until_setup();
    makefilename_setup();
    swap_setup();
    change_setup();
    value_setup();

    t_class *c;
    qlist_class = c = class_new2("qlist",qlist_new,qlist_free,sizeof(t_qlist),0,"");
    class_addmethod2(c,qlist_rewind, "rewind","");
    class_addmethod2(c,qlist_next, "next","F");
    class_addmethod2(c,qlist_set, "set","*");
    class_addmethod2(c,qlist_clear, "clear","");
    class_addmethod2(c,qlist_add, "add","*");
    class_addmethod2(c,qlist_add2, "add2","*");
    class_addmethod2(c,qlist_add, "append","*");
    class_addmethod2(c,qlist_read, "read","sS");
    class_addmethod2(c,qlist_write, "write","sS");
    class_addmethod2(c,qlist_print, "print","S");
    class_addmethod2(c,qlist_tempo, "tempo","f");
    class_addbang(c,qlist_bang);

    textfile_class = c = class_new2("textfile",textfile_new,textfile_free,sizeof(t_textfile),0,"");
    class_addmethod2(c,textfile_rewind, "rewind","");
    class_addmethod2(c,qlist_set, "set","*");
    class_addmethod2(c,qlist_clear, "clear","");
    class_addmethod2(c,qlist_add, "add","*");
    class_addmethod2(c,qlist_add2, "add2","*");
    class_addmethod2(c,qlist_add, "append","*");
    class_addmethod2(c,qlist_read, "read","sS");
    class_addmethod2(c,qlist_write, "write","sS");
    class_addmethod2(c,qlist_print, "print","S");
    class_addbang(c,textfile_bang);
    netsend_setup();
    netreceive_setup();
    openpanel_setup();
    savepanel_setup();
    key_setup();

extern t_class *binbuf_class;
    class_addlist(binbuf_class, alist_list);
    class_addanything(binbuf_class, alist_anything);
    list_setup();
    arithmetic_setup();
    midi_setup();
    matju_setup();
}
