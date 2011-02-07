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


#ifndef DELAY_HPP_
# define DELAY_HPP_

# include <string>

using namespace std;

class Delay
{
public:
  Delay(unsigned size);
  virtual ~Delay();
  virtual void process(const float* input, float* output, float new_delay, unsigned n) = 0;
  static Delay* create(string delay_name, unsigned size);
protected:
  inline void push_value(float f);
  
  const unsigned _size;
  unsigned _fifo_id;
  float *  _fifo;
  float _old_delay, _cur_delay;
};


// Does no fractional Delay
class DummyDelay: public Delay
{
public:
  DummyDelay(int size): Delay(size) {}
  virtual void process(const float* input, float* output, float new_delay, unsigned n);
};

// 1st order fractional Delay
class BasicInterpolDelay: public Delay
{
public:
  BasicInterpolDelay(int size): Delay(size) {}
  virtual void process(const float* input, float* output, float new_delay, unsigned n);
};

// SuperCollider fractional Delay
class SCDelay: public Delay
{
public:
  SCDelay(int size): Delay(size) {}
  virtual void process(const float* input, float* output, float new_delay, unsigned n);
};


// 6th order fractional Delay
class CZDelay: public Delay
{
public:
  CZDelay(int size): Delay(size) {}
  virtual void process(const float* input, float* output, float new_delay, unsigned n);
};


class NoDelay: public Delay
{
public:
  NoDelay(int size): Delay(size) {}
  virtual void process(const float* input, float* output, float new_delay, unsigned n);
};

#endif
