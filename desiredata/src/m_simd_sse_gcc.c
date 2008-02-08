/* 
    Implementation of SIMD functionality for Intel SSE with GCC compiler
    added by T.Grill
*/

#include "m_pd.h"
#include "m_simd.h"

#if defined(__GNUC__) && (defined(_X86_) || defined(__i386__) || defined(__i586__) || defined(__i686__)) && !(defined DONTUSESIMD)


/* TB: adapted from thomas' vc routines */

/* dst is assumed to be aligned */
void zerovec_simd(t_float *dst,int n)
{
    asm(
		".set T_FLOAT,4                            \n" /* sizeof(t_float) */
		"xorps     %%xmm0, %%xmm0                  \n" /* zero value */
		"shr       $4, %0                          \n"
		
		/* should we do more loop unrolling? */
		/* *dst = 0 */
		"1:                                        \n"
		"movaps    %%xmm0, (%1)                    \n"
		"movaps    %%xmm0, 4*T_FLOAT(%1)           \n"
		"movaps    %%xmm0, 8*T_FLOAT(%1)           \n"
		"movaps    %%xmm0, 12*T_FLOAT(%1)          \n"
		
		"addl      $16*T_FLOAT,%1                  \n"
		"loop      1b                              \n"
		:
		:"c"(n),"r"(dst)
		:"%xmm0");
}

/* dst is assumed to be aligned */
void setvec_simd(t_float *dst,t_float v,int n)
{
    asm(
		".set T_FLOAT,4                            \n" /* sizeof(t_float) */
		"movss     (%2),%%xmm0                     \n"
		"shufps    $0,%%xmm0,%%xmm0                \n" /* load value */
		"shr       $4,%0                           \n"
		
		/* should we do more loop unrolling? */
		/* *dst = v */
		"1:                                        \n"
		"movaps    %%xmm0, (%1)                    \n"
		"movaps    %%xmm0, 4*T_FLOAT(%1)           \n"
		"movaps    %%xmm0, 8*T_FLOAT(%1)           \n"
		"movaps    %%xmm0, 12*T_FLOAT(%1)          \n"
		
		"addl      $16*T_FLOAT,%1                  \n"
		"loop      1b                              \n"
		:
		:"c"(n),"r"(dst),"r"(&v)
		:"%xmm0");
}


/* dst and src are assumed to be aligned */
void copyvec_simd(t_float *dst,const t_float *src,int n)
{
    asm(
		".set T_FLOAT,4                            \n" /* sizeof(t_float) */
		"shr       $4, %0                          \n"

		/* loop: *dst = *src */
		"1:                                        \n"
		"movaps    (%1), %%xmm0                    \n"
		"movaps    4*T_FLOAT(%1), %%xmm1           \n"
		"movaps    8*T_FLOAT(%1), %%xmm2           \n"
		"movaps    12*T_FLOAT(%1), %%xmm3          \n"
		"movaps    %%xmm0, (%2)                    \n"
		"movaps    %%xmm1, 4*T_FLOAT(%2)           \n"
		"movaps    %%xmm2, 8*T_FLOAT(%2)           \n"
		"movaps    %%xmm3, 12*T_FLOAT(%2)          \n"
		
		
		"addl      $16*T_FLOAT,%1                  \n"
		"addl      $16*T_FLOAT,%2                  \n"
		"loop      1b                              \n"
		:
		:"c"(n),"r"(src),"r"(dst)
		:"%xmm0","%xmm1","%xmm2","%xmm3");
}

/* dst is assumed to be aligned */
void copyvec_simd_unalignedsrc(t_float *dst,const t_float *src,int n)
{
    asm(
		".set T_FLOAT,4                            \n" /* sizeof(t_float) */
		"shr       $4, %0                          \n"
		
		/* loop: *dst = *src */
		"1:                                        \n"
		"movups    (%1), %%xmm0                    \n"
		"movups    4*T_FLOAT(%1), %%xmm1           \n"
		"movups    8*T_FLOAT(%1), %%xmm2           \n"
		"movups    12*T_FLOAT(%1), %%xmm3          \n"
		"movaps    %%xmm0, (%2)                    \n"
		"movaps    %%xmm1, 4*T_FLOAT(%2)           \n"
		"movaps    %%xmm2, 8*T_FLOAT(%2)           \n"
		"movaps    %%xmm3, 12*T_FLOAT(%2)          \n"
		
		
		"addl      $16*T_FLOAT,%1                  \n"
		"addl      $16*T_FLOAT,%2                  \n"
		"loop      1b                              \n"
		:
		:"c"(n),"r"(src),"r"(dst)
		:"%xmm0","%xmm1","%xmm2","%xmm3");
}


/* dst and src are assumed to be aligned */
void addvec_simd(t_float *dst,const t_float *src,int n)
{
    asm(
		".set T_FLOAT,4                            \n" /* sizeof(t_float) */
		"shr       $4, %0                          \n"
		
		/* loop: *dst += *src */
		"1:                                        \n"
		"movaps    (%2,%3),%%xmm0                  \n"
		"movaps    (%1,%3),%%xmm1                  \n"
		"addps     %%xmm0,%%xmm1                   \n"
		"movaps    %%xmm1,(%2,%3)                  \n"
		
		"movaps    4*T_FLOAT(%2,%3),%%xmm0         \n"
		"movaps    4*T_FLOAT(%1,%3),%%xmm1         \n"
		"addps     %%xmm0,%%xmm1                   \n"
		"movaps    %%xmm1,4*T_FLOAT(%2,%3)         \n"
		
		"movaps    8*T_FLOAT(%2,%3),%%xmm0         \n"
		"movaps    8*T_FLOAT(%1,%3),%%xmm1         \n"
		"addps     %%xmm0,%%xmm1                   \n"
		"movaps    %%xmm1,8*T_FLOAT(%2,%3)         \n"
		
		"movaps    12*T_FLOAT(%2,%3),%%xmm0        \n"
		"movaps    12*T_FLOAT(%1,%3),%%xmm1        \n"
		"addps     %%xmm0,%%xmm1                   \n"
		"movaps    %%xmm1,12*T_FLOAT(%2,%3)        \n"
		
		"addl      $16*T_FLOAT,%3                  \n"
		"loop      1b                              \n"
		:
		: "c"(n),"r"(src),"r"(dst),"r"(0)
		: "%xmm0","%xmm1","%xmm2","%xmm3","%xmm4","%xmm5","%xmm6","%xmm7");
}


