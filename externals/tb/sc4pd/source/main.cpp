/* sc4pd
   library initialization

   Copyright (c) 2004 Tim Blechmann.               

   This code is derived from:
	SuperCollider real time audio synthesis system
    Copyright (c) 2002 James McCartney. All rights reserved.
	http://www.audiosynth.com


   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,             
   but WITHOUT ANY WARRANTY; without even the implied warranty of         
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

   Based on:
     PureData by Miller Puckette and others.
         http://www.crca.ucsd.edu/~msp/software.html
     FLEXT by Thomas Grill
         http://www.parasitaere-kapazitaeten.net/ext
     SuperCollider by James McCartney
         http://www.audiosynth.com
     
   Coded while listening to: Phosphor
*/

#include <flext.h>
#include "SC_PlugIn.h"

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 406)
#error You need at least FLEXT version 0.4.6
#endif

#define SC4PD_VERSION "0.01"


void sc4pd_library_setup()
{
    post("\nsc4pd: by tim blechmann");
    post("based on SuperCollider by James McCartney");
    post("version "SC4PD_VERSION);
    post("compiled on "__DATE__);
    post("contains: Dust(~), MantissaMask(~), Hasher(~), Median(~), "
	 "BrownNoise(~),\n"
	 "          ClipNoise(~), GrayNoise(~), Dust2(~), WhiteNoise(~), "
	 "PinkNoise(~), \n          Crackle(~), Rand(~), TRand(~), "
	 "TExpRand(~), IRand(~), TIRand(~),\n          CoinGate, "
	 "LinRand(~), NRand(~), ExpRand(~), LFClipNoise(~),\n"
	 "          LFNoise0(~), LFNoise1(~), LFNoise2(~), Logistic(~), "
	 "Latoocarfian(~),\n"
	 "          LinCong(~), amclip(~), scaleneg(~), excess(~), hypot(~), "
	 "ring1(~),\n"
	 "          ring2(~), ring3(~), ring4(~), difsqr(~), sumsqr(~), "
	 "sqrdif(~),\n"
	 "          sqrsum(~), absdif(~), LFSaw(~), LFPulse(~)");

    //initialize objects
    FLEXT_DSP_SETUP(Dust_ar);
    FLEXT_SETUP(Dust_kr);

    FLEXT_DSP_SETUP(MantissaMask_ar);
    FLEXT_SETUP(MantissaMask_kr);

    FLEXT_DSP_SETUP(Hasher_ar);
    FLEXT_SETUP(Hasher_kr);
    
    FLEXT_DSP_SETUP(Median_ar);
    FLEXT_SETUP(Median_kr);

    FLEXT_DSP_SETUP(BrownNoise_ar);
    FLEXT_SETUP(BrownNoise_kr);

    FLEXT_DSP_SETUP(ClipNoise_ar);
    FLEXT_SETUP(ClipNoise_kr);

    FLEXT_DSP_SETUP(GrayNoise_ar);
    FLEXT_SETUP(GrayNoise_kr);

    FLEXT_DSP_SETUP(WhiteNoise_ar);
    FLEXT_SETUP(WhiteNoise_kr);

    FLEXT_DSP_SETUP(PinkNoise_ar);
    FLEXT_SETUP(PinkNoise_kr);

    FLEXT_DSP_SETUP(Dust2_ar);
    FLEXT_SETUP(Dust2_kr);

    FLEXT_DSP_SETUP(Crackle_ar);
    FLEXT_SETUP(Crackle_kr);

    FLEXT_DSP_SETUP(Rand_ar);
    FLEXT_SETUP(Rand_kr);

    FLEXT_DSP_SETUP(TRand_ar);
    FLEXT_SETUP(TRand_kr);

    FLEXT_DSP_SETUP(TExpRand_ar);
    FLEXT_SETUP(TExpRand_kr);

    FLEXT_DSP_SETUP(IRand_ar);
    FLEXT_SETUP(IRand_kr);

    FLEXT_DSP_SETUP(TIRand_ar);
    FLEXT_SETUP(TIRand_kr);

    FLEXT_SETUP(CoinGate_kr);

    FLEXT_DSP_SETUP(LinRand_ar);
    FLEXT_SETUP(LinRand_kr);

    FLEXT_DSP_SETUP(NRand_ar);
    FLEXT_SETUP(NRand_kr);

    FLEXT_DSP_SETUP(ExpRand_ar);
    FLEXT_SETUP(ExpRand_kr);

    FLEXT_DSP_SETUP(LFClipNoise_ar);
    FLEXT_SETUP(LFClipNoise_kr);

    FLEXT_DSP_SETUP(LFNoise0_ar);
    FLEXT_SETUP(LFNoise0_kr);

    FLEXT_DSP_SETUP(LFNoise1_ar);
    FLEXT_SETUP(LFNoise1_kr);

    FLEXT_DSP_SETUP(LFNoise2_ar);
    FLEXT_SETUP(LFNoise2_kr);

    FLEXT_DSP_SETUP(Logistic_ar);
    FLEXT_SETUP(Logistic_kr);

    FLEXT_DSP_SETUP(Latoocarfian_ar);
    FLEXT_SETUP(Latoocarfian_kr);

    FLEXT_DSP_SETUP(LinCong_ar);
    FLEXT_SETUP(LinCong_kr);

    FLEXT_DSP_SETUP(amclip_ar);
    FLEXT_SETUP(amclip_kr);

    FLEXT_DSP_SETUP(scaleneg_ar);
    FLEXT_SETUP(scaleneg_kr);

    FLEXT_DSP_SETUP(excess_ar);
    FLEXT_SETUP(excess_kr);

    FLEXT_DSP_SETUP(hypot_ar);
    FLEXT_SETUP(hypot_kr);

    FLEXT_DSP_SETUP(ring1_ar);
    FLEXT_SETUP(ring1_kr);

    FLEXT_DSP_SETUP(ring2_ar);
    FLEXT_SETUP(ring2_kr);

    FLEXT_DSP_SETUP(ring3_ar);
    FLEXT_SETUP(ring3_kr);

    FLEXT_DSP_SETUP(ring4_ar);
    FLEXT_SETUP(ring4_kr);

    FLEXT_DSP_SETUP(difsqr_ar);
    FLEXT_SETUP(difsqr_kr);

    FLEXT_DSP_SETUP(sumsqr_ar);
    FLEXT_SETUP(sumsqr_kr);

    FLEXT_DSP_SETUP(sqrsum_ar);
    FLEXT_SETUP(sqrsum_kr);

    FLEXT_DSP_SETUP(sqrdif_ar);
    FLEXT_SETUP(sqrdif_kr);

    FLEXT_DSP_SETUP(absdif_ar);
    FLEXT_SETUP(absdif_kr);

    FLEXT_DSP_SETUP(LFSaw_ar);
    FLEXT_SETUP(LFSaw_kr);

    FLEXT_DSP_SETUP(LFPulse_ar);
    FLEXT_SETUP(LFPulse_kr);
}

FLEXT_LIB_SETUP(sc4pd,sc4pd_library_setup);
