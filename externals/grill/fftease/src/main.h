/*

FFTease - A set of Live Spectral Processors
Originally written by Eric Lyon and Christopher Penrose for the Max/MSP platform

Copyright (c)Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __FFTEASE_H
#define __FFTEASE_H

#define FFTEASE_VERSION "0.0.1"


#define FLEXT_ATTRIBUTES 1 

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error You need at least flext version 0.4.1
#endif


#include "pv.h"

// lazy me
#define F float
#define D double
#define I int
#define L long
#define C char
#define V void
#define BL bool
#define S t_sample



class fftease:
	public flext_dsp
{
	FLEXT_HEADER_S(fftease,flext_dsp,setup)
	
public:
	fftease(I mult,BL stereo,BL window,BL bitshuf);
	virtual ~fftease();

protected:

	virtual BL Init();
	virtual V Exit();

	virtual V m_dsp(I n,S *const *in,S *const *out);
	virtual V m_signal(I n,S *const *in,S *const *out);

	virtual V Set();
	virtual V Clear();
	virtual V Delete();
	virtual V Transform(I _N2,S *const *in) = 0;

	I Mult() const { return _mult; }

    F *_input1,*_input2;
    F *_buffer1,*_buffer2;
    F *_channel1,*_channel2;
    F *_output;
    F *_trigland;
    I *_bitshuffle;
    F *_Wanal,*_Wsyn,*_Hwin;

    I _inCount;

	I _mult;
	BL _stereo,_window,_bitshuf;

private:
	I blsz;
	F smprt;

	static V setup(t_classid c) {}

};


#endif
