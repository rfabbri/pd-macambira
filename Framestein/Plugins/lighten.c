//
//  lighten - lighten of two images
//  
//	written by Olaf Matthes <olaf.matthes@gmx.de>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//   
//  usage: lighten
//

#include <stdlib.h>
#include "plugin.h"

void perform_effect(struct frame f, struct args a)
{
	printf("Using lighten as effect does nothing!\n");
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	short x, y, w, h;
	long count = 0;
	pixel16 *pix1_16, *pix2_16;
	pixel24 *pix1_24, *pix2_24;
	pixel32 *pix1_32, *pix2_32;

	w = f1.width<f2.width ? f1.width : f2.width;
	h = f1.height<f2.height ? f1.height : f2.height;

	printf("lighten\n");

	switch(f1.pixelformat)
	{
		case 16:
			// compare images pixel by pixel
			for(y = 0; y < h; y++) 
			{
				pix1_16 = scanline16(f1, y);
				pix2_16 = scanline16(f2, y);
				for(x = 0; x < w; x++)
				{
					pix2_16[x] = rgbtocolor16(klamp255(max(r16(pix1_16[x]), r16(pix2_16[x]))), 
						                      klamp255(max(g16(pix1_16[x]), g16(pix2_16[x]))), 
											  klamp255(max(b16(pix1_16[x]), b16(pix2_16[x]))));
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
					pix2_24[x] = rgbtocolor24(klamp255(max(r24(pix1_24[x]), r24(pix2_24[x]))), 
						                      klamp255(max(g24(pix1_24[x]), g24(pix2_24[x]))), 
											  klamp255(max(b24(pix1_24[x]), b24(pix2_24[x]))));
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
					pix2_32[x] = rgbtocolor32(klamp255(max(r32(pix1_32[x]), r32(pix2_32[x]))), 
						                      klamp255(max(g32(pix1_32[x]), g32(pix2_32[x]))), 
											  klamp255(max(b32(pix1_32[x]), b32(pix2_32[x]))));
				}
			}
			break;
	}
}
