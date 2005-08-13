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
#ifdef READ_MAD

//#include <m_pd.h>
#include "ReadMad.h"

#include <iostream>
using namespace std;


#define mad_f_tofloat(x)  ((float) ((x) / (float) (1L << MAD_F_FRACBITS)))

ReadMad::ReadMad (Input * input) {
  in = input;
  bytes_avail = 0;	// IMPORTANT!!!
  //map_offset = 0;       // do we need this?
  bitrate = 0;
  samplesperframe = 0;
  samplestotal = 0;
  time = 0;
  nr_frames = 0;
  lengthinseconds = 0.0;
}

ReadMad::~ReadMad () {
  if (mad_init) {
    mad_synth_finish (&synth);
    mad_frame_finish (&frame);
    mad_stream_finish (&stream);
    mad_init = 0;
  }
}

bool ReadMad::fill_buffer (long newoffset) {
  size_t bytes_read;
  if (!seekable) return false;
  if ( in->SeekSet( offset + newoffset ) == -1 )
    return false;
  bytes_read = in->Read( (void *)mad_map, MAD_BUFSIZE );
  if (bytes_read == 0) { 
    //cout << "ReadMad:: fillbuf offset, got zero on read, EOF?" << endl; 
    return 0; }
  if (bytes_read < 0 ) { 
    //cout << "ReadMad:: fillbuf offset got -1 on read, ERROR?" << endl; 
    return 0; }
  bytes_avail = bytes_read;
  return true;
  //map_offset = newoffset;
}


bool ReadMad::fill_buffer () {
  size_t bytes_read, br;
  memmove (mad_map, mad_map + MAD_BUFSIZE - bytes_avail, bytes_avail);
  br = bytes_read = MAD_BUFSIZE - bytes_avail;
  bytes_read = in->Read ((void *) (mad_map + bytes_avail), bytes_read);
  if (bytes_read == 0) { 
    //cout << "ReadMad:: got zero on read, EOF? br=%d",br); 
    return 0; 
  }
  if (bytes_read < 0 ) { 
    //cout << "ReadMad:: got -1 on read, ERROR?" << endl; 
    return 0; }
  //map_offset += (MAD_BUFSIZE - bytes_avail);
  bytes_avail += bytes_read;
  return 1;
}


int ReadMad::Decode (float *buffer, int size) {
  struct mad_pcm *pcm;
  mad_fixed_t const *left_ch;
  mad_fixed_t const *right_ch;
  int nsamples;
  int nchannels;
  bool ret =1;
  //if (MAD_BUFSIZE > size)
  //return false;
  
  if (bytes_avail < 3072) {
    //cout << "Filling buffer = %d,%d", bytes_avail,map_offset + MAD_BUFSIZE - bytes_avail);
    ret = fill_buffer ();	// map_offset + MAD_BUFSIZE - bytes_avail);

    if ( ret == 0 && in->get_recover() ) {  // got EOF on stream, but we want to recover
      nsamples = 0;
      while (nsamples < size/2) 
	buffer[nsamples++] = 0.0;
      return nsamples;
    }
    mad_stream_buffer (&stream, mad_map, bytes_avail);
  }


  if (mad_frame_decode (&frame, &stream) == -1) {
    if (!MAD_RECOVERABLE (stream.error)) {
      mad_frame_mute (&frame);
      return 0;
    } else {
      if ( !ret ) {  // error or EOF
	// not if stream input goes down, fill buffer with
	// cout << "ReadMad:: seems we have end of file" << endl;
	return 0;      
      }
      // cout << "ReadMad::MAD error: (not fatal)" << endl;
    }
  }
  
  mad_synth_frame (&synth, &frame);
  {
    pcm = &synth.pcm;
    nsamples = pcm->length;
    nchannels = pcm->channels;
    left_ch = pcm->samples[0];
    right_ch = pcm->samples[1];
    int x = 0;
    while (nsamples--) {
      buffer[x++] = mad_f_tofloat (*(left_ch++));
      if (nchannels == 2)
	buffer[x++] = mad_f_tofloat (*(right_ch++));
    }
  }
  
  bytes_avail = stream.bufend - stream.next_frame;
  //if (pcm->length != 1152) 
  //cout << " %d", pcm->length);
  return pcm->length * nchannels;
}




