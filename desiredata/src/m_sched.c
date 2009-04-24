/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  scheduling stuff  */

#include "desire.h"
#include <stdlib.h>

/* for timeval */
#ifdef UNISTD
#include <sys/time.h>
#endif
#ifdef __CYGWIN__
#include <sys/time.h>
#endif
#ifdef MSW
#include "winsock.h"
#endif

#include "assert.h"

/* LATER consider making this variable.  It's now the LCM of all sample
   rates we expect to see: 32000, 44100, 48000, 88200, 96000. */
#define TIMEUNITPERSEC (32.*441000.)

/* T.Grill - enable PD global thread locking - sys_lock, sys_unlock, sys_trylock functions */
#define THREAD_LOCKING
#include "pthread.h"
#include "time.h"

static int sys_quit;
double sys_time;
static double sys_time_per_msec = TIMEUNITPERSEC / 1000.;

/* tb: { */
int sys_keepsched = 1;          /* if 0: change scheduler mode  */
int sys_callbackscheduler = 0;  /* if 1: change scheduler to callback based dsp */
static void run_idle_callbacks(int microsec);
t_fifo * callback_fifo = NULL;
/* tb: }*/

int sys_usecsincelastsleep ();
int sys_sleepgrain;

typedef void (*t_clockmethod)(void *client);

#ifdef UNISTD
#include <unistd.h>
#endif

struct _clock {
    double settime;
    void *owner;
    t_clockmethod fn;
    struct _clock *next;
};

t_clock *clock_setlist;
t_clock *clock_new(void *owner, t_method fn) {
    t_clock *x = (t_clock *)malloc(sizeof(*x));
    x->settime = -1;
    x->owner = owner;
    x->fn = (t_clockmethod)fn;
    x->next = 0;
    return x;
}

void clock_unset(t_clock *x) {
    if (x->settime >= 0) {
        if (x == clock_setlist) clock_setlist = x->next;
        else {
            t_clock *x2 = clock_setlist;
            while (x2->next != x) x2 = x2->next;
            x2->next = x->next;
        }
        x->settime = -1;
    }
}

/* set the clock to call back at an absolute system time */
void clock_set(t_clock *x, double setticks) {
    if (setticks < sys_time) setticks = sys_time;
    clock_unset(x);
    x->settime = setticks;
    if (clock_setlist && clock_setlist->settime <= setticks) {
        t_clock *cbefore, *cafter;
        for (cbefore = clock_setlist, cafter = clock_setlist->next; cbefore; cbefore = cafter, cafter = cbefore->next) {
            if (!cafter || cafter->settime > setticks) {
                cbefore->next = x;
                x->next = cafter;
                return;
            }
        }
    } else x->next = clock_setlist, clock_setlist = x;
}

/* set the clock to call back after a delay in msec */
void clock_delay(t_clock *x, double delaytime) {
    clock_set(x, sys_time + sys_time_per_msec * delaytime);
}

/* get current logical time.  We don't specify what units this is in;
   use clock_gettimesince() to measure intervals from time of this call. 
   This was previously, incorrectly named "clock_getsystime"; the old
   name is aliased to the new one in m_pd.h. */
double clock_getlogicaltime () {return sys_time;}

/* OBSOLETE NAME */
double clock_getsystime () {return sys_time;}

/* elapsed time in milliseconds since the given system time */
double clock_gettimesince(double prevsystime) {
    return (sys_time - prevsystime)/sys_time_per_msec;
}

/* what value the system clock will have after a delay */
double clock_getsystimeafter(double delaytime) {
    return sys_time + sys_time_per_msec * delaytime;
}

void clock_free(t_clock *x) {
    clock_unset(x);
    free(x);
}

/* the following routines maintain a real-execution-time histogram of the
various phases of real-time execution. */

static int sys_bin[] = {0, 2, 5, 10, 20, 30, 50, 100, 1000};
#define NBIN (sizeof(sys_bin)/sizeof(*sys_bin))
#define NHIST 10
static int sys_histogram[NHIST][NBIN];
static double sys_histtime;
static int sched_diddsp, sched_didpoll, sched_didnothing;

void sys_clearhist () {
    unsigned i,j;
    for (i=0; i<NHIST; i++) for (j=0; j<NBIN; j++) sys_histogram[i][j] = 0;
    sys_histtime = sys_getrealtime();
    sched_diddsp = sched_didpoll = sched_didnothing = 0;
}

