// 242.modgain -- does modulo channel shifting on an input image.
//		by r. luke dubois (luke@music.columbia.edu),
//			computer music center, columbia university, 2001.
//
//  Pd / Framestein port by Olaf Matthes <olaf.matthes@gmx.de>, June 2002
//
//	objects and source are provided without warranty of any kind, express or implied.
//
//  usage: modgain <red> <green> <blue>
//

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "plugin.h"

void perform_effect(struct frame f, struct args arg)
{
	int x,y;
	int red, green, blue, alpha;    // parameters
	byte r, g, b, a;
	byte bits = f.pixelformat/8;
	byte redpix, greenpix, bluepix;	// the color values
	pixel16 *c16;
	pixel24 *c24;
	pixel32 *c32;
	char *t;

	// get params
	if(!arg.s) return;
	red = atoi(arg.s);
	if(!(t = strstr(arg.s, " "))) return;
	green = atoi(t+1);
	if(!(t = strstr(arg.s, " "))) return;
	blue = atoi(t+1);

	printf("modgain: r%d g%d b%d\n", red, green, blue);

	for(y = 0; y < f.height; y++) 
	{
		for(x = 0; x < f.width; x++)
		{
			switch(f.pixelformat)
			{
				case 16:
					c16 = scanline16(f, y);
					r = (r16(c16[x]) + red) % 255;
					g = (g16(c16[x]) + green) % 255;
					b = (b16(c16[x]) + blue) % 255;
						// shift the pixels and wrap them
					if(r<0) r += 255;
					if(g<0) g += 255;
					if(b<0) b += 255;

					c16[x] = rgbtocolor16(r, g, b);
					break;
				case 24:
					c24 = scanline24(f, y);
					r = (r24(c24[x]) + red) % 255;
					g = (g24(c24[x]) + green) % 255;
					b = (b24(c24[x]) + blue) % 255;
						// shift the pixels and wrap them
					if(r<0) r += 255;
					if(g<0) g += 255;
					if(b<0) b += 255;

					c24[x] = rgbtocolor24(r, g, b);
					break;
				case 32:
					c32 = scanline32(f, y);
					r = (r32(c32[x]) + red) % 255;
					g = (g32(c32[x]) + green) % 255;
					b = (b32(c32[x]) + blue) % 255;
						// shift the pixels and wrap them
					if(r<0) r += 255;
					if(g<0) g += 255;
					if(b<0) b += 255;

					c32[x] = rgbtocolor32(r, g, b);
					break;
			}
		}
	}
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	printf("Using modgain as copy operation does nothing!\n");
}
