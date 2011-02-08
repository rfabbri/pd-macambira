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



#ifndef SPECTRAL_HRTFCONT_HPP_
# define SPECTRAL_HRTFCONT_HPP_

# include "hrtfcont.hpp"

// This class is used to store spectral domain impulse responses
// The corresponding temporal impulse response of size n
// will be 0-padded to size 2n before putting it to spectral domain
// In order to be able to use the spectral impulse responses
// in an overlap-add filter
class SpectralHrtfCont: public HrtfCont
{
public:
  SpectralHrtfCont(const ir_key& k);
};

#endif
