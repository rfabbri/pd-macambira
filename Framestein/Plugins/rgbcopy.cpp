#include <stdio.h>
#include <string.h>
#include "plugin.h"
#include "pixels.h"

INFO("mix color components of two images. usage: rgbcopy RG red/green from first, blue from second image")

void perform_copy(_frame f1, _frame f2, _args a)
{
	pixels p1(f1), p2(f2);
	char *t;
	short r=0, g=0, b=0;

	if(!a.s) return;

	if(strstr(a.s, "r") || strstr(a.s, "R")) r=1;
	if(strstr(a.s, "g") || strstr(a.s, "G")) g=1;
	if(strstr(a.s, "b") || strstr(a.s, "B")) b=1;

	while(!p1.eof()&&!p2.eof())
	{
		p2.putrgb(
		 r ? p1.red() : p2.red(),
		 g ? p1.green() : p2.green(),
		 b ? p1.blue() : p2.blue()
		);
		p1.next();
		p2.next();
	}
}
