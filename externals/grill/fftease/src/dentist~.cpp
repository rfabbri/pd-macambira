/*

FFTease - A set of Live Spectral Processors
Originally written by Eric Lyon and Christopher Penrose for the Max/MSP platform

Copyright (c)Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include <stdlib.h>

class dentist:
	public flext_dsp
{
	FLEXT_HEADER_S(dentist,flext_dsp,setup)
	
public:
	dentist(I argc,const t_atom *argv);
	~dentist();

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

	// -----------------------------

	I *_bin_selection;
	I _tooth_count;  
	F _knee;
	I _max_bin; // determined by _knee and fundamental frequency

	V reset_shuffle();

private:
	enum { _MULT_ = 4 };

	V Clear();
	V Delete();

	V ms_knee(F knee);
	V ms_teeth(I teeth) { _tooth_count = teeth;	reset_shuffle(); }

	
	static V setup(t_classid c);

	FLEXT_CALLBACK(reset_shuffle)
	FLEXT_ATTRGET_F(_knee)
	FLEXT_CALLSET_F(ms_knee)
	FLEXT_ATTRGET_I(_tooth_count)
	FLEXT_CALLSET_I(ms_teeth)
};

FLEXT_LIB_DSP_V("fftease, dentist~",dentist)


V dentist::setup(t_classid c)
{
	FLEXT_CADDBANG(c,0,reset_shuffle);

	FLEXT_CADDATTR_VAR(c,"knee",_knee,ms_knee);
	FLEXT_CADDATTR_VAR(c,"teeth",_tooth_count,ms_teeth);
}


dentist::dentist(I argc,const t_atom *argv):
	blsz(0),smprt(0),
	_knee(500),_tooth_count(10)
{
	/* parse and set object's options given */
	if(argc >= 1) {
		if(CanbeFloat(argv[0]))
			_knee = GetAFloat(argv[0]);
		else
			post("%s - Knee must be a float value - set to %0f",thisName(),_knee);
	}
	if(argc >= 2) {
		if(CanbeInt(argv[1]))
			_tooth_count = GetAInt(argv[1]);
		else
			post("%s - Teeth must be an integer value - set to %0i",thisName(),_tooth_count);
	}

	Clear();

	AddInSignal("Messages and input signal");
	AddOutSignal("Transformed signal");
}

dentist::~dentist()
{
	Delete();
}

V dentist::Clear()
{
	_input1 = _input2 = NULL;
	_buffer1 = _buffer2 = NULL;
	_channel1 = _channel2 = NULL;
	_output = NULL;

	_bitshuffle = NULL;
	_trigland = NULL;

	_Hwin = NULL;
	_Wanal = _Wsyn = NULL;

	// -----------------------------

	_bin_selection = NULL;
}

V dentist::Delete() 
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

	if(_bitshuffle) delete[] _bitshuffle;
	if(_trigland) delete[] _trigland;

	if(_Wanal) delete[] _Wanal;
	if(_Wsyn) delete[] _Wsyn;
	if(_Hwin) delete[] _Hwin;

	// -----------------------------

	if(_bin_selection) delete[] _bin_selection;
}


V dentist::ms_knee(F f)
{
	_knee = f;	// store original

	const F funda = Samplerate()/(2*_MULT_*Blocksize());

	// TG: This is a different, but steady correction than in original fftease
	if( f < funda ) f = funda;
	else if(f > funda/2) f = funda/2;

	_max_bin = 1;  
	for(F curfreq = 0; curfreq < f; curfreq += funda) ++_max_bin;
}


V dentist::m_dsp(I n,S *const *,S *const *)
{
	const I _D = n;
	const F _R = Samplerate();
	if(_D != blsz || _R != smprt) {
		blsz = _D;
		smprt = _R;

		Delete();
		// ---------------------------------------------

		const int _N = _D*_MULT_,_Nw = _N,_Nw2 = _Nw>>1,_N2 = _N>>1;

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

		_bitshuffle = new I[_N*2];
		_trigland = new F[_N*2];

		_Wanal = new F[_Nw];	
		_Wsyn = new F[_Nw];	
		_Hwin = new F[_Nw];

		init_rdft( _N, _bitshuffle, _trigland);
		makewindows( _Hwin, _Wanal, _Wsyn, _Nw, _N, _D, 0);

		// ---------------------------------------------

		// calculation of _max_bin
		ms_knee(_knee); 

		_bin_selection = new I[_N2];
		reset_shuffle();
	}
}

V dentist::m_signal(I n,S *const *in,S *const *out)
{
	/* declare working variables */
	I i, j; 
	const I _D = n,_N = _D*_MULT_,_Nw = _N,_N2 = _N/2,_Nw2 = _Nw/2; 

	_inCount += _D;

	for ( j = 0 ; j < _N-_D ; j++ )
		_input1[j] = _input1[j+_D];
	for (i = 0; j < _N; j++,i++ )
		_input1[j] = in[0][i];

	fold( _input1, _Wanal, _Nw, _buffer1, _N, _inCount );		
	rdft( _N, 1, _buffer1, _bitshuffle, _trigland );

	// ---- BEGIN --------------------------------

    leanconvert( _buffer1, _channel1, _N2 );

	for( i = 0; i < _N2 ; i++){
		if( !_bin_selection[i] ) _channel1[i*2] = 0;
	}

	leanunconvert( _channel1, _buffer1,  _N2 );

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


V dentist::reset_shuffle()
{
	const I _N2 = Blocksize()*_MULT_/2;
	I teeth = _tooth_count;

	// check number of teeth
	if( teeth < 0 ) teeth = 0;
	else if( teeth > _N2 ) teeth = _N2;

	// clear and set random bins
	I i;
	for( i = 0; i < _N2; i++ )
		_bin_selection[i] = 0;
	for( i = 0; i < _tooth_count; i++ )
		_bin_selection[rand()%_max_bin] = 1;
}
