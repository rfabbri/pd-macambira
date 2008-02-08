/* Copyright (c) 2003, Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  machine-independent (well, mostly!) audio layer.  Stores and recalls
    audio settings from argparse routine and from dialog window.
*/

#define PD_PLUSPLUS_FACE
#include "m_pd.h"
#include "s_stuff.h"
#include "m_simd.h"
#include <stdio.h>
#ifdef UNISTD
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sstream>

#define SYS_DEFAULTCH 2
#define SYS_MAXCH 100
typedef long t_pa_sample;
#define SYS_SAMPLEWIDTH sizeof(t_pa_sample)
#define SYS_BYTESPERCHAN (sys_dacblocksize * SYS_SAMPLEWIDTH)
#define SYS_XFERSAMPS (SYS_DEFAULTCH*sys_dacblocksize)
#define SYS_XFERSIZE (SYS_SAMPLEWIDTH * SYS_XFERSAMPS)
#define MAXNDEV 100
#define DEVDESCSIZE 80

extern t_audioapi pa_api, jack_api, oss_api, alsa_api, sgi_api, mmio_api, asio_api;

t_sample *sys_soundin;
t_sample *sys_soundout;
float sys_dacsr;

using namespace std;

static t_audioapi *sys_audio() {
#ifdef USEAPI_PORTAUDIO
    if (sys_audioapi == API_PORTAUDIO) return &pa_api;
#endif
#ifdef USEAPI_JACK
    if (sys_audioapi == API_JACK) return &jack_api;
#endif
#ifdef USEAPI_OSS
    if (sys_audioapi == API_OSS) return &oss_api;
#endif
#ifdef USEAPI_ALSA
    if (sys_audioapi == API_ALSA) return &alsa_api;
#endif
#ifdef USEAPI_SGI
    if (sys_audioapi == API_SGI) return &sgi_api;
#endif
#ifdef USEAPI_MMIO
    if (sys_audioapi == API_MMIO) return &mmio_api;
#endif
#ifdef USEAPI_ASIO
	if (sys_audioapi == API_ASIO) return &asio_api;
#endif
    post("sys_close_audio: unknown API %d", sys_audioapi);
    sys_inchannels = sys_outchannels = 0;
    sched_set_using_dacs(0); /* tb: dsp is switched off */
    return 0;
}

static void audio_getdevs(char *indevlist, int *nindevs, char *outdevlist, int *noutdevs, int *canmulti, int maxndev, int devdescsize);

/* these are set in this file when opening audio, but then may be reduced,
   even to zero, in the system dependent open_audio routines. */
int sys_inchannels;
int sys_outchannels;
int sys_advance_samples;        /* scheduler advance in samples */
int sys_blocksize = 0;          /* audio I/O block size in sample frames */
#ifndef API_DEFAULT
#define API_DEFAULT 0
#endif
int sys_audioapi = API_DEFAULT;
int sys_dacblocksize;
int sys_schedblocksize;
int sys_meters;          /* true if we're metering */
static float sys_inmax;         /* max input amplitude */
static float sys_outmax;        /* max output amplitude */
int sys_schedadvance;   /* scheduler advance in microseconds */

/* the "state" is normally one if we're open and zero otherwise; but if the state is one,
   we still haven't necessarily opened the audio hardware; see audio_isopen() below. */
static int audio_state;

/* last requested parameters */
static t_audiodevs audio_in;
static t_audiodevs audio_out;
static int audio_rate;
static int audio_dacblocksize;
static int audio_advance;
static int audio_scheduler;

extern int sys_callbackscheduler;

float peakvec(t_float* vec, t_int n, t_float cur_max);
static float (*peak_fp)(t_float*, t_int, t_float) = peakvec;

static int audio_isopen() {
    return audio_state && ((audio_in.ndev > 0 &&  audio_in.chdev[0] > 0)
                       || (audio_out.ndev > 0 && audio_out.chdev[0] > 0));
}

extern "C" void sys_get_audio_params(t_audiodevs *in, t_audiodevs *out, int *prate, int *pdacblocksize, int *padvance, int *pscheduler) {
    in->ndev  = audio_in.ndev;
    out->ndev = audio_out.ndev;
    for (int i=0; i<MAXAUDIOINDEV; i++)  {in ->dev[i] = audio_in.dev[i];   in->chdev[i]  = audio_in.chdev[i];}
    for (int i=0; i<MAXAUDIOOUTDEV; i++) {out->dev[i] = audio_out.dev[i]; out->chdev[i] = audio_out.chdev[i];}
    *prate = audio_rate;
    *pdacblocksize = audio_dacblocksize;
    *padvance = audio_advance;
    *pscheduler = audio_scheduler;
}

