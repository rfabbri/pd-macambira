/*
 * readanysf~  external for pd. 
 * 
 * Copyright (C) 2003, 2004 August Black
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
 * ReadFlac.h
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef READ_FLAC

#ifndef _READFLAC_H
#define _READFLAC_H

extern "C" {
#include "FLAC/stream_decoder.h"
};

#include "Readsf.h"
#include "generic.h"

class ReadFlac  : public Readsf {
 public:
  ReadFlac( Input *input);
  virtual ~ReadFlac();
  virtual bool Initialize();
  virtual int Decode(float *buffer, int size);
  virtual bool Rewind();
  virtual bool PCM_seek(long bytes);
  virtual bool TIME_seek(double seconds);

 protected:
  bool needs_seek;
  FLAC__uint64 seek_sample;
  unsigned samples_in_reservoir;
  bool abort_flag;
  FLAC__StreamDecoder *decoder;
  FLAC__StreamMetadata streaminfo;
  FLAC__int16 reservoir[FLAC__MAX_BLOCK_SIZE * 2 * 2]; // *2 for max channels, another *2 for overflow
  //unsigned char output[576 * 2 * (16/8)]; // *2 for max channels, (16/8) for max bytes per sample
  
  unsigned lengthInMsec() { return (unsigned)((FLAC__uint64)1000 * streaminfo.data.stream_info.total_samples / streaminfo.data.stream_info.sample_rate); }
 private:
  void ErrorCheck(int state);
  long filelength;
  void cleanup();



  static FLAC__StreamDecoderReadStatus readCallback_(const FLAC__StreamDecoder *decoder, 
							     FLAC__byte buffer[], 
							     unsigned *bytes, 
							     void *client_data);

  static FLAC__StreamDecoderWriteStatus writeCallback_(const FLAC__StreamDecoder *decoder, 
						       const FLAC__Frame *frame, 
						       const FLAC__int32 * const buffer[], 
						       void *client_data);


  static FLAC__bool eofCallback_(const FLAC__StreamDecoder *decoder, void *client_data);

  static void metadataCallback_(const FLAC__StreamDecoder *decoder, 
				const FLAC__StreamMetadata *metadata, 
				void *client_data);
  static void errorCallback_(const FLAC__StreamDecoder *decoder, 
			     FLAC__StreamDecoderErrorStatus status, 
			     void *client_data);
};

#endif
#endif //#ifdef READ_FLAC
