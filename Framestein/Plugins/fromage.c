// 242.fromage -- does cheezy wipes between two input images.
//		by r. luke dubois (luke@music.columbia.edu),
//			computer music center, columbia university, 2001.
//
//  Pd / Framestein port by Olaf Matthes <olaf.matthes@gmx.de>, June 2002
//
//	objects and source are provided without warranty of any kind, express or implied.
//
//  usage: fromage <x> <y> <columns> <rows> <flip>
//

#include <stdio.h>
#include <string.h>
#include "plugin.h"

void perform_effect(struct frame f, struct args a)
{
	printf("Using fromage as effect does nothing!\n");
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	short x, y, w, h, wf, hf, x_c, y_c, c, r, columns, rows, checky, checkx, flip = 0;
	pixel16 *pix1_16, *pix2_16;
	pixel24 *pix1_24, *pix2_24;
	pixel32 *pix1_32, *pix2_32;
	char *t;

	// get params
	if(!a.s) return;
	x_c = atoi(a.s);
	if(!(t = strstr(a.s, " "))) return;
	y_c = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	columns = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	rows = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	flip = atoi(t+1);

	printf("fromage: x%d y%d c%d r%d - f%d\n", x_c, y_c, columns, rows, flip);

	// calculate size we need to process
	wf = f1.width<f2.width ? f1.width : f2.width;
	hf = f1.height<f2.height ? f1.height : f2.height;

    w = wf / columns; // set column width
    h = hf / rows;    // set row height

	switch(f1.pixelformat)
	{
		case 16:
			for(r = 0; r < (rows + 1); r++)
			{
				for(y = r * h; y < h * (r + 1); y++) 
				{
                	pix1_16 = scanline16(f1, y);
                	pix2_16 = scanline16(f2, y);

					if(y >= hf) continue;
            
					if(y < (y_c + (r * h))) // occlude or don't occlude
					{
						checky = 1;
					}
					else
					{
						checky = 0;
					}
					if(flip) checky = !checky;
            
					for(c = 0; c < (columns + 1); c++)
					{
            			for(x = c * w; x < w * (c + 1); x++)
            			{
            				if (x >= wf) continue;
                
							if(x < (x_c + (c * w))) // occlude or don't occlude
							{
								checkx = 1;
							}
							else
							{
								checkx = 0;
							}
							if(flip) checkx = !checkx;

							if (checkx || checky)
							{
								pix2_16[x] = pix1_16[x]; // set output1
							}
            			}
					}

				 }
			}
			break;
		case 24:
			for(r = 0; r < (rows + 1); r++)
			{
				for(y = r * h; y < h * (r + 1); y++) 
				{
                	pix1_24 = scanline24(f1, y);
                	pix2_24 = scanline24(f2, y);

					if(y >= hf) continue;
            
					if(y < (y_c + (r*h))) // occlude or don't occlude
					{
						checky = 1;
					}
					else
					{
						checky = 0;
					}
					if(flip) checky = !checky;
            
					for(c = 0; c < (columns + 1); c++)
					{
            			for(x = c * w; x < w * (c + 1); x++)
            			{
            				if (x >= wf) continue;
                
							if(x < (x_c + (c * w))) // occlude or don't occlude
							{
								checkx = 1;
							}
							else
							{
								checkx = 0;
							}
							if(flip) checkx = !checkx;

							if (checkx || checky)
							{
								pix2_24[x] = pix1_24[x]; // set output2
							}
            			}
					}

				 }
			}
			break;
		case 32:
			for(r = 0; r < (rows + 1); r++)
			{
				for(y = r * h; y < h * (r + 1); y++) 
				{
                	pix1_32 = scanline32(f1, y);
                	pix2_32 = scanline32(f2, y);

					if(y >= hf) continue;
            
					if(y < (y_c + (r * h))) // occlude or don't occlude
					{
						checky = 1;
					}
					else
					{
						checky = 0;
					}
					if(flip) checky = !checky;
            
					for(c = 0; c < (columns + 1); c++)
					{
            			for(x = c * w; x < w * (c + 1); x++)
            			{
            				if (x >= wf) continue;
                
							if(x < (x_c + (c * w))) // occlude or don't occlude
							{
								checkx = 1;
							}
							else
							{
								checkx = 0;
							}
							if(flip) checkx = !checkx;

							if (checkx || checky)
							{
								pix2_32[x] = pix1_32[x]; // set output2
							}
            			}
					}

				 }
			}
			break;
	}
}
