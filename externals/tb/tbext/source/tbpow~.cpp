/* Copyright (c) 2003 Tim Blechmann.                                            */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "COPYING"  in this distribution.                   */
/*                                                                              */
/*                                                                              */
/* tbpow~ calculates the power of each sample. In fact i expected it to sound   */
/* better than it does. but maybe someone is interested in using it...          */
/*                                                                              */
/*                                                                              */
/* tbpow~ uses the flext C++ layer for Max/MSP and PD externals.                */
/* get it at http://www.parasitaere-kapazitaeten.de/PD/ext                      */
/* thanks to Thomas Grill                                                       */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* See file LICENSE for further informations on licensing terms.                */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/*                                                                              */
/*                                                                              */
/* coded while listening to: Assif Tsahar & Susie Ibarra: Home Cookin'          */
/*                           Painkiller: Talisman                               */
/*                                                                              */



#include <flext.h>

#include <cmath>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error upgrade your flext version!!!!!!
#endif

class tbpow: public flext_dsp
{
  FLEXT_HEADER(tbpow,flext_dsp);

public: // constructor
  tbpow();

protected:
  virtual void m_signal (int n, float *const *in, float *const *out);

  void set_power(float f);
  
private:
  float power;
  
  FLEXT_CALLBACK_1(set_power,float)
  
};


FLEXT_LIB_DSP("tbpow~",tbpow);

tbpow::tbpow()
{
  AddInSignal();
  AddInFloat();
  AddOutSignal();
  
  FLEXT_ADDMETHOD(1,set_power);
  power=1;
} 


void tbpow::m_signal(int n, float *const *in, float *const *out)
{
  const float *ins=in[0];
  float *outs = out[0];

  while (n--)
    {
      *outs = pow(*ins,power);
      *outs++;
      *ins++;
    }
 
}

void tbpow::set_power(float f)
{
  power=f;
}
