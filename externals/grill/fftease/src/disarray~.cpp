/*

FFTease - A set of Live Spectral Processors
Originally written by Eric Lyon and Christopher Penrose for the Max/MSP platform

Copyright (c)Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include <stdlib.h>

class disarray:
	public flext_dsp
{
	FLEXT_HEADER_S(disarray,flext_dsp,setup)
	
public:
	disarray(I argc,const t_atom *argv);
	~disarray();

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

    I _inCount,_nmult;

	// -----------------------------

	BL _qual;
	I _shuffle_count,_max_bin;
	F _freq;
	I *_shuffle_in,*_shuffle_out;


	V reset_shuffle();

private:
	V Clear();
	V Delete();

	V ms_freq(F f);

	static V setup(t_classid c);

	FLEXT_CALLBACK(reset_shuffle)
	FLEXT_ATTRGET_F(_freq)
	FLEXT_CALLSET_F(ms_freq)
};

FLEXT_LIB_DSP_V("fftease, disarray~",disarray)


V disarray::setup(t_classid c)
{
	FLEXT_CADDBANG(c,0,reset_shuffle);

	FLEXT_CADDATTR_VAR(c,"freq",_freq,ms_freq);
}


disarray::disarray(I argc,const t_atom *argv):
	blsz(0),smprt(0),
	_freq(1300),_qual(false),_shuffle_count(20)
{
	/* parse and set object's options given */
	if(argc >= 1) {
		if(CanbeFloat(argv[0]))
			_freq = GetAFloat(argv[0]);
		else
			post("%s - Freq must be a float value - set to %0f",thisName(),_freq);
	}
	if(argc >= 2) {
		if(CanbeBool(argv[1]))
			_qual = GetABool(argv[1]);
		else
			post("%s - Quality must be a boolean value - set to %0i",thisName(),_qual?1:0);
	}
	if(argc >= 3) {
		if(CanbeInt(argv[2]))
			_shuffle_count = GetAInt(argv[2]);
		else
			post("%s - Shufflecount must be an integer value - set to %0i",thisName(),_shuffle_count);
	}

	_nmult = _qual?4:2;

	Clear();

	AddInSignal("Messages and input signal");
	AddOutSignal("Transformed signal");
}

disarray::~disarray()
{
	Delete();
}

V disarray::Clear()
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

	_shuffle_in = _shuffle_out = NULL;
}

V disarray::Delete() 
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

	if(_shuffle_in) delete[] _shuffle_in;
	if(_shuffle_out) delete[] _shuffle_out;
}


V disarray::ms_freq(F f)
{
	_freq = f; // store original

	const F funda = Samplerate()/(2*_nmult*Blocksize());

	// TG: This is a different, but steady correction than in original fftease
	if( f < funda ) f = funda;
	else if(f > funda/2) f = funda/2;

	_max_bin = 1;  
	for(F curfreq = 0; curfreq < f; curfreq += funda) ++_max_bin;
}


V disarray::m_dsp(I n,S *const *in,S *const *out)
{
	const I _D = Blocksize();
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

		_bitshuffle = new I[_N*2];
		_trigland = new F[_N*2];

		_Wanal = new F[_Nw];	
		_Wsyn = new F[_Nw];	
		_Hwin = new F[_Nw];

		init_rdft( _N, _bitshuffle, _trigland);

		if(_qual) 
			makewindows( _Hwin, _Wanal, _Wsyn, _Nw, _N, _D, 0);
		else
			makehanning( _Hwin, _Wanal, _Wsyn, _Nw, _N, _D, 0,0);

		// ---------------------------------------------

		_shuffle_in = new I[_N2];
		_shuffle_out = new I[_N2];

		// calculate _max_bin
		ms_freq(_freq);

		reset_shuffle();
	}
}

V disarray::m_signal(I n,S *const *in,S *const *out)
{
	/* declare working variables */
	I i, j; 
	const I _D = blsz,_N = _D*_nmult,_Nw = _N,_N2 = _N/2,_Nw2 = _Nw/2; 

	_inCount += _D;

	for ( j = 0 ; j < _N-_D ; j++ )
		_input1[j] = _input1[j+_D];
	for (i = 0; j < _N; j++,i++ )
		_input1[j] = in[0][i];

	fold( _input1, _Wanal, _Nw, _buffer1, _N, _inCount );		
	rdft( _N, 1, _buffer1, _bitshuffle, _trigland );

	// ---- BEGIN --------------------------------

    leanconvert( _buffer1, _channel1, _N2 );

	for( i = 0; i < _shuffle_count ; i++){
		F tmp = _channel1[ _shuffle_in[ i ] * 2 ];
		_channel1[ _shuffle_in[ i ] * 2]  = _channel1[ _shuffle_out[ i ] * 2];
		_channel1[ _shuffle_out[ i ] * 2]  = tmp;
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


V disarray::reset_shuffle()
{
	const I _N2 = Blocksize()*_nmult/2;

	I i;
	for( i = 0; i < _N2; i++ )
		_shuffle_out[i] = _shuffle_in[i] = i ;

	for( i = 0; i < 10000; i++ ) {
		int p1 = _shuffle_out[ rand()%_max_bin ];
		int p2 = _shuffle_out[ rand()%_max_bin ];
		int temp = _shuffle_out[ p1 ];
		_shuffle_out[ p1 ] = _shuffle_out[ p2 ];
		_shuffle_out[ p2 ] = temp;
	}

}

