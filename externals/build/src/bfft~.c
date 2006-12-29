/* 
 * hack to work around my lack of Windows linking knowledge
 * <hans@at.or.at>
 */
#ifdef WIN32
#include "../../../pd/src/d_fft_mayer.c"
#endif

#include "../../creb/modules/bfft~.c"
