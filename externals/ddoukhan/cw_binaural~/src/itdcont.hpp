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


#ifndef ITDCONT_HPP_
# define ITDCONT_HPP_

# include <map>

# include "ir_key.hpp"
# include "interpolation_candidates.hpp"

class ItdCont
{
public:
  ItdCont(const ir_key& k);
  // a positive return value means the right signal is late
  // a negative return value means the left signal is late
  float	itd_from_candidates(const interp_cdts& ic);

  // to remove
  inline map<float,map<float,float> >& map_get() {return _m;}
protected:
  float	cross_correlation_itd(const float *lsig, const float *rsig, size_t len_sig);
  float	cross_correlation(const float* s1, const float* s2, size_t len_sig, int n);

  map<float,map<float,float> >	_m;
};

#endif
