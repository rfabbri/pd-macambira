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
 * ReadRaw.cpp  || code here was kindly 'borrowed' from d_soundfile.c from
 * puredata source code by Miller Puckette 
 */

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>

#include "ReadRaw.h"
#include <m_pd.h>

//# define _F_FRACBITS         28
//# define do_f_fromint(x)       ((x) << _F_FRACBITS)
#define SCALE (1./(1024. * 1024. * 1024. * 2.))

using namespace std;



int ambigendian(void) {
  unsigned short s = 1;
  unsigned char c = *(char *)(&s);
  return (c==0);
}


static unsigned int swap4 (unsigned int n, int doit) {
  if (doit)
    return (((n & 0xff) << 24) | ((n & 0xff00) << 8) |
	    ((n & 0xff0000) >> 8) | ((n & 0xff000000) >> 24));
  else
    return (n);
}

static unsigned short swap2 (unsigned int n, int doit) {
  if (doit)
    return (((n & 0xff) << 8) | ((n & 0xff00) >> 8));
  else
    return (n);
}

#define ULPOW2TO31      ((unsigned int)0x80000000)
#define DPOW2TO31       ((double)2147483648.0)	/* 2^31 */

static double myUlongToDouble (unsigned int ul) {
  double val;
  if (ul & ULPOW2TO31)
    val = DPOW2TO31 + (ul & (~ULPOW2TO31));
  else
    val = ul;
  return val;
}

static double ieee_80_to_double (unsigned char *p) {
  unsigned char sign;
  short lexp = 0;
  unsigned int mant1 = 0;
  unsigned int mant0 = 0;
  double val;
  lexp = *p++;
  lexp <<= 8;
  lexp |= *p++;
  sign = (lexp & 0x8000) ? 1 : 0;
  lexp &= 0x7FFF;
  mant1 = *p++;
  mant1 <<= 8;
  mant1 |= *p++;
  mant1 <<= 8;
  mant1 |= *p++;
  mant1 <<= 8;
  mant1 |= *p++;
  mant0 = *p++;
  mant0 <<= 8;
  mant0 |= *p++;
  mant0 <<= 8;
  mant0 |= *p++;
  mant0 <<= 8;
  mant0 |= *p++;
  if (mant1 == 0 && mant0 == 0 && lexp == 0 && sign == 0)
    return 0.0;
  else {
    val = myUlongToDouble (mant0) * pow (2.0, -63.0);
    val += myUlongToDouble (mant1) * pow (2.0, -31.0);
    val *= pow (2.0, ((double) lexp) - 16383.0);
    return sign ? -val : val;
  }
}

ReadRaw::ReadRaw ()
{

}

ReadRaw::ReadRaw (Input * input) {
  //char file;
  in = input;
  bigendian = ambigendian();
  //bigendian = 0;  
}

ReadRaw::~ReadRaw () {
  if (in != NULL)
    in->Close ();
}

