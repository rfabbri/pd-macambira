/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_imp.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#ifdef HAVE_BSTRING_H
#include <bstring.h>
#endif
#include <sys/types.h>
#include <sys/time.h>

#include <dmedia/audio.h>
#include <sys/fpu.h>
#include <dmedia/midi.h>
int mdInit(void);   	    	/* prototype was messed up in midi.h */
/* #include "sys/select.h" */

static ALport iport;
static ALport oport;
static ALconfig sgi_config;
#define DEFAULTCHANS 2
#define SGI_MAXCH 8
static int sys_inchannels, sys_outchannels;
static int sys_audiobufsamps;
int sys_schedadvance = 50000;	/* scheduler advance in microseconds */
    	/* (this is set ridiculously high until we can get the real-time
    	scheduling act together.) */
int sys_hipriority = 0;
static int sgi_meters;        /* true if we're metering */
static float sgi_inmax;       /* max input amplitude */
static float sgi_outmax;      /* max output amplitude */


  /*
    set the special "flush zero" but (FS, bit 24) in the
    Control Status Register of the FPU of R4k and beyond
    so that the result of any underflowing operation will
    be clamped to zero, and no exception of any kind will
    be generated on the CPU.
    
    thanks to cpirazzi@cp.esd.sgi.com (Chris Pirazzi).
  */

static void sgi_flush_all_underflows_to_zero(void)
{
    union fpc_csr f;
    f.fc_word = get_fpc_csr();
    f.fc_struct.flush = 1;
    set_fpc_csr(f.fc_word);
}

static void sgi_setchsr(int inchans, int outchans, int sr)
{
    int inbytes = inchans * (DACBLKSIZE*sizeof(float));
    int outbytes = outchans * (DACBLKSIZE*sizeof(float));

    sys_audiobufsamps = 64 * (int)(((float)sys_schedadvance)* sr / 64000000.);
    sys_inchannels = inchans;
    sys_outchannels = outchans;
    sys_dacsr = sr;

    if (sys_soundin)
        free(sys_soundin);
    sys_soundin = (t_float *)malloc(inbytes);
    memset(sys_soundin, 0, inbytes);

    if (sys_soundout)
        free(sys_soundout);
    sys_soundout = (t_float *)malloc(outbytes);
    memset(sys_soundout, 0, outbytes);

    memset(sys_soundout, 0, sys_outchannels * (DACBLKSIZE*sizeof(float)));
    memset(sys_soundin, 0, sys_inchannels * (DACBLKSIZE*sizeof(float)));
}

static void sgi_open_audio(void)
{  
    long pvbuf[4];
    long pvbuflen;
    /*  get current sample rate -- should use this to set logical SR */
    pvbuf[0] = AL_INPUT_RATE;
    pvbuf[2] = AL_OUTPUT_RATE;
    pvbuflen = 4;
    
    ALgetparams(AL_DEFAULT_DEVICE, pvbuf, pvbuflen);

    if (sys_inchannels && pvbuf[1] != sys_dacsr)
    	post("warning: input sample rate (%d) doesn't match mine (%f)\n",
    	    pvbuf[1], sys_dacsr);
    
    if (sys_outchannels && pvbuf[3] != sys_dacsr)
    	post("warning: output sample rate (%d) doesn't match mine (%f)\n",
    	    pvbuf[3], sys_dacsr);
    
    pvbuf[3] = pvbuf[1];
    ALsetparams(AL_DEFAULT_DEVICE, pvbuf, pvbuflen);

    sgi_config = ALnewconfig();

    ALsetsampfmt(sgi_config, AL_SAMPFMT_FLOAT);

    if (sys_outchannels)
    {
    	ALsetchannels(sgi_config, sys_outchannels);
    	ALsetqueuesize(sgi_config, sys_audiobufsamps * sys_outchannels);
    	oport = ALopenport("the ouput port", "w", sgi_config);
	if (!oport)
	    fprintf(stderr,"Pd: failed to open audio write port\n");
    }
    else oport = 0;    
    if (sys_inchannels)
    {
    	ALsetchannels(sgi_config, sys_inchannels);
    	ALsetqueuesize(sgi_config, sys_audiobufsamps * sys_inchannels);
    	iport = ALopenport("the input port", "r", sgi_config);
	if (!iport)
	    fprintf(stderr,"Pd: failed to open audio read port\n");
    }
    else iport = 0;
}

void sys_close_audio( void)
{
    if (iport) ALcloseport(iport);
    if (oport) ALcloseport(oport);
}

void sys_close_midi( void)
{
    /* ??? */
}

t_sample *sys_soundout;
t_sample *sys_soundin;
float sys_dacsr;

