#include <stdio.h>
#include "plugin.h"

INFO("turn some random bits")

void perform_effect(struct frame f, struct args a)
{
	int i,o,x,y;
	byte bits = f.pixelformat/8;

	o=(f.width*f.height)/10;
	for(i=0; i<o; i++)
	{
		x=rand()%(f.width);
		y=rand()%(f.height);
		f.bits[y*f.lpitch+x*bits+1]=rand()%(256);
	}
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	printf("Using noize as copy operation does nothing!\n");
}
