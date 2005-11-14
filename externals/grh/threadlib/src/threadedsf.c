/* 
* 
* threadedsf
*
* this is a little bit hacked version of the
* threaded soundfiler of pd_devel_0.38 by Tim Blechmann
*
* (c) 2005, Georg Holzmann, <grh@mur.at>
*/

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


#include "threadlib.h"

#ifndef MSW
#include <unistd.h>
#include <fcntl.h>
#endif
#ifdef MSW
#include <io.h>
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "g_canvas.h"

#define MAXSFCHANS 64


// forward declarations

struct _garray
{
  t_gobj x_gobj;
  t_scalar *x_scalar;     /* scalar "containing" the array */
  t_glist *x_glist;       /* containing glist */
  t_symbol *x_name;       /* unexpanded name (possibly with leading '$') */
  t_symbol *x_realname;   /* expanded name (symbol we're bound to) */
  char x_usedindsp;       /* true if some DSP routine is using this */
  char x_saveit;          /* true if we should save this with parent */
  char x_listviewing;     /* true if list view window is open */
};

t_array *garray_getarray(t_garray *x);
int garray_ambigendian(void);



/***************** soundfile header structures ************************/

typedef unsigned short uint16;
typedef unsigned int uint32; /* long isn't 32-bit on amd64 */

#define FORMAT_WAVE 0
#define FORMAT_AIFF 1
#define FORMAT_NEXT 2

/* the NeXTStep sound header structure; can be big or little endian  */

typedef struct _nextstep
{
    char ns_fileid[4];      /* magic number '.snd' if file is big-endian */
    uint32 ns_onset;        /* byte offset of first sample */
    uint32 ns_length;       /* length of sound in bytes */
    uint32 ns_format;        /* format; see below */
    uint32 ns_sr;           /* sample rate */
    uint32 ns_nchans;       /* number of channels */
    char ns_info[4];        /* comment */
} t_nextstep;

#define NS_FORMAT_LINEAR_16     3
#define NS_FORMAT_LINEAR_24     4
#define NS_FORMAT_FLOAT         6
#define SCALE (1./(1024. * 1024. * 1024. * 2.))

/* the WAVE header.  All Wave files are little endian.  We assume
    the "fmt" chunk comes first which is usually the case but perhaps not
    always; same for AIFF and the "COMM" chunk.   */

typedef struct _wave
{
    char  w_fileid[4];              /* chunk id 'RIFF'            */
    uint32 w_chunksize;             /* chunk size                 */
    char  w_waveid[4];              /* wave chunk id 'WAVE'       */
    char  w_fmtid[4];               /* format chunk id 'fmt '     */
    uint32 w_fmtchunksize;          /* format chunk size          */
    uint16  w_fmttag;               /* format tag (WAV_INT etc)   */
    uint16  w_nchannels;            /* number of channels         */
    uint32 w_samplespersec;         /* sample rate in hz          */
    uint32 w_navgbytespersec;       /* average bytes per second   */
    uint16  w_nblockalign;          /* number of bytes per frame  */
    uint16  w_nbitspersample;       /* number of bits in a sample */
    char  w_datachunkid[4];         /* data chunk id 'data'       */
    uint32 w_datachunksize;         /* length of data chunk       */
} t_wave;

typedef struct _fmt         /* format chunk */
{
    uint16 f_fmttag;                /* format tag, 1 for PCM      */
    uint16 f_nchannels;             /* number of channels         */
    uint32 f_samplespersec;         /* sample rate in hz          */
    uint32 f_navgbytespersec;       /* average bytes per second   */
    uint16 f_nblockalign;           /* number of bytes per frame  */
    uint16 f_nbitspersample;        /* number of bits in a sample */
} t_fmt;

typedef struct _wavechunk           /* ... and the last two items */
{
    char  wc_id[4];                 /* data chunk id, e.g., 'data' or 'fmt ' */
    uint32 wc_size;                 /* length of data chunk       */
} t_wavechunk;

#define WAV_INT 1
#define WAV_FLOAT 3

/* the AIFF header.  I'm assuming AIFC is compatible but don't really know
    that. */

typedef struct _datachunk
{
    char  dc_id[4];                 /* data chunk id 'SSND'       */
    uint32 dc_size;                 /* length of data chunk       */
} t_datachunk;

typedef struct _comm
{
    uint16 c_nchannels;             /* number of channels         */
    uint16 c_nframeshi;             /* # of sample frames (hi)    */
    uint16 c_nframeslo;             /* # of sample frames (lo)    */
    uint16 c_bitspersamp;           /* bits per sample            */
    unsigned char c_samprate[10];   /* sample rate, 80-bit float! */
} t_comm;

    /* this version is more convenient for writing them out: */
typedef struct _aiff
{
    char  a_fileid[4];              /* chunk id 'FORM'            */
    uint32 a_chunksize;             /* chunk size                 */
    char  a_aiffid[4];              /* aiff chunk id 'AIFF'       */
    char  a_fmtid[4];               /* format chunk id 'COMM'     */
    uint32 a_fmtchunksize;          /* format chunk size, 18      */
    uint16 a_nchannels;             /* number of channels         */
    uint16 a_nframeshi;             /* # of sample frames (hi)    */
    uint16 a_nframeslo;             /* # of sample frames (lo)    */
    uint16 a_bitspersamp;           /* bits per sample            */
    unsigned char a_samprate[10];   /* sample rate, 80-bit float! */
} t_aiff;

#define AIFFHDRSIZE 38      /* probably not what sizeof() gives */


#define AIFFPLUS (AIFFHDRSIZE + 8)  /* header size including first chunk hdr */

#define WHDR1 sizeof(t_nextstep)
#define WHDR2 (sizeof(t_wave) > WHDR1 ? sizeof (t_wave) : WHDR1)
#define WRITEHDRSIZE (AIFFPLUS > WHDR2 ? AIFFPLUS : WHDR2)

#define READHDRSIZE (16 > WHDR2 + 2 ? 16 : WHDR2 + 2)

#define OBUFSIZE MAXPDSTRING  /* assume MAXPDSTRING is bigger than headers */

#ifdef MSW
#include <fcntl.h>
#define BINCREATE _O_WRONLY | _O_CREAT | _O_TRUNC | _O_BINARY
#else
#define BINCREATE O_WRONLY | O_CREAT | O_TRUNC
#endif

/* this routine returns 1 if the high order byte comes at the lower
address on our architecture (big-endianness.).  It's 1 for Motorola,
0 for Intel: */

extern int garray_ambigendian(void);

/* byte swappers */

