/*

FFTease - A set of Live Spectral Processors
Originally written by Eric Lyon and Christopher Penrose for the Max/MSP platform

This flext port is based on the jMax port of Christian Klippel

Copyright (c)Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"


class burrow:
	public flext_dsp
{
	FLEXT_HEADER_S(burrow,flext_dsp,setup)
	
public:
	burrow(I argc,const t_atom *argv);
	~burrow();

protected:

	virtual V m_dsp(I n,S *const *in,S *const *out);
	virtual V m_signal(I n,S *const *in,S *const *out);

	I blsz;
    BL _invert;
    I _inCount;
    I *_bitshuffle;

    F _threshold,_multiplier;
    F _thresh_dB,_mult_dB;

    F *_Wanal;
    F *_Wsyn;
    F *_inputOne,*_inputTwo;
    F *_Hwin;
    F *_bufferOne,*_bufferTwo;
    F *_channelOne,*_channelTwo;
    F *_output;
    F *_trigland;

private:
	V Clear();
	V Delete();
	
	V ms_thresh(F v) { _threshold = (float) (pow( 10., ((_thresh_dB = v) * .05))); }
	V ms_mult(F v) { _multiplier = (float) (pow( 10., ((_mult_dB = v) * .05))); }


	static V setup(t_classid c);

	FLEXT_ATTRGET_F(_thresh_dB)
	FLEXT_CALLSET_F(ms_thresh)
	FLEXT_ATTRGET_F(_mult_dB)
	FLEXT_CALLSET_F(ms_mult)
	FLEXT_ATTRVAR_B(_invert)
};

FLEXT_LIB_DSP_V("fftease, burrow~",burrow)


V burrow::setup(t_classid c)
{
	FLEXT_CADDATTR_VAR(c,"thresh",_thresh_dB,ms_thresh);
	FLEXT_CADDATTR_VAR(c,"mult",_mult_dB,ms_mult);
	FLEXT_CADDATTR_VAR1(c,"invert",_invert);
}


burrow::burrow(I argc,const t_atom *argv):
	_thresh_dB(-30),_mult_dB(-18),
	_invert(false),
	blsz(0)
{
	/* parse and set object's options given */
	if(argc >= 1) {
		if(CanbeFloat(argv[0]))
			_thresh_dB = GetAFloat(argv[0]);
		else
			post("%s - Threshold must be a float value - set to %0f",thisName(),_thresh_dB);
	}
	if(argc >= 2) {
		if(CanbeFloat(argv[1]))
			_mult_dB = GetAFloat(argv[1]);
		else
			post("%s - Multiplier must be a float value - set to %0f",thisName(),_mult_dB);
	}
	if(argc >= 3) {
		if(CanbeBool(argv[2]))
			_invert = GetABool(argv[2]);
		else
			post("%s - Invert flag must be a boolean value - set to %i",thisName(),_invert?1:0);
	}

	ms_thresh(_thresh_dB);
	ms_mult(_mult_dB);

	Clear();

	AddInSignal("Commands and original signal");
	AddInSignal("Modulating signal");
	AddOutSignal("Transformed signal");
}

burrow::~burrow()
{
	Delete();
}

V burrow::Clear()
{
	_bitshuffle = NULL;
	_trigland = NULL;
	_inputOne = _inputTwo = NULL;
	_Hwin = NULL;
	_Wanal = _Wsyn = NULL;
	_bufferOne = _bufferTwo = NULL;
	_channelOne = _channelTwo = NULL;
	_output = NULL;
}

V burrow::Delete() 
{
	if(_bitshuffle) delete[] _bitshuffle;
	if(_trigland) delete[] _trigland;
	if(_inputOne) delete[] _inputOne;
	if(_inputTwo) delete[] _inputTwo;
	if(_Hwin) delete[] _Hwin;
	if(_Wanal) delete[] _Wanal;
	if(_Wsyn) delete[] _Wsyn;
	if(_bufferOne) delete[] _bufferOne;
	if(_bufferTwo) delete[] _bufferTwo;
	if(_channelOne) delete[] _channelOne;
	if(_channelTwo) delete[] _channelTwo;
	if(_output) delete[] _output;
}



