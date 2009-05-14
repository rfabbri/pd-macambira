/* Copyright (c) 1997-2003 Guenter Geiger, Miller Puckette, Larry Troxler,
* Winfried Ritsch, Karl MacMillan, and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* this file inputs and outputs audio using the OSS API available on linux. */
#include <linux/soundcard.h>

#define PD_PLUSPLUS_FACE
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
#include <sys/mman.h>

/* Defines */
#define DEBUG(x) x
#define DEBUG2(x) {x;}

#define OSS_MAXCHPERDEV 32      /* max channels per OSS device */
#define OSS_MAXDEV 4            /* maximum number of input or output devices */
#define OSS_DEFFRAGSIZE 256     /* default log fragment size (frames) */
#define OSS_DEFAUDIOBUF 40000   /* default audiobuffer, microseconds */
#define OSS_DEFAULTCH 2
#define RME_DEFAULTCH 8     /* need this even if RME undefined */
typedef int16_t t_oss_int16;
typedef int32_t t_oss_int32;
#define OSS_MAXSAMPLEWIDTH sizeof(t_oss_int32)
#define OSS_BYTESPERCHAN(width) (sys_dacblocksize * (width))
#define OSS_XFERSAMPS(chans) (sys_dacblocksize* (chans))
#define OSS_XFERSIZE(chans, width) (sys_dacblocksize * (chans) * (width))

static int linux_fragsize = 0;  /* for block mode; block size (sample frames) */

/* our device handles */
struct t_oss_dev {
    int fd;
    unsigned int space;   /* bytes available for writing/reading  */
    int bufsize;          /* total buffer size in blocks for this device */
    int dropcount;        /* # of buffers to drop for resync (output only) */
    unsigned int nchannels;   /* number of channels for this device */
    unsigned int bytespersamp; /* bytes per sample (2 for 16 bit, 4 for 32) */
};

static t_oss_dev linux_dacs[OSS_MAXDEV];
static t_oss_dev linux_adcs[OSS_MAXDEV];
static int linux_noutdevs = 0;
static int linux_nindevs = 0;

/* OSS-specific private variables */
static int oss_blockmode = 0;   /* flag to use "blockmode"  */
static int oss_32bit = 0;       /* allow 23 bit transfers in OSS  */

/* don't assume we can turn all 31 bits when doing float-to-fix;
   otherwise some audio drivers (e.g. Midiman/ALSA) wrap around. */
#define FMAX 0x7ffff000
#define CLIP32(x) (((x)>FMAX)?FMAX:((x) < -FMAX)?-FMAX:(x))

/* ---------------- public routines ----------------------- */
static int oss_ndev = 0;

/* find out how many OSS devices we have.  Since this has to
   open the devices to find out if they're there, we have
   to be called before audio is actually started up.  So we
   cache the results, which in effect are the number of available devices.  */
void oss_init() {
    static int countedthem = 0;
    if (countedthem) return;
    for (int i=0; i<10; i++) {
        char devname[100];
        if (i == 0) strcpy(devname, "/dev/dsp"); else sprintf(devname, "/dev/dsp%d", i);
        int fd = open(devname, O_WRONLY|O_NONBLOCK);
        if (fd<0) break;
        oss_ndev++;
        close(fd);
    }
    countedthem = 1;
}

void oss_set32bit() {oss_32bit=1;}

typedef struct _multidev {
     int fd;
     int channels;
     int format;
} t_multidev;

int oss_reset(int fd) {
     int err = ioctl(fd,SNDCTL_DSP_RESET);
     if (err<0) error("OSS: Could not reset");
     return err;
}

/* The AFMT_S32_BLOCKED format is not defined in standard linux kernels
   but is proposed by Guenter Geiger to support extending OSS to handle
   32 bit sample.  This is user in Geiger's OSS driver for RME Hammerfall.
   I'm not clear why this isn't called AFMT_S32_[SLN]E... */

