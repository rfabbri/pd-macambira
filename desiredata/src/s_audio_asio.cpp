/* Copyright (c) 2004, Tim Blechmann and others
 * supported by vibrez.net
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt" in this distribution.  */

/* native ASIO interface for windows
 * adapted from hostsample.cpp (ASIO SDK)
 */

#ifdef MSW
#include "windows.h" /* for application window handle */
#define IEEE754_64FLOAT 1
#else
#error This is for MS Windows (Intel CPU architecture) only!!
#endif

#ifdef _MSC_VER
#pragma warning( disable : 4091 )
#endif

#include "m_pd.h"
extern "C" {
#include "s_stuff.h"
#include "m_simd.h"
}

#include "asio.h"     /* steinberg's header file */
#include "asiodrivers.h" /* ASIODrivers class */
#include "asiosys.h"
#include "pthread.h"
#include "stdio.h" /* for sprintf */

#include <time.h>
#include <sys/timeb.h>

#include "assert.h"
#define ASSERT assert


/* fast float to integer conversion adapted from Erik de Castro Lopo */
#define	_ISOC9X_SOURCE	1
#define _ISOC99_SOURCE	1
#define	__USE_ISOC9X	1
#define	__USE_ISOC99	1
#include "math.h"

// seconds to wait for driver to respond
#define DRIVERWAIT 1

#define ASIODEBUG

/* public function prototypes */
// extern "C" void asio_open_audio(int naudioindev, int *audioindev, int nchindev,
// int *chindev, int naudiooutdev, int *audiooutdev, int nchoutdev, int *choutdev, int srate, int scheduler);
extern "C" void asio_close_audio();
extern "C" void asio_getdevs(char *indevlist, int *nindevs, char *outdevlist, int *noutdevs, int *canmulti, int maxndev, int devdescsize);
extern "C" int asio_send_dacs();

/* asio callback prototypes for traditional scheduling*/
static void asio_bufferSwitch(long db_idx, ASIOBool directprocess);
static void asio_sampleRateDidChange(ASIOSampleRate srate);
static long asio_messages(long selector, long value, void* message, double* opt);
static ASIOTime *asio_bufferSwitchTimeInfo(ASIOTime *params, long db_idx, ASIOBool directprocess);

/* asio callback prototypes for callback-based scheduling*/
static void asio_bufferSwitch_cb(long db_idx, ASIOBool directprocess);
static void asio_sampleRateDidChange_cb(ASIOSampleRate srate);
static long asio_messages_cb(long selector, long value, void* message, double* opt);
static ASIOTime *asio_bufferSwitchTimeInfo_cb(ASIOTime *params, long db_idx, ASIOBool directprocess);

static void float32tofloat32(void* inbuffer, void* outbuffer, long frames);
static void float32tofloat32_S(void* inbuffer, void* outbuffer, long frames);
static void float32tofloat64(void* inbuffer, void* outbuffer, long frames);
static void float64tofloat32(void* inbuffer, void* outbuffer, long frames);
static void float32toInt16(void* inbuffer, void* outbuffer, long frames);
static void Int16tofloat32(void* inbuffer, void* outbuffer, long frames);
static void float32toInt24(void* inbuffer, void* outbuffer, long frames);
static void Int24tofloat32(void* inbuffer, void* outbuffer, long frames);
static void float32toInt32(void* inbuffer, void* outbuffer, long frames);
static void Int32tofloat32(void* inbuffer, void* outbuffer, long frames);
static void float32toInt16_S(void* inbuffer, void* outbuffer, long frames);
static void Int16tofloat32_S(void* inbuffer, void* outbuffer, long frames);
static void float32toInt24_S(void* inbuffer, void* outbuffer, long frames);
static void Int24tofloat32_S(void* inbuffer, void* outbuffer, long frames);
static void float32toInt32_S(void* inbuffer, void* outbuffer, long frames);
static void Int32tofloat32_S(void* inbuffer, void* outbuffer, long frames);
void asio_close_audio(void);

typedef void converter_t(void* inbuffer, void* outbuffer, long frames);

/* sample converting helper functions:
 * - global send / receive functions
 * - sample conversion functions (adapted from ASIOConvertSamples.cpp */
static converter_t *asio_converter_send (ASIOSampleType format);
static converter_t *asio_converter_receive (ASIOSampleType format);

/* pointers to the converter functions of each channel are stored here */
static converter_t **asio_converter = NULL;

/* function to get sample width of data according to ASIOSampleType */
static int asio_get_samplewidth(ASIOSampleType format);

/* that's the sample width in bytes (per output channel) -
 * it's only for silence when stopping the driver.... (please find a better solution) */
static int *asio_samplewidth = NULL;


/* some local helper functions */
static void prepare_asio_drivernames();

/* system dependent helper functions */
static unsigned long get_sys_reference_time();

/* global storage */
static ASIODriverInfo * asio_driver = NULL;
static ASIOBufferInfo * asio_bufferinfo = NULL;
static ASIOChannelInfo* asio_channelinfo = NULL;
static AsioTimeInfo   * asio_timerinfo = NULL;
static ASIOCallbacks    asio_callbacks;
extern AsioDrivers *    asioDrivers; /* declared in asiodrivers.cpp */

static char ** asio_drivernames = NULL;

static ASIOSampleRate asio_srate;
static long asio_inchannels;
static long asio_outchannels;

