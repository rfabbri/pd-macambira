/* Copyright (c) 2001 Miller Puckette and others.
 * Copyright (c) 2005-2006 Tim Blechmann
 * supported by vibrez.net
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* this file calls Ross Bencina's and Phil Burk's Portaudio package.  It's
   the main way in for Mac OS and, with Michael Casey's help, also into
   ASIO in Windows. */

/* tb: requires portaudio >= V19 */

#include "m_pd.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <portaudio.h>
#include <errno.h>
#include "assert.h"
#ifdef MSW
# include <malloc.h>
# include <pa_asio.h>
#else
# include <alloca.h>
#endif
#include "pthread.h"
/* for M_PI */
#if defined(_MSC_VER) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES
#endif
#include <math.h>
#define MAX_PA_CHANS 32

static int pa_inchans, pa_outchans;
static int pa_blocksize;

static PaStream *pa_stream;
/* Initialize PortAudio  */
PaError pa_status = -1;
int pa_initialized = 0;

void pa_initialize() {
//    if (pa_initialized) return;
    pa_status = Pa_Initialize();
    if (pa_status!=paNoError) {
        error("Error number %d occured initializing portaudio: %s", pa_status, Pa_GetErrorText(pa_status));
        return;
    }
    pa_initialized = 1;
}

static float* pa_inbuffer[MAX_PA_CHANS];
static float* pa_outbuffer[MAX_PA_CHANS];
static int pa_bufferpos;
static int pddev2padev(int pdindev,int isinput);
static int padev2pddev(int padev,int isinput);
int process (const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags, void *userData);

static int pa_indev = -1, pa_outdev = -1;