void testcopyvec_simd(t_float *dst,const t_float *src,int n)
{
	
	asm(
		".section	.rodata                        \n"
		".align 16                                 \n"
		"2:                                        \n"
		".long	1610612736                         \n" /* bitmask */
		".long	1610612736                         \n" /* 0x60000000 */
		".long	1610612736                         \n"
		".long	1610612736                         \n"

		".set T_FLOAT,4                            \n"
		".text                                     \n"
		
		"shr       $4, %0                          \n"
		"movaps    (2b), %%xmm0                    \n" /* xmm0 = bitmask */
		"xorps     %%xmm1, %%xmm1                  \n" /* xmm1 = 0x0     */

		
		"1:                                        \n"
		"movaps    (%1), %%xmm2                    \n"
		"movaps    %%xmm2, %%xmm3                  \n"
		"andps     %%xmm0, %%xmm3                  \n"
		"movaps    %%xmm3, %%xmm4                  \n"
		"cmpneqps  %%xmm0, %%xmm3                  \n"
		"cmpneqps  %%xmm1, %%xmm4                  \n"
		"andps     %%xmm4, %%xmm3                  \n"
		"andps     %%xmm3, %%xmm2                  \n"
		"movaps    %%xmm2, (%2)                    \n"

		"movaps    4*T_FLOAT(%1), %%xmm2           \n"
		"movaps    %%xmm2, %%xmm3                  \n"
		"andps     %%xmm0, %%xmm3                  \n"
		"movaps    %%xmm3, %%xmm4                  \n"
		"cmpneqps  %%xmm0, %%xmm3                  \n"
		"cmpneqps  %%xmm1, %%xmm4                  \n"
		"andps     %%xmm4, %%xmm3                  \n"
		"andps     %%xmm3, %%xmm2                  \n"
		"movaps    %%xmm2, 4*T_FLOAT(%2)           \n"

		"movaps    8*T_FLOAT(%1), %%xmm2           \n"
		"movaps    %%xmm2, %%xmm3                  \n"
		"andps     %%xmm0, %%xmm3                  \n"
		"movaps    %%xmm3, %%xmm4                  \n"
		"cmpneqps  %%xmm0, %%xmm3                  \n"
		"cmpneqps  %%xmm1, %%xmm4                  \n"
		"andps     %%xmm4, %%xmm3                  \n"
		"andps     %%xmm3, %%xmm2                  \n"
		"movaps    %%xmm2, 8*T_FLOAT(%2)           \n"

		"movaps    12*T_FLOAT(%1), %%xmm2          \n"
		"movaps    %%xmm2, %%xmm3                  \n"
		"andps     %%xmm0, %%xmm3                  \n"
		"movaps    %%xmm3, %%xmm4                  \n"
		"cmpneqps  %%xmm0, %%xmm3                  \n"
		"cmpneqps  %%xmm1, %%xmm4                  \n"
		"andps     %%xmm4, %%xmm3                  \n"
		"andps     %%xmm3, %%xmm2                  \n"
		"movaps    %%xmm2, 12*T_FLOAT(%2)          \n"
		
		
		"addl      $16*T_FLOAT,%1                  \n"
		"addl      $16*T_FLOAT,%2                  \n"
		"decl      %0                              \n"
		"jne       1b                              \n"
		:
		:"c"(n),"r"(src),"r"(dst)
		:"%xmm0","%xmm1","%xmm2","%xmm3", "%xmm4");
}


void testaddvec_simd(t_float *dst,const t_float *src,int n)
{
	asm(
		".section	.rodata                        \n"
		".align 16                                 \n"
		"2:                                        \n"
		".long	1610612736                         \n" /* bitmask */
		".long	1610612736                         \n" /* 0x60000000 */
		".long	1610612736                         \n"
		".long	1610612736                         \n"

		".set T_FLOAT,4                            \n"
		".text                                     \n"
		
		"shr       $4, %0                          \n"
		"movaps    (2b), %%xmm0                    \n" /* xmm0 = bitmask */
		"xorps     %%xmm1, %%xmm1                  \n" /* xmm1 = 0x0     */

		
		"1:                                        \n"
		"movaps    (%1), %%xmm2                    \n"
		"movaps    (%2), %%xmm5                    \n"
		"movaps    %%xmm2, %%xmm3                  \n"
		"andps     %%xmm0, %%xmm3                  \n"
		"movaps    %%xmm3, %%xmm4                  \n"
		"cmpneqps  %%xmm0, %%xmm3                  \n"
		"cmpneqps  %%xmm1, %%xmm4                  \n"
		"andps     %%xmm4, %%xmm3                  \n"
		"andps     %%xmm3, %%xmm2                  \n"
		"addps     %%xmm2, %%xmm5                  \n"
		"movaps    %%xmm5, (%2)                    \n"

		"movaps    4*T_FLOAT(%1), %%xmm2           \n"
		"movaps    4*T_FLOAT(%2), %%xmm5           \n"
		"movaps    %%xmm2, %%xmm3                  \n"
		"andps     %%xmm0, %%xmm3                  \n"
		"movaps    %%xmm3, %%xmm4                  \n"
		"cmpneqps  %%xmm0, %%xmm3                  \n"
		"cmpneqps  %%xmm1, %%xmm4                  \n"
		"andps     %%xmm4, %%xmm3                  \n"
		"andps     %%xmm3, %%xmm2                  \n"
		"addps     %%xmm2, %%xmm5                  \n"
		"movaps    %%xmm5, 4*T_FLOAT(%2)           \n"

		"movaps    8*T_FLOAT(%1), %%xmm2           \n"
		"movaps    8*T_FLOAT(%2), %%xmm5           \n"
		"movaps    %%xmm2, %%xmm3                  \n"
		"andps     %%xmm0, %%xmm3                  \n"
		"movaps    %%xmm3, %%xmm4                  \n"
		"cmpneqps  %%xmm0, %%xmm3                  \n"
		"cmpneqps  %%xmm1, %%xmm4                  \n"
		"andps     %%xmm4, %%xmm3                  \n"
		"andps     %%xmm3, %%xmm2                  \n"
		"addps     %%xmm2, %%xmm5                  \n"
		"movaps    %%xmm5, 8*T_FLOAT(%2)           \n"

		"movaps    12*T_FLOAT(%1), %%xmm2          \n"
		"movaps    12*T_FLOAT(%2), %%xmm5          \n"
		"movaps    %%xmm2, %%xmm3                  \n"
		"andps     %%xmm0, %%xmm3                  \n"
		"movaps    %%xmm3, %%xmm4                  \n"
		"cmpneqps  %%xmm0, %%xmm3                  \n"
		"cmpneqps  %%xmm1, %%xmm4                  \n"
		"andps     %%xmm4, %%xmm3                  \n"
		"andps     %%xmm3, %%xmm2                  \n"
		"addps     %%xmm2, %%xmm5                  \n"
		"movaps    %%xmm5, 12*T_FLOAT(%2)          \n"
		
		
		"addl      $16*T_FLOAT,%1                  \n"
		"addl      $16*T_FLOAT,%2                  \n"
		"decl      %0                              \n"
		"jne       1b                              \n"
		:
		:"c"(n),"r"(src),"r"(dst)
		:"%xmm0","%xmm1","%xmm2","%xmm3", "%xmm4", "%xmm5");
}