#ifndef AFMT_S32_BLOCKED
#define AFMT_S32_BLOCKED 0x0000400
#endif

void oss_configure(t_oss_dev *dev, int srate, int dac, int skipblocksize) {
    /* IOhannes */
    int orig, param, fd = dev->fd, wantformat;
    int nchannels = dev->nchannels;
    audio_buf_info ainfo;
    /* IOhannes : pd is very likely to crash if different formats are used on multiple soundcards */
    /* set resolution - first try 4 byte samples */
    if (oss_32bit && (ioctl(fd,SNDCTL_DSP_GETFMTS,&param) >= 0) && (param & AFMT_S32_BLOCKED)) {
        wantformat = AFMT_S32_BLOCKED;
        dev->bytespersamp = 4;
    } else {
        wantformat = AFMT_S16_NE;
        dev->bytespersamp = 2;
    }
    param = wantformat;

    if (sys_verbose) post("bytes per sample = %d", dev->bytespersamp);
    if (ioctl(fd, SNDCTL_DSP_SETFMT, &param) == -1) error("OSS: Could not set DSP format");
    else if (wantformat != param) error("OSS: DSP format: wanted %d, got %d", wantformat, param);
    /* sample rate */
    orig = param = srate;
    if (ioctl(fd, SNDCTL_DSP_SPEED, &param) == -1) error("OSS: Could not set sampling rate for device");
    else if (orig != param) error("OSS: sampling rate: wanted %d, got %d", orig, param );

    if (oss_blockmode && !skipblocksize) {
        int fragbytes, logfragsize, nfragment;
        /* setting fragment count and size.  */
        linux_fragsize = sys_blocksize;
        if (!linux_fragsize) {
            linux_fragsize = OSS_DEFFRAGSIZE;
            while (linux_fragsize > sys_dacblocksize && linux_fragsize * 6 > sys_advance_samples)
                    linux_fragsize = linux_fragsize/2;
        }
        /* post("adv_samples %d", sys_advance_samples); */
        nfragment = int(sys_schedadvance * 44100.e-6 / linux_fragsize);
        fragbytes = linux_fragsize * (dev->bytespersamp * nchannels);
        logfragsize = ilog2(fragbytes);
        if (fragbytes != (1 << logfragsize))
            post("warning: OSS takes only power of 2 blocksize; using %d",
                (1 << logfragsize)/(dev->bytespersamp * nchannels));
        if (sys_verbose) post("setting nfrags = %d, fragsize %d", nfragment, fragbytes);

        param = orig = (nfragment<<16) + logfragsize;
        if (ioctl(fd,SNDCTL_DSP_SETFRAGMENT, &param) == -1)
            error("OSS: Could not set or read fragment size");
        if (param != orig) {
            nfragment = ((param >> 16) & 0xffff);
            logfragsize = (param & 0xffff);
            post("warning: actual fragments %d, blocksize %d", nfragment, 1<<logfragsize);
        }
        if (sys_verbose) post("audiobuffer set to %d msec", (int)(0.001 * sys_schedadvance));
    }

    if (dac) {
        /* use "free space" to learn the buffer size.  Normally you
        should set this to your own desired value; but this seems not
        to be implemented uniformly across different sound cards.  LATER
        we should figure out what to do if the requested scheduler advance
        is greater than this buffer size; for now, we just print something
        out.  */
        int defect;
        if (ioctl(fd, SOUND_PCM_GETOSPACE,&ainfo) < 0) error("OSS: ioctl on output device failed");
        dev->bufsize = ainfo.bytes;
        defect = sys_advance_samples * (dev->bytespersamp * nchannels)
            - dev->bufsize - OSS_XFERSIZE(nchannels, dev->bytespersamp);
        if (defect > 0) {
            if (sys_verbose || defect > (dev->bufsize >> 2))
                error("OSS: requested audio buffer size %d limited to %d",
                        sys_advance_samples * (dev->bytespersamp * nchannels), dev->bufsize);
            sys_advance_samples = (dev->bufsize-OSS_XFERSAMPS(nchannels)) / (dev->bytespersamp*nchannels);
        }
    }
}