static uint32 swap4(uint32 n, int doit)
{
    if (doit)
        return (((n & 0xff) << 24) | ((n & 0xff00) << 8) |
            ((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24));
    else return (n);
}

static uint16 swap2(uint32 n, int doit)
{
    if (doit)
        return (((n & 0xff) << 8) | ((n & 0xff00) >> 8));
    else return (n);
}

static void swapstring(char *foo, int doit)
{
    if (doit)
    {
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

int open_soundfile(const char *dirname, const char *filename, int headersize,
    int *p_bytespersamp, int *p_bigendian, int *p_nchannels, long *p_bytelimit,
    long skipframes)
{
    char buf[OBUFSIZE], *bufptr;
    int fd, format, nchannels, bigendian, bytespersamp, swap, sysrtn;
    long bytelimit = 0x7fffffff;
    errno = 0;
    fd = open_via_path(dirname, filename,
        "", buf, &bufptr, MAXPDSTRING, 1);
    if (fd < 0)
        return (-1);
    if (headersize >= 0) /* header detection overridden */
    {
        bigendian = *p_bigendian;
        nchannels = *p_nchannels;
        bytespersamp = *p_bytespersamp;
        bytelimit = *p_bytelimit;
    }
    else
    {
        int bytesread = read(fd, buf, READHDRSIZE);
        int format;
        if (bytesread < 4)
            goto badheader;
        if (!strncmp(buf, ".snd", 4))
            format = FORMAT_NEXT, bigendian = 1;
        else if (!strncmp(buf, "dns.", 4))
            format = FORMAT_NEXT, bigendian = 0;
        else if (!strncmp(buf, "RIFF", 4))
        {
            if (bytesread < 12 || strncmp(buf + 8, "WAVE", 4))
                goto badheader;
            format = FORMAT_WAVE, bigendian = 0;
        }
        else if (!strncmp(buf, "FORM", 4))
        {
            if (bytesread < 12 || strncmp(buf + 8, "AIFF", 4))
                goto badheader;
            format = FORMAT_AIFF, bigendian = 1;
        }
        else
            goto badheader;
        swap = (bigendian != garray_ambigendian());
        if (format == FORMAT_NEXT)   /* nextstep header */
        {
            uint32 param;
            if (bytesread < (int)sizeof(t_nextstep))
                goto badheader;
            nchannels = swap4(((t_nextstep *)buf)->ns_nchans, swap);
            format = swap4(((t_nextstep *)buf)->ns_format, swap);
            headersize = swap4(((t_nextstep *)buf)->ns_onset, swap);
            if (format == NS_FORMAT_LINEAR_16)
                bytespersamp = 2;
            else if (format == NS_FORMAT_LINEAR_24)
                bytespersamp = 3;
            else if (format == NS_FORMAT_FLOAT)
                bytespersamp = 4;
            else goto badheader;
            bytelimit = 0x7fffffff;
        }
        else if (format == FORMAT_WAVE)     /* wave header */
        {
               /*  This is awful.  You have to skip over chunks,
               except that if one happens to be a "fmt" chunk, you want to
               find out the format from that one.  The case where the
               "fmt" chunk comes after the audio isn't handled. */
            headersize = 12;
            if (bytesread < 20)
                goto badheader;
                /* First we guess a number of channels, etc., in case there's
                no "fmt" chunk to follow. */
            nchannels = 1;
            bytespersamp = 2;
                /* copy the first chunk header to beginnning of buffer. */
            memcpy(buf, buf + headersize, sizeof(t_wavechunk));
            /* post("chunk %c %c %c %c",
                    ((t_wavechunk *)buf)->wc_id[0],
                    ((t_wavechunk *)buf)->wc_id[1],
                    ((t_wavechunk *)buf)->wc_id[2],
                    ((t_wavechunk *)buf)->wc_id[3]); */
                /* read chunks in loop until we get to the data chunk */
            while (strncmp(((t_wavechunk *)buf)->wc_id, "data", 4))
            {
                long chunksize = swap4(((t_wavechunk *)buf)->wc_size,
                    swap), seekto = headersize + chunksize + 8, seekout;
                
                if (!strncmp(((t_wavechunk *)buf)->wc_id, "fmt ", 4))
                {
                    long commblockonset = headersize + 8;
                    seekout = lseek(fd, commblockonset, SEEK_SET);
                    if (seekout != commblockonset)
                        goto badheader;
                    if (read(fd, buf, sizeof(t_fmt)) < (int) sizeof(t_fmt))
                            goto badheader;
                    nchannels = swap2(((t_fmt *)buf)->f_nchannels, swap);
                    format = swap2(((t_fmt *)buf)->f_nbitspersample, swap);
                    if (format == 16)
                        bytespersamp = 2;
                    else if (format == 24)
                        bytespersamp = 3;
                    else if (format == 32)
                        bytespersamp = 4;
                    else goto badheader;
                }
                seekout = lseek(fd, seekto, SEEK_SET);
                if (seekout != seekto)
                    goto badheader;
                if (read(fd, buf, sizeof(t_wavechunk)) <
                    (int) sizeof(t_wavechunk))
                        goto badheader;
                /* post("new chunk %c %c %c %c at %d",
                    ((t_wavechunk *)buf)->wc_id[0],
                    ((t_wavechunk *)buf)->wc_id[1],
                    ((t_wavechunk *)buf)->wc_id[2],
                    ((t_wavechunk *)buf)->wc_id[3], seekto); */
                headersize = seekto;
            }
            bytelimit = swap4(((t_wavechunk *)buf)->wc_size, swap);
            headersize += 8;
        }
        else
        {
                /* AIFF.  same as WAVE; actually predates it.  Disgusting. */
            headersize = 12;
            if (bytesread < 20)
                goto badheader;
                /* First we guess a number of channels, etc., in case there's
                no COMM block to follow. */
            nchannels = 1;
            bytespersamp = 2;
                /* copy the first chunk header to beginnning of buffer. */
            memcpy(buf, buf + headersize, sizeof(t_datachunk));
                /* read chunks in loop until we get to the data chunk */
            while (strncmp(((t_datachunk *)buf)->dc_id, "SSND", 4))
            {
                long chunksize = swap4(((t_datachunk *)buf)->dc_size,
                    swap), seekto = headersize + chunksize + 8, seekout;
                /* post("chunk %c %c %c %c seek %d",
                    ((t_datachunk *)buf)->dc_id[0],
                    ((t_datachunk *)buf)->dc_id[1],
                    ((t_datachunk *)buf)->dc_id[2],
                    ((t_datachunk *)buf)->dc_id[3], seekto); */
                if (!strncmp(((t_datachunk *)buf)->dc_id, "COMM", 4))
                {
                    long commblockonset = headersize + 8;
                    seekout = lseek(fd, commblockonset, SEEK_SET);
                    if (seekout != commblockonset)
                        goto badheader;
                    if (read(fd, buf, sizeof(t_comm)) <
                        (int) sizeof(t_comm))
                            goto badheader;
                    nchannels = swap2(((t_comm *)buf)->c_nchannels, swap);
                    format = swap2(((t_comm *)buf)->c_bitspersamp, swap);
                    if (format == 16)
                        bytespersamp = 2;
                    else if (format == 24)
                        bytespersamp = 3;
                    else goto badheader;
                }
                seekout = lseek(fd, seekto, SEEK_SET);
                if (seekout != seekto)
                    goto badheader;
                if (read(fd, buf, sizeof(t_datachunk)) <
                    (int) sizeof(t_datachunk))
                        goto badheader;
                headersize = seekto;
            }
            bytelimit = swap4(((t_datachunk *)buf)->dc_size, swap);
            headersize += 8;
        }
    }
        /* seek past header and any sample frames to skip */
    sysrtn = lseek(fd, nchannels * bytespersamp * skipframes + headersize, 0);
    if (sysrtn != nchannels * bytespersamp * skipframes + headersize)
        return (-1);
     bytelimit -= nchannels * bytespersamp * skipframes;
     if (bytelimit < 0)
        bytelimit = 0;
        /* copy sample format back to caller */
    *p_bigendian = bigendian;
    *p_nchannels = nchannels;
    *p_bytespersamp = bytespersamp;
    *p_bytelimit = bytelimit;
    return (fd);
badheader:
        /* the header wasn't recognized.  We're threadable here so let's not
        print out the error... */
    errno = EIO;
    return (-1);
}

static void soundfile_xferin(int sfchannels, int nvecs, float **vecs,
    long itemsread, unsigned char *buf, int nitems, int bytespersamp,
    int bigendian)
{
    int i, j;
    unsigned char *sp, *sp2;
    float *fp;
    int nchannels = (sfchannels < nvecs ? sfchannels : nvecs);
    int bytesperframe = bytespersamp * sfchannels;
    for (i = 0, sp = buf; i < nchannels; i++, sp += bytespersamp)
    {
        if (bytespersamp == 2)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp++)
                        *fp = SCALE * ((sp2[0] << 24) | (sp2[1] << 16));
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp++)
                        *fp = SCALE * ((sp2[1] << 24) | (sp2[0] << 16));
            }
        }
        else if (bytespersamp == 3)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp++)
                        *fp = SCALE * ((sp2[0] << 24) | (sp2[1] << 16)
                            | (sp2[2] << 8));
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp++)
                        *fp = SCALE * ((sp2[2] << 24) | (sp2[1] << 16)
                            | (sp2[0] << 8));
            }
        }
        else if (bytespersamp == 4)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp++)
                        *(long *)fp = ((sp2[0] << 24) | (sp2[1] << 16)
                            | (sp2[2] << 8) | sp2[3]);
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + itemsread;
                    j < nitems; j++, sp2 += bytesperframe, fp++)
                        *(long *)fp = ((sp2[3] << 24) | (sp2[2] << 16)
                            | (sp2[1] << 8) | sp2[0]);
            }
        }
    }
        /* zero out other outputs */
    for (i = sfchannels; i < nvecs; i++)
        for (j = nitems, fp = vecs[i]; j--; )
            *fp++ = 0;

}

    /* soundfiler_write ...
 
    usage: write [flags] filename table ...
    flags:
        -nframes <frames>
        -skip <frames>
        -bytes <bytes per sample>
        -normalize
        -nextstep
        -wave
        -big
        -little
    */

    /* the routine which actually does the work should LATER also be called
    from garray_write16. */


    /* Parse arguments for writing.  The "obj" argument is only for flagging
    errors.  For streaming to a file the "normalize", "onset" and "nframes"
    arguments shouldn't be set but the calling routine flags this. */

