/* Copyright (c) 2001 Miller Puckette and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* this file calls Ross Bencina's and Phil Burk's Portaudio package.  It's
   the main way in for Mac OS and, with Michael Casey's help, also into
   ASIO in Windows. */

/* tb: requires portaudio >= V19 */

#include "m_pd.h"
#include "s_stuff.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <portaudio.h>
#include <errno.h>

#ifdef MSW
/* jmz thinks that we have to include:
 * on Windows: <malloc.h> (probably this is already included?)
 * on linux: <alloca.h> (might be true for osX too, no way to check right now...)
 */
#zinclude <malloc.h>
#else
# include <alloca.h>
#endif

#ifdef MSW
#include "pthread.h" /* for ETIMEDOUT */
#endif

#define MAX_PA_CHANS 32

/* portaudio's blocking api is not working, yet: */
/* #define PABLOCKING */

//#ifndef PABLOCKING
#include "s_audio_pablio.h"
//#endif

static int pa_inchans, pa_outchans;

static PaStream *pa_stream;
static PABLIO_Stream *pablio_stream;
static PaStreamCallback *pa_callback = NULL;

int process (const void *input, void *output, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo,
PaStreamCallbackFlags statusFlags, void *userData);

int pa_open_audio(int inchans, int outchans, int rate, int advance, int indeviceno, int outdeviceno, int schedmode) {
    PaError err;
    int j, devno, pa_indev = -1, pa_outdev = -1;
    pa_callback = schedmode==1 ? process : NULL;
    sys_setscheduler(schedmode);
    /* Initialize PortAudio  */
    err = Pa_Initialize();
    if (err != paNoError) {
	error("Error number %d occured initializing portaudio: %s", err, Pa_GetErrorText(err));
	return 1;
    }
    /* post("in %d out %d rate %d device %d", inchans, outchans, rate, deviceno); */
    if ( inchans > MAX_PA_CHANS) {post( "input channels reduced to maximum %d", MAX_PA_CHANS);  inchans = MAX_PA_CHANS;}
    if (outchans > MAX_PA_CHANS) {post("output channels reduced to maximum %d", MAX_PA_CHANS); outchans = MAX_PA_CHANS;}
    if (inchans > 0) {
        for (j = 0, devno = 0; j < Pa_GetDeviceCount(); j++) {
            const PaDeviceInfo *info = Pa_GetDeviceInfo(j);
	    int maxchans = info->maxInputChannels;
            if (maxchans > 0) {
                if (devno == indeviceno) {
		    if (maxchans < inchans) inchans = maxchans;
                    pa_indev = j;
                    break;
                }
                devno++;
            }
        }
    }
    if (outchans > 0) {
        for (j = 0, devno = 0; j < Pa_GetDeviceCount(); j++) {
            const PaDeviceInfo *info = Pa_GetDeviceInfo(j);
	    int maxchans = info->maxOutputChannels;
            if (maxchans > 0) {
                if (devno == outdeviceno) {
		    if (maxchans < outchans) outchans = maxchans;
                    pa_outdev = j;
                    break;
                }
                devno++;
            }
        }
    }
    if (sys_verbose) {
        post( "input device %d, channels %d", pa_indev,   inchans);
        post("output device %d, channels %d", pa_outdev, outchans);
        post("latency advance %d", advance);
    }
    if (inchans || outchans) {
#ifndef PABLOCKING
        if (schedmode == 1) {
#endif
	    PaStreamParameters inputParameters, outputParameters;
	    /* initialize input */
	    inputParameters.device = pa_indev ;
	    inputParameters.channelCount = inchans;
	    inputParameters.sampleFormat = paFloat32 | paNonInterleaved;
	    inputParameters.suggestedLatency = advance * 0.001;
	    inputParameters.hostApiSpecificStreamInfo = NULL;
	    /* initialize output */
	    outputParameters.device = pa_outdev;
	    outputParameters.channelCount = outchans;
	    outputParameters.sampleFormat = paFloat32 | paNonInterleaved;
	    outputParameters.suggestedLatency = advance * 0.001;
	    outputParameters.hostApiSpecificStreamInfo = NULL;
	    /* report to portaudio */
	    err = Pa_OpenStream(&pa_stream,
		( pa_indev!=-1 ?  &inputParameters : 0),
		(pa_outdev!=-1 ? &outputParameters : 0),
		rate, sys_dacblocksize, paClipOff, /* tb: we should be faster ;-) */ pa_callback, NULL);
	    if (err == paNoError) {
		const PaStreamInfo * streaminfo = Pa_GetStreamInfo (pa_stream);
		sys_schedadvance = 1e6 * streaminfo->outputLatency;
	    }
#ifndef PABLOCKING
	} else {
	    int nbuffers = sys_advance_samples / sys_dacblocksize;
	    err = PD_OpenAudioStream( &pablio_stream, rate, paFloat32,
	        inchans, outchans, sys_dacblocksize, nbuffers, pa_indev, pa_outdev);
	}
#endif
    } else err = 0;
    if (err != paNoError) {
        post("Error number %d occured opening portaudio stream", err);
        post("Error message: %s", Pa_GetErrorText(err));
        Pa_Terminate();
        sys_inchannels = sys_outchannels = 0;
        return 1;
    } else if (sys_verbose) post("... opened OK.");
    pa_inchans = inchans;
    pa_outchans = outchans;
#ifndef PABLOCKING
    if (schedmode)
#endif
    err = Pa_StartStream(pa_stream);
    if (err != paNoError) {
        post("Error number %d occured starting portaudio stream", err);
        post("Error message: %s", Pa_GetErrorText(err));
        Pa_Terminate();
        sys_inchannels = sys_outchannels = 0;
        return 1;
    }
    post("successfully started");
    return 0;
}

