#include "plugin.h"

INFO("color distortion")

void perform_effect(struct frame f, struct args a)
{
	int x, y;
	pixel16 *p16;
	pixel32 *p32;
	pixel16 c1, c2, c3;

	switch(f.pixelformat)
	{
		case 16:
			for(y=0; y<f.height; y++)
			{
				p16=scanline16(f, y);
				for(x=1; x<f.width; x++)
				{
					p16[x]=(p16[x]+p16[x-1])/2;
				}
			}
			break;
		case 32:
			for(y=0; y<f.height; y++)
			{
				p32=scanline32(f, y);
				for(x=1; x<f.width; x++)
				{
					p32[x]=(p32[x]+p32[x-1])/2;
				}
			}
			break;
	}
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	int x, y, w, h;
	pixel16 *p1_16, *p2_16, c1, c2, c3;
	pixel32 *p1_32, *p2_32;

	w = f1.width<f2.width ? f1.width : f2.width;
	h = f1.height<f2.height ? f1.height : f2.height;

	switch(f1.pixelformat)
	{
		case 16:
			for(y=0; y<h; y++)
			{
				p1_16 = scanline16(f1, y);
				p2_16 = scanline16(f2, y);
				for(x=1; x<w; x++)
				{
					p2_16[x]=(p1_16[x]+p2_16[x])/2;
				}
			}
			break;
		case 32:
			for(y=0; y<h; y++)
			{
				p1_32 = scanline32(f1, y);
				p2_32 = scanline32(f2, y);
				for(x=1; x<w; x++)
				{
					p2_32[x]=(p1_32[x]+p2_32[x])/2;
				}
			}
			break;
	}
}
