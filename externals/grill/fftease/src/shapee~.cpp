/*

FFTease - A set of Live Spectral Processors
Originally written by Eric Lyon and Christopher Penrose for the Max/MSP platform

Copyright (c)Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include <stdlib.h>

class shapee:
	public fftease
{
	FLEXT_HEADER(shapee,fftease)
	
public:
	shapee(I argc,const t_atom *argv);

protected:

	virtual V Transform(I n,S *const *in);

	BL _qual;
};

FLEXT_LIB_DSP_V("fftease, shapee~",shapee)


shapee::shapee(I argc,const t_atom *argv):
	fftease(2,F_STEREO|F_BITSHUFFLE|F_CONVERT),
	_qual(false)
{
	/* parse and set object's options given */
	if(argc >= 1) {
		if(CanbeBool(argv[0]))
			_qual = GetABool(argv[0]);
		else
			post("%s - Quality must be a boolean value - set to %0i",thisName(),_qual?1:0);
	}

	if(_qual)
		_mult = 4,_flags |= F_WINDOW;
	else
		_mult = 2;

	AddInSignal("Messages and frequency reference signal");
	AddInSignal("Amplitude reference signal");
	AddOutSignal("Transformed signal");
}


V shapee::Transform(I _N2,S *const *in)
{
	// lets just shape the entire signal in groups of three 

	const _N = _N2*2;
	I i;
	for ( i=2; i < _N+2; i += 6 ) {
		F lowerMult = _channel1[i-2] / _channel1[i];
		F upperMult = _channel1[i+2] / _channel1[i];
		F newCenter = ( _channel2[i-2]+_channel2[i]+_channel2[i+2] ) / (upperMult + lowerMult + 1);

		_channel2[i-2] = lowerMult * newCenter;
		_channel2[i+2] = upperMult * newCenter;
		_channel2[i] = newCenter; 
	}

	for ( i=0; i < _N; i+=2 ) {
		_channel1[i] = _channel2[i];
		if ( _channel1[i] == 0. ) 
			_channel1[i+1] = 0.;
		else if( _channel1[i+1] == 0. )
			_channel1[i+1] = _channel2[i+1];
	}
}
