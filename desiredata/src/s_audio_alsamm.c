/* Copyright (c) 1997-2003 Guenter Geiger, Miller Puckette, Larry Troxler,
* Winfried Ritsch, Karl MacMillan, and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */
/*
   this audiodriverinterface inputs and outputs audio data using
   the ALSA MMAP API available on linux.
   this is optimized for hammerfall cards and does not make an attempt to be general
   now, please adapt to your needs or let me know ...
   constrains now:
    - audio Card with ALSA-Driver > 1.0.3,
    - alsa-device (preferable hw) with MMAP NONINTERLEAVED SIGNED-32Bit features
    - up to 4 cards with has to be hardwaresynced
   (winfried)
*/
#include <alsa/asoundlib.h>
#include "desire.h"
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
#include "s_audio_alsa.h"

/* needed for alsa 0.9 compatibility: */
#if (SND_LIB_MAJOR < 1)
#define ALSAAPI9
#endif
/* sample type magic ...
   Hammerfall/HDSP/DSPMADI cards always 32Bit where lower 8Bit not used (played) in AD/DA,
   but can have some bits set (subchannel coding)
*/
#define ALSAMM_SAMPLEWIDTH_32 sizeof(t_alsa_sample32)

#ifndef INT32_MAX
#define INT32_MAX 0x7fffffff
#endif

/* maybe:
    don't assume we can turn all 31 bits when doing float-to-fix;
    otherwise some audio drivers (e.g. Midiman/ALSA) wrap around.
    but not now on hammerfall (w)
*/

/* 24 Bit are used so MAX Samplevalue not INT32_MAX ??? */
#define F32MAX 0x7fffff00
#define CLIP32(x) (((x)>F32MAX)?F32MAX:((x) < -F32MAX)?-F32MAX:(x))

#define ALSAMM_FORMAT SND_PCM_FORMAT_S32
/* maximum of 4 devices. you can mix rme9632,hdsp9632 (18 chans) rme9652,hdsp9652 (26 chans), dsp-madi (64 chans) if synced */

/* we need same samplerate, buffertime and so on for each card soo we use global vars...
   time is in us, size in frames (i hope so ;-) */
static unsigned int alsamm_sr = 0;
static unsigned int alsamm_buffertime = 0;
static unsigned int alsamm_buffersize = 0;

static bool debug=0;

/* bad style: we asume all cards give the same answer at init so we make this vars global
   to have a faster access in writing reading during send_dacs */
static snd_pcm_sframes_t alsamm_period_size;
static unsigned int alsamm_periods;
static snd_pcm_sframes_t alsamm_buffer_size;

/* if more than this sleep detected, should be more than periodsize/samplerate ??? */
static double sleep_time;

/* now we just sum all inputs/outputs of used cards to a global count
   and use them all
   ... later we should just use some channels of each card for pd
   so we reduce the overhead of using alsways all channels,
   and zero the rest once at start,
   because rme9652 and hdsp forces us to use all channels
   in mmap mode...

Note on why:
   normally hdsp and dspmadi can handle channel
   count from one to all since they can switch on/off
   the dma for them to reduce pci load, but this is only
   implemented in alsa low level drivers for dspmadi now and maybe fixed for hdsp in future
*/

static int alsamm_inchannels = 0;
static int alsamm_outchannels = 0;

/* Defines */
 #define WATCH_PERIODS 90
 static int in_avail[WATCH_PERIODS];
 static int out_avail[WATCH_PERIODS];
 static int in_offset[WATCH_PERIODS];
 static int out_offset[WATCH_PERIODS];
 static int out_cm[WATCH_PERIODS];
 static char *outaddr[WATCH_PERIODS];
 static char *inaddr[WATCH_PERIODS];
 static int xruns_watch[WATCH_PERIODS];
 static int broken_opipe;

 static int dac_send = 0;
 static int alsamm_xruns = 0;

static void show_availist() {
  for(int i=1; i<WATCH_PERIODS; i++) {
    post("%2d:avail i=%7d %s o=%7d(%5d), offset i=%7d %s o=%7d, ptr i=%12p o=%12p, %d xruns ",
         i,in_avail[i],(out_avail[i] != in_avail[i])? "!=" : "==" , out_avail[i],out_cm[i],
         in_offset[i],(out_offset[i] != in_offset[i])? "!=" : "==" , out_offset[i],
         inaddr[i], outaddr[i], xruns_watch[i]);
  }
}

