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


#ifndef HRTFCONT_HPP_
# define HRTFCONT_HPP_

# include <map>

# include "ir_key.hpp"
# include "interpolation_candidates.hpp"

// atomic element
// contains a buffer for a right and left impulse response
// corresponding to a given azimuth and elevation
typedef struct
{
  float	*lbuf; // left buffer
  float	*rbuf; // right buffer
  //  string fname;
} ir_buffer;


// each HRTF is indexed by two angles in degree: azimuth and elevation
// Depending on the HRTF dabase considered, those angles may be expressed
// in a different coordinate system
//
// For the Listen Database, azimuth and elevation are expressed
// in the Vertical-polar coordinate system (most common spherical coordinate system)
// azimuth is in the range [0,360[ and distance between 2 azimuth is circular
// elevation is in the range [-90,+90]
// Consequently, database expressed in Vertical-polar coordinate system
// will be indexed by elevation first, and then azimuth
//
// For the CIPIC Database, azimuth and elevation are expressed
// in the Interaural-polar coordinate system
// elevation is in the range [0,360[ and distance between 2 elevations is circular
// azimuth is in the range [-90,+90]
// Consequently, database expressed in Interoral-polar coordinate system
// will be indexed by elevation first, and then azimuth

// store impulse responses indexed by an angle whose values are in [0,360[ degree
// azimuth in vertical-polar coords, or elevation in interaural-polar coords
// corresponds to the second index of the storage map
typedef map<float, ir_buffer>		angle2_cont;
// const interator on the second angle index
typedef angle2_cont::const_iterator	angle2_cit;
// type used to store HRTFs corresponding of a given subject
// indexed by an angle in [-90,+90[ first and then by an angle in [0,360[
typedef map<float, angle2_cont>		hrtf_map;
// const interator on the first angle angle index
typedef map<float, angle2_cont>::const_iterator angle1_cit;


// HRTF Container
class HrtfCont
{
public:
  HrtfCont(const ir_key& k);

  // return the map indexing all available impulse responses
  map<float, map<float, ir_buffer> >* map_get() {return &_m;}

  inline size_t	ir_length_get() const  {return _ir_length;}

  // update and interpolation structure containing the candidates to be interpolated
  void set_candidates(interp_cdts& ic, float az, float el);
  // called from set_candidates, for a fixed angle index 1
  // add the angle index2 candidates
  void add_a2_candidates(interp_cdts& ic, float a1_key, float a2, float weight);

  // update filter coefficients from interpolation candidates
  void update_from_candidates(const interp_cdts& ic, float* left, float* right);

protected:
  // normalize vertical polar coordinates expressed in degree
  // ie: set elevation in [-90, +90], and azimuth in [-180, 180[
  void normalize_vertpolar_coords(float& azimuth, float& elevation) const;

  // angular distance (ie dist(350,0) == 10)
  float angular_dist(float a1, float a2) const;

  // convert an azimuth/elevation couple expressed in
  // vertical polar coordinates to interaural polar coordinates
  void vertpol2interaurpol(float& az, float& el) const;

  size_t	_ir_length;
  hrtf_map	_m;
  const bool	_vert_pol_coords;
};

#endif
