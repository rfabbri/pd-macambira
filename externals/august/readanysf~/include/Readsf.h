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
 * Readsf.h
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef _READSF_H_
#define _READSF_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Input.h"

class Readsf  {

private:


 public:
  Readsf();
  Readsf( Input *input );
  
  virtual ~Readsf();
  virtual bool Initialize();
  virtual int Decode(float *buffer, int size);
  virtual bool Rewind();
  virtual bool PCM_seek(long bytes);
  virtual bool TIME_seek(double seconds);
  double get_samplerate() { return samplerate;}
  int get_channels() { return num_channels;}
  float get_lengthinseconds() { return lengthinseconds; }

 protected:
  Input *in;
  double samplerate;
  int num_channels;
  float lengthinseconds;
  bool eof;
};

#endif
