/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __VASP_UTIL_H
#define __VASP_UTIL_H

#include "main.h"

#ifndef PI
#define PI 3.1415926535897932385
#endif

#define BIG 1.e10

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

template<class T>
inline V swap(T &a,T &b) { T c = a; a = b; b = c; }

template<class T>
inline T min(T a,T b) { return a < b?a:b; }

template<class T>
inline T max(T a,T b) { return a > b?a:b; }


template<class T>
T arg(T re,T im)
{
    if(re) 
	return (T)(fmod(atan(im/re)+(re < 0?2*PI:PI),2*PI)-PI);
    else
	if(im || re) return (T)(im > 0?PI/2:-PI/2);
        else return 0;
}

template<class T>
inline T sgn(T x) { return (T)(x?(x < 0?-1:1):0); }

template<class T>
inline T sqabs(T re,T im) { return re*re+im*im; }



#endif
