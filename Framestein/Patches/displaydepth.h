#ifndef __DISPLAYDEPTH_H
#define __DISPLAYDEPTH_H

#include "windows.h"

int getdisplaydepth(void)
{
	DEVMODE dm;

	dm.dmSize = sizeof(DEVMODE);
	dm.dmDriverExtra = 0;

	if(!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
		return(0);

	return(dm.dmBitsPerPel);
}

#endif
