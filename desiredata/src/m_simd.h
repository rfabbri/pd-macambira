/* Definitions for SIMD functionality; added by T.Grill */
#ifndef __M_SIMD_H
#define __M_SIMD_H

/* general vector functions */
void zerovec_8(t_float *dst,int n);
void setvec_8(t_float *dst,t_float v,int n);
void copyvec_8(t_float *dst,const t_float *src,int n);
void addvec_8(t_float *dst,const t_float *src,int n);
void testcopyvec_8(t_float *dst,const t_float *src,int n);
void testaddvec_8(t_float *dst,const t_float *src,int n);

#define SIMD_BYTEALIGN (128/8)

/* how many floats do we calculate in the loop of a SIMD codelet? */
#define SIMD_BLOCK 16  /* must be a power of 2 */

//#if defined(__GNUC__)  && (defined(_X86_) || defined(__i386__) || defined(__i586__) || defined(__i686__))
#ifdef SIMD_SSE /* Intel SSE with GNU C */ /* ought to add this in configure.in */
t_int *sigwrap_perf_simd(t_int *w);
void line_tilde_slope_simd(t_float* out, t_int n, t_float* value, t_float* slopes, t_float* slopestep);
float env_tilde_accum_simd(t_float* in, t_float* hp, t_int n);
t_int* sigsqrt_perf_simd(t_int *w);
t_int* sigrsqrt_perf_simd(t_int *w);
float peakvec_simd(t_float* vec, t_int n, t_float cur_max);
#endif

//#if defined(__GNUC__) && defined(__POWERPC__) && defined(__ALTIVEC__)
#ifdef SIMD_ALTIVEC /* Altivec with GNU C  ( -faltivec must be given as a compiler option! ) */ /* ought to add this in configure.in */
    #include "m_simd_ve_gcc.h"
#endif

#ifdef DONTUSESIMD
    /* This is used when there's no implementation of SIMD code for the current platform and/or compiler */
    /* These are the functions that can be coded for SIMD */
    #define zero_perf_simd          zero_perf8
    #define copy_perf_simd          copy_perf8
    #define sig_tilde_perf_simd     sig_tilde_perf8
    #define sigwrap_perf_simd       sigwrap_perform
    #define line_tilde_slope_simd   line_tilde_slope
    #define env_tilde_accum_simd    env_tilde_accum_8 /* it's a bad place to set that here since there's no public prototype */
    #define plus_perf_simd          plus_perf8
    #define scalarplus_perf_simd    scalarplus_perf8
    #define minus_perf_simd         minus_perf8
    #define scalarminus_perf_simd   scalarminus_perf8
    #define times_perf_simd         times_perf8
    #define scalartimes_perf_simd   scalartimes_perf8
    #define sqr_perf_simd           sqr_perf8
    #define over_perf_simd          over_perf8
    #define scalarover_perf_simd    scalarover_perf8
    #define min_perf_simd           min_perf8
    #define scalarmin_perf_simd     scalarmin_perf8
    #define max_perf_simd           max_perf8
    #define scalarmax_perf_simd     scalarmax_perf8
    #define clip_perf_simd          clip_perform  /* SIMD not implemented */
    #define sigwrap_perf_simd       sigwrap_perform  /* SIMD not implemented */
    #define sigsqrt_perf_simd       sigsqrt_perform  /* SIMD not implemented */
    #define sigrsqrt_perf_simd      sigrsqrt_perform /* SIMD not implemented */
    #define peakvec_simd            peakvec
    /* #define sum_vecsimd                sumvec_8 */
#endif

/* check if n meets the requirements for SIMD codelets */
#define SIMD_CHKCNT(n) ( ((n)&(SIMD_BLOCK-1)) == 0 )
/* check if a pointer is correctly aligned for SIMD codelets */
#define SIMD_CHKALIGN(ptr) ( ((size_t)(ptr) & (SIMD_BYTEALIGN-1)) == 0 )
/* check n and 1 pointer at once */
#define SIMD_CHECK1(n,ptr1) (SIMD_CHKCNT(n) && SIMD_CHKALIGN(ptr1) && simd_runtime_check())
/* check n and 2 pointers at once */
#define SIMD_CHECK2(n,ptr1,ptr2) (SIMD_CHKCNT(n) && SIMD_CHKALIGN(ptr1) && SIMD_CHKALIGN(ptr2) && simd_runtime_check())
/* check n and 3 pointers at once */
#define SIMD_CHECK3(n,ptr1,ptr2,ptr3) (SIMD_CHKCNT(n) && SIMD_CHKALIGN(ptr1) && SIMD_CHKALIGN(ptr2) && SIMD_CHKALIGN(ptr3) && simd_runtime_check())

/* T.Grill - bit alignment for signal vectors (must be a multiple of 8!) */
/* if undefined no alignment occurs */
#ifdef SIMD_BYTEALIGN
    #define VECTORALIGNMENT (SIMD_BYTEALIGN*8)
#else
    #define VECTORALIGNMENT 128
#endif

#endif /* __M_SIMD_H */
