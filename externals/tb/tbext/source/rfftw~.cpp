/* Copyright (c) 2004 Tim Blechmann.                                            */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "COPYING"  in this distribution.                   */
/*                                                                              */
/*                                                                              */
/* rfftw~ is doing the same as rfft~, but it's based on the fftw library,       */
/* that is much faster that pd's internal fft ...                               */
/*                                                                              */
/*                                                                              */
/* rfftw~ uses the flext C++ layer for Max/MSP and PD externals.                */
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
/* coded while listening to: Wolfgang Mitterer: Radiofractal & Beat Music       */
/*                           Sun Ra: Reflections In Blue                        */
/*                                                                              */
/*                                                                              */



#include <flext.h>

#include "fftw3.h"
#include <algorithm>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error upgrade your flext version!!!!!!
#endif

class rfftw: public flext_dsp
{
  FLEXT_HEADER(rfftw,flext_dsp);

public: // constructor
  rfftw();
  ~rfftw();

protected:
  virtual void m_signal (int n, float *const *in, float *const *out);
  
  fftwf_plan p;    //fftw plan
  int bins;        //number of bins
  float * outreal; //pointer to real output
  float * outimag; //pointer to imaginary output
  
  float * infft;   //array fftw is working on
  float * outfft;  //array fftw uses to output it's values
  

 private:
};


FLEXT_LIB_DSP("rfftw~",rfftw);

rfftw::rfftw()
  :bins(64)
{
  //get ready for the default blocksize
  infft = fftwf_malloc(sizeof(float) * bins);
  outfft = fftwf_malloc(sizeof(float) * bins);
  p=fftwf_plan_r2r_1d(bins,infft,outfft,FFTW_FORWARD,FFTW_MEASURE);
  
  AddInSignal();
  AddOutSignal();
  AddOutSignal();
} 

rfftw::~rfftw()
{
  fftwf_free(infft);
  fftwf_free(outfft);
  fftwf_destroy_plan(p);
} 


void rfftw::m_signal(int n, float *const *in, float *const *out)
{
  //set output pointers
  outreal = out[0];
  outimag = out[1];

  //if blocksize changed, we have to set a new plan for the fft
  if (n!=bins)
    {
      bins=n;

      //re-allocate fft buffers
      fftwf_free(infft);
      infft = fftwf_malloc(sizeof(float) * bins);
      fftwf_free(outfft);
      outfft = fftwf_malloc(sizeof(float) * bins);
      
      //set plan, this might take a few seconds
      //but you don't have to do that on the fly...
      fftwf_destroy_plan(p);
      p=fftwf_plan_r2r_1d(bins,infft,outfft,FFTW_FORWARD,FFTW_MEASURE);
  }

  CopySamples(infft,in[0],n);
  
  //execute
  fftwf_execute(p);

  //Copy samples to outlets
  CopySamples(outreal,outfft,n/2);
  std::reverse_copy(outfft+n/2+1,outfft+n,outimag+1);

  //why do we have to invert the samples???
  for (int i = n/2+1; i!=0;--i)
    {
      *(outimag+i)=-*(outimag+i);
    }

}

