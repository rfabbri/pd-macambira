/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* this file contains, first, a collection of soundfile access routines, a
sort of soundfile library.  Second, the "soundfiler" object is defined which
uses the routines to read or write soundfiles, synchronously, from garrays.
These operations are not to be done in "real time" as they may have to wait
for disk accesses (even the write routine.)  Finally, the realtime objects
readsf~ and writesf~ are defined which confine disk operations to a separate
thread so that they can be used in real time.  The readsf~ and writesf~
objects use Posix-like threads.  */

/* threaded soundfiler by Tim Blechmann */
// #define THREADED_SF

#ifndef MSW
#include <unistd.h>
#include <fcntl.h>
#endif
#include <pthread.h>
#ifdef MSW
#include <io.h>
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define PD_PLUSPLUS_FACE
#include "desire.h"
using namespace desire;
#define a_symbol a_w.w_symbol
#define a_float a_w.w_float

#define MAXSFCHANS 256

#ifdef _LARGEFILE64_SOURCE
# define open open64
# define lseek lseek64
#endif

#if 0
static bool debug=0;
#endif

#define EAT_ARG(ATYPE,VAR) if (argc<1 || argv->a_type != ATYPE) goto usage; else {VAR = *argv++; argc--;}

/***************** soundfile header structures ************************/

typedef unsigned short uint16;
typedef unsigned int uint32; /* long isn't 32-bit on amd64 */

#define FORMAT_WAVE 0
#define FORMAT_AIFF 1
#define FORMAT_NEXT 2

/* the NeXTStep sound header structure; can be big or little endian  */

struct t_nextstep {
    char fileid[4];   /* magic number '.snd' if file is big-endian */
    uint32 onset;        /* byte offset of first sample */
    uint32 length;       /* length of sound in bytes */
    uint32 format;       /* format; see below */
    uint32 sr;           /* sample rate */
    uint32 nchans;       /* number of channels */
    char info[4];        /* comment */
};

#define NS_FORMAT_LINEAR_16     3
#define NS_FORMAT_LINEAR_24     4
#define NS_FORMAT_FLOAT         6
#define SCALE (1./(1024. * 1024. * 1024. * 2.))

/* the WAVE header.  All Wave files are little endian.  We assume
    the "fmt" chunk comes first which is usually the case but perhaps not
    always; same for AIFF and the "COMM" chunk.   */

struct t_wave {
    char  fileid[4];        /* chunk id 'RIFF'            */
    uint32 chunksize;       /* chunk size                 */
    char  waveid[4];        /* wave chunk id 'WAVE'       */
    char  fmtid[4];         /* format chunk id 'fmt '     */
    uint32 fmtchunksize;    /* format chunk size          */
    uint16  fmttag;         /* format tag (WAV_INT etc)   */
    uint16  nchannels;      /* number of channels         */
    uint32 samplespersec;   /* sample rate in hz          */
    uint32 navgbytespersec; /* average bytes per second   */
    uint16  nblockalign;    /* number of bytes per frame  */
    uint16  nbitspersample; /* number of bits in a sample */
    char  datachunkid[4];   /* data chunk id 'data'       */
    uint32 datachunksize;   /* length of data chunk       */
};

struct t_fmt { /* format chunk */
    uint16 fmttag;          /* format tag, 1 for PCM      */
    uint16 nchannels;       /* number of channels         */
    uint32 samplespersec;   /* sample rate in hz          */
    uint32 navgbytespersec; /* average bytes per second   */
    uint16 nblockalign;     /* number of bytes per frame  */
    uint16 nbitspersample;  /* number of bits in a sample */
};

struct t_wavechunk { /* ... and the last two items */
    char  id[4];            /* data chunk id, e.g., 'data' or 'fmt ' */
    uint32 size;            /* length of data chunk       */
};

#define WAV_INT 1
#define WAV_FLOAT 3

typedef unsigned char byte;

/* the AIFF header.  I'm assuming AIFC is compatible but don't really know that. */

struct t_datachunk {
    char  id[4];        // data chunk id 'SSND'
    uint32 size;        // length of data chunk
    uint32 offset;      // additional offset in bytes
    uint32 block;       // block size
};

struct t_comm {
    uint16 nchannels;   // number of channels
    uint16 nframeshi;   // # of sample frames (hi)
    uint16 nframeslo;   // # of sample frames (lo)
    uint16 bitspersamp; // bits per sample
    byte   samprate[10];// sample rate, 80-bit float!
};

/* this version is more convenient for writing them out: */
struct t_aiff {
    char   fileid[4];    // chunk id 'FORM'
    uint32 chunksize;    // chunk size
    char   aiffid[4];    // aiff chunk id 'AIFF'
    char   fmtid[4];     // format chunk id 'COMM'
    uint32 fmtchunksize; // format chunk size, 18
    uint16 nchannels;    // number of channels
    uint16 nframeshi;    // # of sample frames (hi)
    uint16 nframeslo;    // # of sample frames (lo)
    uint16 bitspersamp;  // bits per sample
    byte   samprate[10]; // sample rate, 80-bit float!
};

struct t_param {
    int bytespersample;
    int bigendian;
    int nchannels;
    long bytelimit;
    int bytesperchannel() {return bytespersample * nchannels;}
};

#define AIFFHDRSIZE 38      /* probably not what sizeof() gives */
#define AIFFPLUS (AIFFHDRSIZE + 16)  /* header size including SSND chunk hdr */
#define WHDR1 sizeof(t_nextstep)
#define WHDR2 (sizeof(t_wave) > WHDR1 ? sizeof (t_wave) : WHDR1)
#define WRITEHDRSIZE (AIFFPLUS > WHDR2 ? AIFFPLUS : WHDR2)
#define READHDRSIZE (16 > WHDR2 + 2 ? 16 : WHDR2 + 2)

#ifdef MSW
#include <fcntl.h>
#define BINCREATE (_O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY)
#else
#define BINCREATE (O_WRONLY | O_CREAT | O_TRUNC)
#endif

/* this routine returns 1 if the high order byte comes at the lower
address on our architecture (big-endianness.).  It's 1 for Motorola, 0 for Intel: */

extern int garray_ambigendian();

/* byte swappers */

