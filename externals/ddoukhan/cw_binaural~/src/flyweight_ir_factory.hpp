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


#ifndef FLYWEIGHT_IR_FACTORY_HPP_
# define FLYWEIGHT_IR_FACTORY_HPP_

# include <map>
# include "ir_key.hpp"

# include "hrtfcont.hpp"
# include "itdcont.hpp"


// This class is a mixed flyweight and factory approach
// aimed to obtain the requested impulse response
class FlyweightIrFactory
{
public:
  // there should be only one flyweightfactory
  // so we use a singleton design pattern
  static FlyweightIrFactory*	instance();

  // get a hrtf set container corresponding
  // to a given set of parameters
  HrtfCont*			hrtf_set_get(const ir_key& k);
  // get a set of itd corresponding to a given set of parameters
  ItdCont*			itd_set_get(ir_key k);
private:
  // check if a given key is consistent
  bool				is_hrtf_set_key_consistent(const ir_key& k);

  // tells if a number is a power of 2
  bool				ispowerof2(size_t n);

  // All Hrtf sets will be stored in a hash map
  // indexed by the requested preprocessing options
  // on the hrtf set
  map<ir_key, HrtfCont*, ir_key_isless>	_hrtf_db; 
  
  // a separate map is used for ITD computations
  // on a longer term, it will allow to compute itd using
  // different methods
  map<ir_key, ItdCont*, ir_key_isless>		_itd_db;

  FlyweightIrFactory() {}
  static FlyweightIrFactory*	_instance;
};

#endif
