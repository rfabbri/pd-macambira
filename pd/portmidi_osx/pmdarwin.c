/*
 * PortMidi OS-dependent interface for Darwin (MacOS X)
 * Jon Parise <jparise@cmu.edu>
 *
 * $Id: pmdarwin.c,v 1.1.1.1 2003-05-09 16:04:00 ggeiger Exp $
 */

/*
 * This file only needs to implement pm_init(), which calls various
 * routines to register the available midi devices. This file must
 * be separate from the main portmidi.c file because it is system
 * dependent, and it is separate from, say, pmwinmm.c, because it
 * might need to register devices for winmm, directx, and others.
 */

#include <stdlib.h>
#include "portmidi.h"
#include "pmmacosx.h"

PmError pm_init()
{
	return pm_macosx_init();
}

PmError pm_term()
{
	return pm_macosx_term();
}

PmDeviceID Pm_GetDefaultInputDeviceID() { return 0; };
PmDeviceID Pm_GetDefaultOutputDeviceID() { return 0; };

void *pm_alloc(size_t s) { return malloc(s); }

void pm_free(void *ptr) { free(ptr); }