int pa_open_audio(int inchans, int outchans, int rate, int advance,
int indeviceno, int outdeviceno, int schedmode) {
    PaError err;
    const PaDeviceInfo *pdi,*pdo;
    schedmode = 1; /* we don't support blocking io */
    pa_initialize();
    sys_setscheduler(schedmode);
    /* post("in %d out %d rate %d device %d", inchans, outchans, rate, deviceno); */
    if ( inchans > MAX_PA_CHANS) {post( "input channels reduced to maximum %d", MAX_PA_CHANS);  inchans = MAX_PA_CHANS;}
    if (outchans > MAX_PA_CHANS) {post("output channels reduced to maximum %d", MAX_PA_CHANS); outchans = MAX_PA_CHANS;}
    pdi = NULL;
    if (inchans > 0) {
        pa_indev = pddev2padev(indeviceno,1);
        if(pa_indev >= 0) {
            pdi = Pa_GetDeviceInfo(pa_indev);
            if(pdi->maxInputChannels < inchans) inchans = pdi->maxInputChannels;
        }
    }
    pdo = NULL;
    if (outchans > 0) {
        pa_outdev = pddev2padev(outdeviceno,0);
        if(pa_outdev >= 0) {
            pdo = Pa_GetDeviceInfo(pa_outdev);
            if(pdo->maxOutputChannels < outchans) outchans = pdo->maxOutputChannels;
        }
    }
    if (sys_verbose) {
        post("input device %d, channels %d", pa_indev, inchans);
        post("output device %d, channels %d", pa_outdev, outchans);
        post("latency advance %d", advance);
    }
    if (inchans || outchans) {
        int blocksize;
        PaStreamParameters iparam,oparam;
        /* initialize input */
        iparam.device = pa_indev;
        iparam.channelCount = inchans;
        iparam.sampleFormat = paFloat32 | paNonInterleaved;
        iparam.suggestedLatency = advance * 0.001;
        iparam.hostApiSpecificStreamInfo = NULL;
        /* initialize output */
        oparam.device = pa_outdev;
        oparam.channelCount = outchans;
        oparam.sampleFormat = paFloat32 | paNonInterleaved;
        oparam.suggestedLatency = advance * 0.001;
        oparam.hostApiSpecificStreamInfo = NULL;
        /* set block size */
        blocksize=64;
        while ((float)blocksize/(float)rate*1000*2 < advance && blocksize==1024) blocksize *= 2;
        pa_blocksize = blocksize;
        /* initialize io buffer */
        for (int j=0; j != MAX_PA_CHANS;++j) {
            if (pa_inbuffer[j])  freealignedbytes(pa_inbuffer[j], 0);
            if (pa_outbuffer[j]) freealignedbytes(pa_outbuffer[j], 0);
            pa_inbuffer[j]  = (float *)getalignedbytes((blocksize + sys_dacblocksize)*sizeof(float));
            pa_outbuffer[j] = (float *)getalignedbytes((blocksize + sys_dacblocksize)*sizeof(float));
        }
        pa_bufferpos = 0;
        /* report to portaudio */
        err = Pa_OpenStream(&pa_stream,
            (( pa_indev!=-1) ? &iparam : 0),
            ((pa_outdev!=-1) ? &oparam : 0),
            rate, pa_blocksize, paClipOff, /* tb: we should be faster ;-) */ process /* patestCallback */, NULL);
        if (err == paNoError) {
            const PaStreamInfo *streaminfo = Pa_GetStreamInfo(pa_stream);
            t_atom atoms[4];
            t_symbol *pd = gensym("pd");
            sys_schedadvance = int(1e-6 * streaminfo->outputLatency);

            SETFLOAT(atoms, (float)indeviceno);
            SETFLOAT(atoms+1, (float)inchans);
            SETFLOAT(atoms+2, (float)rate);
            SETFLOAT(atoms+3, (float)streaminfo->inputLatency * 1000.f);
            typedmess(pd->s_thing, gensym("audiocurrentininfo"), 4, atoms);

            SETFLOAT(atoms, (float)outdeviceno);
            SETFLOAT(atoms+1, (float)outchans);
            SETFLOAT(atoms+2, (float)rate);
            SETFLOAT(atoms+3, (float)streaminfo->outputLatency * 1000.f);
            typedmess(pd->s_thing, gensym("audiocurrentoutinfo"), 4, atoms);
        }
    } else err = 0;

    if (err != paNoError) {
        error("Error number %d occured opening portaudio stream: %s", err, Pa_GetErrorText(err));
        sys_inchannels = sys_outchannels = 0;
        pa_indev = pa_outdev = -1;
        pa_inchans = pa_outchans = 0;
        return 1;
    } else if (sys_verbose) post("... opened OK.");
    pa_inchans = inchans;
    pa_outchans = outchans;

    /* we might have adapted the channel count */
    sys_setchsr(inchans, outchans, rate, sys_dacblocksize);
    err = Pa_StartStream(pa_stream);
    if (err!=paNoError) {
        post("Error number %d occured starting portaudio stream: %s", err, Pa_GetErrorText(err));
        sys_inchannels = sys_outchannels = 0;
        return 1;
    }
    if(sys_verbose) post("successfully started");
    return 0;
}

void sys_peakmeters();
extern int sys_meters;          /* true if we're metering */

void run_all_idle_callbacks();
void sys_xrun_notification(); /* in m_sched.c */
void sys_lock_timeout_notification();

int process (const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo,
PaStreamCallbackFlags statusFlags, void *userData) {
    int timeout = int((float)frameCount / (float) sys_dacsr * 1e6);
    if (statusFlags) sys_xrun_notification();
    if (sys_timedlock(timeout) == ETIMEDOUT) /* we're late */ {
        sys_lock_timeout_notification();
        return 0;
    }
    for (int i=0; (unsigned)i < frameCount / sys_dacblocksize; ++i) {
        for (int j=0; j < sys_inchannels; j++) {
            t_sample  *in = ((t_sample**)input)[j] + i * sys_dacblocksize;
            copyvec(sys_soundin + j * sys_dacblocksize, in, sys_dacblocksize);
        }
        sched_tick(sys_time + sys_time_per_dsp_tick);
        for (int j=0; j < sys_outchannels; j++) {
            t_sample *out = ((t_sample**)output)[j] + i * sys_dacblocksize;
            copyvec(out, sys_soundout + j * sys_dacblocksize, sys_dacblocksize);
        }
        if (sys_meters) sys_peakmeters();
        zerovec(sys_soundout, pa_outchans * sys_dacblocksize);
    }
    run_all_idle_callbacks();
    sys_unlock();
    return 0;
}

