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
	public fftease
{
	FLEXT_HEADER_S(dentist,fftease,setup)
	
public:
	dentist(I argc,const t_atom *argv);

protected:

	virtual V Transform(I _N2,S *const *in);

	I *_bin_selection;
	I _tooth_count;  
	F _knee;
	I _max_bin; // determined by _knee and fundamental frequency

	V reset_shuffle();

private:

	virtual V Set();
	virtual V Clear();
	virtual V Delete();

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
	fftease(4,false,true,true),	
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

	AddInSignal("Messages and input signal");
	AddOutSignal("Transformed signal");
}

V dentist::Clear()
{
	fftease::Clear();

	_bin_selection = NULL;
}

V dentist::Delete() 
{
	fftease::Delete();

	if(_bin_selection) delete[] _bin_selection;
}


V dentist::ms_knee(F f)
{
	_knee = f;	// store original

	const F funda = Samplerate()/(2*Mult()*Blocksize());

	// TG: This is a different, but steady correction than in original fftease
	if( f < funda ) f = funda;
	else if(f > funda/2) f = funda/2;

	_max_bin = 1;  
	for(F curfreq = 0; curfreq < f; curfreq += funda) ++_max_bin;
}


V dentist::Set()
{
	fftease::Set();
	
	// calculation of _max_bin
	ms_knee(_knee); 

	_bin_selection = new I[(Blocksize()*Mult())>>1];
	reset_shuffle();
}

V dentist::Transform(I _N2,S *const *in)
{
    leanconvert( _buffer1, _channel1, _N2 );

	for(I i = 0; i < _N2 ; i++){
		if( !_bin_selection[i] ) _channel1[i*2] = 0;
	}

	leanunconvert( _channel1, _buffer1,  _N2 );
}


V dentist::reset_shuffle()
{
	const I _N2 = Blocksize()*Mult()/2;
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
