#include <stdlib.h>
#include <string.h>
#include "plugin.h"

INFO("copy one vertical line at a given position")

void perform_copy(_frame f1, _frame f2, _args a)
{
	byte pixelsize = f1.pixelformat/8;
	short x, y, xoff, w, h, size=1;
	char *t;

	if(!a.s) return;
	xoff = atoi(a.s);

	if(t = strstr(a.s, " ")) size=atoi(t+1);

	w = f1.width<f2.width ? f1.width : f2.width;
	h = f1.height<f2.height ? f1.height : f2.height;

	if(xoff+size>=w)
	{
		size = w - xoff;
		if(!size) return;
	}

	if(xoff<0 || xoff>=w) return;

	for(y=0; y<h; y++)
		memcpy(&f2.bits[y*f2.lpitch+xoff*pixelsize], &f1.bits[y*f1.lpitch+xoff*pixelsize],
		 size*pixelsize);
}
