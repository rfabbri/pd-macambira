/*

FFTease - A set of Live Spectral Processors
Originally written by Eric Lyon and Christopher Penrose for the Max/MSP platform

Copyright (c)Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include <stdlib.h>

class ether:
	public fftease
{
	FLEXT_HEADER_S(ether,fftease,setup)
	
public:
	ether(I argc,const t_atom *argv);

protected:

	virtual V Transform(I n,S *const *in);

	BL _qual,_invert;
	F _threshMult;

private:
	static V setup(t_classid c);


	FLEXT_ATTRVAR_B(_invert)
	FLEXT_ATTRVAR_F(_threshMult)
};

FLEXT_LIB_DSP_V("fftease, ether~",ether)


V ether::setup(t_classid c)
{
	FLEXT_CADDATTR_VAR1(c,"invert",_invert);
	FLEXT_CADDATTR_VAR1(c,"index",_threshMult);
}


ether::ether(I argc,const t_atom *argv):
	fftease(2,true,true,true),
	_qual(false),_threshMult(0),_invert(false)
{
	/* parse and set object's options given */
	if(argc >= 1) {
		if(CanbeBool(argv[0]))
			_qual = GetABool(argv[0]);
		else
			post("%s - Quality must be a boolean value - set to %0i",thisName(),_qual?1:0);
	}

	_mult = _qual?4:2;
	_window = _qual;

	AddInSignal("Messages and input signal");
	AddInSignal("Reference signal");
	AddOutSignal("Transformed signal");
}


V ether::Transform(I _N2,S *const *in)
{
	F threshMult = _threshMult;
	if (threshMult == 0. )  threshMult = 1;

	for (I i = 0; i <= _N2; i++ ) {
		int even = i<<1,odd = even + 1;

		/* convert to polar coordinates from complex values */
		register F a,b;

		a = ( i == _N2 ? _buffer1[1] : _buffer1[even] );
		b = ( i == 0 || i == _N2 ? 0. : _buffer1[odd] );

		_channel1[even] = hypot( a, b );
		_channel1[odd] = -atan2( b, a );

		a = ( i == _N2 ? _buffer2[1] : _buffer2[even] );
		b = ( i == 0 || i == _N2 ? 0. : _buffer2[odd] );

		_channel2[even] = hypot( a, b );
		_channel2[odd] = -atan2( b, a );

		/* use simple threshold for inverse compositing */

		if(_invert?(_channel1[even] > _channel2[even]*threshMult):(_channel1[even] < _channel2[even]*threshMult) )
			_channel1[even] = _channel2[even];

		if (_channel1[odd] == 0. ) _channel1[odd] = _channel2[odd];


		/* convert back to complex form, read for the inverse fft */
		_buffer1[even] = _channel1[even] * cos( _channel1[odd] );

		if (i != _N2 )
			_buffer1[odd] = -_channel1[even] * sin( _channel1[odd] );
	}
}

