/* Copyright (c) 1997-2001 Guenter Geiger, Miller Puckette, Larry Troxler,
* Winfried Ritsch, Karl MacMillan, and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* this file implements the sys_ functions profiled in m_imp.h for
 audio and MIDI I/O on Macintosh OS X. 
 
 Audio simply calls routines in s_portaudio.c, which in turn call the
 portaudio package.  s_portaudio.c is also intended for use from NT.
 
 MIDI is handled by "portmidi".
*/


#include "m_imp.h"
#include <stdio.h>
#ifdef UNIX
#include <unistd.h>
#endif
#ifndef MACOSX
#include <stdlib.h>
#else
#include <stdlib.h>
#include "portaudio.h"
#include "portmidi.h"
#include "porttime.h"
#include "pminternal.h"
#endif
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>

/* Defines */
#define DEBUG(x) x
#define DEBUG2(x) {x;}

#define PA_DEFAULTCH 2	    /* portaudio specific? */
#define PA_MAXCH 100
#define PA_DEFAULTSRATE 44100
typedef short t_pa_sample;
#define PA_SAMPLEWIDTH sizeof(t_pa_sample)
#define PA_BYTESPERCHAN (DACBLKSIZE * PA_SAMPLEWIDTH) 
#define PA_XFERSAMPS (PA_DEFAULTCH*DACBLKSIZE)
#define PA_XFERSIZE (PA_SAMPLEWIDTH * PA_XFERSAMPS)

static int mac_whichapi = API_PORTAUDIO;
static int mac_inchannels;
static int mac_outchannels;
static int mac_advance_samples; /* scheduler advance in samples */
static int mac_meters;    	/* true if we're metering */
static float mac_inmax;    	/* max input amplitude */
static float mac_outmax;    	/* max output amplitude */
static int mac_blocksize = 256;   /* audio I/O block size in sample frames */

    /* exported variables */
int sys_schedadvance = 50000; 	/* scheduler advance in microseconds */
float sys_dacsr;
int sys_hipriority = 0;
t_sample *sys_soundout;
t_sample *sys_soundin;

static PmStream *mac_midiindevlist[MAXMIDIINDEV];
static PmStream *mac_midioutdevlist[MAXMIDIOUTDEV];
static int mac_nmidiindev;
static int mac_nmidioutdev;

    /* set channels and sample rate.  */

static void mac_setchsr(int chin, int chout, int sr)
{
    int nblk;
    int inbytes = chin * (DACBLKSIZE*sizeof(float));
    int outbytes = chout * (DACBLKSIZE*sizeof(float));

    mac_inchannels = chin;
    mac_outchannels = chout;
    sys_dacsr = sr;
    mac_advance_samples = (sys_schedadvance * sys_dacsr) / (1000000.);
    if (mac_advance_samples < 3 * DACBLKSIZE)
    	mac_advance_samples = 3 * DACBLKSIZE;

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
    	    mac_inchannels, mac_outchannels);
}

/* ----------------------- public routines ----------------------- */

void sys_open_audio(int naudioindev, int *audioindev, int nchindev,
    int *chindev, int naudiooutdev, int *audiooutdev, int nchoutdev,
    int *choutdev, int rate) /* IOhannes */
{
    int inchans=
    	(nchindev > 0 ? chindev[0] : (nchindev == 0 ? 0 : PA_DEFAULTCH));
    int outchans=
    	(nchoutdev > 0 ? choutdev[0] : (nchoutdev == 0 ? 0 : PA_DEFAULTCH));
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
    	rate = PA_DEFAULTSRATE;
    mac_setchsr(inchans, outchans, rate);
    pa_open_audio(inchans, outchans, rate, sys_soundin, sys_soundout,
    	mac_blocksize, mac_advance_samples/mac_blocksize,
	    soundindev, soundoutdev);
}

void sys_close_audio(void)
{
    pa_close_audio();
}


int sys_send_dacs(void)
{
    if (mac_meters)
    {
    	int i, n;
	float maxsamp;
	for (i = 0, n = mac_inchannels * DACBLKSIZE, maxsamp = mac_inmax;
	    i < n; i++)
	{
	    float f = sys_soundin[i];
	    if (f > maxsamp) maxsamp = f;
	    else if (-f > maxsamp) maxsamp = -f;
	}
	mac_inmax = maxsamp;
	for (i = 0, n = mac_outchannels * DACBLKSIZE, maxsamp = mac_outmax;
	    i < n; i++)
	{
	    float f = sys_soundout[i];
	    if (f > maxsamp) maxsamp = f;
	    else if (-f > maxsamp) maxsamp = -f;
	}
	mac_outmax = maxsamp;
    }
    return pa_send_dacs();
}

float sys_getsr(void)
{
     return (sys_dacsr);
}

