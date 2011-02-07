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


#ifndef FFTRIFF_HPP_
# define FFTRIFF_HPP_

# include "generic_riff.hpp"

class FftRiff: public GenericRiff
{
public:
	FftRiff(int size);//: GenericRiff(size) {}
	~FftRiff();
	virtual void process(float* input, float* output, int n);
	//float* coeff_get() {return _coeffs;}
protected:
	// void process_core(float* input, float*output);
	//void swap_fpointer((float*)& a, (float*)& b);
	//const int _size;
	//int _fifo_id;
	//float *_last_output;
	//float *_window;
	float *_tmp_input;
	float *_tmp_output;
	float *_tmp_rest;

	//float	*_cur_input, *_next_input;
	//float	*_cur_output, *_next_output;
	//int		_inputpos;
};

#endif