/* protos */
static int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params, int *chs);
static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams, int playback);
static void alsamm_start();
static void alsamm_stop();

/* for debugging attach output of alsa mesages to stdout stream */
snd_output_t* alsa_stdout;

static void check_error(int err, const char *why) {if (err < 0) error("%s: %s", why, snd_strerror(err));}
class AlsaError : Error {
public:
  int err;
  AlsaError (int err) : err(err) {}};
#define CHK(CODE) do {int err=CODE; if (err<0) {error("%s: %s", #CODE, snd_strerror(err)); throw AlsaError(err);}} while (0)
#define  CH(CODE) do {int err=CODE; if (err<0) {error("%s: %s", #CODE, snd_strerror(err));                      }} while (0)

int alsamm_open_audio(int rate) {
  int err;
  snd_pcm_hw_params_t* hw_params;
  snd_pcm_sw_params_t* sw_params;
  /* fragsize is an old concept now use periods, used to be called fragments. */
  /* Be aware in ALSA periodsize can be in bytes, where buffersize is in frames,
     but sometimes buffersize is in bytes and periods in frames, crazy alsa...
     ...we use periodsize and buffersize in frames */
  int i;
  snd_pcm_hw_params_alloca(&hw_params);
  snd_pcm_sw_params_alloca(&sw_params);
  /* see add_devname */
  /* first have a look which cards we can get and set up device infos for them */
  /* init some structures */
  for(i=0;i < ALSA_MAXDEV;i++) {
    alsa_indev[i].a_synced=alsa_outdev[i].a_synced=0;
    alsa_indev[i].a_channels=alsa_outdev[i].a_channels=-1; /* query defaults */
  }
  alsamm_inchannels = 0;
  alsamm_outchannels = 0;
  /* opening alsa debug channel */
  CH(snd_output_stdio_attach(&alsa_stdout, stdout, 0));
  /* Weak failure prevention:
     first card found (out then in) is used as a reference for parameter,
     so this  set the globals and other cards hopefully dont change them
  */
  alsamm_sr = rate;
  /* set the asked buffer time (alsa buffertime in us)*/
  alsamm_buffertime = alsamm_buffersize = 0;
  if(sys_blocksize == 0)
    alsamm_buffertime = sys_schedadvance;
  else
    alsamm_buffersize = sys_blocksize;
  if(sys_verbose)
    post("syschedadvance=%d us(%d Samples)so buffertime max should be this=%d"
         "or sys_blocksize=%d (samples) to use buffersize=%d",
         sys_schedadvance,sys_advance_samples,alsamm_buffertime,
         sys_blocksize,alsamm_buffersize);
  alsamm_periods = 0; /* no one wants periods setting from command line ;-) */
  for (int i=0; i<alsa_noutdev;i++) {
    /* post("open audio out %d, of %lx, %d",i,&alsa_device[i], alsa_outdev[i].a_handle); */
    try {
      CHK(set_hwparams(alsa_outdev[i].a_handle, hw_params, &(alsa_outdev[i].a_channels)));
      CHK(set_swparams(alsa_outdev[i].a_handle, sw_params,1));
      alsamm_outchannels += alsa_outdev[i].a_channels;
      alsa_outdev[i].a_addr = (char **)malloc(sizeof(char *)*alsa_outdev[i].a_channels);
      if(!alsa_outdev[i].a_addr) {error("playback device outaddr allocation error:"); continue;}
      memset(alsa_outdev[i].a_addr, 0, sizeof(char*) * alsa_outdev[i].a_channels);
      post("playback device with %d channels and buffer_time %d us opened", alsa_outdev[i].a_channels, alsamm_buffertime);
    } catch (AlsaError) {continue;}
  }
  for (int i=0; i<alsa_nindev; i++) {
      if(sys_verbose) post("capture card %d:--------------------",i);
      CHK(set_hwparams(alsa_indev[i].a_handle, hw_params, &(alsa_indev[i].a_channels)));
      alsamm_inchannels += alsa_indev[i].a_channels;
      CHK(set_swparams(alsa_indev[i].a_handle, sw_params,0));
      alsa_indev[i].a_addr = (char **)malloc(sizeof(char*)*alsa_indev[i].a_channels);
      if(!alsa_indev[i].a_addr) {error("capture device inaddr allocation error:"); continue;}
      memset(alsa_indev[i].a_addr, 0, sizeof(char*) * alsa_indev[i].a_channels);
      if(sys_verbose) post("capture device with %d channels and buffertime %d us opened", alsa_indev[i].a_channels,alsamm_buffertime);
  }
  /* check for linked handles of input for each output*/
  for (int i=0; i<(alsa_noutdev < alsa_nindev ? alsa_noutdev:alsa_nindev); i++) {
    if (alsa_outdev[i].a_devno == alsa_indev[i].a_devno) {
      if ((err = snd_pcm_link(alsa_indev[i].a_handle, alsa_outdev[i].a_handle)) == 0) {
        alsa_indev[i].a_synced = alsa_outdev[i].a_synced = 1;
        if(sys_verbose) post("Linking in and outs of card %d",i);
      } else error("could not link in and outs");
    }
  }
  /* some globals */
  sleep_time =  (float) alsamm_period_size/ (float) alsamm_sr;
 if (debug) {
  /* start ---------------------------- */
  if(sys_verbose) post("open_audio: after dacsend=%d (xruns=%d)done",dac_send,alsamm_xruns);
  alsamm_xruns = dac_send = 0; /* reset debug */
  /* start alsa in open or better in send_dacs once ??? we will see */
  for (int i=0;i<alsa_noutdev;i++) snd_pcm_dump(alsa_outdev[i].a_handle, alsa_stdout);
  for (int i=0;i<alsa_nindev;i++)  snd_pcm_dump( alsa_indev[i].a_handle, alsa_stdout);
  fflush(stdout);
 }
  sys_setchsr(alsamm_inchannels,  alsamm_outchannels, alsamm_sr, sys_dacblocksize);
  alsamm_start();
  /* report success  */
  return 0;
}