void sys_printhist () {
    for (int i=0; i<NHIST; i++) {
        int doit = 0;
        for (unsigned int j=0; j<NBIN; j++) if (sys_histogram[i][j]) doit = 1;
        if (doit) {
            post("%2d %8d %8d %8d %8d %8d %8d %8d %8d", i,
                sys_histogram[i][0], sys_histogram[i][1], sys_histogram[i][2], sys_histogram[i][3],
                sys_histogram[i][4], sys_histogram[i][5], sys_histogram[i][6], sys_histogram[i][7]);
        }
    }
    post("dsp %d, pollgui %d, nothing %d", sched_diddsp, sched_didpoll, sched_didnothing);
}

static int sys_histphase;

int sys_addhist(int phase) {
    int phasewas = sys_histphase;
    double newtime = sys_getrealtime();
    int msec = int((newtime-sys_histtime)*1000.);
    for (int j=NBIN-1; j >= 0; j--) {
        if (msec >= sys_bin[j]) {
            sys_histogram[phasewas][j]++;
            break;
        }
    }
    sys_histtime = newtime;
    sys_histphase = phase;
    return phasewas;
}

#define NRESYNC 20

struct t_resync {
    int ntick;
    int error;
};

static int oss_resyncphase = 0;
static int oss_nresync = 0;
static t_resync oss_resync[NRESYNC];

static const char *(oss_errornames[]) = {
"unknown",
"ADC blocked",
"DAC blocked",
"A/D/A sync",
"data late",
"xrun",
"sys_lock timeout"
};

void glob_audiostatus (void *dummy) {
    int nresync, nresyncphase, i;
    nresync = oss_nresync >= NRESYNC ? NRESYNC : oss_nresync;
    nresyncphase = oss_resyncphase - 1;
    post("audio I/O error history:");
    post("seconds ago\terror type");
    for (i = 0; i < nresync; i++) {
        int errtype;
        if (nresyncphase < 0) nresyncphase += NRESYNC;
        errtype = oss_resync[nresyncphase].error;
        if (errtype < 0 || errtype > 4) errtype = 0;
        post("%9.2f\t%s", (sched_diddsp - oss_resync[nresyncphase].ntick)
                * ((double)sys_schedblocksize) / sys_dacsr, oss_errornames[errtype]);
        nresyncphase--;
    }
}

static int sched_diored;
static int sched_dioredtime;
static int sched_meterson;

void sys_log_error(int type) {
    oss_resync[oss_resyncphase].ntick = sched_diddsp;
    oss_resync[oss_resyncphase].error = type;
    oss_nresync++;
    if (++oss_resyncphase == NRESYNC) oss_resyncphase = 0;
    if (type != ERR_NOTHING && !sched_diored && (sched_diddsp >= sched_dioredtime)) {
        sys_vgui("pdtk_pd_dio 1\n");
        sched_diored = 1;
    }
    sched_dioredtime = sched_diddsp + (int)(sys_dacsr /(double)sys_schedblocksize);
}

static int sched_lastinclip, sched_lastoutclip, sched_lastindb, sched_lastoutdb;

static void sched_pollformeters () {
    int inclip, outclip, indb, outdb;
    static int sched_nextmeterpolltime, sched_nextpingtime;
    /* if there's no GUI but we're running in "realtime", here is
       where we arrange to ping the watchdog every 2 seconds. */
#ifdef __linux__
    if (sys_hipriority && (sched_diddsp - sched_nextpingtime > 0)) {
        glob_watchdog(0);
        /* ping every 2 seconds */
        sched_nextpingtime = sched_diddsp + 2*(int)(sys_dacsr /(double)sys_schedblocksize);
    }
#endif
    if (sched_diddsp - sched_nextmeterpolltime < 0) return;
    if (sched_diored && sched_diddsp-sched_dioredtime > 0) {
        sys_vgui("pdtk_pd_dio 0\n");
        sched_diored = 0;
    }
    if (sched_meterson) {
        float inmax, outmax;
        sys_getmeters(&inmax, &outmax);
        indb = int(0.5 + rmstodb(inmax));
        outdb = int(0.5 + rmstodb(outmax));
        inclip = inmax > 0.999;
        outclip = outmax >= 1.0;
    } else {
        indb = outdb = 0;
        inclip = outclip = 0;
    }
    if (inclip != sched_lastinclip || outclip != sched_lastoutclip
    || indb != sched_lastindb || outdb != sched_lastoutdb) {
        sys_vgui("pdtk_pd_meters %d %d %d %d\n", indb, outdb, inclip, outclip);
        sched_lastinclip = inclip;
        sched_lastoutclip = outclip;
        sched_lastindb = indb;
        sched_lastoutdb = outdb;
    }
    sched_nextmeterpolltime = sched_diddsp + (int)(sys_dacsr /(double)sys_schedblocksize);
}