static long asio_minbufsize;
static long asio_maxbufsize;
static long asio_prefbufsize;
static long asio_granularity;
static unsigned char asio_useoutputready;
static long asio_inputlatency;
static long asio_outputlatency;

static long asio_bufsize;                /* hardware buffer size */
static long asio_ticks_per_callback;

unsigned long sys_reftime;

/* ringbuffer stuff */
static t_sample ** asio_ringbuffer = NULL;                   /* ringbuffers */
static int asio_ringbuffer_inoffset;     /* ringbuffer(in) pointer offset for dac */
static int asio_ringbuffer_outoffset;    /* ringbuffer(out) pointer offset */
static int asio_ringbuffer_length;       /* latency - hardware latency in samples*/

/* i hope we can remove this to use callback based dsp scheduling */
static pthread_mutex_t asio_ringbuf_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t asio_ringbuf_cond = PTHREAD_COND_INITIALIZER;

/* some global definitions: */
#define ASIOVERSION 2          /* i hope we are compatible with asio 2 */

/* definitions from s_audio.c ... it should be save to use them */
#define DEVDESCSIZE   80
#define MAXNDEV   20

/* from m_sched.c: */
extern "C" double sys_time_per_dsp_tick;
extern "C" double sys_time;


/**************************************************************************/
/* some function pointers for eventual fast copying when SIMD is possible */

static void (*copyblock)(t_sample *dst,t_sample *src,int n);
static void (*zeroblock)(t_sample *dst,int n);
static t_int *(*clipblock)(t_int *w);

static void copyvec_nrm(t_sample *dst,t_sample *src,int n) { memcpy(dst,src,n*sizeof(t_sample)); }
static void zerovec_nrm(t_sample *dst,int n) { memset(dst,0,n*sizeof(t_sample)); }

/*************************************************************************/