void sys_save_audio_params(
int nindev,  int *indev,  int *chindev,
int noutdev, int *outdev, int *choutdev,
int rate, int dacblocksize, int advance, int scheduler) {
    audio_in.ndev  = nindev;
    audio_out.ndev = noutdev;
    for (int i=0; i<MAXAUDIOINDEV;  i++) {audio_in.dev[i]  = indev[i];  audio_in.chdev[i]  = chindev[i];}
    for (int i=0; i<MAXAUDIOOUTDEV; i++) {audio_out.dev[i] = outdev[i]; audio_out.chdev[i] = choutdev[i];}
    audio_rate = rate;
    audio_dacblocksize = dacblocksize;
    audio_advance = advance;
    audio_scheduler = scheduler;
}

extern "C" void sys_open_audio2(t_audiodevs *in, t_audiodevs *out, int rate, int dacblocksize, int advance, int scheduler) {
	sys_open_audio(in->ndev,  in->dev,  in->ndev,  in->chdev,
		      out->ndev, out->dev, out->ndev, out->chdev, rate, dacblocksize, advance, scheduler, 1);
}

/* init routines for any API which needs to set stuff up before any other API gets used.  This is only true of OSS so far. */
#ifdef USEAPI_OSS
void oss_init();
#endif

static void audio_init() {
    static int initted = 0;
    if (initted) return;
    initted = 1;
#ifdef USEAPI_OSS
    oss_init();
#endif
}

/* set channels and sample rate.  */
void sys_setchsr(int chin, int chout, int sr, int dacblocksize) {
    int inbytes  = (chin  ? chin  : 2) * (sys_dacblocksize*sizeof(float));
    int outbytes = (chout ? chout : 2) * (sys_dacblocksize*sizeof(float));
    if (dacblocksize != (1<<ilog2(dacblocksize))) {
      dacblocksize = 1<<ilog2(dacblocksize);
      post("warning: adjusting dac~blocksize to power of 2: %d", dacblocksize);
    }
    sys_dacblocksize   = dacblocksize;
    sys_schedblocksize = dacblocksize;
    sys_inchannels  = chin;
    sys_outchannels = chout;
    sys_dacsr = double(sr);
    sys_advance_samples = max(int(sys_schedadvance*sys_dacsr/1000000.),sys_dacblocksize);
    if (sys_soundin) freealignedbytes(sys_soundin,inbytes);
    sys_soundin = (t_float *)getalignedbytes(inbytes);
    memset(sys_soundin, 0, inbytes);
    if (sys_soundout) freealignedbytes(sys_soundout,outbytes);
    sys_soundout = (t_float *)getalignedbytes(outbytes);
    memset(sys_soundout, 0, outbytes);
    /* tb: modification for simd-optimized peak finding */
    if (SIMD_CHKCNT(sys_inchannels * sys_dacblocksize) &&
	SIMD_CHKCNT(sys_outchannels * sys_dacblocksize))
	 peak_fp = peakvec_simd;
    else peak_fp = peakvec;
    if (sys_verbose) post("input channels = %d, output channels = %d", sys_inchannels, sys_outchannels);
    canvas_resume_dsp(canvas_suspend_dsp());
}

/* ----------------------- public routines ----------------------- */

/* open audio devices (after cleaning up the specified device and channel vectors).  The audio devices are "zero based"
   (i.e. "0" means the first one.)  We also save the cleaned-up device specification so that we
    can later re-open audio and/or show the settings on a dialog window. */
