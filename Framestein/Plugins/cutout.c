// 242.cutout -- does a rectangular mask really quickly.
//		by r. luke dubois (luke@music.columbia.edu),
//			computer music center, columbia university, 2001.
//
//  Pd / Framestein port by Olaf Matthes <olaf.matthes@gmx.de>, June 2002
//
//	objects and source are provided without warranty of any kind, express or implied.
//
//  usage: cutout <x> <y> <width> <height> <flip>
//

#include <stdio.h>
#include <string.h>
#include "plugin.h"

void perform_effect(struct frame f, struct args a)
{
	printf("Using cutout as effect does nothing!\n");
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	short x, y, w, h, x_c, y_c, w_c, h_c, x_m, y_m, check, flip = 0;
	byte pixelsize = f1.pixelformat/8;
	pixel16 *pix1_16, *pix2_16;
	pixel24 *pix1_24, *pix2_24;
	pixel32 *pix1_32, *pix2_32;
	char *t;

	w = f1.width<f2.width ? f1.width : f2.width;
	h = f1.height<f2.height ? f1.height : f2.height;

	// get params
	if(!a.s) return;
	x_c = atoi(a.s);
	if(!(t = strstr(a.s, " "))) return;
	y_c = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	w_c = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	h_c = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	flip = atoi(t+1);

	printf("cutout: x%d y%d w%d h%d - f%d\n", x_c, y_c, w_c, h_c, flip);

	x_m = x_c + w_c; // figure out bounds of rectangle.
	y_m = y_c + h_c;            

	switch(f1.pixelformat)
	{
		case 16:
			for(y = 0; y < h; y++) 
			{
				pix1_16 = scanline16(f1, y);
				pix2_16 = scanline16(f2, y);
				for(x = 0; x < w; x++)
				{
						if((x >= x_c) && (x <= x_m) && (y >= y_c) && (y <= y_m))
						{
							check = 0;
						}
						else
						{
							check = 1;
						} // occlude or don't occlude

						// sort out which image input goes over and which goes under
						if(flip)check = !check;

						if(!check)
						{
							pix2_16[x] = pix1_16[x]; // set output1
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
						if((x >= x_c) && (x <= x_m) && (y >= y_c) && (y <= y_m))
						{
							check = 0;
						}
						else
						{
							check = 1;
						} // occlude or don't occlude

						// sort out which image input goes over and which goes under
						if(flip)check = !check;

						if(!check)
						{
							pix2_24[x] = pix1_24[x]; // set output1
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
						if((x >= x_c) && (x <= x_m) && (y >= y_c) && (y <= y_m))
						{
							check = 0;
						}
						else
						{
							check = 1;
						} // occlude or don't occlude

						// sort out which image input goes over and which goes under
						if(flip)check = !check;

						if(!check)
						{
							pix2_32[x] = pix1_32[x]; // set output1
						}
				 }
			}
			break;
	}
}
