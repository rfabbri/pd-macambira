#include "plugin.h"

INFO("ask fs.frame to bang the receive given as 1st argument")

// can be used at the end of a processing chain to inform when the processing is done.
// very useful.

void perform_effect(_frame f, _args a)
{
	if(!a.s) return;

	sprintf(a.ret, "%s=0", a.s);	// bang receive given in a.s
}
