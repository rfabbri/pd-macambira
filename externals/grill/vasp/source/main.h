/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __VASP_H
#define __VASP_H

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error You need at least flext version 0.4.1
#endif


#include <typeinfo>
#include <stdlib.h>

class complex;
class vector;

#if 0
	#define I int
	#define L long
	#define UL unsigned long
	#define F float
	#define D double
	#define C char
	#define BL bool
	#define V void
	#define S t_sample // type for samples
	#define R double // type for internal calculations
	#define CX complex
	#define VX vector
#else
	typedef int I;
	typedef long L;
	typedef unsigned long UL;
	typedef float F;
	typedef double D;
	typedef char C;
	typedef bool BL;
	typedef void V;
	typedef t_sample S; // type for samples
	typedef double R; // type for internal calculations
	typedef complex CX;
	typedef vector VX;
#endif

#ifdef PD
// buffers are never interleaved - special optimizations may occur
// attention: possibly obsolete when immediate file access is implemented
#define VASP_CHN1  
#endif

class complex 
{ 
public:
	complex() {}
	complex(F re,F im = 0): real(re),imag(im) {}

	F real,imag; 
};

class vector 
{ 
public:
	vector(): dim(0),data(NULL) {}
	~vector() { if(data) delete[] data; }

	I Dim() const { return dim; }
	F *Data() { return data; }
	const F *Data() const { return data; }
protected:
	I dim; F *data; 
};

#endif
