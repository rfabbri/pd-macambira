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


#ifndef INTERPOLATION_CANDIDATES_HPP_
# define INTERPOLATION_CANDIDATES_HPP_

# include <stddef.h>

// this class is aimed to store the set of available measures,
// referenced by their azimuth and elevation
// required to interpolate the impulse response for a given position
// each candidate has an associated corresponding to its
// contribution in the final result
// a longer class name could be interpolation_candidates
class interp_cdts
{
public:
  interp_cdts() : size(0) {}

  void add(float a1, float a2, float w);

  // number of elements
  size_t	size;
  // first angle index in the map in [-90,90]
  float		angle_index1[4];
  // second angle index in the map in [0,360[
  float		angle_index2[4];
  // weights
  float		weight[4];
};


#endif
