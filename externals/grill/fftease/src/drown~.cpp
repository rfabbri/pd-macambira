/*

FFTease - A set of Live Spectral Processors
Originally written by Eric Lyon and Christopher Penrose for the Max/MSP platform

Copyright (c)Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include <stdlib.h>

class drown:
	public fftease
{
	FLEXT_HEADER(drown,fftease)
	
public:
	drown(I argc,const t_atom *argv);

protected:
	virtual V Transform(I n,S *const *in);
};

FLEXT_LIB_DSP_V("fftease, drown~ denude~",drown)


drown::drown(I argc,const t_atom *argv):
	fftease(4,F_BALANCED|F_CONVERT)
{
	AddInSignal("Messages and input signal");
	AddInSignal("Threshold generator signal");
	AddInSignal("Multiplier signal for weak bins");
	AddOutSignal("Transformed signal");
}


V drown::Transform(I _N2,S *const *in)
{
	// only first value of the signal vectors
	const F thresh = *in[0],mult = *in[1];

	const I _N = _N2*2;

	// make up low amplitude bins
	for (I i = 0; i <= _N; i += 2)
		if(_channel1[i] < thresh) _channel1[i] *= mult;
}


