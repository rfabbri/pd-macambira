/*

FFTease - A set of Live Spectral Processors
Originally written by Eric Lyon and Christopher Penrose for the Max/MSP platform

Copyright (c)Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"


class cross:
	public fftease
{
	FLEXT_HEADER(cross,fftease)
	
public:
	cross();

protected:

	virtual V Transform(I _N2,S *const *in);
};

FLEXT_LIB_DSP("fftease, cross~",cross)


cross::cross():
	fftease(2,F_STEREO|F_BALANCED|F_BITSHUFFLE|F_CONVERT)
{
	AddInSignal("Messages and driver signal");
	AddInSignal("Filter signal");
	AddInSignal("Threshold signal for cross synthesis");
	AddOutSignal("Transformed signal");
}

V cross::Transform(I _N2,S *const *in)
{
	// filled only once per signal vector!!
	register const F threshie = *in[0];

	const I _N = _N2*2;
	for (I i = 0; i <= _N; i += 2) {
		// modulate amp2 with amp1 (if over threshold)
		if(_channel1[i] > threshie ) _channel2[i] *= _channel1[i];
	}
}
