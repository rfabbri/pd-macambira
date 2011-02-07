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


#ifndef BINAURALPROCESSOR_HPP_
# define BINAURALPROCESSOR_HPP_

# include <string>
# include "generic_riff.hpp"
# include "delay.hpp"
# include "ir_key.hpp"
# include "hrtfcont.hpp"
# include "itdcont.hpp"


class BinauralProcessor
{
public:
  BinauralProcessor(int impulse_response_size, std::string& filtering_method, std::string& delay_method);
  ~BinauralProcessor();
  
  // do the processing for a given filter
  void process(float* input, float *azimuths, float* elevations, float* left_output, float* right_output, int n);

  // load a HRTF database, assuming it is LISTEN database
  int	set_listen_db(string path);

  // load a HRTF database, assuming it is CIPIC database
  int	set_cipic_db(string path);

  // load any hrtf database
  int	set_hrtf(string hrtf_path, string regex, bool azfirst, bool vertpolar);
	
protected:


  // we should do an optimized riff structure
  GenericRiff *_leftriff, *_rightriff;
  Delay* _leftdelay, *_rightdelay;

  // creation arguments
  int	_length;
  bool	_spectral_processing;
  bool	_minphase_allpass_decomposition;

  // hrtf ad itd database
  HrtfCont	*_newhrtfcont;
  ItdCont	*_itdcont;
};


#endif