t_int *zero_perf_simd(t_int *w)
{
    zerovec_simd((t_float *)w[1],w[2]);
    return w+3;
}

t_int *copy_perf_simd(t_int *w)
{
    copyvec_simd((t_float *)w[2],(const t_float *)w[1],w[3]);
    return w+4;
}

t_int *sig_tilde_perf_simd(t_int *w)
{
    setvec_simd((t_float *)w[2],*(const t_float *)w[1],w[3]);
    return w+4;
}


t_int *plus_perf_simd (t_int * w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"shrl      $4, %3                          \n" /* divide by 16 */
    
	/* loop: *out = *in1 + *in2 */
	"1:                                        \n"
	"movaps    (%0,%4), %%xmm0                 \n"
	"movaps    (%1,%4), %%xmm1                 \n"
	"addps     %%xmm1, %%xmm0                  \n"
	"movaps    %%xmm0, (%2,%4)                 \n"
	
	"movaps    4*T_FLOAT(%0,%4), %%xmm2        \n"
	"movaps    4*T_FLOAT(%1,%4), %%xmm3        \n"
	"addps     %%xmm3, %%xmm2                  \n"
	"movaps    %%xmm2, 4*T_FLOAT(%2,%4)        \n"
	
	"movaps    8*T_FLOAT(%0,%4), %%xmm4        \n"
	"movaps    8*T_FLOAT(%1,%4), %%xmm5        \n"
	"addps     %%xmm5, %%xmm4                  \n"
	"movaps    %%xmm4, 8*T_FLOAT(%2,%4)        \n"
	
	"movaps    12*T_FLOAT(%0,%4), %%xmm6       \n"
	"movaps    12*T_FLOAT(%1,%4), %%xmm7       \n"
	"addps     %%xmm7, %%xmm6                  \n"
	"movaps    %%xmm6, 12*T_FLOAT(%2,%4)       \n"
	
	"addl      $16*T_FLOAT, %4                 \n"
	"loop      1b                              \n"
	:
	/* in1, in2, out, n */
	:"r"(w[1]),"r"(w[2]),"r"(w[3]),"c"(w[4]),"r"(0)
	:"%xmm0","%xmm1","%xmm2","%xmm3","%xmm4","%xmm5","%xmm6","%xmm7"
	);
    return w+5;
}


t_int *scalarplus_perf_simd(t_int *w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"movss     (%1), %%xmm0                    \n"
	"shufps    $0, %%xmm0, %%xmm0              \n"
	"shrl      $4, %3                          \n" /* divide by 16 */

	/* loop: *out = *in + value */
	"1:                                        \n"
	"movaps    (%0), %%xmm1                    \n"
	"addps     %%xmm0, %%xmm1                  \n"
	"movaps    %%xmm1, (%2)                    \n"
    
	"movaps    4*T_FLOAT(%0), %%xmm2           \n"
	"addps     %%xmm0, %%xmm2                  \n"
	"movaps    %%xmm2, 4*T_FLOAT(%2)           \n"
	
	"movaps    8*T_FLOAT(%0), %%xmm3           \n"
	"addps     %%xmm0, %%xmm3                  \n"
	"movaps    %%xmm3, 8*T_FLOAT(%2)           \n"
	
	"movaps    12*T_FLOAT(%0), %%xmm4          \n"
	"addps     %%xmm0, %%xmm4                  \n"
	"movaps    %%xmm4, 12*T_FLOAT(%2)          \n"
	
	"addl      $16*T_FLOAT, %0                 \n"
	"addl      $16*T_FLOAT, %2                 \n"
	"loop      1b                              \n"
	:
	/* in, value, out, n */
	:"r"(w[1]),"r"(w[2]),"r"(w[3]),"c"(w[4])
	:"%xmm0", "%xmm1","%xmm2","%xmm3","%xmm4"
	);
    return w+5;
}

