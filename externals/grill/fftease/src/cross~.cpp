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
	cross(I argc,const t_atom *argv);

protected:

	virtual V Transform(I _N2,S *const *in);
};

FLEXT_LIB_DSP_V("fftease, cross~",cross)


cross::cross(I argc,const t_atom *argv):
	fftease(2,true,true,true)
{
	AddInSignal("Messages and driver signal");
	AddInSignal("Filter signal");
	AddInSignal("Threshold signal for cross synthesis");
	AddOutSignal("Transformed signal");
}

V cross::Transform(I _N2,S *const *in)
{
	// TG: filled only once per signal vector!!
	float threshie = *in[0];
  
	for (I i = 0; i <= _N2; i++ ) {
		const I even = i<<1,odd = even+1;

		F a = ( i == _N2 ? _buffer1[1] : _buffer2[even] );
		F b = ( i == 0 || i == _N2 ? 0. : _buffer2[odd] );
		F gainer = hypot( a, b ) ;

		a = ( i == _N2 ? _buffer1[1] : _buffer1[even] );
		b = ( i == 0 || i == _N2 ? 0. : _buffer1[odd] );

		if( gainer > threshie ) 
			_channel1[even] = hypot( a, b ) * gainer;
		// else
		//	channel1[even] = hypot( a, b); 

		_channel1[odd] = -atan2( b, a );
		_buffer1[even] = _channel1[even] * cos( _channel1[odd] );

		if ( i != _N2 )
			_buffer1[odd] = -_channel1[even] * sin( _channel1[odd] );
	}
}



