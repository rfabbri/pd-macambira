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


#include "itdcont.hpp"
#include "flyweight_ir_factory.hpp"

#include "logstring.hpp"

// returns an itd set corresponding to key k
// assumption: constructor will alwas be called from FlyweightIrFactory
// and the field spectral set to false in the factory
ItdCont::ItdCont(const ir_key& k)
{
  ir_key	k2 = k;
  k2.minp_ap_dec = false;

  HrtfCont*	hc = FlyweightIrFactory::instance()->hrtf_set_get(k2);
  
  for (angle1_cit ia1 = hc->map_get()->begin(); ia1 != hc->map_get()->end(); ia1++)
    for (angle2_cit ia2 = ia1->second.begin(); ia2 != ia1->second.end(); ia2++)
      _m[ia1->first][ia2->first]
	= cross_correlation_itd(ia2->second.lbuf, ia2->second.rbuf, hc->ir_length_get());
}

// compute the cross-correlation between signals s1, s2 of size len_sig
// for n sample time lag
float	ItdCont::cross_correlation(const float* s1, const float* s2, size_t len_sig, int n)
{
  float	res = 0;
  //  slog << "ARG " << n << endl;
  for (int i = (n >= 0 ? 0 : -n); i < (n <= 0 ? (int) len_sig : (int) len_sig - n); ++i)
    res += s1[i] * s2[i+n];

  //  slog << "local cross corr " << n << ":" << res << endl;
  return res;
}

// compute an estimation of the ITD using crosscorrelation
// taking into account the left and right impulse responses
float	ItdCont::cross_correlation_itd(const float *lsig, const float *rsig, size_t len_sig)
{
  int	arg, argmax;
  float val, valmax;
  int ilen_sig = (int) len_sig;

  // slog << "lensig " << len_sig << ilen_sig<< endl;

  //slog << lsig << rsig << len_sig << endl;
  argmax = 1-len_sig;
  valmax = cross_correlation(lsig, rsig, len_sig, argmax);
  for (arg = 2 -  ilen_sig; arg < ilen_sig - 1; ++arg)
    //{
    //  slog << "FOR ARG " << arg << endl;
    if ((val = cross_correlation(lsig, rsig, len_sig, arg)) > valmax)
      {
	argmax = arg;
	valmax = val;
      }
  //  }
  // slog << "ARGMAXXXX " << argmax << endl;
  return argmax;
}

float	ItdCont::itd_from_candidates(const interp_cdts& ic)
{
  float res = _m[ic.angle_index1[0] ][ic.angle_index2[0] ] * ic.weight[0];
  for (size_t i = 1; i < ic.size; ++i)
    res += _m[ic.angle_index1[i]][ic.angle_index2[i]] * ic.weight[i];
  return res;
}