void sys_open_audio(int nindev, int *indev, int nchindev, int *chindev, int noutdev, int *outdev, int nchoutdev,
int *choutdev, int rate, int dacblocksize, int advance, int schedmode, int enable) {
    int defaultchannels = SYS_DEFAULTCH;
    int realinchans[MAXAUDIOINDEV], realoutchans[MAXAUDIOOUTDEV];
    char indevlist[MAXNDEV*DEVDESCSIZE], outdevlist[MAXNDEV*DEVDESCSIZE];
    int indevs = 0, outdevs = 0, canmulti = 0;
    audio_getdevs(indevlist, &indevs, outdevlist, &outdevs, &canmulti, MAXNDEV, DEVDESCSIZE);
    if (sys_externalschedlib) return;
    if (sys_inchannels || sys_outchannels) sys_close_audio();
    if (rate < 1) rate = DEFAULTSRATE;
    if (dacblocksize < 1) dacblocksize = DEFDACBLKSIZE;
    if (advance <= 0) advance = DEFAULTADVANCE;
     audio_init();
     /* Since the channel vector might be longer than the audio device vector, or vice versa, we fill the shorter one
        in to match the longer one.  Also, if both are empty, we fill in one device (the default) and two channels. */
    if (nindev == -1) { /* no input audio devices specified */
        if (nchindev == -1) {
            if (indevs >= 1) {
                nchindev=1;
                chindev[0] = defaultchannels;
                nindev = 1;
                indev[0] = DEFAULTAUDIODEV;
            } else nindev = nchindev = 0;
        } else {
            for (int i=0; i<MAXAUDIOINDEV; i++) indev[i] = i;
            nindev = nchindev;
        }
    } else {
        if (nchindev == -1) {
            nchindev = nindev;
            for (int i=0; i<nindev; i++) chindev[i] = defaultchannels;
        } else if (nchindev > nindev) {
            for (int i=nindev; i<nchindev; i++) {
                if (i == 0) indev[0] = DEFAULTAUDIODEV; else indev[i] = indev[i-1] + 1;
            }
            nindev = nchindev;
        } else if (nchindev < nindev) {
            for (int i=nchindev; i<nindev; i++) {
                if (i == 0) chindev[0] = defaultchannels; else chindev[i] = chindev[i-1];
            }
            nindev = nchindev;
        }
    }
    if (noutdev == -1) { /* not set */
        if (nchoutdev == -1) {
            if (outdevs >= 1) {
                nchoutdev=1;
                choutdev[0]=defaultchannels;
                noutdev=1;
                outdev[0] = DEFAULTAUDIODEV;
            } else nchoutdev = noutdev = 0;
        } else {
            for (int i=0; i<MAXAUDIOOUTDEV; i++) outdev[i] = i;
            noutdev = nchoutdev;
        }
    } else {
        if (nchoutdev == -1) {
            nchoutdev = noutdev;
            for (int i=0; i<noutdev; i++) choutdev[i] = defaultchannels;
        } else if (nchoutdev > noutdev) {
            for (int i=noutdev; i<nchoutdev; i++) {
                if (i == 0) outdev[0] = DEFAULTAUDIODEV; else outdev[i] = outdev[i-1] + 1;
            }
            noutdev = nchoutdev;
        } else if (nchoutdev < noutdev) {
            for (int i=nchoutdev; i<noutdev; i++) {
                if (i == 0) choutdev[0] = defaultchannels; else choutdev[i] = choutdev[i-1];
            }
            noutdev = nchoutdev;
        }
    }
    /* count total number of input and output channels */
    int inchans=0, outchans=0;
    for (int i=0;  i < nindev;  i++)  inchans += (realinchans[i]  = (chindev[i] > 0  ?  chindev[i] : 0));
    for (int i=0; i < noutdev; i++) outchans += (realoutchans[i] = (choutdev[i] > 0 ? choutdev[i] : 0));
    /* if no input or output devices seem to have been specified, this really means just disable audio, which we now do. */
    if (!inchans && !outchans) enable = 0;
    sys_schedadvance = advance * 1000;
    sys_setchsr(inchans, outchans, rate, dacblocksize);
    sys_log_error(ERR_NOTHING);
    if (enable) {
        /* for alsa, only one device is supported; it may be open for both input and output. */
#ifdef USEAPI_PORTAUDIO
        if (sys_audioapi == API_PORTAUDIO)
            pa_open_audio(inchans, outchans, rate, advance,
		  (noutdev > 0 ? indev[0] : 0),
		  (noutdev > 0 ? outdev[0] : 0), schedmode);
	else
#endif
#ifdef USEAPI_JACK
        if (sys_audioapi == API_JACK)
            jack_open_audio((nindev > 0 ? realinchans[0] : 0),
                            (noutdev > 0 ? realoutchans[0] : 0), rate, schedmode);
        else
#endif
        if (sys_audioapi == API_OSS || sys_audioapi == API_ALSA || sys_audioapi == API_MMIO)
            sys_audio()->open_audio(nindev, indev, nchindev, realinchans, noutdev, outdev, nchoutdev, realoutchans, rate, -42);
        else if (sys_audioapi == API_SGI)
            sys_audio()->open_audio(nindev, indev, nchindev,     chindev, noutdev, outdev, nchoutdev,     choutdev, rate, -42);
        else if (sys_audioapi == API_ASIO)
	    sys_audio()->open_audio(nindev, indev, nchindev,     chindev, noutdev, outdev, nchoutdev,     choutdev, rate, schedmode);
	else post("unknown audio API specified");
    }
    sys_save_audio_params(nindev, indev, chindev, noutdev, outdev, choutdev, int(sys_dacsr), sys_dacblocksize, advance, schedmode);
    if (sys_inchannels == 0 && sys_outchannels == 0) enable = 0;
    audio_state = enable;
    sys_vgui("set pd_whichapi %d\n", audio_isopen() ? sys_audioapi : 0);
    sched_set_using_dacs(enable);
	sys_update_sleepgrain();
	if (enable) {
		t_atom argv[1];
		t_symbol *selector = gensym("audio_started");
		t_symbol *pd = gensym("pd");
		SETFLOAT(argv, 1.);
		typedmess(pd->s_thing, selector, 1, argv);
	}
}

