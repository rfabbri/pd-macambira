/* Copyright (c) 1997-1999 Guenter Geiger, Miller Puckette, Larry Troxler,
* Winfried Ritsch, Karl MacMillan, and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* this file implements the sys_ functions profiled in m_imp.h for
 audio and MIDI I/O.  In Linux there might be several APIs for doing the
 audio part; right now there are three (OSS, ALSA, RME); the third is
 for the RME 9652 driver by Ritsch (but not for the OSS compatible
 one by Geiger; for that one, OSS should work.)

 FUNCTION PREFIXES.
    sys_ -- functions which must be exported to Pd on all platforms
    linux_ -- linux-specific objects which don't depend on API,
    	mostly static but some exported.
    oss_, alsa_, rme_ -- API-specific functions, all of which are
    	static.

 ALSA SUPPORT.  If ALSA99 is defined we support ALSA 0.5x; if ALSA01,
    ALSA 0.9x.  (the naming scheme reflects the possibility of further API
    changes in the future...) We define "ALSA" for code relevant to both
    APIs.

 For MIDI, we only offer the OSS API; ALSA has to emulate OSS for us.
*/

/* OSS include (whether we're doing OSS audio or not we need this for MIDI) */


/* IOhannes:::
 * hacked this to add advanced multidevice-support
 * 1311:forum::für::umläute:2001
 */

#include <linux/soundcard.h>

#if (defined(ALSA01) || defined(ALSA99))
#define ALSA
#endif

#ifdef ALSA99
#include <sys/asoundlib.h>
#endif
#ifdef ALSA01
#include <alsa/asoundlib.h>
#endif

#include "m_imp.h"
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mman.h>

/* local function prototypes */

static void linux_close_midi( void);

static int oss_open_audio(int naudioindev, int *audioindev, int nchindev,
    int *chindev, int naudiooutdev, int *audiooutdev, int nchoutdev,
    int *choutdev, int rate); /* IOhannes */

static void oss_close_audio(void);
static int oss_send_dacs(void);
static void oss_reportidle(void);

#ifdef ALSA
typedef int16_t t_alsa_sample16;
typedef int32_t t_alsa_sample32;
#define ALSA_SAMPLEWIDTH_16 sizeof(t_alsa_sample16)
#define ALSA_SAMPLEWIDTH_32 sizeof(t_alsa_sample32)
#define ALSA_XFERSIZE16  (signed int)(sizeof(t_alsa_sample16) * DACBLKSIZE)
#define ALSA_XFERSIZE32  (signed int)(sizeof(t_alsa_sample32) * DACBLKSIZE)
#define ALSA_MAXDEV 1
#define ALSA_JITTER 1024
#define ALSA_EXTRABUFFER 2048
#define ALSA_DEFFRAGSIZE 64
#define ALSA_DEFNFRAG 12

#ifdef ALSA99
typedef struct _alsa_dev
{
    snd_pcm_t *handle;
    snd_pcm_channel_info_t info;
    snd_pcm_channel_setup_t setup;
} t_alsa_dev;

t_alsa_dev alsa_device[ALSA_MAXDEV];
static int n_alsa_dev;
static char *alsa_buf;
static int alsa_samplewidth;
#endif /* ALSA99 */

#ifdef ALSA01
typedef struct _alsa_dev
{
    snd_pcm_t *inhandle;
    snd_pcm_t *outhandle;
} t_alsa_dev;

t_alsa_dev alsa_device;
static short *alsa_buf;
static int alsa_samplewidth;
static snd_pcm_status_t* in_status;
static snd_pcm_status_t* out_status;
#endif /* ALSA01 */

#if 0    /* early alsa 0.9 beta dists had different names for these: */
#define SND_PCM_ACCESS_RW_INTERLEAVED SNDRV_PCM_ACCESS_RW_INTERLEAVED
#define SND_PCM_FORMAT_S32            SNDRV_PCM_FORMAT_S32
#define SND_PCM_FORMAT_S16            SNDRV_PCM_FORMAT_S16
#define SND_PCM_SUBFORMAT_STD         SNDRV_PCM_SUBFORMAT_STD
#endif

static int alsa_mode;
static int alsa_open_audio(int inchans, int outchans, int rate);
static void alsa_close_audio(void);
static int alsa_send_dacs(void);
static void alsa_set_params(t_alsa_dev *dev, int dir, int rate, int voices);
static void alsa_reportidle(void);
#endif /* ALSA */

#ifdef RME_HAMMERFALL
static int rme9652_open_audio(int inchans, int outchans, int rate);
static void rme9652_close_audio(void);
static int rme9652_send_dacs(void);
static void rme9652_reportidle(void);
#endif /* RME_HAMMERFALL */

/* Defines */
#define DEBUG(x) x
#define DEBUG2(x) {x;}

#define OSS_MAXCHPERDEV 32	/* max channels per OSS device */
#define OSS_MAXDEV 4    	/* maximum number of input or output devices */
#define OSS_DEFFRAGSIZE 256 	/* default log fragment size (frames) */
#define OSS_DEFAUDIOBUF 40000   /* default audiobuffer, microseconds */
#define OSS_DEFAULTCH 2
#define RME_DEFAULTCH 8     /* need this even if RME undefined */
typedef int16_t t_oss_int16;
typedef int32_t t_oss_int32;
#define OSS_MAXSAMPLEWIDTH sizeof(t_oss_int32)
#define OSS_BYTESPERCHAN(width) (DACBLKSIZE * (width)) 
#define OSS_XFERSAMPS(chans) (DACBLKSIZE* (chans))
#define OSS_XFERSIZE(chans, width) (DACBLKSIZE * (chans) * (width))

#ifdef RME_HAMMERFALL
typedef int32_t t_rme_sample;
#define RME_SAMPLEWIDTH sizeof(t_rme_sample)
#define RME_BYTESPERCHAN (DACBLKSIZE * RME_SAMPLEWIDTH)
#endif /* RME_HAMMERFALL */

/* GLOBALS */
static int linux_whichapi = API_OSS;
static int linux_inchannels;
static int linux_outchannels;
static int linux_advance_samples; /* scheduler advance in samples */
static int linux_meters;    	/* true if we're metering */
static float linux_inmax;    	/* max input amplitude */
static float linux_outmax;    	/* max output amplitude */
static int linux_fragsize = 0;	/* for block mode; block size (sample frames) */
static int linux_nfragment = 0; /* ... and number of blocks. */

#ifdef ALSA99
static int alsa_devno = 1;
#endif
#ifdef ALSA01
static char alsa_devname[512] = "hw:0,0";
static int alsa_use_plugin = 0;
#endif

/* our device handles */

typedef struct _oss_dev
{
    int d_fd;
    unsigned int d_space;   /* bytes available for writing/reading  */
    int d_bufsize;  	    /* total buffer size in blocks for this device */
    int d_dropcount;  	    /* # of buffers to drop for resync (output only) */
    unsigned int d_nchannels;	/* number of channels for this device */
    unsigned int d_bytespersamp; /* bytes per sample (2 for 16 bit, 4 for 32) */
} t_oss_dev;

static t_oss_dev linux_dacs[OSS_MAXDEV];
static t_oss_dev linux_adcs[OSS_MAXDEV];
static int linux_noutdevs = 0;
static int linux_nindevs = 0;

    /* exported variables */
int sys_schedadvance = OSS_DEFAUDIOBUF;   /* scheduler advance in microsecs */
float sys_dacsr;
int sys_hipriority = 0;
t_sample *sys_soundout;
t_sample *sys_soundin;

    /* OSS-specific private variables */
static int oss_blockmode = 1;	/* flag to use "blockmode"  */
static int oss_32bit = 0;	/* allow 23 bit transfers in OSS  */
static char ossdsp[] = "/dev/dsp%d"; 

#ifndef INT32_MAX
#define INT32_MAX 0x7fffffff
#endif
    /* don't assume we can turn all 31 bits when doing float-to-fix; 
    otherwise some audio drivers (e.g. Midiman/ALSA) wrap around. */
#define FMAX 0x7ffff000
#define CLIP32(x) (((x)>FMAX)?FMAX:((x) < -FMAX)?-FMAX:(x))


/* ------------- private routines for all APIS ------------------- */

static void linux_flush_all_underflows_to_zero(void)
{
/*
    TODO: Implement similar thing for linux (GGeiger) 

    One day we will figure this out, I hope, because it 
    costs CPU time dearly on Intel  - LT
  */
     /*    union fpc_csr f;
	   f.fc_word = get_fpc_csr();
	   f.fc_struct.flush = 1;
	   set_fpc_csr(f.fc_word);
     */
}

    /* set sample rate and channels.  Must set sample rate before "configuring"
    any devices so we know scheduler advance in samples.  */

static void linux_setsr(int sr)
{
    sys_dacsr = sr;
    linux_advance_samples = (sys_schedadvance * sys_dacsr) / (1000000.);
    if (linux_advance_samples < 3 * DACBLKSIZE)
    	linux_advance_samples = 3 * DACBLKSIZE;
}

static void linux_setch(int chin, int chout)
{
    int nblk;
    int inbytes = chin * (DACBLKSIZE*sizeof(float));
    int outbytes = chout * (DACBLKSIZE*sizeof(float));

    linux_inchannels = chin;
    linux_outchannels = chout;
    if (sys_soundin)
    	free(sys_soundin);
    sys_soundin = (t_float *)malloc(inbytes);
    memset(sys_soundin, 0, inbytes);

    if (sys_soundout)
    	free(sys_soundout);
    sys_soundout = (t_float *)malloc(outbytes);
    memset(sys_soundout, 0, outbytes);

    if (sys_verbose)
    	post("input channels = %d, output channels = %d",
    	    linux_inchannels, linux_outchannels);
}

/* ---------------- MIDI routines -------------------------- */

static int oss_nmidiin;
static int oss_midiinfd[MAXMIDIINDEV];
static int oss_nmidiout;
static int oss_midioutfd[MAXMIDIOUTDEV];

static void oss_midiout(int fd, int n)
{
    char b = n;
    if ((write(fd, (char *) &b, 1)) != 1)
    	perror("midi write");
}

#define O_MIDIFLAG O_NDELAY

void linux_open_midi(int nmidiin, int *midiinvec, int nmidiout, int *midioutvec)
{
    int i;
    for (i = 0; i < nmidiout; i++)
    	oss_midioutfd[i] = -1;
    for (i = 0, oss_nmidiin = 0; i < nmidiin; i++)
    {
    	int fd = -1, j, outdevindex = -1;
	char namebuf[80];
	int devno = midiinvec[i];
	
	for (j = 0; j < nmidiout; j++)
	    if (midioutvec[j] == midiinvec[i])
	    	outdevindex = j;
	
	    /* try to open the device for read/write. */
	if (devno == 1 && fd < 0 && outdevindex >= 0)
	{
    	    sys_setalarm(1000000);
	    fd = open("/dev/midi", O_RDWR | O_MIDIFLAG);
	    if (sys_verbose)
	    	fprintf(stderr,
		    "device 1: tried /dev/midi READ/WRITE; returned %d\n", fd);
	    if (outdevindex >= 0 && fd >= 0)
	    	oss_midioutfd[outdevindex] = fd;
	}
    	if (fd < 0 && outdevindex >= 0)
	{
    	    sys_setalarm(1000000);
	    sprintf(namebuf, "/dev/midi%2.2d", devno-1);
	    fd = open(namebuf, O_RDWR | O_MIDIFLAG);
	    if (sys_verbose)
	    	fprintf(stderr,
		    "device %d: tried %s READ/WRITE; returned %d\n",
		    	devno, namebuf, fd);
	    if (outdevindex >= 0 && fd >= 0)
	    	oss_midioutfd[outdevindex] = fd;
	}
    	if (fd < 0 && outdevindex >= 0)
	{
    	    sys_setalarm(1000000);
	    sprintf(namebuf, "/dev/midi%d", devno-1);
	    fd = open(namebuf, O_RDWR | O_MIDIFLAG);
	    if (sys_verbose)
	    	fprintf(stderr, "device %d: tried %s READ/WRITE; returned %d\n",
		    devno, namebuf, fd);
	    if (outdevindex >= 0 && fd >= 0)
	    	oss_midioutfd[outdevindex] = fd;
	}
	if (devno == 1 && fd < 0)
	{
    	    sys_setalarm(1000000);
	    fd = open("/dev/midi", O_RDONLY | O_MIDIFLAG);
	    if (sys_verbose)
	    	fprintf(stderr,
		    "device 1: tried /dev/midi READONLY; returned %d\n", fd);
	}
    	if (fd < 0)
	{
    	    sys_setalarm(1000000);
	    sprintf(namebuf, "/dev/midi%2.2d", devno-1);
	    fd = open(namebuf, O_RDONLY | O_MIDIFLAG);
	    if (sys_verbose)
	    	fprintf(stderr, "device %d: tried %s READONLY; returned %d\n",
		    devno, namebuf, fd);
	}
    	if (fd < 0)
	{
    	    sys_setalarm(1000000);
	    sprintf(namebuf, "/dev/midi%d", devno-1);
	    fd = open(namebuf, O_RDONLY | O_MIDIFLAG);
	    if (sys_verbose)
	    	fprintf(stderr, "device %d: tried %s READONLY; returned %d\n",
		    devno, namebuf, fd);
	}
	if (fd >= 0)
	    oss_midiinfd[oss_nmidiin++] = fd;	    
	else post("couldn't open MIDI input device %d", devno);
    }
    for (i = 0, oss_nmidiout = 0; i < nmidiout; i++)
    {
    	int fd = oss_midioutfd[i];
	char namebuf[80];
	int devno = midioutvec[i];
	if (devno == 1 && fd < 0)
	{
    	    sys_setalarm(1000000);
	    fd = open("/dev/midi", O_WRONLY | O_MIDIFLAG);
	    if (sys_verbose)
	    	fprintf(stderr,
		    "device 1: tried /dev/midi WRITEONLY; returned %d\n", fd);
	}
    	if (fd < 0)
	{
    	    sys_setalarm(1000000);
	    sprintf(namebuf, "/dev/midi%2.2d", devno-1);
	    fd = open(namebuf, O_WRONLY | O_MIDIFLAG);
	    if (sys_verbose)
	    	fprintf(stderr, "device %d: tried %s WRITEONLY; returned %d\n",
		    devno, namebuf, fd);
	}
    	if (fd < 0)
	{
    	    sys_setalarm(1000000);
	    sprintf(namebuf, "/dev/midi%d", devno-1);
	    fd = open(namebuf, O_WRONLY | O_MIDIFLAG);
	    if (sys_verbose)
	    	fprintf(stderr, "device %d: tried %s WRITEONLY; returned %d\n",
		    devno, namebuf, fd);
	}
	if (fd >= 0)
	    oss_midioutfd[oss_nmidiout++] = fd;	    
	else post("couldn't open MIDI output device %d", devno);
    }

    if (oss_nmidiin < nmidiin || oss_nmidiout < nmidiout || sys_verbose)
    	post("opened %d MIDI input device(s) and %d MIDI output device(s).",
	    oss_nmidiin, oss_nmidiout);
}

