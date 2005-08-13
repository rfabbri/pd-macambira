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
 * ReadVorbis.cpp || much code (C) by Olaf Matthes
 * http://www.akustische-kunst.org/puredata/shout/shout.html
 */

#ifdef READ_VORBIS

#include "ReadVorbis.h"
//#include <m_pd.h>

#include <iostream>
using namespace std;


ReadVorbis::ReadVorbis( Input *input) {
  in = input;
  seekable = false;
  callbacks.read_func = read_func;
  callbacks.seek_func = seek_func;
  callbacks.close_func = close_func;
  callbacks.tell_func = tell_func;
}

ReadVorbis::~ReadVorbis(){

  if ( ov_clear(&vf) != 0) 
    cout << "ReadVorbis::couldn't deconstruct the vorbis file reader" << endl;
  //if (fp != NULL) { //don't know why, but this will cause a segfault
  // fclose(fp); 
  //}
}


bool ReadVorbis::Initialize( ) {  

  vorbis_info *vi;

  if ( in == NULL ) {
    return 0;
  }
  if ( in->get_format() == FORMAT_HTTP_VORBIS ) {
    seekable = false;
  } else 
    seekable = true;
  
  int err = ov_open_callbacks( (void *)in, &vf, NULL, 0, callbacks);
  if( err < 0 ) {
    cout << "ReadVorbis:: does not appear to be an Ogg bitstream.\n" << endl;
    switch (err) {
    case OV_EREAD:
      cout << "ReadVorbis:: read from media retuned an error" << endl;
      break;
    case OV_ENOTVORBIS:
      cout << "ReadVorbis:: bistream is not vorbis data" << endl;
      break; 
    case OV_EVERSION:
      cout << "ReadVorbis:: vorbis version mismatch" << endl;
      break;
    case OV_EBADHEADER:
      cout << "ReadVorbis:: invalid vorbis btistream header" << endl;
      break;
    case OV_EFAULT:
      cout << "ReadVorbis:: internal logic fault" << endl;
      break;
    default:
      cout << "ReadVorbis:: general error on ov_open_callbacks" << endl;
      break;
    }
    return false;
  } 


  vi=ov_info(&vf,-1);
  samplerate = (double)vi->rate;
  num_channels = vi->channels;
  if ( in->get_format() == FORMAT_HTTP_VORBIS )
    lengthinseconds = 0.0;
  else 
    lengthinseconds = (double)ov_time_total(&vf, -1);
  //cout << "ReadVorbis: opening url: [%s] %ld (Hz), %d chan(s)", 
  //   fullurl, vi->rate, vi->channels);
  
  return true;
}

int ReadVorbis::Decode(float *buffer, int size) {
  long ret = 0;
  int current_section;
  float **buftmp;
  int x=0;
  if (CHUNKSIZE > (unsigned int)size) return 0;


  ret = ov_read_float(&vf, &buftmp , CHUNKSIZE,  &current_section);
  if (ret == 0 ) {
    // This means it is a definite end of file, lets return zero here.
    return 0;
  } else  if (ret <= 0) {
    switch (ret) {
    case OV_HOLE:
      cout << "ReadVorbis:: there was an interruption in the data. " << endl;
      cout << "one of: garbage between pages, loss of sync followed" << 
	" by recapture, or a corrupt page" << endl;
      break;
    case OV_EBADLINK:
      cout << "ReadVorbis:: an invalid stream section was supplied " << 
	"to libvorbisfile, or the requested link is corrupt" << endl;
      break; 
    default:
      cout << "ReadVorbis:: unknown error on ov_read_float" << endl;
      break;
    }
    
    for(int j = 0; j < 1024; j++) {
      buffer[x++] = 0.0;
    }
    return 512;

  } else {

    // we should check here to see if ret is larger than size!
    for(int j = 0; j < ret; j++) {
      buffer[x++] = buftmp[0][j];
      if (num_channels == 2) 
      	buffer[x++] = buftmp[1][j];
    }
  }
  //cout << "x %d", x);
  return x;  //ret;

}

size_t ReadVorbis::read_func(void *ptr, size_t size, size_t nmemb, void *datasource) {

  Input * tmpin = ( Input *)datasource;
  unsigned int   get = size*nmemb;
  size_t ret;
  // cout << "ReadVorbis:: calling read function" << endl;

  ret = tmpin->Read( ptr,  get);

  //cout << "read from fifo, get %d,  ret %d, size %d, nmemb %d", 
  //get, ret,size,nmemb );

  return ret;//size*nmemb;
}

int ReadVorbis::seek_func(void *datasource, ogg_int64_t offset, int whence) {
  // InputStream will always return -1
  Input * tmpin = ( Input *)datasource;
  switch ( whence ) {
    case SEEK_SET:
      return tmpin->SeekSet( offset );
      break;
    case SEEK_CUR:
      return tmpin->SeekCur( offset );
      break;
    case SEEK_END:
      return tmpin->SeekEnd( offset );
      break;
    default:
      return -1;
      break;
    }
    
}

int ReadVorbis::close_func(void *datasource) {
  
  cout << "ReadVorbis:: calling close function" << endl;
  //ov_clear(&vf);
  //return tmpin->Close();
  return 0;
}

long ReadVorbis::tell_func(void *datasource) {
	// InputStream will always return -1
	Input * tmpin = ( Input *)datasource;
    return tmpin->SeekCur( 0 );   

}

bool ReadVorbis::Rewind() {
  ov_pcm_seek(&vf, 0);
  return true;  // need to return true here for fill_buffer of main.cpp
}

bool ReadVorbis::PCM_seek(long bytes) {
  int ret = ov_pcm_seek(&vf, bytes);
  if ( ret == 0)
    return true;
  else {
    switch (ret) {
    case OV_ENOSEEK:
      cout << "ReadVorbis:: stream not seekable" << endl;
      break;
    case OV_EINVAL:
	  ret = ov_pcm_seek(&vf, 0);
	  if (ret == 0) 
	    return true;
	  else 
	    cout << "ReadVorbis:: invalid argument" << endl;
	  
	  break; 
    case OV_EREAD:
      cout << "ReadVorbis:: read returned an error" << endl;
      break;
    case OV_EOF:
      cout << "ReadVorbis:: End of File" << endl;
      break;
    case OV_EBADLINK:
      cout << "ReadVorbis:: invalid stream section" << endl;
      break;
    default:
      cout << "ReadVorbis:: some other seek error PCM_seek" << endl;
      break;
    }
    return false;
  }
}

bool ReadVorbis::TIME_seek(double seconds) {
  int ret = ov_time_seek(&vf, seconds);
  if ( ret == 0)
    return true;
  else {
    switch (ret) {
    case OV_ENOSEEK:
      cout << "ReadVorbis:: stream not seekable" << endl;
      break;
    case OV_EINVAL:
      cout << "ReadVorbis:: invalid argument" << endl;
      break; 
    case OV_EREAD:
      cout << "ReadVorbis:: read returned an error" << endl;
      break;
    case OV_EOF:
      cout << "ReadVorbis:: End of File" << endl;
      break;
    case OV_EBADLINK:
      cout << "ReadVorbis:: invalid stream section" << endl;
      break;
    default:
      cout << "ReadVorbis:: some other seek error Time_seek" << endl;
      break;
    }
    return false;
  }
}




#endif
