// 242.cga -- does bit-level quantization on an input image.
//		by r. luke dubois (luke@music.columbia.edu),
//			computer music center, columbia university, 2001.
//
//  Pd / Framestein port by Olaf Matthes <olaf.matthes@gmx.de>, June 2002
//
//	objects and source are provided without warranty of any kind, express or implied.
//
//  usage: cga <red> <green> <blue>
//

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "plugin.h"

INFO("242.cga -- does bit-level quantization on an input image")

#pragma warning( disable : 4761 )	// the (lossy) conversion _IS_ our effect !

void perform_effect(struct frame f, struct args arg)
{
	int x,y;
	int red, green, blue;    // parameters
	byte r, g, b;
	byte bits = f.pixelformat/8;
	pixel16 *c16;
	pixel24 *c24;
	pixel32 *c32;
	char *t;

	// get r g b and alpha params
	if(!arg.s) return;
	red = atoi(arg.s);
	if(!(t = strstr(arg.s, " "))) return;
	green = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	blue = atoi(t+1);

	printf("cga: r%d g%d b%d\n", red, green, blue);

	for(y = 0; y < f.height; y++) 
	{
		for(x = 0; x < f.width; x++)
		{
			switch(f.pixelformat)
			{
				case 16:
					c16 = scanline16(f, y);
					r = r16(c16[x]);
					g = g16(c16[x]);
					b = b16(c16[x]);
                        // shift it up, then shift it back down
					c16[x] = rgbtocolor16((r>>red<<red), (g>>green<<green), (b>>blue<<blue));
					break;
				case 24:
					c24 = scanline24(f, y);
					r = r24(c24[x]);
					g = g24(c24[x]);
					b = b24(c24[x]);
                        // shift it up, then shift it back down
					c24[x] = rgbtocolor24((r>>red<<red), (g>>green<<green), (b>>blue<<blue));
					break;
				case 32:
					c32 = scanline32(f, y);
					r = r32(c32[x]);
					g = g32(c32[x]);
					b = b32(c32[x]);
                        // shift it up, then shift it back down
					c32[x] = rgbtocolor32((r>>red<<red), (g>>green<<green), (b>>blue<<blue));
					break;
			}
		}
	}
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	printf("Using cga as copy operation does nothing!\n");
}
