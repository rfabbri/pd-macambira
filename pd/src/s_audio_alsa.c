/* Copyright (c) 1997-2003 Guenter Geiger, Miller Puckette, Larry Troxler,
* Winfried Ritsch, Karl MacMillan, and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* this file inputs and outputs audio using the ALSA API available on linux. */

#include <alsa/asoundlib.h>

#include "m_pd.h"
#include "s_stuff.h"
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

typedef int16_t t_alsa_sample16;
typedef int32_t t_alsa_sample32;
#define ALSA_SAMPLEWIDTH_16 sizeof(t_alsa_sample16)
#define ALSA_SAMPLEWIDTH_32 sizeof(t_alsa_sample32)
#define ALSA_XFERSIZE16  (signed int)(sizeof(t_alsa_sample16) * DEFDACBLKSIZE)
#define ALSA_XFERSIZE32  (signed int)(sizeof(t_alsa_sample32) * DEFDACBLKSIZE)
#define ALSA_MAXDEV 1
#define ALSA_JITTER 1024
#define ALSA_EXTRABUFFER 2048
#define ALSA_DEFFRAGSIZE 64
#define ALSA_DEFNFRAG 12

#ifndef INT32_MAX
#define INT32_MAX 0x7fffffff
#endif

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

static int alsa_mode;

/* Defines */
#define DEBUG(x) x
#define DEBUG2(x) {x;}

static char alsa_devname[512] = "hw:0,0";
static int alsa_use_plugin = 0;

    /* don't assume we can turn all 31 bits when doing float-to-fix; 
    otherwise some audio drivers (e.g. Midiman/ALSA) wrap around. */
#define FMAX 0x7ffff000
#define CLIP32(x) (((x)>FMAX)?FMAX:((x) < -FMAX)?-FMAX:(x))

    /* ugly Alsa-specific flags  */
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

/* support for ALSA pcmv2 api by Karl MacMillan<karlmac@peabody.jhu.edu> */ 

static void check_error(int err, const char *why)
{
    if (err < 0)
	fprintf(stderr, "%s: %s\n", why, snd_strerror(err));
}

int alsa_open_audio(int wantinchans, int wantoutchans, int srate)
{
    int err, inchans = 0, outchans = 0, subunitdir;
    char devname[512];
    snd_pcm_hw_params_t* hw_params;
    snd_pcm_sw_params_t* sw_params;
    snd_output_t* out;
    int frag_size = (sys_blocksize ? sys_blocksize : ALSA_DEFFRAGSIZE);
    int nfrags, i;
    short* tmp_buf;
    unsigned int tmp_uint;

    nfrags = sys_schedadvance * (float)srate / (1e6 * frag_size);

    if (sys_verbose)
    	post("audio buffer set to %d", (int)(0.001 * sys_schedadvance));
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

    if (inchans)
      snd_pcm_prepare(alsa_device.inhandle);
    if (outchans)
      snd_pcm_prepare(alsa_device.outhandle);

    // if duplex we can link the channels so they start together
    if (inchans && outchans)
      snd_pcm_link(alsa_device.inhandle, alsa_device.outhandle);

    // set up the buffer
    if (outchans > inchans) 
	alsa_buf = (short *)calloc(sizeof(char) * alsa_samplewidth,
	    DEFDACBLKSIZE * outchans);
    else
	alsa_buf = (short *)calloc(sizeof(char) * alsa_samplewidth, 
	    DEFDACBLKSIZE * inchans);
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
    if (sys_inchannels)
    {
	err = snd_pcm_close(alsa_device.inhandle);
	check_error(err, "snd_pcm_close (input)");
    }
    if (sys_outchannels)
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
    int inchannels = sys_inchannels;
    int outchannels = sys_outchannels;
    unsigned int intransfersize = DEFDACBLKSIZE;
    unsigned int outtransfersize = DEFDACBLKSIZE;

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
	    for (i = 0, fp1 = fp; i < outchannels; i++, fp1 += DEFDACBLKSIZE)
	    {
		for (j = i, k = DEFDACBLKSIZE, fp2 = fp1; k--;
		     j += outchannels, fp2++)
		{
		    float s1 = *fp2 * INT32_MAX;
		    ((t_alsa_sample32 *)alsa_buf)[j] = CLIP32(s1);
		} 
	    }
	}
	else
	{
	    for (i = 0, fp1 = fp; i < outchannels; i++, fp1 += DEFDACBLKSIZE)
	    {
		for (j = i, k = DEFDACBLKSIZE, fp2 = fp1; k--;
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
		    (ALSA_EXTRABUFFER + sys_advance_samples)
		    * alsa_samplewidth * outchannels);
    #endif
	    sys_log_error(ERR_DACSLEPT);
	    return (SENDDACS_NO);
	}

	/* zero out the output buffer */
	memset(sys_soundout, 0, DEFDACBLKSIZE * sizeof(*sys_soundout) *
	       sys_outchannels);
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
    if (sys_inchannels)
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
	    	    (ALSA_EXTRABUFFER + sys_advance_samples)
		    * alsa_samplewidth * inchannels);
#endif
    	    sys_log_error(ERR_ADCSLEPT);
	    return (SENDDACS_NO);
	}
	fp = sys_soundin;
	if (alsa_samplewidth == 4)
	{
	    for (i = 0, fp1 = fp; i < inchannels; i++, fp1 += DEFDACBLKSIZE)
	    {
		for (j = i, k = DEFDACBLKSIZE, fp2 = fp1; k--;
		     j += inchannels, fp2++)
	    	    *fp2 = (float) ((t_alsa_sample32 *)alsa_buf)[j]
		    	* (1./ INT32_MAX);
	    }
	}
	else
	{
	    for (i = 0, fp1 = fp; i < inchannels; i++, fp1 += DEFDACBLKSIZE)
	    {
		for (j = i, k = DEFDACBLKSIZE, fp2 = fp1; k--; j += inchannels,
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
    if (sys_audioapi != API_ALSA)
    {
    	error("restart-audio: implemented for ALSA only.");
	return;
    }
    memset(alsa_buf, 0,
    	sizeof(char) * alsa_samplewidth * DEFDACBLKSIZE * sys_outchannels);
    for (i = 0; i < 1000000; i++)
    {
	result = snd_pcm_writei(alsa_device.outhandle, alsa_buf,
	    DEFDACBLKSIZE);
	if (result != (int)DEFDACBLKSIZE)
	    break;
    }
    post("%d written", i);
}

void alsa_listdevs( void)
{
    post("device listing not implemented in ALSA yet\n");
}

