/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* modified 2/98 by Winfried Ritsch to deal with up to 4 synchronized
"wave" devices, which is how ADAT boards appear to the WAVE API. */

#include "m_imp.h"
#include <stdio.h>

#include <windows.h>

#include <MMSYSTEM.H>

/* ------------------------- audio -------------------------- */

static void nt_close_midiin(void);

static void postflags(void);

#define NAPORTS 16   /* wini hack for multiple ADDA devices  */
#define NT_MAXCH (2 * NAPORTS)
#define CHANNELS_PER_DEVICE 2
#define DEFAULTCHANS 2
#define DEFAULTSRATE 44100
#define SAMPSIZE 2

#define REALDACBLKSIZE (4 * DACBLKSIZE) /* larger underlying bufsize */

#define MAXBUFFER 100   /* number of buffers in use at maximum advance */
#define DEFBUFFER 30	/* default is about 30x6 = 180 msec! */
static int nt_naudiobuffer = DEFBUFFER;
static int nt_advance_samples;

float sys_dacsr = DEFAULTSRATE;

static int nt_whichapi = API_MMIO;
static int nt_meters;        /* true if we're metering */
static float nt_inmax;       /* max input amplitude */
static float nt_outmax;      /* max output amplitude */
static int nt_nwavein, nt_nwaveout; 	/* number of WAVE devices in and out */
static int nt_blocksize = 0; /* audio I/O block size in sample frames */
int sys_schedadvance = 20000;	/* scheduler advance in microseconds */

typedef struct _sbuf
{
    HANDLE hData;
    HPSTR  lpData;      // pointer to waveform data memory 
    HANDLE hWaveHdr; 
    WAVEHDR   *lpWaveHdr;   // pointer to header structure
} t_sbuf;

t_sbuf ntsnd_outvec[NAPORTS][MAXBUFFER];    /* circular buffer array */
HWAVEOUT ntsnd_outdev[NAPORTS];  	    /* output device */
static int ntsnd_outphase[NAPORTS]; 	    /* index of next buffer to send */

t_sbuf ntsnd_invec[NAPORTS][MAXBUFFER];	    /* circular buffer array */
HWAVEIN ntsnd_indev[NAPORTS];  	    	    /* input device */
static int ntsnd_inphase[NAPORTS]; 	    /* index of next buffer to read */
int sys_hipriority = 0;

static void nt_waveinerror(char *s, int err)
{
    char t[256];
    waveInGetErrorText(err, t, 256);
    fprintf(stderr, s, t);
}

static void nt_waveouterror(char *s, int err)
{
    char t[256];
    waveOutGetErrorText(err, t, 256);
    fprintf(stderr, s, t);
}

static void wave_prep(t_sbuf *bp)
{
    WAVEHDR *wh;
    short *sp;
    int i;
    /* 
     * Allocate and lock memory for the waveform data. The memory 
     * for waveform data must be globally allocated with 
     * GMEM_MOVEABLE and GMEM_SHARE flags. 
     */ 

    if (!(bp->hData =
	GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
	    (DWORD) (CHANNELS_PER_DEVICE * REALDACBLKSIZE * SAMPSIZE)))) 
	    	printf("alloc 1 failed\n");

    if (!(bp->lpData =
	(HPSTR) GlobalLock(bp->hData)))
	    printf("lock 1 failed\n");

    /*  Allocate and lock memory for the header.  */ 

    if (!(bp->hWaveHdr =
	GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD) sizeof(WAVEHDR))))
    	    printf("alloc 2 failed\n");

    if (!(wh = bp->lpWaveHdr =
	(WAVEHDR *) GlobalLock(bp->hWaveHdr))) 
	    printf("lock 2 failed\n");

    for (i = CHANNELS_PER_DEVICE * REALDACBLKSIZE,
    	sp = (short *)bp->lpData; i--; )
    	    *sp++ = 0;

    wh->lpData = bp->lpData;
    wh->dwBufferLength = (CHANNELS_PER_DEVICE * REALDACBLKSIZE * SAMPSIZE);
    wh->dwFlags = 0;
    wh->dwLoops = 0L;
    wh->lpNext = 0;
    wh->reserved = 0;
}

static int nt_inalloc[NAPORTS], nt_outalloc[NAPORTS];
static UINT nt_whichdac = WAVE_MAPPER, nt_whichadc = WAVE_MAPPER;

int mmio_open_audio(void)
{ 
    PCMWAVEFORMAT  form; 
    int i;
    UINT mmresult;
    int nad, nda;

    if (sys_verbose)
    	post("%d devices in, %d devices out",
    	    nt_nwavein, nt_nwaveout);

    form.wf.wFormatTag = WAVE_FORMAT_PCM;
    form.wf.nChannels = CHANNELS_PER_DEVICE;
    form.wf.nSamplesPerSec = sys_dacsr;
    form.wf.nAvgBytesPerSec = sys_dacsr * (CHANNELS_PER_DEVICE * SAMPSIZE);
    form.wf.nBlockAlign = CHANNELS_PER_DEVICE * SAMPSIZE;
    form.wBitsPerSample = 8 * SAMPSIZE;

    for (nad=0; nad < nt_nwavein; nad++)
    {
	/* Open waveform device(s), sucessively numbered, for input */

	mmresult = waveInOpen(&ntsnd_indev[nad], nt_whichadc+nad,
     		(WAVEFORMATEX *)(&form), 0L, 0L, CALLBACK_NULL);

	if (sys_verbose)
    	    printf("opened adc device %d with return %d\n",
    		nt_whichadc+nad,mmresult);

	if (mmresult != MMSYSERR_NOERROR) 
	{
	    nt_waveinerror("waveInOpen: %s\n", mmresult);
	    nt_nwavein = nad; /* nt_nwavein = 0 wini */
	} 
	else 
	{
	    if (!nt_inalloc[nad])
	    {
		for (i = 0; i < nt_naudiobuffer; i++)
	    	    wave_prep(&ntsnd_invec[nad][i]);
		nt_inalloc[nad] = 1;
	    }
	    for (i = 0; i < nt_naudiobuffer; i++)
	    {
		mmresult = waveInPrepareHeader(ntsnd_indev[nad],
	    	    ntsnd_invec[nad][i].lpWaveHdr, sizeof(WAVEHDR));
		if (mmresult != MMSYSERR_NOERROR)
	    	    nt_waveinerror("waveinprepareheader: %s\n", mmresult);
		mmresult = waveInAddBuffer(ntsnd_indev[nad],
	    	    ntsnd_invec[nad][i].lpWaveHdr, sizeof(WAVEHDR));
		if (mmresult != MMSYSERR_NOERROR)
	    	    nt_waveinerror("waveInAddBuffer: %s\n", mmresult);
	    }
	}
    }
    	/* quickly start them all together */
    for(nad=0; nad < nt_nwavein; nad++)
    	waveInStart(ntsnd_indev[nad]);

    for(nda=0; nda < nt_nwaveout; nda++)
    {
    	
    	    /* Open a waveform device for output in sucessiv device numbering*/
    	mmresult = waveOutOpen(&ntsnd_outdev[nda], nt_whichdac + nda,
    	    (WAVEFORMATEX *)(&form), 0L, 0L, CALLBACK_NULL);

    	if (sys_verbose)
	    fprintf(stderr,"opened dac device %d, with return %d\n",
	    	nt_whichdac +nda, mmresult);

    	if (mmresult != MMSYSERR_NOERROR)
    	{
	    fprintf(stderr,"Wave out open device %d + %d\n",nt_whichdac,nda);
            nt_waveouterror("waveOutOpen device: %s\n",  mmresult);
            nt_nwaveout = nda;
    	}
	else
	{
	    if (!(nt_outalloc[nda]))
	    {
	    	for (i = 0; i < nt_naudiobuffer; i++)
	    	{
    	    	    wave_prep(&ntsnd_outvec[nda][i]);
	    	    	    /* set DONE flag as if we had queued them */
    	    	    ntsnd_outvec[nda][i].lpWaveHdr->dwFlags = WHDR_DONE;
    	    	}
    	    	nt_outalloc[nda] = 1;
	    }
    	}
    }

    return (0);
}

