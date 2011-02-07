/*
    cw_binaural~: a binaural synthesis external for pure data
    by David Doukhan - david.doukhan@gmail.com - http://www.limsi.fr/Individu/doukhan
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

#ifndef GENERICRIFF_HPP_
# define GENERICRIFF_HPP_

// this is an astract class for Riff filters
// TODO: REMOVE the size attribute: it may change
//  depending on the block size
// currently is is dependant of the HRTF impulse response size...

class GenericRiff
{
public:
  GenericRiff(int size);
  virtual ~GenericRiff();
  virtual void process(float* input, float* output, int n) = 0;
  float* coeff_get() {return _coeffs;}
protected:
  const int _size;
  float *_coeffs;
};

#endif