static int soundfiler_writeargparse(void *obj, int *p_argc, t_atom **p_argv,
    t_symbol **p_filesym,
    int *p_filetype, int *p_bytespersamp, int *p_swap, int *p_bigendian,
    int *p_normalize, long *p_onset, long *p_nframes, float *p_rate)
{
    int argc = *p_argc;
    t_atom *argv = *p_argv;
    int bytespersamp = 2, bigendian = 0,
        endianness = -1, swap, filetype = -1, normalize = 0;
    long onset = 0, nframes = 0x7fffffff;
    t_symbol *filesym;
    float rate = -1;
    
    while (argc > 0 && argv->a_type == A_SYMBOL &&
        *argv->a_w.w_symbol->s_name == '-')
    {
        char *flag = argv->a_w.w_symbol->s_name + 1;
        if (!strcmp(flag, "skip"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                ((onset = argv[1].a_w.w_float) < 0))
                    goto usage;
            argc -= 2; argv += 2;
        }
        else if (!strcmp(flag, "nframes"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                ((nframes = argv[1].a_w.w_float) < 0))
                    goto usage;
            argc -= 2; argv += 2;
        }
        else if (!strcmp(flag, "bytes"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                ((bytespersamp = argv[1].a_w.w_float) < 2) ||
                    bytespersamp > 4)
                        goto usage;
            argc -= 2; argv += 2;
        }
        else if (!strcmp(flag, "normalize"))
        {
            normalize = 1;
            argc -= 1; argv += 1;
        }
        else if (!strcmp(flag, "wave"))
        {
            filetype = FORMAT_WAVE;
            argc -= 1; argv += 1;
        }
        else if (!strcmp(flag, "nextstep"))
        {
            filetype = FORMAT_NEXT;
            argc -= 1; argv += 1;
        }
        else if (!strcmp(flag, "aiff"))
        {
            filetype = FORMAT_AIFF;
            argc -= 1; argv += 1;
        }
        else if (!strcmp(flag, "big"))
        {
            endianness = 1;
            argc -= 1; argv += 1;
        }
        else if (!strcmp(flag, "little"))
        {
            endianness = 0;
            argc -= 1; argv += 1;
        }
        else if (!strcmp(flag, "r") || !strcmp(flag, "rate"))
        {
            if (argc < 2 || argv[1].a_type != A_FLOAT ||
                ((rate = argv[1].a_w.w_float) <= 0))
                    goto usage;
            argc -= 2; argv += 2;
        }
        else goto usage;
    }
    if (!argc || argv->a_type != A_SYMBOL)
        goto usage;
    filesym = argv->a_w.w_symbol;
    
        /* check if format not specified and fill in */
    if (filetype < 0) 
    {
        if (strlen(filesym->s_name) >= 5 &&
                        (!strcmp(filesym->s_name + strlen(filesym->s_name) - 4, ".aif") ||
                        !strcmp(filesym->s_name + strlen(filesym->s_name) - 4, ".AIF")))
                filetype = FORMAT_AIFF;
        if (strlen(filesym->s_name) >= 6 &&
                        (!strcmp(filesym->s_name + strlen(filesym->s_name) - 5, ".aiff") ||
                        !strcmp(filesym->s_name + strlen(filesym->s_name) - 5, ".AIFF")))
                filetype = FORMAT_AIFF;
        if (strlen(filesym->s_name) >= 5 &&
                        (!strcmp(filesym->s_name + strlen(filesym->s_name) - 4, ".snd") ||
                        !strcmp(filesym->s_name + strlen(filesym->s_name) - 4, ".SND")))
                filetype = FORMAT_NEXT;
        if (strlen(filesym->s_name) >= 4 &&
                        (!strcmp(filesym->s_name + strlen(filesym->s_name) - 3, ".au") ||
                        !strcmp(filesym->s_name + strlen(filesym->s_name) - 3, ".AU")))
                filetype = FORMAT_NEXT;
        if (filetype < 0)
            filetype = FORMAT_WAVE;
    }
        /* don't handle AIFF floating point samples */
    if (bytespersamp == 4)
    {
        if (filetype == FORMAT_AIFF)
        {
            pd_error(obj, "AIFF floating-point file format unavailable");
            goto usage;
        }
    }
        /* for WAVE force little endian; for nextstep use machine native */
    if (filetype == FORMAT_WAVE)
    {
        bigendian = 0;
        if (endianness == 1)
            pd_error(obj, "WAVE file forced to little endian");
    }
    else if (filetype == FORMAT_AIFF)
    {
        bigendian = 1;
        if (endianness == 0)
            pd_error(obj, "AIFF file forced to big endian");
    }
    else if (endianness == -1)
    {
        bigendian = garray_ambigendian();
    }
    else bigendian = endianness;
    swap = (bigendian != garray_ambigendian());
    
    argc--; argv++;
    
    *p_argc = argc;
    *p_argv = argv;
    *p_filesym = filesym;
    *p_filetype = filetype;
    *p_bytespersamp = bytespersamp;
    *p_swap = swap;
    *p_normalize = normalize;
    *p_onset = onset;
    *p_nframes = nframes;
    *p_bigendian = bigendian;
    *p_rate = rate;
    return (0);
usage:
    return (-1);
}