void mmio_close_audio( void)
{
    int errcode;
    int nda, nad;
    if (sys_verbose)
    	post("closing audio...");

    for (nda=0; nda < nt_nwaveout; nda++) /*if (nt_nwaveout) wini */
    {
       errcode = waveOutReset(ntsnd_outdev[nda]);
       if (errcode != MMSYSERR_NOERROR)
	   printf("error resetting output %d: %d\n", nda, errcode);
       errcode = waveOutClose(ntsnd_outdev[nda]);
       if (errcode != MMSYSERR_NOERROR)
	   printf("error closing output %d: %d\n",nda , errcode);	   
    }
    nt_nwaveout = 0;

    for(nad=0; nad < nt_nwavein;nad++) /* if (nt_nwavein) wini */
    {
	errcode = waveInReset(ntsnd_indev[nad]);
	if (errcode != MMSYSERR_NOERROR)
	    printf("error resetting input: %d\n", errcode);
	errcode = waveInClose(ntsnd_indev[nad]);
	if (errcode != MMSYSERR_NOERROR)
	    printf("error closing input: %d\n", errcode);
    }
    nt_nwavein = 0;
}


#define ADCJITTER 10	/* We tolerate X buffers of jitter by default */
#define DACJITTER 10

static int nt_adcjitterbufsallowed = ADCJITTER;
static int nt_dacjitterbufsallowed = DACJITTER;

    /* ------------- MIDI time stamping from audio clock ------------ */

#ifdef MIDI_TIMESTAMP

static double nt_hibuftime;
static double initsystime = -1;

    /* call this whenever we reset audio */
static void nt_resetmidisync(void)
{
    initsystime = clock_getsystime();
    nt_hibuftime = sys_getrealtime();
}

    /* call this whenever we're idled waiting for audio to be ready. 
    The routine maintains a high and low water point for the difference
    between real and DAC time. */

static void nt_midisync(void)
{
    double jittersec, diff;
    
    if (initsystime == -1) nt_resetmidisync();
    jittersec = (nt_dacjitterbufsallowed > nt_adcjitterbufsallowed ?
    	nt_dacjitterbufsallowed : nt_adcjitterbufsallowed)
    	    * REALDACBLKSIZE / sys_getsr();
    diff = sys_getrealtime() - 0.001 * clock_gettimesince(initsystime);
    if (diff > nt_hibuftime) nt_hibuftime = diff;
    if (diff < nt_hibuftime - jittersec)
    {
    	post("jitter excess %d %f", dac, diff);
    	nt_resetmidisync();
    }
}

static double nt_midigettimefor(LARGE_INTEGER timestamp)
{
    /* this is broken now... used to work when "timestamp" was derived from
    	QueryPerformanceCounter() instead of the gates approved 
	    timeGetSystemTime() call in the MIDI callback routine below. */
    return (nt_tixtotime(timestamp) - nt_hibuftime);
}
#endif	    /* MIDI_TIMESTAMP */


static int nt_fill = 0;
#define WRAPFWD(x) ((x) >= nt_naudiobuffer ? (x) - nt_naudiobuffer: (x))
#define WRAPBACK(x) ((x) < 0 ? (x) + nt_naudiobuffer: (x))
#define MAXRESYNC 500

#if 0 	    /* this is used for debugging */
static void nt_printaudiostatus(void)
{
    int nad, nda;
    for (nad = 0; nad < nt_nwavein; nad++)
    {
    	int phase = ntsnd_inphase[nad];
    	int phase2 = phase, phase3 = WRAPFWD(phase2), count, ntrans = 0;
    	int firstphasedone = -1, firstphasebusy = -1;
    	for (count = 0; count < nt_naudiobuffer; count++)
    	{
    	    int donethis =
    	    	(ntsnd_invec[nad][phase2].lpWaveHdr->dwFlags & WHDR_DONE);
    	    int donenext =
    	    	(ntsnd_invec[nad][phase3].lpWaveHdr->dwFlags & WHDR_DONE);
    	    if (donethis && !donenext)
    	    {
    	    	if (firstphasebusy >= 0) goto multipleadc;
    	    	firstphasebusy = count;
    	    }
    	    if (!donethis && donenext)
    	    {
    	    	if (firstphasedone >= 0) goto multipleadc;
    	    	firstphasedone = count;
    	    }
    	    phase2 = phase3;
    	    phase3 = WRAPFWD(phase2 + 1);
    	}
    	post("nad %d phase %d busy %d done %d", nad, phase, firstphasebusy,
    	    firstphasedone);
    	continue;
    multipleadc:
	startpost("nad %d phase %d: oops:", nad, phase);
	for (count = 0; count < nt_naudiobuffer; count++)
	{
    	    char buf[80];
    	    sprintf(buf, " %d", 
    		(ntsnd_invec[nad][count].lpWaveHdr->dwFlags & WHDR_DONE));
    	    poststring(buf);
	}
	endpost();
    }
    for (nda = 0; nda < nt_nwaveout; nda++)
    {
    	int phase = ntsnd_outphase[nad];
    	int phase2 = phase, phase3 = WRAPFWD(phase2), count, ntrans = 0;
    	int firstphasedone = -1, firstphasebusy = -1;
    	for (count = 0; count < nt_naudiobuffer; count++)
    	{
    	    int donethis =
    	    	(ntsnd_outvec[nda][phase2].lpWaveHdr->dwFlags & WHDR_DONE);
    	    int donenext =
    	    	(ntsnd_outvec[nda][phase3].lpWaveHdr->dwFlags & WHDR_DONE);
    	    if (donethis && !donenext)
    	    {
    	    	if (firstphasebusy >= 0) goto multipledac;
    	    	firstphasebusy = count;
    	    }
    	    if (!donethis && donenext)
    	    {
    	    	if (firstphasedone >= 0) goto multipledac;
    	    	firstphasedone = count;
    	    }
    	    phase2 = phase3;
    	    phase3 = WRAPFWD(phase2 + 1);
    	}
    	if (firstphasebusy < 0) post("nda %d phase %d all %d",
    	    nda, phase, (ntsnd_outvec[nad][0].lpWaveHdr->dwFlags & WHDR_DONE));
    	else post("nda %d phase %d busy %d done %d", nda, phase, firstphasebusy,
    	    firstphasedone);
    	continue;
    multipledac:
	startpost("nda %d phase %d: oops:", nda, phase);
	for (count = 0; count < nt_naudiobuffer; count++)
	{
    	    char buf[80];
    	    sprintf(buf, " %d", 
    		(ntsnd_outvec[nad][count].lpWaveHdr->dwFlags & WHDR_DONE));
    	    poststring(buf);
	}
	endpost();
    }
}
#endif /* 0 */

