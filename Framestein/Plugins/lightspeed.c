#include <stdlib.h>
#include "plugin.h"

void perform_effect(_frame f, _args a)
{
	int pos, y, pixelsize=f.pixelformat/8;

	if(!a.s) pos = f.height / 2;
	else pos = atoi(a.s);

	if(pos<=0 || pos>=f.height) pos = f.height / 2;

	for(y=pos; y>=0; y--)
	{
		memcpy(scanline(f, y), scanline(f, y+1), pixelsize*f.width);
	}
}