ssize_t ReadMad::find_initial_frame (uint8_t * buf, int size) {
  uint8_t *data = buf;
  int ext_header = 0;
  int pos = 0;
  ssize_t header_size = 0;
  while (pos < (size - 10)) {
    if (pos == 0 && data[pos] == 0x0d && data[pos + 1] == 0x0a)
      pos += 2;
    if (data[pos] == 0xff && (data[pos + 1] == 0xfb
			      || data[pos + 1] == 0xfa
			      || data[pos + 1] == 0xf3
			      || data[pos + 1] == 0xe2
			      || data[pos + 1] == 0xe3))
      {
	//error("found header at %d", pos);
	return pos;
      }
    if (pos == 0 && data[pos] == 0x0d && data[pos + 1] == 0x0a) {
      return -1;	// Let MAD figure this out
    }
    if (pos == 0 && (data[pos] == 'I' && data[pos + 1] == 'D' && data[pos + 2] == '3')) {
      header_size = (data[pos + 6] << 21) + (data[pos + 7] << 14) + (data[pos + 8] << 7) + data[pos + 9];	/* syncsafe integer */
      if (data[pos + 5] & 0x10) {
	ext_header = 1;
	header_size += 10;	/* 10 byte extended header */
      }
      
      header_size += 10;
      
      if (header_size > MAD_BUFSIZE) {
	//cout << "Header larger than 32K (%d)", header_size);
	return header_size;
      }
      return header_size;
    } else if (data[pos] == 'R' && data[pos + 1] == 'I' &&
	       data[pos + 2] == 'F' && data[pos + 3] == 'F')
      {
	pos += 4;
	//error("Found a RIFF header" << endl;
	while (pos < size) {
	  if (data[pos] == 'd' && data[pos + 1] == 'a'
	      && data[pos + 2] == 't'
	      && data[pos + 3] == 'a')
	    {
	      pos += 8;	/* skip 'data' and ignore size */
	      return pos;
	    } else
	      pos++;
	}
	cout << "MAD debug: invalid header" << endl;
	return -1;
      } else if (pos == 0 && data[pos] == 'T' && data[pos + 1] == 'A'
	     && data[pos + 2] == 'G') {
	return 128;	/* TAG is fixed 128 bytes, we assume! */
      }
    else
      {
	pos++;
      }
  }
  cout << "MAD debug: potential problem file or unhandled info block" << endl;
  //cout << "next 4 bytes =  %x %x %x %x (index = %d, size = %d)",
  //data[header_size], data[header_size + 1],
  //data[header_size + 2], data[header_size + 3],
  //header_size, size);
  return -1;
}


bool ReadMad::Initialize () {
  
  if (in->get_format () == FORMAT_HTTP_MP3)
    seekable = 0;
  else
    seekable = 1;	//assume seekable if its a file
  mad_synth_init (&synth);
  mad_stream_init (&stream);
  mad_frame_init (&frame);
  memset (&xing, 0, sizeof (struct xing));
  xing_init (&xing);
  mad_init = 1;
  
  fill_buffer ();
  offset = find_initial_frame (mad_map, bytes_avail < MAD_BUFSIZE ? bytes_avail : MAD_BUFSIZE);
  
  if (offset < 0) {
    offset = 0; 
    for (int i=0; i < 10; i++) {  // lets try this a coupla times to sync on a proper header
      fill_buffer();       
      offset = find_initial_frame (mad_map,  bytes_avail < MAD_BUFSIZE ? bytes_avail : MAD_BUFSIZE);
      if ( offset > 0) {
	//cout << "ReadMad:: found Header on %d try\n", i);
	break;
      }
    }
    if ( offset < 0 ) {
      cout << "ReadMad::mad_open() couldn't find valid MPEG header\n" << endl;
      return false;
    }
  }
  
  if (offset > bytes_avail) {
    //cout << "ReadMad:: offset > bytes_avail " << endl;
    if ( !fill_buffer() ) { 
      cout << "ReadMad::couldn't read from InputFIFO, bailing..." << endl; 
      return 0;
    }
    mad_stream_buffer (&stream, mad_map, bytes_avail);
  } else { 
    //cout << "ReadMad:: not sure why we are here, offset = %d, bytes_avail = %d", offset, bytes_avail);
    mad_stream_buffer (&stream, mad_map + offset,
		       bytes_avail - offset);
    bytes_avail -= offset;
  }
  
  if ((mad_frame_decode (&frame, &stream) != 0)) {
    //error("MAD error: %s", error_str(data->stream.error, data->str));
    switch (stream.error) {
    case MAD_ERROR_BUFLEN:
      cout << "MAD_ERROR_BUFLEN" << endl;
      return false;	//return 0;
    case MAD_ERROR_LOSTSYNC:
      if (mad_header_decode (&frame.header, &stream) == -1) {
	cout << "ReadMad::Invalid header (" << in->get_filename () << ")" << endl;
      }
      mad_stream_buffer (&stream, stream.this_frame, bytes_avail - (stream.this_frame - mad_map));
      bytes_avail -= (stream.this_frame - mad_map);
      //cout << "avail = %d", data->bytes_avail);
      mad_frame_decode (&frame, &stream);
      break;
    case MAD_ERROR_BADBITALLOC:
      cout << "MAD_ERROR_BADBITALLOC" << endl;
      return false;	//return 0;
    case MAD_ERROR_BADCRC:
      cout << "MAD_ERROR_BADCRC" << endl;	//error_str( stream.error, str));
    case 0x232:
    case 0x235:
      break;
    default:
      cout << "ReadMad:: no valid frame found at start" << endl;
      //cout << "No valid frame found at start (pos: %d, error: 0x%x --> %x %x %x %x) (%s)", offset, stream.error, stream.this_frame[0], stream.this_frame[1], stream.this_frame[2], stream.this_frame[3], in->get_filename ());
      return false;	//return 0;
    }
  }
  if (stream.error != MAD_ERROR_LOSTSYNC)
    if (xing_parse
	(&xing, stream.anc_ptr, stream.anc_bitlen) == 0)
      {
	// We use the xing data later on
      }
  
  num_channels = (frame.header.mode == MAD_MODE_SINGLE_CHANNEL) ? 1 : 2;
  samplerate = (double) frame.header.samplerate;
  bitrate = frame.header.bitrate;
  mad_synth_frame (&synth, &frame);
  
  /* Calculate some values */
  bytes_avail = stream.bufend - stream.next_frame;
  if (seekable) {
    int64_t lframes;
    
    long oldpos = in->SeekCur (0);
    in->SeekEnd (0);
    
    filesize = in->SeekCur (0);
    filesize -= offset;
    
    in->SeekSet (oldpos);
    if (bitrate)
      lengthinseconds =  (float) ((filesize * 8) / (bitrate));
    else
      lengthinseconds = 0.0;
    time = (long) lengthinseconds;
    
    samplesperframe = 32 * MAD_NSBSAMPLES (&frame.header);
    samplestotal = (long) (samplerate * (time + 1));
    lframes = samplestotal / samplesperframe;
    nr_frames = xing.frames ? xing.frames : (int) lframes;
  }
  
  mad_init = 1;
  
  if (xing.frames) {
    //cout << "xing.frames " <<  xing.frames << endl;
    seekable = 1;
  }
 
  
  return true;		//return 1;
}

