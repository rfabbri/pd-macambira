/* 
    Implementation of general vectorized functions
    added by T.Grill
*/

#include "m_pd.h"
#include "m_simd.h"

void zerovec_8(t_float *dst,int n)
{
    for(n >>= 3; n--; dst += 8) {
        dst[0] = dst[1] = dst[2] = dst[3] = dst[4] = dst[5] = dst[6] = dst[7] = 0;
    }
}

void setvec_8(t_float *dst,t_float v,int n)
{
    for(n >>= 3; n--; dst += 8) {
        dst[0] = dst[1] = dst[2] = dst[3] = dst[4] = dst[5] = dst[6] = dst[7] = v;
    }
}

void copyvec_8(t_float *dst,const t_float *src,int n)
{
    for(n >>= 3; n--; src += 8,dst += 8) {
        dst[0] = src[0],dst[1] = src[1],dst[2] = src[2],dst[3] = src[3];
        dst[4] = src[4],dst[5] = src[5],dst[6] = src[6],dst[7] = src[7];
    }
}

void addvec_8(t_float *dst,const t_float *src,int n)
{
    for(n >>= 3; n--; src += 8,dst += 8) {
        dst[0] += src[0],dst[1] += src[1],dst[2] += src[2],dst[3] += src[3];
        dst[4] += src[4],dst[5] += src[5],dst[6] += src[6],dst[7] += src[7];
    }
}

void copyvec(t_float *dst,const t_float *src,int n)
{
	while(n--)
		*dst++ = *src++;
}

void zerovec(t_float *dst, int n)
{
	while(n--)
		*dst++ = 0;
}


void addvec(t_float *dst,const t_float *src,int n)
{
	while(n--)
		*dst++ += *src++;
}


void testcopyvec_8(t_float *dst,const t_float *src,int n)
{
    while(n--) {
        *(dst++) = (PD_BIGORSMALL(*src) ? 0 : *src); src++;
	}
}

void testcopyvec(t_float *dst,const t_float *src,int n)
{
	testcopyvec_8(dst, src, n);
}

void testaddvec_8(t_float *dst,const t_float *src,int n)
{
    while(n--) {
        *(dst++) += (PD_BIGORSMALL(*src) ? 0 : *src); src++;
	}
}

void testaddvec(t_float *dst,const t_float *src,int n)
{
	testaddvec_8(dst, src, n);
}


int simd_check1(t_int n, t_float* ptr1)
{
	return SIMD_CHECK1(n,ptr1);
}

int simd_check2(t_int n, t_float* ptr1, t_float* ptr2)
{
	return SIMD_CHECK2(n,ptr1,ptr2);
}

int simd_check3(t_int n, t_float* ptr1, t_float* ptr2, t_float* ptr3)
{
	return SIMD_CHECK3(n,ptr1,ptr2,ptr3);
}



#ifdef DONTUSESIMD
int simd_runtime_check()
{
	return 0;
}

/* tb: wrapper for simd functions */
void zerovec_simd(t_float *dst,int n)
{
	zerovec_8(dst,n);
}

void setvec_simd(t_float *dst,t_float v,int n)
{
	setvec_8(dst,v,n);
}

void copyvec_simd(t_float *dst,const t_float *src,int n)
{
	copyvec_8(dst,src,n);
}

void copyvec_simd_unalignedsrc(t_float *dst,const t_float *src,int n)
{
	copyvec_8(dst,src,n);
}

void addvec_simd(t_float *dst,const t_float *src,int n)
{
	addvec_8(dst,src,n);
}

void testcopyvec_simd(t_float *dst,const t_float *src,int n)
{
	testcopyvec_8(dst,src,n);
}

void testaddvec_simd(t_float *dst,const t_float *src,int n)
{
	testaddvec_8(dst,src,n);
}


#endif /* DONTUSESIMD */