/* open asio interface */
/* todo: some more error messages */
void asio_open_audio(int naudioindev, int *audioindev, int nchindev,
int *chindev, int naudiooutdev, int *audiooutdev, int nchoutdev, int *choutdev, int srate, int scheduler) {
	ASIOError status = ASE_OK;
	ASIOBufferInfo * buffers = NULL;
	int i;
	int channels;

#ifdef IEEE754_64FLOAT
	asio_srate=(ASIOSampleRate)srate;
#else
	sprintf(asio_srate,"%8d",srate);
#endif
	/* check, if the driver is still running */
	if(asio_driver) asio_close_audio();
	/* check, if we use the first asio device */
	prepare_asio_drivernames();
	asioDrivers->getDriverNames(asio_drivernames,MAXNDEV);

    try {
	    asioDrivers->loadDriver(asio_drivernames[*audioindev]);
    }
    catch(...) {
        error("ASIO: Error loading driver");
        goto bailout;
    }
	/* initialize ASIO */
	asio_driver = (ASIODriverInfo*) getbytes (sizeof(ASIODriverInfo));
	asio_driver->asioVersion = ASIOVERSION;
	asio_driver->sysRef = GetDesktopWindow();
	status = ASIOInit(asio_driver);
#ifdef ASIODEBUG
	post("sysRef: %x", asio_driver->sysRef);
	post("asioversion: %d", asio_driver->asioVersion);
	post("driverversion: %d", asio_driver->driverVersion);
	post("name: %s", asio_driver->name);
#endif

	switch (status) {
		if(status) post("error: %s", asio_driver->errorMessage);
	case ASE_NotPresent:    error("ASIO: ASE_NotPresent");    goto bailout;
 	case ASE_NoMemory:      error("ASIO: ASE_NoMemory");      goto bailout;
 	case ASE_HWMalfunction: error("ASIO: ASE_HWMalfunction"); goto bailout;
	}
#ifdef ASIODEBUG
	post("ASIO initialized successfully");
#endif


	/* query driver */
	status = ASIOGetChannels(&asio_inchannels, &asio_outchannels);
    if(status != ASE_OK) {
        error("ASIO: Couldn't get channel count");
        goto bailout;
    }

#ifdef ASIODEBUG
	post ("ASIOGetChannels\tinputs: %d, outputs: %d", asio_inchannels,
		  asio_outchannels);
#endif

	sys_inchannels = *chindev <= asio_inchannels ? *chindev : asio_inchannels;
	sys_outchannels = *choutdev <= asio_outchannels ? *choutdev : asio_outchannels;
	channels = sys_inchannels + sys_outchannels;
	status = ASIOGetBufferSize(&asio_minbufsize, &asio_maxbufsize, &asio_prefbufsize, &asio_granularity);
    if(status != ASE_OK) {
        error("ASIO: Couldn't get buffer size");
        goto bailout;
    }
#ifdef ASIODEBUG
	post ("ASIOGetBufferSize\tmin: %d, max: %d, preferred: %d, granularity: "
		  "%d", asio_minbufsize, asio_maxbufsize, asio_prefbufsize,
		  asio_granularity);
#endif

	/* todo: buffer size hardcoded to asio hardware */
	asio_bufsize = asio_prefbufsize;
	if (scheduler) {
		if ( asio_bufsize % sys_dacblocksize == 0 ) {
			/* use callback scheduler */
			sys_setscheduler(1);
			asio_ticks_per_callback = asio_bufsize / sys_dacblocksize;
			post("ASIO: using callback-based scheduler");
		}
	} else post("ASIO: using traditional scheduler");
	/* try to set sample rate */
	if(ASIOCanSampleRate( asio_srate ) != ASE_OK) {
		error ("Samplerate not supported, using default");
#ifdef IEEE754_64FLOAT
		asio_srate = (ASIOSampleRate)44100.0;
#else
		sprintf(&asio_srate,"%8d",44100);
#endif
		srate=44100;
	}

    status = ASIOSetSampleRate( asio_srate );
    if(status != ASE_OK)
#ifdef IEEE754_64FLOAT
	    post("Setting ASIO sample rate to %lg failed... is the device in slave sync mode?", (double)asio_srate);
#else
	    post("Setting ASIO sample rate to %s failed... is the device in slave sync mode?", asio_srate);
#endif

	if(ASIOOutputReady() == ASE_OK)
		asio_useoutputready = 1;
	else
		asio_useoutputready = 0;


	/* set callbacks */
	if(sys_callbackscheduler) {
		asio_callbacks.bufferSwitch = &asio_bufferSwitch_cb;
		asio_callbacks.sampleRateDidChange = &asio_sampleRateDidChange_cb;
		asio_callbacks.asioMessage = &asio_messages_cb;
		asio_callbacks.bufferSwitchTimeInfo = &asio_bufferSwitchTimeInfo_cb;
	} else {
		asio_callbacks.bufferSwitch = &asio_bufferSwitch;
		asio_callbacks.sampleRateDidChange = &asio_sampleRateDidChange;
		asio_callbacks.asioMessage = &asio_messages;
		asio_callbacks.bufferSwitchTimeInfo = &asio_bufferSwitchTimeInfo;
	}
	/* prepare, create and set up buffers */
	asio_bufferinfo  = (ASIOBufferInfo*) getbytes (channels*sizeof(ASIOBufferInfo));
	asio_channelinfo = (ASIOChannelInfo*) getbytes(channels*sizeof(ASIOChannelInfo));
	if (!(asio_bufferinfo && asio_channelinfo)) {
		error("ASIO: couldn't allocate buffer or channel info");
        goto bailout;
	}

	for (i = 0; i != sys_inchannels + sys_outchannels; ++i) {
		if (i < sys_outchannels) {
			asio_bufferinfo[i].isInput = ASIOFalse;
			asio_bufferinfo[i].channelNum = i;
			asio_bufferinfo[i].buffers[0] = asio_bufferinfo[i].buffers[1] = 0;
		} else {
			asio_bufferinfo[i].isInput = ASIOTrue;
			asio_bufferinfo[i].channelNum = i - sys_outchannels;
			asio_bufferinfo[i].buffers[0] = asio_bufferinfo[i].buffers[1] = 0;
		}
	}
	status = ASIOCreateBuffers(asio_bufferinfo, sys_inchannels + sys_outchannels, asio_bufsize, &asio_callbacks);
	if(status != ASE_OK) {
		error("ASIO: couldn't allocate buffers");
		goto bailout;
	}
#ifdef ASIODEBUG
	post("ASIO: buffers allocated");
#endif

	asio_converter = (converter_t **)getbytes(channels * sizeof (converter_t *));
	asio_samplewidth = (int *)getbytes((sys_outchannels + sys_inchannels) * sizeof (int));
	for (i = 0; i != sys_outchannels + sys_inchannels; ++i) {
		asio_channelinfo[i].channel = asio_bufferinfo[i].channelNum;
		asio_channelinfo[i].isInput = asio_bufferinfo[i].isInput;
		ASIOGetChannelInfo(&asio_channelinfo[i]);

#ifdef ASIODEBUG
		post("ASIO: channel %d type %d", i, asio_channelinfo[i].type);
#endif
		asio_samplewidth[i] = asio_get_samplewidth(asio_channelinfo[i].type);
        if (i < sys_outchannels) asio_converter[i] = asio_converter_send(   asio_channelinfo[i].type);
        else                     asio_converter[i] = asio_converter_receive(asio_channelinfo[i].type);
	}

	/* get latencies */
	ASIOGetLatencies(&asio_inputlatency, &asio_outputlatency);
#ifdef ASIODEBUG
	post("ASIO: input latency: %d, output latency: %d",asio_inputlatency, asio_outputlatency);
#endif


	/* we need a ringbuffer if we use the traditional scheduler */
	if (!sys_callbackscheduler) {
		/* a strange way to find the least common multiple, but works, since sys_dacblocksize (expt 2 x) */
		asio_ringbuffer_length = asio_bufsize * sys_dacblocksize;
		while ( !(asio_ringbuffer_length % sys_dacblocksize) && !(asio_ringbuffer_length % asio_bufsize)) {
			asio_ringbuffer_length /= 2;
		}
		asio_ringbuffer_length *= 2;
#ifdef ASIODEBUG
		post("ASIO: ringbuffer size: %d",asio_ringbuffer_length);
#endif
		/* allocate ringbuffer */
		asio_ringbuffer = (t_sample**) getbytes (channels * sizeof (t_sample*));
		for (i = 0; i != channels; ++i) {
			asio_ringbuffer[i] = (t_sample*)getalignedbytes(asio_ringbuffer_length * sizeof (t_sample));
			if (!asio_ringbuffer[i])
				error("ASIO: couldn't allocate ASIO ringbuffer");
			memset(asio_ringbuffer[i], 0, asio_ringbuffer_length * sizeof (t_sample));
		}
		/* initialize ringbuffer pointers */
		asio_ringbuffer_inoffset = asio_ringbuffer_outoffset = 0;
	}
    if(ASIOStart() != ASE_OK) goto bailout;
    /* set block copy/zero/clip functions */
    if(SIMD_CHKCNT(sys_dacblocksize) && simd_runtime_check()) {
        // urgh... ugly cast
        copyblock = (void (*)(t_sample *,t_sample *,int))&copyvec_simd;
        zeroblock = &zerovec_simd;
        clipblock = &clip_perf_simd;
    } else {
        copyblock = &copyvec_nrm;
        zeroblock = &zerovec_nrm;
        clipblock = &clip_perform;
    }

	post("ASIO: started");
    return;

bailout:
	if(status) post("error: %s", asio_driver->errorMessage);
    post("ASIO: couldn't start");
    asio_close_audio();
    return;
}



