// 242.keyscreen -- does 3-source chroma keying.
//		by r. luke dubois (luke@music.columbia.edu),
//			computer music center, columbia university, 2001.
//
//  Pd / Framestein port by Olaf Matthes <olaf.matthes@gmx.de>, June 2002
//
//	objects and source are provided without warranty of any kind, express or implied.
//
//  usage: keyscreen <red> <green> <blue> <fuzzi red> <fuzzi green> <fuzzi blue>
//
//         f2 is the key & target, f1 is the mask
//

#include <stdio.h>
#include <string.h>
#include "plugin.h"

INFO("242.keyscreen -- does 3-source chroma keying")

void perform_effect(struct frame f, struct args a)
{
	printf("Using keyscreen as effect does nothing!\n");
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	short x, y, w, h;
	pixel16 *pix1_16, *pix2_16;
	pixel24 *pix1_24, *pix2_24;
	pixel32 *pix1_32, *pix2_32;
	byte redpix, greenpix, bluepix;
	byte red, green, blue, check, rf, gf, bf;
	char *t;

	w = f1.width<f2.width ? f1.width : f2.width;
	h = f1.height<f2.height ? f1.height : f2.height;

	// get params
	if(!a.s) return;
	red = atoi(a.s);
	if(!(t = strstr(a.s, " "))) return;
	green = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	blue = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	rf = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	gf = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	bf = atoi(t+1);

	printf("keyscreen: r%d g%d b%d rf%d gf%d bf%d\n", red, green, blue, rf, gf, bf);

	// key all channels simultaneously
	switch(f1.pixelformat)
	{
		case 16:
			for(y = 0; y < h; y++) 
			{
				pix1_16 = scanline16(f1, y);
				pix2_16 = scanline16(f2, y);
				for(x = 0; x < w; x++)
				{
					check = 0; // start with each pixel anew

					redpix = r16(pix2_16[x]);
					greenpix = g16(pix2_16[x]);
					bluepix = b16(pix2_16[x]);
					if ((redpix>=(red-rf))&&(redpix<=(red+rf))&&(greenpix>=(green-gf))&&(greenpix<=(green+gf))&&(bluepix>=(blue-bf))&&(bluepix<=(blue+bf)))
					{
						check = 1; // mask it
					}        


					if(check)		// output frame 1
					{
						pix2_16[x] = pix1_16[x];
					}
				}
			}
			break;
		case 24:
			for(y = 0; y < h; y++) 
			{
				pix1_24 = scanline24(f1, y);
				pix2_24 = scanline24(f2, y);
				for(x = 0; x < w; x++)
				{
					check = 0; // start with each pixel anew

					redpix = r24(pix2_24[x]);
					greenpix = g24(pix2_24[x]);
					bluepix = b24(pix2_24[x]);
					if ((redpix>=(red-rf))&&(redpix<=(red+rf))&&(greenpix>=(green-gf))&&(greenpix<=(green+gf))&&(bluepix>=(blue-bf))&&(bluepix<=(blue+bf)))
					{
						check = 1; // mask it
					}        


					if(check)		// output frame 1
					{
						pix1_24[x] = pix1_24[x];
					}
				}
			}
			break;
		case 32:
			for(y = 0; y < h; y++) 
			{
				pix1_32 = scanline32(f1, y);
				pix2_32 = scanline32(f2, y);
				for(x = 0; x < w; x++)
				{
					check = 0; // start with each pixel anew

					redpix = r32(pix2_32[x]);
					greenpix = g32(pix2_32[x]);
					bluepix = b32(pix2_32[x]);
					if ((redpix>=(red-rf))&&(redpix<=(red+rf))&&(greenpix>=(green-gf))&&(greenpix<=(green+gf))&&(bluepix>=(blue-bf))&&(bluepix<=(blue+bf)))
					{
						check = 1; // mask it
					}        


					if(check)		// output frame 1
					{
						pix2_32[x] = pix1_32[x];
					}
				}
			}
			break;
	}
}
