/*

FFTease - A set of Live Spectral Processors
Originally written by Eric Lyon and Christopher Penrose for the Max/MSP platform

Copyright (c)Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include <stdlib.h>


// add quality switch

class swinger:
	public fftease
{
	FLEXT_HEADER(swinger,fftease)
	
public:
	swinger();

protected:

	virtual V Transform(I n,S *const *in);
};

FLEXT_LIB_DSP("fftease, swinger~",swinger)


swinger::swinger():
	fftease(2,F_STEREO|F_BITSHUFFLE)
{
	AddInSignal("Messages and input signal");
	AddInSignal("Signal to supply phase information");
	AddOutSignal("Transformed signal");
}


V swinger::Transform(I _N2,S *const *in)
{
	for (I i = 0; i <= _N2; i++ ) {
		const I even = i*2,odd = even+1;

		// convert to polar coordinates from complex values 
		// replace signal one's phases with those of signal two 
		const F a1 = ( i == _N2 ? _buffer1[1] : _buffer1[even] );
		const F b1 = ( i == 0 || i == _N2 ? 0. : _buffer1[odd] );
		const F amp = hypot( a1, b1 );

		const F a2 = ( i == _N2 ? _buffer2[1] : _buffer2[even] );
		const F b2 = ( i == 0 || i == _N2 ? 0. : _buffer2[odd] );
		const F ph = -atan2( b2, a2 );

		_buffer1[even] = amp * cos( ph );
		if ( i != _N2 )
			_buffer1[odd] = -amp * sin( ph );
	}
}
