#ifndef __i386__
#define IS_DENORMAL(f) f
#else
#include "../../creb/include/extlib_util.h"
#endif
#include "../../../pd/src/d_mayer_fft.c"
#include "../../creb/modules/bdiag.c"
