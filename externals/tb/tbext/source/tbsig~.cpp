/* Copyright (c) 2003 Tim Blechmann.                                            */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "COPYING"  in this distribution.                   */
/*                                                                              */
/*                                                                              */
/* tbsig~ is gives you the sign of an audio signal. should be useful to create  */
/* a square oscillator or some noisy sounds                                     */
/*                                                                              */
/*                                                                              */
/* tbsig~ uses the flext C++ layer for Max/MSP and PD externals.                */
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
/* coded while listening to: Peter Broetzmann & Hamid Drake: The Dried Rat-Dog  */
/*                                                                              */
/*                                                                              */



#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error upgrade your flext version!!!!!!
#endif

class tbsig: public flext_dsp
{
  FLEXT_HEADER(tbsig,flext_dsp);

public: // constructor
  tbsig();

protected:
  virtual void m_signal (int n, float *const *in, float *const *out);

};


FLEXT_LIB_DSP("tbsig~",tbsig);

tbsig::tbsig()
{
  AddInSignal();
  AddOutSignal();
} 


void tbsig::m_signal(int n, float *const *in, float *const *out)
{
  const float *ins=in[0];
  float *outs = out[0];

  while (n--)
    {
      if (*ins>0) 
	*outs = 1;
      else
	*outs = -1;
      *outs++;
      *ins++;
    }
 
}