void glob_meters(void *dummy, float f) {
    if (f == 0) sys_getmeters(0, 0);
    sched_meterson = (f != 0);
    sched_lastinclip = sched_lastoutclip = sched_lastindb = sched_lastoutdb = -1;
}

#if 0
void glob_foo(void *dummy, t_symbol *s, int argc, t_atom *argv) {
    if (argc) sys_clearhist();
    else sys_printhist();
}
#endif

extern void dsp_tick ();

static int sched_usedacs = 0;
static double sched_referencerealtime, sched_referencelogicaltime;
double sys_time_per_dsp_tick;

void sched_set_using_dacs(int flag) {
    sched_usedacs = flag;
    if (!flag) {
        sched_referencerealtime = sys_getrealtime();
        sched_referencelogicaltime = clock_getlogicaltime();
    }
    sys_time_per_dsp_tick = (TIMEUNITPERSEC) * ((double)sys_schedblocksize) / sys_dacsr;
}

static void run_clock_callbacks(double next_sys_time) {
    if (clock_setlist && clock_setlist->settime <= next_sys_time) {
        do {
            t_clock *c = clock_setlist;
            sys_time = c->settime;
            clock_unset(c); /* the compiler should easily inline this */
            outlet_setstacklim();
            c->fn(c->owner);
        } while (clock_setlist && clock_setlist->settime <= next_sys_time);
    }
}

/* take the scheduler forward one DSP tick, also handling clock timeouts */
void sched_tick(double next_sys_time) {
    run_clock_callbacks(next_sys_time);
    sys_time = next_sys_time;
    sched_diddsp++; /* rethink: how to get rid of this stupid histogram??? */
    dsp_tick();
    /* rethink: should we really do all this midi messaging in the realtime thread ? */
    sys_pollmidiqueue();
    sys_setmiditimediff(0, 1e-6 * sys_schedadvance);
}


/*
Here is Pd's "main loop."  This routine dispatches clock timeouts and DSP
"ticks" deterministically, and polls for input from MIDI and the GUI.  If
we're left idle we also poll for graphics updates; but these are considered
lower priority than the rest.

The time source is normally the audio I/O subsystem via the "sys_send_dacs()"
call.  This call returns true if samples were transferred; false means that
the audio I/O system is still busy with previous transfers.
*/

void sys_pollmidiqueue ();
void sys_initmidiqueue ();

void canvas_stop_dsp ();

int m_scheduler () {
    int idlecount = 0;
    sys_time_per_dsp_tick = (TIMEUNITPERSEC) * ((double)sys_schedblocksize) / sys_dacsr;
    /* T.Grill - lock mutex */
    sys_lock();
    sys_clearhist();
    /* tb: adapt sleepgrain with advance */
    sys_update_sleepgrain();
    sched_set_using_dacs(0); /* tb: dsp is switched off */
    sys_initmidiqueue();
    while (!sys_quit) {
        if (!sys_callbackscheduler || !sched_usedacs)
            while (sys_keepsched) {
                int didsomething = 0;
                int timeforward;
            waitfortick:
                if (sched_usedacs) {
                    timeforward = sys_send_dacs();
                    /* if dacs remain "idle" for 1 sec, they're hung up. */
                    if (timeforward != 0)
                        idlecount = 0;
                    else {
                        idlecount++;
                        if (!(idlecount & 31)) {
                            static double idletime;
                            /* on 32nd idle, start a clock watch;  every 32 ensuing idles, check it */
                            if (idlecount == 32) idletime = sys_getrealtime();
                            else if (sys_getrealtime() - idletime > 1.) {
                                post("audio I/O stuck... closing audio");
                                sys_close_audio();
                                sched_set_using_dacs(0);
				canvas_stop_dsp(); /* added by matju 2007.06.30 */
                                goto waitfortick;
                            }
                        }
                    }
                } else {
                    if (1000. * (sys_getrealtime() - sched_referencerealtime) > clock_gettimesince(sched_referencelogicaltime))
                        timeforward = SENDDACS_YES;
                    else timeforward = SENDDACS_NO;
                }
                sys_setmiditimediff(0, 1e-6 * sys_schedadvance);
                if (timeforward != SENDDACS_NO) sched_tick(sys_time + sys_time_per_dsp_tick);
                if (timeforward == SENDDACS_YES) didsomething = 1;
                sys_pollmidiqueue();
                if (sys_pollgui()) didsomething = 1;
                /* test for idle; if so, do graphics updates. */
                if (!didsomething) {
                    sched_pollformeters();
                    /* tb: call idle callbacks */
                    if (timeforward != SENDDACS_SLEPT) run_idle_callbacks(sys_sleepgrain);
                }
            }
        else /* tb: scheduler for callback-based dsp scheduling */
            while(sys_keepsched) {
                /* tb: allow the audio callback to run */
                sys_unlock();
                sys_microsleep(sys_sleepgrain);
                sys_lock();
                sys_pollmidiqueue();
                sys_setmiditimediff(0, 1e-6 * sys_schedadvance);
                if (sys_pollgui()) continue;
                /* do graphics updates and run idle callbacks */
                sched_pollformeters();
            }
        sys_keepsched = 1;
    }
    sys_close_audio();
    sys_unlock();
    return 0;
}