t_int *minus_perf_simd(t_int *w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"shrl      $4, %3                          \n" /* divide by 16 */
	
	/* loop: *out = *in1 - *in2 */
	"1:                                        \n"
	"movaps    (%0,%4), %%xmm0                 \n"
	"movaps    (%1,%4), %%xmm1                 \n"
	"subps     %%xmm1, %%xmm0                  \n"
	"movaps    %%xmm0, (%2,%4)                 \n"
	
	"movaps    4*T_FLOAT(%0,%4), %%xmm2        \n"
	"movaps    4*T_FLOAT(%1,%4), %%xmm3        \n"
	"subps     %%xmm3, %%xmm2                  \n"
	"movaps    %%xmm2, 4*T_FLOAT(%2,%4)        \n"
	
	"movaps    8*T_FLOAT(%0,%4), %%xmm4        \n"
	"movaps    8*T_FLOAT(%1,%4), %%xmm5        \n"
	"subps     %%xmm5, %%xmm4                  \n"
	"movaps    %%xmm4, 8*T_FLOAT(%2,%4)        \n"
	
	"movaps    12*T_FLOAT(%0,%4), %%xmm6       \n"
	"movaps    12*T_FLOAT(%1,%4), %%xmm7       \n"
	"subps     %%xmm7, %%xmm6                  \n"
	"movaps    %%xmm6, 12*T_FLOAT(%2,%4)       \n"
	
	"addl      $16*T_FLOAT, %4                 \n"
	"loop      1b                              \n"
	:
	/* in1, in2, out, n */
	:"r"(w[1]),"r"(w[2]),"r"(w[3]),"c"(w[4]),"r"(0)
	:"%xmm0","%xmm1","%xmm2","%xmm3","%xmm4","%xmm5","%xmm6","%xmm7"
	);
    return w+5;
}

t_int* scalarminus_perf_simd(t_int *w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"movss     (%1), %%xmm0                    \n"
	"shufps    $0, %%xmm0, %%xmm0              \n"
	"shrl      $4, %3                          \n" /* divide by 16 */

	/* loop: *out = *in - value */
	"1:                                        \n"
	"movaps    (%0), %%xmm1                    \n"
	"subps     %%xmm0, %%xmm1                  \n"
	"movaps    %%xmm1, (%2)                    \n"
    
	"movaps    4*T_FLOAT(%0), %%xmm2           \n"
	"subps     %%xmm0, %%xmm2                  \n"
	"movaps    %%xmm2, 4*T_FLOAT(%2)           \n"
	
	"movaps    8*T_FLOAT(%0), %%xmm3           \n"
	"subps     %%xmm0, %%xmm3                  \n"
	"movaps    %%xmm3, 8*T_FLOAT(%2)           \n"
	
	"movaps    12*T_FLOAT(%0), %%xmm4          \n"
	"subps     %%xmm0, %%xmm4                  \n"
	"movaps    %%xmm4, 12*T_FLOAT(%2)          \n"
	
	"addl      $16*T_FLOAT, %0                 \n"
	"addl      $16*T_FLOAT, %2                 \n"
	"loop      1b                              \n"
	:
	/* in, value, out, n */
	:"r"(w[1]),"r"(w[2]),"r"(w[3]),"c"(w[4])
	:"%xmm0","%xmm1","%xmm2","%xmm3","%xmm4"
	);
    return w+5;
}


t_int *times_perf_simd(t_int *w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"shrl      $4, %3                          \n" /* divide by 16 */
	
	/* loop: *out = *in1 * *in2 */
	"1:                                        \n"
	"movaps    (%0,%4), %%xmm0                 \n"
	"movaps    (%1,%4), %%xmm1                 \n"
	"mulps     %%xmm1, %%xmm0                  \n"
	"movaps    %%xmm0, (%2,%4)                 \n"
	
	"movaps    4*T_FLOAT(%0,%4), %%xmm2        \n"
	"movaps    4*T_FLOAT(%1,%4), %%xmm3        \n"
	"mulps     %%xmm3, %%xmm2                  \n"
	"movaps    %%xmm2, 4*T_FLOAT(%2,%4)        \n"
	
	"movaps    8*T_FLOAT(%0,%4), %%xmm4        \n"
	"movaps    8*T_FLOAT(%1,%4), %%xmm5        \n"
	"mulps     %%xmm5, %%xmm4                  \n"
	"movaps    %%xmm4, 8*T_FLOAT(%2,%4)        \n"
	
	"movaps    12*T_FLOAT(%0,%4), %%xmm6       \n"
	"movaps    12*T_FLOAT(%1,%4), %%xmm7       \n"
	"mulps     %%xmm7, %%xmm6                  \n"
	"movaps    %%xmm6, 12*T_FLOAT(%2,%4)       \n"
	
	"addl      $16*T_FLOAT, %4                 \n"
	"loop      1b                              \n"
	:
	/* in1, in2, out, n */
	:"r"(w[1]),"r"(w[2]),"r"(w[3]),"c"(w[4]),"r"(0)
	:"%xmm0","%xmm1","%xmm2","%xmm3","%xmm4","%xmm5","%xmm6","%xmm7"
	);
    return w+5;
}

t_int* scalartimes_perf_simd(t_int *w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"movss     (%1), %%xmm0                    \n"
	"shufps    $0, %%xmm0, %%xmm0              \n"
	"shrl      $4, %3                          \n" /* divide by 16 */

	/* loop: *out = *in * value */
	"1:                                        \n"
	"movaps    (%0), %%xmm1                    \n"
	"mulps     %%xmm0, %%xmm1                  \n"
	"movaps    %%xmm1, (%2)                    \n"
    
	"movaps    4*T_FLOAT(%0), %%xmm2           \n"
	"mulps     %%xmm0, %%xmm2                  \n"
	"movaps    %%xmm2, 4*T_FLOAT(%2)           \n"
	
	"movaps    8*T_FLOAT(%0), %%xmm3           \n"
	"mulps     %%xmm0, %%xmm3                  \n"
	"movaps    %%xmm3, 8*T_FLOAT(%2)           \n"
	
	"movaps    12*T_FLOAT(%0), %%xmm4          \n"
	"mulps     %%xmm0, %%xmm4                  \n"
	"movaps    %%xmm4, 12*T_FLOAT(%2)          \n"
	
	"addl      $16*T_FLOAT, %0                 \n"
	"addl      $16*T_FLOAT, %2                 \n"
	"loop      1b                              \n"
	:
	/* in, value, out, n */
	:"r"(w[1]),"r"(w[2]),"r"(w[3]),"c"(w[4])
	:"%xmm0","%xmm1","%xmm2","%xmm3","%xmm4"
	);
    return w+5;
}