/* stop asio, free buffers and close asio interface */
void asio_close_audio() {
	if (asio_driver) {
    	pthread_cond_broadcast(&asio_ringbuf_cond);

	    ASIOError status;
	    int channels = sys_inchannels + sys_outchannels;
	    int i;

        if(asio_useoutputready) {
            // the DMA buffers would be played past ASIOStop
            // -> clear output buffers and notify driver
#if 0
            if(asio_ringbuffer) {
                // slow, blocking method
	            for(i = 0; i != sys_outchannels; ++i)
		            zerovec_simd(asio_ringbuffer[i], asio_ringbuffer_length);
                // wait for bufferswitch to process silence (twice)
	            pthread_cond_wait(&asio_ringbuf_cond, &asio_ringbuf_mutex);
	            for(i = 0; i != sys_outchannels; ++i) memset(asio_ringbuffer[i], 0, asio_ringbuffer_length * sizeof (t_sample));
	            pthread_cond_wait(&asio_ringbuf_cond, &asio_ringbuf_mutex);
            }
#else
            // direct method - clear both hardware buffers
            if(asio_bufferinfo && asio_samplewidth) {
                for(i = 0; i < sys_outchannels; ++i) {
                    long bytes = asio_bufsize*asio_samplewidth[i];
	                memset(asio_bufferinfo[i].buffers[0],0,bytes);
	                memset(asio_bufferinfo[i].buffers[1],0,bytes);
                }
            }
            // notify driver
		    status = ASIOOutputReady();
#endif
        }

        status = ASIOStop();
        if(status == ASE_OK) post("ASIO: stopped");
        status = ASIODisposeBuffers();
        try {
            // ASIOExit can crash if driver not really running
            status = ASIOExit();
        } catch(...) {}
        // deallocate all memory
	if(asio_ringbuffer) {
    		for(i = 0; i < channels; i++)
	    		if(asio_ringbuffer[i]) freealignedbytes(asio_ringbuffer[i],asio_ringbuffer_length * sizeof (t_sample));
            freebytes(asio_ringbuffer, channels * sizeof (t_sample *));
    		asio_ringbuffer = NULL;
        }

	if(asio_bufferinfo) {
            freebytes(asio_bufferinfo, channels * sizeof (ASIOBufferInfo));
            asio_bufferinfo = NULL;
        }
	if(asio_channelinfo) {
            freebytes(asio_channelinfo, channels * sizeof (ASIOChannelInfo));
            asio_channelinfo = NULL;
        }
	if(asio_converter) {
	    freebytes(asio_converter, channels * sizeof (converter_t *));
            asio_converter = NULL;
        }
	if(asio_samplewidth) {
		freebytes(asio_samplewidth, (sys_outchannels + sys_inchannels) * sizeof (int));
		asio_samplewidth = NULL;
        }
	freebytes(asio_driver, sizeof (ASIODriverInfo));
	asio_driver = NULL;
	/* leave the scheduler and return to traditional mode */
	if (sys_callbackscheduler) sys_setscheduler(0);
    }
}

void asio_getdevs(char *indevlist, int *nindevs, char *outdevlist, int *noutdevs, int *canmulti, int maxndev, int devdescsize) {
	prepare_asio_drivernames();
	*canmulti = 0; /* we will only support one asio device */
	*nindevs = *noutdevs = (int)asioDrivers->getDriverNames(asio_drivernames, maxndev);
	for(int i = 0; i!= *nindevs; ++i) {
		sprintf(indevlist  + i * devdescsize, "%s", asio_drivernames[i]);
		sprintf(outdevlist + i * devdescsize, "%s", asio_drivernames[i]);
	}
}

