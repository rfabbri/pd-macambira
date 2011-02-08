/*
    cw_binaural~: a binaural synthesis external for pure data
    by David Doukhan - david.doukhan@gmail.com - http://perso.limsi.fr/doukhan
    and Anne Sedes - sedes.anne@gmail.com
    Copyright (C) 2009-2011  David Doukhan and Anne Sedes

    For more details, see CW_binaural~, a binaural synthesis external for Pure Data
    David Doukhan and Anne Sedes, PDCON09


    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "riff.hpp"


Riff::Riff(int size): GenericRiff(size)
{
  _fifo = new float[size];
  _fifo_id = size-1;
  for (int i = 0; i < size; ++i)
    _fifo[i] = 0;
}

Riff::~Riff()
{

  delete [] _fifo;
}

void Riff::process(float* input, float* output, int n)
{
  int i,j;
  float* curcoeff;
  
  for (i = 0; i < n; ++i)
    {
      float out = 0;
      _fifo[_fifo_id] = input[i];
      
      for (j = _fifo_id, curcoeff = _coeffs; j < _size; ++j, ++curcoeff)
	out += _fifo[j] * *curcoeff;
      for (j = 0; j < _fifo_id; ++j, ++curcoeff)
	out += _fifo[j] * *curcoeff;
      
      output[i] = out;
      _fifo_id = _fifo_id ? _fifo_id - 1 : _size - 1;
    }
}
