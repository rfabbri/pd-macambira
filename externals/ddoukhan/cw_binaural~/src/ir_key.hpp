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


#ifndef IR_KEY_HPP_
# define IR_KEY_HPP_

# include <string>

using namespace std;

// this structure is aimed at identifying
// a requested impulse response set
// that will be use as it

typedef struct s_ir_key
{
  // TODO
  // * add ITD computation method!!
  // * add resampling options

  // tells if we want a minphase/allpass decomposition
  // having this options set to true implies more computations
  // and a better hrtf interpolation
  bool		minp_ap_dec;
  // tells whether we want to use the impulse response
  // in the frequency domain or in the time domain
  bool		spectral;
  // the length of the considered impulse response
  size_t	length;
  // the path to the directory storing the set of impulse responses
  string	path;
  // the regex that should match the name of valid IR files
  // contains 2 groups corresponding to the azimuth and elevation
  // composing the sound file name
  string	iir_regex;
  // set to true if the azimuth is in the first group of iir_regex
  // false if the elevation is first
  bool		is_azimuth_first;
  // set to true in the db is indexed in vertical-polar coordinates
  // set to false if the db is indexed in interaural-polar_coordinates
  bool		vertical_polar_coords;

  // constructor init the structure fields to default values
  s_ir_key(): minp_ap_dec(false),
	      spectral(false),
	      length(0),
	      path(""),
	      iir_regex(""),
	      is_azimuth_first(true),
	      vertical_polar_coords(true) {}
} ir_key;


// The impulse response key comparison function
// returns true if its first argument is less than its second argument,
// and false otherwise
// it is necessary to index ir_key into sorted containers (map, etc...)
class ir_key_isless
{
public:
  bool operator() (const ir_key& a, const ir_key& b);
};


#endif
