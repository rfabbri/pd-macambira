/*

FFTease - A set of Live Spectral Processors
Originally written by Eric Lyon and Christopher Penrose for the Max/MSP platform

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

    F _threshold,_multiplier;
    F _thresh_dB,_mult_dB;

    F *_input1,*_input2;
    F *_buffer1,*_buffer2;
    F *_channel1,*_channel2;
    F *_output;
    F *_trigland;
    I *_bitshuffle;
    F *_Wanal,*_Wsyn,*_Hwin;

    I _inCount;

private:
	enum { _MULT_ = 4 };

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

	AddInSignal("Messages and input signal");
	AddInSignal("Reference signal");
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
	_input1 = _input2 = NULL;
	_Hwin = NULL;
	_Wanal = _Wsyn = NULL;
	_buffer1 = _buffer2 = NULL;
	_channel1 = _channel2 = NULL;
	_output = NULL;
}

V burrow::Delete() 
{
	if(_bitshuffle) delete[] _bitshuffle;
	if(_trigland) delete[] _trigland;
	if(_input1) delete[] _input1;
	if(_input2) delete[] _input2;
	if(_Hwin) delete[] _Hwin;
	if(_Wanal) delete[] _Wanal;
	if(_Wsyn) delete[] _Wsyn;
	if(_buffer1) delete[] _buffer1;
	if(_buffer2) delete[] _buffer2;
	if(_channel1) delete[] _channel1;
	if(_channel2) delete[] _channel2;
	if(_output) delete[] _output;
}



V burrow::m_dsp(I n,S *const *,S *const *)
{
	const I _D = n;
	if(_D != blsz) {
		blsz = _D;

		Delete();

		/* preset the objects data */
		const I _N = _D*_MULT_,_Nw = _N,_N2 = _N/2,_Nw2 = _Nw/2; 

		_inCount = -_Nw;

		/* assign memory to the buffers */
		_input1 = new F[_Nw];
		_input2 = new F[_Nw];
		_buffer1 = new F[_N];
		_buffer2 = new F[_N];
		_channel1 = new F[_N+2];
		_channel2 = new F[_N+2];
		_output = new F[_Nw];

		_bitshuffle = new I[_N*2];
		_trigland = new F[_N*2];

		_Hwin = new F[_Nw];
		_Wanal = new F[_Nw];
		_Wsyn = new F[_Nw];

		/* initialize pv-lib functions */
		init_rdft( _N, _bitshuffle, _trigland);
		makewindows( _Hwin, _Wanal, _Wsyn, _Nw, _N, _D, 0);
	}
}

V burrow::m_signal(I n,S *const *in,S *const *out)
{
	/* declare working variables */
	I i, j; 
	const I _D = n,_N = _D*_MULT_,_Nw = _N,_N2 = _N/2,_Nw2 = _Nw/2; 

	/* fill our retaining buffers */
	_inCount += _D;

	for(i = 0; i < _N-_D ; i++ ) {
		_input1[i] = _input1[i+_D];
		_input2[i] = _input2[i+_D];
	}
	for(j = 0; i < _N; i++,j++) {
		_input1[i] = in[0][j];
		_input2[i] = in[1][j];
	}

	/* apply hamming window and fold our window buffer into the fft buffer */
	fold( _input1, _Wanal, _Nw, _buffer1, _N, _inCount );
	fold( _input2, _Wanal, _Nw, _buffer2, _N, _inCount );

	/* do an fft */
	rdft( _N, 1, _buffer1, _bitshuffle, _trigland );
	rdft( _N, 1, _buffer2, _bitshuffle, _trigland );


	// ---- BEGIN --------------------------------

	for ( i = 0; i <= _N2; i++ ) {
		const I even = i<<1,odd = even+1;

		/* convert to polar coordinates from complex values */
		register F a,b;

		a = ( i == _N2 ? _buffer1[1] : _buffer1[even] );
		b = ( i == 0 || i == _N2 ? 0. : _buffer1[odd] );

		_channel1[even] = hypot( a, b );
		_channel1[odd] = -atan2( b, a );

		a = ( i == _N2 ? _buffer2[1] : _buffer2[even] );
		b = ( i == 0 || i == _N2 ? 0. : _buffer2[odd] );

		_channel2[even] = hypot( a, b );

		/* use simple threshold from second signal to trigger filtering */
		if (_invert?(_channel2[even] < _threshold):(_channel2[even] > _threshold) )
			_channel1[even] *= _multiplier;

		/* convert back to complex form, read for the inverse fft */
		_buffer1[even] = _channel1[even] * cos( _channel1[odd] );

		if ( i != _N2 )
			_buffer1[odd] = -_channel1[even] * sin( _channel1[odd] );
	}


	// ---- END --------------------------------


	/* do an inverse fft */
	rdft( _N, -1, _buffer1, _bitshuffle, _trigland );

	/* dewindow our result */
	overlapadd( _buffer1, _N, _Wsyn, _output, _Nw, _inCount);

	/* set our output and adjust our retaining output buffer */
	const F mult = 1./_N;
	for ( j = 0; j < _D; j++ )
		out[0][j] = _output[j] * mult;

	for ( j = 0; j < _N-_D; j++ )
		_output[j] = _output[j+_D];
	for (; j < _N; j++ )
		_output[j] = 0.;
}