/* called on every dac~ send */
int asio_send_dacs() {
	t_sample * sp; /* sample pointer */
	int i, j;
	double timenow;
	double timeref = sys_getrealtime();
#ifdef ASIODEBUG
	if (!asio_driver) {
        static int written = 0;
		if(written%100 == 0) error("ASIO not running");
        written++;
		return SENDDACS_NO;
	}

#endif

    /* send sound to ringbuffer */
    sp = sys_soundout;
    for (i = 0; i < sys_outchannels; i++) {
	t_float lo = -1.f;
	t_float hi = 1.f;
	t_int clipargs[6];
	clipargs[1] = (t_int)sp;
	clipargs[2] = (t_int)(asio_ringbuffer[i] + asio_ringbuffer_inoffset);
	clipargs[3] = (t_int)&lo;
	clipargs[4] = (t_int)&hi;
	clipargs[5] = (t_int)sys_dacblocksize;
	clipblock(clipargs);
	zeroblock(sp,sys_dacblocksize);
	sp+=sys_dacblocksize;
    }
    /* get sound from ringbuffer */
    sp = sys_soundin;
    for (j = 0; j < sys_inchannels; j++) {
#if 0
	/* we should be able to read from the ringbuffer on a different position
	 * to reduce latency for asio buffer sizes that aren't multiples of 64... */
	int offset = asio_bufsize + sys_dacblocksize;
	offset += sys_dacblocksize - offset % sys_dacblocksize;
	if (asio_ringbuffer_inoffset < offset) {
 		memcpy(sp, asio_ringbuffer[i+j] + asio_ringbuffer_length +
 		   asio_ringbuffer_inoffset - offset, 64 *sizeof(t_sample));
	} else memcpy(sp, asio_ringbuffer[i+j] + asio_ringbuffer_inoffset - offset, 64*sizeof(t_sample));
#else
        /* working but higher latency */
	copyblock(sp, asio_ringbuffer[i+j] + asio_ringbuffer_inoffset,sys_dacblocksize);
#endif
	sp+=sys_dacblocksize;
    }
    asio_ringbuffer_inoffset += sys_dacblocksize;
#if 1
    // blocking method
    if (asio_ringbuffer_inoffset >= asio_ringbuffer_outoffset + asio_bufsize) {
        struct timespec tm;
	    _timeb tmb;
	    _ftime(&tmb);
	    tm.tv_nsec = tmb.millitm*1000000;
	    tm.tv_sec = tmb.time+DRIVERWAIT; // delay
        if(pthread_cond_timedwait(&asio_ringbuf_cond, &asio_ringbuf_mutex, &tm) == ETIMEDOUT) {
            error("ASIO: ASIO driver non-responsive! - closing");
            asio_close_audio();
	    return SENDDACS_SLEPT;
        }
	if (asio_ringbuffer_inoffset == asio_ringbuffer_length) {
		asio_ringbuffer_outoffset = 0;
		asio_ringbuffer_inoffset = 0;
	} else asio_ringbuffer_outoffset += asio_bufsize;
    }
    if ((timenow = sys_getrealtime()) - timeref > 0.002) {
	return SENDDACS_SLEPT;
    }
#else
    // non-blocking... let PD wait -> doesn't work!
    if (asio_ringbuffer_inoffset >= asio_ringbuffer_outoffset + asio_bufsize) return SENDDACS_NO;
    if (asio_ringbuffer_inoffset == asio_ringbuffer_length) {
	asio_ringbuffer_outoffset = 0;
	asio_ringbuffer_inoffset = 0;
    } else asio_ringbuffer_outoffset += asio_bufsize;
#endif
    return SENDDACS_YES;
}

/* buffer switch callback */
static void asio_bufferSwitch(long db_idx, ASIOBool directprocess) {
	ASIOTime time;
#ifdef ASIODEBUG
	static int written = 0;
	if(written  == 0) {
		post("ASIO: asio_bufferSwitch_cb");
		written = 1;
	}
#endif
	memset (&time, 0, sizeof (time));
	/* todo: do we need to syncronize with other media ??? */
	asio_bufferSwitchTimeInfo(&time, db_idx, directprocess);
}

/* sample rate change callback */
static void asio_sampleRateDidChange(ASIOSampleRate srate) {
	asio_srate = srate;
#ifdef ASIODEBUG
	post("sample rate changed");
#endif
}

/* asio messaging callback */
static long asio_messages(long selector, long value, void* message, double* opt) {
	switch (selector) {
	case kAsioSelectorSupported:
                if (value == kAsioResetRequest || value == kAsioSupportsTimeInfo) return 1L;
		return 0L;
	case kAsioEngineVersion:
		return ASIOVERSION;
	case kAsioResetRequest:
		/* how to handle this without changing the dsp scheduler? */
        post("ASIO: Reset request");
		return 1L;
	case kAsioBufferSizeChange:
		/* todo */
        post("ASIO: Buffer size changed");
		sys_reopen_audio();
		return 1L;
	case kAsioResyncRequest:
        post("ASIO: Resync request");
		return 0L;
	case kAsioLatenciesChanged:
		/* we are not handling the latencies atm */
		return 0L;
	case kAsioSupportsTimeInfo:
		return 1L;
	case kAsioSupportsTimeCode:
		/* we don't support that atm */
		return 0L;
	}
	return 0L;
}

static ASIOTime *asio_bufferSwitchTimeInfo(ASIOTime *params, long db_idx, ASIOBool directprocess) {
	/* todo: i'm not sure if we'll have to synchronize with other media ... probably yes ... */
	/* sys_reftime = get_sys_reference_time(); */
	/* perform the processing */
	int timeout = sys_dacblocksize * (float)asio_ticks_per_callback / (float) sys_dacsr * 1e6;
	if (sys_timedlock(timeout) == ETIMEDOUT) /* we're late */ {
		post("timeout %d", timeout);
		sys_log_error(ERR_SYSLOCK);
		return 0;
	}
	for (long i = 0; i < sys_outchannels + sys_inchannels; i++) {
        if(asio_converter[i])
			if (asio_bufferinfo[i].isInput != ASIOTrue) {
				asio_converter[i](asio_ringbuffer[i]+asio_ringbuffer_outoffset,
				  asio_bufferinfo[i].buffers[db_idx], asio_bufsize);
			}
			else /* these are the input channels */ {
				asio_converter[i](asio_bufferinfo[i].buffers[db_idx],
				  asio_ringbuffer[i]+asio_ringbuffer_outoffset, asio_bufsize);
			}
	}
	pthread_cond_broadcast(&asio_ringbuf_cond);
	sys_unlock();
	if(asio_useoutputready) ASIOOutputReady();
        return 0L; /* time info!!! */
}