/* this is a hack to avoid ever resyncing audio pointers in case for whatever
reason the sync testing below gives false positives. */

static int nt_resync_cancelled;

void nt_noresync( void)
{
    nt_resync_cancelled = 1;
}

static void nt_resyncaudio(void)
{
    UINT mmresult; 
    int nad, nda, count;
    if (nt_resync_cancelled)
    	return;
    	/* for each open input device, eat all buffers which are marked
    	    ready.  The next one will thus be "busy".  */
    post("resyncing audio");
    for (nad = 0; nad < nt_nwavein; nad++)
    {
    	int phase = ntsnd_inphase[nad];
    	for (count = 0; count < MAXRESYNC; count++)
    	{
	    WAVEHDR *inwavehdr = ntsnd_invec[nad][phase].lpWaveHdr;
	    if (!(inwavehdr->dwFlags & WHDR_DONE)) break;
	    if (inwavehdr->dwFlags & WHDR_PREPARED)
    		waveInUnprepareHeader(ntsnd_indev[nad],
    		    inwavehdr, sizeof(WAVEHDR)); 
	    inwavehdr->dwFlags = 0L;
	    waveInPrepareHeader(ntsnd_indev[nad], inwavehdr, sizeof(WAVEHDR)); 
	    mmresult = waveInAddBuffer(ntsnd_indev[nad], inwavehdr,
	    	sizeof(WAVEHDR));
	    if (mmresult != MMSYSERR_NOERROR)
        	nt_waveinerror("waveInAddBuffer: %s\n", mmresult);
	    ntsnd_inphase[nad] = phase = WRAPFWD(phase + 1);
    	}
    	if (count == MAXRESYNC) post("resync error 1");
    }
    	/* Each output buffer which is "ready" is filled with zeros and
    	queued. */
    for (nda = 0; nda < nt_nwaveout; nda++)
    {
    	int phase = ntsnd_outphase[nda];
    	for (count = 0; count < MAXRESYNC; count++)
    	{
	    WAVEHDR *outwavehdr = ntsnd_outvec[nda][phase].lpWaveHdr;
	    if (!(outwavehdr->dwFlags & WHDR_DONE)) break;
	    if (outwavehdr->dwFlags & WHDR_PREPARED)
    		waveOutUnprepareHeader(ntsnd_outdev[nda],
    		    outwavehdr, sizeof(WAVEHDR)); 
	    outwavehdr->dwFlags = 0L;
	    memset((char *)(ntsnd_outvec[nda][phase].lpData),
	    	0, (CHANNELS_PER_DEVICE * REALDACBLKSIZE * SAMPSIZE));
	    waveOutPrepareHeader(ntsnd_outdev[nda], outwavehdr,
	    	sizeof(WAVEHDR)); 
	    mmresult = waveOutWrite(ntsnd_outdev[nda], outwavehdr,
	    	sizeof(WAVEHDR)); 
	    if (mmresult != MMSYSERR_NOERROR)
        	nt_waveouterror("waveOutAddBuffer: %s\n", mmresult);
	    ntsnd_outphase[nda] = phase = WRAPFWD(phase + 1);
    	}
    	if (count == MAXRESYNC) post("resync error 2");
    }

#ifdef MIDI_TIMESTAMP
    nt_resetmidisync();
#endif

} 

#define LATE 0
#define RESYNC 1
#define NOTHING 2
static int nt_errorcount;
static int nt_resynccount;
static double nt_nextreporttime = -1;

void nt_logerror(int which)
{
#if 0
    post("error %d %d", count, which);
    if (which < NOTHING) nt_errorcount++;
    if (which == RESYNC) nt_resynccount++;
    if (sys_getrealtime() > nt_nextreporttime)
    {
    	post("%d audio I/O error%s", nt_errorcount,
    	    (nt_errorcount > 1 ? "s" : ""));
    	if (nt_resynccount) post("DAC/ADC sync error");
    	nt_errorcount = nt_resynccount = 0;
    	nt_nextreporttime = sys_getrealtime() - 5;
    }
#endif
}

/* system buffer with t_sample types for one tick */
t_sample *sys_soundout;
t_sample *sys_soundin;
float sys_dacsr;

