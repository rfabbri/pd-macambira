//
// sonogram.cpp
//
// use with fs.sonogram
//
// see example-sonogram.pd
//

#include <stdlib.h>
#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include "plugin.h"
#include "pixels.h"

#define SONOSIZE 128
#define SONOMAXVAL 500
#define sonotype float

INFO("helper for fs.sonogram")

int callcount=-1;

void perform_effect(_frame f, _args a)
{
	if(!a.s) return;

	if(++callcount>=f.width) callcount=0;

	FILE *inf;
	if((inf = fopen(a.s, "rb"))==NULL)
	{
		printf("sonogram: error opening %s\n", a.s);
		return;
	}

	char buf[80];
	sonotype sono[SONOSIZE];
	int i=0;

	while(!feof(inf))
	{
		if(!fgets(buf, 80, inf))
		{
			printf("sonogram: read error.\n");
			return;
		}
		sono[i] = atof(buf);
		if(++i>=SONOSIZE) break;
	}
	fclose(inf);

	int y;

	for(i=0; i<SONOSIZE; i++)
	{
		// scale values 0-500 to 0-255
		if(sono[i]>SONOMAXVAL) sono[i]=SONOMAXVAL;
		sono[i] = (sonotype)((sono[i] / (float)SONOMAXVAL)*255);
		// white = no sound
		sono[i] = abs(sono[i]-255);

		y = (int)((i / (float)SONOSIZE) * (float)f.height);
		// bottom = bass
		y = abs((f.height-1) - y);

		switch(f.pixelformat)
		{
			case 16:
				scanline16(f, y)[callcount] =
				 rgbtocolor16(sono[i], sono[i], sono[i]);
				break;
			case 24:
				scanline24(f, y)[callcount] =
				 rgbtocolor24(sono[i], sono[i], sono[i]);
				break;
			case 32:
				scanline32(f, y)[callcount] =
				 rgbtocolor32(sono[i], sono[i], sono[i]);
				break;
		}
	}
}

