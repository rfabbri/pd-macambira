/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  scheduling stuff  */

#include "m_imp.h"

    /* LATER consider making this variable.  It's now the LCM of all sample
    rates we expect to see: 32000, 44100, 48000, 88200, 96000. */
#define TIMEUNITPERSEC (32.*441000.)

static int sys_quit;
static double sys_time;
static double sys_time_per_dsp_tick;
static double sys_time_per_msec;

int sys_usecsincelastsleep(void);
int sys_sleepgrain;

typedef void (*t_clockmethod)(void *client);

struct _clock
{
    double c_settime;
    void *c_owner;
    t_clockmethod c_fn;
    struct _clock *c_next;
};

t_clock *clock_setlist;

#ifdef UNIX
#include <unistd.h>
#endif

t_clock *clock_new(void *owner, t_method fn)
{
    t_clock *x = (t_clock *)getbytes(sizeof *x);
    x->c_settime = -1;
    x->c_owner = owner;
    x->c_fn = (t_clockmethod)fn;
    x->c_next = 0;
    return (x);
}

void clock_unset(t_clock *x)
{
    if (x->c_settime >= 0)
    {
    	if (x == clock_setlist) clock_setlist = x->c_next;
    	else
    	{
    	    t_clock *x2 = clock_setlist;
    	    while (x2->c_next != x) x2 = x2->c_next;
    	    x2->c_next = x->c_next;
    	}
    	x->c_settime = -1;
    }
}

    /* set the clock to call back at an absolute system time */
void clock_set(t_clock *x, double setticks)
{
    if (setticks < sys_time) setticks = sys_time;
    clock_unset(x);
    x->c_settime = setticks;
    if (clock_setlist && clock_setlist->c_settime <= setticks)
    {
    	t_clock *cbefore, *cafter;
    	for (cbefore = clock_setlist, cafter = clock_setlist->c_next;
    	    cbefore; cbefore = cafter, cafter = cbefore->c_next)
    	{
    	    if (!cafter || cafter->c_settime > setticks)
    	    {
    	    	cbefore->c_next = x;
    	    	x->c_next = cafter;
    	    	return;
    	    }
    	}
    }
    else x->c_next = clock_setlist, clock_setlist = x;
}

    /* set the clock to call back after a delay in msec */
void clock_delay(t_clock *x, double delaytime)
{
    clock_set(x, sys_time + sys_time_per_msec * delaytime);
}

    /* get current logical time.  We don't specify what units this is in;
    use clock_gettimesince() to measure intervals from time of this call. 
    This was previously, incorrectly named "clock_getsystime"; the old
    name is aliased to the new one in m_pd.h. */
double clock_getlogicaltime( void)
{
    return (sys_time);
}
    /* OBSOLETE NAME */
double clock_getsystime( void) { return (sys_time); }

    /* elapsed time in milliseconds since the given system time */
double clock_gettimesince(double prevsystime)
{
    return ((sys_time - prevsystime)/sys_time_per_msec);
}

    /* what value the system clock will have after a delay */
double clock_getsystimeafter(double delaytime)
{
    return (sys_time + sys_time_per_msec * delaytime);
}

void clock_free(t_clock *x)
{
    clock_unset(x);
    freebytes(x, sizeof *x);
}

/* the following routines maintain a real-execution-time histogram of the
various phases of real-time execution. */

static int sys_bin[] = {0, 2, 5, 10, 20, 30, 50, 100, 1000};
#define NBIN (sizeof(sys_bin)/sizeof(*sys_bin))
#define NHIST 10
static int sys_histogram[NHIST][NBIN];
static double sys_histtime;
static int sched_diddsp, sched_didmidi, sched_didpoll, sched_didnothing;

static void sys_clearhist( void)
{
    unsigned int i, j;
    for (i = 0; i < NHIST; i++)
    	for (j = 0; j < NBIN; j++) sys_histogram[i][j] = 0;
    sys_histtime = sys_getrealtime();
    sched_diddsp = sched_didmidi = sched_didpoll = sched_didnothing = 0;
}

void sys_printhist( void)
{
    unsigned int i, j;
    for (i = 0; i < NHIST; i++)
    {
    	int doit = 0;
    	for (j = 0; j < NBIN; j++) if (sys_histogram[i][j]) doit = 1;
    	if (doit)
    	{
    	    post("%2d %8d %8d %8d %8d %8d %8d %8d %8d", i,
    	    	sys_histogram[i][0],
    	    	sys_histogram[i][1],
    	    	sys_histogram[i][2],
    	    	sys_histogram[i][3],
    	    	sys_histogram[i][4],
    	    	sys_histogram[i][5],
    	    	sys_histogram[i][6],
    	    	sys_histogram[i][7]);
    	}
    }
    post("dsp %d, midi %d, poll %d, nothing %d",
    	sched_diddsp, sched_didmidi, sched_didpoll, sched_didnothing);
}

