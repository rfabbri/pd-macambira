// this is just an example of using the pixels-class..

#include "plugin.h"
#include "pixels.h"

void perform_effect(_frame f, _args a)
{
	pixels p(f);
	while(!p.eof())
	{
		p.putrgb(0, p.green(), 0);
		p.next();
	}
}