void sys_close_audio() {
    /* jsarlo { (*/
    if (sys_externalschedlib) return;
    /* } jsarlo */
    if (!audio_isopen()) return;
    if (sys_audio()) sys_audio()->close_audio();
    else post("sys_close_audio: unknown API %d", sys_audioapi);
    sys_inchannels = sys_outchannels = 0;
    sched_set_using_dacs(0); /* tb: dsp is switched off */
}

/* open audio using whatever parameters were last used */
void sys_reopen_audio() {
    t_audiodevs in, out;
    int rate, dacblocksize, advance, scheduler;
    sys_close_audio();
    sys_get_audio_params(&in,&out,&rate, &dacblocksize, &advance, &scheduler);
    sys_open_audio2(&in,&out, rate, dacblocksize, advance, scheduler);
}

/* tb: default value of peak_fp {*/
float peakvec(t_float* vec, t_int n, t_float cur_max) {
	for (int i=0; i<n; i++) {
		float f = *vec++;
		if (f > cur_max) cur_max = f;
		else if (-f > cur_max) cur_max = -f;
	}
	return cur_max;
}
/* } */

void sys_peakmeters() {
    if (sys_inchannels)  sys_inmax  = peak_fp(sys_soundin, sys_inchannels  * sys_dacblocksize, sys_inmax);
    if (sys_outchannels) sys_outmax = peak_fp(sys_soundout,sys_outchannels * sys_dacblocksize, sys_outmax);
}

int sys_send_dacs() {
    if (sys_meters) sys_peakmeters();
    if (sys_audio()) return sys_audio()->send_dacs();
    post("unknown API");
    return 0;
}

float sys_getsr() {return sys_dacsr;}
int sys_get_outchannels() {return sys_outchannels;}
int sys_get_inchannels()  {return sys_inchannels;}

void sys_getmeters(float *inmax, float *outmax) {
    if (inmax) {
        sys_meters = 1;
        *inmax = sys_inmax;
        *outmax = sys_outmax;
    } else sys_meters = 0;
    sys_inmax = sys_outmax = 0;
}

static void audio_getdevs(char *indevlist, int *nindevs, char *outdevlist, int *noutdevs, int *canmulti, int maxndev, int devdescsize) {
    audio_init();
    if (sys_audio()) sys_audio()->getdevs(indevlist, nindevs, outdevlist, noutdevs, canmulti, maxndev, devdescsize);
    else {*nindevs = *noutdevs = 0;}
}

static void sys_listaudiodevs() {
    char indevlist[MAXNDEV*DEVDESCSIZE], outdevlist[MAXNDEV*DEVDESCSIZE];
    int nindevs = 0, noutdevs = 0, canmulti = 0;
    audio_getdevs(indevlist, &nindevs, outdevlist, &noutdevs, &canmulti,  MAXNDEV, DEVDESCSIZE);
    /* To agree with command line flags, normally start at 1; but microsoft "MMIO" device list starts at 0 (the "mapper"). */
    /* (see also sys_mmio variable in s_main.c)  */
    if (!nindevs) post("no audio input devices found");
    else {
        post("audio input devices:");
        for (int i=0; i<nindevs;  i++) post("%d. %s", i + (sys_audioapi != API_MMIO),  indevlist + i * DEVDESCSIZE);
    }
    if (!noutdevs)  post("no audio output devices found");
    else {
        post("audio output devices:");
        for (int i=0; i<noutdevs; i++) post("%d. %s", i + (sys_audioapi != API_MMIO), outdevlist + i * DEVDESCSIZE);
    }
    post("API number %d", sys_audioapi);
}

