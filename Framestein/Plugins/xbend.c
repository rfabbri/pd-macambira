#include <stdlib.h>
#include <string.h>
#include "plugin.h"

INFO("horizontal bend. usage: xbend <from> <to>")

void perform_effect(_frame f, _args a)
{
	byte pixelsize = f.pixelformat/8;
	int x, y, pos, pos2, widthminuspos, widthminuspos2;
	pixel8 *p, *tp=0;
	char *t;

	if(!a.s) return;

	pos = atoi(a.s);
	if(t = strstr(a.s, " "))
	{
		pos2 = atoi(t+1);
		tp = (pixel8 *)malloc(f.width*pixelsize);
	}

	if(pos<0) pos=0;
	if(pos>=f.width) pos=f.width-1;
	if(pos2<0) pos2=0;
	if(pos2>=f.width) pos2=f.width-1;

	widthminuspos = f.width-pos;
	widthminuspos2 = f.width-pos2;

	for(y=0; y<f.height; y++)
	{
		p = scanline(f, y);
		for(x=0; x<f.width; x++)
		{
			if(!tp)
//				p[x] = x<pos||pos-(x-pos)<0 ? p[x] : p[pos-(x-pos)];
				memcpy(&p[x*pixelsize],
				 x<pos||pos-(x-pos)<0 ? &p[x*pixelsize] : &p[(pos-(x-pos))*pixelsize],
				 pixelsize);
			else
			{
				memcpy(&tp[x*pixelsize],
				 &p[x<pos ? ((int)(x/(float)pos*pos2))*pixelsize
				 : (pos2+(int)(((x-pos)/(float)widthminuspos*widthminuspos2)))*pixelsize],
				 pixelsize);
			}
		}
		if(tp) memcpy(p, tp, f.width*pixelsize);
	}
	if(tp) free(tp);
}

void perform_copy(_frame f1, _frame f2, _args a)
{
	byte pixelsize = f1.pixelformat/8;
	int x, y, w, h, pos, pos2, widthminuspos, widthminuspos2, widthminusone;
	pixel8 *p1, *p2, tp=0;
	char *t;

	if(!a.s) return;

	pos = atoi(a.s);
	if(t = strstr(a.s, " "))
	{
		pos2 = atoi(t+1);
		tp = 1;
	}

	w = f1.width<f2.width ? f1.width : f2.width;
	h = f1.height<f2.height ? f1.height : f2.height;

	if(pos<0) pos=0;
	if(pos>=w) pos=w-1;
	if(pos2<0) pos2=0;
	if(pos2>=w) pos2=w-1;

	widthminuspos = f1.width-pos;
	widthminuspos2 = f1.width-pos2;
	widthminusone = w-1;

	for(y=0; y<h; y++)
	{
		p1 = scanline(f1, y);
		p2 = scanline(f2, y);
		for(x=0; x<w; x++)
		{
			if(!tp)
				memcpy(&p2[x*pixelsize],
				 x<pos||pos-(x-pos)<0 ? &p1[x*pixelsize] : &p1[(pos-(x-pos))*pixelsize],
				 pixelsize);
			else
			{
				memcpy(&p2[x*pixelsize],
				 &p1[ x<pos ? ((int)(x/(float)pos*pos2))*pixelsize
				 : (pos2+(int)(((x-pos)/(float)widthminuspos*widthminuspos2)))*pixelsize],
				 pixelsize);
			}
		}
	}
}
