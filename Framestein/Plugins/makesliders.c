//
// makesliders: display slider values
//
// usage: makesliders 14 56 76 140
// displays four sliders with given values
//

#include <string.h>
#include "plugin.h"

#define MAXVALS 128

void perform_effect(_frame f, _args a)
{
	pixel8 *p8;
	char *t=a.s;
	int val[MAXVALS], valcount=0, i, y, bitsperpixel=f.pixelformat / 8, col;

	if(!t) return;

	while(t && valcount<MAXVALS)
	{
		val[valcount++] = atoi(t);
		t = strstr(t, " ");
		if(t) t++;
	}

	memset(f.bits, 0, f.height*f.lpitch);
	col = rand()%256;

	for(i=0; i<f.width; i++)
	{
		y = val[(int)(i / (float)f.width * (float)valcount)];
		if(y>=0 && y<f.height)
		{
			p8 = scanline(f, y);
			memset(p8+i*bitsperpixel, 200, bitsperpixel);
		}
	}
}
