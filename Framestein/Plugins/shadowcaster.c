//
//  shadowcaster - porduce a cutout with shadow
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
//  usage: shadowcaster <x_c> <y_c> <w_c> <h_c> <shadow width> <red intensity> <green intensity> <blue intensity>
//

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "plugin.h"
#include "tools.h"

#define MAX_SHADOW  128	/* maximum shadow width in px */

void perform_effect(struct frame f, struct args a)
{
	short x, y;
	short x_c, y_c, w_c, h_c;	// position and dimension of cutout
	byte pixelsize = f.pixelformat/8;
	pixel16 *pix16[MAX_SHADOW], *c16;
	pixel24 *pix24[MAX_SHADOW], *c24;
	pixel32 *pix32[MAX_SHADOW], *c32;
	long r, g, b;
	byte shadow, red, green, blue;
	char *t;

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
	shadow = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	red = klamp255(atoi(t+1));
	if(!(t = strstr(t+1, " "))) return;
	green = klamp255(atoi(t+1));
	if(!(t = strstr(t+1, " "))) return;
	blue = klamp255(atoi(t+1));
	// check image boundaries
	if(shadow > MAX_SHADOW)shadow = MAX_SHADOW;
	if(shadow > x_c)shadow = x_c;
	if(x_c + w_c + shadow > f.width) x_c -= ((x_c + w_c + shadow) - f.width);
	if(y_c + h_c + shadow > f.height) y_c -= ((y_c + h_c + shadow) - f.height);

	printf("shadowcaster x%d y%d w%d h%d s%d - r%d g%d b%d\n", 
		    x_c, y_c, w_c, h_c, shadow, red, green, blue);

	// allocate memory for cutout
	for(y = 0; y < shadow; y++)
	{
		switch(f.pixelformat)
		{
			case 16:
				pix16[y] = malloc(f.width*pixelsize);
				if(!pix16[y])return;
				break;
			case 24:
				pix24[y] = malloc(f.width*pixelsize);
				if(!pix24[y])return;
				break;
			case 32:
				pix32[y] = malloc(f.width*pixelsize);
				if(!pix32[y])return;
				break;
		}
	}

	switch(f.pixelformat)
	{
		case 16:
			for(y = y_c - shadow - 1; y < y_c + h_c + shadow; y++) 
			{
				c16 = scanline16(f, y);
				memcpy(pix16[y % shadow], c16, f.width*pixelsize);	// make copy of original image
				if(y > y_c)	// only process relevant lines
				{
					for(x = x_c; x < x_c + w_c + shadow; x++)
					{
						if(x >= x_c)
							if((x > x_c + shadow - 1) && (y > y_c + shadow - 1))	// no shadow, just move cutout image
							{
								r = r16(pix16[(y-shadow) % shadow][x-shadow]);
								g = g16(pix16[(y-shadow) % shadow][x-shadow]);
								b = b16(pix16[(y-shadow) % shadow][x-shadow]);
							}
							else if((x < x_c + shadow) && (y > y_c + shadow-1))	// apply left shadow
							{
								r = (scl(x-x_c,0,shadow-1,red,255) * r16(pix16[(y-shadow) % shadow][x-shadow]))/255;
								g = (scl(x-x_c,0,shadow-1,green,255) * g16(pix16[(y-shadow) % shadow][x-shadow]))/255;
								b = (scl(x-x_c,0,shadow-1,blue,255) * b16(pix16[(y-shadow) % shadow][x-shadow]))/255;
							}
							else if((x > x_c + shadow-1) && (y < y_c + shadow))	// apply top shadow
							{
								r = (scl(y-y_c,0,shadow-1,red,255) * r16(pix16[(y-shadow) % shadow][x-shadow]))/255;
								g = (scl(y-y_c,0,shadow-1,green,255) * g16(pix16[(y-shadow) % shadow][x-shadow]))/255;
								b = (scl(y-y_c,0,shadow-1,blue,255) * b16(pix16[(y-shadow) % shadow][x-shadow]))/255;
							}
							else	// apply top and left shadow together
							{
								r = (min(scl(x-x_c,0,shadow-1,red,255),scl(y-y_c,0,shadow-1,red,255)) * r16(pix16[(y-shadow) % shadow][x-shadow]))/255;
								g = (min(scl(x-x_c,0,shadow-1,green,255),scl(y-y_c,0,shadow-1,green,255)) * g16(pix16[(y-shadow) % shadow][x-shadow]))/255;
								b = (min(scl(x-x_c,0,shadow-1,blue,255),scl(y-y_c,0,shadow-1,blue,255)) * b16(pix16[(y-shadow) % shadow][x-shadow]))/255;
							}
							c16[x] = rgbtocolor16(klamp255(r), klamp255(g), klamp255(b));
					}
				}
			}
			break;
		case 24:
			for(y = y_c - shadow - 1; y < y_c + h_c + shadow; y++) 
			{
				c24 = scanline24(f, y);
				memcpy(pix24[y % shadow], c24, f.width*pixelsize);	// make copy of original image
				if(y > y_c)	// only process relevant lines
				{
					for(x = x_c; x < x_c + w_c + shadow; x++)
					{
						if(x >= x_c)
							if((x > x_c + shadow - 1) && (y > y_c + shadow - 1))	// no shadow, just move cutout image
							{
								r = r24(pix24[(y-shadow) % shadow][x-shadow]);
								g = g24(pix24[(y-shadow) % shadow][x-shadow]);
								b = b24(pix24[(y-shadow) % shadow][x-shadow]);
							}
							else if((x < x_c + shadow) && (y > y_c + shadow-1))	// apply left shadow
							{
								r = (scl(x-x_c,0,shadow-1,red,255) * r24(pix24[(y-shadow) % shadow][x-shadow]))/255;
								g = (scl(x-x_c,0,shadow-1,green,255) * g24(pix24[(y-shadow) % shadow][x-shadow]))/255;
								b = (scl(x-x_c,0,shadow-1,blue,255) * b24(pix24[(y-shadow) % shadow][x-shadow]))/255;
							}
							else if((x > x_c + shadow-1) && (y < y_c + shadow))	// apply top shadow
							{
								r = (scl(y-y_c,0,shadow-1,red,255) * r24(pix24[(y-shadow) % shadow][x-shadow]))/255;
								g = (scl(y-y_c,0,shadow-1,green,255) * g24(pix24[(y-shadow) % shadow][x-shadow]))/255;
								b = (scl(y-y_c,0,shadow-1,blue,255) * b24(pix24[(y-shadow) % shadow][x-shadow]))/255;
							}
							else	// apply top and left shadow together
							{
								r = (min(scl(x-x_c,0,shadow-1,red,255),scl(y-y_c,0,shadow-1,red,255)) * r24(pix24[(y-shadow) % shadow][x-shadow]))/255;
								g = (min(scl(x-x_c,0,shadow-1,green,255),scl(y-y_c,0,shadow-1,green,255)) * g24(pix24[(y-shadow) % shadow][x-shadow]))/255;
								b = (min(scl(x-x_c,0,shadow-1,blue,255),scl(y-y_c,0,shadow-1,blue,255)) * b24(pix24[(y-shadow) % shadow][x-shadow]))/255;
							}
							c24[x] = rgbtocolor24(klamp255(r), klamp255(g), klamp255(b));
					}
				}
			}
			break;
		case 32:
			for(y = y_c - shadow - 1; y < y_c + h_c + shadow; y++) 
			{
				c32 = scanline32(f, y);
				memcpy(pix32[y % shadow], c32, f.width*pixelsize);	// make copy of original image
				if(y > y_c)	// only process relevant lines
				{
					for(x = x_c; x < x_c + w_c + shadow; x++)
					{
						if(x >= x_c)
							if((x > x_c + shadow - 1) && (y > y_c + shadow - 1))	// no shadow, just move cutout image
							{
								r = r32(pix32[(y-shadow) % shadow][x-shadow]);
								g = g32(pix32[(y-shadow) % shadow][x-shadow]);
								b = b32(pix32[(y-shadow) % shadow][x-shadow]);
							}
							else if((x < x_c + shadow) && (y > y_c + shadow-1))	// apply left shadow
							{
								r = (scl(x-x_c,0,shadow-1,red,255) * r32(pix32[(y-shadow) % shadow][x-shadow]))/255;
								g = (scl(x-x_c,0,shadow-1,green,255) * g32(pix32[(y-shadow) % shadow][x-shadow]))/255;
								b = (scl(x-x_c,0,shadow-1,blue,255) * b32(pix32[(y-shadow) % shadow][x-shadow]))/255;
							}
							else if((x > x_c + shadow-1) && (y < y_c + shadow))	// apply top shadow
							{
								r = (scl(y-y_c,0,shadow-1,red,255) * r32(pix32[(y-shadow) % shadow][x-shadow]))/255;
								g = (scl(y-y_c,0,shadow-1,green,255) * g32(pix32[(y-shadow) % shadow][x-shadow]))/255;
								b = (scl(y-y_c,0,shadow-1,blue,255) * b32(pix32[(y-shadow) % shadow][x-shadow]))/255;
							}
							else	// apply top and left shadow together
							{
								r = (min(scl(x-x_c,0,shadow-1,red,255),scl(y-y_c,0,shadow-1,red,255)) * r32(pix32[(y-shadow) % shadow][x-shadow]))/255;
								g = (min(scl(x-x_c,0,shadow-1,green,255),scl(y-y_c,0,shadow-1,green,255)) * g32(pix32[(y-shadow) % shadow][x-shadow]))/255;
								b = (min(scl(x-x_c,0,shadow-1,blue,255),scl(y-y_c,0,shadow-1,blue,255)) * b32(pix32[(y-shadow) % shadow][x-shadow]))/255;
							}
							c32[x] = rgbtocolor32(klamp255(r), klamp255(g), klamp255(b));
					}
				}
			}
			break;
	}

	// free memory for cutout
	for(y = 0; y < shadow; y++)
	{
		switch(f.pixelformat)
		{
			case 16:
				free(pix16[y]);
				break;
			case 24:
				free(pix24[y]);
				break;
			case 32:
				free(pix32[y]);
				break;
		}
	}
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	short x, y, w, h;
	short x_c, y_c, w_c, h_c;	// position and dimension of cutout
	byte pixelsize = f1.pixelformat/8;
	pixel16 *pix16[MAX_SHADOW], *c16;
	pixel24 *pix24[MAX_SHADOW], *c24;
	pixel32 *pix32[MAX_SHADOW], *c32;
	long r, g, b;
	byte shadow, red, green, blue;
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
	shadow = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	red = klamp255(atoi(t+1));
	if(!(t = strstr(t+1, " "))) return;
	green = klamp255(atoi(t+1));
	if(!(t = strstr(t+1, " "))) return;
	blue = klamp255(atoi(t+1));

	// check image boundaries
	if(shadow > MAX_SHADOW)shadow = MAX_SHADOW;
	if(shadow > x_c)shadow = x_c;
	if(x_c + w_c + shadow > w) x_c -= ((x_c + w_c + shadow) - w);
	if(y_c + h_c + shadow > h) y_c -= ((y_c + h_c + shadow) - h);

	printf("shadowcaster x%d y%d w%d h%d s%d - r%d g%d b%d\n", 
		    x_c, y_c, w_c, h_c, shadow, red, green, blue);

	switch(f1.pixelformat)
	{
		case 16:
			for(y = y_c - shadow - 1; y < y_c + h_c + shadow; y++) 
			{
				pix16[y % shadow] = scanline16(f1, y);	// read in lines we use later on
				c16 = scanline16(f2, y);
				if(y > y_c)	// only process relevant lines
				{
					for(x = x_c; x < x_c + w_c + shadow; x++)
					{
						if(x >= x_c)
							if((x > x_c + shadow - 1) && (y > y_c + shadow - 1))	// no shadow, just move cutout image
							{
								r = r16(pix16[(y-shadow) % shadow][x-shadow]);
								g = g16(pix16[(y-shadow) % shadow][x-shadow]);
								b = b16(pix16[(y-shadow) % shadow][x-shadow]);
							}
							else if((x < x_c + shadow) && (y > y_c + shadow-1))	// apply left shadow
							{
								r = ((scl(x-x_c,0,shadow-1,red,255) * r16(pix16[(y-shadow) % shadow][x-shadow]))/255) + 
									(((255-scl(x-x_c,0,shadow-1,red,255)) * r16(c16[x]))/255);
								g = ((scl(x-x_c,0,shadow-1,green,255) * g16(pix16[(y-shadow) % shadow][x-shadow]))/255) + 
									(((255-scl(x-x_c,0,shadow-1,green,255)) * g16(c16[x]))/255);
								b = ((scl(x-x_c,0,shadow-1,blue,255) * b16(pix16[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-scl(x-x_c,0,shadow-1,blue,255)) * b16(c16[x]))/255);
							}
							else if((x > x_c + shadow-1) && (y < y_c + shadow))	// apply top shadow
							{
								r = ((scl(y-y_c,0,shadow-1,red,255) * r16(pix16[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-scl(y-y_c,0,shadow-1,red,255)) * r16(c16[x]))/255);
								g = ((scl(y-y_c,0,shadow-1,green,255) * g16(pix16[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-scl(y-y_c,0,shadow-1,green,255)) * g16(c16[x]))/255);
								b = ((scl(y-y_c,0,shadow-1,blue,255) * b16(pix16[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-scl(y-y_c,0,shadow-1,blue,255)) * b16(c16[x]))/255);
							}
							else	// apply top and left shadow together
							{
								r = ((min(scl(x-x_c,0,shadow-1,red,255),scl(y-y_c,0,shadow-1,red,255)) * r16(pix16[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-min(scl(x-x_c,0,shadow-1,red,255),scl(y-y_c,0,shadow-1,red,255))) * r16(c16[x]))/255);
								g = ((min(scl(x-x_c,0,shadow-1,green,255),scl(y-y_c,0,shadow-1,green,255)) * g16(pix16[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-min(scl(x-x_c,0,shadow-1,green,255),scl(y-y_c,0,shadow-1,green,255))) * g16(c16[x]))/255);
								b = ((min(scl(x-x_c,0,shadow-1,blue,255),scl(y-y_c,0,shadow-1,blue,255)) * b16(pix16[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-min(scl(x-x_c,0,shadow-1,blue,255),scl(y-y_c,0,shadow-1,blue,255))) * b16(c16[x]))/255);
							}
							c16[x] = rgbtocolor16(klamp255(r), klamp255(g), klamp255(b));
					}
				}
			}
			break;
		case 24:
			for(y = y_c - shadow - 1; y < y_c + h_c + shadow; y++) 
			{
				pix24[y % shadow] = scanline24(f1, y);	// read in lines we use later on
				c24 = scanline24(f2, y);
				if(y > y_c)	// only process relevant lines
				{
					for(x = x_c; x < x_c + w_c + shadow; x++)
					{
						if(x >= x_c)
							if((x > x_c + shadow - 1) && (y > y_c + shadow - 1))	// no shadow, just move cutout image
							{
								r = r24(pix24[(y-shadow) % shadow][x-shadow]);
								g = g24(pix24[(y-shadow) % shadow][x-shadow]);
								b = b24(pix24[(y-shadow) % shadow][x-shadow]);
							}
							else if((x < x_c + shadow) && (y > y_c + shadow-1))	// apply left shadow
							{
								r = ((scl(x-x_c,0,shadow-1,red,255) * r24(pix24[(y-shadow) % shadow][x-shadow]))/255) + 
									(((255-scl(x-x_c,0,shadow-1,red,255)) * r24(c24[x]))/255);
								g = ((scl(x-x_c,0,shadow-1,green,255) * g24(pix24[(y-shadow) % shadow][x-shadow]))/255) + 
									(((255-scl(x-x_c,0,shadow-1,green,255)) * g24(c24[x]))/255);
								b = ((scl(x-x_c,0,shadow-1,blue,255) * b24(pix24[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-scl(x-x_c,0,shadow-1,blue,255)) * b24(c24[x]))/255);
							}
							else if((x > x_c + shadow-1) && (y < y_c + shadow))	// apply top shadow
							{
								r = ((scl(y-y_c,0,shadow-1,red,255) * r24(pix24[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-scl(y-y_c,0,shadow-1,red,255)) * r24(c24[x]))/255);
								g = ((scl(y-y_c,0,shadow-1,green,255) * g24(pix24[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-scl(y-y_c,0,shadow-1,green,255)) * g24(c24[x]))/255);
								b = ((scl(y-y_c,0,shadow-1,blue,255) * b24(pix24[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-scl(y-y_c,0,shadow-1,blue,255)) * b24(c24[x]))/255);
							}
							else	// apply top and left shadow together
							{
								r = ((min(scl(x-x_c,0,shadow-1,red,255),scl(y-y_c,0,shadow-1,red,255)) * r24(pix24[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-min(scl(x-x_c,0,shadow-1,red,255),scl(y-y_c,0,shadow-1,red,255))) * r24(c24[x]))/255);
								g = ((min(scl(x-x_c,0,shadow-1,green,255),scl(y-y_c,0,shadow-1,green,255)) * g24(pix24[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-min(scl(x-x_c,0,shadow-1,green,255),scl(y-y_c,0,shadow-1,green,255))) * g24(c24[x]))/255);
								b = ((min(scl(x-x_c,0,shadow-1,blue,255),scl(y-y_c,0,shadow-1,blue,255)) * b24(pix24[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-min(scl(x-x_c,0,shadow-1,blue,255),scl(y-y_c,0,shadow-1,blue,255))) * b24(c24[x]))/255);
							}
							c24[x] = rgbtocolor24(klamp255(r), klamp255(g), klamp255(b));
					}
				}
			}
			break;
		case 32:
			for(y = y_c - shadow - 1; y < y_c + h_c + shadow; y++) 
			{
				pix32[y % shadow] = scanline32(f1, y);	// read in lines we use later on
				c32 = scanline32(f2, y);
				if(y > y_c)	// only process relevant lines
				{
					for(x = x_c; x < x_c + w_c + shadow; x++)
					{
						if(x >= x_c)
							if((x > x_c + shadow - 1) && (y > y_c + shadow - 1))	// no shadow, just move cutout image
							{
								r = r32(pix32[(y-shadow) % shadow][x-shadow]);
								g = g32(pix32[(y-shadow) % shadow][x-shadow]);
								b = b32(pix32[(y-shadow) % shadow][x-shadow]);
							}
							else if((x < x_c + shadow) && (y > y_c + shadow-1))	// apply left shadow
							{
								r = ((scl(x-x_c,0,shadow-1,red,255) * r32(pix32[(y-shadow) % shadow][x-shadow]))/255) + 
									(((255-scl(x-x_c,0,shadow-1,red,255)) * r32(c32[x]))/255);
								g = ((scl(x-x_c,0,shadow-1,green,255) * g32(pix32[(y-shadow) % shadow][x-shadow]))/255) + 
									(((255-scl(x-x_c,0,shadow-1,green,255)) * g32(c32[x]))/255);
								b = ((scl(x-x_c,0,shadow-1,blue,255) * b32(pix32[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-scl(x-x_c,0,shadow-1,blue,255)) * b32(c32[x]))/255);
							}
							else if((x > x_c + shadow-1) && (y < y_c + shadow))	// apply top shadow
							{
								r = ((scl(y-y_c,0,shadow-1,red,255) * r32(pix32[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-scl(y-y_c,0,shadow-1,red,255)) * r32(c32[x]))/255);
								g = ((scl(y-y_c,0,shadow-1,green,255) * g32(pix32[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-scl(y-y_c,0,shadow-1,green,255)) * g32(c32[x]))/255);
								b = ((scl(y-y_c,0,shadow-1,blue,255) * b32(pix32[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-scl(y-y_c,0,shadow-1,blue,255)) * b32(c32[x]))/255);
							}
							else	// apply top and left shadow together
							{
								r = ((min(scl(x-x_c,0,shadow-1,red,255),scl(y-y_c,0,shadow-1,red,255)) * r32(pix32[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-min(scl(x-x_c,0,shadow-1,red,255),scl(y-y_c,0,shadow-1,red,255))) * r32(c32[x]))/255);
								g = ((min(scl(x-x_c,0,shadow-1,green,255),scl(y-y_c,0,shadow-1,green,255)) * g32(pix32[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-min(scl(x-x_c,0,shadow-1,green,255),scl(y-y_c,0,shadow-1,green,255))) * g32(c32[x]))/255);
								b = ((min(scl(x-x_c,0,shadow-1,blue,255),scl(y-y_c,0,shadow-1,blue,255)) * b32(pix32[(y-shadow) % shadow][x-shadow]))/255) +
									(((255-min(scl(x-x_c,0,shadow-1,blue,255),scl(y-y_c,0,shadow-1,blue,255))) * b32(c32[x]))/255);
							}
							c32[x] = rgbtocolor32(klamp255(r), klamp255(g), klamp255(b));
					}
				}
			}
			break;
	}
}