int mmio_send_dacs(void)
{
    HMMIO hmmio; 
    UINT mmresult; 
    HANDLE hFormat; 
    int i, j;
    short *sp1, *sp2;
    float *fp1, *fp2;
    int nextfill, doxfer = 0;
    int nda, nad;
    if (!nt_nwavein && !nt_nwaveout) return (0);


    if (nt_meters)
    {
        int i, n;
        float maxsamp;
        for (i = 0, n = 2 * nt_nwavein * DACBLKSIZE, maxsamp = nt_inmax;
            i < n; i++)
        {
            float f = sys_soundin[i];
            if (f > maxsamp) maxsamp = f;
            else if (-f > maxsamp) maxsamp = -f;
        }
        nt_inmax = maxsamp;
        for (i = 0, n = 2 * nt_nwaveout * DACBLKSIZE, maxsamp = nt_outmax;
            i < n; i++)
        {
            float f = sys_soundout[i];
            if (f > maxsamp) maxsamp = f;
            else if (-f > maxsamp) maxsamp = -f;
        }
        nt_outmax = maxsamp;
    }

    	/* the "fill pointer" nt_fill controls where in the next
    	I/O buffers we will write and/or read.  If it's zero, we
    	first check whether the buffers are marked "done". */

    if (!nt_fill)
    {
	for (nad = 0; nad < nt_nwavein; nad++)
	{
    	    int phase = ntsnd_inphase[nad];
    	    WAVEHDR *inwavehdr = ntsnd_invec[nad][phase].lpWaveHdr;
    	    if (!(inwavehdr->dwFlags & WHDR_DONE)) goto idle;
	}
	for (nda = 0; nda < nt_nwaveout; nda++)
	{
    	    int phase = ntsnd_outphase[nda];
    	    WAVEHDR *outwavehdr =
    		ntsnd_outvec[nda][phase].lpWaveHdr;
    	    if (!(outwavehdr->dwFlags & WHDR_DONE)) goto idle;
	}
	for (nad = 0; nad < nt_nwavein; nad++)
	{
    	    int phase = ntsnd_inphase[nad];
    	    WAVEHDR *inwavehdr =
    		ntsnd_invec[nad][phase].lpWaveHdr;
    	    if (inwavehdr->dwFlags & WHDR_PREPARED)
    	    	waveInUnprepareHeader(ntsnd_indev[nad],
    	    	    inwavehdr, sizeof(WAVEHDR)); 
    	}
	for (nda = 0; nda < nt_nwaveout; nda++)
	{
    	    int phase = ntsnd_outphase[nda];
    	    WAVEHDR *outwavehdr = ntsnd_outvec[nda][phase].lpWaveHdr;
    	    if (outwavehdr->dwFlags & WHDR_PREPARED)
	    	waveOutUnprepareHeader(ntsnd_outdev[nda],
	    	    outwavehdr, sizeof(WAVEHDR)); 
	}
    }

    	/* Convert audio output to fixed-point and put it in the output
    	buffer. */ 
    for (nda = 0, fp1 = sys_soundout; nda < nt_nwaveout; nda++)
    {
    	int phase = ntsnd_outphase[nda];

	for (i = 0, sp1 = (short *)(ntsnd_outvec[nda][phase].lpData) +
	    CHANNELS_PER_DEVICE * nt_fill;
	    	i < 2; i++, fp1 += DACBLKSIZE, sp1++)
    	{
    	    for (j = 0, fp2 = fp1, sp2 = sp1; j < DACBLKSIZE;
    	    	j++, fp2++, sp2 += CHANNELS_PER_DEVICE)
    	    {
    		int x1 = 32767.f * *fp2;
    		if (x1 > 32767) x1 = 32767;
    		else if (x1 < -32767) x1 = -32767;
    		*sp2 = x1;  
	    }
    	}
    }
    memset(sys_soundout, 0, 
    	(DACBLKSIZE*sizeof(t_sample)*CHANNELS_PER_DEVICE)*nt_nwaveout);

    	/* vice versa for the input buffer */ 

    for (nad = 0, fp1 = sys_soundin; nad < nt_nwavein; nad++)
    {
    	int phase = ntsnd_inphase[nad];

	for (i = 0, sp1 = (short *)(ntsnd_invec[nad][phase].lpData) +
	    CHANNELS_PER_DEVICE * nt_fill;
	    	i < 2; i++, fp1 += DACBLKSIZE, sp1++)
    	{
    	    for (j = 0, fp2 = fp1, sp2 = sp1; j < DACBLKSIZE;
    	    	j++, fp2++, sp2 += CHANNELS_PER_DEVICE)
    	    {
    	    	*fp2 = ((float)(1./32767.)) * (float)(*sp2);    
    	    }
    	}
    }

    nt_fill = nt_fill + DACBLKSIZE;
    if (nt_fill == REALDACBLKSIZE)
    {
    	nt_fill = 0;

	for (nad = 0; nad < nt_nwavein; nad++)
	{
    	    int phase = ntsnd_inphase[nad];
    	    HWAVEIN device = ntsnd_indev[nad];
    	    WAVEHDR *inwavehdr = ntsnd_invec[nad][phase].lpWaveHdr;
    	    waveInPrepareHeader(device, inwavehdr, sizeof(WAVEHDR)); 
    	    mmresult = waveInAddBuffer(device, inwavehdr, sizeof(WAVEHDR)); 
    	    if (mmresult != MMSYSERR_NOERROR)
        	nt_waveinerror("waveInAddBuffer: %s\n", mmresult);
    	    ntsnd_inphase[nad] = WRAPFWD(phase + 1);
    	}
	for (nda = 0; nda < nt_nwaveout; nda++)
	{
    	    int phase = ntsnd_outphase[nda];
    	    HWAVEOUT device = ntsnd_outdev[nda];
    	    WAVEHDR *outwavehdr = ntsnd_outvec[nda][phase].lpWaveHdr;
	    waveOutPrepareHeader(device, outwavehdr, sizeof(WAVEHDR)); 
	    mmresult = waveOutWrite(device, outwavehdr, sizeof(WAVEHDR)); 
            if (mmresult != MMSYSERR_NOERROR)
        	nt_waveouterror("waveOutWrite: %s\n", mmresult);
	    ntsnd_outphase[nda] = WRAPFWD(phase + 1);
	}   	

    	    /* check for DAC underflow or ADC overflow. */
	for (nad = 0; nad < nt_nwavein; nad++)
	{
    	    int phase = WRAPBACK(ntsnd_inphase[nad] - 2);
    	    WAVEHDR *inwavehdr = ntsnd_invec[nad][phase].lpWaveHdr;
    	    if (inwavehdr->dwFlags & WHDR_DONE) goto late;
    	}
	for (nda = 0; nda < nt_nwaveout; nda++)
	{
    	    int phase = WRAPBACK(ntsnd_outphase[nda] - 2);
    	    WAVEHDR *outwavehdr = ntsnd_outvec[nda][phase].lpWaveHdr;
    	    if (outwavehdr->dwFlags & WHDR_DONE) goto late;
	}   	
    }
    return (1);

late:

    nt_logerror(LATE);
    nt_resyncaudio();
    return (1);

idle:

    /* If more than nt_adcjitterbufsallowed ADC buffers are ready
    on any input device, resynchronize */

    for (nad = 0; nad < nt_nwavein; nad++)
    {
    	int phase = ntsnd_inphase[nad];
    	WAVEHDR *inwavehdr =
    	    ntsnd_invec[nad]
    	    	[WRAPFWD(phase + nt_adcjitterbufsallowed)].lpWaveHdr;
    	if (inwavehdr->dwFlags & WHDR_DONE)
    	{
    	    nt_resyncaudio();
    	    return (0);
    	}
    }

    	/* test dac sync the same way */
    for (nda = 0; nda < nt_nwaveout; nda++)
    {
    	int phase = ntsnd_outphase[nda];
    	WAVEHDR *outwavehdr =
    	    ntsnd_outvec[nda]
    	    	[WRAPFWD(phase + nt_dacjitterbufsallowed)].lpWaveHdr;
    	if (outwavehdr->dwFlags & WHDR_DONE)
    	{
    	    nt_resyncaudio();
    	    return (0);
    	}
    }
#ifdef MIDI_TIMESTAMP
    nt_midisync();
#endif
    return (0);
}


static void nt_setchsr(int inchannels, int outchannels, int sr)
{
    int inbytes = inchannels * (DACBLKSIZE*sizeof(float));
    int outbytes = outchannels * (DACBLKSIZE*sizeof(float));

    if (nt_nwavein)
        free(sys_soundin);
    if (nt_nwaveout)
        free(sys_soundout);

    nt_nwavein = inchannels/CHANNELS_PER_DEVICE;
    nt_nwaveout = outchannels/CHANNELS_PER_DEVICE;
    sys_dacsr = sr;

    sys_soundin = (t_float *)malloc(inbytes);
    memset(sys_soundin, 0, inbytes);

    sys_soundout = (t_float *)malloc(outbytes);
    memset(sys_soundout, 0, outbytes);

    nt_advance_samples = (sys_schedadvance * sys_dacsr) / (1000000.);
    if (nt_advance_samples < 3 * DACBLKSIZE)
    	nt_advance_samples = 3 * DACBLKSIZE;
}

/* ------------------------- MIDI output -------------------------- */
static void nt_midiouterror(char *s, int err)
{
    char t[256];
    midiOutGetErrorText(err, t, 256);
    fprintf(stderr, s, t);
}

static HMIDIOUT hMidiOut[MAXMIDIOUTDEV];    /* output device */
static int nt_nmidiout;	    	    	    /* number of devices */

