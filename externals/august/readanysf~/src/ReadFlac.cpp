/*
 * readanysf~  external for pd. 
 * 
 * Copyright (C) 2003,2004 August Black
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
 * ReadFlac.cpp
 *
 * much of the code comes from FLAC input plugin for Winamp3
 * distributed with the flac source under the GPL
 * Copyright (C) 2000,2001,2002,2003  Josh Coalson
 */

#ifdef READ_FLAC

//#include <m_pd.h>
#include "ReadFlac.h"
#include <iostream>

extern "C" {
#include "FLAC/metadata.h"
};

using namespace std;

ReadFlac::ReadFlac( Input *input ) {
  in=input;
  needs_seek = false;
  seek_sample = 0;
  samples_in_reservoir =0;
  abort_flag = false;
  decoder = NULL;
  filelength = 0;
}

ReadFlac::~ReadFlac() {
  cleanup();
}

bool ReadFlac::Initialize( ) {

  //@@@ to be really "clean" we should go through the reader instead of directly to the file...
  if(!FLAC__metadata_get_streaminfo(in->get_filename(), &streaminfo)) {
    cout << "what the fuck" << endl;
    return 1;
  }
  
  //length_msec = lengthInMsec();
  /*cout << "FLAC:<%ihz:%ibps:%dch>", 
       streaminfo.data.stream_info.sample_rate, 
       streaminfo.data.stream_info.bits_per_sample, 
       streaminfo.data.stream_info.channels); //@@@ fix later
  */

  samplerate = (double)streaminfo.data.stream_info.sample_rate;
  num_channels = streaminfo.data.stream_info.channels;
  lengthinseconds = streaminfo.data.stream_info.total_samples/samplerate;

  filelength = in->SeekEnd(0);
  filelength = in->SeekCur(0);
  in->SeekSet(0);

  decoder = FLAC__stream_decoder_new();
  if(decoder == 0)
    return false;
  
  FLAC__stream_decoder_set_read_callback(decoder, readCallback_);
  FLAC__stream_decoder_set_write_callback(decoder, writeCallback_);

  FLAC__stream_decoder_set_metadata_callback(decoder, metadataCallback_);
  FLAC__stream_decoder_set_error_callback(decoder, errorCallback_);
  FLAC__stream_decoder_set_client_data(decoder, this);
    
  if(FLAC__stream_decoder_init(decoder) != FLAC__STREAM_DECODER_SEARCH_FOR_METADATA ) {
    cleanup();
    return false;
  }
  if(!FLAC__stream_decoder_process_until_end_of_metadata(decoder)) {
    cleanup();
    return false;
  }
  
  return true;
}



int ReadFlac::Decode(float *buffer, int size) {
  
  if(decoder == NULL)
    return 0;
  
  //while (samples_in_reservoir < 576) {
  //if (samples_in_reservoir < 576) {
  if(FLAC__stream_decoder_get_state(decoder) == FLAC__STREAM_DECODER_END_OF_STREAM) {
    cout << "FLAC: end of file" << endl;
    return 0;
  } else if(!FLAC__stream_decoder_process_single(decoder)) {

    //ErrorCheck( FLAC__stream_decoder_get_state(decoder) );
    //ErrorCheck( FLAC__stream_decoder_finish(decoder) );
    //ErrorCheck( FLAC__stream_decoder_init(decoder) );
    //FLAC__stream_decoder_reset(decoder);
    //FLAC__stream_decoder_flush(decoder);
    cout << "FLAC: no process single " << endl;
    //break;
    //exit(1);
    //return 0;
    //return samples_in_reservoir;
  }
  //}

  int n = samples_in_reservoir; // > 576 ? samples_in_reservoir: 576;
  const unsigned channels = streaminfo.data.stream_info.channels;

  if(samples_in_reservoir == 0) {
    //cout << "FLAC: reservoir is empty" << endl;
    return 0;
  }  else {
    
    //const unsigned bits_per_sample = streaminfo.data.stream_info.bits_per_sample;
    //const unsigned bytes_per_sample = (bits_per_sample+7)/8;
    //const unsigned sample_rate = streaminfo.data.stream_info.sample_rate;
    unsigned i;
    //16 > WHDR2 + 2 ? 16 : WHDR2 + 2
    
    //unsigned  delta;
    

    for(i = 0; i < n*channels; i++)
      buffer[i] = (float) ( reservoir[i]/ 32768.0 );
    

    samples_in_reservoir = 0;
    
    //const int bytes = n * channels * bytes_per_sample;
  }
  
  //if(eof)
  //return 0;
  
  return n*channels; //1;
}

