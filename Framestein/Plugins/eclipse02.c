// 242.eclipse02 -- does meta-imaging on two input images.
//		by r. luke dubois (luke@music.columbia.edu),
//			computer music center, columbia university, 2001.
//		grid quantization algorithm massively improved by jeremy bernstein, bootsquadresearch.
//
//  Pd / Framestein port by Olaf Matthes <olaf.matthes@gmx.de>, June 2002
//
//	objects and source are provided without warranty of any kind, express or implied.
//
//  usage: eclipse02 <red> <green> <blue> <rows> <columns>
//

#include <stdio.h>
#include <string.h>
#include "plugin.h"

INFO("242.eclipse02 -- does meta-imaging on two input images")

// the process image routines work like this:
// 
// two pointers (src and src2) access the pixels in the input image
// src controls the output of the smaller grids, src2 controls the tinting from
// the upper-left corner of the point in the image that the grid is supposed to represent.
//
// the grid dimensions are computed by the size of the input image and are used to define 
// the size of each pixel in the grid (which is why some row and columns combinations
// leave orphaned pixels on the edges).
// the loop goes like this:
//
// do each row {
//    do each column {
//       get the 'tint pixel' for that frame;
//
//       do the height within each meta-frame {
//          do the width within each meta-frame {
//             output the pixels for each frame, tinting it;
//          }
//       }
//    }
// }
//
//

void perform_effect(struct frame f, struct args a)  // color tints color
{
	printf("Using eclipse02 as effect does nothing!\n");
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	short i, i1, j, j1, r, c, src1[2], w, h;
	short rows, columns, rowstep, colstep, rowoffset, coloffset;
	byte red, green, blue, check, r1, g1, b1;
	byte bits = f1.pixelformat/8;
	byte redpix, greenpix, bluepix;	// the color values
	pixel16 *pix2_16, *pix1_16;
	pixel24 *pix2_24, *pix1_24;
	pixel32 *pix2_32, *pix1_32;
	char *t;

	w = f1.width<f2.width ? f1.width : f2.width;
	h = f1.height<f2.height ? f1.height : f2.height;

	// get r g b and fuzzy params
	if(!a.s) return;
	red = atoi(a.s);
	if(!(t = strstr(a.s, " "))) return;
	green = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	blue = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	rows = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	columns = atoi(t+1);


	rowstep = (h / rows) + .5;
	colstep = (w / columns) + .5;
	// printf("red is %ld green is %ld blue is %ld", red, green, blue);

	if ( rows > (h * .1) )
		rowoffset = rows - (h * .1);
	else
		rowoffset = 1;
    
	if ( columns > (w * .1) )
		coloffset = columns - (w * .1);
	else
		coloffset = 1;
        

	for(r = 0; r <= ( rows + rowoffset ); r++ ) 
	{
		for(i = 0, i1 = ( r * rowstep ); i < h; i += rows, i1++) 
		{
			for( c = 0; c <= columns + coloffset; c++ ) 
			{
				switch(f1.pixelformat)
				{
					case 16:
						src1[0] = c * colstep;
						src1[1] = r * rowstep;
						pix1_16 = scanline16(f1, src1[1]);
						r1 = r16(pix1_16[src1[0]]);
						g1 = g16(pix1_16[src1[0]]);
						b1 = b16(pix1_16[src1[0]]);

						for(j = 0, j1 = c * colstep; j < w; j += columns, j1++)
						{
							if ( j1 >= w )
							goto dink;

							if ( i1 >= h )
							goto yoink;

							pix2_16 = scanline16(f2, i1);
							redpix = r16(pix2_16[j1]) + r1 + red;
							greenpix = g16(pix2_16[j1]) + g1 + green;
							bluepix = b16(pix2_16[j1]) + b1 + blue;

							pix2_16[j1] = rgbtocolor16(redpix, greenpix, bluepix);
						}
						break;
					case 24:
						src1[0] = c * colstep;
						src1[1] = r * rowstep;
						pix1_24 = scanline24(f1, src1[1]);
						r1 = r24(pix1_24[src1[0]]);
						g1 = g24(pix1_24[src1[0]]);
						b1 = b24(pix1_24[src1[0]]);

						for(j = 0, j1 = c * colstep; j < w; j += columns, j1++)
						{
							if ( j1 >= w )
							goto dink;

							if ( i1 >= h )
							goto yoink;

							pix2_24 = scanline24(f2, i1);
							redpix = r24(pix2_24[j1]) + r1 + red;
							greenpix = g24(pix2_24[j1]) + g1 + green;
							bluepix = b24(pix2_24[j1]) + b1 + blue;

							pix2_24[j1] = rgbtocolor24(redpix, greenpix, bluepix);
						}
						break;
					case 32:
						src1[0] = c * colstep;
						src1[1] = r * rowstep;
						pix1_32 = scanline32(f1, src1[1]);
						r1 = r32(pix1_32[src1[0]]);
						g1 = g32(pix1_32[src1[0]]);
						b1 = b32(pix1_32[src1[0]]);

						for(j = 0, j1 = c * colstep; j < w; j += columns, j1++)
						{
							if ( j1 >= w )
							goto dink;

							if ( i1 >= h )
							goto yoink;

							pix2_32 = scanline32(f2, i1);
							redpix = r32(pix2_32[j1]) + r1 + red;
							greenpix = g32(pix2_32[j1]) + g1 + green;
							bluepix = b32(pix2_32[j1]) + b1 + blue;

							pix2_32[j1] = rgbtocolor32(redpix, greenpix, bluepix);
						}
						break;
				}
			}
			dink:
			;
		}
	}
 	yoink:
 	;
}