/* start an audio settings dialog window */
void glob_audio_properties(t_pd *dummy, t_floatarg flongform) {
    /* these are the devices you're using: */
    t_audiodevs in,out;
    int rate, dacblocksize, advance, scheduler;
    /* these are all the devices on your system: */
    char indevlist[MAXNDEV*DEVDESCSIZE], outdevlist[MAXNDEV*DEVDESCSIZE];
    int nindevs = 0, noutdevs = 0, canmulti = 0;
    audio_getdevs(indevlist, &nindevs, outdevlist, &noutdevs, &canmulti, MAXNDEV, DEVDESCSIZE);
    ostringstream indevliststring;  for (int i=0; i<nindevs;  i++) indevliststring  << " {" <<  (indevlist + i*DEVDESCSIZE) << "}";
    ostringstream outdevliststring; for (int i=0; i<noutdevs; i++) outdevliststring << " {" << (outdevlist + i*DEVDESCSIZE) << "}";
    sys_get_audio_params(&in,&out,&rate,&dacblocksize,&advance,&scheduler);
    if (in.ndev > 1 || out.ndev > 1) flongform = 1;
    ostringstream indevs;   for (int i=0; i< in.ndev; i++) indevs   << " " <<   in.dev [i];
    ostringstream outdevs;  for (int i=0; i<out.ndev; i++) outdevs  << " " <<   out.dev[i];
    ostringstream inchans;  for (int i=0; i< in.ndev; i++) inchans  << " " <<  in.chdev[i];
    ostringstream outchans; for (int i=0; i<out.ndev; i++) outchans << " " << out.chdev[i];
    sys_vgui("pdtk_audio_dialog {%s} {%s} {%s} {%s} {%s} {%s} %d %d %d %d %d\n",
        indevliststring .str().data()+1,  indevs.str().data()+1,  inchans.str().data()+1,
        outdevliststring.str().data()+1, outdevs.str().data()+1, outchans.str().data()+1,
        rate, dacblocksize, advance, canmulti, flongform!=0);
}

/* new values from dialog window */
void glob_audio_dialog(t_pd *dummy, t_symbol *s, int argc, t_atom *argv) {
    int nindev=0, noutdev=0;
    int newindev[4], newinchan[4], newoutdev[4], newoutchan[4];
    /* the new values the dialog came back with: */
    int newrate         = atom_getintarg(16, argc, argv);
    int newdacblocksize = atom_getintarg(17, argc, argv);
    int newadvance      = atom_getintarg(18, argc, argv);
    int newschedmode    = atom_getintarg(19, argc, argv);
    for (int i=0; i<4; i++) {
        newindev[i]   = atom_getintarg(i,    argc, argv);
        newinchan[i]  = atom_getintarg(i+4,  argc, argv);
        newoutdev[i]  = atom_getintarg(i+8,  argc, argv);
        newoutchan[i] = atom_getintarg(i+12, argc, argv);
    }
    for (int i=0; i<4; i++) {
        if (newinchan[i]) {
            newindev[nindev]  = newindev[i];
            newinchan[nindev] = newinchan[i];
            nindev++;
        }
    }
    for (int i=0; i<4; i++) {
        if (newoutchan[i]) {
            newoutdev[noutdev]  = newoutdev[i];
            newoutchan[noutdev] = newoutchan[i];
            noutdev++;
        }
    }
    sys_close_audio();
    sys_open_audio(nindev,  newindev,  nindev,  newinchan,
                   noutdev, newoutdev, noutdev, newoutchan,
                   newrate, newdacblocksize, newadvance, newschedmode, 1);
}

extern void sgi_listaudiodevs();
void sys_listdevs() {
    if (sys_audioapi == API_PORTAUDIO) sys_listaudiodevs(); else
#ifdef USEAPI_JACK
    if (sys_audioapi == API_JACK)      jack_listdevs();     else
#endif
    if (sys_audioapi == API_OSS)       sys_listaudiodevs(); else
    if (sys_audioapi == API_ALSA)      sys_listaudiodevs(); else
#ifdef USEAPI_SGI
    if (sys_audioapi == API_SGI)       sgi_listaudiodevs(); else
#endif
    if (sys_audioapi == API_MMIO)      sys_listaudiodevs(); else
    post("unknown API");
    sys_listmididevs();
}

