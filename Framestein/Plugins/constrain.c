// 242.constrain -- does color range constraining on an input image.
//		by r. luke dubois (luke@music.columbia.edu),
//			computer music center, columbia university, 2001.
//
//  Pd / Framestein port by Olaf Matthes <olaf.matthes@gmx.de>, June 2002
//
//	objects and source are provided without warranty of any kind, express or implied.
//
//  usage: constrain <min red> <min green> <min blue> <max red> <max green> <max blue>
//

#include <stdio.h>
#include <string.h>
#include "plugin.h"

INFO("242.constrain -- does color range constraining on an input image")

void perform_effect(struct frame f, struct args a)
{
	int x,y;
	byte redmin, greenmin, bluemin, redmax, greenmax, bluemax;
	byte redpix, greenpix, bluepix;	// the color values
	pixel16 *c16;
	pixel24 *c24;
	pixel32 *c32;
	char *t;

	// get params
	if(!a.s) return;
	redmin = atoi(a.s);
	if(!(t = strstr(a.s, " "))) return;
	greenmin = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	bluemin = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	redmax = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	greenmax = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	bluemax = atoi(t+1);

	printf("constrain: rmin%d gmin%d bmin%d - rmax%d gmax%d - bmax%d\n", 
		    redmin, greenmin, bluemin, redmax, greenmax, bluemax);


	for(y = 0; y < f.height; y++) 
	{
		for(x = 0; x < f.width; x++)
		{
			switch(f.pixelformat)
			{
				case 16:
					c16 = scanline16(f, y);
					redpix = r16(c16[x]);
					greenpix = g16(c16[x]);
					bluepix = b16(c16[x]);

					if(redpix < redmin) redpix = redmin;
					if(redpix > redmax) redpix = redmax;
					if(greenpix < greenmin) greenpix = greenmin;
					if(greenpix > greenmax) greenpix = greenmax;
					if(bluepix < bluemin) bluepix = bluemin;
					if(bluepix > bluemax) bluepix = bluemax;

					// set the output pixel
					c16[x] = rgbtocolor16(redpix, greenpix, bluepix);
					break;
				case 24:
					c24 = scanline24(f, y);
					redpix = r24(c24[x]);
					greenpix = g24(c24[x]);
					bluepix = b24(c24[x]);

					if(redpix < redmin) redpix = redmin;
					if(redpix > redmax) redpix = redmax;
					if(greenpix < greenmin) greenpix = greenmin;
					if(greenpix > greenmax) greenpix = greenmax;
					if(bluepix < bluemin) bluepix = bluemin;
					if(bluepix > bluemax) bluepix = bluemax;

					// set the output pixel
					c24[x] = rgbtocolor24(redpix, greenpix, bluepix);
					break;
				case 32:
					c32 = scanline32(f, y);
					redpix = r32(c32[x]);
					greenpix = g32(c32[x]);
					bluepix = b32(c32[x]);

					if(redpix < redmin) redpix = redmin;
					if(redpix > redmax) redpix = redmax;
					if(greenpix < greenmin) greenpix = greenmin;
					if(greenpix > greenmax) greenpix = greenmax;
					if(bluepix < bluemin) bluepix = bluemin;
					if(bluepix > bluemax) bluepix = bluemax;

					// set the output pixel
					c32[x] = rgbtocolor32(redpix, greenpix, bluepix);
					break;
			}
		}
	}
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	printf("Using constrain as copy operation does nothing!\n");
}
