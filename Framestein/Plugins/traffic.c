// 242.traffic -- does tristimulus matrix operations on an input image.
//		by r. luke dubois (luke@music.columbia.edu),
//			computer music center, columbia university, 2001.
//
//  Pd / Framestein port by Olaf Matthes <olaf.matthes@gmx.de>, June 2002
//
//	objects and source are provided without warranty of any kind, express or implied.
//

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "plugin.h"

// tristimulus product used for matrix conversions

static void tristimulus_product(float *matrix, byte *input, byte *output)
{
	int f, g;
	for(f = 0; f < 3; f++)
	{
		output[f] = (byte)input[0]*matrix[f*3+0]+
					      input[1]*matrix[f*3+1]+
					      input[2]*matrix[f*3+2]; 
	}
}


void perform_effect(struct frame f, struct args a)
{
	int i, x, y;
	float matrix[9];
	byte bits = f.pixelformat/8;
	byte input[3], output[3];	// the color values
	byte r, g, b;
	pixel16 *c16;
	pixel24 *c24;
	pixel32 *c32;
	char *t;

	// get matrix params
	if(!a.s) return;
	matrix[0] = atof(a.s);
	for(i = 1; i < 9; i++)
	{
		if(!(t = strstr(t+1, " "))) return;
		matrix[i] = atof(t+1);
	}

	printf("traffic: matrix: %d %d %d\n"
		   "                 %d %d %d\n"
		   "                 %d %d %d\n", matrix[0], matrix[1], matrix[2],
		    matrix[3], matrix[4], matrix[5], matrix[6], matrix[7], matrix[8]);

	for(y = 0; y < f.height; y++) 
	{
		for(x = 0; x < f.width; x++)
		{
			switch (f.pixelformat)
			{
				case 16:
					c16 = scanline16(f, y);
					input[0] = r16(c16[x]);
					input[1] = g16(c16[x]);
					input[2] = b16(c16[x]);
					tristimulus_product(matrix,input,output);
					r = output[0]; 
					g = output[1]; 
					b = output[2]; 

					c16[x] = rgbtocolor16(r, g, b);
					break;
				case 24:
					c24 = scanline24(f, y);
					input[0] = r24(c24[x]);
					input[1] = g24(c24[x]);
					input[2] = b24(c24[x]);
					tristimulus_product(matrix,input,output);
					r = output[0]; 
					g = output[1]; 
					b = output[2]; 

					c24[x] = rgbtocolor24(r, g, b);
					break;
				case 32:
					c32 = scanline32(f, y);
					input[0] = r32(c32[x]);
					input[1] = g32(c32[x]);
					input[2] = b32(c32[x]);
					tristimulus_product(matrix,input,output);
					r = output[0]; 
					g = output[1]; 
					b = output[2]; 

					c32[x] = rgbtocolor32(r, g, b);
					break;
			}
		}
	}
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	printf("Using traffic as copy operation does nothing!\n");
}