static int create_soundfile(t_canvas *canvas, const char *filename,
    int filetype, int nframes, int bytespersamp,
    int bigendian, int nchannels, int swap, float samplerate)
{
    char filenamebuf[MAXPDSTRING], buf2[MAXPDSTRING];
    char headerbuf[WRITEHDRSIZE];
    t_wave *wavehdr = (t_wave *)headerbuf;
    t_nextstep *nexthdr = (t_nextstep *)headerbuf;
    t_aiff *aiffhdr = (t_aiff *)headerbuf;
    int fd, headersize = 0;
    
    strncpy(filenamebuf, filename, MAXPDSTRING-10);
    filenamebuf[MAXPDSTRING-10] = 0;

    if (filetype == FORMAT_NEXT)
    {
        if (strcmp(filenamebuf + strlen(filenamebuf)-4, ".snd"))
            strcat(filenamebuf, ".snd");
        if (bigendian)
            strncpy(nexthdr->ns_fileid, ".snd", 4);
        else strncpy(nexthdr->ns_fileid, "dns.", 4);
        nexthdr->ns_onset = swap4(sizeof(*nexthdr), swap);
        nexthdr->ns_length = 0;
        nexthdr->ns_format = swap4((bytespersamp == 3 ? NS_FORMAT_LINEAR_24 :
           (bytespersamp == 4 ? NS_FORMAT_FLOAT : NS_FORMAT_LINEAR_16)), swap);
        nexthdr->ns_sr = swap4(samplerate, swap);
        nexthdr->ns_nchans = swap4(nchannels, swap);
        strcpy(nexthdr->ns_info, "Pd ");
        swapstring(nexthdr->ns_info, swap);
        headersize = sizeof(t_nextstep);
    }
    else if (filetype == FORMAT_AIFF)
    {
        long datasize = nframes * nchannels * bytespersamp;
        long longtmp;
        static unsigned char dogdoo[] =
            {0x40, 0x0e, 0xac, 0x44, 0, 0, 0, 0, 0, 0, 'S', 'S', 'N', 'D'};
        if (strcmp(filenamebuf + strlen(filenamebuf)-4, ".aif") &&
            strcmp(filenamebuf + strlen(filenamebuf)-5, ".aiff"))
                strcat(filenamebuf, ".aif");
        strncpy(aiffhdr->a_fileid, "FORM", 4);
        aiffhdr->a_chunksize = swap4(datasize + sizeof(*aiffhdr) + 4, swap);
        strncpy(aiffhdr->a_aiffid, "AIFF", 4);
        strncpy(aiffhdr->a_fmtid, "COMM", 4);
        aiffhdr->a_fmtchunksize = swap4(18, swap);
        aiffhdr->a_nchannels = swap2(nchannels, swap);
        longtmp = swap4(nframes, swap);
        memcpy(&aiffhdr->a_nframeshi, &longtmp, 4);
        aiffhdr->a_bitspersamp = swap2(8 * bytespersamp, swap);
        memcpy(aiffhdr->a_samprate, dogdoo, sizeof(dogdoo));
        longtmp = swap4(datasize, swap);
        memcpy(aiffhdr->a_samprate + sizeof(dogdoo), &longtmp, 4);
        headersize = AIFFPLUS;
    }
    else    /* WAVE format */
    {
        long datasize = nframes * nchannels * bytespersamp;
        if (strcmp(filenamebuf + strlen(filenamebuf)-4, ".wav"))
            strcat(filenamebuf, ".wav");
        strncpy(wavehdr->w_fileid, "RIFF", 4);
        wavehdr->w_chunksize = swap4(datasize + sizeof(*wavehdr) - 8, swap);
        strncpy(wavehdr->w_waveid, "WAVE", 4);
        strncpy(wavehdr->w_fmtid, "fmt ", 4);
        wavehdr->w_fmtchunksize = swap4(16, swap);
        wavehdr->w_fmttag =
            swap2((bytespersamp == 4 ? WAV_FLOAT : WAV_INT), swap);
        wavehdr->w_nchannels = swap2(nchannels, swap);
        wavehdr->w_samplespersec = swap4(samplerate, swap);
        wavehdr->w_navgbytespersec =
            swap4((int)(samplerate * nchannels * bytespersamp), swap);
        wavehdr->w_nblockalign = swap2(nchannels * bytespersamp, swap);
        wavehdr->w_nbitspersample = swap2(8 * bytespersamp, swap);
        strncpy(wavehdr->w_datachunkid, "data", 4);
        wavehdr->w_datachunksize = swap4(datasize, swap);
        headersize = sizeof(t_wave);
    }

    canvas_makefilename(canvas, filenamebuf, buf2, MAXPDSTRING);
    sys_bashfilename(buf2, buf2);
    if ((fd = open(buf2, BINCREATE, 0666)) < 0)
        return (-1);

    if (write(fd, headerbuf, headersize) < headersize)
    {
        close (fd);
        return (-1);
    }
    return (fd);
}

static void soundfile_finishwrite(void *obj, char *filename, int fd,
    int filetype, long nframes, long itemswritten, int bytesperframe, int swap)
{
    if (itemswritten < nframes) 
    {
        if (nframes < 0x7fffffff)
            pd_error(obj, "soundfiler_write: %d out of %d bytes written",
                itemswritten, nframes);
            /* try to fix size fields in header */
        if (filetype == FORMAT_WAVE)
        {
            long datasize = itemswritten * bytesperframe, mofo;
            
            if (lseek(fd,
                ((char *)(&((t_wave *)0)->w_chunksize)) - (char *)0,
                    SEEK_SET) == 0)
                        goto baddonewrite;
            mofo = swap4(datasize + sizeof(t_wave) - 8, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
            if (lseek(fd,
                ((char *)(&((t_wave *)0)->w_datachunksize)) - (char *)0,
                    SEEK_SET) == 0)
                        goto baddonewrite;
            mofo = swap4(datasize, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
        }
        if (filetype == FORMAT_AIFF)
        {
            long mofo;
            if (lseek(fd,
                ((char *)(&((t_aiff *)0)->a_nframeshi)) - (char *)0,
                    SEEK_SET) == 0)
                        goto baddonewrite;
            mofo = swap4(nframes, swap);
            if (write(fd, (char *)(&mofo), 4) < 4)
                goto baddonewrite;
        }
        if (filetype == FORMAT_NEXT)
        {
            /* do it the lazy way: just set the size field to 'unknown size'*/
            uint32 nextsize = 0xffffffff;
            if (lseek(fd, 8, SEEK_SET) == 0)
            {
                goto baddonewrite;
            }
            if (write(fd, &nextsize, 4) < 4)
            {
                goto baddonewrite;
            }
        }
    }
    return;
baddonewrite:
    post("%s: %s", filename, strerror(errno));
}

static void soundfile_xferout(int nchannels, float **vecs,
    unsigned char *buf, int nitems, long onset, int bytespersamp,
    int bigendian, float normalfactor)
{
    int i, j;
    unsigned char *sp, *sp2;
    float *fp;
    int bytesperframe = bytespersamp * nchannels;
    long xx;
    for (i = 0, sp = buf; i < nchannels; i++, sp += bytespersamp)
    {
        if (bytespersamp == 2)
        {
            float ff = normalfactor * 32768.;
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp = vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp++)
                {
                    int xx = 32768. + (*fp * ff);
                    xx -= 32768;
                    if (xx < -32767)
                        xx = -32767;
                    if (xx > 32767)
                        xx = 32767;
                    sp2[0] = (xx >> 8);
                    sp2[1] = xx;
                }
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp++)
                {
                    int xx = 32768. + (*fp * ff);
                    xx -= 32768;
                    if (xx < -32767)
                        xx = -32767;
                    if (xx > 32767)
                        xx = 32767;
                    sp2[1] = (xx >> 8);
                    sp2[0] = xx;
                }
            }
        }
        else if (bytespersamp == 3)
        {
            float ff = normalfactor * 8388608.;
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp++)
                {
                    int xx = 8388608. + (*fp * ff);
                    xx -= 8388608;
                    if (xx < -8388607)
                        xx = -8388607;
                    if (xx > 8388607)
                        xx = 8388607;
                    sp2[0] = (xx >> 16);
                    sp2[1] = (xx >> 8);
                    sp2[2] = xx;
                }
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp++)
                {
                    int xx = 8388608. + (*fp * ff);
                    xx -= 8388608;
                    if (xx < -8388607)
                        xx = -8388607;
                    if (xx > 8388607)
                        xx = 8388607;
                    sp2[2] = (xx >> 16);
                    sp2[1] = (xx >> 8);
                    sp2[0] = xx;
                }
            }
        }
        else if (bytespersamp == 4)
        {
            if (bigendian)
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp++)
                {
                    float f2 = *fp * normalfactor;
                    xx = *(long *)&f2;
                    sp2[0] = (xx >> 24); sp2[1] = (xx >> 16);
                    sp2[2] = (xx >> 8); sp2[3] = xx;
                }
            }
            else
            {
                for (j = 0, sp2 = sp, fp=vecs[i] + onset;
                    j < nitems; j++, sp2 += bytesperframe, fp++)
                {
                    float f2 = *fp * normalfactor;
                    xx = *(long *)&f2;
                    sp2[3] = (xx >> 24); sp2[2] = (xx >> 16);
                    sp2[1] = (xx >> 8); sp2[0] = xx;
                }
            }
        }
    }
}



