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
 * main.h
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif


#include <flext.h>
#include <stdio.h>


extern "C" {
#include <samplerate.h>  //libsamplerate
}

#include "Readsf.h"
#include "ReadRaw.h"
#ifdef READ_VORBIS
#include "ReadVorbis.h"
#endif
#ifdef READ_MAD
#include "ReadMad.h"
#endif
#ifdef READ_MAD_URL
#include "ReadMadUrl.h"
#endif
#ifdef READ_FLAC
#include "ReadFlac.h"
#endif

#include "Fifo.h"
#include "generic.h"
#include "Input.h"
#include "InputFile.h"
#include "InputStream.h"

// check for appropriate flext version
#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 400)
#error You need at least flext version 0.4.0
#endif


class readanysf: public flext_dsp
{
  // obligatory flext header (class name,base class name)
  FLEXT_HEADER(readanysf, flext_dsp)

public:
  
  readanysf();
  ~readanysf();

  
protected:
  // here we declare the virtual DSP function
  virtual void m_signal(int n, float *const *in, float *const *out);

  void m_open(t_symbol *s);		// open file Input; set req = R_OPEN
  void m_reopen();		// re open a file Input; set req = R_OPEN
  void m_start();				// set state = S_PROCESS
  void m_pause();				// set state = S_IDLE
  void m_stop();				// set state = S_STATE; cond.signal();

  void m_child();
  void FillFifo();
  
  //void m_loop_s(t_symbol *s);
  void m_recover(float f);
  void m_loop_f(float f);
  void m_set_tick(int i);
  void m_pcm_seek(int i);
  void m_time_seek(float f);
  void m_src_factor(float f);

  void m_bang();

  int m_resample(int frames);

  int getState();
  int getRequest();
  void setState(int i);
  void setRequest(int i);
  void setSys(int state, int request);
private:
  FLEXT_CALLBACK_S(m_open)     
  FLEXT_CALLBACK(m_reopen)     


  FLEXT_CALLBACK_F(m_loop_f)     
  FLEXT_CALLBACK_F(m_recover)     
  FLEXT_CALLBACK_I(m_set_tick)     
  FLEXT_CALLBACK(m_start)     
  FLEXT_CALLBACK(m_pause)     
  FLEXT_CALLBACK(m_stop)     

  FLEXT_CALLBACK_I(m_pcm_seek)     
  FLEXT_CALLBACK_F(m_time_seek)

  FLEXT_CALLBACK_F(m_src_factor)     

  FLEXT_THREAD(m_child)
  FLEXT_CALLBACK(m_bang)


  Readsf *readsf;
  ThrCond cond;
  ThrMutex sysmut;
  ThrMutex varmutex;
  Fifo *fifo;
  Input *in;
  int format;
  int outtick, counttick;
  int fifosecondsize;

  volatile int state, request;
  volatile bool loop, eof, quit, sendout;
  volatile float floatmsg, cachemsg, lengthinseconds;
  
  char filename[1024];
  int num_channels;
  double samplerate;

  volatile long pcmseek;
  volatile double timeseek;

  float read_buffer[READBUFFER];
  float pd_buffer[1024*2*FLOATSIZE];

  ///Secret Rabbit Code stuff
  int src_channels;
  float *src_buffer;
  SRC_STATE * src_state;
  SRC_DATA  src_data;
  double src_factor;
  int src_error;
  int src_mode;
};
