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

FLEXT_LIB_DSP_V("fftease, drown~",drown)


drown::drown(I argc,const t_atom *argv):
	fftease(4,F_WINDOW)
{
	AddInSignal("Messages and input signal");
	AddInSignal("Threshold generator signal");
	AddInSignal("Multiplier signal for weak bins");
	AddOutSignal("Transformed signal");
}


V drown::Transform(I _N2,S *const *in)
{
	// only first value of the signal vectors
	const F thresh = in[0][0],mult = in[1][0];

	// nudist helper function is integrated

	for (I i = 0; i <= _N2; i++ ) {
		const I real = i*2,imag = real+1;
		F amp,phase;

		// convert to amp and phase
		const F a = ( i == _N2 ? _buffer1[1] : _buffer1[real] );
		const F b = ( i == 0 || i == _N2 ? 0. : _buffer1[imag] );

		amp = hypot( a, b );
		// make up low amplitude bins
		if(amp < thresh) amp *= mult;

		phase = -atan2( b, a );


		// convert back to real and imag
		_buffer1[real] = amp * cos( phase );
		if (i != _N2)	_buffer1[imag] = -amp * sin( phase );
	}
}


