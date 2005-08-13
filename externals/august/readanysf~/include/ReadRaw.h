/*
 * readanysf~  external for pd. 
 * 
 * Copyright (C) 2003 August Black
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * ReadRaw.h
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#ifndef _READRAW_H_
#define _READRAW_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Readsf.h"

typedef struct _nextstep
{
    char ns_fileid[4];      /* magic number '.snd' if file is big-endian */
    unsigned int ns_onset;        /* byte offset of first sample */
    unsigned int ns_length;       /* length of sound in bytes */
    unsigned int ns_format;        /* format; see below */
    unsigned int ns_sr;           /* sample rate */
    unsigned int ns_nchans;       /* number of channels */
    char ns_info[4];        /* comment */
} t_nextstep;

typedef struct _wave
{
    char  w_fileid[4];	    	    /* chunk id 'RIFF'            */
    unsigned int w_chunksize;     	    /* chunk size                 */
    char  w_waveid[4];	    	    /* wave chunk id 'WAVE'       */
    char  w_fmtid[4];	    	    /* format chunk id 'fmt '     */
    unsigned int w_fmtchunksize;   	    /* format chunk size          */
    unsigned short  w_fmttag;	    	    /* format tag, 1 for PCM      */
    unsigned short  w_nchannels;    	    /* number of channels         */
    unsigned int w_samplespersec;  	    /* sample rate in hz          */
    unsigned int w_navgbytespersec; 	    /* average bytes per second   */
    unsigned short  w_nblockalign;    	    /* number of bytes per frame  */
    unsigned short  w_nbitspersample; 	    /* number of bits in a sample */
    char  w_datachunkid[4]; 	    /* data chunk id 'data'       */
    unsigned int w_datachunksize;         /* length of data chunk       */
} t_wave;

typedef struct _fmt	    /* format chunk */
{
    unsigned short f_fmttag;	    	    /* format tag, 1 for PCM      */
    unsigned short f_nchannels;    	    /* number of channels         */
    unsigned int f_samplespersec;  	    /* sample rate in hz          */
    unsigned int f_navgbytespersec; 	    /* average bytes per second   */
    unsigned short f_nblockalign;    	    /* number of bytes per frame  */
    unsigned short f_nbitspersample; 	    /* number of bits in a sample */
} t_fmt;

typedef struct _wavechunk	    /* ... and the last two items */
{
    char  wc_id[4]; 	    	    /* data chunk id, e.g., 'data' or 'fmt ' */
    unsigned int wc_size;         	    /* length of data chunk       */
} t_wavechunk;

/* the AIFF header.  I'm assuming AIFC is compatible but don't really know
    that. */

typedef struct _datachunk
{
    char  dc_id[4]; 	    	    /* data chunk id 'SSND'       */
    unsigned int dc_size;         	    /* length of data chunk       */
} t_datachunk;

typedef struct _comm
{
    unsigned short c_nchannels;	            /* number of channels         */
    unsigned short c_nframeshi;    	    /* # of sample frames (hi)    */
    unsigned short c_nframeslo;    	    /* # of sample frames (lo)    */
    unsigned short c_bitspersamp;  	    /* bits per sample            */
    unsigned char c_samprate[10];   /* sample rate, 80-bit float! */
} t_comm;

    /* this version is more convenient for writing them out: */
typedef struct _aiff
{
    char  a_fileid[4];	    	    /* chunk id 'FORM'            */
    unsigned int a_chunksize;     	    /* chunk size                 */
    char  a_aiffid[4];	    	    /* aiff chunk id 'AIFF'       */
    char  a_fmtid[4];	    	    /* format chunk id 'COMM'     */
    unsigned int a_fmtchunksize;   	    /* format chunk size, 18      */
    unsigned short a_nchannels;	            /* number of channels         */
    unsigned short a_nframeshi;    	    /* # of sample frames (hi)    */
    unsigned short a_nframeslo;    	    /* # of sample frames (lo)    */
    unsigned short a_bitspersamp;  	    /* bits per sample            */
    unsigned char a_samprate[10];   /* sample rate, 80-bit float! */
} t_aiff;


#define NS_FORMAT_LINEAR_16     3
#define NS_FORMAT_LINEAR_24     4
#define NS_FORMAT_FLOAT         6
#define SCALE (1./(1024. * 1024. * 1024. * 2.))


#define AIFFHDRSIZE 38	    /* probably not what sizeof() gives */
#define AIFFPLUS (AIFFHDRSIZE + 8)  /* header size including first chunk hdr */

#define WHDR1 sizeof(t_nextstep)
#define WHDR2 (sizeof(t_wave) > WHDR1 ? sizeof (t_wave) : WHDR1)
#define WRITEHDRSIZE (AIFFPLUS > WHDR2 ? AIFFPLUS : WHDR2)

#define READHDRSIZE (16 > WHDR2 + 2 ? 16 : WHDR2 + 2)


class ReadRaw : public Readsf {
 private:

  long ret;
  unsigned char data[WAVCHUNKSIZE * 4 * 2]; //WAVCHUNKSIZE * bytespersamp * num_channels;
  
  int headersize;
  int bytespersamp;
  int bigendian;
 public:
  ReadRaw ( );
  ReadRaw( Input *input );
  virtual ~ReadRaw();
  virtual bool Initialize();
  virtual int Decode(float *buffer,int size);
  virtual bool Rewind();
  virtual bool PCM_seek(long bytes);
  virtual bool TIME_seek(double seconds);
};





#endif
