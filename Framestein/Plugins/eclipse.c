// 242.eclipse -- does meta-imaging on an input image.
//		by r. luke dubois (luke@music.columbia.edu),
//			computer music center, columbia university, 2001.
//		grid quantization algorithm massively improved by jeremy bernstein, bootsquadresearch.
//
//  Pd / Framestein port by Olaf Matthes <olaf.matthes@gmx.de>, June 2002
//
//	objects and source are provided without warranty of any kind, express or implied.
//
//  usage: eclipse <red> <green> <blue> <rows> <columns>
//

#include <stdio.h>
#include <string.h>
#include "plugin.h"

INFO("242.eclipse -- does meta-imaging on an input image")

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
	int i, i1, j, j1, r, c, src2[2];
	int rows, columns, rowstep, colstep, rowoffset, coloffset, check;
	byte red, green, blue, r2, g2, b2;
	byte bits = f.pixelformat/8;
	byte redpix, greenpix, bluepix;	// the color values
	pixel16 *pix_16, *pix2_16;
	pixel24 *pix_24, *pix2_24;
	pixel32 *pix_32, *pix2_32;
	char *t;

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
	
	printf("eclipse: r%d g%d b%d - r%d c%d\n", red, green, blue, rows, columns);


	rowstep = (f.height / rows) + .5;
	colstep = (f.width / columns) + .5;
	// printf("red is %ld green is %ld blue is %ld", red, green, blue);

	if ( rows > (f.height * .1) )
		rowoffset = rows - (f.height * .1);
	else
		rowoffset = 1;
    
	if ( columns > (f.width * .1) )
		coloffset = columns - (f.width * .1);
	else
		coloffset = 1;
        

	for(r = 0; r <= ( rows + rowoffset ); r++ ) 
	{
		for(i = 0, i1 = ( r * rowstep ); i < f.height; i+=rows, i1++) 
		{
			for( c = 0; c <= columns + coloffset; c++ ) 
			{
				switch(f.pixelformat)
				{
					case 16:
						src2[0] = c * colstep;
						src2[1] = r * rowstep;
						pix2_16 = scanline16(f, src2[1]);
						r2 = r16(pix2_16[src2[0]]);
						g2 = g16(pix2_16[src2[0]]);
						b2 = b16(pix2_16[src2[0]]);

						for(j = 0, j1 = c * colstep; j < f.width; j += columns, j1++)
						{
							if ( j1 >= f.width )
							goto dink;

							if ( i1 >= f.height )
							goto yoink;

							pix_16 = scanline16(f, i1);
							redpix = r16(pix_16[j1]) + r2 + red;
							greenpix = g16(pix_16[j1]) + g2 + green;
							bluepix = b16(pix_16[j1]) + b2 + blue;

							pix_16[j1] = rgbtocolor16(redpix, greenpix, bluepix);
						}
						break;
					case 24:
						src2[0] = c * colstep;
						src2[1] = r * rowstep;
						pix2_24 = scanline24(f, src2[1]);
						r2 = r24(pix2_24[src2[0]]);
						g2 = g24(pix2_24[src2[0]]);
						b2 = b24(pix2_24[src2[0]]);

						for(j = 0, j1 = c * colstep; j < f.width; j += columns, j1++)
						{
							if ( j1 >= f.width )
							goto dink;

							if ( i1 >= f.height )
							goto yoink;

							pix_24 = scanline24(f, i1);
							redpix = r24(pix_24[j1]) + r2 + red;
							greenpix = g24(pix_24[j1]) + g2 + green;
							bluepix = b24(pix_24[j1]) + b2 + blue;

							pix_24[j1] = rgbtocolor24(redpix, greenpix, bluepix);
						}
						break;
					case 32:
						src2[0] = c * colstep;
						src2[1] = r * rowstep;
						pix2_32 = scanline32(f, src2[1]);
						r2 = r32(pix2_32[src2[0]]);
						g2 = g32(pix2_32[src2[0]]);
						b2 = b32(pix2_32[src2[0]]);

						for(j = 0, j1 = c * colstep; j < f.width; j += columns, j1++)
						{
							if ( j1 >= f.width )
							goto dink;

							if ( i1 >= f.height )
							goto yoink;

							pix_32 = scanline32(f, i1);
							redpix = r32(pix_32[j1]) + r2 + red;
							greenpix = g32(pix_32[j1]) + g2 + green;
							bluepix = b32(pix_32[j1]) + b2 + blue;

							pix_32[j1] = rgbtocolor32(redpix, greenpix, bluepix);
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

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	printf("Using eclipse as copy operation does nothing!\n");
}