static int oss_setchannels(int fd, int wantchannels, char *devname) {
    /* IOhannes */
    int param = wantchannels;
    while (param > 1) {
        int save = param;
        if (ioctl(fd, SNDCTL_DSP_CHANNELS, &param) == -1) error("OSS: SNDCTL_DSP_CHANNELS failed %s",devname);
        else if (param == save) return param;
        param = save - 1;
    }
    return 0;
}

#define O_AUDIOFLAG O_NDELAY
/* what's the deal with (!O_NDELAY) ? does it make sense to you? */

ssize_t read2(int fd, void *buf, size_t count) {
    ssize_t r = read(fd,buf,count);
    if (r<0) error("can't read: %s",strerror(errno));
    if (r<(ssize_t)count) error("incomplete read");
    return r;
}
ssize_t write2(int fd, const void *buf, size_t count) {
    ssize_t r = write(fd,buf,count);
    if (r<0) error("can't write: %s",strerror(errno));
    if (r<(ssize_t)count) error("incomplete write");
    return r;
}

int oss_open_audio(int nindev,  int *indev,  int nchin,  int *chin,
                   int noutdev, int *outdev, int nchout, int *chout, int rate, int bogus)
{ /* IOhannes */
    int capabilities = 0;
    int inchannels = 0, outchannels = 0;
    char devname[20];
    int n, i, fd, flags;
    char buf[OSS_MAXSAMPLEWIDTH * sys_dacblocksize * OSS_MAXCHPERDEV];
    int wantmore=0;

    linux_nindevs = linux_noutdevs = 0;
    /* mark devices unopened */
    for (int i=0; i<OSS_MAXDEV; i++) linux_adcs[i].fd = linux_dacs[i].fd = -1;

    /* open output devices */
    wantmore=0;
    if (noutdev < 0 || nindev < 0) bug("linux_open_audio");
    for (int n=0; n<noutdev; n++) {
        int gotchans, j, inindex = -1;
        int thisdevice = (outdev[n] >= 0 ? outdev[n] : 0);
        int wantchannels = (nchout>n) ? chout[n] : wantmore;
        fd = -1;
        if (!wantchannels) goto end_out_loop;
        if (thisdevice > 0) sprintf(devname, "/dev/dsp%d", thisdevice); else sprintf(devname, "/dev/dsp");
        /* search for input request for same device.  Succeed only if the number of channels matches. */
        for (j = 0; j < nindev; j++) if (indev[j] == thisdevice && chin[j] == wantchannels) inindex = j;
        /* if the same device is requested for input and output, try to open it read/write */
        if (inindex >= 0) {
            sys_setalarm(1000000);
            if ((fd = open(devname, O_RDWR | O_AUDIOFLAG)) == -1) {
                post("%s (read/write): %s", devname, strerror(errno));
                post("(now will try write-only...)");
            } else {
                if (fcntl(fd, F_SETFD, 1) < 0)                        post("couldn't set close-on-exec flag on audio");
                if ((flags = fcntl(fd, F_GETFL)) < 0)                 post("couldn't get audio device flags");
                else if (fcntl(fd, F_SETFL, flags & (!O_NDELAY)) < 0) post("couldn't set audio device flags");
                if (sys_verbose) post("opened %s for reading and writing", devname);
                linux_adcs[inindex].fd = fd;
            }
        }
        /* if that didn't happen or if it failed, try write-only */
        if (fd == -1) {
            sys_setalarm(1000000);
            if ((fd = open(devname, O_WRONLY | O_AUDIOFLAG)) == -1) {
                post("%s (writeonly): %s", devname, strerror(errno));
                break;
            }
            if (fcntl(fd, F_SETFD, 1) < 0)                        post("couldn't set close-on-exec flag on audio");
            if ((flags = fcntl(fd, F_GETFL)) < 0)                 post("couldn't get audio device flags");
            else if (fcntl(fd, F_SETFL, flags & (!O_NDELAY)) < 0) post("couldn't set audio device flags");
            if (sys_verbose) post("opened %s for writing only", devname);
        }
        if (ioctl(fd, SNDCTL_DSP_GETCAPS, &capabilities) == -1) error("OSS: SNDCTL_DSP_GETCAPS failed %s", devname);
        gotchans = oss_setchannels(fd, (wantchannels>OSS_MAXCHPERDEV)?OSS_MAXCHPERDEV:wantchannels, devname);
        if (sys_verbose) post("opened audio output on %s; got %d channels", devname, gotchans);
        if (gotchans < 2) {
            close(fd); /* can't even do stereo? just give up. */
        } else {
            linux_dacs[linux_noutdevs].nchannels = gotchans;
            linux_dacs[linux_noutdevs].fd = fd;
            oss_configure(&linux_dacs[linux_noutdevs], rate, 1, 0);
            linux_noutdevs++;
            outchannels += gotchans;
            if (inindex >= 0) {
                linux_adcs[inindex].nchannels = gotchans;
                chin[inindex] = gotchans;
            }
        }
        /* LATER think about spreading large numbers of channels over
            various dsp's and vice-versa */
        wantmore = wantchannels - gotchans;
    end_out_loop: ;
    }

    /* open input devices */
    wantmore = 0;
    for (n = 0; n < nindev; n++) {
        int gotchans=0;
        int thisdevice = (indev[n] >= 0 ? indev[n] : 0);
        int wantchannels = (nchin>n)?chin[n]:wantmore;
        int alreadyopened = 0;
        if (!wantchannels) goto end_in_loop;
        if (thisdevice > 0) sprintf(devname, "/dev/dsp%d", thisdevice); else sprintf(devname, "/dev/dsp");
        sys_setalarm(1000000);
        /* perhaps it's already open from the above? */
        if (linux_dacs[n].fd >= 0) {
            fd = linux_adcs[n].fd;
            alreadyopened = 1;
        } else {
            /* otherwise try to open it here. */
            if ((fd = open(devname, O_RDONLY | O_AUDIOFLAG)) == -1) {
                post("%s (readonly): %s", devname, strerror(errno));
                goto end_in_loop;
            }
            if (fcntl(fd, F_SETFD, 1) < 0) post("couldn't set close-on-exec flag on audio");
            if ((flags = fcntl(fd, F_GETFL)) < 0) post("couldn't get audio device flags");
            else if (fcntl(fd, F_SETFL, flags & (!O_NDELAY)) < 0) post("couldn't set audio device flags");
            if (sys_verbose) post("opened %s for reading only", devname);
        }
        linux_adcs[linux_nindevs].fd = fd;
        gotchans = oss_setchannels(fd, (wantchannels>OSS_MAXCHPERDEV)?OSS_MAXCHPERDEV:wantchannels, devname);
        if (sys_verbose) post("opened audio input device %s; got %d channels", devname, gotchans);
        if (gotchans < 1) {
            close(fd);
            goto end_in_loop;
        }
        linux_adcs[linux_nindevs].nchannels = gotchans;
        oss_configure(linux_adcs+linux_nindevs, rate, 0, alreadyopened);
        inchannels += gotchans;
        linux_nindevs++;
        wantmore = wantchannels-gotchans;
        /* LATER think about spreading large numbers of channels over various dsp's and vice-versa */
    end_in_loop: ;
    }
    /* We have to do a read to start the engine. This is necessary because sys_send_dacs waits until the input
       buffer is filled and only reads on a filled buffer. This is good, because it's a way to make sure that we
       will not block.  But I wonder why we only have to read from one of the devices and not all of them??? */
    if (linux_nindevs) {
        if (sys_verbose) post("OSS: issuing first ADC 'read'...");
        read2(linux_adcs[0].fd, buf, linux_adcs[0].bytespersamp * linux_adcs[0].nchannels * sys_dacblocksize);
        if (sys_verbose) post("...done.");
    }
    /* now go and fill all the output buffers. */
    for (i = 0; i < linux_noutdevs; i++) {
	t_oss_dev &d = linux_dacs[i];
        memset(buf, 0, d.bytespersamp * d.nchannels * sys_dacblocksize);
        for (int j = 0; j < sys_advance_samples/sys_dacblocksize; j++)
            write2(d.fd, buf, d.bytespersamp * d.nchannels * sys_dacblocksize);
    }
    sys_setalarm(0);
    sys_inchannels = inchannels;
    sys_outchannels = outchannels;
    return 0;
}