bool ReadRaw::Initialize () {
  char buf[128];
  int format, swap;
  //long bytelimit = 0x7fffffff;
  
  
  if (in == NULL) {
    cout << "ReadRaw:: Input is NULL, this is bad, bailing...." << endl;
    //shouldn't ever happen, but just checking
    return false;	//cout << "already opened, now closing file" << endl;
  }
  
  int bytesread = in->Read (buf, READHDRSIZE);
  
  if (bytesread < 4) {
    cout << "ReadRaw:: bytesread is < 4, this is bad, bailing...." << endl;
    return false;
  }
  format = in->get_format ();  // we know the format already
  
  
  if (format == FORMAT_NEXT){	/* nextstep header */
    
    //unsigned int param;
    bigendian = 1;
    swap = (bigendian != ambigendian());

    if (bytesread < (int) sizeof (t_nextstep)) { 
      cout << "ReadRaw:: bytesread < sizeof(nextstep), this is bad, bailing...."<< endl; 
      return false; 
    }
    num_channels = swap4 (((t_nextstep *) buf)->ns_nchans, swap);
    format = swap4 (((t_nextstep *) buf)->ns_format, swap);
    samplerate = (double) swap4( ((t_nextstep *) buf)->ns_sr, swap );
    
    headersize = swap4 (((t_nextstep *) buf)->ns_onset, swap);
    if (format == NS_FORMAT_LINEAR_16)
      bytespersamp = 2;
    else if (format == NS_FORMAT_LINEAR_24)
      bytespersamp = 3;
    else if (format == NS_FORMAT_FLOAT)
      bytespersamp = 4;
    else 
      return false;
    
    //bytelimit = 0x7fffffff;
    
  } else if (format == FORMAT_WAVE) {	/* wave header */
    
    /* This is awful.  You have to skip over chunks,
     * except that if one happens to be a "fmt" chunk, you want to
     * find out the format from that one.  The case where the
     * "fmt" chunk comes after the audio isn't handled. */
    
    bigendian = 0;
    swap = (bigendian != ambigendian());

    headersize = 12;
    if (bytesread < 20) {
      cout << "ReadRaw:: bytesread < 20, this is bad, bailing...." << endl;
      return false;
    }
    /* First we guess a number of channels, etc., in case there's
     * no "fmt" chunk to follow. */
    num_channels = 1;
    bytespersamp = 2;
    /* copy the first chunk header to beginnning of buffer. */
    memcpy (buf, buf + headersize, sizeof (t_wavechunk));
    
    /* read chunks in loop until we get to the data chunk */
    while (strncmp (((t_wavechunk *) buf)->wc_id, "data", 4)) {
      long chunksize =  swap4 (((t_wavechunk *) buf)->wc_size,
			       swap), seekto = headersize + chunksize + 8, seekout;
      
      if (!strncmp(((t_wavechunk *) buf)->wc_id, "fmt ", 4)) {
	long commblockonset = headersize + 8;
	seekout = in->SeekSet ( commblockonset);
	if (seekout != commblockonset) {
	  cout << "ReadRaw:: Seek prob, seekout != commblockonset" << endl;
	  return false;
	}
	if ( in->Read ( buf, sizeof (t_fmt) ) <   (int) sizeof (t_fmt)) {
	  cout << "ReadRaw:: Read prob, read < sizeopf(t_fmt)" << endl;
	  return false;
	} 
	
	num_channels = swap2 (((t_fmt *) buf)->f_nchannels, swap);
	samplerate = (double) swap2 (((t_fmt *) buf)->f_samplespersec, swap);
	
	int sampsize = swap2 (((t_fmt *) buf)->f_nbitspersample, swap);
	
	if (sampsize == 16)
	  bytespersamp = 2;
	else if (sampsize == 24)
	  bytespersamp = 3;
	else if (sampsize == 32)
	  bytespersamp = 4;
	else {
	  cout << "ReadRaw:: bytespersamp is not supported, samplesize= "<<  sampsize << endl;
	  //return false;
	}
      }
      seekout = in->SeekSet ( seekto );
      if (seekout != seekto) {
	cout << "ReadRaw:: Seek prob, seekout != seekto"<< endl;
	return false;
      }
      if ( in->Read ( buf, sizeof (t_wavechunk) ) < (int) sizeof (t_wavechunk)) {
	cout << "ReadRaw:: Read prob, read < sizeof(wavechunk)" << endl;
	return false;
      }
      /* cout << "new chunk %c %c %c %c at %d",
       * ((t_wavechunk *)buf)->wc_id[0],
       * ((t_wavechunk *)buf)->wc_id[1],
       * ((t_wavechunk *)buf)->wc_id[2],
       * ((t_wavechunk *)buf)->wc_id[3], seekto); */
      headersize = seekto;
    }
    //bytelimit = swap4 (((t_wavechunk *) buf)->wc_size, swap);
    headersize += 8;
  } else {
    /* AIFF.  same as WAVE; actually predates it.  Disgusting. */
    bigendian = 1;
    swap = (bigendian != ambigendian());

    headersize = 12;
    if (bytesread < 20)
      return false;
    /* First we guess a number of channels, etc., in case there's
     * no COMM block to follow. */
    num_channels = 1;
    bytespersamp = 2;
    /* copy the first chunk header to beginnning of buffer. */
    memcpy (buf, buf + headersize, sizeof (t_datachunk));
    /* read chunks in loop until we get to the data chunk */
    while (strncmp (((t_datachunk *) buf)->dc_id, "SSND", 4)) {
      long chunksize =	swap4 (((t_datachunk *) buf)->dc_size,
			       swap), seekto = headersize + chunksize + 8, seekout;
      
      if (!strncmp (((t_datachunk *) buf)->dc_id, "COMM", 4)) {
	long commblockonset = headersize + 8;
	seekout = in->SeekSet ( commblockonset );
	if (seekout != commblockonset)
	  return false;
	if ( in->Read (buf, sizeof (t_comm)) <
	     (int) sizeof (t_comm))
	  return false;
	num_channels = swap2 (((t_comm *) buf)->c_nchannels, swap);
	samplerate = ieee_80_to_double (((t_comm *) buf)->c_samprate);
		
	format = swap2 (((t_comm *) buf)->c_bitspersamp, swap);
	if (format == 16)
	  bytespersamp = 2;
	else if (format == 24)
	  bytespersamp = 3;
	else
	  return false;
      }
      seekout = in->SeekSet ( seekto );
      if (seekout != seekto)
	return false;
      if ( in->Read (buf, sizeof (t_datachunk)) <
	   (int) sizeof (t_datachunk))
	return false;
      headersize = seekto;
    }
    //bytelimit = swap4 (((t_datachunk *) buf)->dc_size, bigendian);
    headersize += 8;
  }
  
  //cout << "ReadRaw:: [%s] %1.0lf (Hz), %d chan(s), bps %d",in->get_filename(),
  //   samplerate, num_channels, bytespersamp);
  //cout << " headersize = %d", headersize);
  
  long tmp = in->SeekEnd(0);  // get filesize
  if (tmp == -1)
    post ("couldn't seek on file");
  lengthinseconds = (float) ((tmp - headersize) / bytespersamp / samplerate /  num_channels);
  
  /* seek past header and any sample frames to skip */
  if ( ( in->SeekSet( headersize ) ) != -1 ) {
    return true;
  } else {
    cout << "ReadRaw:: strange, wasn't able to seek on the file" << endl;
    return false;
  }
}

