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


#include "ir_key.hpp"

// The impulse response key comparison function
// returns true if its first argument is less than its second argument,
// and false otherwise
bool	ir_key_isless::operator() (const ir_key& a, const ir_key& b)
{
  int	tmp;

  if (a.minp_ap_dec < b.minp_ap_dec)
    return true;
  if (a.minp_ap_dec > b.minp_ap_dec)
    return false;

  if (a.spectral < b.spectral)
    return true;
  if (a.spectral > b.spectral)
    return false;

  if (a.length < b.length)
    return true;
  if (a.length > b.length)
    return false;

  if (a.spectral < b.spectral)
    return true;
  if (a.spectral > b.spectral)
    return false;

  tmp = a.path.compare(b.path);
  if (tmp < 0)
    return true;
  if (tmp > 0)
    return false;

  tmp = a.iir_regex.compare(b.iir_regex);
  if (tmp < 0)
    return true;
  if (tmp > 0)
    return false;

  if (a.is_azimuth_first < b.is_azimuth_first)
    return true;
  if (a.is_azimuth_first > b.is_azimuth_first)
    return false;

  if (a.vertical_polar_coords < b.vertical_polar_coords)
    return true;
  if (a.vertical_polar_coords > b.vertical_polar_coords)
    return false;

  // keys are equal, then !(a < b)
  return false;
}