t_int *sqr_perf_simd(t_int *w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"shrl      $4, %2                          \n" /* divide by 16 */
	
	/* loop: *out = *in * *in */
	"1:                                        \n"
	"movaps    (%0,%3), %%xmm0                 \n"
	"mulps     %%xmm0, %%xmm0                  \n"
	"movaps    %%xmm0, (%1)                    \n"
    
	"movaps    4*T_FLOAT(%0,%3), %%xmm1        \n"
	"mulps     %%xmm1, %%xmm1                  \n"
	"movaps    %%xmm1, 4*T_FLOAT(%1)           \n"
	
	"movaps    8*T_FLOAT(%0,%3), %%xmm2        \n"
	"mulps     %%xmm2, %%xmm2                  \n"
	"movaps    %%xmm2, 8*T_FLOAT(%1)           \n"
	
	"movaps    12*T_FLOAT(%0,%3), %%xmm3       \n"
	"mulps     %%xmm3, %%xmm3                  \n"
	"movaps    %%xmm3, 12*T_FLOAT(%1)          \n"
	
	"addl      $16*T_FLOAT, %1                 \n"
	"addl      $16*T_FLOAT, %3                 \n"
	"loop      1b                              \n"
	:
	/* in, out, n */
	:"r"(w[1]),"r"(w[2]),"c"(w[3]),"r"(0)
	:"%xmm0","%xmm1","%xmm2","%xmm3"
	);
    return w+4;
}


t_int* over_perf_simd(t_int * w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"shrl      $4, %3                          \n" /* divide by 16 */
	
	/* loop: *out = *in1 / *in2 */
	"1:                                        \n"
	"movaps    (%0,%4), %%xmm0                 \n"
	"movaps    (%1,%4), %%xmm1                 \n"
	"divps     %%xmm1, %%xmm0                  \n"
	"movaps    %%xmm0, (%2,%4)                 \n"
	
	"movaps    4*T_FLOAT(%0,%4), %%xmm2        \n"
	"movaps    4*T_FLOAT(%1,%4), %%xmm3        \n"
	"divps     %%xmm3, %%xmm2                  \n"
	"movaps    %%xmm2, 4*T_FLOAT(%2,%4)        \n"
	
	"movaps    8*T_FLOAT(%0,%4), %%xmm4        \n"
	"movaps    8*T_FLOAT(%1,%4), %%xmm5        \n"
	"divps     %%xmm5, %%xmm4                  \n"
	"movaps    %%xmm4, 8*T_FLOAT(%2,%4)        \n"
	
	"movaps    12*T_FLOAT(%0,%4), %%xmm6       \n"
	"movaps    12*T_FLOAT(%1,%4), %%xmm7       \n"
	"divps     %%xmm7, %%xmm6                  \n"
	"movaps    %%xmm6, 12*T_FLOAT(%2,%4)       \n"
	
	"addl      $16*T_FLOAT, %4                 \n"
	"loop      1b                              \n"
	:
	/* in1, in2, out, n */
	:"r"(w[1]),"r"(w[2]),"r"(w[3]),"c"(w[4]),"r"(0)
	:"%xmm0","%xmm1","%xmm2","%xmm3","%xmm4","%xmm5","%xmm6","%xmm7"
	);
    return w+5;
}

t_int* scalarover_perf_simd(t_int *w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"movss     (%1), %%xmm0                    \n"
	"shufps    $0, %%xmm0, %%xmm0              \n"
	"shrl      $4, %3                          \n" /* divide by 16 */

	/* loop: *out = *in / value */
	"1:                                        \n"
	"movaps    (%0), %%xmm1                    \n"
	"divps     %%xmm0, %%xmm1                  \n"
	"movaps    %%xmm1, (%2)                    \n"
    
	"movaps    4*T_FLOAT(%0), %%xmm2           \n"
	"divps     %%xmm0, %%xmm2                  \n"
	"movaps    %%xmm2, 4*T_FLOAT(%2)           \n"
	
	"movaps    8*T_FLOAT(%0), %%xmm3           \n"
	"divps     %%xmm0, %%xmm3                  \n"
	"movaps    %%xmm3, 8*T_FLOAT(%2)           \n"
	
	"movaps    12*T_FLOAT(%0), %%xmm4          \n"
	"divps     %%xmm0, %%xmm4                  \n"
	"movaps    %%xmm4, 12*T_FLOAT(%2)          \n"
	
	"addl      $16*T_FLOAT, %0                 \n"
	"addl      $16*T_FLOAT, %2                 \n"
	"loop      1b                              \n"
	:
	/* in, value, out, n */
	:"r"(w[1]),"r"(w[2]),"r"(w[3]),"c"(w[4])
	:"%xmm0","%xmm1","%xmm2","%xmm3","%xmm4"
	);
    return w+5;
}


t_int* min_perf_simd(t_int * w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"shrl      $4, %3                          \n" /* divide by 16 */
	
	/* loop: *out = min (*in1, *in2) */
	"1:                                        \n"
	"movaps    (%0,%4), %%xmm0                 \n"
	"movaps    (%1,%4), %%xmm1                 \n"
	"minps     %%xmm1, %%xmm0                  \n"
	"movaps    %%xmm0, (%2,%4)                 \n"
	
	"movaps    4*T_FLOAT(%0,%4), %%xmm2        \n"
	"movaps    4*T_FLOAT(%1,%4), %%xmm3        \n"
	"minps     %%xmm3, %%xmm2                  \n"
	"movaps    %%xmm2, 4*T_FLOAT(%2,%4)        \n"
	
	"movaps    8*T_FLOAT(%0,%4), %%xmm4        \n"
	"movaps    8*T_FLOAT(%1,%4), %%xmm5        \n"
	"minps     %%xmm5, %%xmm4                  \n"
	"movaps    %%xmm4, 8*T_FLOAT(%2,%4)        \n"
	
	"movaps    12*T_FLOAT(%0,%4), %%xmm6       \n"
	"movaps    12*T_FLOAT(%1,%4), %%xmm7       \n"
	"minps     %%xmm7, %%xmm6                  \n"
	"movaps    %%xmm6, 12*T_FLOAT(%2,%4)       \n"
	
	"addl      $16*T_FLOAT, %4                 \n"
	"loop      1b                              \n"
	:
	/* in1, in2, out, n */
	:"r"(w[1]),"r"(w[2]),"r"(w[3]),"c"(w[4]),"r"(0)
	:"%xmm0","%xmm1","%xmm2","%xmm3","%xmm4","%xmm5","%xmm6","%xmm7"
	);
    return w+5;
}