static void nt_open_midiout(int nmidiout, int *midioutvec)
{
    UINT result, wRtn;
    int i;
    int dev;
    MIDIOUTCAPS midioutcaps;
    if (nmidiout > MAXMIDIOUTDEV)
    	nmidiout = MAXMIDIOUTDEV;

    dev = 0;

    for (i = 0; i < nmidiout; i++)
    {
    	MIDIOUTCAPS mocap;
	result = midiOutOpen(&hMidiOut[dev], midioutvec[i]-1, 0, 0, 
	    CALLBACK_NULL);
    	wRtn = midiOutGetDevCaps(i, (LPMIDIOUTCAPS) &mocap,
            sizeof(mocap));
	if (result != MMSYSERR_NOERROR)
	{
	    fprintf(stderr,"midiOutOpen: %s\n",midioutcaps.szPname);
	    nt_midiouterror("midiOutOpen: %s\n", result);
	}
	else
	{
	    if (sys_verbose)
		fprintf(stderr,"midiOutOpen: Open %s as Port %d\n",
		    midioutcaps.szPname, dev);
	    dev++;
	}
    }
    nt_nmidiout = dev;
}

void sys_putmidimess(int portno, int a, int b, int c)
{
    DWORD foo;
    MMRESULT res;
    if (portno >= 0 && portno < nt_nmidiout)
    {
	foo = (a & 0xff) | ((b & 0xff) << 8) | ((c & 0xff) << 16);
	res = midiOutShortMsg(hMidiOut[portno], foo);
    	if (res != MMSYSERR_NOERROR)
	    post("MIDI out error %d", res);
    }
}

void sys_putmidibyte(int portno, int byte)
{
    MMRESULT res;
    if (portno >= 0 && portno < nt_nmidiout)
    {
    	res = midiOutShortMsg(hMidiOut[portno], byte);
    	if (res != MMSYSERR_NOERROR)
    	    post("MIDI out error %d", res);
    }
}

static void nt_close_midiout(void)
{
    int i;
    for (i = 0; i < nt_nmidiout; i++)
    {
    	midiOutReset(hMidiOut[i]);
    	midiOutClose(hMidiOut[i]);
    }
    nt_nmidiout = 0;
}

/* -------------------------- MIDI input ---------------------------- */

#define INPUT_BUFFER_SIZE 1000     // size of input buffer in events

static void nt_midiinerror(char *s, int err)
{
    char t[256];
    midiInGetErrorText(err, t, 256);
    fprintf(stderr, s, t);
}


/* Structure to represent a single MIDI event.
 */

#define EVNT_F_ERROR    0x00000001L

typedef struct event_tag
{
    DWORD fdwEvent;
    DWORD dwDevice;
    LARGE_INTEGER timestamp;
    DWORD data;
} EVENT;
typedef EVENT FAR *LPEVENT;

/* Structure to manage the circular input buffer.
 */
typedef struct circularBuffer_tag
{
    HANDLE  hSelf;          /* handle to this structure */
    HANDLE  hBuffer;        /* buffer handle */
    WORD    wError;         /* error flags */
    DWORD   dwSize;         /* buffer size (in EVENTS) */
    DWORD   dwCount;        /* byte count (in EVENTS) */
    LPEVENT lpStart;        /* ptr to start of buffer */
    LPEVENT lpEnd;          /* ptr to end of buffer (last byte + 1) */
    LPEVENT lpHead;         /* ptr to head (next location to fill) */
    LPEVENT lpTail;         /* ptr to tail (next location to empty) */
} CIRCULARBUFFER;
typedef CIRCULARBUFFER FAR *LPCIRCULARBUFFER;


/* Structure to pass instance data from the application
   to the low-level callback function.
 */
typedef struct callbackInstance_tag
{
    HANDLE              hSelf;  
    DWORD               dwDevice;
    LPCIRCULARBUFFER    lpBuf;
} CALLBACKINSTANCEDATA;
typedef CALLBACKINSTANCEDATA FAR *LPCALLBACKINSTANCEDATA;

/* Function prototypes
 */
LPCALLBACKINSTANCEDATA FAR PASCAL AllocCallbackInstanceData(void);
void FAR PASCAL FreeCallbackInstanceData(LPCALLBACKINSTANCEDATA lpBuf);

LPCIRCULARBUFFER AllocCircularBuffer(DWORD dwSize);
void FreeCircularBuffer(LPCIRCULARBUFFER lpBuf);
WORD FAR PASCAL GetEvent(LPCIRCULARBUFFER lpBuf, LPEVENT lpEvent);

// Callback instance data pointers
LPCALLBACKINSTANCEDATA lpCallbackInstanceData[MAXMIDIINDEV];

UINT wNumDevices = 0;			 // Number of MIDI input devices opened
BOOL bRecordingEnabled = 1;             // Enable/disable recording flag
int  nNumBufferLines = 0;		 // Number of lines in display buffer
RECT rectScrollClip;                    // Clipping rectangle for scrolling

LPCIRCULARBUFFER lpInputBuffer;         // Input buffer structure
EVENT incomingEvent;                    // Incoming MIDI event structure

MIDIINCAPS midiInCaps[MAXMIDIINDEV]; // Device capabilities structures
HMIDIIN hMidiIn[MAXMIDIINDEV];       // MIDI input device handles


/* AllocCallbackInstanceData - Allocates a CALLBACKINSTANCEDATA
 *      structure.  This structure is used to pass information to the
 *      low-level callback function, each time it receives a message.
 *
 *      Because this structure is accessed by the low-level callback
 *      function, it must be allocated using GlobalAlloc() with the 
 *      GMEM_SHARE and GMEM_MOVEABLE flags and page-locked with
 *      GlobalPageLock().
 *
 * Params:  void
 *
 * Return:  A pointer to the allocated CALLBACKINSTANCE data structure.
 */
LPCALLBACKINSTANCEDATA FAR PASCAL AllocCallbackInstanceData(void)
{
    HANDLE hMem;
    LPCALLBACKINSTANCEDATA lpBuf;
    
    /* Allocate and lock global memory.
     */
    hMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE,
                       (DWORD)sizeof(CALLBACKINSTANCEDATA));
    if(hMem == NULL)
        return NULL;
    
    lpBuf = (LPCALLBACKINSTANCEDATA)GlobalLock(hMem);
    if(lpBuf == NULL){
        GlobalFree(hMem);
        return NULL;
    }
    
    /* Page lock the memory.
     */
    //GlobalPageLock((HGLOBAL)HIWORD(lpBuf));

    /* Save the handle.
     */
    lpBuf->hSelf = hMem;

    return lpBuf;
}

/* FreeCallbackInstanceData - Frees the given CALLBACKINSTANCEDATA structure.
 *
 * Params:  lpBuf - Points to the CALLBACKINSTANCEDATA structure to be freed.
 *
 * Return:  void
 */
void FAR PASCAL FreeCallbackInstanceData(LPCALLBACKINSTANCEDATA lpBuf)
{
    HANDLE hMem;

    /* Save the handle until we're through here.
     */
    hMem = lpBuf->hSelf;

    /* Free the structure.
     */
    //GlobalPageUnlock((HGLOBAL)HIWORD(lpBuf));
    GlobalUnlock(hMem);
    GlobalFree(hMem);
}


