//#include <m_pd.h>
#include "m_imp.h"
#include <math.h>
#define MAXFLOAT  1e18f;
#define LOGTEN 2.302585092994

/* NT and OSX don't appear to have single-precision ANSI math */
#if defined(WIN32) || defined(MACOSX)
#define sinf sin
#define cosf cos
#define atanf atan
#define atan2f atan2
#define sqrtf sqrt
#define logf log
#define expf exp
#define fabsf fabs
#define powf pow
#endif