t_int* scalarmin_perf_simd(t_int *w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"movss     (%1), %%xmm0                    \n"
	"shufps    $0, %%xmm0, %%xmm0              \n"
	"shrl      $4, %3                          \n" /* divide by 16 */

	/* loop: *out = min(*in, value) */
	"1:                                        \n"
	"movaps    (%0), %%xmm1                    \n"
	"minps     %%xmm0, %%xmm1                  \n"
	"movaps    %%xmm1, (%2)                    \n"
    
	"movaps    4*T_FLOAT(%0), %%xmm2           \n"
	"minps     %%xmm0, %%xmm2                  \n"
	"movaps    %%xmm2, 4*T_FLOAT(%2)           \n"
	
	"movaps    8*T_FLOAT(%0), %%xmm3           \n"
	"minps     %%xmm0, %%xmm3                  \n"
	"movaps    %%xmm3, 8*T_FLOAT(%2)           \n"
	
	"movaps    12*T_FLOAT(%0), %%xmm4          \n"
	"minps     %%xmm0, %%xmm4                  \n"
	"movaps    %%xmm4, 12*T_FLOAT(%2)          \n"
	
	"addl      $16*T_FLOAT, %0                 \n"
	"addl      $16*T_FLOAT, %2                 \n"
	"loop      1b                              \n"
	:
	/* in, value, out, n */
	:"r"(w[1]),"r"(w[2]),"r"(w[3]),"c"(w[4])
	:"%xmm0","%xmm1","%xmm2","%xmm3","%xmm4"
	);
    return w+5;
}


t_int* max_perf_simd(t_int * w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"shrl      $4, %3                          \n" /* divide by 16 */
	
	/* loop: *out = max (*in1, *in2) */
	"1:                                        \n"
	"movaps    (%0,%4), %%xmm0                 \n"
	"movaps    (%1,%4), %%xmm1                 \n"
	"maxps     %%xmm1, %%xmm0                  \n"
	"movaps    %%xmm0, (%2,%4)                 \n"
	
	"movaps    4*T_FLOAT(%0,%4), %%xmm2        \n"
	"movaps    4*T_FLOAT(%1,%4), %%xmm3        \n"
	"maxps     %%xmm3, %%xmm2                  \n"
	"movaps    %%xmm2, 4*T_FLOAT(%2,%4)        \n"
	
	"movaps    8*T_FLOAT(%0,%4), %%xmm4        \n"
	"movaps    8*T_FLOAT(%1,%4), %%xmm5        \n"
	"maxps     %%xmm5, %%xmm4                  \n"
	"movaps    %%xmm4, 8*T_FLOAT(%2,%4)        \n"
	
	"movaps    12*T_FLOAT(%0,%4), %%xmm6       \n"
	"movaps    12*T_FLOAT(%1,%4), %%xmm7       \n"
	"maxps     %%xmm7, %%xmm6                  \n"
	"movaps    %%xmm6, 12*T_FLOAT(%2,%4)       \n"
	
	"addl      $16*T_FLOAT, %4                 \n"
	"loop      1b                              \n"
	:
	/* in1, in2, out, n */
	:"r"(w[1]),"r"(w[2]),"r"(w[3]),"c"(w[4]),"r"(0)
	:"%xmm0","%xmm1","%xmm2","%xmm3","%xmm4","%xmm5","%xmm6","%xmm7"
	);
    return w+5;
}


t_int* scalarmax_perf_simd(t_int *w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"movss     (%1), %%xmm0                    \n"
	"shufps    $0, %%xmm0, %%xmm0              \n"
	"shrl      $4, %3                          \n" /* divide by 16 */

	/* loop: *out = max(*in, value) */
	"1:                                        \n"
	"movaps    (%0), %%xmm1                    \n"
	"maxps     %%xmm0, %%xmm1                  \n"
	"movaps    %%xmm1, (%2)                    \n"
    
	"movaps    4*T_FLOAT(%0), %%xmm2           \n"
	"maxps     %%xmm0, %%xmm2                  \n"
	"movaps    %%xmm2, 4*T_FLOAT(%2)           \n"
	
	"movaps    8*T_FLOAT(%0), %%xmm3           \n"
	"maxps     %%xmm0, %%xmm3                  \n"
	"movaps    %%xmm3, 8*T_FLOAT(%2)           \n"
	
	"movaps    12*T_FLOAT(%0), %%xmm4          \n"
	"maxps     %%xmm0, %%xmm4                  \n"
	"movaps    %%xmm4, 12*T_FLOAT(%2)          \n"
	
	"addl      $16*T_FLOAT, %0                 \n"
	"addl      $16*T_FLOAT, %2                 \n"
	"loop      1b                              \n"
	:
	/* in, value, out, n */
	:"r"(w[1]),"r"(w[2]),"r"(w[3]),"c"(w[4])
	:"%xmm0","%xmm1","%xmm2","%xmm3","%xmm4"
	);
    return w+5;
}