int sys_send_dacs(void)
{
    float buf[SGI_MAXCH * DACBLKSIZE], *fp1, *fp2, *fp3, *fp4;
    long outfill, infill;
    int outchannels = sys_outchannels, inchannels = sys_inchannels;
    int i, nwait = 0, channel;
    int outblk = DACBLKSIZE * outchannels;
    int inblk = DACBLKSIZE * inchannels;
    outfill = ALgetfillable(oport);
    if (sgi_meters)
    {
        int i, n;
        float maxsamp;
        for (i = 0, n = sys_inchannels * DACBLKSIZE, maxsamp = sgi_inmax;
            i < n; i++)
        {
            float f = sys_soundin[i];
            if (f > maxsamp) maxsamp = f;
            else if (-f > maxsamp) maxsamp = -f;
        }
        sgi_inmax = maxsamp;
        for (i = 0, n = sys_outchannels * DACBLKSIZE, maxsamp = sgi_outmax;
            i < n; i++)
        {
            float f = sys_soundout[i];
            if (f > maxsamp) maxsamp = f;
            else if (-f > maxsamp) maxsamp = -f;
        }
        sgi_outmax = maxsamp;
    }

    if (outfill <= outblk)
    {
    	while ((infill = ALgetfilled(iport)) > 2*inblk)
    	{
    	    if (sys_verbose) post("drop ADC buf");
	    ALreadsamps(iport, buf, inblk);
    	    return (0);
    	}
    }
    if (outchannels)
    {
	for (channel = 0, fp1 = buf, fp2 = sys_soundout;
    	    channel < outchannels; channel++, fp1++, fp2 += DACBLKSIZE)
	{
	    for (i = 0, fp3 = fp1, fp4 = fp2; i < DACBLKSIZE;
		i++, fp3 += outchannels, fp4++)
	    	    *fp3 = *fp4, *fp4 = 0;
	}
	ALwritesamps(oport, buf, outchannels* DACBLKSIZE);
    }
    if (inchannels)
    {
	if (infill > inblk)
	    ALreadsamps(iport, buf, inchannels* DACBLKSIZE);
	else
	{
	    if (sys_verbose) post("extra ADC buf");
	    memset(buf, 0, inblk*sizeof(float));
	}
	for (channel = 0, fp1 = buf, fp2 = sys_soundin;
    	    channel < inchannels; channel++, fp1++, fp2 += DACBLKSIZE)
	{
	    for (i = 0, fp3 = fp1, fp4 = fp2; i < DACBLKSIZE;
		i++, fp3 += inchannels, fp4++)
	    	    *fp4 = *fp3;
	}
    }
    return (1);
}

/* ------------------------- MIDI -------------------------- */

#define NPORT 2

static MDport sgi_inport[NPORT];
static MDport sgi_outport[NPORT];

void sgi_open_midi(int midiin, int midiout)
{
    int i;
    int sgi_nports = mdInit();
    if (sgi_nports < 0) sgi_nports = 0;
    else if (sgi_nports > NPORT) sgi_nports = NPORT;
    if (sys_verbose)
    {
	if (!sgi_nports) 
	{
	    post("no serial ports are configured for MIDI;");
	    post("if you want to use MIDI, try exiting Pd, typing");
	    post("'startmidi -d /dev/ttyd2' to a shell, and restarting Pd.");
	}
	else if (sgi_nports == 1)
	    post("Found one MIDI port on %s", mdGetName(0));
	else if (sgi_nports == 2)
	    post("Found MIDI ports on %s and %s",
    	    	mdGetName(0), mdGetName(1));
    }
    if (midiin)
    {
    	for (i = 0; i < sgi_nports; i++)
    	{
    	    if (!(sgi_inport[i] = mdOpenInPort(mdGetName(i)))) 
    	    	error("MIDI input port %d: open failed", i+1);;
    	}
    }
    if (midiout)
    {
    	for (i = 0; i < sgi_nports; i++)
    	{
    	    if (!(sgi_outport[i] = mdOpenOutPort(mdGetName(i))))
    	    	error("MIDI output port %d: open failed", i+1);;
    	}
    }
    return;
}

void sys_putmidimess(int portno, int a, int b, int c)
{
    MDevent mdv;
    if (portno >= NPORT || portno < 0 || !sgi_outport[portno]) return;
    mdv.msg[0] = a;
    mdv.msg[1] = b;
    mdv.msg[2] = c;
    mdv.msg[3] = 0;
    mdv.sysexmsg = 0;
    mdv.stamp = 0;
    mdv.msglen = 0;
    if (mdSend(sgi_outport[portno], &mdv, 1) < 0)
    	error("MIDI output error\n");
    post("msg out %d %d %d", a, b, c);
}