#define md_msglen(x) (((x)<0xC0)?2:((x)<0xE0)?1:((x)<0xF0)?2:\
    ((x)==0xF2)?2:((x)<0xF4)?1:0)

void sys_putmidimess(int portno, int a, int b, int c)
{
    if (portno >= 0 && portno < oss_nmidiout)
    {
       switch (md_msglen(a))
       {
       case 2:
	    oss_midiout(oss_midioutfd[portno],a);	 
	    oss_midiout(oss_midioutfd[portno],b);	 
	    oss_midiout(oss_midioutfd[portno],c);
	    return;
       case 1:
	    oss_midiout(oss_midioutfd[portno],a);	 
	    oss_midiout(oss_midioutfd[portno],b);	 
	    return;
       case 0:
	    oss_midiout(oss_midioutfd[portno],a);	 
	    return;
       };
    }
}

void sys_putmidibyte(int portno, int byte)
{
    if (portno >= 0 && portno < oss_nmidiout)
    	oss_midiout(oss_midioutfd[portno], byte);	
}

#if 0	/* this is the "select" version which doesn't work with OSS
    	driver for emu10k1 (it doesn't implement select.) */
void sys_poll_midi(void)
{
    int i, throttle = 100;
    struct timeval timout;
    int did = 1, maxfd = 0;
    while (did)
    {
	fd_set readset, writeset, exceptset;
    	did = 0;
	if (throttle-- < 0)
	    break;
	timout.tv_sec = 0;
	timout.tv_usec = 0;

	FD_ZERO(&writeset);
	FD_ZERO(&readset);
	FD_ZERO(&exceptset);
	for (i = 0; i < oss_nmidiin; i++)
	{
	    if (oss_midiinfd[i] > maxfd)
	    	maxfd = oss_midiinfd[i];
    	    FD_SET(oss_midiinfd[i], &readset);
	}
	select(maxfd+1, &readset, &writeset, &exceptset, &timout);
	for (i = 0; i < oss_nmidiin; i++)
    	    if (FD_ISSET(oss_midiinfd[i], &readset))
	{
	    char c;
	    int ret = read(oss_midiinfd[i], &c, 1);
    	    if (ret <= 0)
	    	fprintf(stderr, "Midi read error\n");
    	    else sys_midibytein(i, (c & 0xff));
	    did = 1;
	}
    }
}
#else 

    /* this version uses the asynchronous "read()" ... */
void sys_poll_midi(void)
{
    int i, throttle = 100;
    struct timeval timout;
    int did = 1, maxfd = 0;
    while (did)
    {
	fd_set readset, writeset, exceptset;
    	did = 0;
	if (throttle-- < 0)
	    break;
	for (i = 0; i < oss_nmidiin; i++)
	{
	    char c;
	    int ret = read(oss_midiinfd[i], &c, 1);
    	    if (ret < 0)
	    {
	    	if (errno != EAGAIN)
		    perror("MIDI");
	    }
	    else if (ret != 0)
	    {
    	    	sys_midibytein(i, (c & 0xff));
	    	did = 1;
	    }
	}
    }
}
#endif

void linux_close_midi()
{
    int i;
    for (i = 0; i < oss_nmidiin; i++)
    	close(oss_midiinfd[i]);
    for (i = 0; i < oss_nmidiout; i++)
    	close(oss_midioutfd[i]);
    oss_nmidiin = oss_nmidiout = 0;
}

#define MAXAUDIODEV 4
#define DEFAULTINDEV 1
#define DEFAULTOUTDEV 1

/* ----------------------- public routines ----------------------- */
void sys_listdevs( void)
{
    post("device listing not implemented in Linux yet\n");
}

void sys_open_audio(int naudioindev, int *audioindev, int nchindev,
    int *chindev, int naudiooutdev, int *audiooutdev, int nchoutdev,
    int *choutdev, int rate)
{ /* IOhannes */
    int i, *ip;
    int defaultchannels =
    	(linux_whichapi == API_RME ? RME_DEFAULTCH : OSS_DEFAULTCH);
    if (rate < 1)
    	rate=44100;

    if (naudioindev == -1)
    { 	    	/* not set */
	if (nchindev==-1)
	{
	    nchindev=1;
	    chindev[0]=defaultchannels;
	    naudioindev=1;
	    audioindev[0] = DEFAULTINDEV;
	}
	else
	{
	    for (i = 0; i < MAXAUDIODEV; i++)
      	        audioindev[i]=i+1;
	    naudioindev = nchindev;
	}
    }
    else
    {
	if (nchindev == -1)
	{
	    nchindev = naudioindev;
	    for (i = 0; i < naudioindev; i++)
	    	chindev[i] = defaultchannels;
	}
	else if (nchindev > naudioindev)
	{
	    for (i = naudioindev; i < nchindev; i++)
	    {
	    	if (i == 0)
		    audioindev[0] = DEFAULTINDEV;
		else audioindev[i] = audioindev[i-1] + 1;
    	    }
	    naudioindev = nchindev;
	}
	else if (nchindev < naudioindev)
	{
	    for (i = nchindev; i < naudioindev; i++)
	    {
	    	if (i == 0)
		    chindev[0] = defaultchannels;
		else chindev[i] = chindev[i-1];
    	    }
	    naudioindev = nchindev;
	}
    }

    if (naudiooutdev == -1)
    { 	    	/* not set */
	if (nchoutdev==-1)
	{
	    nchoutdev=1;
	    choutdev[0]=defaultchannels;
	    naudiooutdev=1;
	    audiooutdev[0] = DEFAULTOUTDEV;
	}
	else
	{
	    for (i = 0; i < MAXAUDIODEV; i++)
      	        audiooutdev[i] = i+1;
	    naudiooutdev = nchoutdev;
	}
    }
    else
    {
	if (nchoutdev == -1)
	{
	    nchoutdev = naudiooutdev;
	    for (i = 0; i < naudiooutdev; i++)
	    	choutdev[i] = defaultchannels;
	}
	else if (nchoutdev > naudiooutdev)
	{
	    for (i = naudiooutdev; i < nchoutdev; i++)
	    {
	    	if (i == 0)
		    audiooutdev[0] = DEFAULTOUTDEV;
		else audiooutdev[i] = audiooutdev[i-1] + 1;
    	    }
	    naudiooutdev = nchoutdev;
	}
	else if (nchoutdev < naudiooutdev)
	{
	    for (i = nchoutdev; i < naudiooutdev; i++)
	    {
	    	if (i == 0)
		    choutdev[0] = defaultchannels;
		else choutdev[i] = choutdev[i-1];
    	    }
	    naudiooutdev = nchoutdev;
	}
    }

    linux_flush_all_underflows_to_zero();
#ifdef ALSA
  if (linux_whichapi == API_ALSA)
    alsa_open_audio((naudioindev > 0 ? chindev[0] : 0),
      	(naudiooutdev > 0 ? choutdev[0] : 0), rate);
  else 
#endif
#ifdef RME_HAMMERFALL
    if (linux_whichapi == API_RME)
      rme9652_open_audio((naudioindev > 0 ? chindev[0] : 0),
      	(naudiooutdev > 0 ? choutdev[0] : 0), rate);
    else 
#endif
    	oss_open_audio(naudioindev, audioindev, nchindev, chindev,
    	    naudiooutdev, audiooutdev, nchoutdev, choutdev, rate);
}

void sys_close_audio(void)
{
    /* set timeout to avoid hanging close() call */

    sys_setalarm(1000000);

#ifdef ALSA 
    if (linux_whichapi == API_ALSA)
     	alsa_close_audio();
    else
#endif
#ifdef RME_HAMMERFALL 
    if (linux_whichapi == API_RME)
     	rme9652_close_audio();
    else
#endif
    	oss_close_audio();

    sys_setalarm(0);
}

void sys_open_midi(int nmidiin, int *midiinvec,
    int nmidiout, int *midioutvec)
{
    linux_open_midi(nmidiin, midiinvec, nmidiout, midioutvec);
}

void sys_close_midi( void)
{
    sys_setalarm(1000000);
    linux_close_midi();
    sys_setalarm(0);
}

int sys_send_dacs(void)
{
    if (linux_meters)
    {
    	int i, n;
	float maxsamp;
	for (i = 0, n = linux_inchannels * DACBLKSIZE, maxsamp = linux_inmax;
	    i < n; i++)
	{
	    float f = sys_soundin[i];
	    if (f > maxsamp) maxsamp = f;
	    else if (-f > maxsamp) maxsamp = -f;
	}
	linux_inmax = maxsamp;
	for (i = 0, n = linux_outchannels * DACBLKSIZE, maxsamp = linux_outmax;
	    i < n; i++)
	{
	    float f = sys_soundout[i];
	    if (f > maxsamp) maxsamp = f;
	    else if (-f > maxsamp) maxsamp = -f;
	}
	linux_outmax = maxsamp;
    }
#ifdef ALSA 
    if (linux_whichapi == API_ALSA)
    	return alsa_send_dacs();
#endif
#ifdef RME_HAMMERFALL
    if (linux_whichapi == API_RME)
    	return rme9652_send_dacs();
#endif
    return oss_send_dacs();
}

float sys_getsr(void)
{
     return (sys_dacsr);
}

int sys_get_outchannels(void)
{
     return (linux_outchannels); 
}

int sys_get_inchannels(void) 
{
     return (linux_inchannels);
}

void sys_audiobuf(int n)
{
     /* set the size, in milliseconds, of the audio FIFO */
     if (n < 5) n = 5;
     else if (n > 5000) n = 5000;
     sys_schedadvance = n * 1000;
}

void sys_getmeters(float *inmax, float *outmax)
{
    if (inmax)
    {
    	linux_meters = 1;
	*inmax = linux_inmax;
	*outmax = linux_outmax;
    }
    else
    	linux_meters = 0;
    linux_inmax = linux_outmax = 0;
}

void sys_reportidle(void)
{
}

void sys_set_priority(int higher) 
{
    struct sched_param par;
    int p1 ,p2, p3;
#ifdef _POSIX_PRIORITY_SCHEDULING

    p1 = sched_get_priority_min(SCHED_FIFO);
    p2 = sched_get_priority_max(SCHED_FIFO);
    p3 = (higher ? p2 - 1 : p2 - 3);
    par.sched_priority = p3;

    if (sched_setscheduler(0,SCHED_FIFO,&par) != -1)
       fprintf(stderr, "priority %d scheduling enabled.\n", p3);
#endif

#ifdef _POSIX_MEMLOCK
    if (mlockall(MCL_FUTURE) != -1) 
    	fprintf(stderr, "memory locking enabled.\n");
#endif
}

void sys_setblocksize(int n)
{
    if (n < 1)
    	n = 1;
    linux_fragsize = n;
    oss_blockmode = 1;
}

/* ------------ linux-specific command-line flags -------------- */

void linux_setfrags(int n)
{
    linux_nfragment = n;
    oss_blockmode = 1;
}

void linux_streammode( void)
{
    oss_blockmode = 0;
}

void linux_32bit( void)
{
    oss_32bit = 1;
}

void linux_set_sound_api(int which)
{
     linux_whichapi = which;
     if (sys_verbose)
     	post("linux_whichapi %d", linux_whichapi);
}

#ifdef ALSA99
void linux_alsa_devno(int devno)
{
    alsa_devno = devno;
}

#endif

#ifdef ALSA01
void linux_alsa_devname(char *devname)
{
    strncpy(alsa_devname, devname, 511);
}

void linux_alsa_use_plugin(int t)
{
  if (t == 1)
    alsa_use_plugin = 1;
  else
    alsa_use_plugin = 0;
}

#endif

/* -------------- Audio I/O using the OSS API ------------------ */ 

typedef struct _multidev {
     int fd;
     int channels;
     int format;
} t_multidev;

int oss_reset(int fd) {
     int err;
     if ((err = ioctl(fd,SNDCTL_DSP_RESET)) < 0)
	  error("OSS: Could not reset");
     return err;
}

    /* The AFMT_S32_BLOCKED format is not defined in standard linux kernels
    but is proposed by Guenter Geiger to support extending OSS to handle
    32 bit sample.  This is user in Geiger's OSS driver for RME Hammerfall.
    I'm not clear why this isn't called AFMT_S32_[SLN]E... */

#ifndef AFMT_S32_BLOCKED
#define AFMT_S32_BLOCKED 0x0000400
#endif

void oss_configure(t_oss_dev *dev, int srate, int dac, int skipblocksize)
{ /* IOhannes */
    int orig, param, nblk, fd = dev->d_fd, wantformat;
    int nchannels = dev->d_nchannels;
    int advwas = sys_schedadvance;

    audio_buf_info ainfo;

    /* IOhannes : 
     * pd is very likely to crash if different formats are used on
     multiple soundcards
     */

    	/* set resolution - first try 4 byte samples */
    if (oss_32bit && (ioctl(fd,SNDCTL_DSP_GETFMTS,&param) >= 0) &&
    	(param & AFMT_S32_BLOCKED))
    {
    	wantformat = AFMT_S32_BLOCKED;
	dev->d_bytespersamp = 4;
    }
    else
    {
    	wantformat = AFMT_S16_NE;
	dev->d_bytespersamp = 2;
    }
    param = wantformat;

    if (sys_verbose)
    	post("bytes per sample = %d", dev->d_bytespersamp);
    if (ioctl(fd, SNDCTL_DSP_SETFMT, &param) == -1)
    	fprintf(stderr,"OSS: Could not set DSP format\n");
    else if (wantformat != param)
    	fprintf(stderr,"OSS: DSP format: wanted %d, got %d\n",
	    wantformat, param);

    /* sample rate */
    orig = param = srate; 
    if (ioctl(fd, SNDCTL_DSP_SPEED, &param) == -1)
    	fprintf(stderr,"OSS: Could not set sampling rate for device\n");
    else if( orig != param )
    	fprintf(stderr,"OSS: sampling rate: wanted %d, got %d\n",
	    orig, param );				

    if (oss_blockmode && !skipblocksize)
    {
    	int fragbytes, logfragsize, nfragment;
    	    /* setting fragment count and size.  */
	if (linux_nfragment)   /* if nfrags specified, take literally */
	{
	    nfragment = linux_nfragment;
	    if (!linux_fragsize)
	    	linux_fragsize = OSS_DEFFRAGSIZE;
	    sys_schedadvance = ((nfragment * linux_fragsize) * 1.e6)
	    	/ (float)srate;
	    linux_setsr(srate);
	}
	else
	{
	    if (!linux_fragsize)
	    {
	    	linux_fragsize = OSS_DEFFRAGSIZE;
		while (linux_fragsize > DACBLKSIZE
		    && linux_fragsize * 4 > linux_advance_samples)
		    	linux_fragsize = linux_fragsize/2;
	    }
	    /* post("adv_samples %d", linux_advance_samples); */
	    nfragment = (sys_schedadvance * (44100. * 1.e-6)) / linux_fragsize;
	}
    	fragbytes = linux_fragsize * (dev->d_bytespersamp * nchannels);
	logfragsize = ilog2(fragbytes);

    	if (fragbytes != (1 << logfragsize))
	    post("warning: OSS takes only power of 2 blocksize; using %d",
	    	(1 << logfragsize)/(dev->d_bytespersamp * nchannels));
    	if (sys_verbose)
	    post("setting nfrags = %d, fragsize %d\n", nfragment, fragbytes);

    	param = orig = (nfragment<<16) + logfragsize;
    	if (ioctl(fd,SNDCTL_DSP_SETFRAGMENT, &param) == -1)
    	    error("OSS: Could not set or read fragment size\n");
	if (param != orig)
	{
	    nfragment = ((param >> 16) & 0xffff);
	    logfragsize = (param & 0xffff);
	    post("warning: actual fragments %d, blocksize %d",
	    	nfragment, (1 << logfragsize));
	}
	if (sys_verbose)
	    post("audiobuffer set to %d msec", (int)(0.001 * sys_schedadvance));
    }

    if (dac)
    {
	/* use "free space" to learn the buffer size.  Normally you
	should set this to your own desired value; but this seems not
	to be implemented uniformly across different sound cards.  LATER
	we should figure out what to do if the requested scheduler advance
	is greater than this buffer size; for now, we just print something
	out.  */

    	int defect;
	if (ioctl(fd, SOUND_PCM_GETOSPACE,&ainfo) < 0)
	   fprintf(stderr,"OSS: ioctl on output device failed");
	dev->d_bufsize = ainfo.bytes;

    	defect = linux_advance_samples * (dev->d_bytespersamp * nchannels)
	    - dev->d_bufsize - OSS_XFERSIZE(nchannels, dev->d_bytespersamp);
	if (defect > 0)
	{
	    if (sys_verbose || defect > (dev->d_bufsize >> 2))
	    	fprintf(stderr,
	    	    "OSS: requested audio buffer size %d limited to %d\n",
		    	linux_advance_samples * (dev->d_bytespersamp * nchannels),
		    	dev->d_bufsize);
    	    linux_advance_samples =
	    	(dev->d_bufsize - OSS_XFERSAMPS(nchannels)) /
		    (dev->d_bytespersamp *nchannels);
	}
    }
}

static int oss_setchannels(int fd, int wantchannels, char *devname)
{ /* IOhannes */
    int param = wantchannels;

    while (param>1) {
      int save = param;
      if (ioctl(fd, SNDCTL_DSP_CHANNELS, &param) == -1) {
	error("OSS: SNDCTL_DSP_CHANNELS failed %s",devname);
      } else {
	if (param == save) return (param);
      }
      param=save-1;
    }

    return (0);
}

#define O_AUDIOFLAG 0 /* O_NDELAY */

int oss_open_audio(int nindev,  int *indev,  int nchin,  int *chin,
    int noutdev, int *outdev, int nchout, int *chout, int rate)
{ /* IOhannes */
    int capabilities = 0;
    int inchannels = 0, outchannels = 0;
    char devname[20];
    int n, i, fd;
    char buf[OSS_MAXSAMPLEWIDTH * DACBLKSIZE * OSS_MAXCHPERDEV];
    int num_devs = 0;
    int wantmore=0;
    int spread = 0;
    audio_buf_info ainfo;

    linux_nindevs = linux_noutdevs = 0;

    /* set logical sample rate amd calculate linux_advance_samples.  */
    linux_setsr(rate);

    	/* mark input devices unopened */
    for (i = 0; i < OSS_MAXDEV; i++)
    	linux_adcs[i].d_fd = -1;

    /* open output devices */
    wantmore=0;
    if (noutdev < 0 || nindev < 0)
    	bug("linux_open_audio");

    for (n = 0; n < noutdev; n++)
    {
	int gotchans, j, inindex = -1;
	int thisdevice=outdev[n];
	int wantchannels = (nchout>n) ? chout[n] : wantmore;
    	fd = -1;
	if (!wantchannels)
	    goto end_out_loop;

	if (thisdevice > 1)
	    sprintf(devname, "/dev/dsp%d", thisdevice-1);
	else sprintf(devname, "/dev/dsp");
    	
	    /* search for input request for same device.  Succeed only
	    if the number of channels matches. */
	for (j = 0; j < nindev; j++)
	    if (indev[j] == thisdevice && chin[j] == wantchannels)
	    	inindex = j;
	
    	    /* if the same device is requested for input and output,
	    try to open it read/write */
	if (inindex >= 0)
	{
	    sys_setalarm(1000000);
	    if ((fd = open(devname, O_RDWR | O_AUDIOFLAG)) == -1)
	    {
	    	post("%s (read/write): %s", devname, strerror(errno));
		post("(now will try write-only...)");
	    }
	    else
	    {
	    	if (sys_verbose)
	    	    post("opened %s for reading and writing\n", devname);
	    	linux_adcs[inindex].d_fd = fd;
	    }
	}
	    /* if that didn't happen or if it failed, try write-only */
	if (fd == -1)
	{
	    sys_setalarm(1000000);
	    if ((fd = open(devname, O_WRONLY | O_AUDIOFLAG)) == -1)
	    {
		post("%s (writeonly): %s",
		     devname, strerror(errno));
		break;
	    }
	    if (sys_verbose)
	    	post("opened %s for writing only\n", devname);
    	}
	if (ioctl(fd, SNDCTL_DSP_GETCAPS, &capabilities) == -1)
	    error("OSS: SNDCTL_DSP_GETCAPS failed %s", devname);

	gotchans = oss_setchannels(fd,
	    (wantchannels>OSS_MAXCHPERDEV)?OSS_MAXCHPERDEV:wantchannels,
		    devname);

	if (sys_verbose)
	    post("opened audio output on %s; got %d channels",
	    	 devname, gotchans);

	if (gotchans < 2)
	{
	    	/* can't even do stereo? just give up. */
	    close(fd);
	}
	else
	{
	    linux_dacs[linux_noutdevs].d_nchannels = gotchans;
	    linux_dacs[linux_noutdevs].d_fd = fd;
	    oss_configure(linux_dacs+linux_noutdevs, rate, 1, 0);

	    linux_noutdevs++;
	    outchannels += gotchans;
	    if (inindex >= 0)
	    {
	    	linux_adcs[inindex].d_nchannels = gotchans;
	    	chin[inindex] = gotchans;
	    }
	}
	/* LATER think about spreading large numbers of channels over
	    various dsp's and vice-versa */
	wantmore = wantchannels - gotchans;
    end_out_loop: ;
    }

    /* open input devices */
    wantmore = 0;
    if (nindev==-1)
    	nindev=4; /* spread channels over default-devices */
    for (n = 0; n < nindev; n++)
    {
	int gotchans=0;
	int thisdevice=indev[n];
	int wantchannels = (nchin>n)?chin[n]:wantmore;
	int alreadyopened = 0;
	if (!wantchannels)
	    goto end_in_loop;

	if (thisdevice > 1)
	    sprintf(devname, "/dev/dsp%d", thisdevice - 1);
	else sprintf(devname, "/dev/dsp");

	sys_setalarm(1000000);

    	    /* perhaps it's already open from the above? */
    	if (linux_dacs[n].d_fd >= 0)
	{
	    fd = linux_dacs[n].d_fd;
	    alreadyopened = 1;
	}
	else
	{
	    	/* otherwise try to open it here. */
	    if ((fd = open(devname, O_RDONLY | O_AUDIOFLAG)) == -1)
	    {
		post("%s (readonly): %s", devname, strerror(errno));
		goto end_in_loop;
	    }
	    if (sys_verbose)
	    	post("opened %s for reading only\n", devname);
    	}
	linux_adcs[linux_nindevs].d_fd = fd;
	gotchans = oss_setchannels(fd,
	    (wantchannels>OSS_MAXCHPERDEV)?OSS_MAXCHPERDEV:wantchannels,
	    	devname);
	if (sys_verbose)
	post("opened audio input device %s; got %d channels",
	     devname, gotchans);

	if (gotchans < 1)
	{
	    close(fd);
	    goto end_in_loop;
	}

	linux_adcs[linux_nindevs].d_nchannels = gotchans;
	
	oss_configure(linux_adcs+linux_nindevs, rate, 0, alreadyopened);

	inchannels += gotchans;
	linux_nindevs++;

	wantmore = wantchannels-gotchans;
	/* LATER think about spreading large numbers of channels over
	    various dsp's and vice-versa */
    end_in_loop: ;
    }

    linux_setch(inchannels, outchannels);

    /* We have to do a read to start the engine. This is 
       necessary because sys_send_dacs waits until the input
       buffer is filled and only reads on a filled buffer.
       This is good, because it's a way to make sure that we
       will not block.  But I wonder why we only have to read
       from one of the devices and not all of them??? */

    if (linux_nindevs)
    {
    	if (sys_verbose)
	    fprintf(stderr,("OSS: issuing first ADC 'read' ... "));
	read(linux_adcs[0].d_fd, buf,
	    linux_adcs[0].d_bytespersamp *
	    	linux_adcs[0].d_nchannels * DACBLKSIZE);
    	if (sys_verbose)
	    fprintf(stderr, "...done.\n");
    }
    sys_setalarm(0);
    return (0);
}

void oss_close_audio( void)
{
     int i;
     for (i=0;i<linux_nindevs;i++)
	  close(linux_adcs[i].d_fd);

     for (i=0;i<linux_noutdevs;i++)
          close(linux_dacs[i].d_fd);

    linux_nindevs = linux_noutdevs = 0;
}

static int linux_dacs_write(int fd,void* buf,long bytes)
{
    return write(fd, buf, bytes);
}

static int linux_adcs_read(int fd,void*  buf,long bytes)
{
     return read(fd, buf, bytes);
}

    /* query audio devices for "available" data size. */
static void oss_calcspace(void)
{
    int dev;
    audio_buf_info ainfo;
    for (dev=0; dev < linux_noutdevs; dev++)
    {
	if (ioctl(linux_dacs[dev].d_fd, SOUND_PCM_GETOSPACE, &ainfo) < 0)
	   fprintf(stderr,"OSS: ioctl on output device %d failed",dev);
	linux_dacs[dev].d_space = ainfo.bytes;
    }

    for (dev = 0; dev < linux_nindevs; dev++)
    {
	if (ioctl(linux_adcs[dev].d_fd, SOUND_PCM_GETISPACE,&ainfo) < 0)
	    fprintf(stderr, "OSS: ioctl on input device %d, fd %d failed",
    	    	dev, linux_adcs[dev].d_fd);
	linux_adcs[dev].d_space = ainfo.bytes;
    }
}

void linux_audiostatus(void)
{
    int dev;
    if (!oss_blockmode)
    {
	oss_calcspace();
	for (dev=0; dev < linux_noutdevs; dev++)
	    fprintf(stderr, "dac %d space %d\n", dev, linux_dacs[dev].d_space);

	for (dev = 0; dev < linux_nindevs; dev++)
	    fprintf(stderr, "adc %d space %d\n", dev, linux_adcs[dev].d_space);

    }
}

/* this call resyncs audio output and input which will cause discontinuities
in audio output and/or input. */ 

static void oss_doresync( void)
{
    int dev, zeroed = 0, wantsize;
    char buf[OSS_MAXSAMPLEWIDTH * DACBLKSIZE * OSS_MAXCHPERDEV];
    audio_buf_info ainfo;

    	/* 1. if any input devices are ahead (have more than 1 buffer stored),
	    drop one or more buffers worth */
    for (dev = 0; dev < linux_nindevs; dev++)
    {
    	if (linux_adcs[dev].d_space == 0)
	{
	    linux_adcs_read(linux_adcs[dev].d_fd, buf,
	    	OSS_XFERSIZE(linux_adcs[dev].d_nchannels,
		    linux_adcs[dev].d_bytespersamp));
	}
	else while (linux_adcs[dev].d_space >
	    OSS_XFERSIZE(linux_adcs[dev].d_nchannels,
	    	linux_adcs[dev].d_bytespersamp))
	{
    	    linux_adcs_read(linux_adcs[dev].d_fd, buf, 
	    	OSS_XFERSIZE(linux_adcs[dev].d_nchannels,
		    linux_adcs[dev].d_bytespersamp));
	    if (ioctl(linux_adcs[dev].d_fd, SOUND_PCM_GETISPACE, &ainfo) < 0)
	    {
	    	fprintf(stderr, "OSS: ioctl on input device %d, fd %d failed",
    	    	    dev, linux_adcs[dev].d_fd);
	    	break;
	    }
	    linux_adcs[dev].d_space = ainfo.bytes;
	}
    }

    	/* 2. if any output devices are behind, feed them zeros to catch them
	    up */
    for (dev = 0; dev < linux_noutdevs; dev++)
    {
    	while (linux_dacs[dev].d_space > linux_dacs[dev].d_bufsize - 
	    linux_advance_samples * (linux_dacs[dev].d_nchannels *
	    	linux_dacs[dev].d_bytespersamp))
	{
    	    if (!zeroed)
	    {
	    	unsigned int i;
		for (i = 0; i < OSS_XFERSAMPS(linux_dacs[dev].d_nchannels);
		    i++)    
		    	buf[i] = 0;
		zeroed = 1;
	    }
    	    linux_dacs_write(linux_dacs[dev].d_fd, buf,
	    	OSS_XFERSIZE(linux_dacs[dev].d_nchannels,
		    linux_dacs[dev].d_bytespersamp));
	    if (ioctl(linux_dacs[dev].d_fd, SOUND_PCM_GETOSPACE, &ainfo) < 0)
	    {
	    	fprintf(stderr, "OSS: ioctl on output device %d, fd %d failed",
    	    	    dev, linux_dacs[dev].d_fd);
	    	break;
	    }
	    linux_dacs[dev].d_space = ainfo.bytes;
	}
    }
    	/* 3. if any DAC devices are too far ahead, plan to drop the
	    number of frames which will let the others catch up. */
    for (dev = 0; dev < linux_noutdevs; dev++)
    {
    	if (linux_dacs[dev].d_space > linux_dacs[dev].d_bufsize - 
	    (linux_advance_samples - 1) * linux_dacs[dev].d_nchannels *
	    	linux_dacs[dev].d_bytespersamp)
	{
    	    linux_dacs[dev].d_dropcount = linux_advance_samples - 1 - 
	    	(linux_dacs[dev].d_space - linux_dacs[dev].d_bufsize) /
		     (linux_dacs[dev].d_nchannels *
		     	linux_dacs[dev].d_bytespersamp) ;
	}
	else linux_dacs[dev].d_dropcount = 0;
    }
}

int oss_send_dacs(void)
{
    float *fp1, *fp2;
    long fill;
    int i, j, dev, rtnval = SENDDACS_YES;
    char buf[OSS_MAXSAMPLEWIDTH * DACBLKSIZE * OSS_MAXCHPERDEV];
    t_oss_int16 *sp;
    t_oss_int32 *lp;
    	/* the maximum number of samples we should have in the ADC buffer */
    int idle = 0;
    int thischan;
    double timeref, timenow;

    if (!linux_nindevs && !linux_noutdevs)
    	return (SENDDACS_NO);

    if (!oss_blockmode)
    {
    	/* determine whether we're idle.  This is true if either (1)
    	some input device has less than one buffer to read or (2) some
	output device has fewer than (linux_advance_samples) blocks buffered
	already. */
    	oss_calcspace();
    
	for (dev=0; dev < linux_noutdevs; dev++)
	    if (linux_dacs[dev].d_dropcount ||
		(linux_dacs[dev].d_bufsize - linux_dacs[dev].d_space >
	    	    linux_advance_samples * linux_dacs[dev].d_bytespersamp * 
			linux_dacs[dev].d_nchannels))
    	    	    	    idle = 1;
	for (dev=0; dev < linux_nindevs; dev++)
	    if (linux_adcs[dev].d_space <
	    	OSS_XFERSIZE(linux_adcs[dev].d_nchannels,
		    linux_adcs[dev].d_bytespersamp))
		    	idle = 1;
    }
    
    if (idle && !oss_blockmode)
    {
    	    /* sometimes---rarely---when the ADC available-byte-count is
	    zero, it's genuine, but usually it's because we're so
	    late that the ADC has overrun its entire kernel buffer.  We
	    distinguish between the two by waiting 2 msec and asking again.
	    There should be an error flag we could check instead; look for this
    	    someday... */
    	for (dev = 0;dev < linux_nindevs; dev++) 
	    if (linux_adcs[dev].d_space == 0)
	{
    	    audio_buf_info ainfo;
	    sys_microsleep(2000);
	    oss_calcspace();
	    if (linux_adcs[dev].d_space != 0) continue;

    	    	/* here's the bad case.  Give up and resync. */
	    sys_log_error(ERR_DATALATE);
	    oss_doresync();
	    return (SENDDACS_NO);
	}
    	    /* check for slippage between devices, either because
	    data got lost in the driver from a previous late condition, or
	    because the devices aren't synced.  When we're idle, no
	    input device should have more than one buffer readable and
	    no output device should have less than linux_advance_samples-1
	    */
	    
	for (dev=0; dev < linux_noutdevs; dev++)
	    if (!linux_dacs[dev].d_dropcount &&
		(linux_dacs[dev].d_bufsize - linux_dacs[dev].d_space <
	    	    (linux_advance_samples - 2) *
		    	(linux_dacs[dev].d_bytespersamp *
			    linux_dacs[dev].d_nchannels)))
    	    		goto badsync;
	for (dev=0; dev < linux_nindevs; dev++)
	    if (linux_adcs[dev].d_space > 3 *
	    	OSS_XFERSIZE(linux_adcs[dev].d_nchannels,
		    linux_adcs[dev].d_bytespersamp))
		    	goto badsync;

	    /* return zero to tell the scheduler we're idle. */
    	return (SENDDACS_NO);
    badsync:
	sys_log_error(ERR_RESYNC);
	oss_doresync();
	return (SENDDACS_NO);
    	
    }

	/* do output */

    timeref = sys_getrealtime();
    for (dev=0, thischan = 0; dev < linux_noutdevs; dev++)
    {
    	int nchannels = linux_dacs[dev].d_nchannels;
    	if (linux_dacs[dev].d_dropcount)
	    linux_dacs[dev].d_dropcount--;
	else
	{
	    if (linux_dacs[dev].d_bytespersamp == 4)
	    {
		for (i = DACBLKSIZE * nchannels,  fp1 = sys_soundout +	
	    	    DACBLKSIZE*thischan,
	    	    lp = (t_oss_int32 *)buf; i--; fp1++, lp++)
		{
		    float f = *fp1 * 2147483648.;
		    *lp = (f >= 2147483647. ? 2147483647. : 
			(f < -2147483648. ? -2147483648. : f));
		}
	    }
	    else
	    {
		for (i = DACBLKSIZE,  fp1 = sys_soundout +	
	    	    DACBLKSIZE*thischan,
	    	    sp = (t_oss_int16 *)buf; i--; fp1++, sp += nchannels)
		{
		    for (j=0, fp2 = fp1; j<nchannels; j++, fp2 += DACBLKSIZE)
		    {
	    		int s = *fp2 * 32767.;
			if (s > 32767) s = 32767;
			else if (s < -32767) s = -32767;
	    		sp[j] = s;
		    }
		}
	    }
	    linux_dacs_write(linux_dacs[dev].d_fd, buf,
	    	OSS_XFERSIZE(nchannels, linux_dacs[dev].d_bytespersamp));
	    if ((timenow = sys_getrealtime()) - timeref > 0.002)
	    {
	    	if (!oss_blockmode)
		    sys_log_error(ERR_DACSLEPT);
		else rtnval = SENDDACS_SLEPT;
    	    }
	    timeref = timenow;
    	}
	thischan += nchannels;
    }
    memset(sys_soundout, 0,
    	linux_outchannels * (sizeof(float) * DACBLKSIZE));

    	/* do input */

    for (dev = 0, thischan = 0; dev < linux_nindevs; dev++)
    {
    	int nchannels = linux_adcs[dev].d_nchannels;
    	linux_adcs_read(linux_adcs[dev].d_fd, buf,
	    OSS_XFERSIZE(nchannels, linux_adcs[dev].d_bytespersamp));

	if ((timenow = sys_getrealtime()) - timeref > 0.002)
	{
	    if (!oss_blockmode)
	    	sys_log_error(ERR_ADCSLEPT);
	    else
	    	rtnval = SENDDACS_SLEPT;
    	}
	timeref = timenow;

	if (linux_adcs[dev].d_bytespersamp == 4)
	{
	    for (i = DACBLKSIZE*nchannels,
	    	fp1 = sys_soundin + thischan*DACBLKSIZE,
		    lp = (t_oss_int32 *)buf; i--; fp1++, lp++)
	    {
    	    	*fp1 = ((float)(*lp))*(float)(1./2147483648.);
	    }
    	}
	else
	{
	    for (i = DACBLKSIZE,fp1 = sys_soundin + thischan*DACBLKSIZE,
		sp = (t_oss_int16 *)buf; i--; fp1++, sp += nchannels)
	    {
    		for (j=0;j<linux_inchannels;j++)
    	    	    fp1[j*DACBLKSIZE] = (float)sp[j]*(float)3.051850e-05;
	    }
	}
	thischan += nchannels; 	  
     }
     if (thischan != linux_inchannels)
     	bug("inchannels");
     return (rtnval);
}

/* ----------------- audio I/O using the ALSA native API ---------------- */

#ifdef ALSA
static void alsa_checkversion( void)
{
    char snox[512];
    int fd, nbytes;
    if ((fd = open("/proc/asound/version", 0)) < 0 ||
    	(nbytes = read(fd, snox, 511)) < 1)
    {
    	perror("cannot check Alsa version -- /proc/asound/version");
	return;
    }
    snox[nbytes] = 0;
#ifdef ALSA99
    if (!strstr(snox, "Version 0.5"))
    {
    	fprintf(stderr,
"warning: Pd compiled for Alsa version 0.5 appears to be incompatible with\n\
the installed version of ALSA.  Here is what I found in /proc/asound/version:\n"
    	    	);
    	fprintf(stderr, "%s", snox);
    }
#else
    if (!strstr(snox, "Version 0.9"))
    {
    	fprintf(stderr,
"warning: Pd compiled for Alsa version 0.9 appears to be incompatible with\n\
the installed version of ALSA.  Here is what I found in /proc/asound/version:\n"
    	    	);
    	fprintf(stderr, "%s", snox);
    }
#endif
}
#endif

#ifdef ALSA99
static int alsa_open_audio(int wantinchans, int wantoutchans,
    int srate)
{
    int dir, voices, bsize;
    int err, id, rate, i;
    char *cardname;
    snd_ctl_hw_info_t hwinfo;
    snd_pcm_info_t pcminfo;
    snd_pcm_channel_info_t channelinfo;
    snd_ctl_t *handle;
    snd_pcm_sync_t sync;

    linux_inchannels = 0;
    linux_outchannels = 0;
    
    rate = 44100;
    alsa_samplewidth = 4;   	/* first try 4 byte samples */

    if (!wantinchans && !wantoutchans)
    	return (1);

    alsa_checkversion();
    if (sys_verbose)
    {
	if ((err = snd_card_get_longname(alsa_devno-1, &cardname)) < 0)
	{
	    fprintf(stderr, "PD-ALSA: unable to get name of card number %d\n",
	    	alsa_devno);
	    return 1;
	}
	fprintf(stderr, "PD-ALSA: using card %s\n", cardname);
	free(cardname);
    }

    if ((err = snd_ctl_open(&handle, alsa_devno-1)) < 0)
    {
	fprintf(stderr, "PD-ALSA: unable to open control: %s\n",
    	    snd_strerror(err));
	return 1;
    }

    if ((err = snd_ctl_hw_info(handle, &hwinfo)) < 0)
    {
	fprintf(stderr, "PD-ALSA: unable to open get info: %s\n",
    	    snd_strerror(err));
	return 1;
    }
    if (hwinfo.pcmdevs < 1)
    {
	fprintf(stderr, "PD-ALSA: device %d doesn't support PCM\n",
	    alsa_devno);
    	snd_ctl_close(handle);
	return 1;
    }

    if ((err = snd_ctl_pcm_info(handle, 0, &pcminfo)) < 0)
    {
	fprintf(stderr, "PD-ALSA: unable to open get pcm info: %s\n",
	    snd_strerror(err));
    	snd_ctl_close(handle);
	return (1);
    }
    snd_ctl_close(handle);

    	/* find out if opening for input, output, or both and check that the
	device can handle it. */
    if (wantinchans && wantoutchans)
    {
	if (!(pcminfo.flags & SND_PCM_INFO_DUPLEX))
	{
	    fprintf(stderr, "PD-ALSA: device is not full duplex\n");
	    return (1);
	}
	dir = SND_PCM_OPEN_DUPLEX;
    }
    else if (wantoutchans)
    {
	if (!(pcminfo.flags & SND_PCM_INFO_PLAYBACK))
	{
	    fprintf(stderr, "PD-ALSA: device is not full duplex\n");
	    return (1);
	}
    	dir = SND_PCM_OPEN_PLAYBACK;
    }
    else
    {
	if (!(pcminfo.flags & SND_PCM_INFO_CAPTURE))
	{
	    fprintf(stderr, "PD-ALSA: device is not full duplex\n");
	    return (1);
	}
    	dir = SND_PCM_OPEN_CAPTURE;
    }

    	/* try to open the device */
    if ((err = snd_pcm_open(&alsa_device[0].handle, alsa_devno-1, 0, dir)) < 0)
    {
	fprintf(stderr, "PD-ALSA: error opening device: %s\n",
	    snd_strerror(err));
	return (1);
    }
    	/* get information from the handle */
    if (wantinchans)
    {
	channelinfo.channel = SND_PCM_CHANNEL_CAPTURE;
	channelinfo.subdevice = 0;
	if ((err = snd_pcm_channel_info(alsa_device[0].handle, &channelinfo))
	    < 0)
	{
	    fprintf(stderr, "PD-ALSA: snd_pcm_channel_info (input): %s\n",
		snd_strerror(err));
	    return (1);
	}
	if (sys_verbose)
	    post("input channels supported: %d-%d\n",
	    	channelinfo.min_voices, channelinfo.max_voices);

	if (wantinchans < channelinfo.min_voices)
	    post("increasing input channels to minimum of %d\n",
		wantinchans = channelinfo.min_voices);
	if (wantinchans > channelinfo.max_voices)
	    post("decreasing input channels to maximum of %d\n",
		wantinchans = channelinfo.max_voices);
	if (alsa_samplewidth == 4 &&
	    !(channelinfo.formats & (1<<SND_PCM_SFMT_S32_LE)))
	{
	    fprintf(stderr,
	    	"PD_ALSA: input doesn't support 32-bit samples; using 16\n");
	    alsa_samplewidth = 2;
	}
	if (alsa_samplewidth == 2 &&
	    !(channelinfo.formats & (1<<SND_PCM_SFMT_S16_LE)))
	{
	    fprintf(stderr,
	    	"PD_ALSA: can't find 4 or 2 byte format; giving up\n");
	    return (1);
	}
    }

    if (wantoutchans)
    {
	channelinfo.channel = SND_PCM_CHANNEL_PLAYBACK;
	channelinfo.subdevice = 0;
	if ((err = snd_pcm_channel_info(alsa_device[0].handle, &channelinfo))
	    < 0)
	{
	    fprintf(stderr, "PD-ALSA: snd_pcm_channel_info (output): %s\n",
		snd_strerror(err));
	    return (1);
	}
	if (sys_verbose)
	    post("output channels supported: %d-%d\n",
	    	channelinfo.min_voices, channelinfo.max_voices);
	if (wantoutchans < channelinfo.min_voices)
	    post("increasing output channels to minimum of %d\n",
		wantoutchans = channelinfo.min_voices);
	if (wantoutchans > channelinfo.max_voices)
	    post("decreasing output channels to maximum of %d\n",
		wantoutchans = channelinfo.max_voices);
	if (alsa_samplewidth == 4 &&
	    !(channelinfo.formats & (1<<SND_PCM_SFMT_S32_LE)))
	{
	    fprintf(stderr,
	    	"PD_ALSA: output doesn't support 32-bit samples; using 16\n");
	    alsa_samplewidth = 2;
	}
	if (alsa_samplewidth == 2 &&
	    !(channelinfo.formats & (1<<SND_PCM_SFMT_S16_LE)))
	{
	    fprintf(stderr,
	    	"PD_ALSA: can't find 4 or 2 byte format; giving up\n");
	    return (1);
	}
    }

    linux_setsr(rate);
    linux_setch(wantinchans, wantoutchans);

    if (wantinchans)
    	alsa_set_params(&alsa_device[0], SND_PCM_CHANNEL_CAPTURE,
    	    srate, wantinchans);
    if (wantoutchans)
    	alsa_set_params(&alsa_device[0], SND_PCM_CHANNEL_PLAYBACK,
    	    srate, wantoutchans);

    n_alsa_dev = 1;

    /* check that all is as we think it should be */
    for (i = 0; i < n_alsa_dev; i++)
    {
	/* We need to handle if the rate is not the same for all
	* devices.  For now just hope. */
	rate = alsa_device[i].setup.format.rate;

    /* It turns out that this checking does not work on all of my cards
    * - in full duplex on my trident 4dwave the setup on the capture channel
    * shows a sampling rate of 0.  This is not true on my ess solo1.  Checking
    * the dac last helps the problem.  All of this needs to be much smarter
    * anyway (last minute hack).  A warning above is all I have time for.
    */
	if (rate != srate)
	{
	    post("PD-ALSA: unable to obtain rate %i using %i", srate, rate);
	    post("PD-ALSA: (despite this warning Pd might still work.)");
	}
    }
    bsize = alsa_samplewidth *
    	(linux_inchannels > linux_outchannels ? linux_inchannels :
	    linux_outchannels) * DACBLKSIZE;
    alsa_buf = malloc(bsize);
    if (!alsa_buf)
    	return (1);
    memset(alsa_buf, 0, bsize);
    return 0;
}

void alsa_set_params(t_alsa_dev *dev, int dir, int rate, int voices)
{
    int err;
    struct snd_pcm_channel_params params;

    memset(&dev->info, 0, sizeof(dev->info));
    dev->info.channel = dir;
    if ((err = snd_pcm_channel_info(dev->handle, &dev->info) < 0))
    {
	fprintf(stderr, "PD-ALSA: error getting channel info: %s\n",
	    snd_strerror(err));
    }
    memset(&params, 0, sizeof(params));
    params.format.interleave = 1;   /* may do non-interleaved later */
    	/* format is 2 or 4 bytes per sample depending on what was possible */
    params.format.format = 
    	(alsa_samplewidth == 4 ? SND_PCM_SFMT_S32_LE : SND_PCM_SFMT_S16_LE);

    	/*will check this further down -just try for now*/
    params.format.rate = rate; 
    params.format.voices = voices;
    params.start_mode = SND_PCM_START_GO;   /* seems most reliable */
    	                                    /*do not stop at overrun/underrun*/
    params.stop_mode = SND_PCM_STOP_ROLLOVER;

    params.channel = dir;                   /* playback|capture */
    params.buf.stream.queue_size =
	(ALSA_EXTRABUFFER + linux_advance_samples)
	    * alsa_samplewidth * voices;
    params.buf.stream.fill = SND_PCM_FILL_SILENCE_WHOLE;
    params.mode = SND_PCM_MODE_STREAM;

    if ((err = snd_pcm_channel_params(dev->handle, &params)) < 0)
    {
	printf("PD-ALSA: error setting parameters %s", snd_strerror(err));
    }

	/* This should clear the buffers but does not. There is often noise at
	startup that sounds like crap left in the buffers - maybe in the lib
	instead of the driver?  Some solution needs to be found.
	*/

    if ((err = snd_pcm_channel_prepare(dev->handle, dir)) < 0)
    {
	printf("PD-ALSA: error preparing channel %s", snd_strerror(err));
    }
    dev->setup.channel = dir;

    if ((err = snd_pcm_channel_setup(dev->handle, &dev->setup)) < 0)
    {
	printf("PD-ALSA: error getting setup %s", snd_strerror(err));
    }
    	/* for some reason, if you don't writesomething before starting the
	converters we get trash on startup */
    if (dir == SND_PCM_CHANNEL_PLAYBACK)
    {
    	char foo[1024];
	int xxx = 1024 - (1024 % (linux_outchannels * alsa_samplewidth));
	int i, r;
	for (i = 0; i < xxx; i++)
	    foo[i] = 0;
	if ((r = snd_pcm_write(dev->handle, foo, xxx)) < xxx)
	    fprintf(stderr, "alsa_write: %s\n", snd_strerror(errno));
    }
    snd_pcm_channel_go(dev->handle, dir);
}

void alsa_close_audio(void)
{
    int i;
    for(i = 0; i < n_alsa_dev; i++)
    	snd_pcm_close(alsa_device[i].handle);
}

/* #define DEBUG_ALSA_XFER */

int alsa_send_dacs(void)
{
    static int16_t *sp;
    t_sample *fp, *fp1, *fp2;
    int i, j, k, err, devno = 0;
    int inputcount = 0, outputcount = 0, inputlate = 0, outputlate = 0;
    int result; 
    snd_pcm_channel_status_t stat;
    static int callno = 0;
    static int xferno = 0;
    int countwas = 0;
    double timelast;
    static double timenow;
    int inchannels = linux_inchannels;
    int outchannels = linux_outchannels;
    int inbytesperframe = inchannels * alsa_samplewidth;
    int outbytesperframe = outchannels * alsa_samplewidth;
    int intransfersize = DACBLKSIZE * inbytesperframe;
    int outtransfersize = DACBLKSIZE * outbytesperframe;
    int alsaerror;
    int loggederror = 0;

    if (!inchannels && !outchannels)
    	return (SENDDACS_NO);
    timelast = timenow;
    timenow = sys_getrealtime();

#ifdef DEBUG_ALSA_XFER
    if (timenow - timelast > 0.050)
    	fprintf(stderr, "(%d)",
	    (int)(1000 * (timenow - timelast))), fflush(stderr);
#endif

    callno++;
    	    /* get input and output channel status */
    if (inchannels > 0)
    {
    	devno = 0;
    	stat.channel = SND_PCM_CHANNEL_CAPTURE;
	if (alsaerror = snd_pcm_channel_status(alsa_device[devno].handle,
	    &stat))
	{
	    fprintf(stderr, "snd_pcm_channel_status (input): %s\n",
	    	snd_strerror(alsaerror));
	    return (SENDDACS_NO);
	}
	inputcount = stat.count;
	inputlate = (stat.underrun > 0 || stat.overrun > 0);
    }
    if (outchannels > 0)
    {
	devno = 0;
    	stat.channel = SND_PCM_CHANNEL_PLAYBACK;
	if (alsaerror = snd_pcm_channel_status(alsa_device[devno].handle,
	    &stat))
	{
	    fprintf(stderr, "snd_pcm_channel_status (output): %s\n",
	    	snd_strerror(alsaerror));
	    return (SENDDACS_NO);
	}
	outputcount = stat.count;
    	outputlate = (stat.underrun > 0 || stat.overrun > 0);
    }

	/* check if input not ready */
    if (inputcount < intransfersize)
    {
	/* fprintf(stderr, "no adc; count %d, free %d, call %d, xfer %d\n",
	    stat.count, 
	    stat.free,
	    callno, xferno); */
	if (outchannels > 0)
	{
	    /* if there's no input but output is hungry, feed output. */
	    while (outputcount < (linux_advance_samples + ALSA_JITTER)
		* outbytesperframe)
	    {
	    	if (!loggederror)
		    sys_log_error(ERR_RESYNC), loggederror = 1;
		memset(alsa_buf, 0, outtransfersize);
		result = snd_pcm_write(alsa_device[devno].handle,
	    	    alsa_buf, outtransfersize);
		if (result < outtransfersize)
		{
#ifdef DEBUG_ALSA_XFER
		    if (result >= 0 || errno == EAGAIN)
			fprintf(stderr, "ALSA: write returned %d of %d\n",
			    result, outtransfersize);
		    else fprintf(stderr, "ALSA: write: %s\n",
	    		snd_strerror(errno));
		    fprintf(stderr,
		    	"inputcount %d, outputcount %d, outbufsize %d\n",
		    	inputcount, outputcount, 
	    	    	    (ALSA_EXTRABUFFER + linux_advance_samples)
	    	    	    	* alsa_samplewidth * outchannels);
#endif
		    return (SENDDACS_NO);
		}
    		stat.channel = SND_PCM_CHANNEL_PLAYBACK;
		if (alsaerror =
		    snd_pcm_channel_status(alsa_device[devno].handle,
		    	&stat))
		{
		    fprintf(stderr, "snd_pcm_channel_status (output): %s\n",
	    		snd_strerror(alsaerror));
		    return (SENDDACS_NO);
		}
		outputcount = stat.count;
	    }
	}

	return SENDDACS_NO;
    }

    	/* if output buffer has at least linux_advance_samples in it, we're
	not ready for this batch. */
    if (outputcount > linux_advance_samples * outbytesperframe)
    {
    	if (inchannels > 0)
	{
    	    while (inputcount > (DACBLKSIZE + ALSA_JITTER) * outbytesperframe)
	    {
	    	if (!loggederror)
		    sys_log_error(ERR_RESYNC), loggederror = 1;
	    	devno = 0;
		result = snd_pcm_read(alsa_device[devno].handle, alsa_buf,
		    intransfersize);
		if (result < intransfersize)
		{
#ifdef DEBUG_ALSA_XFER
		    if (result < 0)
    	    	    	fprintf(stderr,
	    		"snd_pcm_read %d %d: %s\n", 
	    		callno, xferno, snd_strerror(errno));
		    else fprintf(stderr,
	    		"snd_pcm_read %d %d returned only %d\n", 
	    		callno, xferno, result);
		    fprintf(stderr,
		    	"inputcount %d, outputcount %d, inbufsize %d\n",
		    	inputcount, outputcount, 
	    	    	    (ALSA_EXTRABUFFER + linux_advance_samples)
	    	    	    	* alsa_samplewidth * inchannels);
#endif
		    return (SENDDACS_NO);
		}
    		devno = 0;
    		stat.channel = SND_PCM_CHANNEL_CAPTURE;
		if (alsaerror =
		    snd_pcm_channel_status(alsa_device[devno].handle,
		    	&stat))
		{
		    fprintf(stderr, "snd_pcm_channel_status (input): %s\n",
	    		snd_strerror(alsaerror));
		    return (SENDDACS_NO);
		}
		inputcount = stat.count;
		inputlate = (stat.underrun > 0 || stat.overrun > 0);
	    }
	    return (SENDDACS_NO);
    	}
    }
    if (sys_getrealtime() - timenow > 0.002)
    {
#ifdef DEBUG_ALSA_XFER
    	fprintf(stderr, "check %d took %d msec\n",
	    callno, (int)(1000 * (timenow - timelast))), fflush(stderr);
#endif
    	sys_log_error(ERR_DACSLEPT);
    	timenow = sys_getrealtime();
    }
    if (inputlate || outputlate)
    	sys_log_error(ERR_DATALATE);    

    /* do output */
    	    /* this "for" loop won't work for more than one device. */
    for (devno = 0, fp = sys_soundout; devno < (outchannels > 0); devno++, 
       fp += 128)
    {
    	if (alsa_samplewidth == 4)
	{
	    for (i = 0, fp1 = fp; i < outchannels; i++, fp1 += DACBLKSIZE)
	    {
		for (j = i, k = DACBLKSIZE, fp2 = fp1; k--;
		    j += outchannels, fp2++)
	        {
		    float s1 = *fp2 * INT32_MAX;
		    ((t_alsa_sample32 *)alsa_buf)[j] = CLIP32(s1);
    	    	} 
	    }
	}
	else
	{
	    for (i = 0, fp1 = fp; i < outchannels; i++, fp1 += DACBLKSIZE)
	    {
		for (j = i, k = DACBLKSIZE, fp2 = fp1; k--;
		    j += outchannels, fp2++)
		{
		    int s = *fp2 * 32767.;
		    if (s > 32767)
			s = 32767;
		    else if (s < -32767)
			s = -32767;
		    ((t_alsa_sample16 *)alsa_buf)[j] = s;
		}
	    }
	}
	
	result = snd_pcm_write(alsa_device[devno].handle, alsa_buf,
	    outtransfersize);
	if (result < outtransfersize)
	{
#ifdef DEBUG_ALSA_XFER
	    if (result >= 0 || errno == EAGAIN)
		fprintf(stderr, "ALSA: write returned %d of %d\n",
		    result, outtransfersize);
	    else fprintf(stderr, "ALSA: write: %s\n",
	    	snd_strerror(errno));
	    fprintf(stderr,
		"inputcount %d, outputcount %d, outbufsize %d\n",
		inputcount, outputcount, 
	    	    (ALSA_EXTRABUFFER + linux_advance_samples)
	    	    	* alsa_samplewidth * outchannels);
#endif
    	    sys_log_error(ERR_DACSLEPT);
	    return (SENDDACS_NO);
	}
    }
    	/* zero out the output buffer */
    memset(sys_soundout, 0, DACBLKSIZE * sizeof(*sys_soundout) *
    	linux_outchannels);
    if (sys_getrealtime() - timenow > 0.002)
    {
#if DEBUG_ALSA_XFER
    	fprintf(stderr, "output %d took %d msec\n",
	    callno, (int)(1000 * (timenow - timelast))), fflush(stderr);
#endif
    	timenow = sys_getrealtime();
    	sys_log_error(ERR_DACSLEPT);
    }

    /* do input */
    for (devno = 0, fp = sys_soundin; devno < (linux_inchannels > 0); devno++, 
        fp += 128)
    {
	result = snd_pcm_read(alsa_device[devno].handle, alsa_buf,
	    intransfersize);
	if (result < intransfersize)
	{
#ifdef DEBUG_ALSA_XFER
	    if (result < 0)
    	    	fprintf(stderr,
	    	"snd_pcm_read %d %d: %s\n", 
	    	callno, xferno, snd_strerror(errno));
	    else fprintf(stderr,
	    	"snd_pcm_read %d %d returned only %d\n", 
	    	callno, xferno, result);
	    fprintf(stderr,
		"inputcount %d, outputcount %d, inbufsize %d\n",
		inputcount, outputcount, 
	    	    (ALSA_EXTRABUFFER + linux_advance_samples)
	    	    	* alsa_samplewidth * inchannels);
#endif
    	    sys_log_error(ERR_ADCSLEPT);
	    return (SENDDACS_NO);
	}
	if (alsa_samplewidth == 4)
	{
	    for (i = 0, fp1 = fp; i < inchannels; i++, fp1 += DACBLKSIZE)
	    {
		for (j = i, k = DACBLKSIZE, fp2 = fp1; k--;
		    j += inchannels, fp2++)
	    	    *fp2 = (float) ((t_alsa_sample32 *)alsa_buf)[j]
		    	* (1./ INT32_MAX);
	    }
	}
	else
	{
	    for (i = 0, fp1 = fp; i < inchannels; i++, fp1 += DACBLKSIZE)
	    {
		for (j = i, k = DACBLKSIZE, fp2 = fp1; k--; j += inchannels, fp2++)
	    	    *fp2 = (float) ((t_alsa_sample16 *)alsa_buf)[j]
		    	* 3.051850e-05;
	    }
    	}
    }
    xferno++;
    if (sys_getrealtime() - timenow > 0.002)
    {
#ifdef DEBUG_ALSA_XFER
    	fprintf(stderr, "routine took %d msec\n",
	    (int)(1000 * (sys_getrealtime() - timenow)));
#endif
    	sys_log_error(ERR_ADCSLEPT);
    }
    return SENDDACS_YES;
}

#endif				/* ALSA99 */

/* support for ALSA pcmv2 api by Karl MacMillan<karlmac@peabody.jhu.edu> */ 

#ifdef ALSA01

static void check_error(int err, const char *why)
{
    if (err < 0)
	fprintf(stderr, "%s: %s\n", why, snd_strerror(err));
}

static int alsa_open_audio(int wantinchans, int wantoutchans, int srate)
{
    int err, inchans = 0, outchans = 0, subunitdir;
    char devname[512];
    snd_pcm_hw_params_t* hw_params;
    snd_pcm_sw_params_t* sw_params;
    snd_output_t* out;
    int frag_size = (linux_fragsize ? linux_fragsize : ALSA_DEFFRAGSIZE);
    int nfrags, i;
    short* tmp_buf;
    unsigned int tmp_uint;
    int advwas = sys_schedadvance;

    if (linux_nfragment)
    {
    	nfrags = linux_nfragment;
    	sys_schedadvance = (frag_size * linux_nfragment * 1.0e6) / srate;
    }
    else nfrags = sys_schedadvance * (float)srate / (1e6 * frag_size);

    if (sys_verbose || (sys_schedadvance != advwas))
    	post("audio buffer set to %d", (int)(0.001 * sys_schedadvance));
    if (wantinchans || wantoutchans)
    	alsa_checkversion();
    if (wantinchans)
    {
	err = snd_pcm_open(&alsa_device.inhandle, alsa_devname,
			   SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);

	check_error(err, "snd_pcm_open (input)");
	if (err < 0)
	    inchans = 0;
	else
	{
	    inchans = wantinchans;
	    snd_pcm_nonblock(alsa_device.inhandle, 1);
	}
    }
    if (wantoutchans)
    {
	err = snd_pcm_open(&alsa_device.outhandle, alsa_devname,
			   SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK);

	check_error(err, "snd_pcm_open (output)");
	if (err < 0)
	    outchans = 0;
	else
	{
	    outchans = wantoutchans;
	    snd_pcm_nonblock(alsa_device.outhandle, 1);
	}
    }
    if (inchans)
    {
    	if (sys_verbose)
	    post("opening sound input...");
	err = snd_pcm_hw_params_malloc(&hw_params);
	check_error(err, "snd_pcm_hw_params_malloc (input)");
    
	// get the default params
	err = snd_pcm_hw_params_any(alsa_device.inhandle, hw_params);
	check_error(err, "snd_pcm_hw_params_any (input)");
	// set interleaved access - FIXME deal with other access types
	err = snd_pcm_hw_params_set_access(alsa_device.inhandle, hw_params,
					   SND_PCM_ACCESS_RW_INTERLEAVED);
	check_error(err, "snd_pcm_hw_params_set_access (input)");
	// Try to set 32 bit format first
	err = snd_pcm_hw_params_set_format(alsa_device.inhandle, hw_params,
					   SND_PCM_FORMAT_S32);
	if (err < 0)
	{
	    /* fprintf(stderr,
	    	"PD-ALSA: 32 bit format not available - using 16\n"); */
	    err = snd_pcm_hw_params_set_format(alsa_device.inhandle, hw_params,
					       SND_PCM_FORMAT_S16);
	    check_error(err, "snd_pcm_hw_params_set_format (input)");
	    alsa_samplewidth = 2;
	}
	else
	{
	    alsa_samplewidth = 4;
	}
	post("Sample width set to %d bytes", alsa_samplewidth);
	// set the subformat
	err = snd_pcm_hw_params_set_subformat(alsa_device.inhandle, hw_params,
					      SND_PCM_SUBFORMAT_STD);
	check_error(err, "snd_pcm_hw_params_set_subformat (input)");
	// set the number of channels
	tmp_uint = inchans;
	err = snd_pcm_hw_params_set_channels_min(alsa_device.inhandle,
						 hw_params, &tmp_uint);
	check_error(err, "snd_pcm_hw_params_set_channels (input)");
	if (tmp_uint != (unsigned)inchans)
	    post("ALSA: set input channels to %d", tmp_uint);
	inchans = tmp_uint;
	// set the sampling rate
	err = snd_pcm_hw_params_set_rate_min(alsa_device.inhandle, hw_params,
					     &srate, 0);
	check_error(err, "snd_pcm_hw_params_set_rate_min (input)");
#if 0
    	err = snd_pcm_hw_params_get_rate(hw_params, &subunitdir);
	post("input sample rate %d", err);
#endif
	// set the period - ie frag size
	// post("fragsize a %d", frag_size);

    	    /* LATER try this to get a recommended period size...
	     right now, it trips an assertion failure in ALSA lib */
#if 0
    	post("input period was %d, min %d, max %d\n",
	    snd_pcm_hw_params_get_period_size(hw_params, 0),
	    snd_pcm_hw_params_get_period_size_min(hw_params, 0),
	    snd_pcm_hw_params_get_period_size_max(hw_params, 0));
#endif	    
	err = snd_pcm_hw_params_set_period_size_near(alsa_device.inhandle,
						    hw_params, 
						    (snd_pcm_uframes_t)
						    frag_size, 0);
	check_error(err, "snd_pcm_hw_params_set_period_size_near (input)");
	// post("fragsize b %d", frag_size);
	// set the number of periods - ie numfrags
	// post("nfrags a %d", nfrags);
	err = snd_pcm_hw_params_set_periods_near(alsa_device.inhandle,
						hw_params, nfrags, 0);
	check_error(err, "snd_pcm_hw_params_set_periods_near (input)");
	// set the buffer size
	err = snd_pcm_hw_params_set_buffer_size_near(alsa_device.inhandle,
						hw_params, nfrags * frag_size);
	check_error(err, "snd_pcm_hw_params_set_buffer_size_near (input)");

	err = snd_pcm_hw_params(alsa_device.inhandle, hw_params);
	check_error(err, "snd_pcm_hw_params (input)");
	
	snd_pcm_hw_params_free(hw_params);
	
	err = snd_pcm_sw_params_malloc(&sw_params);
	check_error(err, "snd_pcm_sw_params_malloc (input)");
	err = snd_pcm_sw_params_current(alsa_device.inhandle, sw_params);
	check_error(err, "snd_pcm_sw_params_current (input)");
#if 1
	err = snd_pcm_sw_params_set_start_mode(alsa_device.inhandle, sw_params,
					       SND_PCM_START_EXPLICIT);
	check_error(err, "snd_pcm_sw_params_set_start_mode (input)");
	err = snd_pcm_sw_params_set_xrun_mode(alsa_device.inhandle, sw_params,
					      SND_PCM_XRUN_NONE);
	check_error(err, "snd_pcm_sw_params_set_xrun_mode (input)");
#else
	err = snd_pcm_sw_params_set_start_threshold(alsa_device.inhandle,
	    sw_params, nfrags * frag_size);
	check_error(err, "snd_pcm_sw_params_set_start_threshold (input)");
	err = snd_pcm_sw_params_set_stop_threshold(alsa_device.inhandle,
	    sw_params, 1);
	check_error(err, "snd_pcm_sw_params_set_stop_threshold (input)");
#endif

	err = snd_pcm_sw_params_set_avail_min(alsa_device.inhandle, sw_params,
					      frag_size);
	check_error(err, "snd_pcm_sw_params_set_avail_min (input)");
	err = snd_pcm_sw_params(alsa_device.inhandle, sw_params);
	check_error(err, "snd_pcm_sw_params (input)");
	
	snd_pcm_sw_params_free(sw_params);
	
	snd_output_stdio_attach(&out, stderr, 0);
#if 0
	if (sys_verbose)
	{
	    snd_pcm_dump_hw_setup(alsa_device.inhandle, out);
	    snd_pcm_dump_sw_setup(alsa_device.inhandle, out);
    	}
#endif
    }

    if (outchans)
    {
    	int foo;
    	if (sys_verbose)
	    post("opening sound output...");
	err = snd_pcm_hw_params_malloc(&hw_params);
	check_error(err, "snd_pcm_sw_params (output)");

	// get the default params
	err = snd_pcm_hw_params_any(alsa_device.outhandle, hw_params);
	check_error(err, "snd_pcm_hw_params_any (output)");
	// set interleaved access - FIXME deal with other access types
	err = snd_pcm_hw_params_set_access(alsa_device.outhandle, hw_params,
					   SND_PCM_ACCESS_RW_INTERLEAVED);
	check_error(err, "snd_pcm_hw_params_set_access (output)");
	// Try to set 32 bit format first
	err = snd_pcm_hw_params_set_format(alsa_device.outhandle, hw_params,
					     SND_PCM_FORMAT_S32);
	if (err < 0)
	{
	    err = snd_pcm_hw_params_set_format(alsa_device.outhandle,
					       hw_params,SND_PCM_FORMAT_S16);
	    check_error(err, "snd_pcm_hw_params_set_format (output)");
	    /* fprintf(stderr,
	    	"PD-ALSA: 32 bit format not available - using 16\n"); */
	    alsa_samplewidth = 2;
	}
	else
	{
	    alsa_samplewidth = 4;
	}
	// set the subformat
	err = snd_pcm_hw_params_set_subformat(alsa_device.outhandle, hw_params,
					      SND_PCM_SUBFORMAT_STD);
	check_error(err, "snd_pcm_hw_params_set_subformat (output)");
	// set the number of channels
	tmp_uint = outchans;
	err = snd_pcm_hw_params_set_channels_min(alsa_device.outhandle,
						 hw_params, &tmp_uint);
	check_error(err, "snd_pcm_hw_params_set_channels (output)");
	if (tmp_uint != (unsigned)outchans)
    	    post("alsa: set output channels to %d", tmp_uint);
    	outchans = tmp_uint;
	// set the sampling rate
	err = snd_pcm_hw_params_set_rate_min(alsa_device.outhandle, hw_params,
					     &srate, 0);
	check_error(err, "snd_pcm_hw_params_set_rate_min (output)");
#if 0
    	err = snd_pcm_hw_params_get_rate(hw_params, &subunitdir);
	post("output sample rate %d", err);
#endif
	// set the period - ie frag size
#if 0
    	post("output period was %d, min %d, max %d\n",
	    snd_pcm_hw_params_get_period_size(hw_params, 0),
	    snd_pcm_hw_params_get_period_size_min(hw_params, 0),
	    snd_pcm_hw_params_get_period_size_max(hw_params, 0));
#endif
	// post("fragsize c %d", frag_size);
	err = snd_pcm_hw_params_set_period_size_near(alsa_device.outhandle,
						    hw_params,
						    (snd_pcm_uframes_t)
						    frag_size, 0);
	// post("fragsize d %d", frag_size);
	check_error(err, "snd_pcm_hw_params_set_period_size_near (output)");
	// set the number of periods - ie numfrags
	err = snd_pcm_hw_params_set_periods_near(alsa_device.outhandle,
						hw_params, nfrags, 0);
	check_error(err, "snd_pcm_hw_params_set_periods_near (output)");
	// set the buffer size
	err = snd_pcm_hw_params_set_buffer_size_near(alsa_device.outhandle,
	    hw_params, nfrags * frag_size);

	check_error(err, "snd_pcm_hw_params_set_buffer_size_near (output)");
	
	err = snd_pcm_hw_params(alsa_device.outhandle, hw_params);
	check_error(err, "snd_pcm_hw_params (output)");
	
	snd_pcm_hw_params_free(hw_params);

	err = snd_pcm_sw_params_malloc(&sw_params);
	check_error(err, "snd_pcm_sw_params_malloc (output)");
	err = snd_pcm_sw_params_current(alsa_device.outhandle, sw_params);
	check_error(err, "snd_pcm_sw_params_current (output)");
#if 1
	err = snd_pcm_sw_params_set_start_mode(alsa_device.outhandle,
					       sw_params,
					       SND_PCM_START_EXPLICIT);
	check_error(err, "snd_pcm_sw_params_set_start_mode (output)");
	err = snd_pcm_sw_params_set_xrun_mode(alsa_device.outhandle, sw_params,
					      SND_PCM_XRUN_NONE);
	check_error(err, "snd_pcm_sw_params_set_xrun_mode (output)");
#else
	err = snd_pcm_sw_params_set_start_threshold(alsa_device.inhandle,
	    sw_params, nfrags * frag_size);
	check_error(err, "snd_pcm_sw_params_set_start_threshold (output)");
	err = snd_pcm_sw_params_set_stop_threshold(alsa_device.inhandle,
	    sw_params, 1);
	check_error(err, "snd_pcm_sw_params_set_stop_threshold (output)");
#endif

	err = snd_pcm_sw_params_set_avail_min(alsa_device.outhandle, sw_params,
					      frag_size);
	check_error(err, "snd_pcm_sw_params_set_avail_min (output)");
	err = snd_pcm_sw_params(alsa_device.outhandle, sw_params);
	check_error(err, "snd_pcm_sw_params (output)");
	
	snd_pcm_sw_params_free(sw_params);
	
	snd_output_stdio_attach(&out, stderr, 0);
#if 0
	if (sys_verbose)
	{
	    snd_pcm_dump_hw_setup(alsa_device.outhandle, out);
	    snd_pcm_dump_sw_setup(alsa_device.outhandle, out);
    	}
#endif
    }

    linux_setsr(srate);
    linux_setch(inchans, outchans);

    if (inchans)
      snd_pcm_prepare(alsa_device.inhandle);
    if (outchans)
      snd_pcm_prepare(alsa_device.outhandle);

    // if duplex we can link the channels so they start together
    if (inchans && outchans)
      snd_pcm_link(alsa_device.inhandle, alsa_device.outhandle);

    // set up the buffer
    if (outchans > inchans) 
	alsa_buf = (short *)calloc(sizeof(char) * alsa_samplewidth, DACBLKSIZE
				  * outchans);
    else
	alsa_buf = (short *)calloc(sizeof(char) * alsa_samplewidth, DACBLKSIZE
				* inchans);
    // fill the buffer with silence
    if (outchans)
    {
	i = nfrags + 1;
	while (i--)
	    snd_pcm_writei(alsa_device.outhandle, alsa_buf, frag_size);
    }

    // set up the status variables
    err = snd_pcm_status_malloc(&in_status);
    check_error(err, "snd_pcm_status_malloc");
    err = snd_pcm_status_malloc(&out_status);
    check_error(err, "snd_pcm_status_malloc");

    // start the device
#if 1
    if (outchans)
    {
	err = snd_pcm_start(alsa_device.outhandle);
	check_error(err, "snd_pcm_start");
    }
    else if (inchans)
    {
	err = snd_pcm_start(alsa_device.inhandle);
	check_error(err, "snd_pcm_start");
    }
#endif
	    
    return 0;
}

void alsa_close_audio(void)
{
    int err;
    if (linux_inchannels)
    {
	err = snd_pcm_close(alsa_device.inhandle);
	check_error(err, "snd_pcm_close (input)");
    }
    if (linux_outchannels)
    {
	err = snd_pcm_close(alsa_device.outhandle);
	check_error(err, "snd_pcm_close (output)");
    }
}

// #define DEBUG_ALSA_XFER

int alsa_send_dacs(void)
{
    static int16_t *sp;
    static int xferno = 0;
    static int callno = 0;
    static double timenow;
    double timelast;
    t_sample *fp, *fp1, *fp2;
    int i, j, k, err, devno = 0;
    int inputcount = 0, outputcount = 0, inputlate = 0, outputlate = 0;
    int result; 
    int inchannels = linux_inchannels;
    int outchannels = linux_outchannels;
    unsigned int intransfersize = DACBLKSIZE;
    unsigned int outtransfersize = DACBLKSIZE;

    // get the status
    if (!inchannels && !outchannels)
    {
	return SENDDACS_NO;
    }
    
    timelast = timenow;
    timenow = sys_getrealtime();

#ifdef DEBUG_ALSA_XFER
    if (timenow - timelast > 0.050)
    	fprintf(stderr, "(%d)",
	    (int)(1000 * (timenow - timelast))), fflush(stderr);
#endif

    callno++;

    if (inchannels)
    {
	snd_pcm_status(alsa_device.inhandle, in_status);
	if (snd_pcm_status_get_avail(in_status) < intransfersize)
	    return SENDDACS_NO;
    }
    if (outchannels)
    {
	snd_pcm_status(alsa_device.outhandle, out_status);
	if (snd_pcm_status_get_avail(out_status) < outtransfersize)
	    return SENDDACS_NO;
    }

    /* do output */
    if (outchannels)
    {
	fp = sys_soundout;
	if (alsa_samplewidth == 4)
	{
	    for (i = 0, fp1 = fp; i < outchannels; i++, fp1 += DACBLKSIZE)
	    {
		for (j = i, k = DACBLKSIZE, fp2 = fp1; k--;
		     j += outchannels, fp2++)
		{
		    float s1 = *fp2 * INT32_MAX;
		    ((t_alsa_sample32 *)alsa_buf)[j] = CLIP32(s1);
		} 
	    }
	}
	else
	{
	    for (i = 0, fp1 = fp; i < outchannels; i++, fp1 += DACBLKSIZE)
	    {
		for (j = i, k = DACBLKSIZE, fp2 = fp1; k--;
		     j += outchannels, fp2++)
		{
		    int s = *fp2 * 32767.;
		    if (s > 32767)
			s = 32767;
		    else if (s < -32767)
			s = -32767;
		    ((t_alsa_sample16 *)alsa_buf)[j] = s;
		}
	    }
	}

	result = snd_pcm_writei(alsa_device.outhandle, alsa_buf,
				outtransfersize);
	if (result != (int)outtransfersize)
	{
    #ifdef DEBUG_ALSA_XFER
	    if (result >= 0 || errno == EAGAIN)
		fprintf(stderr, "ALSA: write returned %d of %d\n",
			result, outtransfersize);
	    else fprintf(stderr, "ALSA: write: %s\n",
			 snd_strerror(errno));
	    fprintf(stderr,
		    "inputcount %d, outputcount %d, outbufsize %d\n",
		    inputcount, outputcount, 
		    (ALSA_EXTRABUFFER + linux_advance_samples)
		    * alsa_samplewidth * outchannels);
    #endif
	    sys_log_error(ERR_DACSLEPT);
	    return (SENDDACS_NO);
	}

	/* zero out the output buffer */
	memset(sys_soundout, 0, DACBLKSIZE * sizeof(*sys_soundout) *
	       linux_outchannels);
	if (sys_getrealtime() - timenow > 0.002)
	{
    #ifdef DEBUG_ALSA_XFER
    	    fprintf(stderr, "output %d took %d msec\n",
		    callno, (int)(1000 * (timenow - timelast))), fflush(stderr);
    #endif
    	    timenow = sys_getrealtime();
    	    sys_log_error(ERR_DACSLEPT);
	}
    }
    /* do input */
    if (linux_inchannels)
    {
	result = snd_pcm_readi(alsa_device.inhandle, alsa_buf, intransfersize);
	if (result < (int)intransfersize)
	{
#ifdef DEBUG_ALSA_XFER
	    if (result < 0)
    	    	fprintf(stderr,
			"snd_pcm_read %d %d: %s\n", 
			callno, xferno, snd_strerror(errno));
	    else fprintf(stderr,
			 "snd_pcm_read %d %d returned only %d\n", 
			 callno, xferno, result);
	    fprintf(stderr,
		    "inputcount %d, outputcount %d, inbufsize %d\n",
		    inputcount, outputcount, 
	    	    (ALSA_EXTRABUFFER + linux_advance_samples)
		    * alsa_samplewidth * inchannels);
#endif
    	    sys_log_error(ERR_ADCSLEPT);
	    return (SENDDACS_NO);
	}
	fp = sys_soundin;
	if (alsa_samplewidth == 4)
	{
	    for (i = 0, fp1 = fp; i < inchannels; i++, fp1 += DACBLKSIZE)
	    {
		for (j = i, k = DACBLKSIZE, fp2 = fp1; k--;
		     j += inchannels, fp2++)
	    	    *fp2 = (float) ((t_alsa_sample32 *)alsa_buf)[j]
		    	* (1./ INT32_MAX);
	    }
	}
	else
	{
	    for (i = 0, fp1 = fp; i < inchannels; i++, fp1 += DACBLKSIZE)
	    {
		for (j = i, k = DACBLKSIZE, fp2 = fp1; k--; j += inchannels,
		    fp2++)
	    	    	*fp2 = (float) ((t_alsa_sample16 *)alsa_buf)[j]
		    	    * 3.051850e-05;
	    }
    	}
    }
    xferno++;
    if (sys_getrealtime() - timenow > 0.002)
    {
#ifdef DEBUG_ALSA_XFER
    	fprintf(stderr, "routine took %d msec\n",
	    (int)(1000 * (sys_getrealtime() - timenow)));
#endif
    	sys_log_error(ERR_ADCSLEPT);
    }
    return SENDDACS_YES;
}

void alsa_resync( void)
{
    int i, result;
    if (linux_whichapi != API_ALSA)
    {
    	error("restart-audio: implemented for ALSA only.");
	return;
    }
    memset(alsa_buf, 0,
    	sizeof(char) * alsa_samplewidth * DACBLKSIZE * linux_outchannels);
    for (i = 0; i < 100; i++)
    {
	result = snd_pcm_writei(alsa_device.outhandle, alsa_buf,
	    DACBLKSIZE);
	if (result != (int)DACBLKSIZE)
	    break;
    }
    post("%d written", i);
}


#endif				/* ALSA01 */

/***************************************************
 *  Code using the RME_9652 API
 */ 

    /*
    trying native device for future use of native memory map:
    because of busmaster if you dont use the dac, you dont need 
    CPU Power und also no nearly no CPU-Power is used in device

    since always all DAs and ADs are synced (else they wouldnt work)
    we use linux_dacs[0], linux_adcs[0]
    */

#ifdef RME_HAMMERFALL

#define RME9652_MAX_CHANNELS 26

#define RME9652_CH_PER_NATIVE_DEVICE 1

static int rme9652_dac_devices[RME9652_MAX_CHANNELS];
static int rme9652_adc_devices[RME9652_MAX_CHANNELS];

static char rme9652_dsp_dac[] = "/dev/rme9652/C0da%d"; 
static char rme9652_dsp_adc[] = "/dev/rme9652/C0ad%d"; 

static int num_of_rme9652_dac = 0;
static int num_of_rme9652_adc = 0;

static int rme_soundindevonset = 1;
static int rme_soundoutdevonset = 1;

void rme_soundindev(int which)
{
    rme_soundindevonset = which;
}

void rme_soundoutdev(int which)
{
    rme_soundoutdevonset = which;
}

void rme9652_configure(int dev, int fd,int srate, int dac) {
  int orig, param, nblk;
  audio_buf_info ainfo;
  orig = param = srate; 

  /* samplerate */

  fprintf(stderr,"RME9652: configuring %d, fd=%d, sr=%d\n, dac=%d\n",
			 dev,fd,srate,dac);

  if (ioctl(fd,SNDCTL_DSP_SPEED,&param) == -1)
	 fprintf(stderr,"RME9652: Could not set sampling rate for device\n");
  else if( orig != param )
	 fprintf(stderr,"RME9652: sampling rate: wanted %d, got %d\n",
				orig, param );				
  
  // setting the correct samplerate (could be different than expected)     
  srate = param;


  /* setting resolution */

  /* use ctrlpanel to change, experiment, channels 1 */

  orig = param = AFMT_S16_NE;
  if (ioctl(fd,SNDCTL_DSP_SETFMT,&param) == -1)
	 fprintf(stderr,"RME9652: Could not set DSP format\n");
  else if( orig != param )
    fprintf(stderr,"RME9652: DSP format: wanted %d, got %d\n",orig, param );

  /* setting channels */
  orig = param = RME9652_CH_PER_NATIVE_DEVICE;

  if (ioctl(fd,SNDCTL_DSP_CHANNELS,&param) == -1)
	 fprintf(stderr,"RME9652: Could not set channels\n");
  else if( orig != param )
    fprintf(stderr,"RME9652: num channels: wanted %d, got %d\n",orig, param );

  if (dac)
    {

 /* use "free space" to learn the buffer size.  Normally you
	 should set this to your own desired value; but this seems not
	 to be implemented uniformly across different sound cards.  LATER
	 we should figure out what to do if the requested scheduler advance
	 is greater than this buffer size; for now, we just print something
	 out.  */

	if( ioctl(linux_dacs[0].d_fd, SOUND_PCM_GETOSPACE,&ainfo) < 0 )
	  fprintf(stderr,"RME: ioctl on output device %d failed",dev);

	linux_dacs[0].d_bufsize = ainfo.bytes;

	fprintf(stderr,"RME: ioctl SOUND_PCM_GETOSPACE says %d buffsize\n",
			  linux_dacs[0].d_bufsize);


	if (linux_advance_samples * (RME_SAMPLEWIDTH *
	    RME9652_CH_PER_NATIVE_DEVICE) 
		 > linux_dacs[0].d_bufsize - RME_BYTESPERCHAN)
	  {
		 fprintf(stderr,
		    "RME: requested audio buffer size %d limited to %d\n",
		    linux_advance_samples 
		    * (RME_SAMPLEWIDTH * RME9652_CH_PER_NATIVE_DEVICE),
		    linux_dacs[0].d_bufsize);
    	    	linux_advance_samples =
		    (linux_dacs[0].d_bufsize - RME_BYTESPERCHAN) 
		    / (RME_SAMPLEWIDTH *RME9652_CH_PER_NATIVE_DEVICE);
	  }
    }
}

 
int rme9652_open_audio(int inchans, int outchans,int srate)
{  
    int orig;
    int tmp;
    int inchannels = 0,outchannels = 0;
    char devname[20];
    int i;
    char buf[RME_SAMPLEWIDTH*RME9652_CH_PER_NATIVE_DEVICE*DACBLKSIZE];
    int num_devs = 0;
    audio_buf_info ainfo;

    linux_nindevs = linux_noutdevs = 0;

    if (sys_verbose)
    	post("RME open");
    /* First check if we can  */
    /* open the write ports	*/

    for (num_devs=0; outchannels < outchans; num_devs++)
    {
	int channels = RME9652_CH_PER_NATIVE_DEVICE;

	sprintf(devname, rme9652_dsp_dac, num_devs + rme_soundoutdevonset);
	if ((tmp = open(devname,O_WRONLY)) == -1) 
	{
	    DEBUG(fprintf(stderr,"RME9652: failed to open %s writeonly\n",
    	    	devname);)
	    break;
	}
	DEBUG(fprintf(stderr,"RME9652: out device Nr. %d (%d) on %s\n",
    	    linux_noutdevs+1,tmp,devname);)

	if (outchans > outchannels)
	{
	    rme9652_dac_devices[linux_noutdevs] = tmp;
	    linux_noutdevs++;
	    outchannels += channels;
	}
	else close(tmp);
    }
    if( linux_noutdevs > 0)
	linux_dacs[0].d_fd =  rme9652_dac_devices[0];

    /* Second check if we can  */
    /* open the read ports	*/

    for (num_devs=0; inchannels < inchans; num_devs++)
    {
	int channels = RME9652_CH_PER_NATIVE_DEVICE;

	sprintf(devname, rme9652_dsp_adc, num_devs+rme_soundindevonset);

	if ((tmp = open(devname,O_RDONLY)) == -1) 
	{
	    DEBUG(fprintf(stderr,"RME9652: failed to open %s readonly\n",
	    	devname);)
	    break;
	}
	DEBUG(fprintf(stderr,"RME9652: in device Nr. %d (%d) on %s\n",
	    linux_nindevs+1,tmp,devname);)

	if (inchans > inchannels)
	{
	    rme9652_adc_devices[linux_nindevs] = tmp;
	    linux_nindevs++;
	    inchannels += channels;
	}
	else
	     close(tmp);
    }
    if(linux_nindevs > 0)
	linux_adcs[0].d_fd = rme9652_adc_devices[0];

    /* configure soundcards */

	 rme9652_configure(0, linux_adcs[0].d_fd,srate, 0);
	 rme9652_configure(0, linux_dacs[0].d_fd,srate, 1);
    
	 /* We have to do a read to start the engine. This is 
		 necessary because sys_send_dacs waits until the input
		 buffer is filled and only reads on a filled buffer.
		 This is good, because it's a way to make sure that we
		 will not block */
	 
    if (linux_nindevs)
    {
	fprintf(stderr,("RME9652: starting read engine ... "));


	for (num_devs=0; num_devs < linux_nindevs; num_devs++) 
	  read(rme9652_adc_devices[num_devs],
	    buf, RME_SAMPLEWIDTH* RME9652_CH_PER_NATIVE_DEVICE*
		DACBLKSIZE);


	for (num_devs=0; num_devs < linux_noutdevs; num_devs++) 
	  write(rme9652_dac_devices[num_devs],
	    buf, RME_SAMPLEWIDTH* RME9652_CH_PER_NATIVE_DEVICE*
		DACBLKSIZE);

	if(linux_noutdevs)
	  ioctl(rme9652_dac_devices[0],SNDCTL_DSP_SYNC);

	fprintf(stderr,"done\n");
    }

    linux_setsr(srate);
    linux_setch(linux_nindevs, linux_noutdevs);

    num_of_rme9652_dac = linux_noutdevs;
    num_of_rme9652_adc = linux_nindevs;

    if(linux_noutdevs)linux_noutdevs=1;
    if(linux_nindevs)linux_nindevs=1;

    /* trick RME9652 behaves as one device fromread write pointers */
    return (0);
}

void rme9652_close_audio( void)
{
     int i;
     for (i=0;i<num_of_rme9652_dac;i++)
		 close(rme9652_dac_devices[i]);

     for (i=0;i<num_of_rme9652_adc;i++)
          close(rme9652_adc_devices[i]);
}


/* query audio devices for "available" data size. */
/* not needed because oss_calcspace does the same */
static int rme9652_calcspace(void)
{
    audio_buf_info ainfo;


    /* one for all */

    if (ioctl(linux_dacs[0].d_fd, SOUND_PCM_GETOSPACE,&ainfo) < 0)
	fprintf(stderr,
   "RME9652: calc ioctl SOUND_PCM_GETOSPACE on output device fd %d failed\n",
    	linux_dacs[0].d_fd);
    linux_dacs[0].d_space = ainfo.bytes;

    if (ioctl(linux_adcs[0].d_fd, SOUND_PCM_GETISPACE,&ainfo) < 0)
	fprintf(stderr, 
 "RME9652: calc ioctl SOUND_PCM_GETISPACE on input device fd %d failed\n", 
    	rme9652_adc_devices[0]);

    linux_adcs[0].d_space = ainfo.bytes;

    return 1;
}

/* this call resyncs audio output and input which will cause discontinuities
in audio output and/or input. */ 

static void rme9652_doresync( void)
{	
  if(linux_noutdevs)
	 ioctl(rme9652_dac_devices[0],SNDCTL_DSP_SYNC);
}

static int mycount =0;

int rme9652_send_dacs(void)
{
    float *fp;
    long fill;
    int i, j, dev;
	 /* the maximum number of samples we should have in the ADC buffer */
    t_rme_sample buf[RME9652_CH_PER_NATIVE_DEVICE*DACBLKSIZE], *sp;

    double timeref, timenow;

    mycount++;

    if (!linux_nindevs && !linux_noutdevs) return (0);

    rme9652_calcspace();
    	 
	/* do output */
	
   timeref = sys_getrealtime();

    if(linux_noutdevs){

    	if (linux_dacs[0].d_dropcount)
    	    linux_dacs[0].d_dropcount--;
    	else{
	    /* fprintf(stderr,"output %d\n", linux_outchannels);*/

	    for(j=0;j<linux_outchannels;j++){

	      t_rme_sample *a,*b,*c,*d;
	      float *fp1,*fp2,*fp3,*fp4;

	      fp1 = sys_soundout + j*DACBLKSIZE-4;
	      fp2 = fp1 + 1;
	      fp3 = fp1 + 2;
	      fp4 = fp1 + 3;
	      a = buf-4;
	      b=a+1;
	      c=a+2;
	      d=a+3; 

	      for (i = DACBLKSIZE>>2;i--;)
		 { 
		    float s1 =  *(fp1+=4) * INT32_MAX;
		    float s2 =  *(fp2+=4) * INT32_MAX;
		    float s3 =  *(fp3+=4) * INT32_MAX;
		    float s4 =  *(fp4+=4) * INT32_MAX;

		    *(a+=4) = CLIP32(s1);
		    *(b+=4) = CLIP32(s2);
		    *(c+=4) = CLIP32(s3);
		    *(d+=4) = CLIP32(s4);
    	    	} 

		linux_dacs_write(rme9652_dac_devices[j],buf,RME_BYTESPERCHAN);
	    }
    	}

	  if ((timenow = sys_getrealtime()) - timeref > 0.02)
		 sys_log_error(ERR_DACSLEPT);
	  timeref = timenow;
    }

    memset(sys_soundout, 0,
    	linux_outchannels * (sizeof(float) * DACBLKSIZE));

	 /* do input */

    if(linux_nindevs) {

    	for(j=0;j<linux_inchannels;j++){

    	    linux_adcs_read(rme9652_adc_devices[j], buf, RME_BYTESPERCHAN);
		  
	    if ((timenow = sys_getrealtime()) - timeref > 0.02)
		 sys_log_error(ERR_ADCSLEPT);
	    timeref = timenow;
	    {
		t_rme_sample *a,*b,*c,*d;
		float *fp1,*fp2,*fp3,*fp4;

		fp1 = sys_soundin + j*DACBLKSIZE-4;
		fp2 = fp1 + 1;
		fp3 = fp1 + 2;
		fp4 = fp1 + 3;
		a = buf-4;
		b=a+1;
		c=a+2;
		d=a+3; 

		for (i = (DACBLKSIZE>>2);i--;)
		{ 
		    *(fp1+=4) = *(a+=4) * (float)(1./INT32_MAX);
		    *(fp2+=4) = *(b+=4) * (float)(1./INT32_MAX);
		    *(fp3+=4) = *(c+=4) * (float)(1./INT32_MAX);
		    *(fp4+=4) = *(d+=4) * (float)(1./INT32_MAX);
		}
	    }  
    	}	
    }
    /*	fprintf(stderr,"ready \n");*/

    return (1);
}

#endif /* RME_HAMMERFALL */