void sys_peakmeters(void);
extern int sys_meters;          /* true if we're metering */

int process (const void *input, void *output, unsigned long frameCount, 
const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void *userData) {
    int i,j;
    int timeout = (float)frameCount / (float) sys_dacsr * 1e6;
    if (sys_timedlock(timeout) == ETIMEDOUT) /* we're late */ {
	post("timeout %d", timeout);
	sys_log_error(ERR_SYSLOCK);
	return 0;
    }
    for (j = 0; j < sys_inchannels; j++) {
	t_sample * in = ((t_sample**)input)[j];
	copyvec(sys_soundin + j * sys_dacblocksize,in,sys_dacblocksize);
    }
    sched_tick(sys_time + sys_time_per_dsp_tick);
    for (j = 0; j < sys_outchannels;  j++) {
	t_sample * out = ((t_sample**)output)[j];
	copyvec(out,sys_soundout + j * sys_dacblocksize,sys_dacblocksize);
    }
    /* update peak meters */
    if (sys_meters) sys_peakmeters();
    /* clear the output buffer */
    zerovec(sys_soundout, pa_outchans * sys_dacblocksize);
    sys_unlock();
    return 0;
}

void pa_close_audio() {
    post("closing portaudio");
    if (pa_inchans || pa_outchans) {
	if (pa_stream) {
	    Pa_StopStream(pa_stream);
	    Pa_CloseStream(pa_stream);
	    pa_stream = NULL;
	}
	if (pablio_stream) {
	    PD_CloseAudioStream(pablio_stream);
	    pablio_stream = NULL;
	}
    }
    post("portaudio closed");
    pa_inchans = pa_outchans = 0;
}

