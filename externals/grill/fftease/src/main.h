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


#endif