/* ------- threadedsf - reads and writes soundfiles to/from "garrays" ---- */

// in pd's soundfiler maxsize is 4000000
// threaded we should be able to hande a bit more
#define DEFMAXSIZE 40000000
#define SAMPBUFSIZE 1024


static t_class *threadedsf_class;

typedef struct _threadedsf
{
    t_object x_obj;
    t_canvas *x_canvas;
} t_threadedsf;

#include <sched.h>

#ifdef _POSIX_MEMLOCK
#include <sys/mman.h>
#endif /* _POSIX_MEMLOCK */

#ifdef DEBUG
#define SFDEBUG
#endif

static pthread_t sf_thread_id; /* id of soundfiler thread */
//static pthread_attr_t sf_attr;

typedef struct _sfprocess
{
    void (* process) (t_threadedsf *,t_symbol *, 
		      int, t_atom *); /* function to call */
    t_threadedsf * x;        /* soundfiler */
    int argc;
    t_atom * argv;
    struct _sfprocess * next;  /* next object in queue */
    pthread_mutex_t mutex;
} t_sfprocess;

/* this is the queue for all threadedsf objects */
typedef struct _sfqueue
{
    t_sfprocess * begin;
    t_sfprocess * end;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} t_sfqueue;

static t_sfqueue * threadedsf_queue; 


/* we fill the queue */
void threadedsf_queue_add(void (* process) (t_threadedsf *,t_symbol *,
				int,t_atom *), void * x, 
				int argc, t_atom * argv)
{
    /* preparing entry */
    t_sfprocess * last_entry = (t_sfprocess*)getbytes(sizeof(t_sfprocess));

#ifdef SFDEBUG
    post("adding process to queue");
#endif

    pthread_mutex_init(&(last_entry->mutex), NULL);
    pthread_mutex_lock(&(last_entry->mutex));
    last_entry->process = process;
    last_entry->x = x;
    last_entry->argc = argc;
    last_entry->argv = copybytes (argv, argc * sizeof(t_atom));
    last_entry->next = NULL; 
    pthread_mutex_unlock(&(last_entry->mutex));

    /* add new entry to queue */
    pthread_mutex_lock(&(threadedsf_queue->mutex));

    if (threadedsf_queue->begin==NULL)
    {
		threadedsf_queue->begin=last_entry;
		threadedsf_queue->end=last_entry;
    }
    else 
    {
		pthread_mutex_lock(&(threadedsf_queue->end->mutex));
		threadedsf_queue->end->next=last_entry;
		pthread_mutex_unlock(&(threadedsf_queue->end->mutex));
		threadedsf_queue->end=last_entry;
    }


    if ( threadedsf_queue->begin == threadedsf_queue->end )
    {
#ifdef DEBUG
		post("signaling");
#endif
		pthread_mutex_unlock(&(threadedsf_queue->mutex));

		/* and signal the helper thread */
		pthread_cond_signal(&(threadedsf_queue->cond));
    }
    else
    {
#ifdef DEBUG
		post("not signaling");
#endif
		pthread_mutex_unlock(&(threadedsf_queue->mutex));
    }
    return;
}

/* global threadedsf thread ... sleeping until signaled */
void threadedsf_thread(void)
{    
    t_sfprocess * me;
    t_sfprocess * next;

#ifdef DEBUG
	post("threadedsf_thread ID: %d", pthread_self());
#endif
	while (1)
    {
#ifdef DEBUG
		post("Soundfiler sleeping");
#endif
		pthread_cond_wait(&threadedsf_queue->cond, &threadedsf_queue->mutex);
#ifdef DEBUG
		post("Soundfiler awake");
#endif

		/* work on the queue */
		while (threadedsf_queue->begin!=NULL)
		{
			post("threadedsf: locked queue");
			/* locking process */
			pthread_mutex_lock(&(threadedsf_queue->begin->mutex));
	    
			me = threadedsf_queue->begin;
		
			pthread_mutex_unlock(&(me->mutex));
			pthread_mutex_unlock(&(threadedsf_queue->mutex)); 
#ifdef DEBUG
			post("threadedsf: mutex unlocked, running process");
#endif

			/* running the specific function */
			me->process(me->x, NULL, me->argc, me->argv);
#ifdef DEBUG
			post("threadedsf: process done, locking mutex");
#endif
	    
			pthread_mutex_lock(&(threadedsf_queue->mutex));
			pthread_mutex_lock(&(me->mutex));

			/* freeing the argument vector */
			freebytes(me->argv, sizeof(t_atom) * me->argc); 

			/* the  process struct */
			next=me->next;
			threadedsf_queue->begin=next;
			freebytes(me, sizeof(t_sfprocess));
		};
		threadedsf_queue->end=NULL;
    }
}

extern int sys_hipriority;   	/* real-time flag, true if priority boosted */

/* create soundfiler thread */
void sys_start_sfthread(void)
{
    pthread_attr_t sf_attr;
    struct sched_param sf_param;

	int status;

	//initialize queue
    threadedsf_queue = getbytes (sizeof(t_sfqueue));

    pthread_mutex_init (&threadedsf_queue->mutex,NULL);
    pthread_cond_init (&threadedsf_queue->cond,NULL);

    threadedsf_queue->begin=threadedsf_queue->end=NULL;
/*     pthread_mutex_unlock(&(threadedsf_queue->mutex));  */
    
    // initialize thread
    pthread_attr_init(&sf_attr);
    
//#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
    sf_param.sched_priority=sched_get_priority_min(SCHED_OTHER);
    pthread_attr_setschedparam(&sf_attr,&sf_param);
/*     pthread_attr_setinheritsched(&sf_attr,PTHREAD_EXPLICIT_SCHED); */

#ifdef UNIX
     if (sys_hipriority == 1 && getuid() == 0)
     {
		 sf_param.sched_priority=sched_get_priority_min(SCHED_RR);
		 pthread_attr_setschedpolicy(&sf_attr,SCHED_RR);
     }
	else
	{
/* 		pthread_attr_setschedpolicy(&sf_attr,SCHED_OTHER); */
/* 		sf_param.sched_priority=sched_get_priority_min(SCHED_OTHER); */
	}
#endif /* UNIX */

//#endif /* _POSIX_THREAD_PRIORITY_SCHEDULING */

    //start thread
    status = pthread_create(&sf_thread_id, &sf_attr, 
				(void *) threadedsf_thread,NULL);
    if (status != 0)
		error("Couldn't create threadedsf thread: %d",status);
#ifdef DEBUG
    else
		post("global threadedsf thread launched, priority: %d", 
			 sf_param.sched_priority);
#endif
}

static void threadedsf_t_write(t_threadedsf *x, t_symbol *s,
				   int argc, t_atom *argv);

static void threadedsf_t_write_addq(t_threadedsf *x, t_symbol *s,
					int argc, t_atom *argv)
{
    threadedsf_queue_add(threadedsf_t_write,(void *)x,argc, argv);
}

static void threadedsf_t_read(t_threadedsf *x, t_symbol *s,
				  int argc, t_atom *argv);

static void threadedsf_t_read_addq(t_threadedsf *x, t_symbol *s,
					   int argc, t_atom *argv)
{
    threadedsf_queue_add(threadedsf_t_read,(void *)x,argc, argv);
}


    /* threadedsf_read ...
    
    usage: read [flags] filename table ...
    flags:
    	-skip <frames> ... frames to skip in file
	-nframes <frames>
	-onset <frames> ... onset in table to read into (NOT DONE YET)
	-raw <headersize channels bytes endian>
	-resize
	-maxsize <max-size>

    TB: adapted for threaded use
    */

/* callback prototypes */
static t_int threadedsf_read_update_garray(t_int * w);
static t_int threadedsf_read_update_graphics(t_int * w);
static t_int threadedsf_read_output(t_int * w);


