//
//  compare	- compare two images for differences
//  
//  if pixels are identical a background color get's displayed,
//	returns number of pixels that are identical
//
//	written by Olaf Matthes <olaf.matthes@gmx.de>
//  inspired by code written by Trond Lossius, Bergen senter for elektronisk kunst (BEK)
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
//  usage: compare <red fuzz> <green fuzz> <blue fuzz> <bg red> <bg green> <bg blue> [<receive name>]
//  in case the (optional) receive name is specified, you get back the number of pixels that have been
//  found to be identical in both images
//

#include <stdio.h>
#include <string.h>
#include "plugin.h"

INFO("compare two images for differences")

void perform_effect(struct frame f, struct args a)
{
	printf("Using compare as effect does nothing!\n");
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	short x, y, w, h, ret;
	long count = 0;
	pixel16 *pix1_16, *pix2_16;
	pixel24 *pix1_24, *pix2_24;
	pixel32 *pix1_32, *pix2_32;
	byte redfuzzi, greenfuzzi, bluefuzzi, redbg, greenbg, bluebg, check;
	char *t;
	char *ret_count;	// returns the result (number of identical pixels)

	w = f1.width<f2.width ? f1.width : f2.width;
	h = f1.height<f2.height ? f1.height : f2.height;

	// get params
	if(!a.s) return;
	redfuzzi = klamp255(atoi(a.s));
	if(!(t = strstr(a.s, " "))) return;
	greenfuzzi = klamp255(atoi(t+1));
	if(!(t = strstr(t+1, " "))) return;
	bluefuzzi = klamp255(atoi(t+1));
	if(!(t = strstr(t+1, " "))) return;
	redbg = klamp255(atoi(t+1));
	if(!(t = strstr(t+1, " "))) return;
	greenbg = klamp255(atoi(t+1));
	if(!(t = strstr(t+1, " "))) return;
	bluebg = klamp255(atoi(t+1));

	printf("compare: rf%d gf%d bf%d - rbg%d gbg%d bbg%d\n", 
		    redfuzzi, greenfuzzi, bluefuzzi, redbg, greenbg, bluebg);

	// get returnvaluereceivenames
	if(!(t = strstr(t+1, " ")))
	{
		ret = 0;	// don't return anything
	}
	else
	{
		ret = 1;	// return number of identical pixels
		ret_count = t+1;
		t[0]=0;
	}

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
					check = 1;

					if(abs(r16(pix1_16[x]) - r16(pix2_16[x])) > redfuzzi)
						check = 0;
					else if(abs(g16(pix1_16[x]) - g16(pix2_16[x])) > greenfuzzi)
						check = 0;
					else if(abs(b16(pix1_16[x]) - b16(pix2_16[x])) > bluefuzzi)
						check = 0;
			
					if(check)
					{
						pix2_16[x] = rgbtocolor16(redbg, greenbg, bluebg);
						count++;
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
					check = 1;

					if(abs(r24(pix1_24[x]) - r24(pix2_24[x])) > redfuzzi)
						check = 0;
					else if(abs(g24(pix1_24[x]) - g24(pix2_24[x])) > greenfuzzi)
						check = 0;
					else if(abs(b24(pix1_24[x]) - b24(pix2_24[x])) > bluefuzzi)
						check = 0;

					if(check)
					{
						pix1_24[x] = rgbtocolor24(redbg, greenbg, bluebg);
						count++;
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
					check = 1;

					if(abs(r32(pix1_32[x]) - r32(pix2_32[x])) > redfuzzi)
						check = 0;
					else if(abs(g32(pix1_32[x]) - g32(pix2_32[x])) > greenfuzzi)
						check = 0;
					else if(abs(b32(pix1_32[x]) - b32(pix2_32[x])) > bluefuzzi)
						check = 0;

					if(check)
					{
						pix2_32[x] = rgbtocolor32(redbg, greenbg, bluebg);
						count++;
					}
				}
			}
			break;
	}
	// return-value:
	//
	// framestein will send data given in the form "pd_receiver_name=value"
	// back to pd.

	if(ret)sprintf(a.ret, "%s=%d", ret_count, count);
}
