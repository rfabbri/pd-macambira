#include <gfsmConfig.h>

/* Define this for verbose memory debugging */
//#define GFSM_DEBUG_VERBOSE

#ifdef GFSM_DEBUG_VERBOSE
# include <gfsmDebug.h>
# define GFSM_INIT   gfsm_debug_init();
# define GFSM_FINISH gfsm_debug_finish(); gfsm_debug_print();
#else
# define GFSM_INIT
# define GFSM_FINISH
#endif