static int sys_histphase;

int sys_addhist(int phase)
{
    int i, j, phasewas = sys_histphase;
    double newtime = sys_getrealtime();
    int msec = (newtime - sys_histtime) * 1000.;
    for (j = NBIN-1; j >= 0; j--)
    {
    	if (msec >= sys_bin[j]) 
    	{
    	    sys_histogram[phasewas][j]++;
    	    break;
    	}
    }
    sys_histtime = newtime;
    sys_histphase = phase;
    return (phasewas);
}

#define NRESYNC 20

typedef struct _resync
{
    int r_ntick;
    int r_error;
} t_resync;

static int oss_resyncphase = 0;
static int oss_nresync = 0;
static t_resync oss_resync[NRESYNC];

#ifdef __linux__
void linux_audiostatus(void);
#endif

static char *(oss_errornames[]) = {
"unknown",
"ADC blocked",
"DAC blocked",
"A/D/A sync",
"data late"
};

void glob_audiostatus(void)
{
    int dev, nresync, nresyncphase, i;
#ifdef __linux__
    linux_audiostatus();
#endif
    nresync = (oss_nresync >= NRESYNC ? NRESYNC : oss_nresync);
    nresyncphase = oss_resyncphase - 1;
    post("audio I/O error history:");
    post("seconds ago\terror type");
    for (i = 0; i < nresync; i++)
    {
	int errtype;
	if (nresyncphase < 0)
	    nresyncphase += NRESYNC;
    	errtype = oss_resync[nresyncphase].r_error;
    	if (errtype < 0 || errtype > 4)
	    errtype = 0;
	
	post("%9.2f\t%s",
	    (sched_diddsp - oss_resync[nresyncphase].r_ntick)
	    	* ((double)DACBLKSIZE) / sys_dacsr,
	    oss_errornames[errtype]);
    	nresyncphase--;
    }
}

static int sched_diored;
static int sched_dioredtime;
static int sched_meterson;

void sys_log_error(int type)
{
    oss_resync[oss_resyncphase].r_ntick = sched_diddsp;
    oss_resync[oss_resyncphase].r_error = type;
    oss_nresync++;
    if (++oss_resyncphase == NRESYNC) oss_resyncphase = 0;
    if (type != ERR_NOTHING && !sched_diored)
    {
    	sys_vgui("pdtk_pd_dio 1\n");
	sched_diored = 1;
    }
    sched_dioredtime =
    	sched_diddsp + (int)(sys_dacsr /(double)DACBLKSIZE);
}

static int sched_lastinclip, sched_lastoutclip,
    sched_lastindb, sched_lastoutdb;

void glob_ping(t_pd *dummy);

static void sched_pollformeters( void)
{
    int inclip, outclip, indb, outdb;
    static int sched_nextmeterpolltime, sched_nextpingtime;

    	/* if there's no GUI but we're running in "realtime", here is
	where we arrange to ping the watchdog every 2 seconds. */
#ifdef UNIX
    if (sys_nogui && sys_hipriority && (sched_diddsp - sched_nextpingtime > 0))
    {
    	glob_ping(0);
	    /* ping every 2 seconds */
	sched_nextpingtime = sched_diddsp +
	    2 * (int)(sys_dacsr /(double)DACBLKSIZE);
    }
#endif

    if (sched_diddsp - sched_nextmeterpolltime < 0)
    	return;
    if (sched_diored && (sched_diddsp - sched_dioredtime > 0))
    {
    	sys_vgui("pdtk_pd_dio 0\n");
	sched_diored = 0;
    }
    if (sched_meterson)
    {
    	float inmax, outmax;
    	sys_getmeters(&inmax, &outmax);
	indb = 0.5 + rmstodb(inmax);
	outdb = 0.5 + rmstodb(outmax);
    	inclip = (inmax > 0.999);
	outclip = (outmax >= 1.0);
    }
    else
    {
    	indb = outdb = 0;
	inclip = outclip = 0;
    }
    if (inclip != sched_lastinclip || outclip != sched_lastoutclip
    	|| indb != sched_lastindb || outdb != sched_lastoutdb)
    {
    	sys_vgui("pdtk_pd_meters %d %d %d %d\n", indb, outdb, inclip, outclip);
	sched_lastinclip = inclip;
	sched_lastoutclip = outclip;
	sched_lastindb = indb;
	sched_lastoutdb = outdb;
    }
    sched_nextmeterpolltime =
    	sched_diddsp + (int)(sys_dacsr /(double)DACBLKSIZE);
}