void sys_setblocksize(int n) {
    if (n < 1) n = 1;
    if (n != (1 << ilog2(n))) post("warning: adjusting blocksize to power of 2: %d", (n = (1 << ilog2(n))));
    sys_blocksize = n;
}

void sys_set_audio_api(int which) {
     sys_audioapi = which;
     if (sys_verbose) post("sys_audioapi %d", sys_audioapi);
}

void glob_audio_setapi(t_pd *dummy, t_floatarg f) {
    int newapi = int(f);
    if (newapi != sys_audioapi) {
	if (newapi != sys_audioapi) {
            sys_close_audio();
            sys_audioapi = newapi;
	    /* bash device params back to default */
            audio_in.ndev = audio_out.ndev = 1;
            audio_in.dev[0]   = audio_out.dev[0] = DEFAULTAUDIODEV;
            audio_in.chdev[0] = audio_out.chdev[0] = SYS_DEFAULTCH;
	}
	sched_set_using_dacs(0);
/* 	glob_audio_properties(0, 0); */
    }
}

/* start or stop the audio hardware */
void sys_set_audio_state(int onoff) {
    if (onoff) {  /* start */
        if (!audio_isopen()) sys_reopen_audio();
    } else {
        if (audio_isopen())  sys_close_audio();
    }
    sched_set_using_dacs(onoff);
    sys_setscheduler(sys_getscheduler()); /* tb: reset scheduler */
    audio_state = onoff;
}

void sys_get_audio_apis(char *buf) {
    int n = 0;
    strcpy(buf, "{ ");
#ifdef USEAPI_OSS
    sprintf(buf + strlen(buf), "{OSS %d} ", API_OSS); n++;
#endif
#ifdef USEAPI_ASIO
    sprintf(buf + strlen(buf), "{ASIO %d} ", API_ASIO); n++;
#endif
#ifdef USEAPI_MMIO
    sprintf(buf + strlen(buf), "{\"standard (MMIO)\" %d} ", API_MMIO); n++;
#endif
#ifdef USEAPI_ALSA
    sprintf(buf + strlen(buf), "{ALSA %d} ", API_ALSA); n++;
#endif
#ifdef USEAPI_PORTAUDIO
#ifdef __APPLE__
    sprintf(buf + strlen(buf), "{\"standard (portaudio)\" %d} ", API_PORTAUDIO);  n++;
#else
    sprintf(buf + strlen(buf), "{portaudio %d} ", API_PORTAUDIO);  n++;
#endif
#endif
#ifdef USEAPI_SGI
    sprintf(buf + strlen(buf), "{SGI %d} ", API_SGI); n++;
#endif
#ifdef USEAPI_JACK
    sprintf(buf + strlen(buf), "{jack %d} ", API_JACK); n++;
#endif
    strcat(buf, "}");
}

#ifdef USEAPI_ALSA
void alsa_putzeros(int iodev, int n);
void alsa_getzeros(int iodev, int n);
void alsa_printstate();
#endif

/* debugging */
void glob_foo(void *dummy, t_symbol *s, int argc, t_atom *argv) {
    t_symbol *arg = atom_getsymbolarg(0, argc, argv);
    if      (arg == gensym("restart"))   sys_reopen_audio();
#ifdef USEAPI_ALSA
    /* what's the matter here? what should be the value of iodev??? */
    else if (arg == gensym("alsawrite")) alsa_putzeros(0, atom_getintarg(1, argc, argv));
    else if (arg == gensym("alsaread"))  alsa_getzeros(0, atom_getintarg(1, argc, argv));
    else if (arg == gensym("print"))     alsa_printstate();
#endif
}

/* tb: message-based audio configuration
 * supported by vibrez.net { */
void glob_audio_samplerate(t_pd * dummy, t_float f) {
	t_audiodevs in, out;
	int rate, dacblocksize, advance, scheduler;
	if (f == sys_getsr()) return;
	sys_get_audio_params(&in,&out,&rate, &dacblocksize, &advance, &scheduler);
	sys_close_audio();
	sys_open_audio(in.ndev, in.dev, in.ndev, in.chdev, out.ndev, out.dev, out.ndev, out.chdev,
		(int)f, dacblocksize, advance, scheduler, 1);
}

