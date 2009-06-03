/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* modified 2/98 by Winfried Ritsch to deal with up to 4 synchronized
"wave" devices, which is how ADAT boards appear to the WAVE API. */

#include "desire.h"
#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>

/* ------------------------- audio -------------------------- */

static void nt_noresync();

#define NAPORTS 16   /* wini hack for multiple ADDA devices  */
#define CHANNELS_PER_DEVICE 2
#define DEFAULTCHANS 2
#define DEFAULTSRATE 44100
#define SAMPSIZE 2

int nt_realdacblksize;
#define DEFREALDACBLKSIZE (4 * sys_dacblocksize) /* larger underlying bufsize */

#define MAXBUFFER 100   /* number of buffers in use at maximum advance */
#define DEFBUFFER 30    /* default is about 30x6 = 180 msec! */
static int nt_naudiobuffer = DEFBUFFER;
static int nt_meters;              /* true if we're metering */
static float mmi_max,   mmo_max;   /* max amplitude */
static int   mmi_nwave, mmo_nwave; /* number of WAVE devices */

typedef struct _sbuf {
    HANDLE hData;
    HPSTR  lpData;      // pointer to waveform data memory
    HANDLE hWaveHdr;
    WAVEHDR *lpWaveHdr;   // pointer to header structure
} t_sbuf;

t_sbuf mmo_vec[NAPORTS][MAXBUFFER]; /* circular buffer array */
t_sbuf mmi_vec[NAPORTS][MAXBUFFER]; /* circular buffer array */
HWAVEOUT mmo_dev[NAPORTS];          /* output device */
HWAVEIN  mmi_dev[NAPORTS];          /* input device */
static int mmo_phase[NAPORTS];      /* index of next buffer to send */
static int mmi_phase[NAPORTS];      /* index of next buffer to read */
static UINT nt_whichdac = WAVE_MAPPER, nt_whichadc = WAVE_MAPPER;

static void mmi_waveerror(const char *s, int err) {char t[256];  waveInGetErrorText(err, t, 256); error(s,t);}
static void mmo_waveerror(const char *s, int err) {char t[256]; waveOutGetErrorText(err, t, 256); error(s,t);}

static void wave_prep(t_sbuf *bp, int setdone) {
    WAVEHDR *wh;
    /* Allocate and lock memory for the waveform data. The memory for waveform data must be globally allocated with
     * GMEM_MOVEABLE and GMEM_SHARE flags. */
    if (!(bp->hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD) (CHANNELS_PER_DEVICE * SAMPSIZE * nt_realdacblksize))))
        printf("alloc 1 failed\n");
    if (!(bp->lpData = (HPSTR) GlobalLock(bp->hData)))
        printf("lock 1 failed\n");
    /*  Allocate and lock memory for the header. */
    if (!(bp->hWaveHdr = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE, (DWORD) sizeof(WAVEHDR))))
        printf("alloc 2 failed\n");
    if (!(wh = bp->lpWaveHdr = (WAVEHDR *) GlobalLock(bp->hWaveHdr)))
        printf("lock 2 failed\n");
    short *sp = (short *)bp->lpData;
    for (int i=CHANNELS_PER_DEVICE*nt_realdacblksize; i--; ) *sp++ = 0;
    wh->lpData = bp->lpData;
    wh->dwBufferLength = CHANNELS_PER_DEVICE * SAMPSIZE * nt_realdacblksize;
    wh->dwFlags = 0;
    wh->dwLoops = 0L;
    wh->lpNext = 0;
    wh->reserved = 0;
    /* optionally (for writing) set DONE flag as if we had queued them */
    if (setdone) wh->dwFlags = WHDR_DONE;
}