static void threadedsf_t_read(t_threadedsf *x, t_symbol *s,
				  int argc, t_atom *argv)
{
    int headersize = -1, channels = 0, bytespersamp = 0, bigendian = 0,
		resize = 0, i, j;
    long skipframes = 0,
		nframes = 0,
		finalsize = 0,                     /* new size */
    	maxsize = DEFMAXSIZE,
		itemsread = 0,
		bytelimit  = 0x7fffffff;
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
	
    while (argc > 0 && argv->a_type == A_SYMBOL &&
		   *argv->a_w.w_symbol->s_name == '-')
    {
    	char *flag = argv->a_w.w_symbol->s_name + 1;
		if (!strcmp(flag, "skip"))
		{
			if (argc < 2 || argv[1].a_type != A_FLOAT ||
				((skipframes = argv[1].a_w.w_float) < 0))
	    	    goto usage;
			argc -= 2; argv += 2;
		}
		else if (!strcmp(flag, "nframes"))
		{
			if (argc < 2 || argv[1].a_type != A_FLOAT ||
				((nframes = argv[1].a_w.w_float) < 0))
	    	    goto usage;
			argc -= 2; argv += 2;
		}
		else if (!strcmp(flag, "raw"))
		{
			if (argc < 5 ||
				argv[1].a_type != A_FLOAT ||
				((headersize = argv[1].a_w.w_float) < 0) ||
				argv[2].a_type != A_FLOAT ||
				((channels = argv[2].a_w.w_float) < 1) ||
				(channels > MAXSFCHANS) ||
				argv[3].a_type != A_FLOAT ||
				((bytespersamp = argv[3].a_w.w_float) < 2) ||
				(bytespersamp > 4) ||
				argv[4].a_type != A_SYMBOL ||
				((endianness = argv[4].a_w.w_symbol->s_name[0]) != 'b'
				 && endianness != 'l' && endianness != 'n'))
				goto usage;
			if (endianness == 'b')
				bigendian = 1;
			else if (endianness == 'l')
				bigendian = 0;
			else
				bigendian = garray_ambigendian();
			argc -= 5; argv += 5;
		}
		else if (!strcmp(flag, "resize"))
		{
			resize = 1;
			argc -= 1; argv += 1;
		}
		else if (!strcmp(flag, "maxsize"))
		{
			if (argc < 2 || argv[1].a_type != A_FLOAT ||
				((maxsize = argv[1].a_w.w_float) < 0))
	    	    goto usage;
			resize = 1;     /* maxsize implies resize. */
			argc -= 2; argv += 2;
		}
		else goto usage;
    }
    if (argc < 2 || argc > MAXSFCHANS + 1 || argv[0].a_type != A_SYMBOL)
    	goto usage;
    filename = argv[0].a_w.w_symbol->s_name;
    argc--; argv++;
    
    for (i = 0; i < argc; i++)
    {
    	if (argv[i].a_type != A_SYMBOL)
			goto usage;
		if (!(garrays[i] =
			  (t_garray *)pd_findbyclass(argv[i].a_w.w_symbol, garray_class)))
		{
			pd_error(x, "%s: no such table", argv[i].a_w.w_symbol->s_name);
			goto done;
		}
    	else if (!garray_getfloatarray(garrays[i], &vecsize[i], &vecs[i]))
    	    error("%s: bad template for tabwrite",
				  argv[i].a_w.w_symbol->s_name);
    	if (finalsize && finalsize != vecsize[i] && !resize)
		{
			post("threadedsf_read: arrays have different lengths; resizing...");
			resize = 1;
		}
		finalsize = vecsize[i];
    }
    fd = open_soundfile(canvas_getdir(x->x_canvas)->s_name, filename,
						headersize, &bytespersamp, &bigendian, &channels, &bytelimit,
						skipframes);

    if (fd < 0)
    {
		pd_error(x, "threadedsf_read: %s: %s", filename, (errno == EIO ?
														  "unknown or bad header format" : strerror(errno)));
    	goto done;
    }

    if (resize)
    {
		/* figure out what to resize to */
    	long poswas, eofis, framesinfile;
	
		poswas = lseek(fd, 0, SEEK_CUR);
		eofis = lseek(fd, 0, SEEK_END);
		if (poswas < 0 || eofis < 0)
		{
			pd_error(x, "lseek failed");
			goto done;
		}
		lseek(fd, poswas, SEEK_SET);
		framesinfile = (eofis - poswas) / (channels * bytespersamp);
		if (framesinfile > maxsize)
		{
			pd_error(x, "threadedsf_read: truncated to %d elements", maxsize);
			framesinfile = maxsize;
		}
        if (framesinfile > bytelimit / (channels * bytespersamp))
            framesinfile = bytelimit / (channels * bytespersamp);
		finalsize = framesinfile;

    }
    if (!finalsize) finalsize = 0x7fffffff;
    if (finalsize > bytelimit / (channels * bytespersamp))
    	finalsize = bytelimit / (channels * bytespersamp);
    fp = fdopen(fd, "rb");
    bufframes = SAMPBUFSIZE / (channels * bytespersamp);

#ifdef SFDEBUG
    post("buffers: %d", argc);
    post("channels: %d", channels);
#endif

#ifdef _POSIX_MEMLOCK
	munlockall();
#endif
	
    /* allocate memory for new array */
	if (resize)
		for (i = 0; i < argc; i++)
		{
#ifdef SFDEBUG
			post("allocating buffer %d",i);
#endif
			nvecs[i]=getbytes (finalsize * sizeof(t_float));
			/* if we are out of memory, free it again and quit */
			if (nvecs[i]==0)
			{
				pd_error(x, "resize failed");
				for (j=0; j!=i;++j)
					/* if the resizing fails, we'll have to free all arrays again */
					freebytes (nvecs[i],finalsize * sizeof(t_float));
				goto done;
			}
			/* zero samples */
			if(i > channels)
			{
				memset(nvecs[i],0,vecsize[i] * sizeof(t_float));
			}
		}
	else
		for (i = 0; i < argc; i++)
		{
			nvecs[i] = getbytes (vecsize[i] * sizeof(t_float));
			/* if we are out of memory, free it again and quit */
			if (nvecs[i]==0)
			{
				pd_error(x, "resize failed");
				for (j=0; j!=i;++j)
					/* if the resizing fails, we'll have to free all arrays again */
					freebytes (nvecs[i],vecsize[i] * sizeof(t_float));
				goto done;
			}
			/* zero samples */
			if(i > channels)
				memset(nvecs[i],0,vecsize[i] * sizeof(t_float));
		}

#ifdef SFDEBUG
	post("transfer soundfile");
#endif

	for (itemsread = 0; itemsread < finalsize; )
	{
		int thisread = finalsize - itemsread;
		thisread = (thisread > bufframes ? bufframes : thisread);
		nitems = fread(sampbuf, channels * bytespersamp, thisread, fp);
		if (nitems <= 0) break;
		soundfile_xferin(channels, argc, nvecs, itemsread,
						 (unsigned char *)sampbuf, nitems, bytespersamp, bigendian);
		itemsread += nitems;
	}

#ifdef SFDEBUG
	post("zeroing remaining elements");
#endif
	/* zero out remaining elements of vectors */
	for (i = 0; i < argc; i++)
	{
		for (j = itemsread; j < finalsize; j++)
			nvecs[i][j] = 0;
	}

	/* set idle callback to switch pointers */
#ifdef SFDEBUG
	post("locked");
#endif
	for (i = 0; i < argc; i++)
	{
		t_int* w = (t_int*)getbytes(4*sizeof(t_int));
		
		w[0] = (t_int)(garrays[i]);
		w[1] = (t_int)nvecs[i];
		w[2] = (t_int)finalsize;
		w[3] = (t_int)(&resume_after_callback);
		
		h_set_callback(&threadedsf_read_update_garray, w, 4);

		pthread_cond_wait(&resume_after_callback, &resume_after_callback_mutex);
	}

#ifdef SFDEBUG
	post("unlocked, doing graphics updates");
#endif

	/* do all graphics updates
	 * run this in the main thread via callback */
	for (i = 0; i < argc; i++)
	{
		t_int* w = (t_int*)getbytes(2*sizeof(t_int));
		
		w[0] = (t_int)(garrays[i]);
		w[1] = (t_int)finalsize;
		
		h_set_callback(&threadedsf_read_update_graphics, w, 2);
	}

	/* free the old arrays */
	for (i = 0; i < argc; i++)
		freebytes(vecs[i], vecsize[i] * sizeof(t_float));

    fclose(fp);
    fd = -1;
    goto done;
    
 usage:
    pd_error(x, "usage: read [flags] filename tablename...");
    post("flags: -skip <n> -nframes <n> -resize -maxsize <n> ...");
    post("-raw <headerbytes> <channels> <bytespersamp> <endian (b, l, or n)>.");
    
 done:
    if (fd >= 0)
    	close (fd);

#ifdef _POSIX_MEMLOCK
	mlockall(MCL_FUTURE);
#endif
	
	outargs = (t_int*)getbytes(2*sizeof(t_int));
	outargs[0] = (t_int)x->x_obj.ob_outlet;
	outargs[1] = (t_int)itemsread;
	h_set_callback(&threadedsf_read_output, outargs, 2);
}

