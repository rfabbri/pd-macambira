/* windows compatibility stuff - cr 2004 */

#include "../../../src/m_pd.h"

#define setenv(a,b,c) _putenv(a)
#define drand48() ((double)rand()/RAND_MAX)
#define srand48(n) srand((n));
#define expm1(e) exp(e)-1
#define bzero(p,n) memset(p,0,n)
#define PROT_READ 1
#define MAP_PRIVATE 2
#define O_NONBLOCK 1
#define O_NDELAY O_NONBLOCK