void oss_close_audio() {
    for (int i=0;i<linux_nindevs ;i++) close(linux_adcs[i].fd);
    for (int i=0;i<linux_noutdevs;i++) close(linux_dacs[i].fd);
    linux_nindevs = linux_noutdevs = 0;
}

static int linux_dacs_write(int fd,void *buf,long bytes) {return write(fd, buf, bytes);}
static int linux_adcs_read (int fd,void *buf,long bytes) {return  read(fd, buf, bytes);}

    /* query audio devices for "available" data size. */
static void oss_calcspace() {
    audio_buf_info ainfo;
    for (int dev=0; dev<linux_noutdevs; dev++) {
        if (ioctl(linux_dacs[dev].fd, SOUND_PCM_GETOSPACE,&ainfo) < 0)
           error("OSS: ioctl on output device %d failed",dev);
        linux_dacs[dev].space = ainfo.bytes;
    }
    for (int dev=0; dev<linux_nindevs; dev++) {
        if (ioctl(linux_adcs[dev].fd, SOUND_PCM_GETISPACE,&ainfo) < 0)
            error("OSS: ioctl on input device %d, fd %d failed", dev, linux_adcs[dev].fd);
        linux_adcs[dev].space = ainfo.bytes;
    }
}

void linux_audiostatus() {
    if (!oss_blockmode) {
        oss_calcspace();
        for (int dev=0; dev < linux_noutdevs; dev++) post("dac %d space %d", dev, linux_dacs[dev].space);
        for (int dev=0; dev < linux_nindevs;  dev++) post("adc %d space %d", dev, linux_adcs[dev].space);
    }
}