/* get system reference time on both platforms */
static unsigned long get_sys_reference_time() {
#if WINDOWS
	return timeGetTime();
#elif MAC
	static const double twoRaisedTo32 = 4294967296.;
	UnsignedWide ys;
	Microseconds(&ys);
	double r = ((double)ys.hi * twoRaisedTo32 + (double)ys.lo);
	return (unsigned long)(r / 1000.);
#endif
}

/* sample converting helper functions */
static converter_t *asio_converter_send(ASIOSampleType format) {
#ifdef ASIODEBUG
 	/* post("ASIO: Sample Type %d", format); */
#endif
	switch (format) {
	case ASIOSTInt16LSB:  return float32toInt16;
	case ASIOSTInt24LSB:  return float32toInt24; // used for 20 bits as well
	case ASIOSTInt32LSB:  return float32toInt32;
	case ASIOSTInt16MSB:  return float32toInt16_S;
	case ASIOSTInt24MSB:  return float32toInt24_S; // used for 20 bits as well
	case ASIOSTInt32MSB:  return float32toInt32_S;
	case ASIOSTFloat32LSB:return float32tofloat32; // IEEE 754 32 bit float, as found on Intel x86 architecture
	case ASIOSTFloat32MSB:return float32tofloat32_S;
	case ASIOSTFloat64LSB:return float32tofloat64; // IEEE 754 64 bit double float, as found on Intel x86 architecture
	case ASIOSTFloat64MSB:
	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can more easily used with these
	case ASIOSTInt32LSB16: // 32 bit data with 16 bit alignment
	case ASIOSTInt32LSB18: // 32 bit data with 18 bit alignment
	case ASIOSTInt32LSB20: // 32 bit data with 20 bit alignment
	case ASIOSTInt32LSB24: // 32 bit data with 24 bit alignment
	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can more easily used with these
	case ASIOSTInt32MSB16: // 32 bit data with 18 bit alignment
	case ASIOSTInt32MSB18: // 32 bit data with 18 bit alignment
	case ASIOSTInt32MSB20: // 32 bit data with 20 bit alignment
	case ASIOSTInt32MSB24: // 32 bit data with 24 bit alignment
    default:
	post("Output sample Type %d not supported, yet!!!",format);
        return NULL;
    }
}

static converter_t *asio_converter_receive (ASIOSampleType format) {
#ifdef ASIODEBUG
 	/* post("ASIO: Sample Type %d", format); */
#endif
    switch (format) {
	case ASIOSTInt16LSB:  return Int16tofloat32;
	case ASIOSTInt24LSB:  return Int24tofloat32; // used for 20 bits as well
	case ASIOSTInt32LSB:  return Int32tofloat32;
	case ASIOSTInt16MSB:  return Int16tofloat32_S;
	case ASIOSTInt24MSB:  return Int24tofloat32_S; // used for 20 bits as well
	case ASIOSTInt32MSB:  return Int32tofloat32_S;
	case ASIOSTFloat32MSB:return float32tofloat32_S; // IEEE 754 32 bit float, as found on Intel x86 architecture
	case ASIOSTFloat32LSB:return float32tofloat32;   // IEEE 754 32 bit float, as found on Intel x86 architecture
	case ASIOSTFloat64LSB:return float64tofloat32;   // IEEE 754 64 bit double float, as found on Intel x86 architecture
    case ASIOSTFloat64MSB:
	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can more easily used with these
	case ASIOSTInt32LSB16: // 32 bit data with 18 bit alignment
	case ASIOSTInt32LSB18: // 32 bit data with 18 bit alignment
	case ASIOSTInt32LSB20: // 32 bit data with 20 bit alignment
	case ASIOSTInt32LSB24: // 32 bit data with 24 bit alignment
	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can more easily used with these
	case ASIOSTInt32MSB16: // 32 bit data with 18 bit alignment
	case ASIOSTInt32MSB18: // 32 bit data with 18 bit alignment
	case ASIOSTInt32MSB20: // 32 bit data with 20 bit alignment
	case ASIOSTInt32MSB24: // 32 bit data with 24 bit alignment
    default:
	post("Input sample Type %d not supported, yet!!!",format);
        return NULL;
    }
}

static int asio_get_samplewidth(ASIOSampleType format) {
	switch (format) {
	case ASIOSTInt16LSB:  case ASIOSTInt16MSB: return 2;
	case ASIOSTInt24LSB:  case ASIOSTInt24MSB: return 3;
	case ASIOSTFloat32LSB:case ASIOSTFloat32MSB:
	case ASIOSTInt32LSB:  case ASIOSTInt32MSB:
	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can more easily used with these
	case ASIOSTInt32LSB16:
	case ASIOSTInt32LSB18:
	case ASIOSTInt32LSB20:
	case ASIOSTInt32LSB24:
	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can more easily used with these
	case ASIOSTInt32MSB16:
	case ASIOSTInt32MSB18:
	case ASIOSTInt32MSB20:
	case ASIOSTInt32MSB24:
        return 4;
	case ASIOSTFloat64MSB:
	case ASIOSTFloat64LSB:
        return 8;
    default:
        post("Input sample Type %d not supported, yet!!!",format);
        return 0;
	}
}

