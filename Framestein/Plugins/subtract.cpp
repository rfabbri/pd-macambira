//
// subtract - subtract red/green/blue color from image
//
// 0.32
// using arguments-class
//
// 0.20
// - from c to c++
// - pixelformat-aware

#include <stdlib.h>
#include <string.h>
#include "plugin.h"
#include "pixels.h"

INFO("subtract color components");

void perform_effect(_frame f, _args a)
{
	arguments ar(a.s);

	if(ar.count()!=3)
	{
		printf("usage: subtract <red> <green> <blue>\n");
		return;
	}

	byte r, g, b;

	r = atoi(ar[0]);
	g = atoi(ar[1]);
	b = atoi(ar[2]);

	pixels p(f);
	byte tr, tg, tb;

	while(!p.eof())
	{
		tr = p.red();
		tg = p.green();
		tb = p.blue();

		p.putrgb(
		 tr>r ? tr-r : 0,
		 tg>g ? tg-g : 0,
		 tb>b ? tb-b : 0
		);

		p.next();
	}
}
