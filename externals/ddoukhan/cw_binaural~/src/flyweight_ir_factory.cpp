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


#include "flyweight_ir_factory.hpp"
#include "logstring.hpp"
#include "raw_wav_hrtfcont.hpp"
#include "unprocessed_fixed_size_hrtfcont.hpp"
#include "spectral_hrtfcont.hpp"
#include "minphase_hrtfcont.hpp"

FlyweightIrFactory*	FlyweightIrFactory::_instance = NULL;

FlyweightIrFactory*	FlyweightIrFactory::instance()
{
  //slog << "flyw instance" << endl;
  if (!_instance)
    _instance = new FlyweightIrFactory();
  return _instance;
}

HrtfCont*	FlyweightIrFactory::hrtf_set_get(const ir_key& k)
{
  //slog << "calling the factory" << endl;
  // if the key is in the map, then it is valid, and an be returned as it
  if (_hrtf_db.find(k) != _hrtf_db.end())
    return _hrtf_db[k];
  
  // slog << "key not in database, creating new db" << endl;

  // check that the key is valid
  if (!is_hrtf_set_key_consistent(k))
    {
      slog << "key not consistent" << endl;
      return NULL;
    }

  // By convention, length == 0 means
  // getting the unprocessed hrtf at its original size
  if (!k.length)
    return (_hrtf_db[k] = new RawWavHrtfCont(k));

  // spectral representation of the filter database
  // to be used in overlap-add
  if (k.spectral)
    return (_hrtf_db[k] = new SpectralHrtfCont(k));

  // minmum phase filter
  if (k.minp_ap_dec)
    return (_hrtf_db[k] = new MinPhaseHrtfCont(k));

  // Unprocessed impulse response of fixed size
  // this test is useless
  if (!k.minp_ap_dec && !k.spectral)
    return (_hrtf_db[k] = new UnprocessedFixedSizeHrtfCont(k));

  //slog << "returb null" << endl;
  return NULL;
}

bool		FlyweightIrFactory::is_hrtf_set_key_consistent(const ir_key& k)
{
  if (!k.path.length() || !k.iir_regex.length())
    return false;

  if (!k.length && (k.minp_ap_dec || k.spectral))
    return false;

  if ((k.minp_ap_dec || k.spectral) && !ispowerof2(k.length))
    {
      slog << "For spectral filtering or for minphase decompostion, "
	   << "the impulse response size should be a power of 2" << endl;
      return false;
    }
  return true;
}

bool		FlyweightIrFactory::ispowerof2(size_t n)
{
  return (n > 0) && !(n & (n - 1));
}

ItdCont*	FlyweightIrFactory::itd_set_get(ir_key k)
{
  // if no minphase/allpass decomposition has been done
  // then, we won't use fractional delay and ITD information
  if (!k.minp_ap_dec)
    return NULL;

  // the itd will be the same idependently of the spectral field
  // so we put spectral to an arbitrary value to avoid duplicates
  k.spectral = false;

  // if the key is in the map, then it is valid, and an be returned as it
  if (_itd_db.find(k) != _itd_db.end())
    return _itd_db[k];

  // Only one ITD computation method is available for now
  return (_itd_db[k] = new ItdCont(k));
}
