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
 * ReadMad.cpp  || much code studied from Andy Lo-A-Foe <www.alsaplayer.org>
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#ifdef READ_MAD

#ifndef _READMAD_H_
#define _READMAD_H_

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>


#define MAD_BUFSIZE (4 * 1024)
#define XING_MAGIC    (('X' << 24) | ('i' << 16) | ('n' << 8) | 'g')

extern "C" { 	
#include <mad.h>
}

//#include "xing.h"
#include "Readsf.h"
#include "Input.h"


#define FRAME_RESERVE	2000


struct xing {
  long flags;			/* valid fields (see below) */
  unsigned long frames;		/* total number of frames */
  unsigned long bytes;		/* total number of bytes */
  unsigned char toc[100];	/* 100-point seek table */
  long scale;			/* ?? */
};

enum {
  XING_FRAMES = 0x00000001L,
  XING_BYTES  = 0x00000002L,
  XING_TOC    = 0x00000004L,
  XING_SCALE  = 0x00000008L
};

# define xing_finish(xing)	/* nothing */


class ReadMad : public Readsf {

 private:
  //FILE *mad_fd;
  uint8_t mad_map[MAD_BUFSIZE];
  long map_offset;
  int bytes_avail;

  struct mad_synth  synth; 
  struct mad_stream stream;
  struct mad_frame  frame;
  mad_timer_t timer;
  struct xing xing;

  int mad_init;
  ssize_t offset;
  ssize_t filesize;
  //  int samplerate;
  int bitrate;
  int samplesperframe;
  long samplestotal;
  long time;
  int seekable;
  int nr_frames;

  //float my_scale(mad_fixed_t sample);
  bool fill_buffer( );
  bool fill_buffer( long newoffset );
  int mad_frame_seek( int frame );
  ssize_t find_initial_frame(uint8_t *buf, int size);
  void seek_bytes(long byte_offset);
  
  void xing_init(struct xing *);
  int xing_parse(struct xing *, struct mad_bitptr, unsigned int);

 public:
  ReadMad();
  ReadMad( Input *input );
  virtual ~ReadMad();
  virtual bool Initialize();
  virtual int Decode(float *buffer, int size);
  virtual bool Rewind();
  virtual bool PCM_seek(long bytes);
  virtual bool TIME_seek(double seconds);  
};		

#endif
#endif //ifdef READ_MAD
