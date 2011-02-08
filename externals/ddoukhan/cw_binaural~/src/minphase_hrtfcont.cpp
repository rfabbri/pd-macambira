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
#include <m_pd.h> //needed for fft implementation
#include "minphase_hrtfcont.hpp"
#include "flyweight_ir_factory.hpp"

MinPhaseHrtfCont::MinPhaseHrtfCont(const ir_key&k):
  HrtfCont(k)
{
  ir_key	k2 = k;

  // set impulse response length
  _ir_length = k.length;
  // FIXME: IR size verification ??? => power of 2

  // get the container storing the coresponding temporal impulse response
  k2.minp_ap_dec = false;
  HrtfCont* hc = FlyweightIrFactory::instance()->hrtf_set_get(k2);
  
  // iterate on the container storing the temporal impulse response
  for (angle1_cit ie = hc->map_get()->begin(); ie != hc->map_get()->end(); ie++)
    for (angle2_cit ia2 = ie->second.begin(); ia2 != ie->second.end(); ia2++)
      {
	// current azimuth and elevation
	const float el = ie->first;
	const float az = ia2->first;

	// allocate buffers
	float *lbuf = _m[el][az].lbuf = new float[_ir_length];
	float *rbuf = _m[el][az].rbuf = new float[_ir_length];
	
	minphase_ir(ia2->second.lbuf, lbuf, _ir_length);
	minphase_ir(ia2->second.rbuf, rbuf, _ir_length);
      }
}

// store in dst the magnitude of spectrum src of size n
// assumption: src is the specetrum of a real signal
void MinPhaseHrtfCont::magnitude(const float* src, float* dst, size_t n)
{
  for (size_t i = 1; i < n/2; ++i)
    {
      dst[i] = sqrt(src[i]*src[i] + src[n-i] * src[n-i]);
      dst[n -i] = dst[i];
    }
  dst[0] = src[0] >= 0 ? src[0] : -src[0];
  dst[n/2] = src[n/2] >= 0 ? src[n/2] : -src[n/2];
}

// store in dst the imaginary part of hilbert transform
// applied to signal src of size n
void MinPhaseHrtfCont::im_hilbert(const float* src, float* dst, size_t n)
{
  size_t i;
  float tmp;
  
  if (dst != src)
    for (i = 0; i < n; ++i)
      dst[i] = src[i];
  
  mayer_realfft(n, dst);
  
  for (i = 1; i < n/2; ++i)
    {
      tmp = dst[i];
      dst[i] = -dst[n -i];
      dst[n-i] = tmp;
    }
  dst[0] = dst[n/2] = 0;
  
  mayer_realifft(n, dst);
  
  for (i = 0; i < n; ++i)
    dst[i] /= n;
}

// store in dst the minphase impulse response corresponding to src
void MinPhaseHrtfCont::minphase_ir(const float* src, float* dst, int n)
{
  float *sig_spectrum = new float[n];
  float *magn, *phase;
  int i;
  
  // compute the spectrum from input signal
  for (i = 0; i < n; ++i)
    sig_spectrum[i] = src[i];
  mayer_realfft(n, sig_spectrum);
  
  // get the magnitude of the spectrum => symetric signal
  magn = new float[n];
  magnitude(sig_spectrum, magn, n);
  
  // compute hilbert transform of the log of the magnitude
  // this computation correspond to the minimum phase
  phase = new float[n];
  for (i = 0; i < n; ++i)
    phase[i] = -log(magn[i]);
  im_hilbert(phase, phase, n);
   
  float* real = new float[n];
  float* imag = new float[n];
  
  for (i = 0; i < n; ++i)
    {
      real[i] = magn[i] * cos(phase[i]);
      imag[i] = magn[i] * sin(phase[i]);
    }
  mayer_ifft(n, real, imag);
  
  delete [] sig_spectrum;
  delete [] imag;
  delete [] magn;
  delete [] phase;

  for (i = 0; i < n; ++i)
    dst[i] = real[i] /= n;
  delete [] real;
}