int mmio_do_open_audio() {
    PCMWAVEFORMAT  form;
    UINT r;
    static int naudioprepped = 0, nindevsprepped = 0, noutdevsprepped = 0;
    if (sys_verbose) post("%d devices in, %d devices out", mmi_nwave, mmo_nwave);
    form.wf.wFormatTag = WAVE_FORMAT_PCM;
    form.wf.nChannels = CHANNELS_PER_DEVICE;
    form.wf.nSamplesPerSec = sys_dacsr;
    form.wf.nAvgBytesPerSec = sys_dacsr * (CHANNELS_PER_DEVICE * SAMPSIZE);
    form.wf.nBlockAlign = CHANNELS_PER_DEVICE * SAMPSIZE;
    form.wBitsPerSample = 8 * SAMPSIZE;
    if (mmi_nwave <= 1 && mmo_nwave <= 1) nt_noresync();
    if (nindevsprepped < mmi_nwave) {
        for (int i=nindevsprepped;  i<mmi_nwave;  i++) for (int j=0; j<naudioprepped; j++) wave_prep(&mmi_vec[i][j], 0);
        nindevsprepped = mmi_nwave;
    }
    if (noutdevsprepped < mmo_nwave) {
        for (int i=noutdevsprepped; i<mmo_nwave; i++) for (int j=0; j<naudioprepped; j++) wave_prep(&mmo_vec[i][j], 1);
        noutdevsprepped = mmo_nwave;
    }
    if (naudioprepped < nt_naudiobuffer) {
        for (int j=naudioprepped; j<nt_naudiobuffer; j++) {
            for (int i=0; i<mmi_nwave; i++) wave_prep(&mmi_vec[i][j], 0);
            for (int i=0; i<mmo_nwave; i++) wave_prep(&mmo_vec[i][j], 1);
        }
        naudioprepped = nt_naudiobuffer;
    }
    for (int n=0; n<mmi_nwave; n++) {
        /* Open waveform device(s), sucessively numbered, for input */
        r = waveInOpen(&mmi_dev[n], nt_whichadc+n, (WAVEFORMATEX *)&form, 0L, 0L, CALLBACK_NULL);
        if (sys_verbose) printf("opened adc device %d with return %d\n", nt_whichadc+n,r);
        if (r != MMSYSERR_NOERROR) {
            mmi_waveerror("waveInOpen: %s", r);
            mmi_nwave = n; /* mmi_nwave = 0 wini */
        } else {
            for (int i=0; i<nt_naudiobuffer; i++) {
                r = waveInPrepareHeader(mmi_dev[n], mmi_vec[n][i].lpWaveHdr, sizeof(WAVEHDR));
                if (r != MMSYSERR_NOERROR) mmi_waveerror("waveinprepareheader: %s", r);
                r = waveInAddBuffer(    mmi_dev[n], mmi_vec[n][i].lpWaveHdr, sizeof(WAVEHDR));
                if (r != MMSYSERR_NOERROR) mmi_waveerror("waveInAddBuffer: %s", r);
            }
        }
    }
    /* quickly start them all together */
    for (int n=0; n<mmi_nwave; n++) waveInStart(mmi_dev[n]);
    for (int n=0; n<mmo_nwave; n++) {
        /* Open a waveform device for output in sucessiv device numbering*/
        r = waveOutOpen(&mmo_dev[n], nt_whichdac + n, (WAVEFORMATEX *)&form, 0L, 0L, CALLBACK_NULL);
        if (sys_verbose) post("opened dac device %d, with return %d", nt_whichdac +n, r);
        if (r != MMSYSERR_NOERROR) {
            post("Wave out open device %d + %d",nt_whichdac,n);
            mmo_waveerror("waveOutOpen device: %s", r);
            mmo_nwave = n;
        }
    }
    return 0;
}

static void mmio_close_audio() {
    int errcode;
    if (sys_verbose) post("closing audio...");
    for (int n=0; n<mmo_nwave; n++) /*if (mmo_nwave) wini */ {
       errcode = waveOutReset(mmo_dev[n]); if (errcode!=MMSYSERR_NOERROR) printf("error resetting output %d: %d",n,errcode);
       errcode = waveOutClose(mmo_dev[n]); if (errcode!=MMSYSERR_NOERROR) printf("error closing output %d: %d",  n,errcode);
    }
    mmo_nwave = 0;
    for (int n=0; n<mmi_nwave;n++) /* if (mmi_nwave) wini */ {
        errcode = waveInReset(mmi_dev[n]); if (errcode!=MMSYSERR_NOERROR) printf("error resetting input: %d", errcode);
        errcode = waveInClose(mmi_dev[n]); if (errcode!=MMSYSERR_NOERROR) printf("error closing input: %d",   errcode);
    }
    mmi_nwave = 0;
}

#define ADCJITTER 10    /* We tolerate X buffers of jitter by default */
#define DACJITTER 10

static int nt_adcjitterbufsallowed = ADCJITTER;
static int nt_dacjitterbufsallowed = DACJITTER;

/* ------------- MIDI time stamping from audio clock ------------ */

#ifdef MIDI_TIMESTAMP

static double nt_hibuftime;
static double initsystime = -1;