/*
 * AllocCircularBuffer -    Allocates memory for a CIRCULARBUFFER structure 
 * and a buffer of the specified size.  Each memory block is allocated 
 * with GlobalAlloc() using GMEM_SHARE and GMEM_MOVEABLE flags, locked 
 * with GlobalLock(), and page-locked with GlobalPageLock().
 *
 * Params:  dwSize - The size of the buffer, in events.
 *
 * Return:  A pointer to a CIRCULARBUFFER structure identifying the 
 *      allocated display buffer.  NULL if the buffer could not be allocated.
 */
 
 
LPCIRCULARBUFFER AllocCircularBuffer(DWORD dwSize)
{
    HANDLE hMem;
    LPCIRCULARBUFFER lpBuf;
    LPEVENT lpMem;
    
    /* Allocate and lock a CIRCULARBUFFER structure.
     */
    hMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE,
                       (DWORD)sizeof(CIRCULARBUFFER));
    if(hMem == NULL)
        return NULL;

    lpBuf = (LPCIRCULARBUFFER)GlobalLock(hMem);
    if(lpBuf == NULL)
    {
        GlobalFree(hMem);
        return NULL;
    }
    
    /* Page lock the memory.  Global memory blocks accessed by
     * low-level callback functions must be page locked.
     */
#ifndef _WIN32
    GlobalSmartPageLock((HGLOBAL)HIWORD(lpBuf));
#endif

    /* Save the memory handle.
     */
    lpBuf->hSelf = hMem;
    
    /* Allocate and lock memory for the actual buffer.
     */
    hMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, dwSize * sizeof(EVENT));
    if(hMem == NULL)
    {
#ifndef _WIN32
        GlobalSmartPageUnlock((HGLOBAL)HIWORD(lpBuf));
#endif
        GlobalUnlock(lpBuf->hSelf);
        GlobalFree(lpBuf->hSelf);
        return NULL;
    }
    
    lpMem = (LPEVENT)GlobalLock(hMem);
    if(lpMem == NULL)
    {
        GlobalFree(hMem);
#ifndef _WIN32
        GlobalSmartPageUnlock((HGLOBAL)HIWORD(lpBuf));
#endif
        GlobalUnlock(lpBuf->hSelf);
        GlobalFree(lpBuf->hSelf);
        return NULL;
    }
    
    /* Page lock the memory.  Global memory blocks accessed by
     * low-level callback functions must be page locked.
     */
#ifndef _WIN32
    GlobalSmartPageLock((HGLOBAL)HIWORD(lpMem));
#endif
    
    /* Set up the CIRCULARBUFFER structure.
     */
    lpBuf->hBuffer = hMem;
    lpBuf->wError = 0;
    lpBuf->dwSize = dwSize;
    lpBuf->dwCount = 0L;
    lpBuf->lpStart = lpMem;
    lpBuf->lpEnd = lpMem + dwSize;
    lpBuf->lpTail = lpMem;
    lpBuf->lpHead = lpMem;
        
    return lpBuf;
}

/* FreeCircularBuffer - Frees the memory for the given CIRCULARBUFFER 
 * structure and the memory for the buffer it references.
 *
 * Params:  lpBuf - Points to the CIRCULARBUFFER to be freed.
 *
 * Return:  void
 */
void FreeCircularBuffer(LPCIRCULARBUFFER lpBuf)
{
    HANDLE hMem;
    
    /* Free the buffer itself.
     */
#ifndef _WIN32
    GlobalSmartPageUnlock((HGLOBAL)HIWORD(lpBuf->lpStart));
#endif
    GlobalUnlock(lpBuf->hBuffer);
    GlobalFree(lpBuf->hBuffer);
    
    /* Free the CIRCULARBUFFER structure.
     */
    hMem = lpBuf->hSelf;
#ifndef _WIN32
    GlobalSmartPageUnlock((HGLOBAL)HIWORD(lpBuf));
#endif
    GlobalUnlock(hMem);
    GlobalFree(hMem);
}

/* GetEvent - Gets a MIDI event from the circular input buffer.  Events
 *  are removed from the buffer.  The corresponding PutEvent() function
 *  is called by the low-level callback function, so it must reside in
 *  the callback DLL.  PutEvent() is defined in the CALLBACK.C module.
 *
 * Params:  lpBuf - Points to the circular buffer.
 *          lpEvent - Points to an EVENT structure that is filled with the
 *              retrieved event.
 *
 * Return:  Returns non-zero if successful, zero if there are no 
 *   events to get.
 */
WORD FAR PASCAL GetEvent(LPCIRCULARBUFFER lpBuf, LPEVENT lpEvent)
{
    	/* If no event available, return */
    if (!wNumDevices || lpBuf->dwCount <= 0) return (0);
    
    /* Get the event.
     */
    *lpEvent = *lpBuf->lpTail;
    
    /* Decrement the byte count, bump the tail pointer.
     */
    --lpBuf->dwCount;
    ++lpBuf->lpTail;
    
    /* Wrap the tail pointer, if necessary.
     */
    if(lpBuf->lpTail >= lpBuf->lpEnd)
        lpBuf->lpTail = lpBuf->lpStart;

    return 1;
}

/* PutEvent - Puts an EVENT in a CIRCULARBUFFER.  If the buffer is full, 
 *      it sets the wError element of the CIRCULARBUFFER structure 
 *      to be non-zero.
 *
 * Params:  lpBuf - Points to the CIRCULARBUFFER.
 *          lpEvent - Points to the EVENT.
 *
 * Return:  void
*/

void FAR PASCAL PutEvent(LPCIRCULARBUFFER lpBuf, LPEVENT lpEvent)
{
    /* If the buffer is full, set an error and return. 
     */
    if(lpBuf->dwCount >= lpBuf->dwSize){
        lpBuf->wError = 1;
        return;
    }
    
    /* Put the event in the buffer, bump the head pointer and the byte count.
     */
    *lpBuf->lpHead = *lpEvent;
    
    ++lpBuf->lpHead;
    ++lpBuf->dwCount;

    /* Wrap the head pointer, if necessary.
     */
    if(lpBuf->lpHead >= lpBuf->lpEnd)
        lpBuf->lpHead = lpBuf->lpStart;
}

/* midiInputHandler - Low-level callback function to handle MIDI input.
 *      Installed by midiInOpen().  The input handler takes incoming
 *      MIDI events and places them in the circular input buffer.  It then
 *      notifies the application by posting a MM_MIDIINPUT message.
 *
 *      This function is accessed at interrupt time, so it should be as 
 *      fast and efficient as possible.  You can't make any
 *      Windows calls here, except PostMessage().  The only Multimedia
 *      Windows call you can make are timeGetSystemTime(), midiOutShortMsg().
 *      
 *
 * Param:   hMidiIn - Handle for the associated input device.
 *          wMsg - One of the MIM_***** messages.
 *          dwInstance - Points to CALLBACKINSTANCEDATA structure.
 *          dwParam1 - MIDI data.
 *          dwParam2 - Timestamp (in milliseconds)
 *
 * Return:  void
 */     
