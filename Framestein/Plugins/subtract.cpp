// 0.20
// - from c to c++
// - pixelformat-aware

#include <stdlib.h>
#include <string.h>
#include "plugin.h"
#include "pixels.h"

void perform_effect(_frame f, _args a)
{
	pixels p(f);
	char *t;
	byte tr, tg, tb, r=0, g=0, b=0;

	if(!a.s) return;

	r = atoi(a.s);
	if(t = strstr(a.s, " "))
	{
		g = atoi(t+1);
		if (t = strstr(t+1, " "))
			b = atoi(t+1);
	}

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
