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
	public flext_dsp
{
	FLEXT_HEADER_S(drown,flext_dsp,setup)
	
public:
	drown(I argc,const t_atom *argv);
	~drown();

protected:

	virtual V m_dsp(I n,S *const *in,S *const *out);
	virtual V m_signal(I n,S *const *in,S *const *out);

	I blsz;
	F smprt;

    F *_input1,*_input2;
    F *_buffer1,*_buffer2;
    F *_channel1,*_channel2;
    F *_output;
//    F *_trigland;
 //   I *_bitshuffle;
    F *_Wanal,*_Wsyn,*_Hwin;

    I _inCount,_nmult;

	// -----------------------------

private:
	V Clear();
	V Delete();

	static V setup(t_classid c);
};

FLEXT_LIB_DSP_V("fftease, drown~",drown)


V drown::setup(t_classid c)
{
}


drown::drown(I argc,const t_atom *argv):
	blsz(0),smprt(0),_nmult(4)
{
	Clear();

	AddInSignal("Messages and input signal");
	AddInSignal("Threshold generator signal");
	AddInSignal("Multiplier signal for weak bins");
	AddOutSignal("Transformed signal");
}

drown::~drown()
{
	Delete();
}

V drown::Clear()
{
	_input1 = _input2 = NULL;
	_buffer1 = _buffer2 = NULL;
	_channel1 = _channel2 = NULL;
	_output = NULL;
/*
	_bitshuffle = NULL;
	_trigland = NULL;
*/
	_Hwin = NULL;
	_Wanal = _Wsyn = NULL;

	// -----------------------------

}

V drown::Delete() 
{
	if(_input1) delete[] _input1;
	if(_buffer1) delete[] _buffer1;
	if(_channel1) delete[] _channel1;
/*
	if(_input2) delete[] _input2;
	if(_buffer2) delete[] _buffer2;
	if(_channel2) delete[] _channel2;
*/
	if(_output) delete[] _output;
/*
	if(_bitshuffle) delete[] _bitshuffle;
	if(_trigland) delete[] _trigland;
*/
	if(_Wanal) delete[] _Wanal;
	if(_Wsyn) delete[] _Wsyn;
	if(_Hwin) delete[] _Hwin;

	// -----------------------------

}


V drown::m_dsp(I n,S *const *,S *const *)
{
	const I _D = n;
	const F _R = Samplerate();

	if(_D != blsz || _R != smprt) {
		blsz = _D;
		smprt = _R;

		Delete();
		// ---------------------------------------------

		const int _N = _D*_nmult,_Nw = _N,_Nw2 = _Nw>>1,_N2 = _N>>1;

		_inCount = -_Nw;

		_input1 = new F[_Nw];	
		_buffer1 = new F[_N];
		_channel1 = new F[_N+2];
		/*
		_input2 = new F[_Nw];	
		_buffer2 = new F[_N];
		_channel2 = new F[_N+2];
		*/

		_output = new F[_Nw];

		/*
		_bitshuffle = new I[_N*2];
		_trigland = new F[_N*2];
		init_rdft( _N, _bitshuffle, _trigland);
		*/

		_Wanal = new F[_Nw];	
		_Wsyn = new F[_Nw];	
		_Hwin = new F[_Nw];
		makewindows( _Hwin, _Wanal, _Wsyn, _Nw, _N, _D, 0);
	}
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


V drown::m_signal(I n,S *const *in,S *const *out)
{
	/* declare working variables */
	I i, j; 
	const I _D = n,_N = _D*_nmult,_Nw = _N,_N2 = _N/2,_Nw2 = _Nw/2; 

	_inCount += _D;

	for ( j = 0 ; j < _N-_D ; j++ )
		_input1[j] = _input1[j+_D];
	for (i = 0; j < _N; j++,i++ )
		_input1[j] = in[0][i];

	fold( _input1, _Wanal, _Nw, _buffer1, _N, _inCount );		

//	rdft( _N, 1, _buffer1, _bitshuffle, _trigland );
	rfft( _buffer1, _N2, 1);

	// ---- BEGIN --------------------------------

	{
		// only first value of the signal vectors
		const F thresh = in[1][0],mult = in[2][0];

		nudist( _buffer1, _channel1, thresh, mult, _N2 );
	}

	// ---- END --------------------------------

	rfft( _buffer1, _N2, 0);
//	rdft( _N, -1, _buffer1, _bitshuffle, _trigland );

	overlapadd( _buffer1, _N, _Wsyn, _output, _Nw, _inCount);

	const F mult = 1./_N;
	for ( j = 0; j < _D; j++ )
		out[0][j] = _output[j] * mult;

	for ( j = 0; j < _N-_D; j++ )
		_output[j] = _output[j+_D];	
	for (; j < _N; j++ )
		_output[j] = 0.;
}


