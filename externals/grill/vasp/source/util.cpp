/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include "util.h"
#include <math.h>

/*
R arg(R re,R im)
{
	if(re) 
		return fmod(atan(im/re)+(re < 0?2*PI:PI),2*PI)-PI;
	else
		if(im || re) return im > 0?PI/2:-PI/2;
		else return 0;
}
*/
