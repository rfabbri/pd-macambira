#include "util.h"
#include <math.h>

R arg(R re,R im)
{
	if(re) 
		return fmod(atan(im/re)+(re < 0?2*PI:PI),2*PI)-PI;
	else
		if(im || re) return im > 0?PI/2:-PI/2;
		else return 0;
}

