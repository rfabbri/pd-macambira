/* sc4pd: 
   support functions
   
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
//#include <flsupport.h>
#include "SC_PlugIn.h"


//#include <strings.h>
#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 406)
#error You need at least FLEXT version 0.4.6
#endif

/* for argument parsing */
bool sc_add (flext::AtomList a);
float sc_getfloatarg (flext::AtomList a,int i);
bool sc_ar(flext::AtomList a);


/* for rngs */

// macros to put rgen state in registers
#define RGET \
	uint32 s1 = rgen.s1; \
	uint32 s2 = rgen.s2; \
	uint32 s3 = rgen.s3; 

#define RPUT \
	rgen.s1 = s1; \
	rgen.s2 = s2; \
	rgen.s3 = s3;

int32 timeseed();


/* this is copied from thomas grill's xsample:
xsample - extended sample objects for Max/MSP and pd (pure data)

Copyright (c) 2001-2004 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  
*/

#define F float
#define D double
#define I int
#define L long
#define C char
#define V void
#define BL bool
#define S t_sample

#define SETSIGFUN(VAR,FUN) v_##VAR = FUN

#define DEFSIGFUN(NAME)	V NAME(I n,S *const *in,S *const *out)

#define DEFSIGCALL(NAME) \
	inline V NAME(I n,S *const *in,S *const *out) \
        { (this->*v_##NAME)(n,in,out); } \
	V (thisType::*v_##NAME)(I n,S *const *invecs,S *const *outvecs)

#define SIGFUN(FUN) &thisType::FUN


/* this macro has to be redefined to work with flext */

// calculate a slope for control rate interpolation to audio rate.
//#define CALCSLOPE(next,prev) ((next - prev) * unit->mRate->mSlopeFactor)
#undef CALCSLOPE
#define CALCSLOPE(next,prev) ((next - prev) * 1/Blocksize())