static void pa_close_audio() {
    if(sys_verbose) post("closing portaudio");
    if (pa_inchans || pa_outchans) {
        if (pa_stream) {
            int status = Pa_StopStream(pa_stream);
            if (status) post("error closing audio: %d", status);
            Pa_CloseStream(pa_stream);
            pa_stream = NULL;
        }
    }
    sys_setscheduler(0);
    if(sys_verbose) post("portaudio closed");
    pa_inchans = pa_outchans = 0;
    pa_indev = pa_outdev = -1;
}

/* for blocked IO */
static int pa_send_dacs() {
    /* we don't support blocking i/o */
    return SENDDACS_NO;
}

/* lifted from pa_devs.c in portaudio */
static void pa_listdevs() {
    PaError err;
    pa_initialize();
    int numDevices = Pa_GetDeviceCount();
    if(numDevices < 0) {
        error("ERROR: Pa_GetDeviceCount returned %d", numDevices);
        err = numDevices;
        goto error;
    }
    post("Audio Devices:");
    for(int i=0; i<numDevices; i++) {
        const PaDeviceInfo *pdi = Pa_GetDeviceInfo(i);
        post ("device %s", pdi->name);
        post("device %d:", i+1);
        post(" %s;", pdi->name);
        post("%d inputs, ", pdi->maxInputChannels);
        post("%d outputs ", pdi->maxOutputChannels);
        if (i == Pa_GetDefaultInputDevice())  post(" (Default Input)");
        if (i == Pa_GetDefaultOutputDevice()) post(" (Default Output)");
        post("");
    }
    post("");
    return;
 error:
    error("Error #%d occurred while using the portaudio stream: %s\n", err, Pa_GetErrorText(err));
}

/* scanning for devices */
static void pa_getdevs(char *indevlist, int *nindevs, char *outdevlist, int *noutdevs, int *canmulti, int maxndev, int devdescsize) {
    int nin = 0, nout = 0, ndev;
    *canmulti = 1;  /* one dev each for input and output */
    pa_initialize();
    ndev = Pa_GetDeviceCount();
    for (int i=0; i<ndev; i++) {
        const PaDeviceInfo *pdi = Pa_GetDeviceInfo(i);
        if (pdi->maxInputChannels > 0 && nin < maxndev) {
            PaHostApiIndex api = pdi->hostApi;
            const PaHostApiInfo *info = Pa_GetHostApiInfo(api);
            const char *apiName = info->name;
            unsigned int apiNameLen = strlen(apiName);
            strcpy(indevlist + nin * devdescsize, apiName);
            indevlist[nin * devdescsize + apiNameLen] = '/';
            strcpy(indevlist + nin * devdescsize + apiNameLen + 1, pdi->name);
            nin++;
        }
        if (pdi->maxOutputChannels > 0 && nout < maxndev) {
            PaHostApiIndex api = pdi->hostApi;
            const PaHostApiInfo *info = Pa_GetHostApiInfo(api);
            const char *apiName = info->name;
            unsigned int apiNameLen = strlen(apiName);
            strcpy(outdevlist + nout * devdescsize, apiName);
            outdevlist[nout * devdescsize + apiNameLen] = '/';
            strcpy(outdevlist + nout * devdescsize + apiNameLen + 1, pdi->name);
            nout++;
        }
    }
    *nindevs = nin;
    *noutdevs = nout;
}

void pa_getaudioininfo(t_float f) {
    int i = pddev2padev((int)f,1);
    const PaDeviceInfo *pdi;
    pa_initialize();
    pdi = Pa_GetDeviceInfo(i);
    if (pdi) {
        t_symbol *selector = gensym("audioininfo");
        t_symbol *pd = gensym("pd");
        t_atom argv[4];
        SETFLOAT(argv, pdi->maxInputChannels);
        SETFLOAT(argv+1, pdi->defaultSampleRate);
        SETFLOAT(argv+2, pdi->defaultLowInputLatency*1000.f);
        SETFLOAT(argv+3, pdi->defaultHighInputLatency*1000.f);
        typedmess(pd->s_thing, selector, 4, argv);
    }
}