void alsamm_close_audio() {
  if(debug&&sys_verbose) post("closing devices");
  alsamm_stop();
  for (int i=0; i<alsa_noutdev; i++) {
    //if(debug&&sys_verbose) post("unlink audio out %d, of %lx",i,used_outdevice[i]);
    if(alsa_outdev[i].a_synced != 0) {
      CHK(snd_pcm_unlink(alsa_outdev[i].a_handle));
      alsa_outdev[i].a_synced = 0;
     }
    CHK(snd_pcm_close(alsa_outdev[i].a_handle));
    if(alsa_outdev[i].a_addr) {free(alsa_outdev[i].a_addr); alsa_outdev[i].a_addr=0;}
    alsa_outdev[i].a_channels = 0;
  }
  for (int i=0; i<alsa_nindev; i++) {
    CHK(snd_pcm_close(alsa_indev[i].a_handle));
    if(alsa_indev[i].a_addr) {free(alsa_indev[i].a_addr); alsa_indev[i].a_addr=0;}
    alsa_indev[i].a_channels = 0;
  }
  alsa_nindev = alsa_noutdev = 0;
 if(debug) {
  if(sys_verbose) post("close_audio: after dacsend=%d (xruns=%d)done",dac_send,alsamm_xruns);
   alsamm_xruns = dac_send = 0;
 }
}