/* ------------ thread locking ------------------- */
/* added by Thomas Grill */

#ifdef THREAD_LOCKING
static pthread_mutex_t sys_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sys_cond = PTHREAD_COND_INITIALIZER;

void sys_lock () {
    pthread_mutex_lock(&sys_mutex);
}
void sys_unlock () {
    pthread_mutex_unlock(&sys_mutex);
    pthread_cond_signal(&sys_cond);
}
int sys_trylock () {
    return pthread_mutex_trylock(&sys_mutex);
}

/* tb { */
#include <errno.h>

#ifdef MSW
/* gettimeofday isn't available on windoze ... */
int gettimeofday (struct timeval *tv, void* tz) {
    __int64 now; /* time since 1 Jan 1601 in 100ns  */
    GetSystemTimeAsFileTime ((FILETIME*) &now);
    tv->tv_usec = (long) ((now / 10LL) % 1000000LL);
    tv->tv_sec = (long) ((now - 116444736000000000LL) / 10000000LL);
    return 0;
}
#endif

#if 0
/* osx doesn't define a pthread_mutex_timedlock ... maybe someday
   it will ... */
int sys_timedlock(int microsec) {
        struct timespec timeout;
        struct timeval now;
        /* timedlock seems to have a resolution of 1ms */
        if (microsec < 1000) microsec = 1000;
        gettimeofday(&now,0);
        timeout.tv_sec = now.tv_sec;
        timeout.tv_nsec = (now.tv_usec + microsec) * 1000;
        while (timeout.tv_nsec > 1e9) {
                timeout.tv_sec += 1;
                timeout.tv_nsec -= 1e9;
        }
        int ret = pthread_mutex_timedlock(&sys_mutex, &timeout);
        if (ret) post("timeout, %d", ret);
        return ret;
}
#else

int sys_timedlock(int microsec) {
    struct timespec timeout;
    struct timeval now;
    if (sys_trylock() == 0) return 0;
    if (microsec < 1000) microsec = 1000;
    gettimeofday(&now,0);
    timeout.tv_sec = now.tv_sec;
    timeout.tv_nsec = (now.tv_usec + microsec) * 1000;
    while (timeout.tv_nsec > 1000000000) {
        timeout.tv_sec += 1;
        timeout.tv_nsec -= 1000000000;
    }
    /* in case the lock has been released during the system call, try
       again before waiting for the signal */
    if (sys_trylock() == 0) return 0;
    return pthread_cond_timedwait(&sys_cond, &sys_mutex, &timeout);
}
#endif
/* tb } */

#else
void sys_lock () {}
void sys_unlock () {}
int sys_trylock () { return 0; }
int sys_timedlock (int microsec) { return 0; }
#endif

/* ------------ soft quit ------------------- */
/* added by Thomas Grill -
        just set the quit flag for the scheduler loop
        this is useful for applications using the PD shared library to signal the scheduler to terminate
*/

void sys_exit () {
    sys_keepsched = 0;
    sys_quit = 1;
}

/* tb: place callbacks in scheduler 
 * {   */
/* linked list of callbacks; callback will be freed after returning 0 */
struct t_sched_callback {
    struct t_sched_callback *next; /* next callback in ringbuffer / in fifo */
    t_int (*function)(t_int *argv);
    t_int *argv;
    t_int argc;
};

void sys_callback(t_int (*callback)(t_int* argv), t_int* argv, t_int argc) {
    t_sched_callback* noo = (t_sched_callback *)malloc(sizeof(t_sched_callback));
    noo->function = callback;
    if (argv && argc) {
        noo->argv = (t_int*) copybytes (argv, argc * sizeof (t_int));
        noo->argc = argc;
    } else {
        noo->argc = 0;
        noo->argv = 0;
    }
    noo->next = 0;
    if (!callback_fifo) callback_fifo = fifo_init();
    fifo_put(callback_fifo, noo);
}