V burrow::m_dsp(I n,S *const *in,S *const *out)
{
	const I _D = Blocksize();
	if(_D != blsz) {
		blsz = _D;

		Delete();

		/* preset the objects data */
		const I _N = _D*4,_Nw = _N,_N2 = _N/2,_Nw2 = _Nw/2; 

		_inCount = -_Nw;

		/* assign memory to the buffers */
		_bitshuffle = new I[_N*2];
		_trigland = new F[_N*2];
		_inputOne = new F[_Nw];
		_inputTwo = new F[_Nw];
		_Hwin = new F[_Nw];
		_Wanal = new F[_Nw];
		_Wsyn = new F[_Nw];
		_bufferOne = new F[_N];
		_bufferTwo = new F[_N];
		_channelOne = new F[_N+2];
		_channelTwo = new F[_N+2];
		_output = new F[_Nw];

		/* initialize pv-lib functions */
		init_rdft( _N, _bitshuffle, _trigland);
		makewindows( _Hwin, _Wanal, _Wsyn, _Nw, _N, _D, 0);
	}
}

V burrow::m_signal(I n,S *const *in,S *const *out)
{
	const S *inOne = in[0],*inTwo = in[1];
	S *outOne = out[0];

	/* declare working variables */
	I i, j; 
	const I _D = blsz,_N = _D*4,_Nw = _N,_N2 = _N/2,_Nw2 = _Nw/2; 

	/* fill our retaining buffers */
	_inCount += _D;

	for(i = 0; i < _N-_D ; i++ ) {
		_inputOne[i] = _inputOne[i+_D];
		_inputTwo[i] = _inputTwo[i+_D];
	}
	for(j = 0; i < _N; i++,j++) {
		_inputOne[i] = inOne[j];
		_inputTwo[i] = inTwo[j];
	}

	/* apply hamming window and fold our window buffer into the fft buffer */
	fold( _inputOne, _Wanal, _Nw, _bufferOne, _N, _inCount );
	fold( _inputTwo, _Wanal, _Nw, _bufferTwo, _N, _inCount );

	/* do an fft */
	rdft( _N, 1, _bufferOne, _bitshuffle, _trigland );
	rdft( _N, 1, _bufferTwo, _bitshuffle, _trigland );


	/* convert to polar coordinates from complex values */
	for ( i = 0; i <= _N2; i++ ) {
		const I even = i<<1,odd = even+1;
		register F a,b;

		a = ( i == _N2 ? _bufferOne[1] : _bufferOne[even] );
		b = ( i == 0 || i == _N2 ? 0. : _bufferOne[odd] );

		_channelOne[even] = hypot( a, b );
		_channelOne[odd] = -atan2( b, a );

		a = ( i == _N2 ? _bufferTwo[1] : _bufferTwo[even] );
		b = ( i == 0 || i == _N2 ? 0. : _bufferTwo[odd] );

		_channelTwo[even] = hypot( a, b );

		/* use simple threshold from second signal to trigger filtering */
		if (_invert?(_channelTwo[even] < _threshold):(_channelTwo[even] > _threshold) )
			_channelOne[even] *= _multiplier;
	}

	/* convert back to complex form, read for the inverse fft */
	for ( i = 0; i <= _N2; i++ ) {
		const I even = i<<1,odd = even+1;

		_bufferOne[even] = _channelOne[even] * cos( _channelOne[odd] );

		if ( i != _N2 )
			_bufferOne[odd] = -_channelOne[even] * sin( _channelOne[odd] );
	}

	/* do an inverse fft */
	rdft( _N, -1, _bufferOne, _bitshuffle, _trigland );

	/* dewindow our result */
	overlapadd( _bufferOne, _N, _Wsyn, _output, _Nw, _inCount);

	/* set our output and adjust our retaining output buffer */
	F mult = 1./_N;
	for ( j = 0; j < _D; j++ )
		outOne[j] = _output[j] * mult;

	for ( j = 0; j < _N-_D; j++ )
		_output[j] = _output[j+_D];
	for (; j < _N; j++ )
		_output[j] = 0.;
}



