/* Copyright (c) 2004 Tim Blechmann.                                            */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL     */
/* WARRANTIES, see the file, "COPYING"  in this distribution.                   */
/*                                                                              */
/*                                                                              */
/* rifftw~ is doing the same as rifft~, but it's based on the fftw library,     */
/* that is much faster that pd's internal fft ...                               */
/*                                                                              */
/*                                                                              */
/* rifftw~ uses the flext C++ layer for Max/MSP and PD externals.               */
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
/* coded while listening to: Caged/Uncaged                                      */
/*                           Sunny Murray: Hommage To Africa                    */
/*                                                                              */
/*                                                                              */



#include <flext.h>

#include "fftw3.h"
#include <algorithm>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error upgrade your flext version!!!!!!
#endif

class rifftw: public flext_dsp
{
  FLEXT_HEADER(rifftw,flext_dsp);

public: // constructor
  rifftw();
  ~rifftw();

protected:
  virtual void m_signal (int n, float *const *in, float *const *out);
  
  fftwf_plan p;    //fftw plan
  int bins;        //number of bins
  float * inreal; //pointer to real input
  float * inimag; //pointer to imaginary input
  
  fftwf_complex * incomplex;

  //float * infft;   //array fftw is working on
  float * outfft;  //array fftw uses to output it's values
  

 private:
};


FLEXT_LIB_DSP("rifftw~",rifftw);

rifftw::rifftw()
  :bins(64)
{
  //get ready for the default blocksize
  //  infft = fftwf_malloc(sizeof(float) * bins);
  outfft = fftwf_malloc(sizeof(float) * bins);


  incomplex = fftwf_malloc(sizeof(fftwf_complex) * bins);


  //  p=fftwf_plan_r2r_1d(bins,infft,outfft,FFTW_BACKWARD,FFTW_MEASURE);
  p=fftwf_plan_dft_c2r_1d(bins,incomplex,outfft,FFTW_BACKWARD,FFTW_MEASURE);
  
  AddInSignal();
  AddInSignal();
  AddOutSignal();
} 

rifftw::~rifftw()
{
  //  fftwf_free(infft);
  fftwf_free(outfft);
  fftwf_free(incomplex);
  fftwf_destroy_plan(p);
} 


void rifftw::m_signal(int n, float *const *in, float *const *out)
{
  //set output pointers
  inreal = in[0];
  inimag = in[1];

  //if blocksize changed, we have to set a new plan for the fft
  if (n!=bins)
    {
      bins=n;

      //re-allocate fft buffers
      //      fftwf_free(infft);
      //      infft = fftwf_malloc(sizeof(float) * bins);
      fftwf_free(outfft);
      outfft = fftwf_malloc(sizeof(float) * bins);

      fftwf_free(incomplex);
      incomplex = fftwf_malloc(sizeof(fftwf_complex) * bins);
      
      //set plan, this might take a few seconds
      //but you don't have to do that on the fly...
      fftwf_destroy_plan(p);
      //p=fftwf_plan_r2r_1d(bins,infft,outfft,FFTW_BACKWARD,FFTW_MEASURE);
      p=fftwf_plan_dft_c2r_1d(bins,incomplex,outfft,FFTW_BACKWARD,FFTW_MEASURE);

  }

  //Copy samples to the fft
  //  CopySamples(infft,inreal,n/2);
  //  std::reverse_copy(inimag,inimag+n/2,infft+n/2);
  
  
  /*
  //why do we have to invert the samples???
    for (int i = n/2+1; i!=n;++i)
    {
      *(infft+i)=-*(infft+i);
    }
  */

  for (int i=0;i!=n/2;++i)
    {
      incomplex[i][0]=inreal[i];
      incomplex[i][1]=-inimag[i];
    }

  //execute
  fftwf_execute(p);

  CopySamples(out[0],outfft,n);

}

