/*

	pd
	-------------
		-- tweaks for Win32    www.zeggz.com/raf	13-April-2002
		-- smoothed out build, clean up for Darwin  <hans@at.or.at> 2004.04.04
*/

#if HAVE_CONFIG_H 
#include <config.h> 
#endif

#include <m_pd.h>
#include "OSC-common.h"

#define VERSION "0.2"

#ifndef OSC_API 
#define OSC_API
#endif

typedef struct _OSC
{
     t_object x_obj;
} t_OSC;


OSC_API void OSC_setup(void);
OSC_API void OSC_version(t_OSC*);
/*
OSC_API void sendOSC_setup(void);
OSC_API void dumpOSC_setup(void);
OSC_API void OSCroute_setup(void);
*/

static t_class* OSC_class;


static void* OSC_new(t_symbol* s) {
    t_OSC *x = (t_OSC *)pd_new(OSC_class);
    return (x);
}


OSC_API void OSC_version (t_OSC *x) { 

  // EnterCallback();
  post("OSC4PD Version " VERSION
       "\n ¯\\    original code by matt wright. pd-fication jdl@xdv.org\n"
       "   ·   Win32-port raf@interaccess.com    Darwin cleanup hans@at.or.at\n"
		 "		 \\_ Compiled " __TIME__ " " __DATE__);
  // ExitCallback();
}

OSC_API void OSC_setup(void) { 
  OSC_class = class_new(gensym("OSC"), (t_newmethod)OSC_new, 0,
			  sizeof(t_OSC), 0,0);
  class_addmethod(OSC_class, (t_method)OSC_version, gensym("version"), A_NULL, 0, 0);

  sendOSC_setup();
  dumpOSC_setup();
  OSCroute_setup();
  
  post("O  : Open Sound Control 4 PD, http://www.cnmat.berkeley.edu/OSC");
  post(" S : original code by matt wright, pd hakcs cxc, Win32-port raf@interaccess.com");
  post("  C: ver: "VERSION ", compiled: "__DATE__);
}
