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
	fftease(4,false,true,false)
{
	AddInSignal("Messages and input signal");
	AddInSignal("Threshold generator signal");
	AddInSignal("Multiplier signal for weak bins");
	AddOutSignal("Transformed signal");
}


/* helper function */
static void nudist( float *_S, float *_C, float threshold, float fmult, int N2 )
{
  int real, imag, amp, phase;
  int i;
  float maxamp = 1.;
  for ( i = 0; i <= N2; i++ ) {
    imag = phase = ( real = amp = i<<1 ) + 1;
    F a = ( i == N2 ? _S[1] : _S[real] );
    F b = ( i == 0 || i == N2 ? 0. : _S[imag] );

    _C[amp] = hypot( a, b );
    if( _C[amp] < threshold) _C[amp] *= fmult;

    _C[phase] = -atan2( b, a );
  }

  for ( i = 0; i <= N2; i++ ) {
    imag = phase = ( real = amp = i<<1 ) + 1;
    _S[real] = _C[amp] * cos( _C[phase] );
    if ( i != N2 )
      _S[imag] = -_C[amp] * sin( _C[phase] );
  }
}


V drown::Transform(I _N2,S *const *in)
{
	// only first value of the signal vectors
	const F thresh = in[1][0],mult = in[2][0];

	nudist( _buffer1, _channel1, thresh, mult, _N2 );
}


