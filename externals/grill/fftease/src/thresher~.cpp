/*

FFTease - A set of Live Spectral Processors
Originally written by Eric Lyon and Christopher Penrose for the Max/MSP platform

Copyright (c)Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include <stdlib.h>

class thresher:
	public fftease
{
	FLEXT_HEADER_S(thresher,fftease,setup)
	
public:
	thresher();

protected:
	virtual V Set();
	virtual V Clear();
	virtual V Delete();

	virtual V Transform(I n,S *const *in);

	F *_compositeFrame;
	I *_framesLeft;
	F *_c_lastphase_in,*_c_lastphase_out;
	F _c_fundamental,_c_factor_in,_c_factor_out;
	BL _firstFrame;

	F _moveThreshold;
	I _maxHoldFrames;

private:
	static V setup(t_classid c);
};

FLEXT_LIB_DSP("fftease, thresher~",thresher)


V thresher::setup(t_classid c)
{
}

void thresher::Set()
{
	fftease::Set();

	const F _R = Samplerate();
	const I _D = Blocksize(),_N = get_N(),_N2 = _N/2; 

	_compositeFrame = new F[_N+2];
	_framesLeft = new I[_N2+1];
	_c_lastphase_in = new F[_N2+1];
	ZeroMem(_c_lastphase_in,(_N2+1)*sizeof(*_c_lastphase_in));
	_c_lastphase_out = new F[_N2+1];
	ZeroMem(_c_lastphase_out,(_N2+1)*sizeof(*_c_lastphase_out));

	_c_fundamental = _R/_N;
	_c_factor_in = _R/(_D * PV_2PI);
	_c_factor_out = 1./_c_factor_in;

	_firstFrame = true;
	_moveThreshold = .00001 ;
	F _maxHoldTime = 5.0 ;
	_maxHoldFrames = (I)(_maxHoldTime *_R/_D);
}

void thresher::Clear()
{
	fftease::Clear();
	_compositeFrame = NULL;
	_framesLeft = NULL;
	_c_lastphase_in = _c_lastphase_out = NULL;
}

void thresher::Delete()
{
	fftease::Delete();
	if(_compositeFrame) delete[] _compositeFrame;
	if(_framesLeft) delete[] _framesLeft;
	if(_c_lastphase_in) delete[] _c_lastphase_in;
	if(_c_lastphase_out) delete[] _c_lastphase_out;
}


thresher::thresher():
	fftease(4,F_BALANCED|F_BITSHUFFLE|F_NOSPEC|F_SPECRES)
{
	AddInSignal("Messages and input signal");
	AddOutSignal("Transformed signal");
}


V thresher::Transform(I _N,S *const *in)
{
	convert( _buffer1, _channel1, _N/2, _c_lastphase_in, _c_fundamental, _c_factor_in  );

	I *fr = _framesLeft;
	if( _firstFrame ) {
		for (I i = 0; i <= _N; i += 2,++fr ){
			_compositeFrame[i] = _channel1[i];
			_compositeFrame[i+1] = _channel1[i+1];
			*fr = _maxHoldFrames;
		}
		_firstFrame = false;
	}
	else {
		for(I i = 0; i <= _N; i += 2,++fr ){
			if( (fabs( _compositeFrame[i] - _channel1[i] ) > _moveThreshold) || (*fr <= 0) ) {
				_compositeFrame[i] = _channel1[i];
				_compositeFrame[i+1] = _channel1[i+1];
				*fr = _maxHoldFrames;
			}
			else
				--*fr;
		}
	}

	unconvert( _compositeFrame, _buffer1, _N/2, _c_lastphase_out, _c_fundamental, _c_factor_out  );
}