/* call this whenever we reset audio */
static void nt_resetmidisync() {initsystime = clock_getsystime(); nt_hibuftime = sys_getrealtime();}

/* call this whenever we're idled waiting for audio to be ready.
   The routine maintains a high and low water point for the difference between real and DAC time. */

static void nt_midisync() {
    if (initsystime == -1) nt_resetmidisync();
    double jittersec = max(nt_dacjitterbufsallowed,nt_adcjitterbufsallowed) * nt_realdacblksize / sys_getsr();
    double diff = sys_getrealtime() - 0.001 * clock_gettimesince(initsystime);
    if (diff > nt_hibuftime) nt_hibuftime = diff;
    if (diff < nt_hibuftime - jittersec) {post("jitter excess %d %f", dac, diff); nt_resetmidisync();}
}

static double nt_midigettimefor(LARGE_INTEGER timestamp) {
    /* this is broken now... used to work when "timestamp" was derived from
        QueryPerformanceCounter() instead of the gates approved timeGetSystemTime() call in the MIDI callback routine below. */
    return nt_tixtotime(timestamp) - nt_hibuftime;
}
#endif /* MIDI_TIMESTAMP */

static int nt_fill = 0;
#define WRAPFWD(x) ((x) >= nt_naudiobuffer ? (x) - nt_naudiobuffer: (x))
#define WRAPBACK(x) ((x) < 0 ? (x) + nt_naudiobuffer: (x))
#define MAXRESYNC 500

#if 0 /* this is used for debugging */
static void nt_printaudiostatus() {
    for (int n=0; n<mmi_nwave; n++) {
        int phase = mmi_phase[n];
        int phase2 = phase, phase3 = WRAPFWD(phase2), ntrans = 0;
        int firstphasedone = -1, firstphasebusy = -1;
        for (int count=0; count<nt_naudiobuffer; count++) {
            int donethis = (mmi_vec[n][phase2].lpWaveHdr->dwFlags & WHDR_DONE);
            int donenext = (mmi_vec[n][phase3].lpWaveHdr->dwFlags & WHDR_DONE);
            if (donethis && !donenext) {if (firstphasebusy >= 0) goto multipleadc; else firstphasebusy = count;}
            if (!donethis && donenext) {if (firstphasedone >= 0) goto multipleadc; else firstphasedone = count;}
            phase2 = phase3;
            phase3 = WRAPFWD(phase2 + 1);
        }
        post("nad %d phase %d busy %d done %d", n, phase, firstphasebusy, firstphasedone);
        continue;
    multipleadc:
        startpost("nad %d phase %d: oops:", n, phase);
        for (int count=0; count<nt_naudiobuffer; count++) {
            char buf[80];
            sprintf(buf, " %d", (mmi_vec[n][count].lpWaveHdr->dwFlags & WHDR_DONE));
            poststring(buf);
        }
        endpost();
    }
    for (int n=0; n<mmo_nwave; n++) {
        int phase = mmo_phase[n];
        int phase2 = phase, phase3 = WRAPFWD(phase2), ntrans = 0;
        int firstphasedone = -1, firstphasebusy = -1;
        for (count = 0; count < nt_naudiobuffer; count++) {
            int donethis = (mmo_vec[n][phase2].lpWaveHdr->dwFlags & WHDR_DONE);
            int donenext = (mmo_vec[n][phase3].lpWaveHdr->dwFlags & WHDR_DONE);
            if (donethis && !donenext) {if (firstphasebusy >= 0) goto multipledac; else firstphasebusy = count;}
            if (!donethis && donenext) {if (firstphasedone >= 0) goto multipledac; else firstphasedone = count;}
            phase2 = phase3;
            phase3 = WRAPFWD(phase2 + 1);
        }
        if (firstphasebusy < 0) post("nda %d phase %d all %d", n, phase, (mmo_vec[n][0].lpWaveHdr->dwFlags & WHDR_DONE));
        else post("nda %d phase %d busy %d done %d", n, phase, firstphasebusy, firstphasedone);
        continue;
    multipledac:
        startpost("nda %d phase %d: oops:", n, phase);
        for (count = 0; count < nt_naudiobuffer; count++) {
            char buf[80];
            sprintf(buf, " %d", (mmo_vec[n][count].lpWaveHdr->dwFlags & WHDR_DONE));
            poststring(buf);
        }
        endpost();
    }
}
#endif /* 0 */

/* this is a hack to avoid ever resyncing audio pointers in case for whatever reason the sync testing below gives false positives. */