void FAR PASCAL midiInputHandler(
HMIDIIN hMidiIn, 
WORD wMsg, 
DWORD dwInstance, 
DWORD dwParam1, 
DWORD dwParam2)
{
    EVENT event;
    
    switch(wMsg)
    {
        case MIM_OPEN:
            break;

        /* The only error possible is invalid MIDI data, so just pass
         * the invalid data on so we'll see it.
         */
        case MIM_ERROR:
        case MIM_DATA:
            event.fdwEvent = (wMsg == MIM_ERROR) ? EVNT_F_ERROR : 0;
            event.dwDevice = ((LPCALLBACKINSTANCEDATA)dwInstance)->dwDevice;
            event.data = dwParam1;
#ifdef MIDI_TIMESTAMP
            event.timestamp = timeGetSystemTime();
#endif
            /* Put the MIDI event in the circular input buffer.
             */

            PutEvent(((LPCALLBACKINSTANCEDATA)dwInstance)->lpBuf,
                       (LPEVENT) &event); 

            break;

        default:
            break;
    }
}

void nt_open_midiin(int nmidiin, int *midiinvec)
{
    UINT  wRtn;
    char szErrorText[256];
    unsigned int i;
    unsigned int ndev = 0;
    /* Allocate a circular buffer for low-level MIDI input.  This buffer
     * is filled by the low-level callback function and emptied by the
     * application.
     */
    lpInputBuffer = AllocCircularBuffer((DWORD)(INPUT_BUFFER_SIZE));
    if (lpInputBuffer == NULL)
    {
        printf("Not enough memory available for input buffer.\n");
        return;
    }

    /* Open all MIDI input devices after allocating and setting up
     * instance data for each device.  The instance data is used to
     * pass buffer management information between the application and
     * the low-level callback function.  It also includes a device ID,
     * a handle to the MIDI Mapper, and a handle to the application's
     * display window, so the callback can notify the window when input
     * data is available.  A single callback function is used to service
     * all opened input devices.
     */
    for (i=0; (i<(unsigned)nmidiin) && (i<MAXMIDIINDEV); i++)
    {
        if ((lpCallbackInstanceData[ndev] = AllocCallbackInstanceData()) == NULL)
        {
            printf("Not enough memory available.\n");
            FreeCircularBuffer(lpInputBuffer);
            return;
        }
        lpCallbackInstanceData[i]->dwDevice = i;
        lpCallbackInstanceData[i]->lpBuf = lpInputBuffer;
        
        wRtn = midiInOpen((LPHMIDIIN)&hMidiIn[ndev],
              midiinvec[i] - 1,
              (DWORD)midiInputHandler,
              (DWORD)lpCallbackInstanceData[ndev],
              CALLBACK_FUNCTION);
        if (wRtn)
        {
            FreeCallbackInstanceData(lpCallbackInstanceData[ndev]);
            nt_midiinerror("midiInOpen: %s\n", wRtn);
        }
	else ndev++;
    }

    /* Start MIDI input.
     */
    for (i=0; i<ndev; i++)
    {
        if (hMidiIn[i])
            midiInStart(hMidiIn[i]);
    }
    wNumDevices = ndev;
}

static void nt_close_midiin(void)
{
    unsigned int i;
    /* Stop, reset, close MIDI input.  Free callback instance data.
     */

    for (i=0; (i<wNumDevices) && (i<MAXMIDIINDEV); i++)
    {
        if (hMidiIn[i])
	{
    	    if (sys_verbose)
    	    	post("closing MIDI input %d...", i);
            midiInStop(hMidiIn[i]);
            midiInReset(hMidiIn[i]);
            midiInClose(hMidiIn[i]);
            FreeCallbackInstanceData(lpCallbackInstanceData[i]);
        }
    }
    
    /* Free input buffer.
     */
    if (lpInputBuffer)
    	FreeCircularBuffer(lpInputBuffer);

    if (sys_verbose)
    	post("...done");
    wNumDevices = 0;
}

void inmidi_noteon(int portno, int channel, int pitch, int velo);
void inmidi_controlchange(int portno, int channel, int ctlnumber, int value);
void inmidi_programchange(int portno, int channel, int value);
void inmidi_pitchbend(int portno, int channel, int value);
void inmidi_aftertouch(int portno, int channel, int value);
void inmidi_polyaftertouch(int portno, int channel, int pitch, int value);
void inmidi_realtimein(int portno, int rtmsg);

void sys_poll_midi(void)
{
    static EVENT nt_nextevent;
    static int nt_isnextevent;
    static double nt_nexteventtime;

    while (1)
    {
    	if (!nt_isnextevent)
    	{
    	    if (!GetEvent(lpInputBuffer, &nt_nextevent)) break;
    	    nt_isnextevent = 1;
#ifdef MIDI_TIMESTAMP
    	    nt_nexteventtime = nt_midigettimefor(&foo.timestamp);
#endif
    	}
#ifdef MIDI_TIMESTAMP
    	if (0.001 * clock_gettimesince(initsystime) >= nt_nexteventtime)
#endif
    	{
    	    int msgtype = ((nt_nextevent.data & 0xf0) >> 4) - 8;
    	    int commandbyte = nt_nextevent.data & 0xff;
    	    int byte1 = (nt_nextevent.data >> 8) & 0xff;
    	    int byte2 = (nt_nextevent.data >> 16) & 0xff;
    	    int portno = nt_nextevent.dwDevice;
	    switch (msgtype)
    	    {
    	    case 0: 
    	    case 1: 
    	    case 2:
    	    case 3:
    	    case 6:
		sys_midibytein(portno, commandbyte);
		sys_midibytein(portno, byte1);
		sys_midibytein(portno, byte2);
		break; 
    	    case 4:
    	    case 5:
		sys_midibytein(portno, commandbyte);
		sys_midibytein(portno, byte1);
		break;
	    case 7:
		sys_midibytein(portno, commandbyte);
		break; 
	    }
    	    nt_isnextevent = 0;
    	}
    }
}

/* ------------------- public routines -------------------------- */

