
//
// Game of Life .. what a waste of time
//

#include <stdlib.h>
#include <stdio.h>
#include "plugin.h"

#define BORN	1
#define DYING	2

int dead = 0;

int aroundme(_frame f, int x, int y)
{
	int i=0;
	if(r16(scanline16(f, y-1)[x-1])>dead) i++;
	if(r16(scanline16(f, y-1)[x])>dead) i++;
	if(r16(scanline16(f, y-1)[x+1])>dead) i++;
	if(r16(scanline16(f, y)[x-1])>dead) i++;
	if(r16(scanline16(f, y)[x+1])>dead) i++;
	if(r16(scanline16(f, y+1)[x-1])>dead) i++;
	if(r16(scanline16(f, y+1)[x])>dead) i++;
	if(r16(scanline16(f, y+1)[x+1])>dead) i++;
	return i;
}

void setstate(_frame f, byte *t, int x, int y)
{
	int i = aroundme(f, x, y);

	if(i<=1 || i>=4) t[y*f.width+x]=DYING;
	else
	if(i==3) t[y*f.width+x]=BORN;
}

void perform_effect(struct frame f, struct args a)
{
	int x,y,i,r;
	pixel16 *p, c;
	byte *t;

	if(f.pixelformat!=16) return;

	t = malloc(f.width*f.height);
	memset(t, 0, f.width*f.height);

	for(y=2; y<f.height-3; y++)
	{
		p = scanline16(f, y);
		for(x=2; x<f.width-3; x++)
		{
			if(r16(p[x])>0)
			{
				setstate(f, t, x-1, y-1);
				setstate(f, t, x, y-1);
				setstate(f, t, x+1, y-1);
				setstate(f, t, x-1, y);
				setstate(f, t, x, y);
				setstate(f, t, x+1, y);
				setstate(f, t, x-1, y+1);
				setstate(f, t, x, y+1);
				setstate(f, t, x+1, y+1);
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
					c = p[x];
					r = r16(c);
					p[x] = rgbtocolor16(r>0 ? r : 255, g16(c), b16(c));
					break;
				case DYING:
					c = p[+x];
					p[x] = rgbtocolor16(0, g16(c), b16(c));
					break;
			}
		}
	}
	free(t);
}