/* ------- PCM INITS --------------------------------- */
static int set_hwparams(snd_pcm_t *handle, snd_pcm_hw_params_t *params,int *chs) {
 try {
#ifndef ALSAAPI9
  unsigned int rrate;
  /* choose all parameters */
  CHK(snd_pcm_hw_params_any(handle, params));
  /* set the nointerleaved read/write format */
  CHK(snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_MMAP_NONINTERLEAVED));
  /* set the sample format */
  CHK(snd_pcm_hw_params_set_format(handle, params, ALSAMM_FORMAT));
  if(debug&&sys_verbose) post("Setting format to %s",snd_pcm_format_name(ALSAMM_FORMAT));
  /* first check samplerate since channels numbers are samplerate dependend (double speed) */
  /* set the stream rate */
  rrate = alsamm_sr;
  if(debug&&sys_verbose) post("Samplerate request: %i Hz",rrate);
  int dir=-1;
  CHK(snd_pcm_hw_params_set_rate_near(handle, params, &rrate, &dir));
  if (rrate != alsamm_sr) {
    post("Warning: rate %iHz doesn't match requested %iHz", rrate,alsamm_sr);
    alsamm_sr = rrate;
  } else if (sys_verbose) post("Samplerate is set to %iHz",alsamm_sr);
  /* Info on channels */
  {
    int maxchs,minchs,channels = *chs;
    CHK(snd_pcm_hw_params_get_channels_max(params, (unsigned *)&maxchs));
    CHK(snd_pcm_hw_params_get_channels_min(params, (unsigned *)&minchs));
    if(debug&&sys_verbose) post("Getting channels:min=%d, max= %d for request=%d",minchs,maxchs,channels);
    if(channels<0) channels=maxchs;
    if(channels>maxchs) channels = maxchs;
    if(channels<minchs) channels = minchs;
    if(channels != *chs) post("requested channels=%d but used=%d",*chs,channels);
    *chs = channels;
    if(debug&&sys_verbose) post("trying to use channels: %d",channels);
  }
  /* set the count of channels */
  CHK(snd_pcm_hw_params_set_channels(handle, params, *chs));
  /* testing for channels */
  CH(snd_pcm_hw_params_get_channels(params,(unsigned int *)chs));
  if(debug&&sys_verbose) post("When setting channels count, got %d",*chs);
  /* if buffersize is set use this instead buffertime */
  if(alsamm_buffersize > 0) {
    if(debug&&sys_verbose) post("hw_params: ask for max buffersize of %d samples", (unsigned int) alsamm_buffersize);
    alsamm_buffer_size = alsamm_buffersize;
    CHK(snd_pcm_hw_params_set_buffer_size_near(handle, params, (unsigned long *)&alsamm_buffer_size));
  }
  else {
    if(alsamm_buffertime <= 0) /* should never happen, but use 20ms */
      alsamm_buffertime = 20000;
    if(debug&&sys_verbose) post("hw_params: ask for max buffertime of %d ms", (unsigned int) (alsamm_buffertime*0.001) );
    CHK(snd_pcm_hw_params_set_buffer_time_near(handle, params, &alsamm_buffertime, &dir));
  }
  CHK(snd_pcm_hw_params_get_buffer_time(params, (unsigned *)&alsamm_buffertime, &dir));
  if(debug&&sys_verbose) post("hw_params: got buffertime to %f ms", float(alsamm_buffertime*0.001));
  CHK(snd_pcm_hw_params_get_buffer_size(params, (unsigned long *)&alsamm_buffer_size));
  if(debug&&sys_verbose) post("hw_params: got  buffersize to %d samples",(int) alsamm_buffer_size);
  CHK(snd_pcm_hw_params_get_period_size(params, (unsigned long *)&alsamm_period_size, &dir));
  if(debug&&sys_verbose) post("Got period size of %d", (int) alsamm_period_size);
  {
    unsigned int pmin,pmax;
    CHK(snd_pcm_hw_params_get_periods_min(params, &pmin, &dir));
    CHK(snd_pcm_hw_params_get_periods_min(params, &pmax, &dir));
    /* use maximum of periods */
    if( alsamm_periods <= 0) alsamm_periods = pmax;
    alsamm_periods = (alsamm_periods > pmax)?pmax:alsamm_periods;
    alsamm_periods = (alsamm_periods < pmin)?pmin:alsamm_periods;
    CHK(snd_pcm_hw_params_set_periods(handle, params, alsamm_periods, dir));
    CHK(snd_pcm_hw_params_get_periods(params, &pmin, &dir));
    if(debug&&sys_verbose) post("Got periods of %d, where periodsmin=%d, periodsmax=%d",alsamm_periods,pmin,pmax);
  }
  /* write the parameters to device */
  CHK(snd_pcm_hw_params(handle, params));
#endif /* ALSAAPI9 */
  return 0;
 } catch (AlsaError e) {return e.err;}
}