void glob_meters(void *dummy, float f)
{
    if (f == 0)
    	sys_getmeters(0, 0);
    sched_meterson = (f != 0);
    sched_lastinclip = sched_lastoutclip = sched_lastindb = sched_lastoutdb =
	-1;
}

#if 1
void glob_foo(void *dummy, t_symbol *s, int argc, t_atom *argv)
{
    if (argc) sys_clearhist();
    else sys_printhist();
}
#endif

void dsp_tick(void);

static int m_nodacs = 0;

    /* this must be called earlier than any patches are loaded */
void m_schedsetsr( void)
{
    sys_time_per_dsp_tick =
    	(TIMEUNITPERSEC) * ((double)DACBLKSIZE) / sys_dacsr;
    sys_time_per_msec =
    	TIMEUNITPERSEC / 1000.;
}

/*
Here is Pd's "main loop."  This routine dispatches clock timeouts and DSP
"ticks" deterministically, and polls for input from MIDI and the GUI.  If
we're left idle we also poll for graphics updates; but these are considered
lower priority than the rest.

The time source is normally the audio I/O subsystem via the "sys_send_dacs()"
call.  This call returns true if samples were transferred; false means that
the audio I/O system is still bussy with previous transfers.
The sys_send_dacs call is OS dependent and is variously implemented in
s_linux.c, s_nt.c, and s_sgi.c.  
*/

void sys_pollmidiqueue( void);
void sys_initmidiqueue( void);

int m_scheduler(int nodacs)
{
    int lasttimeforward = SENDDACS_YES;
    int idlecount = 0;
    double lastdactime = 0;
    sys_clearhist();
    m_nodacs = nodacs;
    if (sys_sleepgrain < 1000)
    	sys_sleepgrain = (sys_schedadvance >= 4000? 
    	    (sys_schedadvance >> 2) : 1000);
    sys_initmidiqueue();
    while (1)
    {
    	int didsomething = 0;
    	int timeforward;

    	sys_addhist(0);
    	if (m_nodacs)
    	{
    	    double elapsed = sys_getrealtime() - lastdactime;
    	    static double next = 0;
    	    if (elapsed > next)
    	    {
    	    	timeforward = SENDDACS_YES;
    	    	next += (double)DACBLKSIZE / sys_dacsr;
    	    }
    	    else timeforward = SENDDACS_NO;
    	}
    	else
    	{
    	    timeforward = sys_send_dacs();

    		/* if dacs remain "idle" for 1 sec, they're hung up. */
    	    if (timeforward != 0) idlecount = 0;
    	    else
    	    {
    		idlecount++;
    		if (!(idlecount & 31))
    		{
    	    	    static double idletime;
    	    	    	/* on 32nd idle, start a clock watch;  every
    	    	    	32 ensuing idles, check it */
    	    	    if (idlecount == 32)
    	    	    	idletime = sys_getrealtime();
    	    	    else if (sys_getrealtime() - idletime > 1.)
    	    	    {
    	    		post("audio I/O stuck... closing audio\n");
    	    		m_nodacs = 1;
    	    		sys_close_audio();
    	    		lastdactime = sys_getrealtime();
    	    	    }
    	    	}
    	    }
    	}
    	sys_setmiditimediff(0, 1e-6 * sys_schedadvance);
	lasttimeforward = timeforward;
    	sys_addhist(1);
    	if (timeforward != SENDDACS_NO)
    	{
    	    /* time has moved forward.  Check MIDI and clocks */
    	    
    	    double next_sys_time = sys_time + sys_time_per_dsp_tick;
	    int countdown = 5000;
    	    while (clock_setlist && clock_setlist->c_settime < next_sys_time)
    	    {
    	    	t_clock *c = clock_setlist;
    	    	sys_time = c->c_settime;
    	    	clock_unset(clock_setlist);
		outlet_setstacklim();
    	    	(*c->c_fn)(c->c_owner);
		if (!countdown--)
		{
		    countdown = 5000;
		    sys_pollgui();
		}
    	    }
    	    sys_time = next_sys_time;
    	    if (sys_quit) break;
    	    dsp_tick();
    	    if (timeforward != SENDDACS_SLEPT)
	    	didsomething = 1;
	    sched_diddsp++;
    	}

    	sys_addhist(2);
    	sys_pollmidiqueue();
    	if (sys_pollgui())
	{
	    if (!didsomething)
	    	sched_didpoll++;
	    didsomething = 1;
	}
    	sys_addhist(3);
	    /* test for idle; if so, do graphics updates. */
    	if (!didsomething)
    	{
	    sched_pollformeters();
	    sys_reportidle();
    	    if (timeforward != SENDDACS_SLEPT)
	    	sys_microsleep(sys_sleepgrain);
    	    sys_addhist(5);
	    sched_didnothing++;
    	}
    }
    return (0);
}


