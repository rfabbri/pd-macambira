#include <stdlib.h>
#include "plugin.h"

void perform_copy(_frame f1, _frame f2, _args a)
{
	int step, w, h, y, pixelsize=f1.pixelformat/8;

	if(!a.s) return;

	step = atoi(a.s);
	if(step<=0) step=1;

	w = f1.width<f2.width ? f1.width : f2.width;
	h = f1.height<f2.height ? f1.height : f2.height;

	for(y=0; y<h; y+=step)
	{
		memcpy(scanline(f2, y), scanline(f1, y), pixelsize*w);
	}
}
