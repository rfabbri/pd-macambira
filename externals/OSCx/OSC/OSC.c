/*

	pd
	-------------
		-- tweaks for Win32    www.zeggz.com/raf	13-April-2002

 */


#ifndef VERSION
	#ifdef WIN32
		#define VERSION "0.01-w32"
	#else
		#define VERSION "0.01"
	#endif
#endif

#ifndef __DATE__ 
#define __DATE__ "without using a gnu compiler"
#endif

#include <m_pd.h>


typedef struct _OSC
{
     t_object x_obj;
} t_OSC;

static t_class* OSC_class;

#ifdef WIN32
	#include "OSC-common.h"
	OSC_API void OSC_setup(void);
	OSC_API void OSC_version(t_OSC*);
	OSC_API void sendOSC_setup(void);
	OSC_API void dumpOSC_setup(void);
	OSC_API void OSCroute_setup(void);
#else
	void OSC_version();
	void sendOSC_setup();
	void dumpOSC_setup();
	void OSCroute_setup();
#endif

static void* OSC_new(t_symbol* s) {
    t_OSC *x = (t_OSC *)pd_new(OSC_class);
    return (x);
}

#ifdef WIN32
	OSC_API void OSC_version (t_OSC *x) { 
#else
	void OSC_version (t_OSC *x) {
#endif
  // EnterCallback();
  post("OSC4PD Version " VERSION
       "\n ¯\\    original code by matt wright. pd-fication jdl@xdv.org\n"
       "   ·   Win32-port raf@interaccess.com\n    \\_ Compiled " __TIME__ " " __DATE__);
  // ExitCallback();
}

#ifdef WIN32
	OSC_API void OSC_setup(void) { 
#else
	void OSC_setup(void) {
#endif
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
