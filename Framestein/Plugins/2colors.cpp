#include <stdlib.h>
#include "plugin.h"
#include "pixels.h"

// 2colors: make the image consist of black and white pixels only.
// 	    I haven't tested this yet and it most likely WILL NOT WORK!!!

void perform_effect(_frame f, _args a)
{
	pixels p(f);
	int i=127, o; // intensity

	if(a.s) i=atoi(a.s);

	while(!p.eof())
	{
		o = (p.red() + p.green() + p.blue()) / 3;
		if(i<o) p.putrgb(0, 0, 0); else p.putrgb(255, 255, 255);
		p.next();
	}
}

INFO("go black and white")