static int nt_resync_cancelled;
static void nt_noresync() {nt_resync_cancelled = 1;}

static void nt_resyncaudio() {
    UINT r;
    if (nt_resync_cancelled) return;
    /* for each open input device, eat all buffers which are marked ready. The next one will thus be "busy". */
    post("resyncing audio");
    for (int n=0; n<mmi_nwave; n++) {
        int phase = mmi_phase[n], count;
        for (count=0; count<MAXRESYNC; count++) {
            WAVEHDR *h = mmi_vec[n][phase].lpWaveHdr;
            if (!(h->dwFlags & WHDR_DONE)) break;
            if (h->dwFlags & WHDR_PREPARED) waveInUnprepareHeader(mmi_dev[n], h, sizeof(WAVEHDR));
            h->dwFlags = 0L;
            waveInPrepareHeader(mmi_dev[n], h, sizeof(WAVEHDR));
            r = waveInAddBuffer(mmi_dev[n], h, sizeof(WAVEHDR));
            if (r != MMSYSERR_NOERROR) mmi_waveerror("waveInAddBuffer: %s", r);
            mmi_phase[n] = phase = WRAPFWD(phase+1);
        }
        if (count == MAXRESYNC) post("resync error at input");
    }
    /* Each output buffer which is "ready" is filled with zeros and queued. */
    for (int n=0; n<mmo_nwave; n++) {
        int phase = mmo_phase[n], count;
        for (count=0; count<MAXRESYNC; count++) {
            WAVEHDR *h = mmo_vec[n][phase].lpWaveHdr;
            if (!(h->dwFlags & WHDR_DONE)) break;
            if (h->dwFlags & WHDR_PREPARED) waveOutUnprepareHeader(mmo_dev[n], h, sizeof(WAVEHDR));
            h->dwFlags = 0L;
            memset((char *)(mmo_vec[n][phase].lpData), 0, (CHANNELS_PER_DEVICE * SAMPSIZE * nt_realdacblksize));
            waveOutPrepareHeader(mmo_dev[n], h, sizeof(WAVEHDR));
            r = waveOutWrite(mmo_dev[n], h, sizeof(WAVEHDR));
            if (r != MMSYSERR_NOERROR) mmo_waveerror("waveOutAddBuffer: %s", r);
            mmo_phase[n] = phase = WRAPFWD(phase+1);
        }
        if (count == MAXRESYNC) post("resync error at output");
    }
#ifdef MIDI_TIMESTAMP
    nt_resetmidisync();
#endif
}

#define LATE 0
#define RESYNC 1
#define NOTHING 2

void nt_logerror(int which) {
#if 0
    post("error %d %d", count, which);
    if (which < NOTHING) nt_errorcount++;
    if (which == RESYNC) nt_resynccount++;
    if (sys_getrealtime() > nt_nextreporttime) {
        post("%d audio I/O error%s", nt_errorcount, (nt_errorcount > 1 ? "s" : ""));
        if (nt_resynccount) post("DAC/ADC sync error");
        nt_errorcount = nt_resynccount = 0;
        nt_nextreporttime = sys_getrealtime() - 5;
    }
#endif
}

