// 242.rgbavg -- does funky pixel averaging on an input image.
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

#pragma warning( disable : 4761 )	// that's why it's funky !!

void perform_effect(struct frame f, struct args a)
{
	short x,y;
	long red, green, blue;
	short alpha, mode = 0;
	byte bits = f.pixelformat/8;
	long redpix, greenpix, bluepix;
	pixel16 *c16;
	pixel24 *c24;
	pixel32 *c32;
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

	// perform routines do different pixel averagings in the main loop
	switch (mode)
	{
		case 1:	// output: color / (sum of other two colors)
			switch (f.pixelformat)
			{
				case 16:
					for(y = 0; y < f.height; y++) 
					{
						c16 = scanline16(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r16(c16[x]) + red;
							greenpix = g16(c16[x]) + green;
							bluepix = b16(c16[x]) + blue;
							c16[x] = rgbtocolor16(klamp255(redpix/(greenpix+bluepix)), 
								                  klamp255(greenpix/(redpix+bluepix)), 
											      klamp255(bluepix/(redpix+greenpix)));
						}
					}
					break;
				case 24:
					for(y = 0; y < f.height; y++) 
					{
						c24 = scanline24(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r24(c24[x]) + red;
							greenpix = g24(c24[x]) + green;
							bluepix = b24(c24[x]) + blue;
							c24[x] = rgbtocolor24(klamp255(redpix/(greenpix+bluepix)), 
								                  klamp255(greenpix/(redpix+bluepix)), 
											      klamp255(bluepix/(redpix+greenpix)));
						}
					}
					break;
				case 32:
					for(y = 0; y < f.height; y++) 
					{
						c32 = scanline32(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r32(c32[x]) + red;
							greenpix = g32(c32[x]) + green;
							bluepix = b32(c32[x]) + blue;
							c32[x] = rgbtocolor32(klamp255(redpix/(greenpix+bluepix)), 
								                  klamp255(greenpix/(redpix+bluepix)), 
											      klamp255(bluepix/(redpix+greenpix)));
						}
					}
					break;
			}
			break;
		case 2:	// output: (sum of two colors) / third color
			switch (f.pixelformat)
			{
				case 16:
					for(y = 0; y < f.height; y++) 
					{
						c16 = scanline16(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r16(c16[x]) + red;
							greenpix = g16(c16[x]) + green;
							bluepix = b16(c16[x]) + blue;
							c16[x] = rgbtocolor16(klamp255((greenpix+bluepix)/redpix), 
								                  klamp255((redpix+bluepix)/greenpix), 
											      klamp255((redpix+greenpix)/bluepix));
						}
					}
					break;
				case 24:
					for(y = 0; y < f.height; y++) 
					{
						c24 = scanline24(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r24(c24[x]) + red;
							greenpix = g24(c24[x]) + green;
							bluepix = b24(c24[x]) + blue;
							c24[x] = rgbtocolor24(klamp255((greenpix+bluepix)/redpix), 
								                  klamp255((redpix+bluepix)/greenpix), 
											      klamp255((redpix+greenpix)/bluepix));
						}
					}
					break;
				case 32:
					for(y = 0; y < f.height; y++) 
					{
						c32 = scanline32(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r32(c32[x]) + red;
							greenpix = g32(c32[x]) + green;
							bluepix = b32(c32[x]) + blue;
							c32[x] = rgbtocolor32(klamp255((greenpix+bluepix)/redpix), 
								                  klamp255((redpix+bluepix)/greenpix), 
											      klamp255((redpix+greenpix)/bluepix));
						}
					}
					break;
			}
			break;
		case 3:	// output: color / (sum of other two colors) + color(input)
			switch (f.pixelformat)
			{
				case 16:
					for(y = 0; y < f.height; y++) 
					{
						c16 = scanline16(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r16(c16[x]) + 1;
							greenpix = g16(c16[x]) + 1;
							bluepix = b16(c16[x]) + 1;
							c16[x] = rgbtocolor16(klamp255(redpix/(greenpix+bluepix) + red), 
								                  klamp255(greenpix/(redpix+bluepix) + green), 
											      klamp255(bluepix/(redpix+greenpix) + blue));
						}
					}
					break;
				case 24:
					for(y = 0; y < f.height; y++) 
					{
						c24 = scanline24(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r24(c24[x]) + 1;
							greenpix = g24(c24[x]) + 1;
							bluepix = b24(c24[x]) + 1;
							c24[x] = rgbtocolor24(klamp255(redpix/(greenpix+bluepix) + red), 
								                  klamp255(greenpix/(redpix+bluepix) + green), 
											      klamp255(bluepix/(redpix+greenpix) + blue));
						}
					}
					break;
				case 32:
					for(y = 0; y < f.height; y++) 
					{
						c32 = scanline32(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r32(c32[x]) + 1;
							greenpix = g32(c32[x]) + 1;
							bluepix = b32(c32[x]) + 1;
							c32[x] = rgbtocolor32(klamp255(redpix/(greenpix+bluepix) + red), 
								                  klamp255(greenpix/(redpix+bluepix) + green), 
											      klamp255(bluepix/(redpix+greenpix) + blue));
						}
					}
					break;
			}
			break;
		case 4:
			switch (f.pixelformat)
			{
				case 16:
					for(y = 0; y < f.height; y++) 
					{
						c16 = scanline16(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r16(c16[x]) + 1;
							greenpix = g16(c16[x]) + 1;
							bluepix = b16(c16[x]) + 1;
							c16[x] = rgbtocolor16(klamp255(((greenpix*green)+(bluepix*blue))/redpix), 
								                  klamp255(((redpix*red)+(bluepix*blue))/greenpix), 
											      klamp255(((redpix*red)+(greenpix*green))/bluepix));
						}
					}
					break;
				case 24:
					for(y = 0; y < f.height; y++) 
					{
						c24 = scanline24(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r24(c24[x]) + 1;
							greenpix = g24(c24[x]) + 1;
							bluepix = b24(c24[x]) + 1;
							c24[x] = rgbtocolor24(klamp255(((greenpix*green)+(bluepix*blue))/redpix), 
								                  klamp255(((redpix*red)+(bluepix*blue))/greenpix), 
											      klamp255(((redpix*red)+(greenpix*green))/bluepix));
						}
					}
					break;
				case 32:
					for(y = 0; y < f.height; y++) 
					{
						c32 = scanline32(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r32(c32[x]) + 1;
							greenpix = g32(c32[x]) + 1;
							bluepix = b32(c32[x]) + 1;
							c32[x] = rgbtocolor32(klamp255(((greenpix*green)+(bluepix*blue))/redpix), 
								                  klamp255(((redpix*red)+(bluepix*blue))/greenpix), 
											      klamp255(((redpix*red)+(greenpix*green))/bluepix));
						}
					}
					break;
			}
			break;
		case 5:
			switch (f.pixelformat)
			{
				case 16:
					for(y = 0; y < f.height; y++) 
					{
						c16 = scanline16(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r16(c16[x]) + 1;
							greenpix = g16(c16[x]) + 1;
							bluepix = b16(c16[x]) + 1;
							c16[x] = rgbtocolor16(klamp255(((greenpix*green)+(bluepix*blue))/(redpix*red)), 
								                  klamp255(((redpix*red)+(bluepix*blue))/(greenpix*green)), 
											      klamp255(((redpix*red)+(greenpix*green))/(bluepix*blue)));
						}
					}
					break;
				case 24:
					for(y = 0; y < f.height; y++) 
					{
						c24 = scanline24(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r24(c24[x]) + 1;
							greenpix = g24(c24[x]) + 1;
							bluepix = b24(c24[x]) + 1;
							c24[x] = rgbtocolor24(klamp255(((greenpix*green)+(bluepix*blue))/(redpix*red)), 
								                  klamp255(((redpix*red)+(bluepix*blue))/(greenpix*green)), 
											      klamp255(((redpix*red)+(greenpix*green))/(bluepix*blue)));
						}
					}
					break;
				case 32:
					for(y = 0; y < f.height; y++) 
					{
						c32 = scanline32(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r32(c32[x]) + 1;
							greenpix = g32(c32[x]) + 1;
							bluepix = b32(c32[x]) + 1;
							c32[x] = rgbtocolor32(klamp255(((greenpix*green)+(bluepix*blue))/(redpix*red)), 
								                  klamp255(((redpix*red)+(bluepix*blue))/(greenpix*green)), 
											      klamp255(((redpix*red)+(greenpix*green))/(bluepix*blue)));
						}
					}
					break;
			}
			break;
		case 6:
			switch (f.pixelformat)
			{
				case 16:
					for(y = 0; y < f.height; y++) 
					{
						c16 = scanline16(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r16(c16[x]) + 1;
							greenpix = g16(c16[x]) + 1;
							bluepix = b16(c16[x]) + 1;
							c16[x] = rgbtocolor16(klamp255((redpix*red)/(greenpix*green)+(bluepix*blue)), 
								                  klamp255((greenpix*green)/(redpix*red)+(bluepix*blue)), 
											      klamp255((bluepix*blue)/(redpix*red)+(greenpix*green)));
						}
					}
					break;
				case 24:
					for(y = 0; y < f.height; y++) 
					{
						c24 = scanline24(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r24(c24[x]) + 1;
							greenpix = g24(c24[x]) + 1;
							bluepix = b24(c24[x]) + 1;
							c24[x] = rgbtocolor24(klamp255((redpix*red)/(greenpix*green)+(bluepix*blue)), 
								                  klamp255((greenpix*green)/(redpix*red)+(bluepix*blue)), 
											      klamp255((bluepix*blue)/(redpix*red)+(greenpix*green)));
						}
					}
					break;
				case 32:
					for(y = 0; y < f.height; y++) 
					{
						c32 = scanline32(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r32(c32[x]) + 1;
							greenpix = g32(c32[x]) + 1;
							bluepix = b32(c32[x]) + 1;
							c32[x] = rgbtocolor32(klamp255((redpix*red)/(greenpix*green)+(bluepix*blue)), 
								                  klamp255((greenpix*green)/(redpix*red)+(bluepix*blue)), 
											      klamp255((bluepix*blue)/(redpix*red)+(greenpix*green)));
						}
					}
					break;
			}
			break;
		case 7:
			switch (f.pixelformat)
			{
				case 16:
					for(y = 0; y < f.height; y++) 
					{
						c16 = scanline16(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r16(c16[x]) + red;
							greenpix = g16(c16[x]) + green;
							bluepix = b16(c16[x]) + blue;
							c16[x] = rgbtocolor16(klamp255((redpix/(greenpix+bluepix))*red), 
								                  klamp255((greenpix/(redpix+bluepix))*green), 
											      klamp255((bluepix/(redpix+greenpix))*blue));
						}
					}
					break;
				case 24:
					for(y = 0; y < f.height; y++) 
					{
						c24 = scanline24(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r24(c24[x]) + red;
							greenpix = g24(c24[x]) + green;
							bluepix = b24(c24[x]) + blue;
							c24[x] = rgbtocolor24(klamp255((redpix/(greenpix+bluepix))*red), 
								                  klamp255((greenpix/(redpix+bluepix))*green), 
											      klamp255((bluepix/(redpix+greenpix))*blue));
						}
					}
					break;
				case 32:
					for(y = 0; y < f.height; y++) 
					{
						c32 = scanline32(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r32(c32[x]) + red;
							greenpix = g32(c32[x]) + green;
							bluepix = b32(c32[x]) + blue;
							c32[x] = rgbtocolor32(klamp255((redpix/(greenpix+bluepix))*red), 
								                  klamp255((greenpix/(redpix+bluepix))*green), 
											      klamp255((bluepix/(redpix+greenpix))*blue));
						}
					}
					break;
			}
			break;
		case 8:
			switch (f.pixelformat)
			{
				case 16:
					for(y = 0; y < f.height; y++) 
					{
						c16 = scanline16(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r16(c16[x]) + red;
							greenpix = g16(c16[x]) + green;
							bluepix = b16(c16[x]) + blue;
							c16[x] = rgbtocolor16(klamp255(((greenpix+bluepix)/redpix)*red), 
								                  klamp255(((redpix+bluepix)/greenpix)*green), 
											      klamp255(((redpix+greenpix)/bluepix)*blue));

						}
					}
					break;
				case 24:
					for(y = 0; y < f.height; y++) 
					{
						c24 = scanline24(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r24(c24[x]) + red;
							greenpix = g24(c24[x]) + green;
							bluepix = b24(c24[x]) + blue;
							c24[x] = rgbtocolor24(klamp255(((greenpix+bluepix)/redpix)*red), 
								                  klamp255(((redpix+bluepix)/greenpix)*green), 
											      klamp255(((redpix+greenpix)/bluepix)*blue));
						}
					}
					break;
				case 32:
					for(y = 0; y < f.height; y++) 
					{
						c32 = scanline32(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r32(c32[x]) + red;
							greenpix = g32(c32[x]) + green;
							bluepix = b32(c32[x]) + blue;
							c32[x] = rgbtocolor32(klamp255(((greenpix+bluepix)/redpix)*red), 
								                  klamp255(((redpix+bluepix)/greenpix)*green), 
											      klamp255(((redpix+greenpix)/bluepix)*blue));
						}
					}
					break;
			}
			break;
		case 9:
			switch (f.pixelformat)
			{
				case 16:
					for(y = 0; y < f.height; y++) 
					{
						c16 = scanline16(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r16(c16[x]) + 1;
							greenpix = g16(c16[x]) + 1;
							bluepix = b16(c16[x]) + 1;
							c16[x] = rgbtocolor16(klamp255((red + ((greenpix+bluepix)/redpix))*red), 
								                  klamp255((green + ((redpix+bluepix)/greenpix))*green), 
											      klamp255((blue + ((redpix+greenpix)/bluepix))*blue));
						}
					}
					break;
				case 24:
					for(y = 0; y < f.height; y++) 
					{
						c24 = scanline24(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r24(c24[x]) + 1;
							greenpix = g24(c24[x]) + 1;
							bluepix = b24(c24[x]) + 1;
							c24[x] = rgbtocolor24(klamp255((red + ((greenpix+bluepix)/redpix))*red), 
								                  klamp255((green + ((redpix+bluepix)/greenpix))*green), 
											      klamp255((blue + ((redpix+greenpix)/bluepix))*blue));
						}
					}
					break;
				case 32:
					for(y = 0; y < f.height; y++) 
					{
						c32 = scanline32(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r32(c32[x]) + 1;
							greenpix = g32(c32[x]) + 1;
							bluepix = b32(c32[x]) + 1;
							c32[x] = rgbtocolor32(klamp255((red + ((greenpix+bluepix)/redpix))*red), 
								                  klamp255((green + ((redpix+bluepix)/greenpix))*green), 
											      klamp255((blue + ((redpix+greenpix)/bluepix))*blue));
						}
					}
					break;
			}
			break;
		case 10:
			switch (f.pixelformat)
			{
				case 16:
					for(y = 0; y < f.height; y++) 
					{
						c16 = scanline16(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r16(c16[x]) + 1;
							greenpix = g16(c16[x]) + 1;
							bluepix = b16(c16[x]) + 1;
							c16[x] = rgbtocolor16(klamp255((((greenpix*green)+(bluepix*blue))/redpix)*red), 
								                  klamp255((((redpix*red)+(bluepix*blue))/greenpix)*green), 
											      klamp255((((redpix*red)+(greenpix*green))/bluepix)*blue));
						}
					}
					break;
				case 24:
					for(y = 0; y < f.height; y++) 
					{
						c24 = scanline24(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r24(c24[x]) + 1;
							greenpix = g24(c24[x]) + 1;
							bluepix = b24(c24[x]) + 1;
							c24[x] = rgbtocolor24(klamp255((((greenpix*green)+(bluepix*blue))/redpix)*red), 
								                  klamp255((((redpix*red)+(bluepix*blue))/greenpix)*green), 
											      klamp255((((redpix*red)+(greenpix*green))/bluepix)*blue));
						}
					}
					break;
				case 32:
					for(y = 0; y < f.height; y++) 
					{
						c32 = scanline32(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r32(c32[x]) + 1;
							greenpix = g32(c32[x]) + 1;
							bluepix = b32(c32[x]) + 1;
							c32[x] = rgbtocolor32(klamp255((((greenpix*green)+(bluepix*blue))/redpix)*red), 
								                  klamp255((((redpix*red)+(bluepix*blue))/greenpix)*green), 
											      klamp255((((redpix*red)+(greenpix*green))/bluepix)*blue));
						}
					}
					break;
			}
			break;
		case 11:
			switch (f.pixelformat)
			{
				case 16:
					for(y = 0; y < f.height; y++) 
					{
						c16 = scanline16(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r16(c16[x]) + 1;
							greenpix = g16(c16[x]) + 1;
							bluepix = b16(c16[x]) + 1;
							c16[x] = rgbtocolor16(klamp255((((greenpix*green)+(bluepix*blue))/(redpix*red))*red), 
								                  klamp255((((redpix*red)+(bluepix*blue))/(greenpix*green))*green), 
											      klamp255((((redpix*red)+(greenpix*green))/(bluepix*blue))*blue));
						}
					}
					break;
				case 24:
					for(y = 0; y < f.height; y++) 
					{
						c24 = scanline24(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r24(c24[x]) + 1;
							greenpix = g24(c24[x]) + 1;
							bluepix = b24(c24[x]) + 1;
							c24[x] = rgbtocolor24(klamp255((((greenpix*green)+(bluepix*blue))/(redpix*red))*red), 
								                  klamp255((((redpix*red)+(bluepix*blue))/(greenpix*green))*green), 
											      klamp255((((redpix*red)+(greenpix*green))/(bluepix*blue))*blue));
						}
					}
					break;
				case 32:
					for(y = 0; y < f.height; y++) 
					{
						c32 = scanline32(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r32(c32[x]) + 1;
							greenpix = g32(c32[x]) + 1;
							bluepix = b32(c32[x]) + 1;
							c32[x] = rgbtocolor32(klamp255((((greenpix*green)+(bluepix*blue))/(redpix*red))*red), 
								                  klamp255((((redpix*red)+(bluepix*blue))/(greenpix*green))*green), 
											      klamp255((((redpix*red)+(greenpix*green))/(bluepix*blue))*blue));
						}
					}
					break;
			}
			break;
		case 12:
			switch (f.pixelformat)
			{
				case 16:
					for(y = 0; y < f.height; y++) 
					{
						c16 = scanline16(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r16(c16[x]) + 1;
							greenpix = g16(c16[x]) + 1;
							bluepix = b16(c16[x]) + 1;
							c16[x] = rgbtocolor16(klamp255(((redpix*red)/(greenpix*green)+(bluepix*blue))*red), 
								                  klamp255(((greenpix*green)/(redpix*red)+(bluepix*blue))*green), 
											      klamp255(((bluepix*blue)/(redpix*red)+(greenpix*green))*blue));
						}
					}
					break;
				case 24:
					for(y = 0; y < f.height; y++) 
					{
						c24 = scanline24(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r24(c24[x]) + 1;
							greenpix = g24(c24[x]) + 1;
							bluepix = b24(c24[x]) + 1;
							c24[x] = rgbtocolor24(klamp255(((redpix*red)/(greenpix*green)+(bluepix*blue))*red), 
								                  klamp255(((greenpix*green)/(redpix*red)+(bluepix*blue))*green), 
											      klamp255(((bluepix*blue)/(redpix*red)+(greenpix*green))*blue));
						}
					}
					break;
				case 32:
					for(y = 0; y < f.height; y++) 
					{
						c32 = scanline32(f, y);
						for(x = 0; x < f.width; x++)
						{
							redpix = r32(c32[x]) + 1;
							greenpix = g32(c32[x]) + 1;
							bluepix = b32(c32[x]) + 1;
							c32[x] = rgbtocolor32(klamp255(((redpix*red)/(greenpix*green)+(bluepix*blue))*red), 
								                  klamp255(((greenpix*green)/(redpix*red)+(bluepix*blue))*green), 
											      klamp255(((bluepix*blue)/(redpix*red)+(greenpix*green))*blue));
						}
					}
					break;
			}
			break;
	}

}

void perform_copy(struct frame f1, struct frame f2, struct args a)
{
	printf("Using rgbavg as copy operation does nothing!\n");
}