/* this call resyncs audio output and input which will cause discontinuities
in audio output and/or input. */

static void oss_doresync() {
    int zeroed = 0;
    char buf[OSS_MAXSAMPLEWIDTH * sys_dacblocksize * OSS_MAXCHPERDEV];
    audio_buf_info ainfo;
    /* 1. if any input devices are ahead (have more than 1 buffer stored), drop one or more buffers worth */
    for (int dev=0; dev<linux_nindevs; dev++) {
	t_oss_dev d = linux_adcs[dev];
        if (d.space == 0) {
            linux_adcs_read(d.fd, buf, OSS_XFERSIZE(d.nchannels, d.bytespersamp));
        } else while (d.space > OSS_XFERSIZE(d.nchannels, d.bytespersamp)) {
            linux_adcs_read(d.fd, buf, OSS_XFERSIZE(d.nchannels, d.bytespersamp));
            if (ioctl(d.fd, SOUND_PCM_GETISPACE, &ainfo) < 0) { error("OSS: ioctl on input device %d, fd %d failed",dev,d.fd); break;}
            d.space = ainfo.bytes;
        }
    }
    /* 2. if any output devices are behind, feed them zeros to catch them up */
    for (int dev=0; dev<linux_noutdevs; dev++) {
	t_oss_dev d = linux_dacs[dev];
        while (d.space > d.bufsize - sys_advance_samples*d.nchannels*d.bytespersamp) {
            if (!zeroed) {
                for (unsigned int i = 0; i < OSS_XFERSAMPS(d.nchannels); i++) buf[i] = 0;
                zeroed = 1;
            }
            linux_dacs_write(d.fd, buf, OSS_XFERSIZE(d.nchannels, d.bytespersamp));
            if (ioctl(d.fd, SOUND_PCM_GETOSPACE, &ainfo) < 0) {error("OSS: ioctl on output device %d, fd %d failed",dev,d.fd); break;}
            d.space = ainfo.bytes;
        }
    }
    /* 3. if any DAC devices are too far ahead, plan to drop the number of frames which will let the others catch up. */
    for (int dev=0; dev<linux_noutdevs; dev++) {
	t_oss_dev d = linux_dacs[dev];
        if (d.space > d.bufsize - (sys_advance_samples - 1) * d.nchannels * d.bytespersamp) {
            d.dropcount = sys_advance_samples - 1 - (d.space - d.bufsize) / (d.nchannels * d.bytespersamp);
        } else d.dropcount = 0;
    }
}