t_int* clip_perf_simd(t_int *w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"movss     (%2), %%xmm0                    \n" /* lo */
	"shufps    $0, %%xmm0, %%xmm0              \n"
	"movss     (%3), %%xmm1                    \n" /* hi */
	"shufps    $0, %%xmm1, %%xmm1              \n"

	"shrl      $4, %4                          \n" /* divide by 16 */

	/* loop: *out = min ( max (lo, *in), hi )*/
	"1:                                        \n"
	"movaps    (%0), %%xmm2                    \n"
	"maxps     %%xmm0, %%xmm2                  \n"
	"minps     %%xmm1, %%xmm2                  \n"
	"movaps    %%xmm2, (%1)                    \n"
    
	"movaps    4*T_FLOAT(%0), %%xmm3           \n"
	"maxps     %%xmm0, %%xmm3                  \n"
	"minps     %%xmm1, %%xmm3                  \n"
	"movaps    %%xmm3, 4*T_FLOAT(%1)           \n"
	
	"movaps    8*T_FLOAT(%0), %%xmm4           \n"
	"maxps     %%xmm0, %%xmm4                  \n"
	"minps     %%xmm1, %%xmm4                  \n"
	"movaps    %%xmm4, 8*T_FLOAT(%1)           \n"

	"movaps    12*T_FLOAT(%0), %%xmm5          \n"
	"maxps     %%xmm0, %%xmm5                  \n"
	"minps     %%xmm1, %%xmm5                  \n"
	"movaps    %%xmm5, 12*T_FLOAT(%1)          \n"
	
	"addl      $16*T_FLOAT, %0                 \n"
	"addl      $16*T_FLOAT, %1                 \n"
	"loop      1b                              \n"
	:
	/* in, out, lo, hi, n */
	:"r"(w[1]),"r"(w[2]),"r"(w[3]),"r"(w[4]),"c"(w[5])
	:"%xmm0","%xmm1","%xmm2","%xmm3","%xmm4","%xmm5"
	);
    return w+6;
}


t_int* sigsqrt_perf_simd(t_int *w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"shrl      $4, %2                          \n" /* divide by 16 */

	/* loop: *out = sqrt(*in)  */
	"1:                                        \n"
	"movaps    (%0), %%xmm0                    \n"
	"sqrtps    %%xmm0, %%xmm0                  \n"
	"movaps    %%xmm0, (%1)                    \n"
    
	"movaps    4*T_FLOAT(%0), %%xmm1           \n"
	"sqrtps    %%xmm1, %%xmm1                  \n"
	"movaps    %%xmm1, 4*T_FLOAT(%1)           \n"
	
	"movaps    8*T_FLOAT(%0), %%xmm2           \n"
	"sqrtps    %%xmm2, %%xmm2                  \n"
	"movaps    %%xmm2, 8*T_FLOAT(%1)           \n"
	
	"movaps    12*T_FLOAT(%0), %%xmm3          \n"
	"sqrtps    %%xmm3, %%xmm3                  \n"
	"movaps    %%xmm3, 12*T_FLOAT(%1)          \n"
	
	"addl      $16*T_FLOAT, %0                 \n"
	"addl      $16*T_FLOAT, %1                 \n"
	"loop      1b                              \n"
	:
	/* in, out, n */
	:"r"(w[1]),"r"(w[2]),"c"(w[3])
	:"%xmm0","%xmm1","%xmm2","%xmm3"
	);
    return w+4;
}


t_int* sigrsqrt_perf_simd(t_int *w)
{
    asm(
	".set T_FLOAT,4                            \n"
	
	"shrl      $4, %2                          \n" /* divide by 16 */

	/* loop: *out = sqrt(*in)  */
	"1:                                        \n"
	"movaps    (%0), %%xmm0                    \n"
	"rsqrtps    %%xmm0, %%xmm0                  \n"
	"movaps    %%xmm0, (%1)                    \n"
    
	"movaps    4*T_FLOAT(%0), %%xmm1           \n"
	"rsqrtps    %%xmm1, %%xmm1                  \n"
	"movaps    %%xmm1, 4*T_FLOAT(%1)           \n"
	
	"movaps    8*T_FLOAT(%0), %%xmm2           \n"
	"rsqrtps    %%xmm2, %%xmm2                  \n"
	"movaps    %%xmm2, 8*T_FLOAT(%1)           \n"
	
	"movaps    12*T_FLOAT(%0), %%xmm3          \n"
	"rsqrtps    %%xmm3, %%xmm3                  \n"
	"movaps    %%xmm3, 12*T_FLOAT(%1)          \n"
	
	"addl      $16*T_FLOAT, %0                 \n"
	"addl      $16*T_FLOAT, %1                 \n"
	"loop      1b                              \n"
	:
	/* in, out, n */
	:"r"(w[1]),"r"(w[2]),"c"(w[3])
	:"%xmm0","%xmm1","%xmm2","%xmm3"
	);
    return w+4;
}


/* TB: runtime check */
int simd_runtime_check()
{
    unsigned int eax, edx, ret;
    __asm__("push %%ebx \n" /* ebx might be used as PIC register :-( */
        "cpuid      \n"
        "pop  %%ebx \n"
 		: "=a"(eax),"=d"(edx) : "a" (1): "cx");
    ret = 0x2000000 & edx;
    return (ret);
}

