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
	fftease(2,F_STEREO|F_BITSHUFFLE),
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
	if(_qual) _flags |= F_WINDOW;

	AddInSignal("Messages and input signal");
	AddInSignal("Reference signal");
	AddOutSignal("Transformed signal");
}


V ether::Transform(I _N2,S *const *in)
{
	F threshMult = _threshMult;
	if (threshMult == 0. )  threshMult = 1;

	for (I i = 0; i <= _N2; i++ ) {
		const I even = i*2,odd = even + 1;

		// convert to polar coordinates from complex values 
		register F a,b;

		a = ( i == _N2 ? _buffer1[1] : _buffer1[even] );
		b = ( i == 0 || i == _N2 ? 0. : _buffer1[odd] );

		F amp1 = hypot( a, b );
		F phase1 = -atan2( b, a );

		a = ( i == _N2 ? _buffer2[1] : _buffer2[even] );
		b = ( i == 0 || i == _N2 ? 0. : _buffer2[odd] );

		F amp2 = hypot( a, b );
		F phase2 = -atan2( b, a );

		// use simple threshold for inverse compositing 

		if(_invert?(amp1 > amp2*threshMult):(amp1 < amp2*threshMult) ) amp1 = amp2;
		if (phase1 == 0. ) phase1 = phase2;


		// convert back to complex form, read for the inverse fft 
		_buffer1[even] = amp1 * cos(phase1);
		if(i != _N2) _buffer1[odd] = -amp1 * sin(phase1);
	}
}

