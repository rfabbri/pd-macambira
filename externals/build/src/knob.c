/* 
 * On MinGW, it dies here:
 * knob.o(.text+0x1a58):knob.c: undefined reference to `sys_noloadbang'
 */
#ifndef WIN32
#include "../../footils/knob/knob.c"
#endif

