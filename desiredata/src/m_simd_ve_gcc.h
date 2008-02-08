/* SIMD functionality for Apple Velocity Engine (AltiVec) with GCC compiler; added by T.Grill */
#ifndef __M_SIMD_VE_GCC_H
#define __M_SIMD_VE_GCC_H
#include "m_pd.h"

/* SIMD functions for VE with GCC */
t_int *sigwrap_perf_simd(t_int *w);
t_int *sigsqrt_perf_simd(t_int *w);
t_int *sigrsqrt_perf_simd(t_int *w);

/* SIMD not implemented */
#define env_tilde_accum_simd       env_tilde_accum_8
#define copyvec_simd_unalignedsrc  copyvec_8
/* #define sum_vecsimd                sumvec_8 */
#define line_tilde_slope_simd      line_tilde_slope
#define peakvec_simd               peakvec

#endif /* __M_SIMD_VE_GCC_H */
