#include <stdlib.h>
#include <string.h>
#include "plugin.h"

void perform_effect(_frame f, _args a)
{
	printf("NOTE: bend as effect does nothing. Use bend as copy operation.\n");
}

void perform_copy(_frame f1, _frame f2, _args a)
{
	int x, y, w, h, pos, pos2, src, ysrc, ypos1, ypos2,
	 widthminuspos, widthminuspos2, widthminusone,
	 heightminusypos1, heightminusypos2;
	pixel8 *p1, *p2, tp=0, tpy=0;
	char *t;
	byte pixelsize = f1.pixelformat/8;

	if(!a.s) return;
	pos = atoi(a.s);
	if(t = strstr(a.s, " "))
	{
		pos2 = atoi(t+1);
		tp = 1;
		if(t = strstr(t+1, " "))
		{
			ypos1 = atoi(t+1);
			if(t = strstr(t+1, " "))
			{
				ypos2 = atoi(t+1);
				if(ypos1>0||ypos2>0) tpy = 1;
			}
		}
	}

	w = f1.width<f2.width ? f1.width : f2.width;
	h = f1.height<f2.height ? f1.height : f2.height;

	if(pos<0) pos=0;
	if(pos>=w) pos=w-1;
	if(pos2<0) pos2=0;
	if(pos2>=w) pos2=w-1;

	if(ypos1<0) ypos1=0;
	if(ypos1>=w) ypos1=w-1;
	if(ypos2<0) ypos2=0;
	if(ypos2>=w) ypos2=w-1;

	widthminuspos = f1.width-pos;
	widthminuspos2 = f1.width-pos2;
	widthminusone = w-1;

	heightminusypos1 = f1.height-ypos1;
	heightminusypos2 = f1.height-ypos2;

	// here we'll avoid checking for pixelformat by taking the source and destination
	// offsets as 8-bit, and multiplying that by pixelsize

	for(y=0; y<h; y++)
	{
		p1 = scanline(f1, y);
		p2 = scanline(f2, y);
		for(x=0; x<w; x++)
		{
			if(!tp)
			{
				memcpy(&p2[x*pixelsize], x<pos||pos-(x-pos)<0 ? &p1[x*pixelsize]
				 : &p1[(pos-(x-pos))*pixelsize], pixelsize);
			} else
			{
				src = x<pos ? x/(float)pos*pos2
				 : pos2+((x-pos)/(float)widthminuspos*widthminuspos2);

				if(!tpy) {
					memcpy(&p2[x*pixelsize], &p1[src*pixelsize], pixelsize);
				} else {
					ysrc = y<ypos1 ? y/(float)ypos1*ypos2
					 : ypos2+((y-ypos1)/(float)heightminusypos1*heightminusypos2);
					
					memcpy(&p2[x*pixelsize], &scanline(f1, ysrc)[src*pixelsize],
					 pixelsize);
				}
			}
		}
	}
}
