#ifndef __i386__
#define IS_DENORMAL(f) f
#endif
/* this doesn't work on Mac OS X now <hans@eds.org> */
#ifndef __APPLE__
#include "../../creb/modules/bdiag.c"
#endif