/* idle callback for threadsafe synchronisation */
static t_int threadedsf_read_update_garray(t_int * w)
{
	t_garray* garray = (t_garray*)w[0];
	t_array *data = garray_getarray(garray);
	t_int nvec = w[1];
	t_int finalsize = w[2];
	pthread_cond_t * conditional = (pthread_cond_t*) w[3];

#ifdef SFDEBUG
	post("lock array %p", garray);
#endif

        // no garray_locks in current pd
	//garray_lock(garray);

	data->a_vec = (char *) nvec;
	data->a_n = finalsize;
	if (garray->x_usedindsp) canvas_update_dsp();

#ifdef SFDEBUG
	post("unlock array %p", garray);
#endif

        // no garray_locks in current pd
	//garray_unlock(garray);

	/* signal helper thread */
	pthread_cond_broadcast(conditional);
	
	return 0;
}

static t_int threadedsf_read_update_graphics(t_int * w)
{
	t_garray* garray = (t_garray*) w[0];
	t_glist *gl;
	int n = w[1];
	/* if this is the only array in the graph,
	   reset the graph's coordinates */

#ifdef SFDEBUG
	post("redraw array %p", garray);
#endif

	gl = garray->x_glist;
	if (gl->gl_list == &garray->x_gobj && !garray->x_gobj.g_next)
	{
		vmess(&gl->gl_pd, gensym("bounds"), "ffff",
			  0., gl->gl_y1, (double)(n > 1 ? n-1 : 1), gl->gl_y2);
		/* close any dialogs that might have the wrong info now... */
		gfxstub_deleteforkey(gl);
	}
	else 
		garray_redraw(garray);

	return 0;
}


static t_int threadedsf_read_output(t_int * w)
{
	t_outlet* outlet = (t_outlet*) w[0];
	float itemsread = (float) w[1];

#ifdef SFDEBUG
	post("bang %p", outlet);
#endif
	
	outlet_float (outlet, itemsread);

	return 0;
}




    /* this is broken out from threadedsf_write below so garray_write can
    call it too... not done yet though. */

long threadedsf_t_dowrite(void *obj, t_canvas *canvas,
    int argc, t_atom *argv)
{
    int bytespersamp, bigendian,
    	swap, filetype, normalize, i, j, nchannels;
    long onset, nframes, itemswritten = 0;
    t_garray *garrays[MAXSFCHANS];
    t_float *vecs[MAXSFCHANS];
    char sampbuf[SAMPBUFSIZE];
    int bufframes;
    int fd = -1;
    float normfactor, biggest = 0, samplerate;
    t_symbol *filesym;

    if (soundfiler_writeargparse(obj, &argc, &argv, &filesym, &filetype,
    	&bytespersamp, &swap, &bigendian, &normalize, &onset, &nframes,
	    &samplerate))
    	    	goto usage;
    nchannels = argc;
    if (nchannels < 1 || nchannels > MAXSFCHANS)
    	goto usage;
    if (samplerate < 0)
    	samplerate = sys_getsr();
	for (i = 0; i < nchannels; i++)
    {
    	int vecsize;
    	if (argv[i].a_type != A_SYMBOL)
	    goto usage;
	if (!(garrays[i] =
	    (t_garray *)pd_findbyclass(argv[i].a_w.w_symbol, garray_class)))
	{
	    pd_error(obj, "%s: no such table", argv[i].a_w.w_symbol->s_name);
	    goto fail;
	}
    	else if (!garray_getfloatarray(garrays[i], &vecsize, &vecs[i]))
    	    error("%s: bad template for tabwrite",
	    	argv[i].a_w.w_symbol->s_name);
    	if (nframes > vecsize - onset)
	    nframes = vecsize - onset;
    	
	for (j = 0; j < vecsize; j++)
	{
	    if (vecs[i][j] > biggest)
	    	biggest = vecs[i][j];
	    else if (-vecs[i][j] > biggest)
	    	biggest = -vecs[i][j];
    	}
    }
    if (nframes <= 0)
    {
	pd_error(obj, "threadedsf_write: no samples at onset %ld", onset);
    	goto fail;
    }

    if ((fd = create_soundfile(canvas, filesym->s_name, filetype,
    	nframes, bytespersamp, bigendian, nchannels,
	    swap, samplerate)) < 0)
    {
    	post("%s: %s\n", filesym->s_name, strerror(errno));
    	goto fail;
    }
    if (!normalize)
    {
    	if ((bytespersamp != 4) && (biggest > 1))
	{
    	    post("%s: normalizing max amplitude %f to 1", filesym->s_name, biggest);
    	    normalize = 1;
    	}
	else post("%s: biggest amplitude = %f", filesym->s_name, biggest);
    }
    if (normalize)
	normfactor = (biggest > 0 ? 32767./(32768. * biggest) : 1);
    else normfactor = 1;

    bufframes = SAMPBUFSIZE / (nchannels * bytespersamp);

    for (itemswritten = 0; itemswritten < nframes; )
    {
    	int thiswrite = nframes - itemswritten, nbytes;
    	thiswrite = (thiswrite > bufframes ? bufframes : thiswrite);
	soundfile_xferout(argc, vecs, (unsigned char *)sampbuf, thiswrite,
	    onset, bytespersamp, bigendian, normfactor);
    	nbytes = write(fd, sampbuf, nchannels * bytespersamp * thiswrite);
	if (nbytes < nchannels * bytespersamp * thiswrite)
	{
	    post("%s: %s", filesym->s_name, strerror(errno));
	    if (nbytes > 0)
	    	itemswritten += nbytes / (nchannels * bytespersamp);
	    break;
	}
	itemswritten += thiswrite;
	onset += thiswrite;
    }
    if (fd >= 0)
    {
    	soundfile_finishwrite(obj, filesym->s_name, fd,
    	    filetype, nframes, itemswritten, nchannels * bytespersamp, swap);
    	close (fd);
    }
    return ((float)itemswritten); 
usage:
    pd_error(obj, "usage: write [flags] filename tablename...");
    post("flags: -skip <n> -nframes <n> -bytes <n> -wave -aiff -nextstep ...");
    post("-big -little -normalize");
    post("(defaults to a 16-bit wave file).");
fail:
    if (fd >= 0)
    	close (fd);
    return (0); 
} 

static void threadedsf_t_write(t_threadedsf *x, t_symbol *s,
    int argc, t_atom *argv)
{
    long bozo = threadedsf_t_dowrite(x, x->x_canvas,
    	argc, argv);
    sys_lock();
    outlet_float(x->x_obj.ob_outlet, (float)bozo); 
    sys_lock();
}

static void threadedsf_t_resize(t_threadedsf *x, t_symbol *s,
				int argc, t_atom *argv);

