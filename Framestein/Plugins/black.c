#include <stdlib.h>
#include "plugin.h"

void perform_effect(_frame f, _args a)
{
	memset(f.bits, 0, f.height*f.lpitch);
}

void perform_copy(_frame f1, _frame f2, _args a)
{
	memset(f1.bits, 0, f1.height*f1.lpitch);
	memset(f2.bits, 0, f2.height*f2.lpitch);
}