float env_tilde_accum_simd(t_float* in, t_float* hp, t_int n)
{
	float ret;
    asm(
		".set T_FLOAT,4                            \n"
		
		"shrl      $4, %2                          \n" /* divide by 16 */
		"xorps     %%xmm2, %%xmm2                  \n" /* zero values */
		"xorps     %%xmm3, %%xmm3                  \n"
		"xorps     %%xmm4, %%xmm4                  \n"
		"xorps     %%xmm5, %%xmm5                  \n"

		
		"1:                                        \n"
		"movaps    -4*T_FLOAT(%1), %%xmm0          \n"
		"movhlps   %%xmm0, %%xmm1                  \n" /* reversing xmm0 CHECK!!!*/
		"shufps    $68, %%xmm1, %%xmm0             \n"
		"movaps    (%3), %%xmm1                    \n"
		"mulps     %%xmm0, %%xmm0                  \n"
		"mulps     %%xmm1, %%xmm0                  \n"
		"addps     %%xmm0, %%xmm2                  \n"

		"movaps    -8*T_FLOAT(%1), %%xmm0          \n"
		"movhlps   %%xmm0, %%xmm1                  \n" /* reversing xmm0 */
		"shufps    $68, %%xmm1, %%xmm0             \n"
		"movaps    4*T_FLOAT(%3), %%xmm1           \n"
		"mulps     %%xmm0, %%xmm0                  \n"
		"mulps     %%xmm1, %%xmm0                  \n"
		"addps     %%xmm0, %%xmm2                  \n"

		"movaps    -12*T_FLOAT(%1), %%xmm0         \n"
		"movhlps   %%xmm0, %%xmm1                  \n" /* reversing xmm0 */
		"shufps    $68, %%xmm1, %%xmm0             \n"
		"movaps    8*T_FLOAT(%3), %%xmm1           \n"
		"mulps     %%xmm0, %%xmm0                  \n"
		"mulps     %%xmm1, %%xmm0                  \n"
		"addps     %%xmm0, %%xmm2                  \n"

		"movaps    -16*T_FLOAT(%1), %%xmm0         \n"
		"movhlps   %%xmm0, %%xmm1                  \n" /* reversing xmm0 */
		"shufps    $68, %%xmm1, %%xmm0             \n"
		"movaps    12*T_FLOAT(%3), %%xmm1          \n"
		"mulps     %%xmm0, %%xmm0                  \n"
		"mulps     %%xmm1, %%xmm0                  \n"
		"addps     %%xmm0, %%xmm2                  \n"

		"addl      $-16*T_FLOAT,%1                 \n"
		"addl      $16*T_FLOAT,%3                  \n"
		"loop      1b                              \n"

		"movhlps   %%xmm2, %%xmm3                  \n" /* unpack xmm0 */
		"movups    %%xmm2, %%xmm4                  \n"
		"movups    %%xmm3, %%xmm5                  \n"
		"shufps    $81, %%xmm4, %%xmm4             \n"
		"shufps    $81, %%xmm5, %%xmm5             \n"

		"addss     %%xmm2, %%xmm3                  \n"
		"addss     %%xmm3, %%xmm4                  \n"
		"addss     %%xmm4, %%xmm5                  \n"
		
		"movss     %%xmm5, (%0)                    \n"

		:
		:"r"(&ret),"r"(in),"c"(n), "r"(hp)
		:"%xmm0","%xmm1","%xmm2","%xmm3", "%xmm4", "%xmm5");
	return ret;
}


float peakvec_simd(t_float* vec, t_int n, t_float cur_max)
{
    asm(
		".section	.rodata                        \n"
		".align 16                                 \n"
		"2:                                        \n"
		".long	2147483647                         \n" /* bitmask for abs */
		".long	2147483647                         \n" /* 0x7fffffff */
		".long	2147483647                         \n"
		".long	2147483647                         \n"

		".set T_FLOAT,4                            \n"
		".text                                     \n"

		"shrl      $4, %2                          \n" /* divide by 16 */
		"movaps    (2b), %%xmm0                    \n"

		"movss     (%0), %%xmm5                    \n" /* cur_max */
		"shufps    $0, %%xmm5, %%xmm5              \n"

		"1:                                        \n"
		"movaps    (%1), %%xmm1                    \n"
		"andps     %%xmm0, %%xmm1                  \n"
		"maxps     %%xmm1, %%xmm5                  \n"

		"movaps    4*T_FLOAT(%1), %%xmm1           \n"
		"andps     %%xmm0, %%xmm1                  \n"
		"maxps     %%xmm1, %%xmm5                  \n"

		"movaps    8*T_FLOAT(%1), %%xmm1           \n"
		"andps     %%xmm0, %%xmm1                  \n"
		"maxps     %%xmm1, %%xmm5                  \n"

		"movaps    12*T_FLOAT(%1), %%xmm1          \n"
		"andps     %%xmm0, %%xmm1                  \n"
		"maxps     %%xmm1, %%xmm5                  \n"

		"addl      $16*T_FLOAT, %1                 \n"
		"loop      1b                              \n"
		
		"movhlps   %%xmm5, %%xmm2                  \n"
		"movaps    %%xmm5, %%xmm3                  \n"
		"movaps    %%xmm2, %%xmm4                  \n"
		"shufps    $81, %%xmm3, %%xmm3             \n"
		"shufps    $81, %%xmm4,  %%xmm4            \n"

		"maxss     %%xmm2, %%xmm3                  \n"
		"maxss     %%xmm3, %%xmm4                  \n"
		"maxss     %%xmm4, %%xmm5                  \n"

		"movss     %%xmm5, (%0)                    \n"

		:
		:"r"(&cur_max), "r"(vec),"c"(n)
		:"%xmm0","%xmm1","%xmm2","%xmm3", "%xmm4", "%xmm5");

	return cur_max;
}

void line_tilde_slope_simd(t_float* out, t_int n, t_float* value,
						   t_float* slopes, t_float* slopestep)
{
	asm(
		".set T_FLOAT,4                          \n"
		"movss     (%2),%%xmm0                   \n" /* value */
		"shufps    $0, %%xmm0, %%xmm0            \n"
		"movaps    (%3), %%xmm1                  \n" /* slopes */

		"addps     %%xmm1, %%xmm0                \n" /* compute first output */
		
		"movss     (%4), %%xmm2                  \n" /* slopestep */
		"shufps    $0, %%xmm2, %%xmm2            \n" 

		"shrl      $4, %1                        \n" /* n>>4 */

		"1:                                      \n"
		"movaps    %%xmm0, (%0)                  \n"
		"addps     %%xmm2, %%xmm0                \n"

		"movaps    %%xmm0, 4*T_FLOAT(%0)         \n"
		"addps     %%xmm2, %%xmm0                \n"

		"movaps    %%xmm0, 8*T_FLOAT(%0)         \n"
		"addps     %%xmm2, %%xmm0                \n"

		"movaps    %%xmm0, 12*T_FLOAT(%0)        \n"
		"addps     %%xmm2, %%xmm0                \n"


		"addl      $16*T_FLOAT, %0               \n"
		"loop      1b                            \n"


		:
		:"r"(out),"c"(n), "r"(value), "r"(slopes),
		"r"(slopestep)
		:"%xmm0", "%xmm1", "%xmm2");

}

#endif