void glob_audio_api(t_pd *dummy, t_float f) {
	int newapi = (int)f;
	sys_close_audio();
	sys_audioapi = newapi;
}

void glob_audio_delay(t_pd *dummy, t_float f) {
	t_audiodevs in,out;
	int rate, dacblocksize, advance, scheduler;
	if ((int)f == audio_advance) return;
	sys_get_audio_params(&in,&out, &rate, &dacblocksize, &advance, &scheduler);
	sys_close_audio();
	sys_open_audio(in.ndev, in.dev, in.ndev, in.chdev, out.ndev, out.dev, out.ndev, out.chdev,
		rate, dacblocksize, (int) f, scheduler, 1);
}

void glob_audio_dacblocksize(t_pd * dummy, t_float f) {
	t_audiodevs in,out;
	int rate, dacblocksize, advance, scheduler;
	if ((int)f == audio_dacblocksize) return;
	sys_get_audio_params(&in,&out, &rate, &dacblocksize, &advance, &scheduler);
	sys_close_audio();
	sys_open_audio2(&in,&out, rate, (int)f, advance, scheduler);
}

void glob_audio_scheduler(t_pd * dummy, t_float f) {
	t_audiodevs in,out;
	int rate, dacblocksize, advance, scheduler;
	if ((int)f == sys_callbackscheduler) return;
	scheduler = f!=0;
	sys_get_audio_params(&in,&out, &rate, &dacblocksize, &advance, &scheduler);
	sys_close_audio();
	sys_open_audio2(&in,&out, rate, dacblocksize, advance, scheduler);
	if (scheduler != sys_callbackscheduler) {
		if (scheduler == 1) {
			post("switched to callback-based scheduler");
		} else {
			post("switched to traditional scheduler");
		}
	} else post("couldn't change scheduler");
}

void glob_audio_device(t_pd *dummy, t_symbol *s, int argc, t_atom *argv) {
	t_audiodevs in,out;
	int rate, dacblocksize, advance, scheduler;
	sys_get_audio_params(&in,&out, &rate, &dacblocksize, &advance, &scheduler);
	out.ndev = in.ndev = (int)atom_getfloatarg(0, argc, argv);
	for (int i=0; i<MAXAUDIOINDEV; i++) {
		out.dev  [i] = in.dev  [i] = int(atom_getfloatarg(i*2+1, argc, argv));
		out.chdev[i] = in.chdev[i] = int(atom_getfloatarg(i*2+2, argc, argv));
	}
	sys_close_audio();
	sys_open_audio2(&in,&out, rate, dacblocksize, advance, scheduler);
}

void glob_audio_device_in(t_pd *dummy, t_symbol *s, int argc, t_atom *argv) {
	t_audiodevs in,out;
	int rate, dacblocksize, advance, scheduler;
	sys_get_audio_params(&in,&out, &rate, &dacblocksize, &advance, &scheduler);
	in.ndev = (int)atom_getfloatarg(0, argc, argv);
	for (int i=0; i<MAXAUDIOINDEV; i=i+2) {
		in.dev  [i] = atom_getintarg(i+1, argc, argv);
		in.chdev[i] = atom_getintarg(i+2, argc, argv);
	}
	sys_close_audio();
	sys_open_audio2(&in,&out,rate, dacblocksize, advance, scheduler);
}

void glob_audio_device_out(t_pd *dummy, t_symbol *s, int argc, t_atom *argv) {
	t_audiodevs in,out;
	int rate, dacblocksize, advance, scheduler;
	sys_get_audio_params(&in,&out, &rate, &dacblocksize, &advance, &scheduler);
	out.ndev = (int)atom_getfloatarg(0, argc, argv);
	/* i+=2 ? isn't that a bug??? */
	for (int i=0; i<MAXAUDIOOUTDEV; i+=2) {
		out.dev  [i] = atom_getintarg(i+1, argc, argv);
		out.chdev[i] = atom_getintarg(i+2, argc, argv);
	}
	sys_close_audio();
	sys_open_audio2(&in,&out, rate, dacblocksize, advance, scheduler);
}

/* some general helper functions */
void sys_update_sleepgrain() {
    sys_sleepgrain = sys_schedadvance/4;
    if (sys_sleepgrain < 1000) sys_sleepgrain = 1000;
    else if (sys_sleepgrain > 5000) sys_sleepgrain = 5000;
}

