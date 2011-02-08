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

#include <math.h>
#include "delay.hpp"
#include "logstring.hpp"

Delay*	Delay::create(string delay_name, unsigned size)
{
  //slog << "creating delay " << delay_name << endl;
  // kind of factory design pattern
  if (!delay_name.compare("Hermite4"))
    return new SCDelay(size);
  else if (!delay_name.compare("nodelay"))
    return new NoDelay(size);
  else if (!delay_name.compare("linear"))
    return new BasicInterpolDelay(size);
  else if (!delay_name.compare("nofractional"))
    return new DummyDelay(size);
  else if (!delay_name.compare("6points"))
    return new CZDelay(size);
  else
    {
      slog << delay_name << " delay method is not supported" << endl;
      throw 0;
    }
}

Delay::Delay(unsigned size): _size(size)
{
  // FIXME
  // ASUMPTION: ITD WILL NEVER BE > SIZE/2
  // IT SHOULD BE CHECKED DURING ITD COMPUTATION
  _fifo = new float[size];
  _fifo_id = 0;
  for (unsigned i = 0; i < size; ++i)
    _fifo[i] = 0;
  _cur_delay = 0;
}

Delay::~Delay()
{
  delete [] _fifo;
}

inline void Delay::push_value(float f)
{
  _fifo_id = (_fifo_id ? _fifo_id - 1 : _size - 1);
  _fifo[_fifo_id] = f;
}

// That ugly macro has been done because of the inneficiency
// of virtual interpolation functions for demanding RT purposes
#define DELAY_PROCESS(code)					\
  unsigned i;							\
  _old_delay = _cur_delay; _cur_delay = new_delay;		\
  float delay = _old_delay;					\
  const float inc_delay = (_cur_delay - _old_delay) / n;	\
								\
  for (i = 0; i < n; ++i, delay += inc_delay)			\
    {								\
      push_value(input[i]);					\
      /* compute the id of the current output element		\
	 delayed by and integer value */			\
      float floor_delay = floorf(delay);			\
								\
      /* get the current element id in the fifo */		\
      unsigned id = _fifo_id + (unsigned) floor_delay;			\
      if (id >= _size)						\
	id -= _size;						\
								\
      /* compute the fractional part of the delay */		\
      float frac_del = delay - floor_delay;			\
								\
      code							\
								\
	output[i] = res;					\
    }


void DummyDelay::process(const float* input, float* output, float new_delay, unsigned n)
{
  DELAY_PROCESS(
    float res = _fifo[id];
    )
}

void BasicInterpolDelay::process(const float* input, float* output, float new_delay, unsigned n)
{
  DELAY_PROCESS(
    unsigned older = (id + 1 == _size ? 0: id + 1);
    float res = _fifo[id] * frac_del + (1 - frac_del) * _fifo[older];
    )
}

void SCDelay::process(const float* input, float* output, float new_delay, unsigned n)
{
  DELAY_PROCESS(
    float y0 = _fifo[id];
    float y1 = _fifo[(id + 1) % _size];
    float y2 = _fifo[(id + 2) % _size];
    float y3 = _fifo[(id + 3) % _size];
    
    // 4-point, 3rd-order Hermite (x-form)
    float c0 = y1;
    float c1 = 0.5 * (y2 - y0);
    float c2 = y0 - 2.5 * y1 + 2. * y2 - 0.5 * y3;
    float c3 = 0.5 * (y3 - y0) + 1.5 * (y1 - y2);
    
    float res = ((c3 * frac_del + c2) * frac_del + c1) * frac_del + c0;
    )
}

void CZDelay::process(const float* input, float* output, float new_delay, unsigned n)
{
  // 6 points interpolation
  DELAY_PROCESS(
    float ym2 = _fifo[id];
    float ym1 = _fifo[(id + 1) % _size];
    float y0 = _fifo[(id + 2) % _size];
    float y1 = _fifo[(id + 3) % _size];
    float y2 = _fifo[(id + 4) % _size];
    float y3 = _fifo[(id + 5) % _size];
    
    float a0= y0;
    float a1= (1./12.)*ym2 - (2./3.)*ym1 + (2./3.)*y1 - (1./12.)*y2;
    float a2= (-1./24.)*ym2 + (2./3.)*ym1 - (5./4.)*y0 + (2./3.)*y1 - (1./24.)*y2;
    float a3= (-3./8.)*ym2 + (13./8.)*ym1 - (35./12.)*y0 + (11./4.)*y1 - (11./8.)*y2 + (7./24.)*y3;
    float a4= (13./24.)*ym2 - (8./3.)*ym1 + (21./4.)*y0 - (31./6.)*y1 + (61./24.)*y2 - (1./2.)*y3;
    float a5= (-5./24.)*ym2 + (25./24.)*ym1 - (25./12.)*y0 + (25./12.)*y1 - (25./24.)*y2 + (5./24.)*y3;
    
    float res = a0 + frac_del * (a1 + frac_del * (a2 + frac_del * (a3 + frac_del * (a4 + frac_del * a5))));
    )
}


void NoDelay::process(const float *input, float *output, float new_delay, unsigned n)
{
  for (unsigned i = 0; i < n; ++i)
    output[i] = input[i];
}