static uint32 swap4(uint32 n, int doit) {
    if (doit) return ((n & 0xff) << 24) | ((n & 0xff00) << 8) | ((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24);
    else return n;
}

static uint16 swap2(uint32 n, int doit) {
    if (doit) return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
    else return n;
}

static void swapstring(char *foo, int doit) {
    if (doit) {
        char a = foo[0], b = foo[1], c = foo[2], d = foo[3];
        foo[0] = d; foo[1] = c; foo[2] = b; foo[3] = a;
    }
}

/******************** soundfile access routines **********************/
/* This routine opens a file, looks for either a nextstep or "wave" header,
* seeks to end of it, and fills in bytes per sample and number of channels.
* Only 2- and 3-byte fixed-point samples and 4-byte floating point samples
* are supported.  If "headersize" is nonzero, the
* caller should supply the number of channels, endinanness, and bytes per
* sample; the header is ignored.  Otherwise, the routine tries to read the
* header and fill in the properties.
*/

int open_soundfile_via_fd(int fd, int headersize, t_param *p, long skipframes) {
    int swap, sysrtn;
    errno = 0;
    t_param q;
    q.bytelimit = 0x7fffffff;
    if (headersize >= 0) { /* header detection overridden */
        q = *p;
    } else {
        char buf[MAXPDSTRING];
        int bytesread = read(fd, buf, READHDRSIZE);
        int format;
        if (bytesread < 4) goto badheader;
        if      (!strncmp(buf, ".snd", 4)) {format = FORMAT_NEXT; q.bigendian = 1;}
        else if (!strncmp(buf, "dns.", 4)) {format = FORMAT_NEXT; q.bigendian = 0;}
        else if (!strncmp(buf, "RIFF", 4)) {
            if (bytesread < 12 || strncmp(buf + 8, "WAVE", 4)) goto badheader;
            format = FORMAT_WAVE; q.bigendian = 0;
        }
        else if (!strncmp(buf, "FORM", 4)) {
            if (bytesread < 12 || strncmp(buf + 8, "AIFF", 4)) goto badheader;
            format = FORMAT_AIFF; q.bigendian = 1;
        } else goto badheader;
        swap = (q.bigendian != garray_ambigendian());
        if (format == FORMAT_NEXT) {  /* nextstep header */
            if (bytesread < (int)sizeof(t_nextstep)) goto badheader;
            q.nchannels = swap4(((t_nextstep *)buf)->nchans, swap);
            format      = swap4(((t_nextstep *)buf)->format, swap);
            headersize  = swap4(((t_nextstep *)buf)->onset,  swap);
            if      (format == NS_FORMAT_LINEAR_16) q.bytespersample = 2;
            else if (format == NS_FORMAT_LINEAR_24) q.bytespersample = 3;
            else if (format == NS_FORMAT_FLOAT)     q.bytespersample = 4;
            else goto badheader;
            q.bytelimit = 0x7fffffff;
        } else if (format == FORMAT_WAVE) {    /* wave header */
            /* This is awful.  You have to skip over chunks,
               except that if one happens to be a "fmt" chunk, you want to
               find out the format from that one.  The case where the
               "fmt" chunk comes after the audio isn't handled. */
            headersize = 12;
            if (bytesread < 20) goto badheader;
            /* First we guess a number of channels, etc., in case there's
               no "fmt" chunk to follow. */
            q.nchannels = 1;
            q.bytespersample = 2;
            /* copy the first chunk header to beginnning of buffer. */
            memcpy(buf, buf + headersize, sizeof(t_wavechunk));
            /* read chunks in loop until we get to the data chunk */
            while (strncmp(((t_wavechunk *)buf)->id, "data", 4)) {
                long chunksize = swap4(((t_wavechunk *)buf)->size, swap), seekto = headersize + chunksize + 8, seekout;
                if (!strncmp(((t_wavechunk *)buf)->id, "fmt ", 4)) {
                    long commblockonset = headersize + 8;
                    seekout = lseek(fd, commblockonset, SEEK_SET);
                    if (seekout != commblockonset) goto badheader;
                    if (read(fd, buf, sizeof(t_fmt)) < (int) sizeof(t_fmt)) goto badheader;
                    q.nchannels = swap2(((t_fmt *)buf)->nchannels, swap);
                    format = swap2(((t_fmt *)buf)->nbitspersample, swap);
                    if (format == 16)      q.bytespersample = 2;
                    else if (format == 24) q.bytespersample = 3;
                    else if (format == 32) q.bytespersample = 4;
                    else goto badheader;
                }
                seekout = lseek(fd, seekto, SEEK_SET);
                if (seekout != seekto) goto badheader;
                if (read(fd, buf, sizeof(t_wavechunk)) < (int) sizeof(t_wavechunk)) goto badheader;
                headersize = seekto;
            }
            q.bytelimit = swap4(((t_wavechunk *)buf)->size, swap);
            headersize += 8;
        } else {
            /* AIFF.  same as WAVE; actually predates it.  Disgusting. */
            headersize = 12;
            if (bytesread < 20) goto badheader;
            /* First we guess a number of channels, etc., in case there's no COMM block to follow. */
            q.nchannels = 1;
            q.bytespersample = 2;
            /* copy the first chunk header to beginnning of buffer. */
            memcpy(buf, buf + headersize, sizeof(t_datachunk));
            /* read chunks in loop until we get to the data chunk */
            while (strncmp(((t_datachunk *)buf)->id, "SSND", 4)) {
                long chunksize = swap4(((t_datachunk *)buf)->size, swap), seekto = headersize + chunksize + 8, seekout;
                if (!strncmp(((t_datachunk *)buf)->id, "COMM", 4)) {
                    long commblockonset = headersize + 8;
                    seekout = lseek(fd, commblockonset, SEEK_SET);
                    if (seekout != commblockonset) goto badheader;
                    if (read(fd, buf, sizeof(t_comm)) < (int) sizeof(t_comm)) goto badheader;
                    q.nchannels = swap2(((t_comm *)buf)->nchannels, swap);
                    format = swap2(((t_comm *)buf)->bitspersamp, swap);
                    if      (format == 16) q.bytespersample = 2;
                    else if (format == 24) q.bytespersample = 3;
                    else goto badheader;
                }
                seekout = lseek(fd, seekto, SEEK_SET);
                if (seekout != seekto) goto badheader;
                if (read(fd, buf, sizeof(t_datachunk)) < (int) sizeof(t_datachunk)) goto badheader;
                headersize = seekto;
            }
            q.bytelimit = swap4(((t_datachunk *)buf)->size, swap);
            headersize += 8;
        }
    }
    /* seek past header and any sample frames to skip */
    sysrtn = lseek(fd, q.bytesperchannel() * skipframes + headersize, 0);
    if (sysrtn != q.bytesperchannel() * skipframes + headersize) return -1;
    q.bytelimit -= q.bytesperchannel() * skipframes;
    if (q.bytelimit < 0) q.bytelimit = 0;
    *p = q;
    return fd;
badheader:
    /* the header wasn't recognized.  We're threadable here so let's not print out the error... */
    errno = EIO;
    return -1;
}

/* open a soundfile, using open_via_path().  This is used by readsf~ in
   a not-perfectly-threadsafe way.  LATER replace with a thread-hardened version of open_soundfile_via_canvas() */
static int open_soundfile(const char *dirname, const char *filename, int headersize, t_param *p, long skipframes) {
    char *buf, *bufptr;
    int fd = open_via_path2(dirname, filename, "", &buf, &bufptr, 1);
    if (fd < 0) return -1;
    free(buf);
    return open_soundfile_via_fd(fd, headersize, p, skipframes);
}

/* open a soundfile, using open_via_canvas().  This is used by readsf~ in
   a not-perfectly-threadsafe way.  LATER replace with a thread-hardened version of open_soundfile_via_canvas() */
static int open_soundfile_via_canvas(t_canvas *canvas, const char *filename, int headersize, t_param *p, long skipframes) {
    char *buf, *bufptr;
    int fd = canvas_open2(canvas, filename, "", &buf, &bufptr, 1);
    if (fd < 0) return -1;
    free(buf);
    return open_soundfile_via_fd(fd, headersize, p, skipframes);
}

static void soundfile_xferin(int sfchannels, int nvecs, float **vecs,
    long itemsread, unsigned char *buf, int nitems, int bytespersample, int bigendian) {
    unsigned char *sp, *sp2;
    int nchannels = (sfchannels < nvecs ? sfchannels : nvecs);
    int bytesperframe = bytespersample * sfchannels;
    sp = buf;
    for (int i=0; i < nchannels; i++, sp += bytespersample) {
        int j;
        sp2=sp;
        float *fp=vecs[i] + itemsread;
	#define LOOP for (j=0; j<nitems; j++, sp2 += bytesperframe, fp++)
        if (bytespersample == 2) {
            if (bigendian) LOOP {*fp = SCALE * ((sp2[0]<<24) | (sp2[1]<<16));}
            else           LOOP {*fp = SCALE * ((sp2[1]<<24) | (sp2[0]<<16));}
        } else if (bytespersample == 3) {
            if (bigendian) LOOP {*fp = SCALE * ((sp2[0]<<24) | (sp2[1]<<16) | (sp2[2]<<8));}
            else           LOOP {*fp = SCALE * ((sp2[2]<<24) | (sp2[1]<<16) | (sp2[0]<<8));}
        } else if (bytespersample == 4) {
            if (bigendian) LOOP {*(long *)fp = (sp2[0]<<24) | (sp2[1]<<16) | (sp2[2]<<8) | sp2[3];}
            else           LOOP {*(long *)fp = (sp2[3]<<24) | (sp2[2]<<16) | (sp2[1]<<8) | sp2[0];}
        }
	#undef LOOP
    }
    /* zero out other outputs */
    for (int i=sfchannels; i < nvecs; i++) {
        float *fp=vecs[i];
        for (int j=nitems; j--; ) *fp++ = 0;
    }
}

/* soundfiler_write ...
   usage: write [flags] filename table ...
   flags: -nframes <frames> -skip <frames> -bytes <bytes per sample> -normalize -nextstep -wave -big -little
    the routine which actually does the work should LATER also be called from garray_write16.
    Parse arguments for writing.  The "obj" argument is only for flagging
    errors.  For streaming to a file the "normalize", "onset" and "nframes"
    arguments shouldn't be set but the calling routine flags this. */
static int soundfiler_writeargparse(void *obj, int *p_argc, t_atom **p_argv, t_symbol **p_filesym,
int *p_filetype, int *p_bytespersamp, int *p_swap, int *p_bigendian,
int *p_normalize, long *p_onset, long *p_nframes, float *p_rate) {
    int argc = *p_argc;
    t_atom *argv = *p_argv;
    int bytespersample = 2, bigendian = 0, endianness = -1, swap, filetype = -1, normalize = 0;
    long onset = 0, nframes = 0x7fffffff;
    t_symbol *filesym;
    float rate = -1;
    while (argc > 0 && argv->a_type == A_SYMBOL && *argv->a_symbol->name == '-') {
        char *flag = argv->a_symbol->name + 1;
	argc--; argv++;
        if (!strcmp(flag, "skip")) {
            EAT_ARG(A_FLOAT,onset); if (onset<0) goto usage;
        } else if (!strcmp(flag, "nframes")) {
            EAT_ARG(A_FLOAT,nframes); if (nframes<0) goto usage;
        } else if (!strcmp(flag, "bytes")) {
            EAT_ARG(A_FLOAT,bytespersample); if (bytespersample<2 || bytespersample>4) goto usage;
        } else if (!strcmp(flag, "normalize")) {normalize = 1;
	} else if (!strcmp(flag, "wave"))      {filetype = FORMAT_WAVE;
        } else if (!strcmp(flag, "nextstep"))  {filetype = FORMAT_NEXT;
        } else if (!strcmp(flag, "aiff"))      {filetype = FORMAT_AIFF;
        } else if (!strcmp(flag, "big"))       {endianness = 1;
        } else if (!strcmp(flag, "little"))    {endianness = 0;
        } else if (!strcmp(flag, "r") || !strcmp(flag, "rate")) {
            EAT_ARG(A_FLOAT,rate); if (rate<0) goto usage;
        } else goto usage;
    }
    if (!argc || argv->a_type != A_SYMBOL) goto usage;
    filesym = argv->a_symbol;
    /* check if format not specified and fill in */
    if (filetype < 0) {
	const char *s = filesym->name + strlen(filesym->name);
        if (strlen(filesym->name) >= 5 && !strcasecmp(s-4, ".aif" )) filetype = FORMAT_AIFF;
        if (strlen(filesym->name) >= 6 && !strcasecmp(s-5, ".aiff")) filetype = FORMAT_AIFF;
        if (strlen(filesym->name) >= 5 && !strcasecmp(s-4, ".snd" )) filetype = FORMAT_NEXT;
        if (strlen(filesym->name) >= 4 && !strcasecmp(s-3, ".au"  )) filetype = FORMAT_NEXT;
        if (filetype < 0) filetype = FORMAT_WAVE;
    }
    /* don't handle AIFF floating point samples */
    if (bytespersample == 4) {
        if (filetype == FORMAT_AIFF) {
            error("AIFF floating-point file format unavailable");
            goto usage;
        }
    }
    /* for WAVE force little endian; for nextstep use machine native */
    if (filetype == FORMAT_WAVE) {
        bigendian = 0;
        if (endianness == 1) error("WAVE file forced to little endian");
    } else if (filetype == FORMAT_AIFF) {
        bigendian = 1;
        if (endianness == 0) error("AIFF file forced to big endian");
    } else if (endianness == -1) {
        bigendian = garray_ambigendian();
    } else bigendian = endianness;
    swap = (bigendian != garray_ambigendian());
    argc--; argv++;
    *p_argc = argc;
    *p_argv = argv;
    *p_filesym = filesym;
    *p_filetype = filetype;
    *p_bytespersamp = bytespersample;
    *p_swap = swap;
    *p_normalize = normalize;
    *p_onset = onset;
    *p_nframes = nframes;
    *p_bigendian = bigendian;
    *p_rate = rate;
    return 0;
usage:
    return -1;
}

static bool strcaseends(const char *a, const char *b) {return strcasecmp(a+strlen(a)-strlen(b),b)==0;}

static int create_soundfile(t_canvas *canvas, const char *filename, int filetype, int nframes, int bytespersample,
int bigendian, int nchannels, int swap, float samplerate) {
    char filenamebuf[strlen(filename)+10];
    char headerbuf[WRITEHDRSIZE];
    int fd, headersize = 0;
    strcpy(filenamebuf, filename);
    if (filetype == FORMAT_NEXT) {
        t_nextstep *nexthdr = (t_nextstep *)headerbuf;
        if (!strcaseends(filenamebuf,".snd")) strcat(filenamebuf, ".snd");
        if (bigendian) strncpy(nexthdr->fileid, bigendian?".snd":"dns.", 4);
        nexthdr->onset = swap4(sizeof(*nexthdr), swap);
        nexthdr->length = 0;
        nexthdr->format = swap4(bytespersample == 3 ? NS_FORMAT_LINEAR_24 : bytespersample == 4 ? NS_FORMAT_FLOAT : NS_FORMAT_LINEAR_16, swap);
        nexthdr->sr = swap4((size_t)samplerate, swap);
        nexthdr->nchans = swap4((size_t)nchannels, swap);
        strcpy(nexthdr->info, "Pd ");
        swapstring(nexthdr->info, swap);
        headersize = sizeof(t_nextstep);
    } else if (filetype == FORMAT_AIFF) {
        long datasize = nframes * nchannels * bytespersample;
        long longtmp;
        static unsigned char dogdoo[] = {0x40, 0x0e, 0xac, 0x44, 0, 0, 0, 0, 0, 0, 'S', 'S', 'N', 'D'};
        t_aiff *aiffhdr = (t_aiff *)headerbuf;
        if (!strcaseends(filenamebuf,".aif") && !strcaseends(filenamebuf,".aiff")) strcat(filenamebuf, ".aif");
        strncpy(aiffhdr->fileid, "FORM", 4);
        aiffhdr->chunksize = swap4(datasize + sizeof(*aiffhdr) + 4, swap);
        strncpy(aiffhdr->aiffid, "AIFF", 4);
        strncpy(aiffhdr->fmtid, "COMM", 4);
        aiffhdr->fmtchunksize = swap4(18, swap);
        aiffhdr->nchannels = swap2(nchannels, swap);
        longtmp = swap4(nframes, swap);
        memcpy(&aiffhdr->nframeshi, &longtmp, 4);
        aiffhdr->bitspersamp = swap2(8 * bytespersample, swap);
        memcpy(aiffhdr->samprate, dogdoo, sizeof(dogdoo));
        longtmp = swap4(datasize, swap);
        memcpy(aiffhdr->samprate + sizeof(dogdoo), &longtmp, 4);
        memset(aiffhdr->samprate + sizeof(dogdoo) + 4, 0, 8);
        headersize = AIFFPLUS;
	/* fix by matju for häfeli, 2007.07.04, but really, dogdoo should be removed */
	while (samplerate >= 0x10000) {aiffhdr->samprate[1]++; samplerate/=2;}
	aiffhdr->samprate[2] = (long)samplerate>>8;
	aiffhdr->samprate[3] = (long)samplerate;
    } else {   /* WAVE format */
        long datasize = nframes * nchannels * bytespersample;
        if (!strcaseends(filenamebuf,".wav")) strcat(filenamebuf, ".wav");
        t_wave *wavehdr = (t_wave *)headerbuf;
        strncpy(wavehdr->fileid, "RIFF", 4);
        wavehdr->chunksize = swap4(datasize + sizeof(*wavehdr) - 8, swap);
        strncpy(wavehdr->waveid, "WAVE", 4);
        strncpy(wavehdr->fmtid, "fmt ", 4);
        wavehdr->fmtchunksize = swap4(16, swap);
        wavehdr->fmttag = swap2((bytespersample == 4 ? WAV_FLOAT : WAV_INT), swap);
        wavehdr->nchannels = swap2(nchannels, swap);
        wavehdr->samplespersec = swap4(size_t(samplerate), swap);
        wavehdr->navgbytespersec = swap4((int)(samplerate * nchannels * bytespersample), swap);
        wavehdr->nblockalign = swap2(nchannels * bytespersample, swap);
        wavehdr->nbitspersample = swap2(8 * bytespersample, swap);
        strncpy(wavehdr->datachunkid, "data", 4);
        wavehdr->datachunksize = swap4(datasize, swap);
        headersize = sizeof(t_wave);
    }
    char *buf2 = canvas_makefilename(canvas, filenamebuf,0,0);
    sys_bashfilename(buf2,buf2);
    if ((fd = open(buf2, BINCREATE, 0666)) < 0) {free(buf2); return -1;}
    if (write(fd, headerbuf, headersize) < headersize) {
        close (fd);
        return -1;
    }
    return fd;
}

static void soundfile_finishwrite(void *obj, char *filename, int fd,
int filetype, long nframes, long itemswritten, int bytesperframe, int swap) {
    if (itemswritten < nframes) {
        if (nframes < 0x7fffffff)
            error("soundfiler_write: %ld out of %ld bytes written", itemswritten, nframes);
        /* try to fix size fields in header */
        if (filetype == FORMAT_WAVE) {
            long datasize = itemswritten * bytesperframe, v;
            if (lseek(fd, ((char *)(&((t_wave *)0)->chunksize)) - (char *)0, SEEK_SET) == 0) goto baddonewrite;
            v = swap4(datasize + sizeof(t_wave) - 8, swap);
            if (write(fd, (char *)&v, 4) < 4) goto baddonewrite;
            if (lseek(fd, ((char *)(&((t_wave *)0)->datachunksize)) - (char *)0, SEEK_SET) == 0) goto baddonewrite;
            v = swap4(datasize, swap);
            if (write(fd, (char *)&v, 4) < 4) goto baddonewrite;
        }
        if (filetype == FORMAT_AIFF) {
            long v;
            if (lseek(fd, ((char *)(&((t_aiff *)0)->nframeshi)) - (char *)0, SEEK_SET) == 0) goto baddonewrite;
            v = swap4(itemswritten, swap);
            if (write(fd, (char *)&v,4) < 4) goto baddonewrite;
            if (lseek(fd, ((char *)(&((t_aiff *)0)->chunksize)) - (char *)0, SEEK_SET) == 0) goto baddonewrite;
            v = swap4(itemswritten*bytesperframe+AIFFHDRSIZE, swap);
            if (write(fd, (char *)&v,4) < 4) goto baddonewrite;
            if (lseek(fd, (AIFFHDRSIZE+4), SEEK_SET) == 0) goto baddonewrite;
            v = swap4(itemswritten*bytesperframe, swap);
            if (write(fd, (char *)&v,4) < 4) goto baddonewrite;
        }
        if (filetype == FORMAT_NEXT) {
            /* do it the lazy way: just set the size field to 'unknown size'*/
            uint32 nextsize = 0xffffffff;
            if (lseek(fd, 8, SEEK_SET) == 0) goto baddonewrite;
            if (write(fd, &nextsize, 4) < 4) goto baddonewrite;
        }
    }
    return;
baddonewrite:
    error("%s: %s", filename, strerror(errno));
}

static void soundfile_xferout(int nchannels, float **vecs, unsigned char *buf, int nitems, long onset, int bytespersample,
int bigendian, float normalfactor) {
    unsigned char *sp=buf, *sp2;
    float *fp;
    int bytesperframe = bytespersample * nchannels;
    #define LOOP for (int j=0; j<nitems; j++, sp2 += bytesperframe, fp++)
    for (int i = 0; i < nchannels; i++, sp += bytespersample) {
        sp2 = sp; fp = vecs[i] + onset;
        if (bytespersample == 2) {
            float ff = normalfactor * 32768.;
            if (bigendian) LOOP {
                int xx = clip(int(32768. + *fp * ff) - 0x8000,-0x7fff,+0x7fff);
                sp2[0] = xx>>8; sp2[1] = xx;
            } else LOOP {
                int xx = clip(int(32768. + *fp * ff) - 0x8000,-0x7fff,+0x7fff);
                sp2[1] = xx>>8; sp2[0] = xx;
            }
        } else if (bytespersample == 3) {
            float ff = normalfactor * 8388608.;
            if (bigendian) LOOP {
                int xx = clip(int(8388608. + *fp * ff) - 0x800000,-0x7fffff,+0x7fffff);
                sp2[0] = xx>>16; sp2[1] = xx>>8; sp2[2] = xx;
            } else LOOP {
                int xx = clip(int(8388608. + *fp * ff) - 0x800000,-0x7fffff,+0x7fffff);
                sp2[2] = xx>>16; sp2[1] = xx>>8; sp2[0] = xx;
            }
        } else if (bytespersample == 4) {
            if (bigendian) LOOP {
                float f2 = *fp * normalfactor; long xx = *(long *)&f2;
                sp2[0] = xx >> 24; sp2[1] = xx >> 16; sp2[2] = xx >> 8; sp2[3] = xx;
            } else LOOP {
                float f2 = *fp * normalfactor; long xx = *(long *)&f2;
                sp2[3] = xx >> 24; sp2[2] = xx >> 16; sp2[1] = xx >> 8; sp2[0] = xx;
            }
        }
    }
    #undef LOOP
}

/* ------- soundfiler - reads and writes soundfiles to/from "garrays" ---- */

#define DEFMAXSIZE (16*1024*1024*4) /* default maximum 16 million floats per channel */
#define SAMPBUFSIZE 1024

static t_class *soundfiler_class;

struct t_soundfiler : t_object {t_canvas *canvas;};

#ifdef THREADED_SF
#include <sched.h>
#if (_POSIX_MEMLOCK - 0) >=  200112L
#include <sys/mman.h>
#else
#define munlockall() /* ignore */
#define   mlockall() /* ignore */
#endif /* _POSIX_MEMLOCK */

static pthread_t sf_thread_id; /* id of soundfiler thread */

struct t_sfprocess {
    void (*process)(t_soundfiler *,t_symbol *, int, t_atom *); /* function to call */
    t_soundfiler *x;
    int argc;
    t_atom *argv;
    struct t_sfprocess *next;  /* next object in queue */
    pthread_mutex_t mutex;
};

/* this is the queue for all soundfiler objects */
struct t_sfqueue {
    t_sfprocess *begin;
    t_sfprocess *end;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

static t_sfqueue *soundfiler_queue;

/* we fill the queue */
void soundfiler_queue_add(void (* process) (t_soundfiler *,t_symbol *,int,t_atom *), void * x, int argc, t_atom * argv) {
    /* preparing entry */
    t_sfprocess * last_entry = (t_sfprocess*)getbytes(sizeof(t_sfprocess));
    if (debug) post("adding process to queue");
    pthread_mutex_init(&(last_entry->mutex), NULL);
    pthread_mutex_lock(&(last_entry->mutex));
    last_entry->process = process;
    last_entry->x = (t_soundfiler *)x;
    last_entry->argc = argc;
    last_entry->argv = (t_atom *)copybytes(argv, argc * sizeof(t_atom));
    last_entry->next = NULL;
    pthread_mutex_unlock(&(last_entry->mutex));
    /* add new entry to queue */
    pthread_mutex_lock(&(soundfiler_queue->mutex));
    if (soundfiler_queue->begin==NULL) {
	soundfiler_queue->begin=last_entry;
	soundfiler_queue->end=last_entry;
    } else {
	pthread_mutex_lock(&(soundfiler_queue->end->mutex));
	soundfiler_queue->end->next=last_entry;
	pthread_mutex_unlock(&(soundfiler_queue->end->mutex));
	soundfiler_queue->end=last_entry;
    }
    if ( soundfiler_queue->begin == soundfiler_queue->end ) {
	if (debug) post("signaling");
	pthread_mutex_unlock(&(soundfiler_queue->mutex));
	/* and signal the helper thread */
	pthread_cond_signal(&(soundfiler_queue->cond));
    } else {
	if (debug) post("not signaling");
	pthread_mutex_unlock(&(soundfiler_queue->mutex));
    }
    return;
}

/* global soundfiler thread ... sleeping until signaled */
void soundfiler_thread() {
    t_sfprocess *me;
    t_sfprocess *next;
    if (debug) post("soundfiler_thread ID: %d", pthread_self());
    while (1) {
	if (debug) post("Soundfiler sleeping");
	pthread_cond_wait(&soundfiler_queue->cond, &soundfiler_queue->mutex);
	if (debug) post("Soundfiler awake");
	/* work on the queue */
	while (soundfiler_queue->begin!=NULL) {
		post("soundfiler: locked queue");
		/* locking process */
		pthread_mutex_lock(&(soundfiler_queue->begin->mutex));
		me = soundfiler_queue->begin;
		pthread_mutex_unlock(&(me->mutex));
		pthread_mutex_unlock(&(soundfiler_queue->mutex));
		if (debug) post("soundfiler: mutex unlocked, running process");
		/* running the specific function */
		me->process(me->x, NULL, me->argc, me->argv);
		if (debug) post("soundfiler: process done, locking mutex");
		pthread_mutex_lock(&(soundfiler_queue->mutex));
		pthread_mutex_lock(&(me->mutex));
		free(me->argv);
		/* the  process struct */
		next=me->next;
		soundfiler_queue->begin=next;
		free(me);
	}
	soundfiler_queue->end=NULL;
    }
}

extern int sys_hipriority;   	/* real-time flag, true if priority boosted */

/* create soundfiler thread */
void sys_start_sfthread() {
    pthread_attr_t sf_attr;
    struct sched_param sf_param;
    int status;
    // initialize queue
    soundfiler_queue = (t_sfqueue *)getbytes(sizeof(t_sfqueue));
    pthread_mutex_init(&soundfiler_queue->mutex,NULL);
    pthread_cond_init(&soundfiler_queue->cond,NULL);
    soundfiler_queue->begin=soundfiler_queue->end=NULL;
/*     pthread_mutex_unlock(&(soundfiler_queue->mutex));  */
    // initialize thread
    pthread_attr_init(&sf_attr);
    sf_param.sched_priority=sched_get_priority_min(SCHED_OTHER);
    pthread_attr_setschedparam(&sf_attr,&sf_param);
/*     pthread_attr_setinheritsched(&sf_attr,PTHREAD_EXPLICIT_SCHED); */
#ifdef UNIX
     if (sys_hipriority == 1 && getuid() == 0) {
	sf_param.sched_priority=sched_get_priority_min(SCHED_RR);
	pthread_attr_setschedpolicy(&sf_attr,SCHED_RR);
     } else {
/* 	pthread_attr_setschedpolicy(&sf_attr,SCHED_OTHER); */
/* 	sf_param.sched_priority=sched_get_priority_min(SCHED_OTHER); */
     }
#endif /* UNIX */
    //start thread
    status = pthread_create(&sf_thread_id, &sf_attr, (void *(*)(void *)) soundfiler_thread,NULL);
    if (status != 0) error("Couldn't create soundfiler thread: %d",status);
    else post("global soundfiler thread launched, priority: %d", sf_param.sched_priority);
}

static void soundfiler_t_write(     t_soundfiler *x, t_symbol *s, int argc, t_atom *argv);
static void soundfiler_t_write_addq(t_soundfiler *x, t_symbol *s, int argc, t_atom *argv) {
    soundfiler_queue_add(soundfiler_t_write,(void *)x,argc, argv);
}
static void soundfiler_t_read(     t_soundfiler *x, t_symbol *s, int argc, t_atom *argv);
static void soundfiler_t_read_addq(t_soundfiler *x, t_symbol *s, int argc, t_atom *argv) {
    soundfiler_queue_add(soundfiler_t_read,(void *)x,argc, argv);
}

/* soundfiler_read
   usage: read [flags] filename table ...
   flags: -skip <frames> ... frames to skip in file
	-nframes <frames> -onset <frames> ... onset in table to read into (NOT DONE YET)
	-raw <headersize channels bytes endian> -resize -maxsize <max-size>
    TB: adapted for threaded use */
static t_int soundfiler_read_update_garray(t_int *w);
static t_int soundfiler_read_update_graphics(t_int *w);
static t_int soundfiler_read_output(t_int *w);
static void soundfiler_t_read(t_soundfiler *x, t_symbol *s, int argc, t_atom *argv) {
    t_param p;
    int headersize = -1;
    p.nchannels = 0; p.bytespersample = 0; p.bigendian = 0;
    int resize = 0, i, j;
    long skipframes = 0, nframes = 0, finalsize = 0, maxsize = DEFMAXSIZE, itemsread = 0;
    p.bytelimit = 0x7fffffff;
    int fd = -1;
    char endianness, *filename;
    t_garray *garrays[MAXSFCHANS];
    t_float *vecs[MAXSFCHANS];             /* the old array */
    t_float *nvecs[MAXSFCHANS];            /* the new array */
    int vecsize[MAXSFCHANS];               /* the old array size */
    char sampbuf[SAMPBUFSIZE];
    int bufframes, nitems;
    FILE *fp;
    pthread_cond_t resume_after_callback = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t resume_after_callback_mutex = PTHREAD_MUTEX_INITIALIZER; /* dummy */
    t_int* outargs;
    while (argc > 0 && argv->a_type == A_SYMBOL && *argv->a_symbol->name == '-') {
    	char *flag = argv->a_symbol->name + 1;
	argc--; argv++;
	if (!strcmp(flag, "skip")) {
	    EAT_ARG(A_FLOAT,skipframes); if (skipframes<0) goto usage;
	} else if (!strcmp(flag, "nframes")) {
	    EAT_ARG(A_FLOAT,nframes); if (nframes<0) goto usage;
	} else if (!strcmp(flag, "raw")) {
            EAT_ARG(A_FLOAT,headersize); if (headersize<0) goto usage;
            EAT_ARG(A_FLOAT,p.nchannels); if (p.nchannels<1) goto usage;
            EAT_ARG(A_FLOAT,p.bytespersample); if (p.bytespersample<2 || p.bytespersample>4) goto usage;
            EAT_ARG(A_SYMBOL,endianness); if (endianness!='b' && endianness!='l' && endianness!='n') goto usage;
	    if      (endianness == 'b') p.bigendian = 1;
	    else if (endianness == 'l') p.bigendian = 0;
	    else p.bigendian = garray_ambigendian();
	} else if (!strcmp(flag, "resize")) {
	    resize = 1;
	} else if (!strcmp(flag, "maxsize")) {
	    EAT_ARG(A_FLOAT,maxsize); if (maxsize<0) goto usage;
	    resize = 1;     /* maxsize implies resize. */
	} else goto usage;
    }
    if (argc < 2 || argc > MAXSFCHANS + 1 || argv[0].a_type != A_SYMBOL) goto usage;
    filename = argv[0].a_symbol->name;
    argc--; argv++;
    for (int i=0; i<argc; i++) {
    	if (argv[i].a_type != A_SYMBOL) goto usage;
	garrays[i] = (t_garray *)pd_findbyclass(argv[i].a_symbol, garray_class);
	if (!garrays[i]) {
	    error("%s: no such table", argv[i].a_symbol->name);
	    goto done;
	} else if (!garray_getfloatarray(garrays[i], &vecsize[i], &vecs[i]))
    	    error("%s: bad template for tabwrite", argv[i].a_symbol->name);
    	if (finalsize && finalsize != vecsize[i] && !resize) {
	    post("soundfiler_read: arrays have different lengths; resizing...");
	    resize = 1;
	}
	finalsize = vecsize[i];
    }
    fd = open_soundfile(canvas_getdir(x->canvas)->name, filename, headersize, &p, skipframes);
    if (fd < 0) {
	error("soundfiler_read: %s: %s", filename, (errno == EIO ? "unknown or bad header format" : strerror(errno)));
    	goto done;
    }
    if (resize) {
	/* figure out what to resize to */
    	long poswas, eofis, framesinfile;
	poswas = lseek(fd, 0, SEEK_CUR);
	eofis = lseek(fd, 0, SEEK_END);
	if (poswas < 0 || eofis < 0) {error("lseek failed"); goto done;}
	lseek(fd, poswas, SEEK_SET);
	framesinfile = (eofis - poswas) / p.bytesperchannel();
	if (framesinfile > maxsize) {
	    error("soundfiler_read: truncated to %d elements", maxsize);
	    framesinfile = maxsize;
	}
        framesinfile = min(framesinfile, p.bytelimit / p.bytesperchannel());
	finalsize = framesinfile;
    }
    if (!finalsize) finalsize = 0x7fffffff;
    finalsize = min(finalsize, p.bytelimit / p.bytesperchannel());
    fp = fdopen(fd, "rb");
    bufframes = SAMPBUFSIZE / p.bytesperchannel();
    if (debug) {
        post("buffers: %d", argc);
        post("channels: %d", p.nchannels);
    }
    munlockall();
    /* allocate memory for new array */
    if (resize)
	for (int i=0; i<argc; i++) {
		nvecs[i] = (float *)getalignedbytes(finalsize * sizeof(t_float));
		/* if we are out of memory, free it again and quit */
		if (nvecs[i]==0) {
			error("resize failed");
			/* if the resizing fails, we'll have to free all arrays again */
			for (j=0; j!=i;++j) freealignedbytes (nvecs[i],finalsize * sizeof(t_float));
			goto done;
		}
	}
    else
	for (int i=0; i<argc; i++) {
		nvecs[i] = (float *)getalignedbytes(vecsize[i] * sizeof(t_float));
		/* if we are out of memory, free it again and quit */
		if (nvecs[i]==0) {
			error("resize failed");
			/* if the resizing fails, we'll have to free all arrays again */
			for (j=0; j!=i;++j) freealignedbytes (nvecs[i],vecsize[i] * sizeof(t_float));
			goto done;
		}
	}
    if(i > p.nchannels) memset(nvecs[i],0,vecsize[i] * sizeof(t_float));
    if (debug) post("transfer soundfile");
    for (itemsread = 0; itemsread < finalsize; ) {
	int thisread = finalsize - itemsread;
	thisread = (thisread > bufframes ? bufframes : thisread);
	nitems = fread(sampbuf, p.bytesperchannel(), thisread, fp);
	if (nitems <= 0) break;
	soundfile_xferin(p.nchannels, argc, nvecs, itemsread, (unsigned char *)sampbuf, nitems, p.bytespersample, p.bigendian);
	itemsread += nitems;
    }
    if (debug) post("zeroing remaining elements");
    /* zero out remaining elements of vectors */
    for (int i=0; i<argc; i++) for (int j=itemsread; j<finalsize; j++) nvecs[i][j] = 0;
    /* set idle callback to switch pointers */
    if (debug) post("locked");
    for (int i=0; i<argc; i++) {
	t_int *w = (t_int*)getbytes(4*sizeof(t_int));
	w[0] = (t_int)(garrays[i]);
	w[1] = (t_int)nvecs[i];
	w[2] = (t_int)finalsize;
	w[3] = (t_int)(&resume_after_callback);
	sys_callback(&soundfiler_read_update_garray, w, 4);
	pthread_cond_wait(&resume_after_callback, &resume_after_callback_mutex);
    }
    if (debug) post("unlocked, doing graphics updates");
    /* do all graphics updates. run this in the main thread via callback */
    for (int i=0; i<argc; i++) {
	t_int *w = (t_int*)getbytes(2*sizeof(t_int));
	w[0] = (t_int)(garrays[i]);
	w[1] = (t_int)finalsize;
	sys_callback(&soundfiler_read_update_graphics, w, 2);
    }
    /* free the old arrays */
    for (int i=0; i<argc; i++) freealignedbytes(vecs[i], vecsize[i] * sizeof(t_float));
    fclose(fp);
    fd = -1;
    goto done;
usage:
    error("usage: read [flags] filename tablename...");
    post("flags: -skip <n> -nframes <n> -resize -maxsize <n> ...");
    post("-raw <headerbytes> <channels> <bytespersample> <endian (b, l, or n)>.");
done:
    if (fd>=0) close(fd);
    mlockall(MCL_FUTURE);
    outargs = (t_int*)getbytes(2*sizeof(t_int));
    outargs[0] = (t_int)x->outlet;
    outargs[1] = (t_int)itemsread;
    sys_callback(&soundfiler_read_output, outargs, 2);
}

/* idle callback for threadsafe synchronisation */
static t_int soundfiler_read_update_garray(t_int *w) {
	t_garray *garray = (t_garray*)w[0];
	t_int nvec = w[1];
	t_int finalsize = w[2];
	pthread_cond_t *conditional = (pthread_cond_t*) w[3];
	t_array *a = garray_getarray(garray);
	a->vec = (char *) nvec;
	a->n = finalsize;
	if (garray->usedindsp) canvas_update_dsp();
	/* signal helper thread */
	pthread_cond_broadcast(conditional);
	return 0;
}

static t_int soundfiler_read_update_graphics(t_int *w) {
	t_garray *garray = (t_garray*) w[0];
	t_canvas *gl;
	int n = w[1];
	/* if this is the only array in the graph, reset the graph's coordinates */
	if (debug) post("redraw array %p", garray);
	gl = garray->canvas;
	if (gl->list == garray && !garray->next) {
		vmess(gl, gensym("bounds"), "ffff", 0., gl->y1, double(n > 1 ? n-1 : 1), gl->y2);
		/* close any dialogs that might have the wrong info now... */
	} else garray_redraw(garray);
	return 0;
}

static t_int soundfiler_read_output(t_int * w) {
	t_outlet* outlet = (t_outlet*) w[0];
	float itemsread = (float) w[1];
	if (debug) post("bang %p", outlet);
	outlet->send(itemsread);
	return 0;
}

/* this is broken out from soundfiler_write below so garray_write can call it too... not done yet though. */
long soundfiler_t_dowrite(void *obj, t_canvas *canvas, int argc, t_atom *argv) {
    int bytespersample, bigendian, swap, filetype, normalize, nchannels;
    long onset, nframes, itemswritten = 0;
    t_garray *garrays[MAXSFCHANS];
    t_float *vecs[MAXSFCHANS];
    char sampbuf[SAMPBUFSIZE];
    int bufframes;
    int fd = -1;
    float normfactor, biggest = 0, samplerate;
    t_symbol *filesym;
    if (soundfiler_writeargparse(obj, &argc, &argv, &filesym, &filetype,
	&bytespersample, &swap, &bigendian, &normalize, &onset, &nframes, &samplerate))
		goto usage;
    nchannels = argc;
    if (nchannels < 1 || nchannels > MAXSFCHANS) goto usage;
    if (samplerate < 0) samplerate = sys_getsr();
    for (int i=0; i<nchannels; i++) {
    	int vecsize;
    	if (argv[i].a_type != A_SYMBOL) goto usage;
	if (!(garrays[i] = (t_garray *)pd_findbyclass(argv[i].a_symbol, garray_class))) {
	    error("%s: no such table", argv[i].a_symbol->name);
	    goto fail;
	}
    	else if (!garray_getfloatarray(garrays[i], &vecsize, &vecs[i]))
    	    error("%s: bad template for tabwrite", argv[i].a_symbol->name);
    	if (nframes > vecsize - onset)
	    nframes = vecsize - onset;
	for (int j=0; j<vecsize; j++) {
	    if      (+vecs[i][j] > biggest) biggest = +vecs[i][j];
	    else if (-vecs[i][j] > biggest) biggest = -vecs[i][j];
    	}
    }
    if (nframes <= 0) {
	error("soundfiler_write: no samples at onset %ld", onset);
    	goto fail;
    }
    if ((fd = create_soundfile(canvas, filesym->name, filetype, nframes, bytespersample, bigendian, nchannels, swap, samplerate)) < 0) {
    	post("%s: %s", filesym->name, strerror(errno));
    	goto fail;
    }
    if (!normalize) {
    	if ((bytespersample != 4) && (biggest > 1)) {
    	    post("%s: normalizing max amplitude %f to 1", filesym->name, biggest);
    	    normalize = 1;
    	} else post("%s: biggest amplitude = %f", filesym->name, biggest);
    }
    if (normalize) normfactor = (biggest > 0 ? 32767./(32768. * biggest) : 1);
    else normfactor = 1;
    bufframes = SAMPBUFSIZE / (nchannels * bytespersample);
    for (itemswritten = 0; itemswritten < nframes; ) {
    	int thiswrite = nframes - itemswritten, nbytes;
    	thiswrite = (thiswrite > bufframes ? bufframes : thiswrite);
	soundfile_xferout(argc, vecs, (unsigned char *)sampbuf, thiswrite, onset, bytespersample, bigendian, normfactor);
    	nbytes = write(fd, sampbuf, nchannels * bytespersample * thiswrite);
	if (nbytes < nchannels * bytespersample * thiswrite) {
	    post("%s: %s", filesym->name, strerror(errno));
	    if (nbytes > 0) itemswritten += nbytes / (nchannels * bytespersample);
	    break;
	}
	itemswritten += thiswrite;
	onset += thiswrite;
    }
    if (fd >= 0) {
    	soundfile_finishwrite(obj, filesym->name, fd, filetype, nframes, itemswritten, nchannels * bytespersample, swap);
    	close (fd);
    }
    return itemswritten;
usage:
    error("usage: write [flags] filename tablename...");
    post("flags: -skip <n> -nframes <n> -bytes <n> -wave -aiff -nextstep ...");
    post("-big -little -normalize");
    post("(defaults to a 16-bit wave file).");
fail:
    if (fd >= 0) close(fd);
    return 0;
}

static void soundfiler_t_write(t_soundfiler *x, t_symbol *s, int argc, t_atom *argv) {
    long bozo = soundfiler_t_dowrite(x, x->canvas, argc, argv);
    sys_lock();
    x->outlet->send(float(bozo));
    sys_lock();
}

static void soundfiler_t_resize(t_soundfiler *x, t_symbol *s, int argc, t_atom *argv);
static void soundfiler_t_resize_addq(t_soundfiler *x, t_symbol *s, int argc, t_atom *argv) {
    soundfiler_queue_add(soundfiler_t_resize,(void *)x,argc, argv);
}

/* TB: soundfiler_t_resize ... usage: resize table size; adapted from garray_resize */
static void soundfiler_t_resize(t_soundfiler *y, t_symbol *s, int argc, t_atom *argv) {
    int was, elemsize;       /* array contains was elements of size elemsize */
    t_float *vec;           /* old array */
    t_canvas *gl;
    int n;                   /* resize of n elements */
    char *nvec;              /* new array */
    t_garray *x = (t_garray *)pd_findbyclass(argv[0].a_symbol, garray_class);
    t_array *a = garray_getarray(x);
    if (!x) {error("%s: no such table", argv[0].a_symbol->name); goto usage;}
    vec = (t_float*) a->vec;
    was = a->n;
    if ((argv+1)->a_type == A_FLOAT) {
	n = (int) (argv+1)->a_float;
    } else goto usage;
    if (n == was) return;
    if (n < 1) n = 1;
    elemsize = template_findbyname(a->templatesym)->t_n * sizeof(t_word);
    munlockall();
    if (was > n) {
	nvec = (char *)copyalignedbytes(a->vec, was * elemsize);
    } else {
	nvec = (char *)getalignedbytes(n * elemsize);
	memcpy (nvec, a->vec, was * elemsize);
	memset(nvec + was*elemsize, 0, (n - was) * elemsize);
    }
    if (!nvec) {error("array resize failed: out of memory"); mlockall(MCL_FUTURE); return;}
    /* TB: we'll have to be sure that no one is accessing the array */
    sys_lock();
    a->vec = nvec;
    a->n = n;
    if (x->usedindsp) canvas_update_dsp();
    sys_unlock();
    /* if this is the only array in the graph, reset the graph's coordinates */
    gl = x->canvas;
    if (gl->list == x && !x->next) {
    	vmess(gl, gensym("bounds"), "ffff", 0., gl->y1, (double)(n > 1 ? n-1 : 1), gl->y2);
	/* close any dialogs that might have the wrong info now... */
    } else garray_redraw(x);
    freealignedbytes (vec, was * elemsize);
    mlockall(MCL_FUTURE);
    sys_lock();
    y->outlet->send((float)atom_getintarg(1,argc,argv));
    sys_unlock();
    return;
usage:
    error("usage: resize tablename size");
}

static void soundfiler_t_const(t_soundfiler *x, t_symbol *s, int argc, t_atom *argv);
static void soundfiler_t_const_addq(t_soundfiler *x, t_symbol *s, int argc, t_atom *argv) {
    soundfiler_queue_add(soundfiler_t_const,(void *)x,argc, argv);
}

/* TB: soundfiler_t_const ... usage: const table value */
static void soundfiler_t_const(t_soundfiler *y, t_symbol *s, int argc, t_atom *argv) {
    int size, elemsize;    /* array contains was elements of size elemsize */
    t_float *vec;         /* old array */
    t_canvas *gl;
    int val;               /* value */
    char *nvec;            /* new array */
    t_garray * x = (t_garray *)pd_findbyclass(argv[0].a_symbol, garray_class);
    t_array *a = garray_getarray(x);
    if (!x) {error("%s: no such table", argv[0].a_symbol->name); goto usage;}
    vec = (t_float*) a->vec;
    size = a->n;
    if ((argv+1)->a_type == A_FLOAT) {
	val = (int) (argv+1)->a_float;
    } else goto usage;
    elemsize = template_findbyname(a->templatesym)->t_n * sizeof(t_word);
    /* allocating memory */
    munlockall();
    nvec = (char *)getalignedbytes(size * elemsize);
    if (!nvec) {
    	error("array resize failed: out of memory");
	mlockall(MCL_FUTURE);
	return;
    }
    /* setting array */
    for (int i=0; i!=size; ++i) nvec[i]=val;
    /* TB: we'll have to be sure that no one is accessing the array */
    sys_lock();
    a->vec = nvec;
    if (x->usedindsp) canvas_update_dsp();
    sys_unlock();
    /* if this is the only array in the graph, reset the graph's coordinates */
    gl = x->canvas;
    if (gl->list == x && !x->next) {
    	vmess(gl, gensym("bounds"), "ffff", 0., gl->y1, (double)(size > 1 ? size-1 : 1), gl->y2);
	/* close any dialogs that might have the wrong info now... */
    } else garray_redraw(x);
    freealignedbytes (vec, size * elemsize);
    mlockall(MCL_FUTURE);
    sys_lock();
    y->outlet->send(size);
    sys_unlock();
    return;
 usage:
    error("usage: const tablename value");
}

#endif /* THREADED_SF */

static t_soundfiler *soundfiler_new() {
    t_soundfiler *x = (t_soundfiler *)pd_new(soundfiler_class);
    x->canvas = canvas_getcurrent();
    outlet_new(x,&s_float);
#ifdef THREADED_SF
    post("warning: threaded soundfiler is not synchronous");
#endif /* THREADED_SF */
    return x;
}

/* soundfiler_read ...
    usage: read [flags] filename table ...
    flags: -skip <frames> ... frames to skip in file
        -nframes <frames> -onset <frames> ... onset in table to read into (NOT DONE YET)
        -raw <headersize channels bytes endian> -resize -maxsize <max-size> */
static void soundfiler_read(t_soundfiler *x, t_symbol *s, int argc, t_atom *argv) {
    t_param p;
    p.bytespersample = 0;
    p.bigendian = 0;
    p.nchannels = 0;
    p.bytelimit = 0x7fffffff;
    int headersize = -1, resize = 0;
    long skipframes = 0, nframes = 0, finalsize = 0, maxsize = DEFMAXSIZE, itemsread = 0;
    int fd = -1;
    char endianness, *filename;
    t_garray *garrays[MAXSFCHANS];
    t_float *vecs[MAXSFCHANS];
    char sampbuf[SAMPBUFSIZE];
    int bufframes, nitems;
    FILE *fp;
    while (argc > 0 && argv->a_type == A_SYMBOL && *argv->a_symbol->name == '-') {
        char *flag = argv->a_symbol->name + 1;
	argc--; argv++;
        if (!strcmp(flag, "skip")) {
            EAT_ARG(A_FLOAT,skipframes); if (skipframes<0) goto usage;
        } else if (!strcmp(flag, "nframes")) {
            EAT_ARG(A_FLOAT,nframes); if (nframes<0) goto usage;
        } else if (!strcmp(flag, "raw")) {
            EAT_ARG(A_FLOAT,headersize); if (headersize<0) goto usage;
            EAT_ARG(A_FLOAT,p.nchannels); if (p.nchannels<1) goto usage;
            EAT_ARG(A_FLOAT,p.bytespersample); if (p.bytespersample<2 || p.bytespersample>4) goto usage;
            EAT_ARG(A_SYMBOL,endianness); if (endianness!='b' && endianness!='l' && endianness!='n') goto usage;
            if      (endianness == 'b') p.bigendian = 1;
            else if (endianness == 'l') p.bigendian = 0;
            else p.bigendian = garray_ambigendian();
        } else if (!strcmp(flag, "resize")) {
            resize = 1;
        } else if (!strcmp(flag, "maxsize")) {
            EAT_ARG(A_FLOAT,maxsize); if (maxsize<0) goto usage;
            resize = 1;     /* maxsize implies resize. */
        } else goto usage;
    }
    if (argc < 2 || argc > MAXSFCHANS + 1 || argv[0].a_type != A_SYMBOL) goto usage;
    filename = argv[0].a_symbol->name;
    argc--; argv++;
    for (int i=0; i<argc; i++) {
        int vecsize;
        if (argv[i].a_type != A_SYMBOL) goto usage;
        garrays[i] = (t_garray *)pd_findbyclass(argv[i].a_symbol, garray_class);
	if (!garrays[i]) {
            error("%s: no such table", argv[i].a_symbol->name);
            goto done;
        } else if (!garray_getfloatarray(garrays[i], &vecsize, &vecs[i]))
            error("%s: bad template for tabwrite", argv[i].a_symbol->name);
        if (finalsize && finalsize != vecsize && !resize) {
            post("soundfiler_read: arrays have different lengths; resizing...");
            resize = 1;
        }
        finalsize = vecsize;
    }
    fd = open_soundfile_via_canvas(x->canvas, filename, headersize, &p, skipframes);
    if (fd < 0) {
        error("soundfiler_read: %s: %s", filename, (errno == EIO ? "unknown or bad header format" : strerror(errno)));
        goto done;
    }
    if (resize) {
        /* figure out what to resize to */
        long poswas, eofis, framesinfile;
        poswas = lseek(fd, 0, SEEK_CUR);
        eofis = lseek(fd, 0, SEEK_END);
        if (poswas < 0 || eofis < 0) {error("lseek failed"); goto done;}
        lseek(fd, poswas, SEEK_SET);
        framesinfile = (eofis - poswas) / (p.nchannels * p.bytespersample);
        if (framesinfile > maxsize) {error("soundfiler_read: truncated to %ld elements", maxsize); framesinfile = maxsize;}
        framesinfile = min(framesinfile, p.bytelimit / (p.nchannels * p.bytespersample));
        finalsize = framesinfile;
        for (int i=0; i<argc; i++) {
            int vecsize;
            garray_resize(garrays[i], finalsize);
            /* for sanity's sake let's clear the save-in-patch flag here */
            garray_setsaveit(garrays[i], 0);
            garray_getfloatarray(garrays[i], &vecsize, &vecs[i]);
            /* if the resize failed, garray_resize reported the error */
            if (vecsize != framesinfile) {error("resize failed"); goto done;}
        }
    }
    if (!finalsize) finalsize = 0x7fffffff;
    finalsize = min(finalsize, p.bytelimit / (p.nchannels * p.bytespersample));
    fp = fdopen(fd, "rb");
    bufframes = SAMPBUFSIZE / (p.nchannels * p.bytespersample);
    for (itemsread = 0; itemsread < finalsize; ) {
        int thisread = finalsize - itemsread;
        thisread = min(thisread,bufframes);
        nitems = fread(sampbuf, p.nchannels * p.bytespersample, thisread, fp);
        if (nitems <= 0) break;
        soundfile_xferin(p.nchannels, argc, vecs, itemsread, (unsigned char *)sampbuf, nitems, p.bytespersample, p.bigendian);
        itemsread += nitems;
    }
    /* zero out remaining elements of vectors */
    for (int i=0; i<argc; i++) {
        int vecsize; garray_getfloatarray(garrays[i], &vecsize, &vecs[i]);
        for (int j=itemsread; j<vecsize; j++) vecs[i][j]=0;
    }
    /* zero out vectors in excess of number of channels */
    for (int i=p.nchannels; i<argc; i++) {
        int vecsize; float *foo; garray_getfloatarray(garrays[i], &vecsize, &foo);
        for (int j=0; j<vecsize; j++) foo[j]=0;
    }
    /* do all graphics updates */
    for (int i=0; i<argc; i++) garray_redraw(garrays[i]);
    fclose(fp);
    fd = -1;
    goto done;
usage:
    error("usage: read [flags] filename tablename...");
    post("flags: -skip <n> -nframes <n> -resize -maxsize <n> ...");
    post("-raw <headerbytes> <channels> <bytespersample> <endian (b, l, or n)>.");
done:
    if (fd >= 0) close(fd);
    x->outlet->send(float(itemsread));
}

/* this is broken out from soundfiler_write below so garray_write can
   call it too... not done yet though. */
long soundfiler_dowrite(void *obj, t_canvas *canvas, int argc, t_atom *argv) {
    int bytespersample, bigendian, swap, filetype, normalize, nchannels;
    long onset, nframes, itemswritten = 0;
    t_garray *garrays[MAXSFCHANS];
    t_float *vecs[MAXSFCHANS];
    char sampbuf[SAMPBUFSIZE];
    int bufframes;
    int fd = -1;
    float normfactor, biggest = 0, samplerate;
    t_symbol *filesym;
    if (soundfiler_writeargparse(obj, &argc, &argv, &filesym, &filetype,
        &bytespersample, &swap, &bigendian, &normalize, &onset, &nframes,
            &samplerate))
                goto usage;
    nchannels = argc;
    if (nchannels < 1 || nchannels > MAXSFCHANS) goto usage;
    if (samplerate < 0) samplerate = sys_getsr();
    for (int i=0; i<nchannels; i++) {
        int vecsize;
        if (argv[i].a_type != A_SYMBOL) goto usage;
        if (!(garrays[i] = (t_garray *)pd_findbyclass(argv[i].a_symbol, garray_class))) {
            error("%s: no such table", argv[i].a_symbol->name);
            goto fail;
        }
        else if (!garray_getfloatarray(garrays[i], &vecsize, &vecs[i]))
            error("%s: bad template for tabwrite", argv[i].a_symbol->name);
        nframes = min(nframes, vecsize - onset);
        for (int j=0; j<vecsize; j++) {
            if      (+vecs[i][j] > biggest) biggest = +vecs[i][j];
            else if (-vecs[i][j] > biggest) biggest = -vecs[i][j];
        }
    }
    if (nframes <= 0) {
        error("soundfiler_write: no samples at onset %ld", onset);
        goto fail;
    }
    if ((fd = create_soundfile(canvas, filesym->name, filetype, nframes, bytespersample, bigendian, nchannels, swap, samplerate)) < 0) {
        post("%s: %s", filesym->name, strerror(errno));
        goto fail;
    }
    if (!normalize) {
        if ((bytespersample != 4) && (biggest > 1)) {
            post("%s: normalizing max amplitude %f to 1", filesym->name, biggest);
            normalize = 1;
        } else post("%s: biggest amplitude = %f", filesym->name, biggest);
    }
    if (normalize) normfactor = (biggest > 0 ? 32767./(32768. * biggest) : 1); else normfactor = 1;
    bufframes = SAMPBUFSIZE / (nchannels * bytespersample);
    for (itemswritten = 0; itemswritten < nframes; ) {
        int thiswrite = nframes - itemswritten, nbytes;
        thiswrite = (thiswrite > bufframes ? bufframes : thiswrite);
        soundfile_xferout(argc, vecs, (unsigned char *)sampbuf, thiswrite, onset, bytespersample, bigendian, normfactor);
        nbytes = write(fd, sampbuf, nchannels * bytespersample * thiswrite);
        if (nbytes < nchannels * bytespersample * thiswrite) {
            post("%s: %s", filesym->name, strerror(errno));
            if (nbytes > 0) itemswritten += nbytes / (nchannels * bytespersample);
            break;
        }
        itemswritten += thiswrite;
        onset += thiswrite;
    }
    if (fd >= 0) {
        soundfile_finishwrite(obj, filesym->name, fd, filetype, nframes, itemswritten, nchannels * bytespersample, swap);
        close (fd);
    }
    return itemswritten;
usage:
    error("usage: write [flags] filename tablename...");
    post("flags: -skip <n> -nframes <n> -bytes <n> -wave -aiff -nextstep ...");
    post("-big -little -normalize");
    post("(defaults to a 16-bit wave file).");
fail:
    if (fd >= 0) close(fd);
    return 0;
}

static void soundfiler_write(t_soundfiler *x, t_symbol *s, int argc, t_atom *argv) {
    long bozo = soundfiler_dowrite(x, x->canvas, argc, argv);
    x->outlet->send(bozo);
}

static void soundfiler_setup() {
    t_class *c = soundfiler_class = class_new2("soundfiler", (t_newmethod)soundfiler_new, 0, sizeof(t_soundfiler), 0, "");
#ifdef THREADED_SF
    class_addmethod2(c, soundfiler_t_read_addq, "read", "*");
/*     class_addmethod2(c, soundfiler_t_write_addq, "write", "*"); */
    class_addmethod2(c, soundfiler_t_resize_addq, "resize", "*");
    class_addmethod2(c, soundfiler_t_const_addq,  "const", "*");
#else
    class_addmethod2(c, soundfiler_read, "read", "*");
#endif /* THREADED_SF */
    class_addmethod2(c, soundfiler_write, "write", "*");
}

/************************* readsf object ******************************/

/* READSF uses the Posix threads package; for the moment we're Linux
only although this should be portable to the other platforms.

Each instance of readsf~ owns a "child" thread for doing the unix (MSW?) file
reading.  The parent thread signals the child each time:
    (1) a file wants opening or closing;
    (2) we've eaten another 1/16 of the shared buffer (so that the
        child thread should check if it's time to read some more.)
The child signals the parent whenever a read has completed.  Signalling
is done by setting "conditions" and putting data in mutex-controlled common
areas.
*/

#define MAXBYTESPERSAMPLE 4
#define MAXVECSIZE 128

#define READSIZE 65536
#define WRITESIZE 65536
#define DEFBUFPERCHAN 262144
#define MINBUFSIZE (4 * READSIZE)
#define MAXBUFSIZE 16777216     /* arbitrary; just don't want to hang malloc */

#define REQUEST_NOTHING 0
#define REQUEST_OPEN 1
#define REQUEST_CLOSE 2
#define REQUEST_QUIT 3
#define REQUEST_BUSY 4

#define STATE_IDLE 0
#define STATE_STARTUP 1
#define STATE_STREAM 2

static t_class *readsf_class;

struct t_readsf : t_object {
    t_canvas *canvas;
    t_clock *clock;
    char *buf;                      /* soundfile buffer */
    int bufsize;                    /* buffer size in bytes */
    int noutlets;                   /* number of audio outlets */
    t_sample *(outvec[MAXSFCHANS]); /* audio vectors */
    int vecsize;                    /* vector size for transfers */
    t_outlet *bangout;              /* bang-on-done outlet */
    int state;                      /* opened, running, or idle */
    float insamplerate;   /* sample rate of input signal if known */
        /* parameters to communicate with subthread */
    int requestcode;      /* pending request from parent to I/O thread */
    char *filename;       /* file to open (string is permanently allocated) */
    int fileerror;        /* slot for "errno" return */
    int skipheaderbytes;  /* size of header we'll skip */
    t_param p;
    float samplerate;     /* sample rate of soundfile */
    long onsetframes;     /* number of sample frames to skip */
    int fd;               /* filedesc */
    int fifosize;         /* buffer size appropriately rounded down */
    int fifohead;         /* index of next byte to get from file */
    int fifotail;         /* index of next byte the ugen will read */
    int eof;              /* true if fifohead has stopped changing */
    int sigcountdown;     /* counter for signalling child for more data */
    int sigperiod;        /* number of ticks per signal */
    int filetype;         /* writesf~ only; type of file to create */
    int itemswritten;     /* writesf~ only; items writen */
    int swap;             /* writesf~ only; true if byte swapping */
    float f;              /* writesf~ only; scalar for signal inlet */
    pthread_mutex_t mutex;
    pthread_cond_t requestcondition;
    pthread_cond_t answercondition;
    pthread_t childthread;
};


/************** the child thread which performs file I/O ***********/

#if 1
#define sfread_cond_wait pthread_cond_wait
#define sfread_cond_signal pthread_cond_signal
#else
#include <sys/time.h>    /* debugging version... */
#include <sys/types.h>
static void readsf_fakewait(pthread_mutex_t *b) {
    struct timeval timout;
    timout.tv_sec = 0;
    timout.tv_usec = 1000000;
    pthread_mutex_unlock(b);
    select(0, 0, 0, 0, &timout);
    pthread_mutex_lock(b);
}

#define sfread_cond_wait(a,b) readsf_fakewait(b)
#define sfread_cond_signal(a)
#endif

static void *readsf_child_main(void *zz) {
    t_readsf *x = (t_readsf *)zz;
    pthread_mutex_lock(&x->mutex);
    while (1) {
        int fd, fifohead;
        char *buf;
        if (x->requestcode == REQUEST_NOTHING) {
            sfread_cond_signal(&x->answercondition);
            sfread_cond_wait(&x->requestcondition, &x->mutex);
        } else if (x->requestcode == REQUEST_OPEN) {
            int sysrtn, wantbytes;
            /* copy file stuff out of the data structure so we can
               relinquish the mutex while we're in open_soundfile(). */
            long onsetframes = x->onsetframes;
            t_param p;
            p.bytelimit = 0x7fffffff;
            int skipheaderbytes = x->skipheaderbytes;
            p.bytespersample = x->p.bytespersample;
            p.bigendian = x->p.bigendian;
            /* alter the request code so that an ensuing "open" will get noticed. */
            x->requestcode = REQUEST_BUSY;
            x->fileerror = 0;
            /* if there's already a file open, close it */
            if (x->fd >= 0) {
                fd = x->fd;
                pthread_mutex_unlock(&x->mutex);
                close (fd);
                pthread_mutex_lock(&x->mutex);
                x->fd = -1;
                if (x->requestcode != REQUEST_BUSY) goto lost;
            }
            /* open the soundfile with the mutex unlocked */
            pthread_mutex_unlock(&x->mutex);
            fd = open_soundfile(canvas_getdir(x->canvas)->name, x->filename, skipheaderbytes, &p, onsetframes);
            pthread_mutex_lock(&x->mutex);
            /* copy back into the instance structure. */
            x->p = p;
            x->fd = fd;
            if (fd < 0) {
                x->fileerror = errno;
                x->eof = 1;
                goto lost;
            }
            /* check if another request has been made; if so, field it */
            if (x->requestcode != REQUEST_BUSY) goto lost;
            x->fifohead = 0;
            /* set fifosize from bufsize.  fifosize must be a multiple of the number of bytes eaten for each DSP
               tick.  We pessimistically assume MAXVECSIZE samples per tick since that could change.  There could be a
               problem here if the vector size increases while a soundfile is being played...  */
            x->fifosize = x->bufsize - (x->bufsize % (x->p.bytesperchannel() * MAXVECSIZE));
            /* arrange for the "request" condition to be signalled 16 times per buffer */
            x->sigcountdown = x->sigperiod = x->fifosize / (16 * x->p.bytesperchannel() * x->vecsize);
            /* in a loop, wait for the fifo to get hungry and feed it */
            while (x->requestcode == REQUEST_BUSY) {
                int fifosize = x->fifosize;
                if (x->eof) break;
                if (x->fifohead >= x->fifotail) {
                    /* if the head is >= the tail, we can immediately read to the end of the fifo. Unless, that is, we
                       would read all the way to the end of the buffer and the "tail" is zero; this would fill the buffer completely
                       which isn't allowed because you can't tell a completely full buffer from an empty one. */
                    if (x->fifotail || (fifosize - x->fifohead > READSIZE)) {
                        wantbytes = min(min(fifosize - x->fifohead, READSIZE),(int)x->p.bytelimit);
                    } else {
                        sfread_cond_signal(&x->answercondition);
                        sfread_cond_wait(&x->requestcondition, &x->mutex);
                        continue;
                    }
                } else {
                    /* otherwise check if there are at least READSIZE bytes to read.  If not, wait and loop back. */
                    wantbytes = x->fifotail - x->fifohead - 1;
                    if (wantbytes < READSIZE) {
                        sfread_cond_signal(&x->answercondition);
                        sfread_cond_wait(&x->requestcondition, &x->mutex);
                        continue;
                    } else wantbytes = READSIZE;
                    wantbytes = min(wantbytes,(int)x->p.bytelimit);
                }
                fd = x->fd;
                buf = x->buf;
                fifohead = x->fifohead;
                pthread_mutex_unlock(&x->mutex);
                sysrtn = read(fd, buf + fifohead, wantbytes);
                pthread_mutex_lock(&x->mutex);
                if (x->requestcode != REQUEST_BUSY) break;
                if      (sysrtn < 0)  {x->fileerror = errno; break;}
                else if (sysrtn == 0) {x->eof = 1; break;}
                else {
                    x->fifohead  += sysrtn;
                    x->p.bytelimit -= sysrtn;
                    if (x->p.bytelimit <= 0) {x->eof = 1; break;}
                    if (x->fifohead == fifosize) x->fifohead = 0;
                }
                /* signal parent in case it's waiting for data */
                sfread_cond_signal(&x->answercondition);
            }
        lost:
            if (x->requestcode == REQUEST_BUSY) x->requestcode = REQUEST_NOTHING;
            /* fell out of read loop: close file if necessary, set EOF and signal once more */
            if (x->fd >= 0) {
                fd = x->fd;
                pthread_mutex_unlock(&x->mutex);
                close (fd);
                pthread_mutex_lock(&x->mutex);
                x->fd = -1;
            }
            sfread_cond_signal(&x->answercondition);
        } else if (x->requestcode == REQUEST_CLOSE || x->requestcode == REQUEST_QUIT) {
            if (x->fd >= 0) {
                fd = x->fd;
                pthread_mutex_unlock(&x->mutex);
                close (fd);
                pthread_mutex_lock(&x->mutex);
                x->fd = -1;
            }
            x->requestcode = REQUEST_NOTHING;
            sfread_cond_signal(&x->answercondition);
        }
        if (x->requestcode == REQUEST_QUIT) break;
    }
    pthread_mutex_unlock(&x->mutex);
    return 0;
}

/******** the object proper runs in the calling (parent) thread ****/

static void readsf_tick(t_readsf *x);

static void *readsf_new(t_floatarg fnchannels, t_floatarg fbufsize) {
    int nchannels = int(fnchannels), bufsize = int(fbufsize);
    if (nchannels < 1) nchannels = 1;
    else if (nchannels > MAXSFCHANS) nchannels = MAXSFCHANS;
    if (bufsize <= 0) bufsize = DEFBUFPERCHAN * nchannels;
    else if (bufsize < MINBUFSIZE) bufsize = MINBUFSIZE;
    else if (bufsize > MAXBUFSIZE) bufsize = MAXBUFSIZE;
    char *buf = (char *)getbytes(bufsize);
    if (!buf) return 0;
    t_readsf *x = (t_readsf *)pd_new(readsf_class);
    for (int i=0; i<nchannels; i++) outlet_new(x,gensym("signal"));
    x->noutlets = nchannels;
    x->bangout = outlet_new(x,&s_bang);
    pthread_mutex_init(&x->mutex, 0);
    pthread_cond_init(&x->requestcondition, 0);
    pthread_cond_init(&x->answercondition, 0);
    x->vecsize = MAXVECSIZE;
    x->state = STATE_IDLE;
    x->clock = clock_new(x, (t_method)readsf_tick);
    x->canvas = canvas_getcurrent();
    x->p.bytespersample = 2;
    x->p.nchannels = 1;
    x->fd = -1;
    x->buf = buf;
    x->bufsize = bufsize;
    x->fifosize = x->fifohead = x->fifotail = x->requestcode = 0;
    pthread_create(&x->childthread, 0, readsf_child_main, x);
    return x;
}

static void readsf_tick(t_readsf *x) {x->bangout->send();}

static t_int *readsf_perform(t_int *w) {
    t_readsf *x = (t_readsf *)(w[1]);
    int vecsize = x->vecsize, noutlets = x->noutlets;
    if (x->state == STATE_STREAM) {
        int wantbytes;
        pthread_mutex_lock(&x->mutex);
        wantbytes = vecsize * x->p.bytesperchannel();
        while (!x->eof && x->fifohead >= x->fifotail && x->fifohead < x->fifotail + wantbytes-1) {
            sfread_cond_signal(&x->requestcondition);
            sfread_cond_wait(&x->answercondition, &x->mutex);
        }
        if (x->eof && x->fifohead >= x->fifotail && x->fifohead < x->fifotail + wantbytes-1) {
            if (x->fileerror) {
                error("dsp: %s: %s", x->filename, x->fileerror == EIO ? "unknown or bad header format" : strerror(x->fileerror));
            }
            clock_delay(x->clock, 0);
            x->state = STATE_IDLE;
            /* if there's a partial buffer left, copy it out. */
            int xfersize = (x->fifohead - x->fifotail + 1) / x->p.bytesperchannel();
            if (xfersize) {
                soundfile_xferin(x->p.nchannels, noutlets, x->outvec, 0,
                    (unsigned char *)(x->buf + x->fifotail), xfersize, x->p.bytespersample, x->p.bigendian);
                vecsize -= xfersize;
            }
            /* then zero out the (rest of the) output */
            for (int i=0; i<noutlets; i++) {
		float *fp = x->outvec[i] + xfersize;
                for (int j=vecsize; j--; ) *fp++ = 0;
	    }
            sfread_cond_signal(&x->requestcondition);
            pthread_mutex_unlock(&x->mutex);
            return w+2;
        }
        soundfile_xferin(x->p.nchannels, noutlets, x->outvec, 0, (unsigned char *)(x->buf + x->fifotail), vecsize, x->p.bytespersample, x->p.bigendian);
        x->fifotail += wantbytes;
        if (x->fifotail >= x->fifosize) x->fifotail = 0;
        if ((--x->sigcountdown) <= 0) {
            sfread_cond_signal(&x->requestcondition);
            x->sigcountdown = x->sigperiod;
        }
        pthread_mutex_unlock(&x->mutex);
    } else {
        for (int i=0; i<noutlets; i++) {
	    float *fp = x->outvec[i];
            for (int j=vecsize; j--; ) *fp++=0;
	}
    }
    return w+2;
}

static void readsf_start(t_readsf *x) {
    /* start making output.  If we're in the "startup" state change
    to the "running" state. */
    if (x->state == STATE_STARTUP) x->state = STATE_STREAM;
    else error("readsf: start requested with no prior 'open'");
}

static void readsf_stop(t_readsf *x) {
    /* LATER rethink whether you need the mutex just to set a variable? */
    pthread_mutex_lock(&x->mutex);
    x->state = STATE_IDLE;
    x->requestcode = REQUEST_CLOSE;
    sfread_cond_signal(&x->requestcondition);
    pthread_mutex_unlock(&x->mutex);
}

static void readsf_float(t_readsf *x, t_floatarg f) {
    if (f != 0) readsf_start(x); else readsf_stop(x);
}

/* open method.  Called as: open filename [skipframes headersize channels bytespersample endianness]
   (if headersize is zero, header is taken to be automatically detected; thus, use the special "-1" to mean a truly headerless file.) */
static void readsf_open(t_readsf *x, t_symbol *s, int argc, t_atom *argv) {
    t_symbol *filesym = atom_getsymbolarg(0, argc, argv);
    t_float onsetframes = atom_getfloatarg(1, argc, argv);
    t_float headerbytes = atom_getfloatarg(2, argc, argv);
    t_float channels = atom_getfloatarg(3, argc, argv);
    t_float bytespersample = atom_getfloatarg(4, argc, argv);
    t_symbol *endian = atom_getsymbolarg(5, argc, argv);
    if (!*filesym->name) return;
    pthread_mutex_lock(&x->mutex);
    x->requestcode = REQUEST_OPEN;
    x->filename = filesym->name;
    x->fifotail = 0;
    x->fifohead = 0;
    if      (*endian->name == 'b') x->p.bigendian = 1;
    else if (*endian->name == 'l') x->p.bigendian = 0;
    else if (*endian->name) error("endianness neither 'b' nor 'l'");
    else x->p.bigendian = garray_ambigendian();
    x->onsetframes = max(long(onsetframes),0L);
    x->skipheaderbytes = int(headerbytes > 0 ? headerbytes : headerbytes == 0 ? -1 : 0);
    x->p.nchannels = max(int(channels),1);
    x->p.bytespersample = max(int(bytespersample),2);
    x->eof = 0;
    x->fileerror = 0;
    x->state = STATE_STARTUP;
    sfread_cond_signal(&x->requestcondition);
    pthread_mutex_unlock(&x->mutex);
}

static void readsf_dsp(t_readsf *x, t_signal **sp) {
    int i, noutlets = x->noutlets;
    pthread_mutex_lock(&x->mutex);
    x->vecsize = sp[0]->n;
    x->sigperiod = (x->fifosize / (x->p.bytesperchannel() * x->vecsize));
    for (i = 0; i < noutlets; i++) x->outvec[i] = sp[i]->v;
    pthread_mutex_unlock(&x->mutex);
    dsp_add(readsf_perform, 1, x);
}

static void readsf_print(t_readsf *x) {
    post("state %d", x->state);
    post("fifo head %d", x->fifohead);
    post("fifo tail %d", x->fifotail);
    post("fifo size %d", x->fifosize);
    post("fd %d", x->fd);
    post("eof %d", x->eof);
}

static void readsf_free(t_readsf *x) {
    /* request QUIT and wait for acknowledge */
    void *threadrtn;
    pthread_mutex_lock(&x->mutex);
    x->requestcode = REQUEST_QUIT;
    sfread_cond_signal(&x->requestcondition);
    while (x->requestcode != REQUEST_NOTHING) {
        sfread_cond_signal(&x->requestcondition);
        sfread_cond_wait(&x->answercondition, &x->mutex);
    }
    pthread_mutex_unlock(&x->mutex);
    if (pthread_join(x->childthread, &threadrtn)) error("readsf_free: join failed");
    pthread_cond_destroy(&x->requestcondition);
    pthread_cond_destroy(&x->answercondition);
    pthread_mutex_destroy(&x->mutex);
    free(x->buf);
    clock_free(x->clock);
}

static void readsf_setup() {
    t_class *c = readsf_class = class_new2("readsf~", (t_newmethod)readsf_new, (t_method)readsf_free, sizeof(t_readsf), 0, "FF");
    class_addfloat(c, (t_method)readsf_float);
    class_addmethod2(c, (t_method)readsf_start, "start","");
    class_addmethod2(c, (t_method)readsf_stop, "stop","");
    class_addmethod2(c, (t_method)readsf_dsp, "dsp","");
    class_addmethod2(c, (t_method)readsf_open, "open","*");
    class_addmethod2(c, (t_method)readsf_print, "print","");
}

/******************************* writesf *******************/

static t_class *writesf_class;
typedef t_readsf t_writesf; /* just re-use the structure */

/************** the child thread which performs file I/O ***********/

static void *writesf_child_main(void *zz) {
    t_writesf *x = (t_writesf*)zz;
    pthread_mutex_lock(&x->mutex);
    while (1) {
        if (x->requestcode == REQUEST_NOTHING) {
            sfread_cond_signal(&x->answercondition);
            sfread_cond_wait(&x->requestcondition, &x->mutex);
        } else if (x->requestcode == REQUEST_OPEN) {
            int fd, sysrtn, writebytes;
            /* copy file stuff out of the data structure so we can relinquish the mutex while we're in open_soundfile(). */
            int filetype = x->filetype;
            char *filename = x->filename;
            t_canvas *canvas = x->canvas;
            float samplerate = x->samplerate;
            /* alter the request code so that an ensuing "open" will get noticed. */
            x->requestcode = REQUEST_BUSY;
            x->fileerror = 0;
            /* if there's already a file open, close it. This should never happen since
		writesf_open() calls stop if needed and then waits until we're idle. */
            if (x->fd >= 0) {
                int bytesperframe = x->p.bytesperchannel();
                char *filename = x->filename;
                int fd = x->fd;
                int filetype = x->filetype;
                int itemswritten = x->itemswritten;
                int swap = x->swap;
                pthread_mutex_unlock(&x->mutex);
                soundfile_finishwrite(x, filename, fd, filetype, 0x7fffffff, itemswritten, bytesperframe, swap);
                close (fd);
                pthread_mutex_lock(&x->mutex);
                x->fd = -1;
                if (x->requestcode != REQUEST_BUSY) continue;
            }
            /* open the soundfile with the mutex unlocked */
            pthread_mutex_unlock(&x->mutex);
            fd = create_soundfile(canvas, filename, filetype, 0,
                    x->p.bytespersample, x->p.bigendian, x->p.nchannels, garray_ambigendian() != x->p.bigendian, samplerate);
            pthread_mutex_lock(&x->mutex);
            if (fd < 0) {
                x->fd = -1;
                x->eof = 1;
                x->fileerror = errno;
                x->requestcode = REQUEST_NOTHING;
                continue;
            }
            /* check if another request has been made; if so, field it */
            if (x->requestcode != REQUEST_BUSY) continue;
            x->fd = fd;
            x->fifotail = 0;
            x->itemswritten = 0;
            x->swap = garray_ambigendian() != x->p.bigendian;
            /* in a loop, wait for the fifo to have data and write it to disk */
            while (x->requestcode == REQUEST_BUSY || (x->requestcode == REQUEST_CLOSE && x->fifohead != x->fifotail)) {
                int fifosize = x->fifosize, fifotail;
                char *buf = x->buf;
                /* if the head is < the tail, we can immediately write
                   from tail to end of fifo to disk; otherwise we hold off
                   writing until there are at least WRITESIZE bytes in the
                   buffer */
                if (x->fifohead < x->fifotail || x->fifohead >= x->fifotail + WRITESIZE
                || (x->requestcode == REQUEST_CLOSE && x->fifohead != x->fifotail)) {
                    writebytes = (x->fifohead < x->fifotail ? fifosize : x->fifohead) - x->fifotail;
                    if (writebytes > READSIZE) writebytes = READSIZE;
                } else {
                    sfread_cond_signal(&x->answercondition);
                    sfread_cond_wait(&x->requestcondition,&x->mutex);
                    continue;
                }
                fifotail = x->fifotail;
                fd = x->fd;
                pthread_mutex_unlock(&x->mutex);
                sysrtn = write(fd, buf + fifotail, writebytes);
                pthread_mutex_lock(&x->mutex);
                if (x->requestcode != REQUEST_BUSY && x->requestcode != REQUEST_CLOSE) break;
                if (sysrtn < writebytes) {x->fileerror = errno; break;}
                else {
                    x->fifotail += sysrtn;
                    if (x->fifotail == fifosize) x->fifotail = 0;
                }
                x->itemswritten += sysrtn / x->p.bytesperchannel();
                /* signal parent in case it's waiting for data */
                sfread_cond_signal(&x->answercondition);
            }
        } else if (x->requestcode == REQUEST_CLOSE || x->requestcode == REQUEST_QUIT) {
            if (x->fd >= 0) {
                int bytesperframe = x->p.bytesperchannel();
                char *filename = x->filename;
                int fd = x->fd;
                int filetype = x->filetype;
                int itemswritten = x->itemswritten;
                int swap = x->swap;
                pthread_mutex_unlock(&x->mutex);
                soundfile_finishwrite(x, filename, fd, filetype, 0x7fffffff, itemswritten, bytesperframe, swap);
                close(fd);
                pthread_mutex_lock(&x->mutex);
                x->fd = -1;
            }
            x->requestcode = REQUEST_NOTHING;
            sfread_cond_signal(&x->answercondition);
        }
        if (x->requestcode == REQUEST_QUIT) break;
    }
    pthread_mutex_unlock(&x->mutex);
    return 0;
}

/******** the object proper runs in the calling (parent) thread ****/

static void *writesf_new(t_floatarg fnchannels, t_floatarg fbufsize) {
    int nchannels = int(fnchannels), bufsize = int(fbufsize), i;
    if (nchannels < 1) nchannels = 1;
    else if (nchannels > MAXSFCHANS) nchannels = MAXSFCHANS;
    if (bufsize <= 0) bufsize = DEFBUFPERCHAN * nchannels;
    else if (bufsize < MINBUFSIZE) bufsize = MINBUFSIZE;
    else if (bufsize > MAXBUFSIZE) bufsize = MAXBUFSIZE;
    char *buf = (char *)getbytes(bufsize);
    if (!buf) return 0;
    t_writesf *x = (t_writesf *)pd_new(writesf_class);
    for (i = 1; i < nchannels; i++) inlet_new(x,x, &s_signal, &s_signal);
    x->f = 0;
    x->p.nchannels = nchannels;
    pthread_mutex_init(&x->mutex, 0);
    pthread_cond_init(&x->requestcondition, 0);
    pthread_cond_init(&x->answercondition, 0);
    x->vecsize = MAXVECSIZE;
    x->insamplerate = x->samplerate = 0;
    x->state = STATE_IDLE;
    x->clock = 0; /* no callback needed here */
    x->canvas = canvas_getcurrent();
    x->p.bytespersample = 2;
    x->fd = -1;
    x->buf = buf;
    x->bufsize = bufsize;
    x->fifosize = x->fifohead = x->fifotail = x->requestcode = 0;
    pthread_create(&x->childthread, 0, writesf_child_main, x);
    return x;
}

static t_int *writesf_perform(t_int *w) {
    t_writesf *x = (t_writesf *)(w[1]);
    if (x->state == STATE_STREAM) {
        int wantbytes;
        pthread_mutex_lock(&x->mutex);
        wantbytes = x->vecsize * x->p.bytesperchannel();
        while (x->fifotail > x->fifohead && x->fifotail < x->fifohead + wantbytes + 1) {
            sfread_cond_signal(&x->requestcondition);
            sfread_cond_wait(&x->answercondition, &x->mutex);
	    /* resync local cariables -- bug fix thanks to Shahrokh */
	    wantbytes = x->vecsize * x->p.bytesperchannel();
        }
        soundfile_xferout(x->p.nchannels, x->outvec, (unsigned char *)(x->buf + x->fifohead), x->vecsize, 0, x->p.bytespersample, x->p.bigendian, 1.);
        x->fifohead += wantbytes;
        if (x->fifohead >= x->fifosize) x->fifohead = 0;
        if ((--x->sigcountdown) <= 0) {
            sfread_cond_signal(&x->requestcondition);
            x->sigcountdown = x->sigperiod;
        }
        pthread_mutex_unlock(&x->mutex);
    }
    return w+2;
}

static void writesf_start(t_writesf *x) {
    /* start making output.  If we're in the "startup" state change to the "running" state. */
    if (x->state == STATE_STARTUP) x->state = STATE_STREAM;
    else error("writesf: start requested with no prior 'open'");
}

static void writesf_stop(t_writesf *x) {
    /* LATER rethink whether you need the mutex just to set a Svariable? */
    pthread_mutex_lock(&x->mutex);
    x->state = STATE_IDLE;
    x->requestcode = REQUEST_CLOSE;
    sfread_cond_signal(&x->requestcondition);
    pthread_mutex_unlock(&x->mutex);
}

/* open method.  Called as: open [args] filename with args as in soundfiler_writeargparse(). */
static void writesf_open(t_writesf *x, t_symbol *s, int argc, t_atom *argv) {
    t_symbol *filesym;
    int filetype, bytespersample, swap, bigendian, normalize;
    long onset, nframes;
    float samplerate;
    if (x->state != STATE_IDLE) writesf_stop(x);
    if (soundfiler_writeargparse(x, &argc, &argv, &filesym, &filetype, &bytespersample, &swap, &bigendian,
        &normalize, &onset, &nframes, &samplerate)) {
        error("writesf~: usage: open [-bytes [234]] [-wave,-nextstep,-aiff] ...");
        post("... [-big,-little] [-rate ####] filename");
        return;
    }
    if (normalize || onset || (nframes != 0x7fffffff))
        error("normalize/onset/nframes argument to writesf~: ignored");
    if (argc) error("extra argument(s) to writesf~: ignored");
    pthread_mutex_lock(&x->mutex);
    while (x->requestcode != REQUEST_NOTHING) {
        sfread_cond_signal(&x->requestcondition);
        sfread_cond_wait(&x->answercondition, &x->mutex);
    }
    x->p.bytespersample = max(bytespersample,2);
    x->swap = swap;
    x->p.bigendian = bigendian;
    x->filename = filesym->name;
    x->filetype = filetype;
    x->itemswritten = 0;
    x->requestcode = REQUEST_OPEN;
    x->fifotail = 0;
    x->fifohead = 0;
    x->eof = 0;
    x->fileerror = 0;
    x->state = STATE_STARTUP;
    x->samplerate = samplerate>0 ? samplerate : x->insamplerate>0 ? x->insamplerate : sys_getsr();
    /* set fifosize from bufsize.  fifosize must be a multiple of the number of bytes eaten for each DSP tick.  */
    x->fifosize = x->bufsize - x->bufsize % (x->p.bytesperchannel() * MAXVECSIZE);
    /* arrange for the "request" condition to be signalled 16 times per buffer */
    x->sigcountdown = x->sigperiod = x->fifosize / (16 * x->p.bytesperchannel() * x->vecsize);
    sfread_cond_signal(&x->requestcondition);
    pthread_mutex_unlock(&x->mutex);
}

static void writesf_dsp(t_writesf *x, t_signal **sp) {
    int ninlets = x->p.nchannels;
    pthread_mutex_lock(&x->mutex);
    x->vecsize = sp[0]->n;
    x->sigperiod = x->fifosize / (x->p.bytesperchannel() * x->vecsize);
    for (int i=0; i<ninlets; i++) x->outvec[i] = sp[i]->v;
    x->insamplerate = sp[0]->sr;
    pthread_mutex_unlock(&x->mutex);
    dsp_add(writesf_perform, 1, x);
}

static void writesf_print(t_writesf *x) {
    post("state %d", x->state);
    post("fifo head %d", x->fifohead);
    post("fifo tail %d", x->fifotail);
    post("fifo size %d", x->fifosize);
    post("fd %d", x->fd);
    post("eof %d", x->eof);
}

static void writesf_free(t_writesf *x) {
    /* request QUIT and wait for acknowledge */
    void *threadrtn;
    pthread_mutex_lock(&x->mutex);
    x->requestcode = REQUEST_QUIT;
    /* post("stopping writesf thread..."); */
    sfread_cond_signal(&x->requestcondition);
    while (x->requestcode != REQUEST_NOTHING) {
        /* post("signalling..."); */
        sfread_cond_signal(&x->requestcondition);
        sfread_cond_wait(&x->answercondition, &x->mutex);
    }
    pthread_mutex_unlock(&x->mutex);
    if (pthread_join(x->childthread, &threadrtn)) error("writesf_free: join failed");
    /* post("... done."); */
    pthread_cond_destroy(&x->requestcondition);
    pthread_cond_destroy(&x->answercondition);
    pthread_mutex_destroy(&x->mutex);
    free(x->buf);
}

static void writesf_setup() {
    t_class *c = writesf_class = class_new2("writesf~", (t_newmethod)writesf_new, (t_method)writesf_free, sizeof(t_writesf), 0, "FF");
    class_addmethod2(c, (t_method)writesf_start, "start", "");
    class_addmethod2(c, (t_method)writesf_stop, "stop", "");
    class_addmethod2(c, (t_method)writesf_dsp, "dsp", "");
    class_addmethod2(c, (t_method)writesf_open, "open", "*");
    class_addmethod2(c, (t_method)writesf_print, "print", "");
    CLASS_MAINSIGNALIN(c, t_writesf, f);
}

/* ------------------------ global setup routine ------------------------- */

void d_soundfile_setup() {
    if (sizeof(uint16)!=2) bug("uint16 should really be 16 bits");
    if (sizeof(uint32)!=4) bug("uint32 should really be 32 bits");
    soundfiler_setup();
    readsf_setup();
    writesf_setup();
}
