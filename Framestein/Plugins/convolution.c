//
//  convolution	- 5x5 convolution kernel matrix calculation
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
//  usage: convolution <matrix[0]>  <matrix[1]>  <matrix[2]>  <matrix[3]>  <matrix[4]> 
//                     <matrix[5]>  <matrix[6]>  <matrix[7]>  <matrix[8]>  <matrix[9]> 
//                     <matrix[10]> <matrix[11]> <matrix[12]> <matrix[13]> <matrix[14]> 
//                     <matrix[15]> <matrix[16]> <matrix[17]> <matrix[18]> <matrix[19]> 
//                     <matrix[20]> <matrix[21]> <matrix[22]> <matrix[23]> <matrix[24]> 
//                     <scale> <shift>
//
//  KNOWN BUG: produces ugly colors (due to rounding error I guess)
//

#include <stdio.h>
#include <string.h>
#include "plugin.h"

void perform_effect(struct frame f, struct args a)
{
	short x, y;
	long matrix[25], scale, shift;
	pixel16 *p16[5], *c16;
	pixel24 *p24[5], *c24;
	pixel32 *p32[5], *c32;
	long r, g, b;
	char *t;

	// get params
	if(!a.s) return;
	matrix[0] = atoi(a.s);
	if(!(t = strstr(a.s, " "))) return;
	matrix[1] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[2] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[3] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[4] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[5] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[6] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[7] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[8] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[9] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[10] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[11] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[12] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[13] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[14] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[15] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[16] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[17] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[18] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[19] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[20] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[21] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[22] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[23] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	matrix[24] = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	scale = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	shift = atoi(t+1);

	if(scale == 0) scale = 1;

	printf("convolution: matrix %d %d %d %d %d\n", matrix[0], matrix[1], matrix[2], matrix[3], matrix[4]);
	printf("                    %d %d %d %d %d\n", matrix[5], matrix[6], matrix[7], matrix[8], matrix[9]);
	printf("                    %d %d %d %d %d\n", matrix[10], matrix[11], matrix[12], matrix[13], matrix[14]);
	printf("                    %d %d %d %d %d\n", matrix[15], matrix[16], matrix[17], matrix[18], matrix[19]);
	printf("                    %d %d %d %d %d\n", matrix[20], matrix[21], matrix[22], matrix[23], matrix[24]);
	printf("             scale %d, shift %d\n", scale, shift);

	switch(f.pixelformat)
	{
		case 16:
			for(y = 0; y < f.height; y++)
			{
				if(y > 5)	// we have read all 5 lines needed for calculation
				{
					c16 = scanline16(f, y - 3);	// this will become our newly calculated line
					for(x = 2; x < f.width - 2; x++)
					{
						r = (r16(p16[(y-5)%5][x-2])*matrix[0]  + r16(p16[(y-5)%5][x-1])*matrix[1]  + r16(p16[(y-5)%5][x])*matrix[2]  + r16(p16[(y-5)%5][x+1])*matrix[3]  + r16(p16[(y-5)%5][x+2])*matrix[4]  +
						     r16(p16[(y-4)%5][x-2])*matrix[5]  + r16(p16[(y-4)%5][x-1])*matrix[6]  + r16(p16[(y-3)%5][x])*matrix[7]  + r16(p16[(y-4)%5][x+1])*matrix[8]  + r16(p16[(y-4)%5][x+2])*matrix[9]  +
						     r16(p16[(y-3)%5][x-2])*matrix[10] + r16(p16[(y-3)%5][x-1])*matrix[11] + r16(p16[(y-3)%5][x])*matrix[12] + r16(p16[(y-3)%5][x+1])*matrix[13] + r16(p16[(y-3)%5][x+2])*matrix[14] +
						     r16(p16[(y-2)%5][x-2])*matrix[15] + r16(p16[(y-2)%5][x-1])*matrix[16] + r16(p16[(y-2)%5][x])*matrix[17] + r16(p16[(y-2)%5][x+1])*matrix[18] + r16(p16[(y-2)%5][x+2])*matrix[19] +
						     r16(p16[(y-1)%5][x-2])*matrix[20] + r16(p16[(y-1)%5][x-1])*matrix[21] + r16(p16[(y-1)%5][x])*matrix[22] + r16(p16[(y-1)%5][x+1])*matrix[23] + r16(p16[(y-1)%5][x+2])*matrix[24])/scale - shift;
						g = (g16(p16[(y-5)%5][x-2])*matrix[0]  + g16(p16[(y-5)%5][x-1])*matrix[1]  + g16(p16[(y-5)%5][x])*matrix[2]  + g16(p16[(y-5)%5][x+1])*matrix[3]  + g16(p16[(y-5)%5][x+2])*matrix[4]  +
						     g16(p16[(y-4)%5][x-2])*matrix[5]  + g16(p16[(y-4)%5][x-1])*matrix[6]  + g16(p16[(y-3)%5][x])*matrix[7]  + g16(p16[(y-4)%5][x+1])*matrix[8]  + g16(p16[(y-4)%5][x+2])*matrix[9]  +
						     g16(p16[(y-3)%5][x-2])*matrix[10] + g16(p16[(y-3)%5][x-1])*matrix[11] + g16(p16[(y-3)%5][x])*matrix[12] + g16(p16[(y-3)%5][x+1])*matrix[13] + g16(p16[(y-3)%5][x+2])*matrix[14] +
						     g16(p16[(y-2)%5][x-2])*matrix[15] + g16(p16[(y-2)%5][x-1])*matrix[16] + g16(p16[(y-2)%5][x])*matrix[17] + g16(p16[(y-2)%5][x+1])*matrix[18] + g16(p16[(y-2)%5][x+2])*matrix[19] +
						     g16(p16[(y-1)%5][x-2])*matrix[20] + g16(p16[(y-1)%5][x-1])*matrix[21] + g16(p16[(y-1)%5][x])*matrix[22] + g16(p16[(y-1)%5][x+1])*matrix[23] + g16(p16[(y-1)%5][x+2])*matrix[24])/scale - shift;
						b = (b16(p16[(y-5)%5][x-2])*matrix[0]  + b16(p16[(y-5)%5][x-1])*matrix[1]  + b16(p16[(y-5)%5][x])*matrix[2]  + b16(p16[(y-5)%5][x+1])*matrix[3]  + b16(p16[(y-5)%5][x+2])*matrix[4]  +
						     b16(p16[(y-4)%5][x-2])*matrix[5]  + b16(p16[(y-4)%5][x-1])*matrix[6]  + b16(p16[(y-3)%5][x])*matrix[7]  + b16(p16[(y-4)%5][x+1])*matrix[8]  + b16(p16[(y-4)%5][x+2])*matrix[9]  +
						     b16(p16[(y-3)%5][x-2])*matrix[10] + b16(p16[(y-3)%5][x-1])*matrix[11] + b16(p16[(y-3)%5][x])*matrix[12] + b16(p16[(y-3)%5][x+1])*matrix[13] + b16(p16[(y-3)%5][x+2])*matrix[14] +
						     b16(p16[(y-2)%5][x-2])*matrix[15] + b16(p16[(y-2)%5][x-1])*matrix[16] + b16(p16[(y-2)%5][x])*matrix[17] + b16(p16[(y-2)%5][x+1])*matrix[18] + b16(p16[(y-2)%5][x+2])*matrix[19] +
						     b16(p16[(y-1)%5][x-2])*matrix[20] + b16(p16[(y-1)%5][x-1])*matrix[21] + b16(p16[(y-1)%5][x])*matrix[22] + b16(p16[(y-1)%5][x+1])*matrix[23] + b16(p16[(y-1)%5][x+2])*matrix[24])/scale - shift;
						c16[x] = rgbtocolor16(klamp255(r), klamp255(g), klamp255(b));
					}
				}
				// read in lines needed for calculation
				p16[y % 5] = scanline16(f, y);
			}
			break;
		case 24:
			for(y=0; y<f.height; y++)
			{
				if(y > 5)	// we have read all 5 lines needed for calculation
				{
					c24 = scanline24(f, y - 3);	// this will become our newly calculated line
					for(x = 2; x < f.width - 2; x++)
					{
						r = (r24(p24[(y-5)%5][x-2])*matrix[0]  + r24(p24[(y-5)%5][x-1])*matrix[1]  + r24(p24[(y-5)%5][x])*matrix[2]  + r24(p24[(y-5)%5][x+1])*matrix[3]  + r24(p24[(y-5)%5][x+2])*matrix[4]  +
						     r24(p24[(y-4)%5][x-2])*matrix[5]  + r24(p24[(y-4)%5][x-1])*matrix[6]  + r24(p24[(y-3)%5][x])*matrix[7]  + r24(p24[(y-4)%5][x+1])*matrix[8]  + r24(p24[(y-4)%5][x+2])*matrix[9]  +
						     r24(p24[(y-3)%5][x-2])*matrix[10] + r24(p24[(y-3)%5][x-1])*matrix[11] + r24(p24[(y-3)%5][x])*matrix[12] + r24(p24[(y-3)%5][x+1])*matrix[13] + r24(p24[(y-3)%5][x+2])*matrix[14] +
						     r24(p24[(y-2)%5][x-2])*matrix[15] + r24(p24[(y-2)%5][x-1])*matrix[16] + r24(p24[(y-2)%5][x])*matrix[17] + r24(p24[(y-2)%5][x+1])*matrix[18] + r24(p24[(y-2)%5][x+2])*matrix[19] +
						     r24(p24[(y-1)%5][x-2])*matrix[20] + r24(p24[(y-1)%5][x-1])*matrix[21] + r24(p24[(y-1)%5][x])*matrix[22] + r24(p24[(y-1)%5][x+1])*matrix[23] + r24(p24[(y-1)%5][x+2])*matrix[24])/scale - shift;
						g = (g24(p24[(y-5)%5][x-2])*matrix[0]  + g24(p24[(y-5)%5][x-1])*matrix[1]  + g24(p24[(y-5)%5][x])*matrix[2]  + g24(p24[(y-5)%5][x+1])*matrix[3]  + g24(p24[(y-5)%5][x+2])*matrix[4]  +
						     g24(p24[(y-4)%5][x-2])*matrix[5]  + g24(p24[(y-4)%5][x-1])*matrix[6]  + g24(p24[(y-3)%5][x])*matrix[7]  + g24(p24[(y-4)%5][x+1])*matrix[8]  + g24(p24[(y-4)%5][x+2])*matrix[9]  +
						     g24(p24[(y-3)%5][x-2])*matrix[10] + g24(p24[(y-3)%5][x-1])*matrix[11] + g24(p24[(y-3)%5][x])*matrix[12] + g24(p24[(y-3)%5][x+1])*matrix[13] + g24(p24[(y-3)%5][x+2])*matrix[14] +
						     g24(p24[(y-2)%5][x-2])*matrix[15] + g24(p24[(y-2)%5][x-1])*matrix[16] + g24(p24[(y-2)%5][x])*matrix[17] + g24(p24[(y-2)%5][x+1])*matrix[18] + g24(p24[(y-2)%5][x+2])*matrix[19] +
						     g24(p24[(y-1)%5][x-2])*matrix[20] + g24(p24[(y-1)%5][x-1])*matrix[21] + g24(p24[(y-1)%5][x])*matrix[22] + g24(p24[(y-1)%5][x+1])*matrix[23] + g24(p24[(y-1)%5][x+2])*matrix[24])/scale - shift;
						b = (b24(p24[(y-5)%5][x-2])*matrix[0]  + b24(p24[(y-5)%5][x-1])*matrix[1]  + b24(p24[(y-5)%5][x])*matrix[2]  + b24(p24[(y-5)%5][x+1])*matrix[3]  + b24(p24[(y-5)%5][x+2])*matrix[4]  +
						     b24(p24[(y-4)%5][x-2])*matrix[5]  + b24(p24[(y-4)%5][x-1])*matrix[6]  + b24(p24[(y-3)%5][x])*matrix[7]  + b24(p24[(y-4)%5][x+1])*matrix[8]  + b24(p24[(y-4)%5][x+2])*matrix[9]  +
						     b24(p24[(y-3)%5][x-2])*matrix[10] + b24(p24[(y-3)%5][x-1])*matrix[11] + b24(p24[(y-3)%5][x])*matrix[12] + b24(p24[(y-3)%5][x+1])*matrix[13] + b24(p24[(y-3)%5][x+2])*matrix[14] +
						     b24(p24[(y-2)%5][x-2])*matrix[15] + b24(p24[(y-2)%5][x-1])*matrix[16] + b24(p24[(y-2)%5][x])*matrix[17] + b24(p24[(y-2)%5][x+1])*matrix[18] + b24(p24[(y-2)%5][x+2])*matrix[19] +
						     b24(p24[(y-1)%5][x-2])*matrix[20] + b24(p24[(y-1)%5][x-1])*matrix[21] + b24(p24[(y-1)%5][x])*matrix[22] + b24(p24[(y-1)%5][x+1])*matrix[23] + b24(p24[(y-1)%5][x+2])*matrix[24])/scale - shift;
						c24[x] = rgbtocolor24(klamp255(r), klamp255(g), klamp255(b));
					}
				}
				// read in lines needed for calculation
				p24[y % 5] = scanline24(f, y);
			}
			break;
		case 32:
			for(y=0; y<f.height; y++)
			{
				if(y > 5)	// we have read all 5 lines needed for calculation
				{
					c32 = scanline32(f, y - 3);	// this will become our newly calculated line
					for(x = 2; x < f.width - 2; x++)
					{
						r = (r32(p32[(y-5)%5][x-2])*matrix[0]  + r32(p32[(y-5)%5][x-1])*matrix[1]  + r32(p32[(y-5)%5][x])*matrix[2]  + r32(p32[(y-5)%5][x+1])*matrix[3]  + r32(p32[(y-5)%5][x+2])*matrix[4]  +
						     r32(p32[(y-4)%5][x-2])*matrix[5]  + r32(p32[(y-4)%5][x-1])*matrix[6]  + r32(p32[(y-3)%5][x])*matrix[7]  + r32(p32[(y-4)%5][x+1])*matrix[8]  + r32(p32[(y-4)%5][x+2])*matrix[9]  +
						     r32(p32[(y-3)%5][x-2])*matrix[10] + r32(p32[(y-3)%5][x-1])*matrix[11] + r32(p32[(y-3)%5][x])*matrix[12] + r32(p32[(y-3)%5][x+1])*matrix[13] + r32(p32[(y-3)%5][x+2])*matrix[14] +
						     r32(p32[(y-2)%5][x-2])*matrix[15] + r32(p32[(y-2)%5][x-1])*matrix[16] + r32(p32[(y-2)%5][x])*matrix[17] + r32(p32[(y-2)%5][x+1])*matrix[18] + r32(p32[(y-2)%5][x+2])*matrix[19] +
						     r32(p32[(y-1)%5][x-2])*matrix[20] + r32(p32[(y-1)%5][x-1])*matrix[21] + r32(p32[(y-1)%5][x])*matrix[22] + r32(p32[(y-1)%5][x+1])*matrix[23] + r32(p32[(y-1)%5][x+2])*matrix[24])/scale - shift;
						g = (g32(p32[(y-5)%5][x-2])*matrix[0]  + g32(p32[(y-5)%5][x-1])*matrix[1]  + g32(p32[(y-5)%5][x])*matrix[2]  + g32(p32[(y-5)%5][x+1])*matrix[3]  + g32(p32[(y-5)%5][x+2])*matrix[4]  +
						     g32(p32[(y-4)%5][x-2])*matrix[5]  + g32(p32[(y-4)%5][x-1])*matrix[6]  + g32(p32[(y-3)%5][x])*matrix[7]  + g32(p32[(y-4)%5][x+1])*matrix[8]  + g32(p32[(y-4)%5][x+2])*matrix[9]  +
						     g32(p32[(y-3)%5][x-2])*matrix[10] + g32(p32[(y-3)%5][x-1])*matrix[11] + g32(p32[(y-3)%5][x])*matrix[12] + g32(p32[(y-3)%5][x+1])*matrix[13] + g32(p32[(y-3)%5][x+2])*matrix[14] +
						     g32(p32[(y-2)%5][x-2])*matrix[15] + g32(p32[(y-2)%5][x-1])*matrix[16] + g32(p32[(y-2)%5][x])*matrix[17] + g32(p32[(y-2)%5][x+1])*matrix[18] + g32(p32[(y-2)%5][x+2])*matrix[19] +
						     g32(p32[(y-1)%5][x-2])*matrix[20] + g32(p32[(y-1)%5][x-1])*matrix[21] + g32(p32[(y-1)%5][x])*matrix[22] + g32(p32[(y-1)%5][x+1])*matrix[23] + g32(p32[(y-1)%5][x+2])*matrix[24])/scale - shift;
						b = (b32(p32[(y-5)%5][x-2])*matrix[0]  + b32(p32[(y-5)%5][x-1])*matrix[1]  + b32(p32[(y-5)%5][x])*matrix[2]  + b32(p32[(y-5)%5][x+1])*matrix[3]  + b32(p32[(y-5)%5][x+2])*matrix[4]  +
						     b32(p32[(y-4)%5][x-2])*matrix[5]  + b32(p32[(y-4)%5][x-1])*matrix[6]  + b32(p32[(y-3)%5][x])*matrix[7]  + b32(p32[(y-4)%5][x+1])*matrix[8]  + b32(p32[(y-4)%5][x+2])*matrix[9]  +
						     b32(p32[(y-3)%5][x-2])*matrix[10] + b32(p32[(y-3)%5][x-1])*matrix[11] + b32(p32[(y-3)%5][x])*matrix[12] + b32(p32[(y-3)%5][x+1])*matrix[13] + b32(p32[(y-3)%5][x+2])*matrix[14] +
						     b32(p32[(y-2)%5][x-2])*matrix[15] + b32(p32[(y-2)%5][x-1])*matrix[16] + b32(p32[(y-2)%5][x])*matrix[17] + b32(p32[(y-2)%5][x+1])*matrix[18] + b32(p32[(y-2)%5][x+2])*matrix[19] +
						     b32(p32[(y-1)%5][x-2])*matrix[20] + b32(p32[(y-1)%5][x-1])*matrix[21] + b32(p32[(y-1)%5][x])*matrix[22] + b32(p32[(y-1)%5][x+1])*matrix[23] + b32(p32[(y-1)%5][x+2])*matrix[24])/scale - shift;
						c32[x] = rgbtocolor32(klamp255(r), klamp255(g), klamp255(b));
					}
				}
				// read in lines needed for calculation
				p32[y % 5] = scanline32(f, y);
			}
			break;
	}
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	printf("Using convolution as copy operation does nothing!\n");
}
