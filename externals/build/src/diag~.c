#ifndef __i386__
#define IS_DENORMAL(f) f
#else
#include "../../creb/include/extlib_util.h"
#endif
#ifndef __APPLE__
#include "../../creb/modules/diag.c"
#endif
