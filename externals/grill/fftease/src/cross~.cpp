/*

FFTease - A set of Live Spectral Processors
Originally written by Eric Lyon and Christopher Penrose for the Max/MSP platform

Copyright (c)Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"


class cross:
	public flext_dsp
{
	FLEXT_HEADER_S(cross,flext_dsp,setup)
	
public:
	cross(I argc,const t_atom *argv);
	~cross();

protected:

	virtual V m_dsp(I n,S *const *in,S *const *out);
	virtual V m_signal(I n,S *const *in,S *const *out);

	I blsz;
	F smprt;

    F *_input1,*_input2;
    F *_buffer1,*_buffer2;
    F *_channel1,*_channel2;
    F *_output;
    F *_trigland;
    I *_bitshuffle;
    F *_Wanal,*_Wsyn,*_Hwin;

    I _inCount;

private:
	enum { _MULT_ = 2 };

	V Clear();
	V Delete();
	
	static V setup(t_classid c);
};

FLEXT_LIB_DSP_V("fftease, cross~",cross)


V cross::setup(t_classid c)
{
}


cross::cross(I argc,const t_atom *argv):
	blsz(0),smprt(0)
{
	Clear();

	AddInSignal("Messages and driver signal");
	AddInSignal("Filter signal");
	AddInSignal("Threshold signal for cross synthesis");
	AddOutSignal("Transformed signal");
}

cross::~cross()
{
	Delete();
}

V cross::Clear()
{
	_input1 = _input2 = NULL;
	_buffer1 = _buffer2 = NULL;
	_channel1 = _channel2 = NULL;
	_output = NULL;

	_bitshuffle = NULL;
	_trigland = NULL;

	_Hwin = NULL;
	_Wanal = _Wsyn = NULL;
}

V cross::Delete() 
{
	if(_input1) delete[] _input1;
	if(_input2) delete[] _input2;
	if(_buffer1) delete[] _buffer1;
	if(_buffer2) delete[] _buffer2;
	if(_channel1) delete[] _channel1;
	if(_channel2) delete[] _channel2;
	if(_output) delete[] _output;

	if(_bitshuffle) delete[] _bitshuffle;
	if(_trigland) delete[] _trigland;

	if(_Wanal) delete[] _Wanal;
	if(_Wsyn) delete[] _Wsyn;
	if(_Hwin) delete[] _Hwin;

	// ---------------------------------------------

#if 0
	// T.G. These are not used
	if(c_lastphase_in1) delete[] c_lastphase_in1;
	if(c_lastphase_in2) delete[] c_lastphase_in2;
	if(c_lastphase_out) delete[] c_lastphase_out;
#endif
}



V cross::m_dsp(I n,S *const *,S *const *)
{
	const I _D = n;
	const F _R = Samplerate();

	if(_D != blsz || _R != smprt) {
		blsz = _D;
		smprt = _R;

		Delete();

		const int _N = _D*_MULT_,_Nw = _N,_N2 = _N>>1,_Nw2 = _Nw>>1;

		_inCount = -_Nw;

		_input1 = new F[_Nw];	
		_input2 = new F[_Nw];	
		_buffer1 = new F[_N];
		_buffer2 = new F[_N];
		_channel1 = new F[_N+2];
		_channel2 = new F[_N+2];
		_output = new F[_Nw];

		_bitshuffle = new I[_N*2];
		_trigland = new F[_N*2];

		_Wanal = new F[_Nw];	
		_Wsyn = new F[_Nw];	
		_Hwin = new F[_Nw];

		init_rdft( _N, _bitshuffle, _trigland);

		// makewindows( x->Hwin, x->Wanal, x->Wsyn, x->Nw, x->N, x->D, 0);
		makehanning( _Hwin, _Wanal, _Wsyn, _Nw, _N, _D, 0,0);

		// ---------------------------------------------

#if 0
		// T.G. These are not used
		/* for convert */
		c_lastphase_in1 = new F[_N2+1];
		c_lastphase_in2 = new F[_N2+1];
		c_lastphase_out = new F[_N2+1];
		c_fundamental =  _R/( _N2<<1 );
		c_factor_in =  _R/(_D * __TWOPI);
		c_factor_out = __TWOPI * _D/_R;
#endif
	}
}

V cross::m_signal(I n,S *const *in,S *const *out)
{
	/* declare working variables */
	I i, j; 
	const I _D = n,_N = _D*_MULT_,_Nw = _N,_N2 = _N/2,_Nw2 = _Nw/2; 

	_inCount += _D;

	for ( j = 0 ; j < _N-_D ; j++ ) {
		_input1[j] = _input1[j+_D];
		_input2[j] = _input2[j+_D];
	}
	for (i = 0; j < _N; j++,i++ ) {
		_input1[j] = in[0][i];
		_input2[j] = in[1][i];
	}

	fold( _input1, _Wanal, _Nw, _buffer1, _N, _inCount );		
	fold( _input2, _Wanal, _Nw, _buffer2, _N, _inCount );	

	rdft( _N, 1, _buffer1, _bitshuffle, _trigland );
	rdft( _N, 1, _buffer2, _bitshuffle, _trigland );


	// ---- BEGIN --------------------------------

	// TG: filled only once per signal vector!!
	float threshie = *in[2];
  
	for ( i = 0; i <= _N2; i++ ) {
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

	// ---- END --------------------------------

	rdft( _N, -1, _buffer1, _bitshuffle, _trigland );

	overlapadd( _buffer1, _N, _Wsyn, _output, _Nw, _inCount);

	const F mult = 1./_N;
	for ( j = 0; j < _D; j++ )
		out[0][j] = _output[j] * mult;

	for ( j = 0; j < _N-_D; j++ )
		_output[j] = _output[j+_D];	
	for (; j < _N; j++ )
		_output[j] = 0.;
}



