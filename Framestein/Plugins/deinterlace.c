//
//  deinterlace - deinterlacing through pixel / line repetition
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
//  usage: deinterlace <line> [ with line = 0 or 1 ]
//

#include <stdio.h>
#include <string.h>
#include "plugin.h"

INFO("deinterlacing through pixel / line repetition")

void perform_effect(struct frame f, struct args a)
{
	short x, y, line = 0;
	byte pixelsize = f.pixelformat/8;
	pixel16 *p16, *c16;
	pixel24 *p24, *c24;
	pixel32 *p32, *c32;
	char *t;

	// get params
	if(a.s)
	{
		line = atoi(a.s);
	}	// else keep 0

	printf("deinterlace: %d\n", line);

	y = f.height - line;
	x = f.width;

	switch(f.pixelformat)
	{
		case 16:
			while(y > 0)
			{
				p16 = scanline16(f, y);
				c16 = scanline16(f, y-1);
				memcpy(p16, c16, x*pixelsize);
				y -= 2;
			}
			break;
		case 24:
			while(y > 0)
			{
				p24 = scanline24(f, y);
				c24 = scanline24(f, y-1);
				memcpy(p24, c24, x*pixelsize);
				y -= 2;
			}
			break;
		case 32:
			while(y > 0)
			{
				p32 = scanline32(f, y);
				c32 = scanline32(f, y-1);
				memcpy(p32, c32, x*pixelsize);
				y -= 2;
			}
			break;
	}
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	printf("Using deinterlace as copy operation does nothing!\n");
}
