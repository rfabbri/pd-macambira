#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "plugin.h"
#include "pixels.h"

INFO("plot pixel to vframe")

// plot exists to provide plotting to vframe - much faster than
// plot with fs.draw which goes thru TCP/IP.

void perform_effect(_frame f, _args a)
{
	if(!a.s) return;

	char *t=strstr(a.s, " ");
	if(!t) return;

	t[0]=0;

	int x=atoi(a.s);
	int y=atoi(t+1);

	pixels p(f);
	p.moveto(x, y);
	p.putrgb(255, 255, 255);
}