void sys_init_idle_callbacks () {
    callback_fifo = fifo_init(); /* tb: initialize fifo for idle callbacks */
}

static t_sched_callback *ringbuffer_head = NULL;

void run_all_idle_callbacks () {
    t_sched_callback *new_callback;
    /* append idle callback to ringbuffer */
    while ((new_callback = (t_sched_callback*) fifo_get(callback_fifo))) {
        t_sched_callback *next;
        /* set the next field to 0 ... it might be set in the fifo */
        new_callback->next = 0;
        if (!ringbuffer_head) {
            ringbuffer_head = new_callback;
        } else {
            next = ringbuffer_head;
            while (next->next) next = next->next;
            next->next = new_callback;
        }
    }
    if (ringbuffer_head) {
        t_sched_callback *idle_callback = ringbuffer_head;
        t_sched_callback *last = 0;
        t_sched_callback *next;
        do {
            int status;
            status = (idle_callback->function)(idle_callback->argv);
            switch (status) {
                /* callbacks returning 0 will be deleted */
            case 0:
                next = idle_callback->next;
                if (idle_callback->argv) free(idle_callback->argv);
                free((void*)idle_callback);
                if (!last) ringbuffer_head = next; else last->next = next;
                idle_callback = next;
                /* callbacks returning 1 will be run again */
            case 1:
                break;
                /* callbacks returning 2 will be run during the next idle callback */
            case 2:
                last = idle_callback;
                idle_callback = idle_callback->next;
            }
        } while (idle_callback);
    }
}

static void run_idle_callbacks(int microsec) {
    t_sched_callback *new_callback;
    double stop = sys_getrealtime()*1.e6 + (double)microsec;
    /* append idle callback to ringbuffer */
    while ((new_callback = (t_sched_callback*) fifo_get(callback_fifo))) {
        /* set the next field to NULL ... it might be set in the fifo */
        new_callback->next = 0;
        if (!ringbuffer_head) {
            ringbuffer_head = new_callback;
        } else {
	    t_sched_callback *next = ringbuffer_head;
            while (next->next != 0)
                next = next->next;
            next->next = new_callback;
        }
    }
    if (ringbuffer_head) {
        double remain = stop - sys_getrealtime() * 1.e6;
        t_sched_callback *idle_callback = ringbuffer_head;
        t_sched_callback *last = 0;
        t_sched_callback *next;
        do {
//            sys_lock();
            int status = idle_callback->function(idle_callback->argv);
//            sys_unlock();
            switch (status) {
                /* callbacks returning 0 will be deleted */
            case 0:
                next = idle_callback->next;
                if (idle_callback->argc) free(idle_callback->argv);
                free((void*)idle_callback);
                if (!last) ringbuffer_head = next; else last->next = next;
                idle_callback = next;
                /* callbacks returning 1 will be run again */
            case 1:
                break;
                /* callbacks returning 2 will be run during the next idle callback */
            case 2:
                last = idle_callback;
                idle_callback = idle_callback->next;
            }
            remain = stop-sys_getrealtime()*1.e6;
        } while (idle_callback && remain>0);
        /* sleep for the rest of the time */
        if(remain > 0) {
		sys_unlock();
		sys_microsleep(int(remain));
		sys_lock();
	}
    } else {
	sys_unlock();
        sys_microsleep(microsec);
	sys_lock();
    }
}
/* } tb */

void sys_setscheduler(int scheduler) {
    sys_keepsched = 0;
    sys_callbackscheduler = scheduler;
    return;
}

int sys_getscheduler () {return sys_callbackscheduler;}

static t_int sys_xrun_notification_callback(t_int *dummy) {
    t_symbol *pd = gensym("pd");
    t_symbol *xrun = gensym("xrun");
    typedmess(pd->s_thing, xrun, 0, 0);
    return 0;
}

void sys_xrun_notification () {sys_callback(sys_xrun_notification_callback, 0, 0);}

static t_int sys_lock_timeout_notification_callback(t_int *dummy) {
    t_symbol *pd = gensym("pd");
    t_symbol *timeout = gensym("sys_lock_timeout");
    typedmess(pd->s_thing, timeout, 0, 0);
    return 0;
}

void sys_lock_timeout_notification () {sys_callback(sys_lock_timeout_notification_callback, 0, 0);}
