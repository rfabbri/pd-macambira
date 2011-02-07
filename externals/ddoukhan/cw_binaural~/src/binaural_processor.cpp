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

#include "binaural_processor.hpp"
#include "riff.hpp"
#include "fft_riff.hpp"
#include "logstring.hpp"
#include "flyweight_ir_factory.hpp"



BinauralProcessor::~BinauralProcessor()
{
  delete _leftdelay;
  delete _rightdelay;
  delete _leftriff;
  delete _rightriff; 
}

int	BinauralProcessor::set_listen_db(string path)
{
  string re("IRC_[0-9]+_C_R[0-9]+_T([-+]?[0-9]*\\.?[0-9]+)_P([-+]?[0-9]*\\.?[0-9]+)\\.wav");
  return set_hrtf(path, re, true, true);
}

int	BinauralProcessor::set_cipic_db(string path)
{
  string re("subject_[0-9]+_azim_([-+]?[0-9]*\\.?[0-9]+)_elev_([-+]?[0-9]*\\.?[0-9]+)\\.wav");
  return set_hrtf(path, re, true, false);
}


int	BinauralProcessor::set_hrtf(string hrtf_path, string regex, bool azfirst, bool vertpolar)
{
  ir_key	k;
  
  // slog << "Setting hrtf " << hrtf_path << endl;
  k.path = hrtf_path;
  k.length = _length;
  k.iir_regex = regex;
  k.is_azimuth_first = azfirst;
  k.spectral = _spectral_processing;
  k.minp_ap_dec = _minphase_allpass_decomposition;
  k.vertical_polar_coords = vertpolar;
  
  _newhrtfcont = FlyweightIrFactory::instance()->hrtf_set_get(k);
  _itdcont = FlyweightIrFactory::instance()->itd_set_get(k);

  // slog << "hrtfcont" << _newhrtfcont << endl;
  // slog << "itdcont" << _itdcont << endl;

  return 0;
}



void BinauralProcessor::process(float* input, float* azimuths, float* elevations, float* left_output, float* right_output, int n)
{
  // FIXME: BUG IF LEFT OUTPUT = INPUT....
  if (!_newhrtfcont)
    {
      // if no hrtf is selected, output a null signal
      for (int i = 0; i < n; ++i)
	left_output[i] = right_output[i] = 0;
      return;
    }

  // for a given block, consider only the last requested azimuth and elevation
  float az = azimuths[n-1];
  float el = elevations[n-1];

  // update ic to store identifiers of impulse responses to be interpolated
  interp_cdts ic;
  _newhrtfcont->set_candidates(ic, az, el);

  _newhrtfcont->update_from_candidates(ic, _leftriff->coeff_get(), _rightriff->coeff_get());
  
  // Apply Fractional Delay
  if (_itdcont)
    {
      float itd = _itdcont->itd_from_candidates(ic);
      _leftdelay->process(input, left_output, itd < 0 ? -itd: 0, n);
      _rightdelay->process(input, right_output, itd > 0 ? itd : 0, n);
    }
  else
    for (int i = 0; i < n; ++i)
      left_output[i] = right_output[i] = input[i];
  
  // Filter the signal
  _leftriff->process(left_output, left_output, n);
  _rightriff->process(right_output, right_output, n);
}




BinauralProcessor::BinauralProcessor(int impulse_response_size, string& filtering_method, string& delay_method): _newhrtfcont(NULL), _itdcont(NULL)
{

  //slog << "New binaural processor using IR size " << impulse_response_size << ", filter method " << filtering_method << ", delay method " << delay_method << endl;

  // check if the requested filtering method is supported
  if (!filtering_method.compare("RIFF"))
    _spectral_processing = false;
  else if (!filtering_method.compare("FFT"))
    _spectral_processing = true;
  else
    {
      slog << filtering_method << "is not a valid filtering method name: try \"FFT\" or \"RIFF\"" << endl;
      throw 0;
    }

  // check if the requested impulse response size is supported
  // FIXME: check if it is a correct test!! => PUT THAT IN THE FLYWEIGHT
  _length = impulse_response_size;
  while (impulse_response_size > 2)
    impulse_response_size /= 2;
  if (impulse_response_size != 2)
    {
      slog << "Impulse response size should be a positive power of 2: recommended values are 128, 256 or 512" << endl;
      throw 0;
    }

  // *********************
  // TODO
  // at init, use only a dirac impulsion??

  if (_spectral_processing)
    {
      _leftriff = new FftRiff(_length);
      _rightriff = new FftRiff(_length);
    }
  else
    {
      _leftriff = new Riff(_length);
      _rightriff = new Riff(_length);
    }
  
  // delays
  _leftdelay = Delay::create(delay_method, _length);
  _rightdelay = Delay::create(delay_method, _length);
  _minphase_allpass_decomposition = delay_method.compare("nodelay");
}
