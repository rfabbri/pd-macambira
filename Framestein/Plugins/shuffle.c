#include <string.h>
#include "plugin.h"

INFO("randomly move around pixels")

void perform_effect(_frame f, _args a)
{
	int i, o=1000, x1, y1, x2, y2, range=10;
	char *t;
	pixel8 *p1, *p2;
	pixel32 dot;
	byte pixelsize=f.pixelformat/8;

	if(f.pixelformat>32) return;

	if(a.s)
	{
		o = atoi(a.s);
		if(o==0) o=1000;

		if(t = strstr(a.s, " "))
		if((range = atoi(t+1))==0) return;
	}

	for(i=0; i<o; i++)
	{
		x1 = rand()%f.width;
		y1 = rand()%f.height;
		x2 = x1 + (rand()%(range*2) - range);
		y2 = y1 + (rand()%(range*2) - range);

		if(x2<0) x2=0;
		if(x2>=f.width) x2=f.width-1;
		if(y2<0) y2=0;
		if(y2>=f.height) y2=f.height-1;

		p1 = scanline(f, y1);
		p2 = scanline(f, y2);

		memcpy(&dot, &p2[x2*pixelsize], pixelsize);
		memcpy(&p2[x2*pixelsize], &p1[x1*pixelsize], pixelsize);
		memcpy(&p1[x1*pixelsize], &dot, pixelsize);
	}
}
