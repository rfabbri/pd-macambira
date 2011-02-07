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


#include <m_pd.h> //needed for fft implementation
#include "spectral_hrtfcont.hpp"
#include "flyweight_ir_factory.hpp"

SpectralHrtfCont::SpectralHrtfCont(const ir_key& k):
  HrtfCont(k)
{
  ir_key	k2 = k;

  // length is twice the impulse response considered
  _ir_length = k.length * 2;
  // FIXME: IR size verification ??? => power of 2

  // get the container storing the coresponding temporal impulse response
  k2.spectral = false;
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
	
	// fill buffers
	for (size_t i = 0; i < k.length; ++i)
	  {
	    lbuf[i] = ia2->second.lbuf[i];
	    rbuf[i] = ia2->second.rbuf[i];
	    lbuf[i+k.length] = rbuf[i+k.length] = 0;
	  }

	// apply fft to the buffers
	mayer_realfft(_ir_length, lbuf);
	mayer_realfft(_ir_length, rbuf);
      }
}