void sys_open_audio(int naudioindev, int *audioindev,
    int nchindev, int *chindev, int naudiooutdev, int *audiooutdev,
    int nchoutdev, int *choutdev, int rate) /* IOhannes */
{
    int inchans, outchans;
    if (nchindev < 0)
        inchans = (nchindev < 1 ? -1 : chindev[0]);
    else
    {
	int i = nchindev;
	int *l = chindev;
	inchans = 0;
	while (i--)
	    inchans += *l++;
    }
    if (nchoutdev<0)
        outchans = (nchoutdev < 1 ? -1 : choutdev[0]);
    else
    {
	int i = nchoutdev;
	int *l = choutdev;
	outchans = 0;
	while (i--)
      	  outchans += *l++;
    }
    if (inchans < 0)
    	inchans = DEFAULTCHANS;
    if (outchans < 0)
    	outchans = DEFAULTCHANS;
    if (inchans & 1)
    {
    	post("input channels rounded up to even number");
	inchans += 1;
    }
    if (outchans & 1)
    {
    	post("output channels rounded up to even number");
	outchans += 1;
    }
    if (inchans > NT_MAXCH)
    	inchans = NT_MAXCH;
    if (outchans > NT_MAXCH)
    	outchans = NT_MAXCH;
    if (sys_verbose)
    	post("channels in %d, out %d", inchans, outchans);
    if (rate < 1)
    	rate = DEFAULTSRATE;
    nt_setchsr(inchans, outchans, rate);
    if (nt_whichapi == API_PORTAUDIO)
    {
    	int blocksize = (nt_blocksize ? nt_blocksize : 256);
	if (blocksize != (1 << ilog2(blocksize)))
	    post("warning: blocksize adjusted to power of 2: %d",
	    	(blocksize = (1 << ilog2(blocksize))));
    	pa_open_audio(inchans, outchans, rate, sys_soundin, sys_soundout,
    	    blocksize, nt_advance_samples/blocksize,
	    	(naudioindev < 1 ? -1 : audioindev[0]),
	    	(naudiooutdev < 1 ? -1 : audiooutdev[0]));
    }
    else
    {
	nt_nwavein = inchans / 2;
	nt_nwaveout = outchans / 2;
	nt_whichdac = (naudiooutdev < 1 ? (nt_nwaveout > 1 ? 0 : -1) : audiooutdev[0] - 1);
	nt_whichadc = (naudioindev < 1 ? (nt_nwavein > 1 ? 0 : -1) : audioindev[0] - 1);
	if (naudiooutdev > 1 || naudioindev > 1)
	    post("separate audio device choice not supported; using sequential devices.");
	if (nt_blocksize)
	    post("warning: blocksize not settable for MMIO, just ASIO");
	mmio_open_audio();
    }
}

void sys_open_midi(int nmidiin, int *midiinvec, int nmidiout, int *midioutvec)
{
    if (nmidiout)
    	nt_open_midiout(nmidiout, midioutvec);
    if (nmidiin)
    {
    	post(
	 "midi input enabled; warning, don't close the DOS window directly!");
    	nt_open_midiin(nmidiin, midiinvec);
    }
    else post("not using MIDI input (use 'pd -midiindev 1' to override)");
}

float sys_getsr(void)
{
    return (sys_dacsr);
}

int sys_get_inchannels(void)
{
    return (2 * nt_nwavein);
}

int sys_get_outchannels(void)
{
    return (2 * nt_nwaveout);
}

void sys_audiobuf(int n)
{
    	/* set the size, in msec, of the audio FIFO.  It's incorrect to
	calculate this on the basis of 44100 sample rate; really, the
	work should have been done in nt_setchsr(). */
    int nbuf = n * (44100./(REALDACBLKSIZE * 1000.));
    if (nbuf >= MAXBUFFER)
    {
    	fprintf(stderr, "pd: audio buffering maxed out to %d\n",
    	    (int)(MAXBUFFER * ((REALDACBLKSIZE * 1000.)/44100.)));
    	nbuf = MAXBUFFER;
    }
    else if (nbuf < 4) nbuf = 4;
    fprintf(stderr, "%d audio buffers\n", nbuf);
    nt_naudiobuffer = nbuf;
    if (nt_adcjitterbufsallowed > nbuf - 2)
    	nt_adcjitterbufsallowed = nbuf - 2;
    if (nt_dacjitterbufsallowed > nbuf - 2)
    	nt_dacjitterbufsallowed = nbuf - 2;
    sys_schedadvance = 1000 * n;
}

void sys_getmeters(float *inmax, float *outmax)
{
    if (inmax)
    {
        nt_meters = 1;
        *inmax = nt_inmax;
        *outmax = nt_outmax;
    }
    else
        nt_meters = 0;
    nt_inmax = nt_outmax = 0;
}

void sys_reportidle(void)
{
}

int sys_send_dacs(void)
{
    if (nt_whichapi == API_PORTAUDIO)
    	return (pa_send_dacs());
    else return (mmio_send_dacs());
}

void sys_close_audio( void)
{
    if (nt_whichapi == API_PORTAUDIO)
    	pa_close_audio();
    else mmio_close_audio();
}

void sys_close_midi( void)
{
    nt_close_midiin();
    nt_close_midiout();
}

void sys_setblocksize(int n)
{
    if (n < 1)
    	n = 1;
    nt_blocksize = n;
}

/* ----------- public routines which are only defined for MSW/NT ---------- */

/* select between MMIO and ASIO audio APIs */
void nt_set_sound_api(int which)
{
     nt_whichapi = which;
     if (sys_verbose)
     	post("nt_whichapi %d", nt_whichapi);
}

/* list the audio and MIDI device names */
void sys_listdevs(void)
{
    UINT  wRtn, ndevices;
    unsigned int i;

    /* for MIDI and audio in and out, get the number of devices.
    	Then get the capabilities of each device and print its description. */

    ndevices = midiInGetNumDevs();
    for (i = 0; i < ndevices; i++)
    {
    	MIDIINCAPS micap;
    	wRtn = midiInGetDevCaps(i, (LPMIDIINCAPS) &micap,
            sizeof(micap));
        if (wRtn) nt_midiinerror("midiInGetDevCaps: %s\n", wRtn);
    	else fprintf(stderr,
    	    "MIDI input device #%d: %s\n", i+1, micap.szPname);
    }

    ndevices = midiOutGetNumDevs();
    for (i = 0; i < ndevices; i++)
    {
    	MIDIOUTCAPS mocap;
    	wRtn = midiOutGetDevCaps(i, (LPMIDIOUTCAPS) &mocap,
            sizeof(mocap));
        if (wRtn) nt_midiouterror("midiOutGetDevCaps: %s\n", wRtn);
    	else fprintf(stderr,
    	    "MIDI output device #%d: %s\n", i+1, mocap.szPname);
    }

    if (nt_whichapi == API_PORTAUDIO)
    {
        pa_listdevs();
    	return;
    }
    ndevices = waveInGetNumDevs();
    for (i = 0; i < ndevices; i++)
    {
    	WAVEINCAPS wicap;
    	wRtn = waveInGetDevCaps(i, (LPWAVEINCAPS) &wicap,
            sizeof(wicap));
        if (wRtn) nt_waveinerror("waveInGetDevCaps: %s\n", wRtn);
    	else fprintf(stderr,
    	    "audio input device #%d: %s\n", i+1, wicap.szPname);
    }

    ndevices = waveOutGetNumDevs();
    for (i = 0; i < ndevices; i++)
    {
    	WAVEOUTCAPS wocap;
    	wRtn = waveOutGetDevCaps(i, (LPWAVEOUTCAPS) &wocap,
            sizeof(wocap));
        if (wRtn) nt_waveouterror("waveOutGetDevCaps: %s\n", wRtn);
    	else fprintf(stderr,
    	    "audio output device #%d: %s\n", i+1, wocap.szPname);
    }
}

void nt_soundindev(int which)
{
    nt_whichadc = which - 1;
}

void nt_soundoutdev(int which)
{
    nt_whichdac = which - 1;
}

void glob_audio(void *dummy, t_floatarg fadc, t_floatarg fdac)
{
    int adc = fadc, dac = fdac;
    if (!dac && !adc)
    	post("%d channels in, %d channels out",
    	    2 * nt_nwavein, 2 * nt_nwaveout);
    else
    {
    	sys_close_audio();
    	sys_open_audio(1, 0, 1, 0, /* dummy parameters */
		       1, &adc, 1, &dac, sys_dacsr);
    }
}