static int mmio_send_dacs() {
    UINT r;
    if (!mmi_nwave && !mmo_nwave) return 0;
    if (nt_meters) {
        float maxsamp = mmi_max;
        for (int i=0, n=2*mmi_nwave*sys_dacblocksize; i<n; i++) {
            float f = sys_soundin[i];
            if (f > maxsamp) maxsamp = f; else if (-f > maxsamp) maxsamp = -f;
        }
        mmi_max = maxsamp;
	maxsamp = mmo_max;
        for (int i=0, n=2*mmo_nwave*sys_dacblocksize; i<n; i++) {
            float f = sys_soundout[i];
            if (f > maxsamp) maxsamp = f; else if (-f > maxsamp) maxsamp = -f;
        }
        mmo_max = maxsamp;
    }
    /* the "fill pointer" nt_fill controls where in the next I/O buffers we will write and/or read.  If it's zero, we
       first check whether the buffers are marked "done". */
    if (!nt_fill) {
        for (int n=0; n<mmi_nwave; n++) {
            WAVEHDR *h = mmi_vec[n][mmi_phase[n]].lpWaveHdr; if (!(h->dwFlags & WHDR_DONE)) goto idle;
        }
        for (int n=0; n<mmo_nwave; n++) {
            WAVEHDR *h = mmo_vec[n][mmo_phase[n]].lpWaveHdr; if (!(h->dwFlags & WHDR_DONE)) goto idle;
        }
        for (int n=0; n<mmi_nwave; n++) {
            WAVEHDR *h = mmi_vec[n][mmi_phase[n]].lpWaveHdr; if (h->dwFlags & WHDR_PREPARED) waveInUnprepareHeader(mmi_dev[n],h,sizeof(WAVEHDR));
        }
        for (int n=0; n<mmo_nwave; n++) {
            WAVEHDR *h = mmo_vec[n][mmo_phase[n]].lpWaveHdr; if (h->dwFlags & WHDR_PREPARED) waveOutUnprepareHeader(mmo_dev[n],h,sizeof(WAVEHDR));
        }
    }
    /* Convert audio output to fixed-point and put it in the output buffer. */
    short *sp1, *sp2;
    float *fp1, *fp2;
    fp1 = sys_soundout;
    for (int n=0; n<mmo_nwave; n++) {
        int phase = mmo_phase[n];
	sp1=(short *)(mmo_vec[n][phase].lpData)+CHANNELS_PER_DEVICE*nt_fill;
        for (int i=0; i<2; i++, fp1 += sys_dacblocksize, sp1++) {
	    fp2 = fp1; sp2 = sp1;
            for (int j=0; j<sys_dacblocksize; j++, fp2++, sp2 += CHANNELS_PER_DEVICE) {
                *sp2 = clip(int(*fp2*32767),-32768,32767);
            }
        }
    }
    memset(sys_soundout, 0, (sys_dacblocksize *sizeof(t_sample)*CHANNELS_PER_DEVICE)*mmo_nwave);
    /* vice versa for the input buffer */
    fp1 = sys_soundin;
    for (int n=0; n<mmi_nwave; n++) {
        int phase = mmi_phase[n];
	sp1=(short *)(mmi_vec[n][phase].lpData)+CHANNELS_PER_DEVICE*nt_fill;
        for (int i=0; i<2; i++, fp1 += sys_dacblocksize, sp1++) {
	    fp2 = fp1; sp2 = sp1;
            for (int j=0; j<sys_dacblocksize; j++, fp2++, sp2 += CHANNELS_PER_DEVICE) *fp2 = float(1./32767.)*float(*sp2);
        }
    }
    nt_fill += sys_dacblocksize;
    if (nt_fill == nt_realdacblksize) {
        nt_fill = 0;
        for (int n=0; n<mmi_nwave; n++) {
            int phase = mmi_phase[n];
            HWAVEIN device = mmi_dev[n];
            WAVEHDR *h = mmi_vec[n][phase].lpWaveHdr;
            waveInPrepareHeader(device, h, sizeof(WAVEHDR));
            r = waveInAddBuffer(device, h, sizeof(WAVEHDR));
            if (r != MMSYSERR_NOERROR) mmi_waveerror("waveInAddBuffer: %s", r);
            mmi_phase[n] = WRAPFWD(phase+1);
        }
        for (int n=0; n<mmo_nwave; n++) {
            int phase = mmo_phase[n];
            HWAVEOUT device = mmo_dev[n];
            WAVEHDR *h = mmo_vec[n][phase].lpWaveHdr;
            waveOutPrepareHeader(device, h, sizeof(WAVEHDR));
            r = waveOutWrite(device, h, sizeof(WAVEHDR));
            if (r != MMSYSERR_NOERROR) mmo_waveerror("waveOutWrite: %s", r);
            mmo_phase[n] = WRAPFWD(phase+1);
        }
        /* check for DAC underflow or ADC overflow. */
        for (int n=0; n<mmi_nwave; n++) {
            int phase = WRAPBACK(mmi_phase[n]-2); WAVEHDR *h = mmi_vec[n][phase].lpWaveHdr;
            if (h->dwFlags & WHDR_DONE) goto late;
        }
        for (int n=0; n<mmo_nwave; n++) {
            int phase = WRAPBACK(mmo_phase[n]-2); WAVEHDR *h = mmo_vec[n][phase].lpWaveHdr;
            if (h->dwFlags & WHDR_DONE) goto late;
        }
    }
    return 1;
late:
    nt_logerror(LATE);
    nt_resyncaudio();
    return 1;
idle:
    /* If more than nt_adcjitterbufsallowed ADC buffers are ready on any input device, resynchronize */
    for (int n=0; n<mmi_nwave; n++) {
        WAVEHDR *h = mmi_vec[n][WRAPFWD(mmi_phase[n] + nt_adcjitterbufsallowed)].lpWaveHdr;
        if (h->dwFlags & WHDR_DONE) {nt_resyncaudio(); return 0;}
    }
    /* test dac sync the same way */
    for (int n=0; n<mmo_nwave; n++) {
        WAVEHDR *h = mmo_vec[n][WRAPFWD(mmo_phase[n] + nt_dacjitterbufsallowed)].lpWaveHdr;
        if (h->dwFlags & WHDR_DONE) {nt_resyncaudio(); return 0;}
    }
#ifdef MIDI_TIMESTAMP
    nt_midisync();
#endif
    return 0;
}