/* for blocked IO */
int pa_send_dacs() {
#ifdef PABLOCKING /* tb: blocking IO isn't really working for v19 yet */
    double timenow, timebefore;
    if (( pa_inchans &&  Pa_GetStreamReadAvailable(pa_stream) < sys_dacblocksize*0.8) &&
	(pa_outchans && Pa_GetStreamWriteAvailable(pa_stream) < sys_dacblocksize*0.8)) {
	/* we can't transfer data ... wait in the scheduler */
	return SENDDACS_NO;
    }
    timebefore = sys_getrealtime();
    if (pa_outchans) Pa_WriteStream(pa_stream, &sys_soundout, sys_dacblocksize);
    if ( pa_inchans)  Pa_ReadStream(pa_stream, &sys_soundin,  sys_dacblocksize);
    zerovec(sys_soundout, pa_inchans * sys_dacblocksize);
    while (( pa_inchans &&  Pa_GetStreamReadAvailable(pa_stream) < sys_dacblocksize*0.8) &&
	   (pa_outchans && Pa_GetStreamWriteAvailable(pa_stream) < sys_dacblocksize*0.8)) {
	if (pa_outchans) Pa_WriteStream(pa_stream, &sys_soundout, sys_dacblocksize);
	if ( pa_inchans)  Pa_ReadStream(pa_stream, &sys_soundin,  sys_dacblocksize);
	zerovec(sys_soundout, pa_inchans * sys_dacblocksize);
	sched_tick(sys_time + sys_time_per_dsp_tick);
    }
    if (sys_getrealtime() > timebefore + sys_sleepgrain * 1e-6) {
	return SENDDACS_SLEPT;
    } else return SENDDACS_YES;
#else /* for now we're using pablio */
    float *samples, *fp1, *fp2;
    int i, j;
    double timebefore;
    samples=(float*)alloca(sizeof(float) * MAX_PA_CHANS * sys_dacblocksize);
    timebefore = sys_getrealtime();
    if ((pa_inchans && PD_GetAudioStreamReadable(pablio_stream) < sys_dacblocksize) ||
        (pa_outchans && PD_GetAudioStreamWriteable(pablio_stream) < sys_dacblocksize)) {
        if (pa_inchans && pa_outchans) {
            int synced = 0;
            while (PD_GetAudioStreamWriteable(pablio_stream) > 2*sys_dacblocksize) {
                for (j = 0; j < pa_outchans; j++)
                    for (i = 0, fp2 = samples + j; i < sys_dacblocksize; i++, fp2 += pa_outchans)
                        *fp2 = 0;
                synced = 1;
                PD_WriteAudioStream(pablio_stream, samples, sys_dacblocksize);
            }
            while (PD_GetAudioStreamReadable(pablio_stream) > 2*sys_dacblocksize) {
                synced = 1;
                PD_ReadAudioStream(pablio_stream, samples, sys_dacblocksize);
            }
/*          if (synced) post("sync"); */
        }
        return SENDDACS_NO;
    }
    if (pa_inchans) {
        PD_ReadAudioStream(pablio_stream, samples, sys_dacblocksize);
        for (j = 0, fp1 = sys_soundin; j < pa_inchans; j++, fp1 += sys_dacblocksize)
            for (i = 0, fp2 = samples + j; i < sys_dacblocksize; i++, fp2 += pa_inchans)
                fp1[i] = *fp2;
    }
    if (pa_outchans) {
        for (j = 0, fp1 = sys_soundout; j < pa_outchans; j++, fp1 += sys_dacblocksize)
            for (i = 0, fp2 = samples + j; i < sys_dacblocksize; i++, fp2 += pa_outchans) {
                *fp2 = fp1[i];
                fp1[i] = 0;
            }
        PD_WriteAudioStream(pablio_stream, samples, sys_dacblocksize);
    }
    if (sys_getrealtime() > timebefore + sys_sleepgrain * 1e-6) {
        /* post("slept"); */
        return SENDDACS_SLEPT;
    } else return SENDDACS_YES;
#endif
}

void pa_listdevs() /* lifted from pa_devs.c in portaudio */ {
    int j, numDevices;
    const PaDeviceInfo *pdi;
    PaError err;
    Pa_Initialize();
    numDevices = Pa_GetDeviceCount();
    if (numDevices<0) {
        error("ERROR: Pa_GetDeviceCount returned %d", numDevices);
        err = numDevices;
        goto error;
    }
    post("Audio Devices:");
    for (int i=0; i<numDevices; i++) {
	const PaDeviceInfo *pdi = Pa_GetDeviceInfo(i);
        post("device %s", pdi->name);
	post("device %d:", i+1);
        post(" %s;", pdi->name);
        post("%d inputs, ", pdi->maxInputChannels);
        post("%d outputs", pdi->maxOutputChannels);
#ifdef PA19
        if (i == Pa_GetDefaultInputDevice ())   post(" (Default Input)");
        if (i == Pa_GetDefaultOutputDevice())   post(" (Default Output)");
#else
        if (i == Pa_GetDefaultInputDeviceID ()) post(" (Default Input)");
        if (i == Pa_GetDefaultOutputDeviceID()) post(" (Default Output)");
#endif
        post("");
    }
    post("");
    return;
error:
    error("An error occured while using the portaudio stream: #%d: %s",err,Pa_GetErrorText(err));
}
/* scanning for devices */
void pa_getdevs(char *indevlist, int *nindevs, char *outdevlist, int *noutdevs, int *canmulti, int maxndev, int devdescsize) {
    int i, nin = 0, nout = 0, ndev;
    *canmulti = 1;  /* one dev each for input and output */
    Pa_Initialize();
    ndev = Pa_GetDeviceCount();
    for (i = 0; i < ndev; i++) {
        const PaDeviceInfo *pdi = Pa_GetDeviceInfo(i);
        if (pdi->maxInputChannels > 0 && nin < maxndev) {
            sprintf(indevlist + nin * devdescsize, "(%d)%s", pdi->hostApi,pdi->name);
            /* strcpy(indevlist + nin * devdescsize, pdi->name); */
            nin++;
        }
        if (pdi->maxOutputChannels > 0 && nout < maxndev) {
            sprintf(outdevlist + nout * devdescsize, "(%d)%s", pdi->hostApi,pdi->name);
            /* strcpy(outdevlist + nout * devdescsize, pdi->name); */
            nout++;
        }
    }
    *nindevs = nin;
    *noutdevs = nout;
}

t_audioapi pa_api = {
    pa_open_audio,
    pa_close_audio,
    pa_send_dacs,
    pa_getdevs,
};
