#include <stdlib.h>
#include <string.h>
#include "plugin.h"
#include "pixels.h"

void perform_copy(_frame f1, _frame f2, _args a)
{
	pixels px(f2);
	pixel16 *p16;
	pixel24 *p24;
	pixel32 *p32;
	int tile, tiley=-1, prevy=-1;
	char *t;

	tile = atoi(a.s);
	if(tile<=0) tile=2;
	if(t = strstr(a.s, " "))
	{
		tiley = atoi(t+1);
		if(tiley<=0) tiley=2;
	}

	// i could have the switch() inside the while-loop..
	// but is it more efficient this way?

	switch(f1.pixelformat)
	{
		case 16:
			while(!px.eof()) {
				if(px.y != prevy) {
					p16 = scanline16(f1, (px.y * (tiley>0 ? tiley : tile))%f1.height);
					prevy = px.y;
				}
				px.dot16(p16[ (px.x * tile)%f1.width ]);
				px.next();
			}
			break;
		case 24:
			while(!px.eof()) {
				if(px.y != prevy) {
					p24 = scanline24(f1, (px.y * (tiley>0 ? tiley : tile))%f1.height);
					prevy = px.y;
				}
				px.dot24(p24[ (px.x * tile)%f1.width ]);
				px.next();
			}
			break;
		case 32:
			while(!px.eof()) {
				if(px.y != prevy) {
					p32 = scanline32(f1, (px.y * (tiley>0 ? tiley : tile))%f1.height);
					prevy = px.y;
				}
				px.dot32(p32[ (px.x * tile)%f1.width ]);
				px.next();
			}
			break;
	}
}