/* dithering algo taken from Portaudio ASIO implementation */
/*************************************************************
** Calculate 2 LSB dither signal with a triangular distribution.
** Ranged properly for adding to a 32 bit integer prior to >>15.
*/
#define DITHER_BITS   (15)
#define DITHER_SCALE  (1.0f / ((1<<DITHER_BITS)-1))
inline static long triangulardither() {
        static unsigned long previous = 0;
        static unsigned long randSeed1 = 22222;
        static unsigned long randSeed2 = 5555555;
        long current, highPass;
/* Generate two random numbers. */
        randSeed1 = (randSeed1 * 196314165) + 907633515;
        randSeed2 = (randSeed2 * 196314165) + 907633515;
/* Generate triangular distribution about 0. */
        current = (((long)randSeed1)>>(32-DITHER_BITS)) + (((long)randSeed2)>>(32-DITHER_BITS));
 /* High pass filter to reduce audibility. */
        highPass = current - previous;
        previous = current;
        return highPass;
}

/* sample conversion functions */

#define SCALE_INT16 32767.f       /* (- (expt 2 15) 1) */
#define SCALE_INT24 8388607.f     /* (- (expt 2 23) 1) */
#define SCALE_INT32 2147483647.f  /* (- (expt 2 31) 1) */

/* Swap LSB to MSB and vice versa */
inline __int32 swaplong(__int32 v) {
    return ((v>>24)&0xFF)|((v>>8)&0xFF00)|((v&0xFF00)<<8)|((v&0xFF)<<24);
}

inline __int16 swapshort(__int16 v) {
    return ((v>>8)&0xFF)|((v&0xFF)<<8);
}

/* todo: check dithering */

static void float32tofloat32(void* inbuffer, void* outbuffer, long frames) {
    if(SIMD_CHECK2(frames,inbuffer,outbuffer)) copyvec_simd((float *)outbuffer,(float *)inbuffer,frames);
    else memcpy (outbuffer, inbuffer, frames* sizeof (float));
}

static void float32tofloat32_S(void* inbuffer, void* outbuffer, long frames) {
    __int32 *in = (__int32 *)inbuffer;
    __int32* out = (__int32*)outbuffer;
    while (frames--) *out++ = swaplong(*(in++));
}

static void float32tofloat64(void* inbuffer, void* outbuffer, long frames) {
    const float *in = (const float *)inbuffer;
    double* out = (double*)outbuffer;
    while (frames--) *(out++) = *(in++);
}

static void float64tofloat32(void* inbuffer, void* outbuffer, long frames) {
    const double *in = (const double *)inbuffer;
    float *out = (float *)outbuffer;
    while (frames--) *(out++) = *(in++);
}

static void float32toInt16(void* inbuffer, void* outbuffer, long frames) {
    const float *in = (const float *)inbuffer;
    __int16* out = (__int16*)outbuffer;
    while (frames--) {
	float o = *(in++) * SCALE_INT16 + triangulardither() * DITHER_SCALE;
	__int16 lng = lrintf(o);
	*out++ = lng ;
    }
}

static void Int16tofloat32(void* inbuffer, void* outbuffer, long frames) {
    const __int16* in = (const __int16*)inbuffer;
    float *out = (float *)outbuffer;
    while (frames--) *(out++) = (float)*(in++) * (1.f / SCALE_INT16);
}

static void float32toInt24(void* inbuffer, void* outbuffer, long frames) {
    const float *in = (const float *)inbuffer;
    __int32* out = (__int32*)outbuffer;
    while (frames--) {
        float o = *(in++) * SCALE_INT24;
	__int32 intg = (__int32)lrintf(o);
	*(out++) = intg;
    }
}

static void Int24tofloat32(void* inbuffer, void* outbuffer, long frames) {
    const __int32* in = (const __int32*)inbuffer;
    float *out = (float *)outbuffer;
    while (frames--) *(out++) = (float)*(in++) * (1.f / SCALE_INT24);
}

static void float32toInt32(void* inbuffer, void* outbuffer, long frames) {
    const float *in = (const float *)inbuffer;
    __int32* out = (__int32*)outbuffer;
    while (frames--) {
        float o = (float)*(in++) * SCALE_INT32 + triangulardither() * DITHER_SCALE;
	*out++ = lrintf(o);
    }
}

static void Int32tofloat32(void* inbuffer, void* outbuffer, long frames) {
    const __int32* in = (const __int32*)inbuffer;
    float *out = (float *)outbuffer;
    while (frames--) *(out++) = (float)*(in++) * (1.f / SCALE_INT32);
}

static void float32toInt16_S(void* inbuffer, void* outbuffer, long frames) {
    const float *in = (const float *)inbuffer;
    __int16* out = (__int16*)outbuffer;
    while (frames--) {
	float o = (float)*(in++) * SCALE_INT16 + triangulardither() * DITHER_SCALE;
	__int16 reverse = (__int16)lrintf(o);
        *out++ = swapshort(reverse);
    }
}

static void Int16tofloat32_S(void* inbuffer, void* outbuffer, long frames) {
    const __int16* in = (const __int16*)inbuffer;
    float *out = (float *)outbuffer;
    while (frames--) *(out++) = (float)swapshort(*(in++)) * (1.f / SCALE_INT16);
}