static void threadedsf_t_resize_addq(t_threadedsf *x, t_symbol *s,
    int argc, t_atom *argv)
{
    threadedsf_queue_add(threadedsf_t_resize,(void *)x,argc, argv);
}


    /* TB: threadedsf_t_resize ...
    usage: resize table size
    adapted from garray_resize
    */
static void threadedsf_t_resize(t_threadedsf *y, t_symbol *s,
				int argc, t_atom *argv)
{
    int was, elemsize;       /* array contains was elements of size elemsize */
    t_float * vec;           /* old array */ 
    t_glist *gl;
    int n;                   /* resize of n elements */
    char *nvec;              /* new array */ 

    t_garray * x = (t_garray *)pd_findbyclass(argv[0].a_w.w_symbol, garray_class);
    t_array *data = garray_getarray(x); // TODO muss der im lock sein ?

    if (!(x))
    {
		pd_error(y, "%s: no such table", argv[0].a_w.w_symbol->s_name);
		goto usage;
    }
    
    vec = (t_float*) data->a_vec;
    was = data->a_n;

    if ((argv+1)->a_type == A_FLOAT)
    {
		n = (int) (argv+1)->a_w.w_float;
    }
    else
    {
		goto usage;
    }

    if (n == was)
    {
		return;
    }

    if (n < 1) n = 1;
    elemsize = template_findbyname(data->a_templatesym)->t_n * sizeof(t_word);

#ifdef _POSIX_MEMLOCK
	munlockall();
#endif
    if (was > n)
    {
		nvec = (char*)copybytes(data->a_vec, was * elemsize);
    }
    else
    {
		nvec = getbytes(n * elemsize);
		memcpy (nvec, data->a_vec, was * elemsize);
	
		/* LATER should check t_resizebytes result */
		memset(nvec + was*elemsize,
			   0, (n - was) * elemsize);
    }
    if (!nvec)
    {
    	pd_error(x, "array resize failed: out of memory");
#ifdef _POSIX_MEMLOCK
		mlockall(MCL_FUTURE);
#endif
		return;
    }


    /* TB: we'll have to be sure that no one is accessing the array */
    sys_lock();
    
    // no garray_locks in current pd
//#ifdef GARRAY_THREAD_LOCK 
    //garray_lock(x);
//#endif
    
    data->a_vec = nvec;
    data->a_n = n;
    
    // no garray_locks in current pd
//#ifdef GARRAY_THREAD_LOCK 
    //garray_unlock(x);
//#endif
    
    if (x->x_usedindsp) canvas_update_dsp();
    sys_unlock();

    
	/* if this is the only array in the graph,
	   reset the graph's coordinates */
    gl = x->x_glist;
    if (gl->gl_list == &x->x_gobj && !x->x_gobj.g_next)
    {
    	vmess(&gl->gl_pd, gensym("bounds"), "ffff",
			  0., gl->gl_y1, (double)(n > 1 ? n-1 : 1), gl->gl_y2);
		/* close any dialogs that might have the wrong info now... */
    	gfxstub_deleteforkey(gl);
    }
    else garray_redraw(x);

    freebytes (vec, was * elemsize);
#ifdef _POSIX_MEMLOCK
	mlockall(MCL_FUTURE);
#endif
    sys_lock();
    outlet_float(y->x_obj.ob_outlet, (float)atom_getintarg(1,argc,argv)); 
    sys_unlock();
    return;
    
 usage:
    pd_error(x, "usage: resize tablename size");
}


//static void threadedsf_t_const(t_threadedsf *x, t_symbol *s,
//				int argc, t_atom *argv);
//
//static void threadedsf_t_const_addq(t_threadedsf *x, t_symbol *s,
//    int argc, t_atom *argv)
//{
//    threadedsf_queue_add(threadedsf_t_const,(void *)x,argc, argv);
//}


/* TB: threadedsf_t_const ...
   usage: const table value
*/
//static void threadedsf_t_const(t_threadedsf *y, t_symbol *s,
//							   int argc, t_atom *argv)
//{
//    int size, elemsize;    /* array contains was elements of size elemsize */
//    t_float * vec;         /* old array */ 
//    t_glist *gl;
//    int val;               /* value */
//    char *nvec;            /* new array */ 
//    int i;
//
//    t_garray * x = (t_garray *)pd_findbyclass(argv[0].a_w.w_symbol, garray_class);
//    t_array *data = garray_getarray(x); // TODO muss der im lock sein ?
//
//    if (!(x))
//    {
//		pd_error(y, "%s: no such table", argv[0].a_w.w_symbol->s_name);
//		goto usage;
//    }
//    
//
//
//    vec = (t_float*) data->a_vec;
//    size = data->a_n;
//    
//    if ((argv+1)->a_type == A_FLOAT)
//    {
//		val = (int) (argv+1)->a_w.w_float;
//    }
//    else
//    {
//		goto usage;
//    }
//
//#ifdef SFDEBUG
//    post("array size: %d; new value: %d",size,val);
//#endif
//    
//    elemsize = template_findbyname(data->a_templatesym)->t_n * sizeof(t_word);
//    
//
//    /* allocating memory */
//#ifdef _POSIX_MEMLOCK
//    munlockall();
//#endif
//    nvec = getbytes(size * elemsize);
//
//    if (!nvec)
//    {
//    	pd_error(x, "array resize failed: out of memory");
//#ifdef _POSIX_MEMLOCK
//		mlockall(MCL_FUTURE);
//#endif
//		return;
//    }
//    
//    /* setting array */
//    for (i=0; i!=size; ++i)
//    {
//		nvec[i]=val;
//    }
//
//    /* TB: we'll have to be sure that no one is accessing the array */
//    sys_lock();
//#ifdef GARRAY_THREAD_LOCK 
//    garray_lock(x);
//#endif
//    data->a_vec = nvec;
//#ifdef GARRAY_THREAD_LOCK 
//    garray_unlock(x);
//#endif
//    if (x->x_usedindsp) canvas_update_dsp();
//    sys_unlock();
//
//    
//	/* if this is the only array in the graph,
//	   reset the graph's coordinates */
//    gl = x->x_glist;
//    if (gl->gl_list == &x->x_gobj && !x->x_gobj.g_next)
//    {
//    	vmess(&gl->gl_pd, gensym("bounds"), "ffff",
//			  0., gl->gl_y1, (double)(size > 1 ? size-1 : 1), gl->gl_y2);
//		/* close any dialogs that might have the wrong info now... */
//    	gfxstub_deleteforkey(gl);
//    }
//    else garray_redraw(x);
//
//    freebytes (vec, size * elemsize);
//#ifdef _POSIX_MEMLOCK
//    mlockall(MCL_FUTURE);
//#endif
//    sys_lock();
//	outlet_float(y->x_obj.ob_outlet, size); 
//    sys_unlock();
//    return;
//    
// usage:
//    pd_error(x, "usage: const tablename value");
//}


static t_threadedsf *threadedsf_new(void)
{
    t_threadedsf *x = (t_threadedsf *)pd_new(threadedsf_class);
    x->x_canvas = canvas_getcurrent();
    outlet_new(&x->x_obj, &s_float);
    return (x);
}


void threadedsf_setup(void)
{
    threadedsf_class = class_new(gensym("threadedsf"), (t_newmethod)threadedsf_new, 
					 0, sizeof(t_threadedsf), 0, 0);
    
    class_addmethod(threadedsf_class, (t_method)threadedsf_t_read_addq, 
		    gensym("read"), A_GIMME, 0);
    class_addmethod(threadedsf_class, (t_method)threadedsf_t_write_addq,
 		    gensym("write"), A_GIMME, 0);
    class_addmethod(threadedsf_class, (t_method)threadedsf_t_resize_addq,
		    gensym("resize"), A_GIMME, 0);
//    class_addmethod(threadedsf_class, (t_method)threadedsf_t_const_addq,
//		    gensym("const"), A_GIMME, 0);
}
