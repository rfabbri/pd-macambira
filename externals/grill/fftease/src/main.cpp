/*

FFTease - A set of Live Spectral Processors
Originally written by Eric Lyon and Christopher Penrose for the Max/MSP platform

Copyright (c)Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"


// Initialization function for xsample library
static V fftease_main()
{
	post("");
	post("-------------------------------------------------------------------");
	post("FFTease - A set of Live Spectral Processors");
	post("Originally written by Eric Lyon and Christopher Penrose for MAX/MSP");
	post("");
	post("flext port (version " FFTEASE_VERSION ") provided by Thomas Grill, (C)2003");
	post("-------------------------------------------------------------------");
	post("");

	// call the objects' setup routines
	FLEXT_DSP_SETUP(burrow);
	FLEXT_DSP_SETUP(cross);
	FLEXT_DSP_SETUP(dentist);
	FLEXT_DSP_SETUP(disarray);
	FLEXT_DSP_SETUP(drown);
	FLEXT_DSP_SETUP(ether);
	FLEXT_DSP_SETUP(morphine);
	FLEXT_DSP_SETUP(scrape);
	FLEXT_DSP_SETUP(shapee);
	FLEXT_DSP_SETUP(swinger);
	FLEXT_DSP_SETUP(taint);
	FLEXT_DSP_SETUP(thresher);
	FLEXT_DSP_SETUP(vacancy);
	FLEXT_DSP_SETUP(xsyn);
/*
	FLEXT_DSP_SETUP(pvcompand);
	FLEXT_DSP_SETUP(pvoc);

*/

#if FLEXT_SYS == FLEXT_SYS_MAX
	finder_addclass((C *)"FFTease",(C *)"burrow~");
	finder_addclass((C *)"FFTease",(C *)"cross~");
	finder_addclass((C *)"FFTease",(C *)"dentist~");
	finder_addclass((C *)"FFTease",(C *)"disarray~");
	finder_addclass((C *)"FFTease",(C *)"drown~");
	finder_addclass((C *)"FFTease",(C *)"ether~");
	finder_addclass((C *)"FFTease",(C *)"morphine~");
	finder_addclass((C *)"FFTease",(C *)"scrape~");
	finder_addclass((C *)"FFTease",(C *)"shapee~");
	finder_addclass((C *)"FFTease",(C *)"swinger~");
	finder_addclass((C *)"FFTease",(C *)"taint~");
	finder_addclass((C *)"FFTease",(C *)"thresher~");
	finder_addclass((C *)"FFTease",(C *)"vacancy~");
	finder_addclass((C *)"FFTease",(C *)"xsyn~");
#endif
}

// setup the library
FLEXT_LIB_SETUP(fftease,fftease_main)

