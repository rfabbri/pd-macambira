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
 * ReadVorbisUrl.h
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef READ_VORBIS

#ifndef _READVORBISURL_H_
#define _READVORBISURL_H_

#include "Readsf.h"
#include "generic.h"

extern "C" {
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
}

#include <sys/types.h>
#include <ctype.h>
#include <string.h>

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

//#include <stdlib.h>
//#include <math.h>

#ifdef NT
#include <io.h> /* for 'write' in pute-function only */
#include <winsock.h>
#include <winbase.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <unistd.h>
#define SOCKET_ERROR -1
#endif



//#define VSTREAM_FIFOSIZE (32 * 1024)
//#define VSTREAM_BUFFERSIZE (4 * 1024)
//#define VSOCKET_READSIZE 1024
//#define     STRBUF_SIZE             1024


class ReadVorbis : public Readsf {
 private:

  OggVorbis_File vf;
  ov_callbacks callbacks;
  
  static size_t read_func(void *ptr, size_t size, size_t nmemb, void *datasource);
  static int seek_func(void *datasource, ogg_int64_t offset, int whence);
  static int close_func(void *datasource);
  static long tell_func(void *datasource);
  bool seekable;

 public:
  ReadVorbis( Input *input );
  virtual ~ReadVorbis();
  virtual bool Initialize();
  virtual int Decode(float *buffer, int size);
  virtual bool Rewind();
  virtual bool PCM_seek(long bytes);
  virtual bool TIME_seek(double seconds);
  bool is_seekable() { return seekable;};

};

#endif
#endif //ifdef READVORBIS
