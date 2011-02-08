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
#include <m_pd.h>

#include "fft_riff.hpp"
#include "logstring.hpp"


FftRiff::FftRiff(int size): GenericRiff(size)
{
	delete [] _coeffs;
	_coeffs = new float[2* size];

	for (int i = 0; i < 2*size; ++i)
		_coeffs[i] = 0;

	_tmp_input = new float[2 * size];
	_tmp_output = new float[2 * size];
	_tmp_rest = new float[2*size];
	// we divide by size for not having to normalize after a fft + ifft
	for (int i = 0; i < 2*size; ++i)
		_tmp_rest[i] = 0;
}

FftRiff::~FftRiff()
{
	delete [] _tmp_input;
	delete [] _tmp_output;
	delete [] _tmp_rest;
}


void FftRiff::process(float* input, float* output, int n)
{
	int		i;

	for (i = 0; i < n; ++i)
		_tmp_input[i] = input[i] / (_size * 2);
	for (i = n; i < _size * 2; ++i)
		_tmp_input[i] = 0;

	mayer_realfft(2* _size, _tmp_input);

	_tmp_output[0] = _tmp_input[0] * _coeffs[0];
	_tmp_output[_size] = _tmp_input[_size] * _coeffs[_size];

	/// multiply the spectrum of the input and the hrtf
	for (int i = 1; i < _size; ++i)
	{
		// real part
		_tmp_output[i] = _tmp_input[i] * _coeffs[i] - _tmp_input[2*_size-i] * _coeffs[2*_size-i];
		// imag part
		_tmp_output[2*_size-i] = _tmp_input[i] * _coeffs[2*_size-i] + _tmp_input[2*_size-i] * _coeffs[i];
	} 


	mayer_realifft(2*_size, _tmp_output);

	// output the current block beginning + the corresponding rests
	for (i = 0; i < n; ++i)
		output[i] = _tmp_output[i] + _tmp_rest[i];
	for (i = n; i < 2*_size-n; ++i)
		_tmp_rest[i-n] = _tmp_rest[i] + _tmp_output[i];
	for (i = 2*_size - n; i < 2 * _size; ++i)
		_tmp_rest[i-n] = _tmp_output[i];
}
