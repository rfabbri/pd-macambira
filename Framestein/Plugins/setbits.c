#include <stdlib.h>
#include "plugin.h"

INFO("set image data to a given value")

void perform_effect(_frame f, _args a)
{
	unsigned char c;

	if(!a.s) return;
	c = atoi(a.s);
	memset(f.bits, c, f.height*f.lpitch);
}