static int set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams, int playback) {
 try {
#ifndef ALSAAPI9
  snd_pcm_uframes_t ps,ops;
  snd_pcm_uframes_t bs,obs;
  /* get the current swparams */
  CHK(snd_pcm_sw_params_current(handle, swparams));
  /* AUTOSTART: start the transfer on each write/commit ??? */
  snd_pcm_sw_params_get_start_threshold(swparams, &obs);
  CHK(snd_pcm_sw_params_set_start_threshold(handle, swparams, 0U));
  snd_pcm_sw_params_get_start_threshold(swparams, &bs);
  if(debug&&sys_verbose) post("sw_params: got start_thresh_hold= %d (was %d)",(int) bs,(int)obs);
  /* AUTOSTOP:  never stop the machine */
  snd_pcm_sw_params_get_stop_threshold(swparams, &obs);
  CHK(snd_pcm_sw_params_set_stop_threshold(handle, swparams, (snd_pcm_uframes_t)-1));
  snd_pcm_sw_params_get_stop_threshold(swparams, &bs);
  if(debug&&sys_verbose) post("sw_params: set stop_thresh_hold= %d (was %d)", (int) bs,(int)obs);
  /* AUTOSILENCE: silence if overrun.... */
  snd_pcm_sw_params_get_silence_threshold(swparams, &ops);
  CHK(snd_pcm_sw_params_set_silence_threshold(handle, swparams, alsamm_period_size));
  snd_pcm_sw_params_get_silence_threshold(swparams, &ps);
  if(debug&&sys_verbose) post("sw_params: set silence_threshold = %d (was %d)", (int) ps,(int)ops);
  snd_pcm_sw_params_get_silence_size(swparams, &ops);
  CHK(snd_pcm_sw_params_set_silence_size(handle, swparams, alsamm_period_size));
  snd_pcm_sw_params_get_silence_size(swparams, &ps);
  if(debug&&sys_verbose) post("sw_params: set silence_size = %d (was %d)", (int) ps,(int)ops);
  /* AVAIL: allow the transfer when at least period_size samples can be processed */
  snd_pcm_sw_params_get_avail_min(swparams, &ops);
  CHK(snd_pcm_sw_params_set_avail_min(handle, swparams, sys_dacblocksize/2));
  snd_pcm_sw_params_get_avail_min(swparams, &ps);
  if(debug&&sys_verbose) post("sw_params: set avail_min= %d (was  %d)", (int) ps, (int) ops);
  /* ALIGN: align all transfers to 1 sample */
  snd_pcm_sw_params_get_xfer_align(swparams, &ops);
  CHK(snd_pcm_sw_params_set_xfer_align(handle, swparams, 1));
  snd_pcm_sw_params_get_xfer_align(swparams, &ps);
  if(debug&&sys_verbose) post("sw_params: set xfer_align = %d (was  %d)", (int) ps, (int) ops);
  /* write the parameters to the playback device */
  CHK(snd_pcm_sw_params(handle, swparams));
  if(debug&&sys_verbose) post("set sw finished");
#else
  post("alsa: need version 1.0 or above for mmap operation");
#endif /* ALSAAPI9 */
  return 0;
 } catch (AlsaError e) {return e.err;}
}

/* ALSA Transfer helps */

/* xrun_recovery is called if time to late or error
   Note: use outhandle if synced i/o; the devices are linked so prepare has only be called on out, hopefully resume too...
*/
static int xrun_recovery(snd_pcm_t *handle, int err) {
  if (debug) alsamm_xruns++; /* count xruns */
  if (err == -EPIPE) {    /* under-run */
    CH(snd_pcm_prepare(handle));
    CH(snd_pcm_start(handle));
    return 0;
  } else if (err == -ESTRPIPE) {
    while ((err = snd_pcm_resume(handle)) == -EAGAIN) sleep(1); /* wait until the suspend flag is released */
    if (err < 0) {
      CH(snd_pcm_prepare(handle));
      CH(snd_pcm_start(handle));
    }
    return 0;
  }
  return err;
}

/* note that snd_pcm_avail has to be called before using this funtion */
static int alsamm_get_channels(snd_pcm_t *dev, snd_pcm_uframes_t *avail, snd_pcm_uframes_t *offset, int nchns, char **addr) {
  int err = 0;
  const snd_pcm_channel_area_t *mm_areas;
  if (nchns>0 && avail && offset) {
    err = snd_pcm_mmap_begin(dev, &mm_areas, offset, avail);
    if (err<0) {check_error(err,"setmems: begin_mmap failure ???"); return err;}
    for (int chn=0; chn<nchns; chn++) {
      const snd_pcm_channel_area_t *a = &mm_areas[chn];
      addr[chn] = (char *) a->addr + ((a->first + a->step * *offset) / 8);
    }
    return err;
  }
  return -1;
}

