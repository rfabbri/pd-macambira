#ifdef WIN32
#define setenv(a,b,c) _putenv(a)
#endif
#include "../../cxc/ENV.c"
