//
//  interlace - deinterlacing through pixel / line repetition
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
//  usage: interlace <line> [ with line = 0 or 1 ]
//

#include <stdio.h>
#include <string.h>
#include "plugin.h"

INFO("deinterlacing through pixel / line repetition")

void perform_effect(struct frame f, struct args a)
{
	printf("Using interlace as effect does nothing!\n");
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	short h, w, line = 0;
	byte pixelsize = f1.pixelformat/8;
	pixel16 *p16, *c16;
	pixel24 *p24, *c24;
	pixel32 *p32, *c32;
	char *t;

	// get params
	if(a.s)
	{
		line = atoi(a.s);
	}	// else keep 0

	printf("interlace: %d\n", line);

	w = f1.width<f2.width ? f1.width : f2.width;
	h = f1.height<f2.height ? f1.height : f2.height;
	h -= line;

	switch(f1.pixelformat)
	{
		case 16:
			while(h > 0)
			{
				p16 = scanline16(f1, h);
				c16 = scanline16(f2, h);
				memcpy(c16, p16, w*pixelsize);
				h -= 2;
			}
			break;
		case 24:
			while(h > 0)
			{
				p24 = scanline24(f1, h);
				c24 = scanline24(f2, h);
				memcpy(c24, p24, w*pixelsize);
				h -= 2;
			}
			break;
		case 32:
			while(h > 0)
			{
				p32 = scanline32(f1, h);
				c32 = scanline32(f2, h);
				memcpy(c32, p32, w*pixelsize);
				h -= 2;
			}
			break;
	}
}
