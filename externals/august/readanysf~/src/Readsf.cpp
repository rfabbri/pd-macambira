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
 * Readsf.cpp
 */

#include "Readsf.h"


Readsf::Readsf( )
{
}

Readsf::Readsf( Input *input )
{
  in = input;
  num_channels = 1;
  samplerate = 44100;
  lengthinseconds =0;
}
Readsf::~Readsf()
{

}

int Readsf::Decode(float *buffer, int size) {
  if (CHUNKSIZE > (unsigned int)size) return 0;
  for(unsigned int c=0;c< CHUNKSIZE;c++) {
    buffer[c] = 0.0;
  }
  return CHUNKSIZE;
}

bool Readsf::Initialize() 
{
  //printf( "%s\n", filename);
  return false;
}
bool Readsf::Rewind() {
  return false;
}

bool Readsf::PCM_seek(long bytes) {
  return false;
}

bool Readsf::TIME_seek(double seconds) {
  return false;
}
