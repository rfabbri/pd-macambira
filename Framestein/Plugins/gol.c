//
// Conway's Game of Life .. also take a look at rowca.cpp
//

// Currently only works with 16bit displays

#include <stdlib.h>
#include <stdio.h>
#include "plugin.h"

#define BORN	1
#define DYING	2

INFO("Conway's Game Of Life")

int aroundme(_frame f, int x, int y)
{
	pixel16 *p;
	int i=0;

	p=scanline16(f, y-1);
	if(p[x-1]) i++;
	if(p[x]) i++;
	if(p[x+1]) i++;

	p=scanline16(f, y);
	if(p[x-1]) i++;
	if(p[x+1]) i++;

	p=scanline16(f, y+1);
	if(p[x-1]) i++;
	if(p[x]) i++;
	if(p[x+1]) i++;

	return i;
}

byte *t=0;
long size=0;

void setstate(_frame f, int x, int y)
{
	int i = aroundme(f, x, y);

	if(i<=1 || i>=4) t[y*f.width+x]=DYING;
	else if(i==3) t[y*f.width+x]=BORN;
}

void perform_effect(struct frame f, struct args a)
{
	int x,y,i,r;
	pixel16 *p, c;

	if(f.pixelformat!=16)
	{
		printf("Sorry, gol works only on a 16bit display.\n");
		return;
	}

	if(t && size != f.width*f.height)
	{
		free(t);
		t=0;
	}

	if(!t)
		t = malloc(size = f.width*f.height);

	memset(t, 0, f.width*f.height);

	for(y=2; y<f.height-3; y++)
	{
		p = scanline16(f, y);
		for(x=2; x<f.width-3; x++)
		{
			if(p[x])
			{
				setstate(f, x-1, y-1);
				setstate(f, x, y-1);
				setstate(f, x+1, y-1);
				setstate(f, x-1, y);
				setstate(f, x, y);
				setstate(f, x+1, y);
				setstate(f, x-1, y+1);
				setstate(f, x, y+1);
				setstate(f, x+1, y+1);
			}
		}
	}

	for(y=2; y<f.height-3; y++)
	{
		p = scanline16(f, y);
		for(x=2; x<f.width-3; x++)
		{
			switch(t[y*f.width+x])
			{
				case BORN:
					p[x] = 5000;
					break;
				case DYING:
					p[x] = 0;
					break;
			}
		}
	}
}
