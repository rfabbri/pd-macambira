/* 

idelay~ - interpolating delay using flext layer

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

-------------------------------------------------------------------------

This is an example for usage of flext
It's a simple interpolating delay with signal input with allows for glitchless change of delay times
Watch out for Doppler effects!

*/

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 400)
#error You need at least flext version 0.4.0 
#endif

// template class for delay line
#include "delay.h"


class idelay:
	public flext_dsp
{
	FLEXT_HEADER(idelay,flext_dsp)
 
public:
	idelay(F msec);
	~idelay();

	DelayLine<S> *dline;

protected:
	virtual V m_signal(I n,S *const *in,S *const *out); 
};

// make implementation of a tilde object with one float arg
FLEXT_NEW_DSP_1("idelay~",idelay,F)


idelay::idelay(F maxmsec)
{ 
    I nsamps = (I)ceil(maxmsec*Samplerate()*0.001f);
    if (nsamps < 1) nsamps = 1;
	dline = new DelayLine<F>(nsamps);

	AddInSignal(2);  // audio in & delay signals
	AddOutSignal();  // audio out
	SetupInOut();  // set up inlets and outlets
}

idelay::~idelay()
{
	if(dline) delete dline;
}


V idelay::m_signal(I n,S *const *in,S *const *out)
{
	const S *ins = in[0],*del = in[1];
	S *outs = out[0];
	F msr = Samplerate()*0.001f;

    while (n--)
    {
		dline->Put(*ins++);
    	*outs++ = dline->Get((*del++)*msr);
    }
}