static void alsamm_start() {
  int err = 0;
  /* first prepare for in/out */
  for (int devno=0; devno<alsa_noutdev; devno++) {
    snd_pcm_uframes_t offset, avail;
    t_alsa_dev *dev = &alsa_outdev[devno];
    /* snd_pcm_prepare also in xrun, but cannot harm here */
    err = snd_pcm_prepare(dev->a_handle);
    if (err<0) {check_error(err,"outcard prepare error for playback"); return;}
    offset = 0;
    avail = snd_pcm_avail_update(dev->a_handle);
    if (avail != (snd_pcm_uframes_t) alsamm_buffer_size) {
      check_error(avail,"full buffer not available at start");
    }
    /* cleaning out mmap buffer before start */
    if(debug&&sys_verbose) post("start: set mems for avail=%d,offset=%d at buffersize=%d",avail,offset,alsamm_buffer_size);
    if(avail > 0) {
      int comitted = 0;
      err = alsamm_get_channels(dev->a_handle, &avail, &offset, dev->a_channels,dev->a_addr);
      if (err<0) {check_error(err,"setting initial out channelspointer failure ?"); continue;}
      for (int chn=0; chn<dev->a_channels; chn++) memset(dev->a_addr[chn],0,avail*ALSAMM_SAMPLEWIDTH_32);
      comitted = snd_pcm_mmap_commit (dev->a_handle, offset, avail);
      avail = snd_pcm_avail_update(dev->a_handle);
      if(debug&&sys_verbose) post("start: now channels cleared, out with avail=%d, offset=%d,comitted=%d",avail,offset,comitted);
    }
    /* now start, should be autostarted */
    avail = snd_pcm_avail_update(dev->a_handle);
    if(debug&&sys_verbose) post("start: finish start, out with avail=%d, offset=%d",avail,offset);
    /* we have no autostart so anyway start*/
    err = snd_pcm_start (dev->a_handle);
    if (err<0) check_error(err,"could not start playback");
  }
  for (int devno=0; devno<alsa_nindev; devno++) {
    snd_pcm_uframes_t ioffset, iavail;
    t_alsa_dev *dev = &alsa_indev[devno];
    /* if devices are synced then don't need to prepare; hopefully dma in aereas allready filled correct by the card */
    if (dev->a_synced == 0) {
      err = snd_pcm_prepare (dev->a_handle);
      if (err<0) {check_error(err,"incard prepare error for capture"); /* return err;*/}
    }
    ioffset = 0;
    iavail = snd_pcm_avail_update (dev->a_handle);
    /* cleaning out mmap buffer before start */
    if (debug) post("start in: set in mems for avail=%d,offset=%d at buffersize=%d",iavail,ioffset,alsamm_buffer_size);
    if (iavail > (snd_pcm_uframes_t) 0) {
      if (debug) post("empty buffer not available at start, since avail %d != %d buffersize", iavail, alsamm_buffer_size);
      err = alsamm_get_channels(dev->a_handle, &iavail, &ioffset, dev->a_channels,dev->a_addr);
      if (err<0) {check_error(err,"getting in channelspointer failure ????"); continue;}
      snd_pcm_mmap_commit (dev->a_handle, ioffset, iavail);
      iavail = snd_pcm_avail_update (dev->a_handle);
      if (debug) post("start in now avail=%d",iavail);
    }
    if (debug) post("start: init inchannels with avail=%d, offset=%d",iavail,ioffset);
    /* if devices are synced then dont need to start */
    /* start with autostart , but anyway start */
    if(dev->a_synced == 0) {
      err = snd_pcm_start (dev->a_handle);
      if (err<0) {check_error(err,"could not start capture"); continue;}
    }
  }
}

static void alsamm_stop() {
  for (int devno=0; devno<alsa_nindev; devno++) {
    t_alsa_dev *dev = &alsa_indev[devno];
    if(sys_verbose) post("stop in device %d",devno);
    CH(snd_pcm_drop(dev->a_handle));
  }
  for (int devno=0; devno<alsa_noutdev;devno++) {
    t_alsa_dev *dev = &alsa_outdev[devno];
    if(sys_verbose) post("stop out device %d",devno);
    CH(snd_pcm_drop(dev->a_handle));
  }
  if (debug) show_availist();
}

/* ---------- ADC/DAC tranfer in  the main loop ------- */
/* I see: (a guess as a documentation)
   all DAC data is in sys_soundout array with
   sys_dacblocksize (mostly 64) for each channels which
   if we have more channels opened then dac-channels = sys_outchannels
   we have to zero (silence them), which should be done once.

Problems to solve:

   a) Since in ALSA MMAP, the MMAP reagion can change (don't ask me why)
   we have to do it each cycle or we say on RME HAMMERFALL/HDSP/DSPMADI
   it never changes to it once. so maybe we can do it once in open

   b) we never know if inputs are synced and zero them if not,
   except we use the control interface to check for, but this is
   a systemcall we cannot afford in RT Loops so we just dont
   and if not it will click... users fault ;-)))
*/