int oss_send_dacs() {
    float *fp1, *fp2;
    int i, j, rtnval = SENDDACS_YES;
    char buf[OSS_MAXSAMPLEWIDTH * sys_dacblocksize * OSS_MAXCHPERDEV];
    t_oss_int16 *sp;
    t_oss_int32 *lp;
    /* the maximum number of samples we should have in the ADC buffer */
    int idle = 0;
    double timeref, timenow;
    if (!linux_nindevs && !linux_noutdevs) return SENDDACS_NO;
    if (!oss_blockmode) {
        /* determine whether we're idle.  This is true if either (1)
           some input device has less than one buffer to read or (2) some
           output device has fewer than (sys_advance_samples) blocks buffered already. */
        oss_calcspace();
        for (int dev=0; dev<linux_noutdevs; dev++) {
	    t_oss_dev d = linux_dacs[dev];
            if (d.dropcount || (d.bufsize - d.space > sys_advance_samples * d.bytespersamp * d.nchannels)) idle = 1;
	}
        for (int dev=0; dev<linux_nindevs; dev++) {
	    t_oss_dev d = linux_adcs[dev];
            if (d.space < OSS_XFERSIZE(d.nchannels, d.bytespersamp)) idle = 1;
	}
    }
    if (idle && !oss_blockmode) {
        /* sometimes---rarely---when the ADC available-byte-count is
           zero, it's genuine, but usually it's because we're so
           late that the ADC has overrun its entire kernel buffer.  We
           distinguish between the two by waiting 2 msec and asking again.
           There should be an error flag we could check instead; look for this someday... */
        for (int dev=0; dev<linux_nindevs; dev++) if (linux_adcs[dev].space == 0) {
            sys_microsleep(sys_sleepgrain); /* tb: changed to sys_sleepgrain */
            oss_calcspace();
            if (linux_adcs[dev].space != 0) continue;
            /* here's the bad case.  Give up and resync. */
            sys_log_error(ERR_DATALATE);
            oss_doresync();
            return SENDDACS_NO;
        }
        /* check for slippage between devices, either because
           data got lost in the driver from a previous late condition, or
           because the devices aren't synced.  When we're idle, no
           input device should have more than one buffer readable and
           no output device should have less than sys_advance_samples-1 */
        for (int dev=0; dev<linux_noutdevs; dev++) {
	    t_oss_dev d = linux_dacs[dev];
            if (!d.dropcount && (d.bufsize - d.space < (sys_advance_samples - 2)*d.bytespersamp*d.nchannels))
                        goto badsync;
	}
        for (int dev=0; dev<linux_nindevs; dev++)
            if (linux_adcs[dev].space > 3 * OSS_XFERSIZE(linux_adcs[dev].nchannels, linux_adcs[dev].bytespersamp)) goto badsync;
        /* return zero to tell the scheduler we're idle. */
        return SENDDACS_NO;
    badsync:
        sys_log_error(ERR_RESYNC);
        oss_doresync();
        return SENDDACS_NO;
    }
    /* do output */
    timeref = sys_getrealtime();
    for (int dev=0, thischan = 0; dev < linux_noutdevs; dev++) {
	t_oss_dev d = linux_dacs[dev];
        int nchannels = d.nchannels;
        if (d.dropcount) d.dropcount--;
        else {
	    fp1 = sys_soundout + sys_dacblocksize*thischan;
            if (d.bytespersamp == 4) {
                for (i = sys_dacblocksize * nchannels, lp = (t_oss_int32 *)buf; i--; fp1++, lp++) {
                    float f = *fp1 * 2147483648.;
                    *lp = int(f >= 2147483647. ? 2147483647. : (f < -2147483648. ? -2147483648. : f));
                }
            } else {
                for (i = sys_dacblocksize, sp = (t_oss_int16 *)buf; i--; fp1++, sp += nchannels) {
                    for (j=0, fp2 = fp1; j<nchannels; j++, fp2 += sys_dacblocksize) {
                        int s = int(*fp2 * 32767.);
                        if (s > 32767) s = 32767; else if (s < -32767) s = -32767;
                        sp[j] = s;
                    }
                }
            }
            linux_dacs_write(d.fd, buf, OSS_XFERSIZE(nchannels, d.bytespersamp));
            if ((timenow = sys_getrealtime()) - timeref > 0.002) {
                if (!oss_blockmode) sys_log_error(ERR_DACSLEPT); else rtnval = SENDDACS_SLEPT;
            }
            timeref = timenow;
        }
        thischan += nchannels;
    }
    memset(sys_soundout, 0, sys_outchannels * (sizeof(float) * sys_dacblocksize));
    for (int dev=0, thischan = 0; dev < linux_nindevs; dev++) {
        int nchannels = linux_adcs[dev].nchannels;
        linux_adcs_read(linux_adcs[dev].fd, buf, OSS_XFERSIZE(nchannels, linux_adcs[dev].bytespersamp));
        if ((timenow = sys_getrealtime()) - timeref > 0.002) {
            if (!oss_blockmode) sys_log_error(ERR_ADCSLEPT); else rtnval = SENDDACS_SLEPT;
        }
        timeref = timenow;
	fp1 = sys_soundin + thischan*sys_dacblocksize;
        if (linux_adcs[dev].bytespersamp == 4) {
            for (i = sys_dacblocksize*nchannels, lp = (t_oss_int32 *)buf; i--; fp1++, lp++)
                *fp1 = float(*lp)*float(1./2147483648.);
        } else {
            for (i = sys_dacblocksize, sp = (t_oss_int16 *)buf; i--; fp1++, sp += nchannels)
                for (j=0; j<nchannels; j++) fp1[j*sys_dacblocksize] = (float)sp[j]*(float)3.051850e-05;
        }
        thischan += nchannels;
     }
     return rtnval;
}

void oss_getdevs(char *indevlist, int *nindevs, char *outdevlist, int *noutdevs, int *canmulti, int maxndev, int devdescsize) {
    int ndev = min(oss_ndev,maxndev);
    *canmulti = 2;  /* supports multiple devices */
    for (int i=0; i<ndev; i++) {
        sprintf(indevlist  + i*devdescsize, "OSS device #%d", i+1);
        sprintf(outdevlist + i*devdescsize, "OSS device #%d", i+1);
    }
    *nindevs = *noutdevs = ndev;
}

struct t_audioapi oss_api = {
	oss_open_audio,
	oss_close_audio,
	oss_send_dacs,
	oss_getdevs,
};
