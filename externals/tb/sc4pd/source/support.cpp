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
     
   Coded while listening to: Nmperign & Guenter Mueller: More Gloom, More Light
   
*/

#include <flext.h>
#include <flsupport.h>
#include "SC_PlugIn.h"


#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 406)
#error You need at least FLEXT version 0.4.6
#endif

bool sc_add (flext::AtomList a)
{
    for (int i = 0; i!=a.Count();++i)
    {
	if ( flext::IsSymbol(a[i]) )
	{
	    const char * teststring; 
	    teststring = flext::GetString(a[i]);
	    if((strcmp(teststring,"add"))==0)
		return true;
	}
    }
    return false;
}

float sc_getfloatarg (flext::AtomList a,int i)
{
    if (a.Count() > 0 && a.Count() > i)
	return flext::GetAFloat(a[i]);
    else 
	return 0;
}

bool sc_ar(flext::AtomList a)
{
    for (int i = 0; i!=a.Count();++i)
    {
	if ( flext::IsSymbol(a[i]) )
	{
	    const char * teststring; 
	    teststring = flext::GetString(a[i]);
	    if((strcmp(teststring,"ar"))==0)
		return true;
	}
    }
    return false;
}

// macros to put rgen state in registers
#define RGET \
	uint32 s1 = rgen.s1; \
	uint32 s2 = rgen.s2; \
	uint32 s3 = rgen.s3; 

#define RPUT \
	rgen.s1 = s1; \
	rgen.s2 = s2; \
	rgen.s3 = s3;

int32 timeseed()
{
	static int32 count = 0;

	double time = flext::GetOSTime();
	
	double sec = trunc(time);
	double usec = (time-sec)*1e6;
	
	time_t tsec = sec;
	useconds_t tusec =usec;       /* not exacty the way, it's calculated
					  in SuperCollider, but it's only 
					  the seed */
	
	return (int32)tsec ^ (int32)tusec ^ count--;
}
