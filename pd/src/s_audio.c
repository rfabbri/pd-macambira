/* Copyright (c) 2003, Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  machine-independent (well, mostly!) audio layer.  Stores and recalls
    audio settings from argparse routine and from dialog window. 
*/


#include "m_pd.h"
#include "s_stuff.h"
#include <stdio.h>
#ifdef UNIX
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SYS_DEFAULTCH 2
#define SYS_MAXCH 100
#define SYS_DEFAULTSRATE 44100
typedef long t_pa_sample;
#define SYS_SAMPLEWIDTH sizeof(t_pa_sample)
#define SYS_BYTESPERCHAN (DEFDACBLKSIZE * SYS_SAMPLEWIDTH) 
#define SYS_XFERSAMPS (SYS_DEFAULTCH*DEFDACBLKSIZE)
#define SYS_XFERSIZE (SYS_SAMPLEWIDTH * SYS_XFERSAMPS)
#define MAXAUDIODEV 4

int sys_inchannels;
int sys_outchannels;
int sys_advance_samples;    	/* scheduler advance in samples */
int sys_blocksize = 256;  	/* audio I/O block size in sample frames */
int sys_audioapi = API_DEFAULT;

static int sys_meters;    	/* true if we're metering */
static float sys_inmax;    	/* max input amplitude */
static float sys_outmax;    	/* max output amplitude */

    /* exported variables */
#ifdef MSW
#define DEFAULTADVANCE 70000
#else
#define DEFAULTADVANCE 50000
#endif
int sys_schedadvance = DEFAULTADVANCE; 	/* scheduler advance in microseconds */
float sys_dacsr;
int sys_hipriority = 0;
t_sample *sys_soundout;
t_sample *sys_soundin;

    /* set channels and sample rate.  */

static void sys_setchsr(int chin, int chout, int sr)
{
    int nblk;
    int inbytes = chin * (DEFDACBLKSIZE*sizeof(float));
    int outbytes = chout * (DEFDACBLKSIZE*sizeof(float));

    sys_inchannels = chin;
    sys_outchannels = chout;
    sys_dacsr = sr;
    sys_advance_samples = (sys_schedadvance * sys_dacsr) / (1000000.);
    if (sys_advance_samples < 3 * DEFDACBLKSIZE)
    	sys_advance_samples = 3 * DEFDACBLKSIZE;

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
    	    sys_inchannels, sys_outchannels);
}

/* ----------------------- public routines ----------------------- */

#if 0
void sys_open_audio(int naudioindev, int *audioindev, int nchindev,
    int *chindev, int naudiooutdev, int *audiooutdev, int nchoutdev,
    int *choutdev, int rate) /* IOhannes */
{
    int inchans=
    	(nchindev > 0 ? chindev[0] : (nchindev == 0 ? 0 : SYS_DEFAULTCH));
    int outchans=
    	(nchoutdev > 0 ? choutdev[0] : (nchoutdev == 0 ? 0 : SYS_DEFAULTCH));
    int soundindev = (naudioindev <= 0 ? -1 : (audioindev[0]-1));
    int soundoutdev = (naudiooutdev <= 0 ? -1 : (audiooutdev[0]-1));
    int sounddev = (inchans > 0 ? soundindev : soundoutdev);
    if (naudioindev > 1 || nchindev > 1 || naudiooutdev > 1 || nchoutdev > 1)
	post("sorry, only one portaudio device can be open at once.\n");
    /* post("nindev %d, noutdev %d", naudioindev, naudiooutdev);
    post("soundindev %d, soundoutdev %d", soundindev, soundoutdev); */
    if (sys_verbose)
    	post("channels in %d, out %d", inchans, outchans);
    if (rate < 1)
    	rate = SYS_DEFAULTSRATE;
    sys_setchsr(inchans, outchans, rate);
    pa_open_audio(inchans, outchans, rate, sys_soundin, sys_soundout,
    	sys_blocksize, sys_advance_samples/sys_blocksize,
	    soundindev, soundoutdev);
}
#endif

