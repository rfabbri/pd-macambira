// 242.rene -- does 3-source chroma keying with transparency.
//		by r. luke dubois (luke@music.columbia.edu),
//			computer music center, columbia university, 2001.
//  for rene beekman.
//
//  Pd / Framestein port by Olaf Matthes <olaf.matthes@gmx.de>, June 2002
//
//	objects and source are provided without warranty of any kind, express or implied.
//
//  usage: rene <red> <green> <blue> <fuzzi red> <fuzzi green> <fuzzi blue> <redfloor> <greenfloor> <bluefloor>
//
//         f1 is the key & target, f2 is the mask
//

#include <stdio.h>
#include <string.h>
#include "plugin.h"

INFO("242.rene -- does 3-source chroma keying with transparency")

void perform_effect(struct frame f, struct args a)
{
	printf("Using keyscreen as effect does nothing!\n");
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	short x, y, w, h;
	pixel16 *pix1_16, *pix2_16;
	pixel24 *pix1_24, *pix2_24;
	pixel32 *pix1_32, *pix2_32;
	byte redpix, greenpix, bluepix, rkeyed, gkeyed, bkeyed;
	byte red, green, blue, check, rf, gf, bf, rfl, gfl, bfl;
	float lum, keylum, lowfuzz, highfuzz, lowfloor, highfloor, coeff;
	char *t;

	w = f1.width<f2.width ? f1.width : f2.width;
	h = f1.height<f2.height ? f1.height : f2.height;

	// get params
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
	if(!(t = strstr(t+1, " "))) return;
	rfl = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	gfl = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	bfl = atoi(t+1);

	printf("rene: r%d g%d b%d rg%d gf%d bf%d rfl%d gfl%d bfl%d\n", 
		    red, green, blue, rf, gf, bf, rfl, gfl, bfl);

	// key all channels simultaneously
	switch(f1.pixelformat)
	{
		case 16:
			for(y = 0; y < h; y++) 
			{
				pix1_16 = scanline16(f1, y);
				pix2_16 = scanline16(f2, y);
				for(x = 0; x < w; x++)
				{
					check = 0; // start with each pixel anew

					redpix = r16(pix2_16[x]);
					greenpix = g16(pix2_16[x]);
					bluepix = b16(pix2_16[x]);
					if ((redpix>=(red-rf))&&(redpix<=(red+rf))&&(greenpix>=(green-gf))&&(greenpix<=(green+gf))&&(bluepix>=(blue-bf))&&(bluepix<=(blue+bf)))
					{
                			check = 1; // mask it
					}        
					if ((redpix<(red-rfl))||(redpix>(red+rfl))&&(greenpix<(green-gfl))||(greenpix>(green+gfl))&&(bluepix<(blue-bfl))||(bluepix>(blue+bfl)))
					{
                			check = 2; // target it
					}        

					if (!check)
					{
						lum = (redpix + greenpix + bluepix) / 3.0;
						keylum = (red + blue + green) / 3.0;
						lowfuzz = (rf + gf + bf) / 3.0;
						if (lowfuzz < 0.0) lowfuzz = 0.0;
						highfuzz = (rf + gf + bf) / 3.0;
						if (highfuzz > 1.0) highfuzz = 1.0;
						lowfloor = keylum - ((rfl + gfl + bfl) / 3.0);
						if (lowfloor < 0.0) lowfloor = 0.;
						highfloor = keylum + (rfl + gfl + bfl) / 3.0;
						if (highfloor > 1.0) highfloor = 1.0;
						if (lum < keylum)
						{
							coeff = ((lum - lowfloor) / (keylum - lowfuzz));
						}
						if (lum >= keylum)
						{
							coeff = ((255 - lum - highfloor) / (255 - keylum - highfuzz));
						}

						rkeyed = ((float)r16(pix2_16[x]) * coeff) + ((float)r16(pix1_16[x]) * (1.0 - coeff));
						gkeyed = ((float)g16(pix2_16[x]) * coeff) + ((float)g16(pix1_16[x]) * (1.0 - coeff));
						bkeyed = ((float)b16(pix2_16[x]) * coeff) + ((float)b16(pix1_16[x]) * (1.0 - coeff));	
								
						pix2_16[x] = rgbtocolor16(rkeyed, gkeyed, bkeyed);
					}

					if (check==1)
					{	// was pix3
						// pix2_16[x] = rgbtocolor16(r16(pix2_16[x]), g16(pix2_16[x]), b16(pix2_16[x]));
					}
					if (check==2)
					{	// was pix2
						pix2_16[x] = rgbtocolor16(r16(pix1_16[x]), g16(pix1_16[x]), b16(pix1_16[x]));
					}
				}
			}
			break;
		case 24:
			for(y = 0; y < h; y++) 
			{
				pix1_24 = scanline24(f1, y);
				pix2_24 = scanline24(f2, y);
				for(x = 0; x < w; x++)
				{
					check = 0; // start with each pixel anew

					redpix = r24(pix2_24[x]);
					greenpix = g24(pix2_24[x]);
					bluepix = b24(pix2_24[x]);
					if ((redpix>=(red-rf))&&(redpix<=(red+rf))&&(greenpix>=(green-gf))&&(greenpix<=(green+gf))&&(bluepix>=(blue-bf))&&(bluepix<=(blue+bf)))
					{
                			check = 1; // mask it
					}        
					if ((redpix<(red-rfl))||(redpix>(red+rfl))&&(greenpix<(green-gfl))||(greenpix>(green+gfl))&&(bluepix<(blue-bfl))||(bluepix>(blue+bfl)))
					{
                			check = 2; // target it
					}        

					if (!check)
					{
						lum = (redpix + greenpix + bluepix) / 3.0;
						keylum = (red + blue + green) / 3.0;
						lowfuzz = (rf + gf + bf) / 3.0;
						if (lowfuzz < 0.0) lowfuzz = 0.0;
						highfuzz = (rf + gf + bf) / 3.0;
						if (highfuzz > 1.0) highfuzz = 1.0;
						lowfloor = keylum - ((rfl + gfl + bfl) / 3.0);
						if (lowfloor < 0.0) lowfloor = 0.;
						highfloor = keylum + (rfl + gfl + bfl) / 3.0;
						if (highfloor > 1.0) highfloor = 1.0;
						if (lum < keylum)
						{
							coeff = ((lum - lowfloor) / (keylum - lowfuzz));
						}
						if (lum >= keylum)
						{
							coeff = ((255 - lum - highfloor) / (255 - keylum - highfuzz));
						}

						rkeyed = ((float)r24(pix2_24[x]) * coeff) + ((float)r24(pix1_24[x]) * (1.0 - coeff));
						gkeyed = ((float)g24(pix2_24[x]) * coeff) + ((float)g24(pix1_24[x]) * (1.0 - coeff));
						bkeyed = ((float)b24(pix2_24[x]) * coeff) + ((float)b24(pix1_24[x]) * (1.0 - coeff));	
								
						pix2_24[x] = rgbtocolor24(rkeyed, gkeyed, bkeyed);
					}

					if (check==1)
					{	// was pix3
						// pix2_24[x] = rgbtocolor24(r24(pix2_24[x]), g24(pix2_24[x]), b24(pix2_24[x]));
					}
					if (check==2)
					{	// was pix2
						pix2_24[x] = rgbtocolor24(r24(pix1_24[x]), g24(pix1_24[x]), b24(pix1_24[x]));
					}
				}
			}
			break;
		case 32:
			for(y = 0; y < h; y++) 
			{
				pix1_32 = scanline32(f1, y);
				pix2_32 = scanline32(f2, y);
				for(x = 0; x < w; x++)
				{
					check = 0; // start with each pixel anew

					redpix = r32(pix2_32[x]);
					greenpix = g32(pix2_32[x]);
					bluepix = b32(pix2_32[x]);
					if ((redpix>=(red-rf))&&(redpix<=(red+rf))&&(greenpix>=(green-gf))&&(greenpix<=(green+gf))&&(bluepix>=(blue-bf))&&(bluepix<=(blue+bf)))
					{
                			check = 1; // mask it
					}        
					if ((redpix<(red-rfl))||(redpix>(red+rfl))&&(greenpix<(green-gfl))||(greenpix>(green+gfl))&&(bluepix<(blue-bfl))||(bluepix>(blue+bfl)))
					{
                			check = 2; // target it
					}        

					if (!check)
					{
						lum = (redpix + greenpix + bluepix) / 3.0;
						keylum = (red + blue + green) / 3.0;
						lowfuzz = (rf + gf + bf) / 3.0;
						if (lowfuzz < 0.0) lowfuzz = 0.0;
						highfuzz = (rf + gf + bf) / 3.0;
						if (highfuzz > 1.0) highfuzz = 1.0;
						lowfloor = keylum - ((rfl + gfl + bfl) / 3.0);
						if (lowfloor < 0.0) lowfloor = 0.;
						highfloor = keylum + (rfl + gfl + bfl) / 3.0;
						if (highfloor > 1.0) highfloor = 1.0;
						if (lum < keylum)
						{
							coeff = ((lum - lowfloor) / (keylum - lowfuzz));
						}
						if (lum >= keylum)
						{
							coeff = ((255 - lum - highfloor) / (255 - keylum - highfuzz));
						}

						rkeyed = ((float)r32(pix2_32[x]) * coeff) + ((float)r32(pix1_32[x]) * (1.0 - coeff));
						gkeyed = ((float)g32(pix2_32[x]) * coeff) + ((float)g32(pix1_32[x]) * (1.0 - coeff));
						bkeyed = ((float)b32(pix2_32[x]) * coeff) + ((float)b32(pix1_32[x]) * (1.0 - coeff));	
								
						pix2_32[x] = rgbtocolor32(rkeyed, gkeyed, bkeyed);
					}

					if (check==1)
					{	// was pix3
						// pix2_32[x] = rgbtocolor32(r32(pix2_32[x]), g32(pix2_32[x]), b32(pix2_32[x]));
					}
					if (check==2)
					{	// was pix2
						pix2_32[x] = rgbtocolor32(r32(pix1_32[x]), g32(pix1_32[x]), b32(pix1_32[x]));
					}
				}
			}
			break;
	}
}
