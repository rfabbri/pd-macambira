// 242.rgbavg02 -- does 2-source funky pixel averaging.
//		by r. luke dubois (luke@music.columbia.edu),
//			computer music center, columbia university, 2001.
//
//  Pd / Framestein port by Olaf Matthes <olaf.matthes@gmx.de>, June 2002
//
//	objects and source are provided without warranty of any kind, express or implied.
//
//  usage: rgbavg <red> <green> <blue> <mode>
//

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "plugin.h"

INFO("242.rgbavg02 -- does 2-source funky pixel averaging")

#pragma warning( disable : 4761 )	// that's why it's funky !!

void perform_effect(struct frame f1, struct frame f2, struct args a)
{
	printf("Using rgbavg as effect does nothing!\n");
}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	short x,y, h, w;
	long red, green, blue;
	short alpha, mode;
	long redpix, greenpix, bluepix, rp, gp, bp;
	pixel16 *c1_16, *c2_16;
	pixel24 *c1_24, *c2_24;
	pixel32 *c1_32, *c2_32;
	char *t;

	// get params
	if(!a.s) return;
	red = atoi(a.s);
	if(!(t = strstr(a.s, " "))) return;
	green = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	blue = atoi(t+1);
	if(!(t = strstr(t+1, " "))) return;
	mode = atoi(t+1);	// mode=0 = bypass

	printf("rgbavg: %d %d %d - %d\n", red, green, blue, mode);

	w = f1.width<f2.width ? f1.width : f2.width;
	h = f1.height<f2.height ? f1.height : f2.height;

	// perform routines do different pixel averagings in the main loop
	switch (mode)
	{
		case 1:	// output: color / (sum of other two colors)
			switch (f1.pixelformat)
			{
				case 16:
					for(y = 0; y < h; y++) 
					{
						c1_16 = scanline16(f1, y);
						c2_16 = scanline16(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r16(c1_16[x]) + red;
							gp = g16(c1_16[x]) + green;
							bp = b16(c1_16[x]) + blue;
							redpix = r16(c2_16[x]) + red;
							greenpix = g16(c2_16[x]) + green;
							bluepix = b16(c2_16[x]) + blue;
							c2_16[x] = rgbtocolor16(klamp255(redpix/(gp+bp)), 
								                    klamp255(greenpix/(rp+bp)), 
											        klamp255(bluepix/(rp+gp)));
						}
					}
					break;
				case 24:
					for(y = 0; y < h; y++) 
					{
						c1_24 = scanline24(f1, y);
						c2_24 = scanline24(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r24(c1_24[x]) + red;
							gp = g24(c1_24[x]) + green;
							bp = b24(c1_24[x]) + blue;
							redpix = r24(c2_24[x]) + red;
							greenpix = g24(c2_24[x]) + green;
							bluepix = b24(c2_24[x]) + blue;
							c2_24[x] = rgbtocolor24(klamp255(redpix/(gp+bp)), 
								                    klamp255(greenpix/(rp+bp)), 
											        klamp255(bluepix/(rp+gp)));
						}
					}
					break;
				case 32:
					for(y = 0; y < h; y++) 
					{
						c1_32 = scanline32(f1, y);
						c2_32 = scanline32(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r32(c1_32[x]) + red;
							gp = g32(c1_32[x]) + green;
							bp = b32(c1_32[x]) + blue;
							redpix = r32(c2_32[x]) + red;
							greenpix = g32(c2_32[x]) + green;
							bluepix = b32(c2_32[x]) + blue;
							c2_32[x] = rgbtocolor32(klamp255(redpix/(gp+bp)), 
								                    klamp255(greenpix/(rp+bp)), 
											        klamp255(bluepix/(rp+gp)));
						}
					}
					break;
			}
			break;
		case 2:	// output: (sum of two colors) / third color
			switch (f1.pixelformat)
			{
				case 16:
					for(y = 0; y < h; y++) 
					{
						c1_16 = scanline16(f1, y);
						c2_16 = scanline16(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r16(c1_16[x]) + red;
							gp = g16(c1_16[x]) + green;
							bp = b16(c1_16[x]) + blue;
							redpix = r16(c2_16[x]) + red;
							greenpix = g16(c2_16[x]) + green;
							bluepix = b16(c2_16[x]) + blue;
							c2_16[x] = rgbtocolor16(klamp255((gp+bp)/redpix), 
								                    klamp255((rp+bp)/greenpix), 
											        klamp255((rp+gp)/bluepix));
						}
					}
					break;
				case 24:
					for(y = 0; y < h; y++) 
					{
						c1_24 = scanline24(f1, y);
						c2_24 = scanline24(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r24(c1_24[x]) + red;
							gp = g24(c1_24[x]) + green;
							bp = b24(c1_24[x]) + blue;
							redpix = r24(c2_24[x]) + red;
							greenpix = g24(c2_24[x]) + green;
							bluepix = b24(c2_24[x]) + blue;
							c2_24[x] = rgbtocolor24(klamp255((gp+bp)/redpix), 
								                    klamp255((rp+bp)/greenpix), 
											        klamp255((rp+gp)/bluepix));
						}
					}
					break;
				case 32:
					for(y = 0; y < h; y++) 
					{
						c1_32 = scanline32(f1, y);
						c2_32 = scanline32(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r32(c1_32[x]) + red;
							gp = g32(c1_32[x]) + green;
							bp = b32(c1_32[x]) + blue;
							redpix = r32(c2_32[x]) + red;
							greenpix = g32(c2_32[x]) + green;
							bluepix = b32(c2_32[x]) + blue;
							c2_32[x] = rgbtocolor32(klamp255((gp+bp)/redpix), 
								                    klamp255((rp+bp)/greenpix), 
											        klamp255((rp+gp)/bluepix));
						}
					}
					break;
			}
			break;
		case 3:	// output: color / (sum of other two colors) + color(input)
			switch (f1.pixelformat)
			{
				case 16:
					for(y = 0; y < h; y++) 
					{
						c1_16 = scanline16(f1, y);
						c2_16 = scanline16(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r16(c1_16[x]) + 1;
							gp = g16(c1_16[x]) + 1;
							bp = b16(c1_16[x]) + 1;
							redpix = r16(c2_16[x]) + 1;
							greenpix = g16(c2_16[x]) + 1;
							bluepix = b16(c2_16[x]) + 1;
							c2_16[x] = rgbtocolor16(klamp255(red + ((gp+bp)/redpix)), 
								                    klamp255(green + ((rp+bp)/greenpix)),
													klamp255(blue + ((rp+gp)/bluepix)));
						}
					}
					break;
				case 24:
					for(y = 0; y < h; y++) 
					{
						c1_24 = scanline24(f1, y);
						c2_24 = scanline24(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r24(c1_24[x]) + 1;
							gp = g24(c1_24[x]) + 1;
							bp = b24(c1_24[x]) + 1;
							redpix = r24(c2_24[x]) + 1;
							greenpix = g24(c2_24[x]) + 1;
							bluepix = b24(c2_24[x]) + 1;
							c2_24[x] = rgbtocolor24(klamp255(red + ((gp+bp)/redpix)), 
								                    klamp255(green + ((rp+bp)/greenpix)),
													klamp255(blue + ((rp+gp)/bluepix)));
						}
					}
					break;
				case 32:
					for(y = 0; y < h; y++) 
					{
						c1_32 = scanline32(f1, y);
						c2_32 = scanline32(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r32(c1_32[x]) + 1;
							gp = g32(c1_32[x]) + 1;
							bp = b32(c1_32[x]) + 1;
							redpix = r32(c2_32[x]) + 1;
							greenpix = g32(c2_32[x]) + 1;
							bluepix = b32(c2_32[x]) + 1;
							c2_32[x] = rgbtocolor32(klamp255(red + ((gp+bp)/redpix)), 
								                    klamp255(green + ((rp+bp)/greenpix)),
													klamp255(blue + ((rp+gp)/bluepix)));
						}
					}
					break;
			}
			break;
		case 4:
			switch (f1.pixelformat)
			{
				case 16:
					for(y = 0; y < h; y++) 
					{
						c1_16 = scanline16(f1, y);
						c2_16 = scanline16(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r16(c1_16[x]) + 1;
							gp = g16(c1_16[x]) + 1;
							bp = b16(c1_16[x]) + 1;
							redpix = r16(c2_16[x]) + 1;
							greenpix = g16(c2_16[x]) + 1;
							bluepix = b16(c2_16[x]) + 1;
							c2_16[x] = rgbtocolor16(klamp255(((gp*green)+(bp*blue))/redpix), 
								                    klamp255(((rp*red)+(bp*blue))/greenpix), 
											        klamp255(((rp*red)+(gp*green))/bluepix));
						}
					}
					break;
				case 24:
					for(y = 0; y < h; y++) 
					{
						c1_24 = scanline24(f1, y);
						c2_24 = scanline24(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r24(c1_24[x]) + 1;
							gp = g24(c1_24[x]) + 1;
							bp = b24(c1_24[x]) + 1;
							redpix = r24(c2_24[x]) + 1;
							greenpix = g24(c2_24[x]) + 1;
							bluepix = b24(c2_24[x]) + 1;
							c2_24[x] = rgbtocolor24(klamp255(((gp*green)+(bp*blue))/redpix), 
								                    klamp255(((rp*red)+(bp*blue))/greenpix), 
											        klamp255(((rp*red)+(gp*green))/bluepix));
						}
					}
					break;
				case 32:
					for(y = 0; y < h; y++) 
					{
						c1_32 = scanline32(f1, y);
						c2_32 = scanline32(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r32(c1_32[x]) + 1;
							gp = g32(c1_32[x]) + 1;
							bp = b32(c1_32[x]) + 1;
							redpix = r32(c2_32[x]) + 1;
							greenpix = g32(c2_32[x]) + 1;
							bluepix = b32(c2_32[x]) + 1;
							c2_32[x] = rgbtocolor32(klamp255(((gp*green)+(bp*blue))/redpix), 
								                    klamp255(((rp*red)+(bp*blue))/greenpix), 
											        klamp255(((rp*red)+(gp*green))/bluepix));
						}
					}
					break;
			}
			break;
		case 5:
			switch (f1.pixelformat)
			{
				case 16:
					for(y = 0; y < h; y++) 
					{
						c1_16 = scanline16(f1, y);
						c2_16 = scanline16(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r16(c1_16[x]) + 1;
							gp = g16(c1_16[x]) + 1;
							bp = b16(c1_16[x]) + 1;
							redpix = r16(c2_16[x]) + 1;
							greenpix = g16(c2_16[x]) + 1;
							bluepix = b16(c2_16[x]) + 1;
							c2_16[x] = rgbtocolor16(klamp255(((gp*green)+(bp*blue))/(redpix*red)), 
								                    klamp255(((rp*red)+(bp*blue))/(greenpix*green)), 
											        klamp255(((rp*red)+(gp*green))/(bluepix*blue)));
						}
					}
					break;
				case 24:
					for(y = 0; y < h; y++) 
					{
						c1_24 = scanline24(f1, y);
						c2_24 = scanline24(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r24(c1_24[x]) + 1;
							gp = g24(c1_24[x]) + 1;
							bp = b24(c1_24[x]) + 1;
							redpix = r24(c2_24[x]) + 1;
							greenpix = g24(c2_24[x]) + 1;
							bluepix = b24(c2_24[x]) + 1;
							c2_24[x] = rgbtocolor24(klamp255(((gp*green)+(bp*blue))/(redpix*red)), 
								                    klamp255(((rp*red)+(bp*blue))/(greenpix*green)), 
											        klamp255(((rp*red)+(gp*green))/(bluepix*blue)));
						}
					}
					break;
				case 32:
					for(y = 0; y < h; y++) 
					{
						c1_32 = scanline32(f1, y);
						c2_32 = scanline32(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r32(c1_32[x]) + 1;
							gp = g32(c1_32[x]) + 1;
							bp = b32(c1_32[x]) + 1;
							redpix = r32(c2_32[x]) + 1;
							greenpix = g32(c2_32[x]) + 1;
							bluepix = b32(c2_32[x]) + 1;
							c2_32[x] = rgbtocolor32(klamp255(((gp*green)+(bp*blue))/(redpix*red)), 
								                    klamp255(((rp*red)+(bp*blue))/(greenpix*green)), 
											        klamp255(((rp*red)+(gp*green))/(bluepix*blue)));
						}
					}
					break;
			}
			break;
		case 6:
			switch (f1.pixelformat)
			{
				case 16:
					for(y = 0; y < h; y++) 
					{
						c1_16 = scanline16(f1, y);
						c2_16 = scanline16(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r16(c1_16[x]) + 1;
							gp = g16(c1_16[x]) + 1;
							bp = b16(c1_16[x]) + 1;
							redpix = r16(c2_16[x]) + 1;
							greenpix = g16(c2_16[x]) + 1;
							bluepix = b16(c2_16[x]) + 1;
							c2_16[x] = rgbtocolor16(klamp255((redpix*red)/(gp*green)+(bp*blue)), 
								                    klamp255((greenpix*green)/(rp*red)+(bp*blue)), 
											        klamp255((bluepix*blue)/(rp*red)+(gp*green)));
						}
					}
					break;
				case 24:
					for(y = 0; y < h; y++) 
					{
						c1_24 = scanline24(f1, y);
						c2_24 = scanline24(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r24(c1_24[x]) + 1;
							gp = g24(c1_24[x]) + 1;
							bp = b24(c1_24[x]) + 1;
							redpix = r24(c2_24[x]) + 1;
							greenpix = g24(c2_24[x]) + 1;
							bluepix = b24(c2_24[x]) + 1;
							c2_24[x] = rgbtocolor24(klamp255((redpix*red)/(gp*green)+(bp*blue)), 
								                    klamp255((greenpix*green)/(rp*red)+(bp*blue)), 
											        klamp255((bluepix*blue)/(rp*red)+(gp*green)));
						}
					}
					break;
				case 32:
					for(y = 0; y < h; y++) 
					{
						c1_32 = scanline32(f1, y);
						c2_32 = scanline32(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r32(c1_32[x]) + 1;
							gp = g32(c1_32[x]) + 1;
							bp = b32(c1_32[x]) + 1;
							redpix = r32(c2_32[x]) + 1;
							greenpix = g32(c2_32[x]) + 1;
							bluepix = b32(c2_32[x]) + 1;
							c2_32[x] = rgbtocolor32(klamp255((redpix*red)/(gp*green)+(bp*blue)), 
								                    klamp255((greenpix*green)/(rp*red)+(bp*blue)), 
											        klamp255((bluepix*blue)/(rp*red)+(gp*green)));
						}
					}
					break;
			}
			break;
		case 7:
			switch (f1.pixelformat)
			{
				case 16:
					for(y = 0; y < h; y++) 
					{
						c1_16 = scanline16(f1, y);
						c2_16 = scanline16(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r16(c1_16[x]) + red;
							gp = g16(c1_16[x]) + green;
							bp = b16(c1_16[x]) + blue;
							redpix = r16(c2_16[x]) + red;
							greenpix = g16(c2_16[x]) + green;
							bluepix = b16(c2_16[x]) + blue;
							c2_16[x] = rgbtocolor16(klamp255((redpix/(gp+bp))*red), 
								                    klamp255((greenpix/(rp+bp))*green), 
											        klamp255((bluepix/(rp+gp))*blue));
						}
					}
					break;
				case 24:
					for(y = 0; y < h; y++) 
					{
						c1_24 = scanline24(f1, y);
						c2_24 = scanline24(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r24(c1_24[x]) + red;
							gp = g24(c1_24[x]) + green;
							bp = b24(c1_24[x]) + blue;
							redpix = r24(c2_24[x]) + red;
							greenpix = g24(c2_24[x]) + green;
							bluepix = b24(c2_24[x]) + blue;
							c2_24[x] = rgbtocolor24(klamp255((redpix/(gp+bp))*red), 
								                    klamp255((greenpix/(rp+bp))*green), 
											        klamp255((bluepix/(rp+gp))*blue));
						}
					}
					break;
				case 32:
					for(y = 0; y < h; y++) 
					{
						c1_32 = scanline32(f1, y);
						c2_32 = scanline32(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r32(c1_32[x]) + red;
							gp = g32(c1_32[x]) + green;
							bp = b32(c1_32[x]) + blue;
							redpix = r32(c2_32[x]) + red;
							greenpix = g32(c2_32[x]) + green;
							bluepix = b32(c2_32[x]) + blue;
							c2_32[x] = rgbtocolor32(klamp255((redpix/(gp+bp))*red), 
								                    klamp255((greenpix/(rp+bp))*green), 
											        klamp255((bluepix/(rp+gp))*blue));
						}
					}
					break;
			}
			break;
		case 8:
			switch (f1.pixelformat)
			{
				case 16:
					for(y = 0; y < h; y++) 
					{
						c1_16 = scanline16(f1, y);
						c2_16 = scanline16(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r16(c1_16[x]) + red;
							gp = g16(c1_16[x]) + green;
							bp = b16(c1_16[x]) + blue;
							redpix = r16(c2_16[x]) + red;
							greenpix = g16(c2_16[x]) + green;
							bluepix = b16(c2_16[x]) + blue;
							c2_16[x] = rgbtocolor16(klamp255(((gp+bp)/redpix)*red), 
								                    klamp255(((rp+bp)/greenpix)*green), 
											        klamp255(((rp+gp)/bluepix)*blue));
						}
					}
					break;
				case 24:
					for(y = 0; y < h; y++) 
					{
						c1_24 = scanline24(f1, y);
						c2_24 = scanline24(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r24(c1_24[x]) + red;
							gp = g24(c1_24[x]) + green;
							bp = b24(c1_24[x]) + blue;
							redpix = r24(c2_24[x]) + red;
							greenpix = g24(c2_24[x]) + green;
							bluepix = b24(c2_24[x]) + blue;
							c2_24[x] = rgbtocolor24(klamp255(((gp+bp)/redpix)*red), 
								                    klamp255(((rp+bp)/greenpix)*green), 
											        klamp255(((rp+gp)/bluepix)*blue));
						}
					}
					break;
				case 32:
					for(y = 0; y < h; y++) 
					{
						c1_32 = scanline32(f1, y);
						c2_32 = scanline32(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r32(c1_32[x]) + red;
							gp = g32(c1_32[x]) + green;
							bp = b32(c1_32[x]) + blue;
							redpix = r32(c2_32[x]) + red;
							greenpix = g32(c2_32[x]) + green;
							bluepix = b32(c2_32[x]) + blue;
							c2_32[x] = rgbtocolor32(klamp255(((gp+bp)/redpix)*red), 
								                    klamp255(((rp+bp)/greenpix)*green), 
											        klamp255(((rp+gp)/bluepix)*blue));
						}
					}
					break;
			}
			break;
		case 9:
			switch (f1.pixelformat)
			{
				case 16:
					for(y = 0; y < h; y++) 
					{
						c1_16 = scanline16(f1, y);
						c2_16 = scanline16(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r16(c1_16[x]) + 1;
							gp = g16(c1_16[x]) + 1;
							bp = b16(c1_16[x]) + 1;
							redpix = r16(c2_16[x]) + 1;
							greenpix = g16(c2_16[x]) + 1;
							bluepix = b16(c2_16[x]) + 1;
							c2_16[x] = rgbtocolor16(klamp255((red + ((gp+bp)/redpix))*red), 
								                    klamp255((green + ((rp+bp)/greenpix))*green), 
											        klamp255((blue + ((rp+gp)/bluepix))*blue));
						}
					}
					break;
				case 24:
					for(y = 0; y < h; y++) 
					{
						c1_24 = scanline24(f1, y);
						c2_24 = scanline24(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r24(c1_24[x]) + 1;
							gp = g24(c1_24[x]) + 1;
							bp = b24(c1_24[x]) + 1;
							redpix = r24(c2_24[x]) + 1;
							greenpix = g24(c2_24[x]) + 1;
							bluepix = b24(c2_24[x]) + 1;
							c2_24[x] = rgbtocolor24(klamp255((red + ((gp+bp)/redpix))*red), 
								                    klamp255((green + ((rp+bp)/greenpix))*green), 
											        klamp255((blue + ((rp+gp)/bluepix))*blue));
						}
					}
					break;
				case 32:
					for(y = 0; y < h; y++) 
					{
						c1_32 = scanline32(f1, y);
						c2_32 = scanline32(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r32(c1_32[x]) + 1;
							gp = g32(c1_32[x]) + 1;
							bp = b32(c1_32[x]) + 1;
							redpix = r32(c2_32[x]) + 1;
							greenpix = g32(c2_32[x]) + 1;
							bluepix = b32(c2_32[x]) + 1;
							c2_32[x] = rgbtocolor32(klamp255((red + ((gp+bp)/redpix))*red), 
								                    klamp255((green + ((rp+bp)/greenpix))*green), 
											        klamp255((blue + ((rp+gp)/bluepix))*blue));
						}
					}
					break;
			}
			break;
		case 10:
			switch (f1.pixelformat)
			{
				case 16:
					for(y = 0; y < h; y++) 
					{
						c1_16 = scanline16(f1, y);
						c2_16 = scanline16(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r16(c1_16[x]) + 1;
							gp = g16(c1_16[x]) + 1;
							bp = b16(c1_16[x]) + 1;
							redpix = r16(c2_16[x]) + 1;
							greenpix = g16(c2_16[x]) + 1;
							bluepix = b16(c2_16[x]) + 1;
							c2_16[x] = rgbtocolor16(klamp255((((gp*green)+(bp*blue))/redpix)*red), 
								                    klamp255((((rp*red)+(bp*blue))/greenpix)*green), 
											        klamp255((((rp*red)+(gp*green))/bluepix)*blue));
						}
					}
					break;
				case 24:
					for(y = 0; y < h; y++) 
					{
						c1_24 = scanline24(f1, y);
						c2_24 = scanline24(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r24(c1_24[x]) + 1;
							gp = g24(c1_24[x]) + 1;
							bp = b24(c1_24[x]) + 1;
							redpix = r24(c2_24[x]) + 1;
							greenpix = g24(c2_24[x]) + 1;
							bluepix = b24(c2_24[x]) + 1;
							c2_24[x] = rgbtocolor24(klamp255((((gp*green)+(bp*blue))/redpix)*red), 
								                    klamp255((((rp*red)+(bp*blue))/greenpix)*green), 
											        klamp255((((rp*red)+(gp*green))/bluepix)*blue));
						}
					}
					break;
				case 32:
					for(y = 0; y < h; y++) 
					{
						c1_32 = scanline32(f1, y);
						c2_32 = scanline32(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r32(c1_32[x]) + 1;
							gp = g32(c1_32[x]) + 1;
							bp = b32(c1_32[x]) + 1;
							redpix = r32(c2_32[x]) + 1;
							greenpix = g32(c2_32[x]) + 1;
							bluepix = b32(c2_32[x]) + 1;
							c2_32[x] = rgbtocolor32(klamp255((((gp*green)+(bp*blue))/redpix)*red), 
								                    klamp255((((rp*red)+(bp*blue))/greenpix)*green), 
											        klamp255((((rp*red)+(gp*green))/bluepix)*blue));
						}
					}
					break;
			}
			break;
		case 11:
			switch (f1.pixelformat)
			{
				case 16:
					for(y = 0; y < h; y++) 
					{
						c1_16 = scanline16(f1, y);
						c2_16 = scanline16(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r16(c1_16[x]) + 1;
							gp = g16(c1_16[x]) + 1;
							bp = b16(c1_16[x]) + 1;
							redpix = r16(c2_16[x]) + 1;
							greenpix = g16(c2_16[x]) + 1;
							bluepix = b16(c2_16[x]) + 1;
							c2_16[x] = rgbtocolor16(klamp255((((gp*green)+(bp*blue))/(redpix*red))*red), 
								                    klamp255((((rp*red)+(bp*blue))/(greenpix*green))*green), 
											        klamp255((((rp*red)+(gp*green))/(bluepix*blue))*blue));
						}
					}
					break;
				case 24:
					for(y = 0; y < h; y++) 
					{
						c1_24 = scanline24(f1, y);
						c2_24 = scanline24(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r24(c1_24[x]) + 1;
							gp = g24(c1_24[x]) + 1;
							bp = b24(c1_24[x]) + 1;
							redpix = r24(c2_24[x]) + 1;
							greenpix = g24(c2_24[x]) + 1;
							bluepix = b24(c2_24[x]) + 1;
							c2_24[x] = rgbtocolor24(klamp255((((gp*green)+(bp*blue))/(redpix*red))*red), 
								                    klamp255((((rp*red)+(bp*blue))/(greenpix*green))*green), 
											        klamp255((((rp*red)+(gp*green))/(bluepix*blue))*blue));
						}
					}
					break;
				case 32:
					for(y = 0; y < h; y++) 
					{
						c1_32 = scanline32(f1, y);
						c2_32 = scanline32(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r32(c1_32[x]) + 1;
							gp = g32(c1_32[x]) + 1;
							bp = b32(c1_32[x]) + 1;
							redpix = r32(c2_32[x]) + 1;
							greenpix = g32(c2_32[x]) + 1;
							bluepix = b32(c2_32[x]) + 1;
							c2_32[x] = rgbtocolor32(klamp255((((gp*green)+(bp*blue))/(redpix*red))*red), 
								                    klamp255((((rp*red)+(bp*blue))/(greenpix*green))*green), 
											        klamp255((((rp*red)+(gp*green))/(bluepix*blue))*blue));
						}
					}
					break;
			}
			break;
		case 12:
			switch (f1.pixelformat)
			{
				case 16:
					for(y = 0; y < h; y++) 
					{
						c1_16 = scanline16(f1, y);
						c2_16 = scanline16(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r16(c1_16[x]) + 1;
							gp = g16(c1_16[x]) + 1;
							bp = b16(c1_16[x]) + 1;
							redpix = r16(c2_16[x]) + 1;
							greenpix = g16(c2_16[x]) + 1;
							bluepix = b16(c2_16[x]) + 1;
							c2_16[x] = rgbtocolor16(klamp255(((redpix*red)/(gp*green)+(bp*blue))*red), 
								                    klamp255(((greenpix*green)/(rp*red)+(bp*blue))*green), 
											        klamp255(((bluepix*blue)/(rp*red)+(gp*green))*blue));
						}
					}
					break;
				case 24:
					for(y = 0; y < h; y++) 
					{
						c1_24 = scanline24(f1, y);
						c2_24 = scanline24(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r24(c1_24[x]) + 1;
							gp = g24(c1_24[x]) + 1;
							bp = b24(c1_24[x]) + 1;
							redpix = r24(c2_24[x]) + 1;
							greenpix = g24(c2_24[x]) + 1;
							bluepix = b24(c2_24[x]) + 1;
							c2_24[x] = rgbtocolor24(klamp255(((redpix*red)/(gp*green)+(bp*blue))*red), 
								                    klamp255(((greenpix*green)/(rp*red)+(bp*blue))*green), 
											        klamp255(((bluepix*blue)/(rp*red)+(gp*green))*blue));
						}
					}
					break;
				case 32:
					for(y = 0; y < h; y++) 
					{
						c1_32 = scanline32(f1, y);
						c2_32 = scanline32(f2, y);
						for(x = 0; x < w; x++)
						{
							rp = r32(c1_32[x]) + 1;
							gp = g32(c1_32[x]) + 1;
							bp = b32(c1_32[x]) + 1;
							redpix = r32(c2_32[x]) + 1;
							greenpix = g32(c2_32[x]) + 1;
							bluepix = b32(c2_32[x]) + 1;
							c2_32[x] = rgbtocolor32(klamp255(((redpix*red)/(gp*green)+(bp*blue))*red), 
								                    klamp255(((greenpix*green)/(rp*red)+(bp*blue))*green), 
											        klamp255(((bluepix*blue)/(rp*red)+(gp*green))*blue));
						}
					}
					break;
			}
			break;
	}

}