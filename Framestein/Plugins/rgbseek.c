// 242.rgbseek -- does binary color recognition on an input image.
//		by r. luke dubois (luke@music.columbia.edu),
//			computer music center, columbia university, 2001.
//
//  Pd / Framestein port by Olaf Matthes <olaf.matthes@gmx.de>, June 2002
//
//	objects and source are provided without warranty of any kind, express or implied.
//
//  usage: rgbseek <red> <green> <blue> <fuzzi red> <fuzzi green> <fuzzi blue>
//
//  return: 1 if color exists in frame, 0 if not
//

#include <stdio.h>
#include <string.h>
#include "plugin.h"

INFO("242.rgbseek -- does binary color recognition on an input image")

void perform_effect(struct frame f, struct args a)
{
	int x,y;
	byte red = 0, green = 0, blue = 0, check = 0, rf = 0, gf = 0, bf = 0;
	byte bits = f.pixelformat/8;
	byte redpix, greenpix, bluepix;	// the color values
	pixel16 c16;
	pixel24 c24;
	pixel32 c32;
	char *ret_check;                // returns the result
	char *t;

	// get r g b and fuzzi params
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

	printf("rgbseek: %d %d %d\n", red, green, blue);

	// get returnvaluereceivenames
	if(!(t = strstr(t+1, " "))) return;
	ret_check = t+1;
	t[0]=0;

	for(y = 0; y < f.height; y++) 
	{
		for(x = 0; x < f.width; x++)
		{
			switch (f.pixelformat)
			{
				case 16:
					c16 = scanline16(f, y)[x];
					redpix = r16(c16);
					greenpix = g16(c16);
					bluepix = b16(c16);
					break;
				case 24:
					c24 = scanline24(f, y)[x];
					redpix = r24(c24);
					greenpix = g24(c24);
					bluepix = b24(c24);
					break;
				case 32:
					c32 = scanline32(f, y)[x];
					redpix = r32(c32);
					greenpix = g32(c32);
					bluepix = b32(c32);
					break;
			}
			if ((redpix>=(red-rf))&&(redpix<=(red+rf))&&(greenpix>=(green-gf))&&(greenpix<=(green+gf))&&(bluepix>=(blue-bf))&&(bluepix<=(blue+bf)))
			{
				check = 1; // found the color, let's get outta here
				goto done;
			}        
		}
	}
	check = 0; // didn't find the color

done: 
	// return-values:
	//
	// framestein will send data given in the form "pd_receiver_name=value"
	// back to pd.

	sprintf(a.ret, "%s=%d", ret_check, check);
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	printf("Using rgbseek as copy operation does nothing!\n");
}
