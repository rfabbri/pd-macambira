/*

FFTease - A set of Live Spectral Processors
Originally written by Eric Lyon and Christopher Penrose for the Max/MSP platform

Copyright (c)Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"


fftease::fftease(I mult,BL stereo,BL window,BL bitshuf):
	_mult(mult),_stereo(stereo),_window(window),_bitshuf(bitshuf),
	_inCount(0),
	blsz(0),smprt(0)
{}

fftease::~fftease() {}

BL fftease::Init()
{
	BL ret = flext_dsp::Init();
	Clear();
	return ret;
}

V fftease::Exit()
{
	Delete();
}

V fftease::m_dsp(I n,S *const *in,S *const *out)
{
	const I _D = n;
	const F sr = Samplerate();
	if(_D != blsz || sr != smprt) {
		blsz = _D;
		smprt = sr;

		Delete();
		Set();
	}
}

V fftease::m_signal(I n,S *const *in,S *const *out)
{
	/* declare working variables */
	I i; 
	const I _D = n,_N = _D*Mult(),_Nw = _N,_N2 = _N/2,_Nw2 = _Nw/2; 

	/* fill our retaining buffers */
	_inCount += _D;

	if(_stereo) {
		for(i = 0; i < _N-_D ; i++ ) {
			_input1[i] = _input1[i+_D];
			_input2[i] = _input2[i+_D];
		}
		for(I j = 0; i < _N; i++,j++) {
			_input1[i] = in[0][j];
			_input2[i] = in[1][j];
		}
	}
	else {
		for (i = 0 ; i < _N-_D ; i++ )
			_input1[i] = _input1[i+_D];
		for (I j = 0; i < _N; i++,j++ )
			_input1[i] = in[0][j];
	}

	/* apply hamming window and fold our window buffer into the fft buffer */
	fold( _input1, _Wanal, _Nw, _buffer1, _N, _inCount );
	if(_stereo) fold( _input2, _Wanal, _Nw, _buffer2, _N, _inCount );

	/* do an fft */
	if(_bitshuf) {
		rdft( _N, 1, _buffer1, _bitshuffle, _trigland );
		if(_stereo) rdft( _N, 1, _buffer2, _bitshuffle, _trigland );
	}
	else {
		rfft( _buffer1, _N2, 1);
		if(_stereo) rfft( _buffer2, _N2,1);
	}


	// ---- BEGIN --------------------------------

	Transform(_N2,in+(_stereo?1:2));

	// ---- END --------------------------------


	/* do an inverse fft */
	if(_bitshuf)
		rdft( _N, -1, _buffer1, _bitshuffle, _trigland );
	else
		rfft( _buffer1, _N2, 0);

	/* dewindow our result */
	overlapadd( _buffer1, _N, _Wsyn, _output, _Nw, _inCount);

	/* set our output and adjust our retaining output buffer */
	const F mult = 1./_N;
	for ( i = 0; i < _D; i++ )
		out[0][i] = _output[i] * mult;

	for ( i = 0; i < _N-_D; i++ )
		_output[i] = _output[i+_D];
	for (; i < _N; i++ )
		_output[i] = 0.;
}


void fftease::Set()
{
	/* preset the objects data */
	const I _D = Blocksize(),_N = _D*Mult(),_Nw = _N,_N2 = _N/2,_Nw2 = _Nw/2; 

	_inCount = -_Nw;

	/* assign memory to the buffers */
	_input1 = new F[_Nw];
	_buffer1 = new F[_N];
	_channel1 = new F[_N+2];
	if(_stereo) {
		_input2 = new F[_Nw];
		_buffer2 = new F[_N];
		_channel2 = new F[_N+2];
	}
	_output = new F[_Nw];

	if(_bitshuf) {
		_bitshuffle = new I[_N*2];
		_trigland = new F[_N*2];
		init_rdft( _N, _bitshuffle, _trigland);
	}

	_Hwin = new F[_Nw];
	_Wanal = new F[_Nw];
	_Wsyn = new F[_Nw];
	if(_window)
		makewindows( _Hwin, _Wanal, _Wsyn, _Nw, _N, _D, 0);
	else
		makehanning( _Hwin, _Wanal, _Wsyn, _Nw, _N, _D, 0,0);
}

void fftease::Clear()
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

void fftease::Delete()
{
	if(_input1) delete[] _input1;
	if(_buffer1) delete[] _buffer1;
	if(_channel1) delete[] _channel1;
	if(_stereo) {
		if(_input2) delete[] _input2;
		if(_buffer2) delete[] _buffer2;
		if(_channel2) delete[] _channel2;
	}
	if(_output) delete[] _output;

	if(_bitshuf) {
		if(_bitshuffle) delete[] _bitshuffle;
		if(_trigland) delete[] _trigland;
	}

	if(_Hwin) delete[] _Hwin;
	if(_Wanal) delete[] _Wanal;
	if(_Wsyn) delete[] _Wsyn;
}