void pa_getaudiooutinfo(t_float f) {
    int i = pddev2padev((int)f,0);
    const PaDeviceInfo *pdi;
    pa_initialize();
    pdi = Pa_GetDeviceInfo(i);
    if (pdi) {
        t_symbol *selector = gensym("audiooutinfo");
        t_symbol *pd = gensym("pd");
        t_atom argv[4];
        SETFLOAT(argv, pdi->maxOutputChannels);
        SETFLOAT(argv+1, pdi->defaultSampleRate);
        SETFLOAT(argv+2, pdi->defaultLowOutputLatency*1000.f);
        SETFLOAT(argv+3, pdi->defaultHighOutputLatency*1000.f);
        typedmess(pd->s_thing, selector, 4, argv);
    }
}

void pa_getcurrent_devices() {
    t_symbol *pd = gensym("pd");
    t_symbol *selector = gensym("audiodevice");
    t_atom argv[2];
    SETFLOAT(argv, padev2pddev(pa_indev,1));
    SETFLOAT(argv+1, padev2pddev(pa_outdev,0));
    typedmess(pd->s_thing, selector, 2, argv);
}

void pa_test_setting (int ac, t_atom *av) {
    int indev      = atom_getintarg(0, ac, av);
    int outdev     = atom_getintarg(1, ac, av);
    int samplerate = atom_getintarg(2, ac, av);
    int inchans    = atom_getintarg(3, ac, av);
    int outchans   = atom_getintarg(4, ac, av);
    int advance    = atom_getintarg(5, ac, av);
    t_symbol *pd = gensym("pd");
    t_symbol *selector = gensym("testaudiosettingresult");
    t_atom argv[1];
    pa_initialize();
    indev = pddev2padev(indev,1);
    outdev = pddev2padev(outdev,0);
    if (pa_indev==-1 && pa_outdev==-1) {
        int ret;
        PaStreamParameters iparam, oparam;
        iparam.device = indev;
        iparam.channelCount = inchans;
        iparam.sampleFormat = paFloat32 | paNonInterleaved;
        iparam.suggestedLatency = advance * 0.001;
        iparam.hostApiSpecificStreamInfo = NULL;
        oparam.device = outdev;
        oparam.channelCount = outchans;
        oparam.sampleFormat = paFloat32 | paNonInterleaved;
        oparam.suggestedLatency = advance * 0.001;
        oparam.hostApiSpecificStreamInfo = NULL;
        ret = Pa_IsFormatSupported(&iparam, &oparam, samplerate);
        SETFLOAT(argv, ret == paNoError?1:0);
        typedmess(pd->s_thing, selector, 1, argv);
    }
}

static int pddev2padev(int pddev,int input) {
    pa_initialize();
    for (int j=0, devno=0; j < Pa_GetDeviceCount(); j++) {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(j);
        int maxchans = input?info->maxInputChannels:info->maxOutputChannels;
        if (maxchans > 0) {
            if (devno == pddev) return j;
            devno++;
        }
    }
    return -1;
}

static int padev2pddev(int padev,int input) {
    int count = Pa_GetDeviceCount();
    for (int j=0, devno=0; j < count; j++) {
        const PaDeviceInfo *info = Pa_GetDeviceInfo(j);
        int chans = input?info->maxInputChannels:info->maxOutputChannels;
        if (chans > 0) {
            if(j == padev) return devno;
            devno++;
        }
    }
    return -1; // no found
}

void pa_get_asio_latencies(t_float f) {
    int index = pddev2padev((int)f,0);
    const PaDeviceInfo *pdi = Pa_GetDeviceInfo(index);
    const PaHostApiInfo *phi = Pa_GetHostApiInfo(pdi->hostApi);
    if (phi->type != paASIO) {
        post("device not an asio device");
        return;
    }
#ifdef WIN32
    else {
        long minlat, maxlat, preflat, gran;
        t_atom argv[4];
        t_symbol *selector = gensym("asiolatency");
        t_symbol *pd = gensym("pd");
        PaAsio_GetAvailableLatencyValues(index, &minlat, &maxlat, &preflat, &gran);
        SETFLOAT(argv, (float) minlat);
        SETFLOAT(argv + 1, (float) maxlat);
        SETFLOAT(argv + 2, (float) preflat);
        SETFLOAT(argv + 3, (float) gran);
        typedmess(pd->s_thing, selector, 4, argv);
    }
#endif
}

t_audioapi pa_api = {
    0 /* pa_open_audio */,
    pa_close_audio,
    pa_send_dacs,
    pa_getdevs,
};
