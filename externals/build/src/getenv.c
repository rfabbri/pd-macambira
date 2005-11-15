#ifdef WIN32
#define setenv(a,b,c) _putenv(a)
#endif
#include "../../motex/getenv.c"