bool ReadMad::Rewind () {
  if (in == NULL)
    return false;
  if (!seekable)
    return true;
  seek_bytes (0);
  return true;
}

bool ReadMad::PCM_seek (long position) {
  long byte_offset;
  if (!seekable)
    return false;
  byte_offset = (long) (((double) position / (double) samplestotal) * filesize);
  seek_bytes (byte_offset);
  return true;
}
 


bool ReadMad::TIME_seek (double seconds) {
  //double time = ( filesize * 8) / (bitrate);
  long byte_offset;
  if (!seekable)
    return false;
  byte_offset = (int) ((seconds / (double) time) * filesize);
  seek_bytes (byte_offset);
  return true;
}



void ReadMad::seek_bytes (long byte_offset) {
  struct mad_header header;
  int skip;
  mad_header_init (&header);
  
  bytes_avail = 0;
  skip = 0;
  //The total size in bytes for any given  frame can be calculated with the following formula: 
  //FrameSize = 144 * BitRate / (SampleRate + Padding).
  // 417.96 bytes: 144 * 128000 / (44100 + 0)
  if (byte_offset > 4 * 418){
    skip = 3;
  }
  //(seekpos / song length) * (filelength).
  
  //cout << "byte_offset %ld, position %ld, samplestotal %ld, filesize %ld", 
  //   byte_offset, position, samplestotal, filesize);
  fill_buffer ( byte_offset );
  mad_stream_buffer (&stream, mad_map, bytes_avail);
  skip++;
  while (skip--)
    {
      mad_frame_decode (&frame, &stream);
      if (skip == 0)
	mad_synth_frame (&synth, &frame);
    }
  bytes_avail = stream.bufend - stream.next_frame;
  return;
}


// /////////////// xing stuff


void ReadMad::xing_init (struct xing *xing) {
  xing->flags = 0;
}

/*
 * NAME:	xing->parse()
 * DESCRIPTION:	parse a Xing VBR header
 */
int
ReadMad::xing_parse (struct xing *xing, struct mad_bitptr ptr,
		     unsigned int bitlen)
{
	if (bitlen < 64 || mad_bit_read (&ptr, 32) != XING_MAGIC)
		goto fail;

	xing->flags = mad_bit_read (&ptr, 32);
	bitlen -= 64;

	if (xing->flags & XING_FRAMES)
	{
		if (bitlen < 32)
			goto fail;

		xing->frames = mad_bit_read (&ptr, 32);
		bitlen -= 32;
	}

	if (xing->flags & XING_BYTES)
	{
		if (bitlen < 32)
			goto fail;

		xing->bytes = mad_bit_read (&ptr, 32);
		bitlen -= 32;
	}

	if (xing->flags & XING_TOC)
	{
		int i;

		if (bitlen < 800)
			goto fail;

		for (i = 0; i < 100; ++i)
			xing->toc[i] = mad_bit_read (&ptr, 8);

		bitlen -= 800;
	}

	if (xing->flags & XING_SCALE)
	{
		if (bitlen < 32)
			goto fail;

		xing->scale = mad_bit_read (&ptr, 32);
		bitlen -= 32;
	}

	return 0;

      fail:
	xing->flags = 0;
	return -1;
}

#endif