/* ------------------- public routines -------------------------- */

static int mmio_open_audio(
  int naudioidev, int *audioidev, int nchidev, int *chidev,
  int naudioodev, int *audioodev, int nchodev, int *chodev, int rate, int dummy) /* IOhannes */ {
    nt_realdacblksize = (sys_blocksize ? sys_blocksize : DEFREALDACBLKSIZE);
    int nbuf = sys_advance_samples/nt_realdacblksize;
    if (nbuf >= MAXBUFFER) {
        post("pd: audio buffering maxed out to %d", int(MAXBUFFER * ((nt_realdacblksize * 1000.)/44100.)));
        nbuf = MAXBUFFER;
    } else if (nbuf < 4) nbuf = 4;
    post("%d audio buffers", nbuf);
    nt_naudiobuffer = nbuf;
    if (nt_adcjitterbufsallowed > nbuf-2) nt_adcjitterbufsallowed = nbuf-2;
    if (nt_dacjitterbufsallowed > nbuf-2) nt_dacjitterbufsallowed = nbuf-2;
    mmi_nwave = sys_inchannels  / 2;
    mmo_nwave = sys_outchannels / 2;
    nt_whichadc = (naudioidev < 1 ? (mmi_nwave > 1 ? WAVE_MAPPER : -1) : audioidev[0]);
    nt_whichdac = (naudioodev < 1 ? (mmo_nwave > 1 ? WAVE_MAPPER : -1) : audioodev[0]);
    if (naudioodev>1 || naudioidev>1) post("separate audio device choice not supported; using sequential devices.");
    mmio_do_open_audio();
    return 0;
}

#if 0
/* list the audio and MIDI device names */
void mmio_listdevs() {
    UINT ndevices = waveInGetNumDevs();
    for (unsigned i=0; i<ndevices; i++) {
        WAVEINCAPS w;  UINT wRtn = waveInGetDevCaps( i, (LPWAVEINCAPS) &w, sizeof(w));
	if (wRtn) mmi_waveerror( "waveInGetDevCaps: %s",  wRtn); else post("audio input device #%d: %s", i+1, w.szPname);
    }
    ndevices = waveOutGetNumDevs();
    for (unsigned i=0; i<ndevices; i++) {
        WAVEOUTCAPS w; UINT wRtn = waveOutGetDevCaps(i, (LPWAVEOUTCAPS)&w, sizeof(w));
	if (wRtn) mmo_waveerror("waveOutGetDevCaps: %s", wRtn); else post("audio output device #%d: %s", i+1, w.szPname);
    }
}
#endif

static void mmio_getdevs(char *indevlist, int *nindevs, char *outdevlist, int *noutdevs, int *canmulti, int maxndev, int devdescsize) {
    *canmulti = 2;  /* supports multiple devices */
    int ndev = min(maxndev,int(waveInGetNumDevs()));
    *nindevs = ndev;
    for (int i=0; i<ndev; i++) {
        WAVEINCAPS w;  int wRtn = waveInGetDevCaps(i, (LPWAVEINCAPS) &w, sizeof(w));
        sprintf(indevlist  + i*devdescsize, wRtn?"???":w.szPname);
    }
    ndev = min(maxndev,int(waveOutGetNumDevs()));
    *noutdevs = ndev;
    for (int i=0; i<ndev; i++) {
        WAVEOUTCAPS w; int wRtn = waveOutGetDevCaps(i,(LPWAVEOUTCAPS)&w, sizeof(w));
        sprintf(outdevlist + i*devdescsize, wRtn?"???":w.szPname);
    }
}

struct t_audioapi api_mmio = {
	mmio_open_audio,
	mmio_close_audio,
	mmio_send_dacs,
	mmio_getdevs,
};
