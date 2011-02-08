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


#include "unprocessed_fixed_size_hrtfcont.hpp"
#include "flyweight_ir_factory.hpp"

UnprocessedFixedSizeHrtfCont::UnprocessedFixedSizeHrtfCont(const ir_key& k):
  HrtfCont(k)
{
  HrtfCont*	hc;
  ir_key	k2 = k;

  // set the container's length
  _ir_length = k.length;

  // get the container containing the raw extracted wav
  k2.length = 0;
  hc = FlyweightIrFactory::instance()->hrtf_set_get(k2);

  // iterate on the container storing the raw wav impulse response
  for (angle1_cit ie = hc->map_get()->begin(); ie != hc->map_get()->end(); ie++)
    for (angle2_cit ia2 = ie->second.begin(); ia2 != ie->second.end(); ia2++)
      {
	// get current azimuth and elevation
	const float el = ie->first;
	const float az = ia2->first;

	// allocate buffers
	_m[el][az].lbuf = new float[k.length];
	_m[el][az].rbuf = new float[k.length];
	
	// copy available data
	const size_t av_data_len = _ir_length < hc->ir_length_get() ? _ir_length : hc->ir_length_get();
	for (size_t i = 0; i < av_data_len; ++i)
	  {
	    _m[el][az].lbuf[i] = ia2->second.lbuf[i];
	    _m[el][az].rbuf[i] = ia2->second.rbuf[i];
	  }
	
	// padd with 0 missing data
	for (size_t i = av_data_len; i < _ir_length; ++i)
	  {
	    _m[el][az].lbuf[i] = 0;
	    _m[el][az].rbuf[i] = 0;
	  }

      }
}