bool ReadFlac::Rewind() {

  cleanup();
  Initialize();
  samples_in_reservoir = 0;
  //ErrorCheck( FLAC__stream_decoder_get_state(decoder) );
  //FLAC__stream_decoder_seek_absolute(decoder, 0);
  return true;
}

bool ReadFlac::PCM_seek(long bytes) {
  cout << "ReadFlac:: no seeking on flac files, sorry" << endl;
  return false;
}

bool ReadFlac::TIME_seek(double seconds) {
  cout << "ReadFlac:: no seeking on flac files, sorry" << endl;
  return false;
}


void ReadFlac::cleanup()
{
  if(decoder) {
    FLAC__stream_decoder_finish(decoder);
    FLAC__stream_decoder_delete(decoder);
    decoder = NULL;
  }
}

FLAC__StreamDecoderReadStatus ReadFlac::readCallback_(const FLAC__StreamDecoder *decoder, 
							     FLAC__byte buffer[], 
							     unsigned *bytes, 
							     void *client_data) {
  ReadFlac *instance = (ReadFlac*)client_data;
  *bytes = instance->in->Read( (char *)buffer, *bytes);
  if (*bytes == 0) {
    cout << "FLAC: read returned 0" << endl;
    return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM ;
  } else {
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE ;
  }
}


FLAC__StreamDecoderWriteStatus ReadFlac::writeCallback_(const FLAC__StreamDecoder *decoder, 
							const FLAC__Frame *frame, 
							const FLAC__int32 * const buffer[], 
							void *client_data) {
  ReadFlac *instance = (ReadFlac*)client_data;
  //const unsigned bps = instance->streaminfo.data.stream_info.bits_per_sample;
  const unsigned channels = instance->streaminfo.data.stream_info.channels;
  const unsigned wide_samples = frame->header.blocksize;
  unsigned wide_sample, sample, channel;
  
  (void)decoder;
  
  if(instance->abort_flag) {
    return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
  }
  //cout << "FLAC: blocksize = " << wide_samples << endl;
  for(sample = instance->samples_in_reservoir*channels, wide_sample = 0; 
      wide_sample < wide_samples; wide_sample++)
    for(channel = 0; channel < channels; channel++, sample++)
      instance->reservoir[sample] = (FLAC__int16)buffer[channel][wide_sample];
  
  instance->samples_in_reservoir += wide_samples;
  
  return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void ReadFlac::metadataCallback_(const FLAC__StreamDecoder *decoder, 
				const FLAC__StreamMetadata *metadata, 
				void *client_data) {
  ReadFlac *instance = (ReadFlac*)client_data;
  (void)decoder;

  //cout << "FLAC: metadata callback" << endl;
  if(metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
    instance->streaminfo = *metadata;
    
    if(instance->streaminfo.data.stream_info.bits_per_sample != 16) {
      cout << "\nFLAC: bps is not 16 ..Aboorting ...\n" << endl;
      instance->abort_flag = true;
      //exit(1);
      return;
    }
  }
}

void ReadFlac::errorCallback_(const FLAC__StreamDecoder *decoder, 
			     FLAC__StreamDecoderErrorStatus status, 
			     void *client_data) {
  ReadFlac *instance = (ReadFlac*)client_data;
  (void)decoder;
  if(status != FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC) {
    cout << "FLAC: error callback - lost sync, trying reset,flush" << endl;
    FLAC__stream_decoder_reset(instance->decoder);
    FLAC__stream_decoder_flush(instance->decoder);
    //instance->abort_flag = true;
  }
}

void ReadFlac::ErrorCheck(int state) {
  switch (state) {
  
  case FLAC__STREAM_DECODER_END_OF_STREAM :
    cout << "END_OF_STREAM " << endl;
    break;
  case FLAC__STREAM_DECODER_MEMORY_ALLOCATION_ERROR :
    cout << "MEMORY_ALLOCATION_ERROR " << endl;
    break;
  case FLAC__STREAM_DECODER_READ_FRAME :
    cout << "READ_FRAME " << endl;
    break;
  case FLAC__STREAM_DECODER_INVALID_CALLBACK :
    cout << "INVALID_CALLBACK " << endl;
    break;
  case FLAC__STREAM_DECODER_UNINITIALIZED :
    cout << "UNINITIALIZED " << endl;
    break;
  case FLAC__STREAM_DECODER_ABORTED :
    cout << "ABORTED " << endl;
  default:
    cout << "OK" << endl;
    break;
  }
}


#endif