int sys_get_outchannels(void)
{
     return (mac_outchannels); 
}

int sys_get_inchannels(void) 
{
     return (mac_inchannels);
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
    	mac_meters = 1;
	*inmax = mac_inmax;
	*outmax = mac_outmax;
    }
    else
    	mac_meters = 0;
    mac_inmax = mac_outmax = 0;
}

void sys_reportidle(void)
{
}

void sys_open_midi(int nmidiin, int *midiinvec,
    int nmidiout, int *midioutvec)
{
    int i = 0;
    int n = 0;
    PmError err;
    
    Pt_Start(1, 0, 0); /* start a timer with millisecond accuracy */
    mac_nmidiindev = 0;
    
    for (i = 0; i < nmidiin; i++)
    {
    	err = Pm_OpenInput(&mac_midiindevlist[mac_nmidiindev], midiinvec[i],
	    NULL, 100, NULL, NULL, NULL);
    	if (err)
            post("could not open midi input device number %d: %s",
	    	midiinvec[i], Pm_GetErrorText(err));
	else
	{
	    if (sys_verbose)
    	    	post("Midi Input opened.\n");
	    mac_nmidiindev++;
    	}
    }

    mac_nmidioutdev = 0;
    for (i = 0; i < nmidiout; i++)
    {
    	err = Pm_OpenOutput(&mac_midioutdevlist[mac_nmidioutdev], midioutvec[i],
    	     NULL, 0, NULL, NULL, 0);
    	if (err)
            post("could not open midi output device number %d: %s",
	    	midioutvec[i], Pm_GetErrorText(err));
	else
	{
	    if (sys_verbose)
    	    	post("Midi Output opened.\n");
	    mac_nmidioutdev++;
    	}
    }
}

void sys_close_midi( void)
{
    int i;
    for (i = 0; i < mac_nmidiindev; i++)
    	Pm_Close(mac_midiindevlist[mac_nmidiindev]);
    mac_nmidiindev = 0;
    for (i = 0; i < mac_nmidioutdev; i++)
    	Pm_Close(mac_midioutdevlist[mac_nmidioutdev]);
    mac_nmidioutdev = 0; 
}

void sys_putmidimess(int portno, int a, int b, int c)
{
    PmEvent buffer;
        fprintf(stderr, "put 1 msg %d %d\n", portno, mac_nmidioutdev);
    if (portno >= 0 && portno < mac_nmidioutdev)
    {
    	buffer.message = Pm_Message(a, b, c);
    	buffer.timestamp = 0;
        fprintf(stderr, "put msg\n");
        Pm_Write(mac_midioutdevlist[portno], &buffer, 1);
    }
}

void sys_putmidibyte(int portno, int byte)
{
    post("sorry, no byte-by-byte MIDI output implemented in MAC OSX");
}

void sys_poll_midi(void)
{
    int i, nmess;
    PmEvent buffer;
    for (i = 0; i < mac_nmidiindev; i++)
    {
    	int nmess = Pm_Read(mac_midiindevlist[i], &buffer, 1);
    	if (nmess > 0)
	{
            int status = Pm_MessageStatus(buffer.message);
            int data1  = Pm_MessageData1(buffer.message);
            int data2  = Pm_MessageData2(buffer.message);
            int msgtype = (status >> 4) - 8;
	    switch (msgtype)
    	    {
    	    case 0: 
    	    case 1: 
    	    case 2:
    	    case 3:
    	    case 6:
		sys_midibytein(i, status);
		sys_midibytein(i, data1);
		sys_midibytein(i, data2);
		break; 
    	    case 4:
    	    case 5:
		sys_midibytein(i, status);
		sys_midibytein(i, data1);
		break;
	    case 7:
		sys_midibytein(i, status);
		break; 
	    }
    	}
    }
}

void sys_set_priority(int higher) 
{
    int retval;
    errno = 0;
    retval = setpriority(PRIO_PROCESS, 0, (higher? -20 : -19));
    if (retval == -1 & errno != 0)
    {
        perror("setpriority");
        fprintf(stderr, "priority bost faled.\n");
    }
}

void sys_listdevs(void )
{
    pa_listdevs();
}

void sys_setblocksize(int n)
{
    if (n < 1)
    	n = 1;
    if (n != (1 << ilog2(n)))
    	warn("blocksize adjusted to power of 2: %d", 
	    (n = (1 << ilog2(n))));
    mac_blocksize = n;
}

    /* dummy stuff that shouldn't he here */
void nt_soundindev(int which)
{
}

void nt_soundoutdev(int which)
{
}

void nt_midiindev(int which)
{
}

void nt_midioutdev(int which)
{
}

void nt_noresync(void )
{
}

void glob_audio(void *dummy, t_floatarg fadc, t_floatarg fdac)
{
}