void sys_open_audio(int naudioindev, int *audioindev, int nchindev,
    int *chindev, int naudiooutdev, int *audiooutdev, int nchoutdev,
    int *choutdev, int rate) /* IOhannes */
{
    int i, *ip;
    int defaultchannels = SYS_DEFAULTCH;
    int inchans, outchans;
    if (rate < 1)
    	rate = SYS_DEFAULTSRATE;

    if (naudioindev == -1)
    { 	    	/* not set */
	if (nchindev == -1)
	{
	    nchindev=1;
	    chindev[0] = defaultchannels;
	    naudioindev = 1;
	    audioindev[0] = DEFAULTAUDIODEV;
	}
	else
	{
	    for (i = 0; i < MAXAUDIODEV; i++)
      	        audioindev[i] = i+1;
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
		    audioindev[0] = DEFAULTAUDIODEV;
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
	if (nchoutdev == -1)
	{
	    nchoutdev=1;
	    choutdev[0]=defaultchannels;
	    naudiooutdev=1;
	    audiooutdev[0] = DEFAULTAUDIODEV;
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
		    audiooutdev[0] = DEFAULTAUDIODEV;
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
    for (i = inchans = 0; i < naudioindev; i++)
    	inchans += chindev[i];
    for (i = outchans = 0; i < naudiooutdev; i++)
    	outchans += choutdev[i];

    sys_setchsr(inchans, outchans, rate);
#ifdef USEAPI_OSS
    if (sys_audioapi == API_OSS)
	oss_open_audio(naudioindev, audioindev, nchindev, chindev,
    		naudiooutdev, audiooutdev, nchoutdev, choutdev, rate);
    else
#endif
#ifdef USEAPI_ALSA
    	/* for alsa, the "device numbers" are ignored; they are sent
	straight to the alsa code via linux_alsa_devname().  Only one
	device is supported for each of input, output. */
    if (sys_audioapi == API_ALSA)
	alsa_open_audio((naudioindev > 0 ? chindev[0] : 0),
	    (naudiooutdev > 0 ? choutdev[0] : 0), rate);
    else 
#endif
#ifdef USEAPI_PORTAUDIO
    if (sys_audioapi == API_PORTAUDIO)
	pa_open_audio(inchans, outchans, rate, sys_soundin, sys_soundout,
	    sys_blocksize, sys_advance_samples/sys_blocksize,
		(naudiooutdev > 0 ? audioindev[0] : 0),
		    (naudiooutdev > 0 ? audiooutdev[0] : 0));
    else
#endif
#ifdef USEAPI_MMIO
    if (sys_audioapi == API_MMIO)
	mmio_open_audio(naudioindev, audioindev, nchindev, chindev,
    		naudiooutdev, audiooutdev, nchoutdev, choutdev, rate);
    else
#endif
    post("unknown audio API specified");
}

void sys_close_audio(void)
{

#ifdef USEAPI_PORTAUDIO
    if (sys_audioapi == API_PORTAUDIO)
    	pa_close_audio();
    else 
#endif
#ifdef USEAPI_OSS
    if (sys_audioapi == API_OSS)
    	oss_close_audio();
    else
#endif
#ifdef USEAPI_ALSA
    if (sys_audioapi == API_ALSA)
    	alsa_close_audio();
    else
#ifdef USEAPI_MMIO
    if (sys_audioapi == API_MMIO)
    	mmio_close_audio();
    else
#endif
#endif
    post("unknown API");    

}

int sys_send_dacs(void)
{
    if (sys_meters)
    {
    	int i, n;
	float maxsamp;
	for (i = 0, n = sys_inchannels * DEFDACBLKSIZE, maxsamp = sys_inmax;
	    i < n; i++)
	{
	    float f = sys_soundin[i];
	    if (f > maxsamp) maxsamp = f;
	    else if (-f > maxsamp) maxsamp = -f;
	}
	sys_inmax = maxsamp;
	for (i = 0, n = sys_outchannels * DEFDACBLKSIZE, maxsamp = sys_outmax;
	    i < n; i++)
	{
	    float f = sys_soundout[i];
	    if (f > maxsamp) maxsamp = f;
	    else if (-f > maxsamp) maxsamp = -f;
	}
	sys_outmax = maxsamp;
    }

#ifdef USEAPI_PORTAUDIO
    if (sys_audioapi == API_PORTAUDIO)
    	return (pa_send_dacs());
    else 
#endif
#ifdef USEAPI_OSS
    if (sys_audioapi == API_OSS)
    	return (oss_send_dacs());
    else
#endif
#ifdef USEAPI_ALSA
    if (sys_audioapi == API_ALSA)
    	return (alsa_send_dacs());
    else
#endif
#ifdef USEAPI_MMIO
    if (sys_audioapi == API_MMIO)
    	return (mmio_send_dacs());
    else
#endif
    post("unknown API");    
    return (0);
}

float sys_getsr(void)
{
     return (sys_dacsr);
}

int sys_get_outchannels(void)
{
     return (sys_outchannels); 
}

int sys_get_inchannels(void) 
{
     return (sys_inchannels);
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
    	sys_meters = 1;
	*inmax = sys_inmax;
	*outmax = sys_outmax;
    }
    else
    	sys_meters = 0;
    sys_inmax = sys_outmax = 0;
}

void sys_reportidle(void)
{
}

void sys_listdevs(void )
{
#ifdef USEAPI_PORTAUDIO
    if (sys_audioapi == API_PORTAUDIO)
    	pa_listdevs();
    else 
#endif
#ifdef USEAPI_OSS
    if (sys_audioapi == API_OSS)
    	oss_listdevs();
    else
#endif
#ifdef USEAPI_MMIO
    if (sys_audioapi == API_MMIO)
    	mmio_listdevs();
    else
#endif
#ifdef USEAPI_ALSA
    if (sys_audioapi == API_ALSA)
    	alsa_listdevs();
    else
#endif
    post("unknown API");    

    sys_listmididevs();
}

void sys_setblocksize(int n)
{
    if (n < 1)
    	n = 1;
    if (n != (1 << ilog2(n)))
    	post("warning: adjusting blocksize to power of 2: %d", 
	    (n = (1 << ilog2(n))));
    sys_blocksize = n;
}

void sys_set_sound_api(int which)
{
     sys_audioapi = which;
     if (sys_verbose)
     	post("sys_audioapi %d", sys_audioapi);
}

