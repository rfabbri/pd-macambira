/*
 * PortMidi OS-dependent interface for Darwin (MacOS X)
 * Jon Parise <jparise@cmu.edu>
 *
 * $Id: pmdarwin.c,v 1.6 2004-11-11 04:58:20 millerpuckette Exp $
 *
 * CHANGE LOG:
 * 03Jul03 - X. J. Scott (xjs):
 *   - Pm_GetDefaultInputDeviceID() and Pm_GetDefaultOutputDeviceID()
 *     now return id of first input and output devices in system,
 *     rather than returning 0 as before.
 *     This fix enables valid default port values to be returned.
 *     0 is returned if no such device is found.
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

PmError pm_init(void) // xjs added void
{
	return pm_macosx_init();
}

PmError pm_term(void) // xjs added void
{
	return pm_macosx_term();
}

/* Pm_GetDefaultInputDeviceID() - return input with lowest id # (xjs)
 */
PmDeviceID Pm_GetDefaultInputDeviceID()
{
  int i;
  int device_count;
  const PmDeviceInfo *deviceInfo;

  device_count = Pm_CountDevices();
  for (i = 0; i < device_count; i++) {
    deviceInfo = Pm_GetDeviceInfo(i);
    if (deviceInfo->input)
      return i;
  }

  return 0;
};

/* Pm_GetDefaultOutputDeviceID() - return output with lowest id # (xjs)
*/
PmDeviceID Pm_GetDefaultOutputDeviceID()
{
  int i;
  int device_count;
  const PmDeviceInfo *deviceInfo;

  device_count = Pm_CountDevices();
  for (i = 0; i < device_count; i++) {
    deviceInfo = Pm_GetDeviceInfo(i);
    if (deviceInfo->output)
      return i;
  }

  return 0;
};


void *pm_alloc(size_t s) { return malloc(s); }

void pm_free(void *ptr) { free(ptr); }

