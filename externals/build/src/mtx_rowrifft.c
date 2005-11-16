/* 
 * hack to work around my lack of Windows linking knowledge
 * <hans@at.or.at>
 */
#ifdef WIN32
#include "../../../pd/src/d_mayer_fft.c"
#endif

#include "../../iem/iemmatrix/src/mtx_matrix.c"
#include "../../iem/iemmatrix/src/mtx_rowrifft.c"
void iemmatrix_sources_setup(void)
{
 
}