/* t_audiodevs are the ones you're using; char[] are all the devices available. */
void glob_audio_getaudioindevices(t_pd * dummy, t_symbol *s, int ac, t_atom *av) {
    t_audiodevs in,out;
    int rate, dacblocksize, advance, scheduler;
    int nindevs = 0, noutdevs = 0, canmulti = 0;
    t_atom argv[MAXNDEV];
    int f = ac ? (int)atom_getfloatarg(0,ac,av) : -1;
    t_symbol *selector = gensym("audioindev");
    t_symbol *pd = gensym("pd");
    char indevlist[MAXNDEV*DEVDESCSIZE], outdevlist[MAXNDEV*DEVDESCSIZE];
    audio_getdevs(indevlist, &nindevs, outdevlist, &noutdevs, &canmulti, MAXNDEV, DEVDESCSIZE);
    sys_get_audio_params(&in,&out, &rate, &dacblocksize, &advance, &scheduler);
    if (f < 0) {
        for (int i=0; i<nindevs; i++) SETSTRING(argv+i,indevlist+i*DEVDESCSIZE);
        typedmess(pd->s_thing, selector, nindevs, argv);
    } else if (f < nindevs) {
        SETSTRING(argv, indevlist + f * DEVDESCSIZE);
        typedmess(pd->s_thing, selector, 1, argv);
    }
}
void glob_audio_getaudiooutdevices(t_pd * dummy, t_symbol *s, int ac, t_atom *av) {
    t_audiodevs in,out;
    int rate, dacblocksize, advance, scheduler;
    int nindevs = 0, noutdevs = 0, canmulti = 0;
    t_atom argv[MAXNDEV];
    int f = ac ? (int)atom_getfloatarg(0,ac,av) : -1;
    t_symbol *selector = gensym("audiooutdev");
    t_symbol *pd = gensym("pd");
    char indevlist[MAXNDEV*DEVDESCSIZE], outdevlist[MAXNDEV*DEVDESCSIZE];
    audio_getdevs(indevlist, &nindevs, outdevlist, &noutdevs, &canmulti, MAXNDEV, DEVDESCSIZE);
    sys_get_audio_params(&in,&out, &rate, &dacblocksize, &advance, &scheduler);
    if (f < 0) {
        for (int i=0; i<noutdevs; i++) SETSYMBOL(argv+i, gensym(outdevlist+i*DEVDESCSIZE));
        typedmess(pd->s_thing, selector, noutdevs, argv);
    } else if (f < noutdevs) {
        SETSTRING(argv, outdevlist + f  * DEVDESCSIZE);
        typedmess(pd->s_thing, selector, 1, argv);
    }
}

/* some prototypes from s_audio_portaudio.c */
extern void pa_getcurrent_devices();
extern void pa_getaudioininfo(t_float f);
extern void pa_getaudiooutinfo(t_float f);
extern void pa_test_setting (int ac, t_atom *av);
extern void pa_get_asio_latencies(t_float f);

void glob_audio_getaudioininfo(t_pd * dummy, t_float f) {
#if defined(USEAPI_PORTAUDIO) && !defined(PABLIO)
    if (sys_audioapi == API_PORTAUDIO) pa_getaudioininfo(f);
#endif
}
void glob_audio_getaudiooutinfo(t_pd * dummy, t_float f) {
#if defined(USEAPI_PORTAUDIO) && !defined(PABLIO)
    if (sys_audioapi == API_PORTAUDIO) pa_getaudiooutinfo(f);
#endif
}
void glob_audio_testaudiosetting(t_pd * dummy, t_symbol *s, int ac, t_atom *av) {
#if defined(USEAPI_PORTAUDIO) && !defined(PABLIO)
    if (sys_audioapi == API_PORTAUDIO) pa_test_setting (ac, av);
#endif
}
void glob_audio_getcurrent_devices() {
#if defined(USEAPI_PORTAUDIO) && !defined(PABLIO)
    if (sys_audioapi == API_PORTAUDIO) pa_getcurrent_devices();
#endif
}
void glob_audio_asio_latencies(t_pd * dummy, t_float f) {
#if defined(USEAPI_PORTAUDIO) && !defined(PABLIO)
    if (sys_audioapi == API_PORTAUDIO) pa_get_asio_latencies(f);
#endif
}

/* tb } */