bool ReadRaw::Rewind () {
  if ( ( in->SeekSet( headersize ) ) != -1 )
    return true;
  else
    return false;
}

int ReadRaw::Decode (float *buffer, int size) {
  int ret, x = 0;;
  int chunk = WAVCHUNKSIZE * bytespersamp * num_channels;
  int bytesperframe = bytespersamp * num_channels;
  unsigned char *sp;
  float ftmp;

  if (chunk > size)
    return 0;
  ret = in->Read ( data, chunk );
  ret = ret * bytespersamp;
  if (bytespersamp == 2) {
    
    for (int j = 0; j < ret; j += bytespersamp) {
      sp = (unsigned char *) &data[j];
      if (bigendian)
	ftmp = SCALE * ((sp[0] << 24) | (sp[1] << 16));
      else
	ftmp = SCALE * ((sp[1] << 24) | (sp[0] << 16));
      buffer[x++] = ftmp;
      //if (num_channels == 1) buffer[x++] = ftmp;
      sp += bytesperframe;
    }
    
  } else if (bytespersamp == 3) {
    
    for (int j = 0; j < ret; j += bytespersamp) {
      sp = (unsigned char *) &data[j];
      if (bigendian)
	ftmp = SCALE * ((sp[0] << 24) | (sp[1] << 16) | (sp[2] << 8));
      else
	ftmp = SCALE * ((sp[2] << 24) | (sp[1] << 16) | (sp[0] << 8));
      buffer[x++] = ftmp;
      //if (num_channels == 1) buffer[x++] = ftmp;
      sp += bytesperframe;
    }
    
  } else if (bytespersamp == 4) {
    
    for (int j = 0; j < ret; j += bytespersamp) {
      sp = (unsigned char *) &data[j];
      if (bigendian)
	ftmp = (float) ((sp[0] << 24) | (sp[1] << 16) | (sp[2] << 8) | sp[3]);
      else
	ftmp = (float) ((sp[3] << 24) | (sp[2] << 16) | (sp[1] << 8) | sp[0]);
      buffer[x++] = ftmp;
      //if (num_channels == 1) buffer[x++] = ftmp;
      sp += bytesperframe;
    }
    
  }
  
  return x / 2;		//num_channels; //always two
}



bool ReadRaw::PCM_seek (long frames) {
  if (frames > (long) (lengthinseconds * samplerate))
    return false;
  if ( in->SeekSet ( headersize + (frames * num_channels * bytespersamp ) ) != -1 )
    return true;
  else {
    cout <<  "ReadRaw:: fuck, no seeking!!" << endl;
    return false;
  }
}

bool ReadRaw::TIME_seek (double seconds) {
  long frames = (long) (seconds * samplerate);
  return PCM_seek (frames);
}