static void float32toInt24_S(void* inbuffer, void* outbuffer, long frames) {
    const float *in = (const float *)inbuffer;
    char* out = (char*)outbuffer;
    while (frames--) {
        float o = (float)*(in++) * SCALE_INT24;
	__int32 reverse = (__int32)lrintf(o);
        out[2] = ((char *)&reverse)[0];
        out[1] = ((char *)&reverse)[1];
        out[0] = ((char *)&reverse)[2];
        out += 3;
    }
}

static void Int24tofloat32_S(void* inbuffer, void* outbuffer, long frames) {
    const char* in = (const char*)inbuffer;
    float *out = (float *)outbuffer;
    __int32 d = 0;
    while (frames--) {
        ((char *)&d)[1] = in[2];
        ((char *)&d)[2] = in[1];
        ((char *)&d)[3] = in[0];
	*(out++) = (float)d * (1.f / SCALE_INT24);
        in += 3;
    }
}

static void float32toInt32_S(void* inbuffer, void* outbuffer, long frames) {
    const float *in = (const float *)inbuffer;
    __int32* out = (__int32*)outbuffer;
    while (frames--) {
    float o = (float)*(in++) * SCALE_INT32 + triangulardither() * DITHER_SCALE;
	__int32 reverse = (__int32)lrintf(o);
        *out++ = swaplong(reverse);
    }
}

static void Int32tofloat32_S(void* inbuffer, void* outbuffer, long frames) {
    const __int32* in = (const __int32*)inbuffer;
    float *out = (float *)outbuffer;
    while (frames--) *(out++) = (float)swaplong(*(in++)) * (1.f / SCALE_INT32);
}


/* some local helper functions */
static void prepare_asio_drivernames() {
	if (!asio_drivernames) {
		asio_drivernames = (char**)getbytes(MAXNDEV * sizeof(char*));
		for (int i = 0; i!= MAXNDEV; ++i) {
			asio_drivernames[i] = (char*)getbytes (32 * sizeof(char));
		}
	}
	/* load the driver  */
	if (!asioDrivers) asioDrivers = new AsioDrivers();
	return;
}

/* callback-based scheduling callbacks: */

/* buffer switch callback */
static void asio_bufferSwitch_cb(long db_idx, ASIOBool directprocess) {
	ASIOTime time;
#ifdef ASIODEBUG
	static int written = 0;
	if(written  == 0) {
		post("ASIO: asio_bufferSwitch_cb");
		written = 1;
	}
#endif
	memset (&time, 0, sizeof (time));
	/* todo: do we need to syncronize with other media ??? */
	asio_bufferSwitchTimeInfo_cb(&time, db_idx, directprocess);
}

/* sample rate change callback */
static void asio_sampleRateDidChange_cb(ASIOSampleRate srate) {
	asio_sampleRateDidChange(srate);
}

/* asio messaging callback */
static long asio_messages_cb(long selector, long value, void* message, double* opt) {
	return asio_messages(selector, value, message, opt);
}

static ASIOTime *asio_bufferSwitchTimeInfo_cb(ASIOTime *params, long db_idx, ASIOBool directprocess) {
	/* todo: i'm not sure if we'll have to synchronize with other media ... probably yes ... */
	/* perform the processing */
	int timeout = sys_dacblocksize * (float)asio_ticks_per_callback / (float) sys_dacsr * 1e6;
	if (sys_timedlock(timeout) == ETIMEDOUT)
		/* we're late ... lets hope that jack doesn't kick us out */
		return 0;

	for (int j = 0; j != asio_ticks_per_callback; j++) {
		t_sample * sp = sys_soundin;
		/* get sounds from input channels */
		for (long i = 0; i < sys_outchannels + sys_inchannels; i++) {
			if(asio_converter[i])
				if (asio_bufferinfo[i].isInput == ASIOTrue) {
					asio_converter[i]((char*)asio_bufferinfo[i].buffers[db_idx] +
						asio_samplewidth[i] * j *sys_dacblocksize, sp, sys_dacblocksize);
					sp += sys_dacblocksize;
				}
		}
		/* run dsp */
		sched_tick(sys_time + sys_time_per_dsp_tick);
		sp = sys_soundout;
		/* send sound to hardware */
		for (long i = 0; i < sys_outchannels + sys_inchannels; i++) {
			if (asio_bufferinfo[i].isInput != ASIOTrue) {
				/* clip */
				t_float lo = -1.f;
				t_float hi = 1.f;
				t_int clipargs[6];
				clipargs[1] = clipargs[2] = (t_int)sp;
				clipargs[3] = (t_int)&lo;
				clipargs[4] = (t_int)&hi;
				clipargs[5] = (t_int)sys_dacblocksize;
				clipblock(clipargs);
				/* send */
				if(asio_converter[i])
					asio_converter[i](sp, (char*)asio_bufferinfo[i].buffers[db_idx]
						+ asio_samplewidth[i] * j *sys_dacblocksize, sys_dacblocksize);
				zeroblock(sp,sys_dacblocksize);
				sp += sys_dacblocksize;
			}
		}
	}
	if(asio_useoutputready) ASIOOutputReady();
	sys_unlock();
    return 0L; /* time info!!! */
}

t_audioapi asio_api = {
    asio_open_audio,
    asio_close_audio,
    asio_send_dacs,
    asio_getdevs,
};