void sys_putmidibyte(int portno, int foo)
{
    error("MIDI raw byte output not available on SGI");
}

void inmidi_noteon(int portno, int channel, int pitch, int velo);
void inmidi_controlchange(int portno, int channel, int ctlnumber, int value);
void inmidi_programchange(int portno, int channel, int value);
void inmidi_pitchbend(int portno, int channel, int value);
void inmidi_aftertouch(int portno, int channel, int value);
void inmidi_polyaftertouch(int portno, int channel, int pitch, int value);

void sys_poll_midi(void)
{
    int i;
    MDport *mp;
    for (i = 0, mp = sgi_inport; i < NPORT; i++, mp++)
    {
	int ret,  status,  b1,  b2, nfds;
	MDevent mdv;
	fd_set inports;
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
    	if (!*mp) continue;
	FD_ZERO(&inports);
	FD_SET(mdGetFd(*mp), &inports);

	if (select(mdGetFd(*mp)+1 , &inports, 0, 0, &timeout) < 0)
	    perror("midi select");
	if (FD_ISSET(mdGetFd(*mp),&inports))
	{
    	    if (mdReceive(*mp, &mdv, 1) < 0)
		error("failure receiving message\n");
	    else if (mdv.msg[0] == MD_SYSEX) mdFree(mdv.sysexmsg);

	    else
	    {
	    	int status = mdv.msg[0];
	    	int channel = (status & 0xf) + 1;
	    	int b1 = mdv.msg[1];
	    	int b2 = mdv.msg[2];
		switch(status & 0xf0)
		{
		case MD_NOTEOFF:
		    inmidi_noteon(i, channel, b1, 0);
		    break;
		case MD_NOTEON:
		    inmidi_noteon(i, channel, b1, b2);
		    break;
		case MD_POLYKEYPRESSURE:
		    inmidi_polyaftertouch(i, channel, b1, b2);
		    break;
		case MD_CONTROLCHANGE:
		    inmidi_controlchange(i, channel, b1, b2);
		    break;
		case MD_PITCHBENDCHANGE:
		    inmidi_pitchbend(i, channel, ((b2 << 7) + b1));
		    break;
		case MD_PROGRAMCHANGE:
		    inmidi_programchange(i, channel, b1);
		    break;
		case MD_CHANNELPRESSURE:
		    inmidi_aftertouch(i, channel, b1);
		    break;
		}
	    }
	}
    }
}

    /* public routines */

void sys_open_audio(int naudioindev, int *audioindev, int nchindev, int *chindev,
		    int naudiooutdev, int *audiooutdev, int nchoutdev, int *choutdev,
		    int rate) /* IOhannes */
{
  int inchans=0;
  int outchans=0;
  if (nchindev<0)inchans == -1;
  else {
    int i=nchindev;
    int *l=chindev;
    while(i--)inchans+=*l++;
  }
  if (nchoutdev<0)outchans == -1;
  else {
    int i=nchoutdev;
    int *l=choutdev;
    while(i--)outchans+=*l++;
  }

    if (inchans < 0) inchans = DEFAULTCHANS;
    if (outchans < 0) outchans = DEFAULTCHANS;
    if (inchans > SGI_MAXCH) inchans = SGI_MAXCH;
    if (outchans > SGI_MAXCH) outchans = SGI_MAXCH;

    sgi_setchsr(inchans, outchans, rate);
    sgi_flush_all_underflows_to_zero();
    sgi_open_audio();
}

void sys_open_midi(int nmidiin, int *midiinvec,
    int nmidiout, int *midioutvec)
{
    sgi_open_midi(nmidiin!=0, nmidiout!=0);
}

float sys_getsr(void)
{
    return (sys_dacsr);
}

void sys_audiobuf(int n)
{
    /* set the size, in milliseconds, of the audio FIFO */
    if (n < 5) n = 5;
    else if (n > 5000) n = 5000;
    fprintf(stderr, "audio buffer set to %d milliseconds\n", n);
    sys_schedadvance = n * 1000;
}

void sys_getmeters(float *inmax, float *outmax)
{
    if (inmax)
    {
        sgi_meters = 1;
        *inmax = sgi_inmax;
        *outmax = sgi_outmax;
    }
    else
        sgi_meters = 0;
    sgi_inmax = sgi_outmax = 0;
}

void sys_reportidle(void)
{
}

int sys_get_inchannels(void)
{
    return (sys_inchannels);
}

int sys_get_outchannels(void)
{
    return (sys_outchannels);
}

void sys_set_priority(int foo)
{
    fprintf(stderr,
    	"warning: priority boosting in IRIX not implemented yet\n");
}

void sys_setblocksize(int n)
{
    fprintf(stderr,
    	"warning: blocksize not settable in IRIX\n");
}