int alsamm_send_dacs() {
  static double timenow,timelast;
  t_sample *fpo, *fpi, *fp1, *fp2;
  int i, err, devno;
  snd_pcm_sframes_t size;
  snd_pcm_sframes_t commitres;
  snd_pcm_state_t state;
  snd_pcm_sframes_t ooffset, oavail;
  snd_pcm_sframes_t ioffset, iavail;
  /* unused channels should be zeroed out on startup (open) and stay this */
  int inchannels = sys_inchannels;
  int outchannels = sys_outchannels;
  timelast = sys_getrealtime();
 if (debug) {
  if(dac_send++ < 0) post("dac send called in %d, out %d, xrun %d",inchannels,outchannels, alsamm_xruns);
  if(alsamm_xruns && (alsamm_xruns % 1000) == 0) post("1000 xruns accoured");
  if(dac_send < WATCH_PERIODS) {
    out_cm[dac_send] = -1;
    in_avail[dac_send] = out_avail[dac_send] = -1;
    in_offset[dac_send] = out_offset[dac_send] = -1;
    outaddr[dac_send] = inaddr[dac_send] = NULL;
    xruns_watch[dac_send] = alsamm_xruns;
  }
 }
  if (!inchannels && !outchannels) return SENDDACS_NO;
  /* here we should check if in and out samples are here.
     but, the point is if out samples available also in sample should,
     so we don't make a precheck of insamples here and let outsample check be the first of the first card. */
  /* OUTPUT Transfer */
  fpo = sys_soundout;
  for(devno = 0;devno < alsa_noutdev;devno++) {
    t_alsa_dev *dev = &alsa_outdev[devno];
    snd_pcm_t *out = dev->a_handle;
    int ochannels =dev->a_channels;
    /* how much samples available ??? */
    oavail = snd_pcm_avail_update(out);
    /* only one reason i can think about, the driver stopped and says broken pipe
       so this should not happen if we have enough stopthreshold but if try to restart with next commit */
    if (oavail < 0) {
      if (debug) broken_opipe++;
      err = xrun_recovery(out, -EPIPE);
      if (err < 0) {check_error(err,"otavail<0 recovery failed"); return SENDDACS_NO;}
      oavail = snd_pcm_avail_update(out);
    }
    /* check if we are late and have to (able to) catch up */
    /* xruns will be ignored since you cant do anything since already happend */
    state = snd_pcm_state(out);
    if (state == SND_PCM_STATE_XRUN) {
      err = xrun_recovery(out, -EPIPE);
      if (err < 0) {check_error(err,"DAC XRUN recovery failed"); return SENDDACS_NO;}
      oavail = snd_pcm_avail_update(out);
    } else if (state == SND_PCM_STATE_SUSPENDED) {
      err = xrun_recovery(out, -ESTRPIPE);
      if (err < 0) {check_error(err,"DAC SUSPEND recovery failed"); return SENDDACS_NO;}
      oavail = snd_pcm_avail_update(out);
    }
    if(debug && dac_send < WATCH_PERIODS) out_avail[dac_send] = oavail;
    /* we only transfer transfersize of bytes request,
       this should only happen on first card otherwise we got a problem :-(()*/
    if(oavail < sys_dacblocksize) return SENDDACS_NO;
    /* transfer now */
    size = sys_dacblocksize;
    fp1 = fpo;
    ooffset = 0;
    /* since this can go over a buffer boundery we maybe need two steps to
       transfer (normally when buffersize is a multiple of transfersize
       this should never happen) */
    while (size > 0) {
      snd_pcm_sframes_t oframes;
      oframes = size;
      err =  alsamm_get_channels(out, (unsigned long *)&oframes, (unsigned long *)&ooffset,ochannels,dev->a_addr);
      if(debug && dac_send < WATCH_PERIODS) {
        out_offset[dac_send] = ooffset;
        outaddr[dac_send] = (char *) dev->a_addr[0];
      }
      if (err<0) {
        err = xrun_recovery(out, err);
        if (err<0) {check_error(err,"MMAP begins avail error"); break; /* next card please */}
      }
      /* transfer into memory */
      for (int chn=0; chn<ochannels; chn++) {
        t_alsa_sample32 *buf = (t_alsa_sample32 *)dev->a_addr[chn];
        /* osc(buf, oframes, (dac_send%1000 < 500)?-100.0:-10.0,440,&(indexes[chn])); */
        for (i = 0, fp2 = fp1 + chn*sys_dacblocksize; i < oframes; i++,fp2++) {
            float s1 = *fp2 * F32MAX;
            /* better but slower, better never clip ;-) buf[i] = CLIP32(s1); */
            buf[i] = (int) s1 & 0xFFFFFF00;
            *fp2 = 0.0;
        }
      }
      commitres = snd_pcm_mmap_commit(out, ooffset, oframes);
      if (commitres < 0 || commitres != oframes) {
        if ((err = xrun_recovery(out, commitres >= 0 ? -EPIPE : commitres)) < 0) {
          check_error(err,"MMAP commit error");
          return SENDDACS_NO;
        }
      }
      if(debug && dac_send < WATCH_PERIODS) out_cm[dac_send] = oframes;
      fp1 += oframes;
      size -= oframes;
    } /* while size */
    fpo += ochannels*sys_dacblocksize;
  }/* for devno */
  fpi = sys_soundin; /* star first card first channel */
  for(devno = 0;devno < alsa_nindev;devno++) {
    t_alsa_dev *dev = &alsa_indev[devno];
    snd_pcm_t *in = dev->a_handle;
    int ichannels = dev->a_channels;
    iavail = snd_pcm_avail_update(in);
    if (iavail < 0) {
      err = xrun_recovery(in, iavail);
      if (err < 0) {
        check_error(err,"input avail update failed");
        return SENDDACS_NO;
      }
      iavail=snd_pcm_avail_update(in);
    }
    state = snd_pcm_state(in);
    if (state == SND_PCM_STATE_XRUN) {
      err = xrun_recovery(in, -EPIPE);
      if (err<0) {check_error(err,"ADC XRUN recovery failed"); return SENDDACS_NO;}
      iavail=snd_pcm_avail_update(in);
    } else if (state == SND_PCM_STATE_SUSPENDED) {
      err = xrun_recovery(in, -ESTRPIPE);
      if (err < 0) {check_error(err,"ADC SUSPEND recovery failed"); return SENDDACS_NO;}
      iavail=snd_pcm_avail_update(in);
    }
    /* only transfer full transfersize or nothing */
    if(iavail < sys_dacblocksize) return SENDDACS_NO;
    size = sys_dacblocksize;
    fp1 = fpi;
    ioffset = 0;
    /* since sysdata can go over a driver buffer boundery we maybe need two steps to
       transfer (normally when buffersize is a multiple of transfersize
       this should never happen) */
    while(size > 0) {
      snd_pcm_sframes_t iframes = size;
      err = alsamm_get_channels(in, (unsigned long *)&iframes, (unsigned long *)&ioffset,ichannels,dev->a_addr);
      if (err<0) {
        err = xrun_recovery(in, err);
        if (err<0) {check_error(err,"MMAP begins avail error"); return SENDDACS_NO;}
      }
      if(debug && dac_send < WATCH_PERIODS) {
        in_avail[dac_send] = iavail;
        in_offset[dac_send] = ioffset;
        inaddr[dac_send] = dev->a_addr[0];
      }
      /* transfer into memory */
      for (int chn=0; chn<ichannels; chn++) {
        t_alsa_sample32 *buf = (t_alsa_sample32 *) dev->a_addr[chn];
        for (i = 0, fp2 = fp1 + chn*sys_dacblocksize; i < iframes; i++,fp2++) {
            /* mask the lowest bits, since subchannels info can make zero samples nonzero */
            *fp2 = (float) (t_alsa_sample32(buf[i] & 0xFFFFFF00)) * (1.0 / float(INT32_MAX));
        }
      }
      commitres = snd_pcm_mmap_commit(in, ioffset, iframes);
      if (commitres < 0 || commitres != iframes) {
        post("please never");
        err = xrun_recovery(in, commitres >= 0 ? -EPIPE : commitres);
        if (err<0) {check_error(err,"MMAP synced in commit error"); return SENDDACS_NO;}
      }
      fp1 += iframes;
      size -= iframes;
    }
    fpi += ichannels*sys_dacblocksize;
  } /* for out devno < alsamm_outcards*/
  if ((timenow = sys_getrealtime()) > (timelast + sleep_time)) {
      if(debug && dac_send < 10 && sys_verbose)
        post("slept %f > %f + %f (=%f)", timenow,timelast,sleep_time,(timelast + sleep_time));
      return SENDDACS_SLEPT;
  }
  return SENDDACS_YES;
}

/* extra debug info */
void alsamm_showstat(snd_pcm_t *handle) {
  int err;
  snd_pcm_status_t *status;
  snd_pcm_status_alloca(&status);
  err = snd_pcm_status(handle, status);
  if (err<0) {check_error(err,"Get Stream status error"); return;}
  snd_pcm_status_dump(status, alsa_stdout);
}
