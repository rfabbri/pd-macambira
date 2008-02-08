/* 
    Implementation of SIMD functionality for Apple Velocity Engine (AltiVec) with GCC compiler
    added by T.Grill
*/

#include "m_pd.h"
#include "m_simd.h"

#if defined(__GNUC__) && defined(__POWERPC__) && defined(__ALTIVEC__)

//#define USEVECLIB

#ifdef USEVECLIB
#include <vecLib/vDSP.h>
#include <vecLib/vfp.h>
#endif

/* functions for unaligned vector data - taken from http://developer.apple.com/hardware/ve/alignment.html */

/* T.Grill - this first version _should_ work! but it doesn't... */
#if 0
#define LoadUnaligned(v) (vec_perm( vec_ld( 0, (const vector float *)(v) ), vec_ld( 16, (const vector float *)(v) ), vec_lvsl( 0, (float *) (v) ) ))
#else
/* instead take the slower second one */
static vector float LoadUnaligned(const float *v)
{
	union tmpstruct { float f[4]; vector float vec; } tmp;
	tmp.f[0] = *(float *)v;
	return vec_splat(vec_ld(0,&tmp.vec),0);
}
#endif


#define IsVectorAligned(where) ((unsigned long)(where)&(sizeof(vector float)-1) == 0)
/*
#define LoadValue(where) (IsVectorAligned((void *)(where))?vec_splat(vec_ld(0,(vector float *)(where)),0):LoadUnaligned((vector float *)(where))) 
*/
/* always assume unaligned */
#define LoadValue(where) LoadUnaligned((const float *)(where))

void zerovec_simd(t_float *dst,int n)
{
	const vector float zero = (vector float)(0);
	for(n >>= 4; n--; dst += 16) {
		vec_st(zero, 0,dst);
		vec_st(zero,16,dst);
		vec_st(zero,32,dst);
		vec_st(zero,48,dst);
	}
}

void setvec_simd(t_float *dst,t_float v,int n)
{
	const vector float arg = LoadValue(&v);
	for(n >>= 4; n--; dst += 16) {
		vec_st(arg, 0,dst);
		vec_st(arg,16,dst);
		vec_st(arg,32,dst);
		vec_st(arg,48,dst);
	}
}

void copyvec_simd(t_float *dst,const t_float *src,int n)
{
	for(n >>= 4; n--; src += 16,dst += 16) {
		vector float a1 = vec_ld( 0,src);
		vector float a2 = vec_ld(16,src);
		vector float a3 = vec_ld(32,src);
		vector float a4 = vec_ld(48,src);
		vec_st(a1, 0,dst);
		vec_st(a2,16,dst);
		vec_st(a3,32,dst);
		vec_st(a4,48,dst);
	}
}

void addvec_simd(t_float *dst,const t_float *src,int n)
{
#ifdef USEVECLIB
	vadd(dst,1,src,1,dst,1,n);
#else
	for(n >>= 4; n--; src += 16,dst += 16) {
		vector float a1 = vec_ld( 0,dst),b1 = vec_ld( 0,src);
		vector float a2 = vec_ld(16,dst),b2 = vec_ld(16,src);
		vector float a3 = vec_ld(32,dst),b3 = vec_ld(32,src);
		vector float a4 = vec_ld(48,dst),b4 = vec_ld(48,src);
		
		a1 = vec_add(a1,b1);
		a2 = vec_add(a2,b2);
		a3 = vec_add(a3,b3);
		a4 = vec_add(a4,b4);

		vec_st(a1, 0,dst);
		vec_st(a2,16,dst);
		vec_st(a3,32,dst);
		vec_st(a4,48,dst);
	}
#endif
}

/* no bad float testing for PPC! */
void testcopyvec_simd(t_float *dst,const t_float *src,int n)
{
	copyvec_simd(dst,src,n);
}

void testaddvec_simd(t_float *dst,const t_float *src,int n)
{
	addvec_simd(dst,src,n);
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

t_int *plus_perf_simd(t_int *w)
{
#ifdef USEVECLIB
	vadd((const t_float *)w[1],1,(const t_float *)w[2],1,(t_float *)w[3],1,w[4]);
#else
    const t_float *src1 = (const t_float *)w[1];
    const t_float *src2 = (const t_float *)w[2];
    t_float *dst = (t_float *)w[3];
    int n = w[4]>>4;
   
	for(; n--; src1 += 16,src2 += 16,dst += 16) {
		vector float a1 = vec_ld( 0,src1),b1 = vec_ld( 0,src2);
		vector float a2 = vec_ld(16,src1),b2 = vec_ld(16,src2);
		vector float a3 = vec_ld(32,src1),b3 = vec_ld(32,src2);
		vector float a4 = vec_ld(48,src1),b4 = vec_ld(48,src2);
		
		a1 = vec_add(a1,b1);
		a2 = vec_add(a2,b2);
		a3 = vec_add(a3,b3);
		a4 = vec_add(a4,b4);

		vec_st(a1, 0,dst);
		vec_st(a2,16,dst);
		vec_st(a3,32,dst);
		vec_st(a4,48,dst);
	}
#endif
	return w+5;
}

t_int *scalarplus_perf_simd(t_int *w)
{
    const t_float *src = (const t_float *)w[1];
	const vector float arg = LoadValue(w[2]);
    t_float *dst = (t_float *)w[3];
    int n = w[4]>>4;
   
	for(; n--; src += 16,dst += 16) {
		vector float a1 = vec_ld( 0,src);
		vector float a2 = vec_ld(16,src);
		vector float a3 = vec_ld(32,src);
		vector float a4 = vec_ld(48,src);
		
		a1 = vec_add(a1,arg);
		a2 = vec_add(a2,arg);
		a3 = vec_add(a3,arg);
		a4 = vec_add(a4,arg);

		vec_st(a1, 0,dst);
		vec_st(a2,16,dst);
		vec_st(a3,32,dst);
		vec_st(a4,48,dst);
	}
	return w+5;
}

t_int *minus_perf_simd(t_int *w)
{
#if 0 //def USEVECLIB
    /* vsub is buggy for some OSX versions! */
	vsub((const t_float *)w[1],1,(const t_float *)w[2],1,(t_float *)w[3],1,w[4]);
#else
    const t_float *src1 = (const t_float *)w[1];
    const t_float *src2 = (const t_float *)w[2];
    t_float *dst = (t_float *)w[3];
    int n = w[4]>>4;
   
	for(; n--; src1 += 16,src2 += 16,dst += 16) {
		vector float a1 = vec_ld( 0,src1),b1 = vec_ld( 0,src2);
		vector float a2 = vec_ld(16,src1),b2 = vec_ld(16,src2);
		vector float a3 = vec_ld(32,src1),b3 = vec_ld(32,src2);
		vector float a4 = vec_ld(48,src1),b4 = vec_ld(48,src2);
		
		a1 = vec_sub(a1,b1);
		a2 = vec_sub(a2,b2);
		a3 = vec_sub(a3,b3);
		a4 = vec_sub(a4,b4);

		vec_st(a1, 0,dst);
		vec_st(a2,16,dst);
		vec_st(a3,32,dst);
		vec_st(a4,48,dst);
	}
#endif
	return w+5;
}

t_int *scalarminus_perf_simd(t_int *w)
{
    const t_float *src = (const t_float *)w[1];
	const vector float arg = LoadValue(w[2]);
    t_float *dst = (t_float *)w[3];
    int n = w[4]>>4;
   
	for(; n--; src += 16,dst += 16) {
		vector float a1 = vec_ld( 0,src);
		vector float a2 = vec_ld(16,src);
		vector float a3 = vec_ld(32,src);
		vector float a4 = vec_ld(48,src);
		
		a1 = vec_sub(a1,arg);
		a2 = vec_sub(a2,arg);
		a3 = vec_sub(a3,arg);
		a4 = vec_sub(a4,arg);

		vec_st(a1, 0,dst);
		vec_st(a2,16,dst);
		vec_st(a3,32,dst);
		vec_st(a4,48,dst);
	}
	return w+5;
}

t_int *times_perf_simd(t_int *w)
{
#ifdef USEVECLIB
	vmul((const t_float *)w[1],1,(const t_float *)w[2],1,(t_float *)w[3],1,w[4]);
#else
    const t_float *src1 = (const t_float *)w[1];
    const t_float *src2 = (const t_float *)w[2];
    t_float *dst = (t_float *)w[3];
    const vector float zero = (vector float)(0);
    int n = w[4]>>4;
   
	for(; n--; src1 += 16,src2 += 16,dst += 16) {
		vector float a1 = vec_ld( 0,src1),b1 = vec_ld( 0,src2);
		vector float a2 = vec_ld(16,src1),b2 = vec_ld(16,src2);
		vector float a3 = vec_ld(32,src1),b3 = vec_ld(32,src2);
		vector float a4 = vec_ld(48,src1),b4 = vec_ld(48,src2);
		
		a1 = vec_madd(a1,b1,zero);
		a2 = vec_madd(a2,b2,zero);
		a3 = vec_madd(a3,b3,zero);
		a4 = vec_madd(a4,b4,zero);

		vec_st(a1, 0,dst);
		vec_st(a2,16,dst);
		vec_st(a3,32,dst);
		vec_st(a4,48,dst);
	}
#endif
	return w+5;
}

t_int *scalartimes_perf_simd(t_int *w)
{
#ifdef USEVECLIB
	vsmul((const t_float *)w[1],1,(t_float *)w[2],(t_float *)w[3],1,w[4]);
#else
    const t_float *src = (const t_float *)w[1];
	const vector float arg = LoadValue(w[2]);
    t_float *dst = (t_float *)w[3];
    const vector float zero = (vector float)(0);
    int n = w[4]>>4;
   
	for(; n--; src += 16,dst += 16) {
		vector float a1 = vec_ld( 0,src);
		vector float a2 = vec_ld(16,src);
		vector float a3 = vec_ld(32,src);
		vector float a4 = vec_ld(48,src);
		
		a1 = vec_madd(a1,arg,zero);
		a2 = vec_madd(a2,arg,zero);
		a3 = vec_madd(a3,arg,zero);
		a4 = vec_madd(a4,arg,zero);

		vec_st(a1, 0,dst);
		vec_st(a2,16,dst);
		vec_st(a3,32,dst);
		vec_st(a4,48,dst);
	}
#endif
	return w+5;
}

t_int *sqr_perf_simd(t_int *w)
{
#ifdef USEVECLIB
	vsq((const t_float *)w[1],1,(t_float *)w[2],1,w[3]);
#else
    const t_float *src = (const t_float *)w[1];
    t_float *dst = (t_float *)w[2];
    const vector float zero = (vector float)(0);
    int n = w[3]>>4;
   
	for(; n--; src += 16,dst += 16) {
		vector float a1 = vec_ld( 0,src);
		vector float a2 = vec_ld(16,src);
		vector float a3 = vec_ld(32,src);
		vector float a4 = vec_ld(48,src);
		
		a1 = vec_madd(a1,a1,zero);
		a2 = vec_madd(a2,a2,zero);
		a3 = vec_madd(a3,a3,zero);
		a4 = vec_madd(a4,a4,zero);

		vec_st(a1, 0,dst);
		vec_st(a2,16,dst);
		vec_st(a3,32,dst);
		vec_st(a4,48,dst);
	}
#endif
	return w+4;
}

t_int *over_perf_simd(t_int *w)
{
    const t_float *src1 = (const t_float *)w[1];
    const t_float *src2 = (const t_float *)w[2];
    t_float *dst = (t_float *)w[3];
    const vector float zero = (vector float)(0);
    const vector float one = (vector float)(1);
    int n = w[4]>>4;
   
	for(; n--; src1 += 16,src2 += 16,dst += 16) {
#ifdef USEVECLIB
		/* no zero checking here */
		vec_st(vdivf(vec_ld( 0,src1),vec_ld( 0,src2)), 0,dst);
		vec_st(vdivf(vec_ld(16,src1),vec_ld(16,src2)),16,dst);
		vec_st(vdivf(vec_ld(32,src1),vec_ld(32,src2)),32,dst);
		vec_st(vdivf(vec_ld(48,src1),vec_ld(48,src2)),48,dst);
#else
	    vector float data1 = vec_ld( 0,src2);
	    vector float data2 = vec_ld(16,src2); 
	    vector float data3 = vec_ld(32,src2); 
	    vector float data4 = vec_ld(48,src2); 

		vector unsigned char mask1 = vec_nor((vector unsigned char)vec_cmpeq(data1,zero),(vector unsigned char)zero); /* bit mask... all 0 for data = 0., all 1 else */
		vector unsigned char mask2 = vec_nor((vector unsigned char)vec_cmpeq(data2,zero),(vector unsigned char)zero); /* bit mask... all 0 for data = 0., all 1 else */
		vector unsigned char mask3 = vec_nor((vector unsigned char)vec_cmpeq(data3,zero),(vector unsigned char)zero); /* bit mask... all 0 for data = 0., all 1 else */
		vector unsigned char mask4 = vec_nor((vector unsigned char)vec_cmpeq(data4,zero),(vector unsigned char)zero); /* bit mask... all 0 for data = 0., all 1 else */

		/* make estimated reciprocal and zero out NANs */
		vector float tmp1 = vec_re(data1);
		vector float tmp2 = vec_re(data2);
		vector float tmp3 = vec_re(data3);
		vector float tmp4 = vec_re(data4);
		
		tmp1 = (vector float)vec_and((vector unsigned char)tmp1,mask1); 
		tmp2 = (vector float)vec_and((vector unsigned char)tmp2,mask2); 
		tmp3 = (vector float)vec_and((vector unsigned char)tmp3,mask3); 
		tmp4 = (vector float)vec_and((vector unsigned char)tmp4,mask4); 

		data1 = vec_madd( vec_nmsub( tmp1, data1, one ), tmp1, tmp1 );
		data2 = vec_madd( vec_nmsub( tmp2, data2, one ), tmp2, tmp2 );
		data3 = vec_madd( vec_nmsub( tmp3, data3, one ), tmp3, tmp3 );
		data4 = vec_madd( vec_nmsub( tmp4, data4, one ), tmp4, tmp4 );

		tmp1 = vec_ld( 0,src1);
		tmp2 = vec_ld(16,src1);
		tmp3 = vec_ld(32,src1);
		tmp4 = vec_ld(48,src1);

		data1 = vec_madd(tmp1,data1,zero);
		data2 = vec_madd(tmp2,data2,zero);
		data3 = vec_madd(tmp3,data3,zero);
		data4 = vec_madd(tmp4,data4,zero);

		vec_st(data1, 0,dst);
		vec_st(data2,16,dst);
		vec_st(data3,32,dst);
		vec_st(data4,48,dst);
#endif
	}
	return w+5;
}

t_int *scalarover_perf_simd(t_int *w)
{
    t_float *dst = (t_float *)w[3];
    const vector float zero = (vector float)(0);
    int n = w[4]>>4;

	if(*(t_float *)w[2]) {
	    const t_float *src = (const t_float *)w[1];
#ifdef USEVECLIB
		float arg = *(t_float *)w[2]?1./ *(t_float *)w[2]: 0;
		vsmul(src,1,&arg,dst,1,w[4]);
#else
		const vector float v = LoadValue(w[2]);
	    const vector float one = (vector float)(1);

	    vector float estimate = vec_re(v); 
		vector float arg = vec_madd( vec_nmsub( estimate, v, one ), estimate, estimate );

		for(; n--; src += 16,dst += 16) {
			vector float a1 = vec_ld( 0,src);
			vector float a2 = vec_ld(16,src);
			vector float a3 = vec_ld(32,src);
			vector float a4 = vec_ld(48,src);
			
			a1 = vec_madd(a1,arg,zero);
			a2 = vec_madd(a2,arg,zero);
			a3 = vec_madd(a3,arg,zero);
			a4 = vec_madd(a4,arg,zero);

			vec_st(a1, 0,dst);
			vec_st(a2,16,dst);
			vec_st(a3,32,dst);
			vec_st(a4,48,dst);
		}
#endif
	}
	else {
		/* zero all output */
		for(; n--; dst += 16) {
			vec_st(zero, 0,dst);
			vec_st(zero,16,dst);
			vec_st(zero,32,dst);
			vec_st(zero,48,dst);
		}
	}
	return w+5;
}

t_int *min_perf_simd(t_int *w)
{
    const t_float *src1 = (const t_float *)w[1];
    const t_float *src2 = (const t_float *)w[2];
    t_float *dst = (t_float *)w[3];
    int n = w[4]>>4;
   
	for(; n--; src1 += 16,src2 += 16,dst += 16) {
		vector float a1 = vec_ld( 0,src1),b1 = vec_ld( 0,src2);
		vector float a2 = vec_ld(16,src1),b2 = vec_ld(16,src2);
		vector float a3 = vec_ld(32,src1),b3 = vec_ld(32,src2);
		vector float a4 = vec_ld(48,src1),b4 = vec_ld(48,src2);
		
		a1 = vec_min(a1,b1);
		a2 = vec_min(a2,b2);
		a3 = vec_min(a3,b3);
		a4 = vec_min(a4,b4);

		vec_st(a1, 0,dst);
		vec_st(a2,16,dst);
		vec_st(a3,32,dst);
		vec_st(a4,48,dst);
	}
	return w+5;
}

t_int *scalarmin_perf_simd(t_int *w)
{
    const t_float *src = (const t_float *)w[1];
	const vector float arg = LoadValue(w[2]);
    t_float *dst = (t_float *)w[3];
    int n = w[4]>>4;
   
	for(; n--; src += 16,dst += 16) {
		vector float a1 = vec_ld( 0,src);
		vector float a2 = vec_ld(16,src);
		vector float a3 = vec_ld(32,src);
		vector float a4 = vec_ld(48,src);
		
		a1 = vec_min(a1,arg);
		a2 = vec_min(a2,arg);
		a3 = vec_min(a3,arg);
		a4 = vec_min(a4,arg);

		vec_st(a1, 0,dst);
		vec_st(a2,16,dst);
		vec_st(a3,32,dst);
		vec_st(a4,48,dst);
	}
	return w+5;
}

t_int *max_perf_simd(t_int *w)
{
    const t_float *src1 = (const t_float *)w[1];
    const t_float *src2 = (const t_float *)w[2];
    t_float *dst = (t_float *)w[3];
    int n = w[4]>>4;
   
	for(; n--; src1 += 16,src2 += 16,dst += 16) {
		vector float a1 = vec_ld( 0,src1),b1 = vec_ld( 0,src2);
		vector float a2 = vec_ld(16,src1),b2 = vec_ld(16,src2);
		vector float a3 = vec_ld(32,src1),b3 = vec_ld(32,src2);
		vector float a4 = vec_ld(48,src1),b4 = vec_ld(48,src2);
		
		a1 = vec_max(a1,b1);
		a2 = vec_max(a2,b2);
		a3 = vec_max(a3,b3);
		a4 = vec_max(a4,b4);

		vec_st(a1, 0,dst);
		vec_st(a2,16,dst);
		vec_st(a3,32,dst);
		vec_st(a4,48,dst);
	}
	return w+5;
}

t_int *scalarmax_perf_simd(t_int *w)
{
    const t_float *src = (const t_float *)w[1];
	const vector float arg = LoadValue(w[2]);
    t_float *dst = (t_float *)w[3];
    int n = w[4]>>4;
   
	for(; n--; src += 16,dst += 16) {
		vector float a1 = vec_ld( 0,src);
		vector float a2 = vec_ld(16,src);
		vector float a3 = vec_ld(32,src);
		vector float a4 = vec_ld(48,src);
		
		a1 = vec_max(a1,arg);
		a2 = vec_max(a2,arg);
		a3 = vec_max(a3,arg);
		a4 = vec_max(a4,arg);

		vec_st(a1, 0,dst);
		vec_st(a2,16,dst);
		vec_st(a3,32,dst);
		vec_st(a4,48,dst);
	}
	return w+5;
}

t_int *clip_perf_simd(t_int *w)
{
    const t_float *src = (const t_float *)w[1];
    t_float *dst = (t_float *)w[2];
	const vector float lo = LoadValue(w[3]);
	const vector float hi = LoadValue(w[4]);
    int n = w[5]>>4;
   
	for(; n--; src += 16,dst += 16) {
		vector float data1 = vec_ld( 0,src);
		vector float data2 = vec_ld(16,src);
		vector float data3 = vec_ld(32,src);
		vector float data4 = vec_ld(48,src);
		
		vector unsigned char mlo1 = (vector unsigned char)vec_cmple(data1,lo); /* bit mask data <= lo */
		vector unsigned char mlo2 = (vector unsigned char)vec_cmple(data2,lo); /* bit mask data <= lo */
		vector unsigned char mlo3 = (vector unsigned char)vec_cmple(data3,lo); /* bit mask data <= lo */
		vector unsigned char mlo4 = (vector unsigned char)vec_cmple(data4,lo); /* bit mask data <= lo */

		vector unsigned char mhi1 = (vector unsigned char)vec_cmpge(data1,hi); /* bit mask data >= hi */
		vector unsigned char mhi2 = (vector unsigned char)vec_cmpge(data2,hi); /* bit mask data >= hi */
		vector unsigned char mhi3 = (vector unsigned char)vec_cmpge(data3,hi); /* bit mask data >= hi */
		vector unsigned char mhi4 = (vector unsigned char)vec_cmpge(data4,hi); /* bit mask data >= hi */

		data1 = (vector float)vec_and((vector unsigned char)data1,vec_nor(mlo1,mhi1));
		data2 = (vector float)vec_and((vector unsigned char)data2,vec_nor(mlo2,mhi2));
		data3 = (vector float)vec_and((vector unsigned char)data3,vec_nor(mlo3,mhi3));
		data4 = (vector float)vec_and((vector unsigned char)data4,vec_nor(mlo4,mhi4));
		
		mlo1 = vec_and((vector unsigned char)lo,mlo1);
		mlo2 = vec_and((vector unsigned char)lo,mlo2);
		mlo3 = vec_and((vector unsigned char)lo,mlo3);
		mlo4 = vec_and((vector unsigned char)lo,mlo4);
		
		mhi1 = vec_and((vector unsigned char)hi,mhi1);
		mhi2 = vec_and((vector unsigned char)hi,mhi2);
		mhi3 = vec_and((vector unsigned char)hi,mhi3);
		mhi4 = vec_and((vector unsigned char)hi,mhi4);

		data1 = (vector float)vec_or(vec_or(mlo1,mhi1),(vector unsigned char)data1);
		data2 = (vector float)vec_or(vec_or(mlo2,mhi2),(vector unsigned char)data2);
		data3 = (vector float)vec_or(vec_or(mlo3,mhi3),(vector unsigned char)data3);
		data4 = (vector float)vec_or(vec_or(mlo4,mhi4),(vector unsigned char)data4);

		vec_st(data1, 0,dst);
		vec_st(data2,16,dst);
		vec_st(data3,32,dst);
		vec_st(data4,48,dst);
	}
	return w+6;
}

t_int *sigwrap_perf_simd(t_int *w)
{
    const t_float *src = (const t_float *)w[1];
    t_float *dst = (t_float *)w[2];
    int n = w[3]>>4;

	for(; n--; src += 16,dst += 16) {
		vector float data1 = vec_ld( 0,src);
		vector float data2 = vec_ld(16,src);
		vector float data3 = vec_ld(32,src);
		vector float data4 = vec_ld(48,src);
		
		data1 = vec_sub(data1,vec_floor(data1));
		data2 = vec_sub(data2,vec_floor(data2));
		data3 = vec_sub(data3,vec_floor(data3));
		data4 = vec_sub(data4,vec_floor(data4));
		
		vec_st(data1, 0,dst);
		vec_st(data2,16,dst);
		vec_st(data3,32,dst);
		vec_st(data4,48,dst);
	}
	return w+4;
}

t_int *sigsqrt_perf_simd(t_int *w)
{
    const t_float *src = (const t_float *)w[1];
    t_float *dst = (t_float *)w[2];
    int n = w[3]>>4;
	
	const vector float zero = (vector float)(0);
	const vector float oneHalf = (vector float)(0.5);
	const vector float one = (vector float)(1.0);

	for(; n--; src += 16,dst += 16) {
		/* http://developer.apple.com/hardware/ve/algorithms.html

			Just as in Miller's scalar sigsqrt_perform, 
			first a rsqrt estimate is calculated which is then refined by one round of Newton-Raphson.
			Here, to avoid branching a mask is generated which zeroes out eventual resulting NANs.
		*/
		
#ifdef USEVECLIB
		/* no zero checking here */
		vec_st(vsqrtf(vec_ld( 0,src)), 0,dst); 
		vec_st(vsqrtf(vec_ld(16,src)),16,dst); 
		vec_st(vsqrtf(vec_ld(32,src)),32,dst); 
		vec_st(vsqrtf(vec_ld(48,src)),48,dst); 
#else
		vector float data1 = vec_ld( 0,src);
		vector float data2 = vec_ld(16,src);
		vector float data3 = vec_ld(32,src);
		vector float data4 = vec_ld(48,src);

		const vector unsigned char mask1 = vec_nor((vector unsigned char)vec_cmple(data1,zero),(vector unsigned char)zero); /* bit mask... all 0 for data <= 0., all 1 else */
		const vector unsigned char mask2 = vec_nor((vector unsigned char)vec_cmple(data2,zero),(vector unsigned char)zero); /* bit mask... all 0 for data <= 0., all 1 else */
		const vector unsigned char mask3 = vec_nor((vector unsigned char)vec_cmple(data3,zero),(vector unsigned char)zero); /* bit mask... all 0 for data <= 0., all 1 else */
		const vector unsigned char mask4 = vec_nor((vector unsigned char)vec_cmple(data4,zero),(vector unsigned char)zero); /* bit mask... all 0 for data <= 0., all 1 else */

		const vector float estimate1 = (vector float)vec_and((vector unsigned char)vec_rsqrte(data1),mask1); 
		const vector float estimate2 = (vector float)vec_and((vector unsigned char)vec_rsqrte(data2),mask2); 
		const vector float estimate3 = (vector float)vec_and((vector unsigned char)vec_rsqrte(data3),mask3); 
		const vector float estimate4 = (vector float)vec_and((vector unsigned char)vec_rsqrte(data4),mask4); 

		/* this can still be improved.... */
		data1 = vec_madd(data1,vec_madd( vec_nmsub( data1, vec_madd( estimate1, estimate1, zero ), one ), vec_madd( estimate1, oneHalf, zero ), estimate1 ), zero);
		data2 = vec_madd(data2,vec_madd( vec_nmsub( data2, vec_madd( estimate2, estimate2, zero ), one ), vec_madd( estimate2, oneHalf, zero ), estimate2 ), zero);
		data3 = vec_madd(data3,vec_madd( vec_nmsub( data3, vec_madd( estimate3, estimate3, zero ), one ), vec_madd( estimate3, oneHalf, zero ), estimate3 ), zero);
		data4 = vec_madd(data4,vec_madd( vec_nmsub( data4, vec_madd( estimate4, estimate4, zero ), one ), vec_madd( estimate4, oneHalf, zero ), estimate4 ), zero);
		
		vec_st(data1, 0,dst);
		vec_st(data2,16,dst);
		vec_st(data3,32,dst);
		vec_st(data4,48,dst);
#endif
	}
	return w+4;
}

/* Attention: there's a difference to sigsqrt_perform which delivers non-zero for a zero input... i don't think the latter is intended... */
t_int *sigrsqrt_perf_simd(t_int *w)
{
    const t_float *src = (const t_float *)w[1];
    t_float *dst = (t_float *)w[2];
    int n = w[3]>>4;
	
	const vector float zero = (vector float)(0);
	const vector float oneHalf = (vector float)(0.5);
	const vector float one = (vector float)(1.0);

	for(; n--; src += 16,dst += 16) {
		/* http://developer.apple.com/hardware/ve/algorithms.html

			Just as in Miller's scalar sigrsqrt_perform, 
			first a rsqrt estimate is calculated which is then refined by one round of Newton-Raphson.
			Here, to avoid branching a mask is generated which zeroes out eventual resulting NANs.
		*/

#ifdef USEVECLIB
		/* no zero checking here */
		vec_st(vrsqrtf(vec_ld( 0,src)), 0,dst); 
		vec_st(vrsqrtf(vec_ld(16,src)),16,dst); 
		vec_st(vrsqrtf(vec_ld(32,src)),32,dst); 
		vec_st(vrsqrtf(vec_ld(48,src)),48,dst); 
#else
		vector float data1 = vec_ld( 0,src);
		vector float data2 = vec_ld(16,src);
		vector float data3 = vec_ld(32,src);
		vector float data4 = vec_ld(48,src);

		const vector unsigned char mask1 = vec_nor((vector unsigned char)vec_cmple(data1,zero),(vector unsigned char)zero); /* bit mask... all 0 for data <= 0., all 1 else */
		const vector unsigned char mask2 = vec_nor((vector unsigned char)vec_cmple(data2,zero),(vector unsigned char)zero); /* bit mask... all 0 for data <= 0., all 1 else */
		const vector unsigned char mask3 = vec_nor((vector unsigned char)vec_cmple(data3,zero),(vector unsigned char)zero); /* bit mask... all 0 for data <= 0., all 1 else */
		const vector unsigned char mask4 = vec_nor((vector unsigned char)vec_cmple(data4,zero),(vector unsigned char)zero); /* bit mask... all 0 for data <= 0., all 1 else */

		const vector float estimate1 = (vector float)vec_and((vector unsigned char)vec_rsqrte(data1),mask1); 
		const vector float estimate2 = (vector float)vec_and((vector unsigned char)vec_rsqrte(data2),mask2); 
		const vector float estimate3 = (vector float)vec_and((vector unsigned char)vec_rsqrte(data3),mask3); 
		const vector float estimate4 = (vector float)vec_and((vector unsigned char)vec_rsqrte(data4),mask4); 
		
		data1 = vec_nmsub( data1, vec_madd( estimate1, estimate1, zero ), one );
		data2 = vec_nmsub( data2, vec_madd( estimate2, estimate2, zero ), one );
		data3 = vec_nmsub( data3, vec_madd( estimate3, estimate3, zero ), one );
		data4 = vec_nmsub( data4, vec_madd( estimate4, estimate4, zero ), one );

		data1 = vec_madd( data1, vec_madd( estimate1, oneHalf, zero ), estimate1 );
		data2 = vec_madd( data2, vec_madd( estimate2, oneHalf, zero ), estimate2 );
		data3 = vec_madd( data3, vec_madd( estimate3, oneHalf, zero ), estimate3 );
		data4 = vec_madd( data4, vec_madd( estimate4, oneHalf, zero ), estimate4 );
		
		vec_st(data1, 0,dst);
		vec_st(data2,16,dst);
		vec_st(data3,32,dst);
		vec_st(data4,48,dst);
#endif
	}
	return w+4;
}

int simd_runtime_check()
{
	return 1;
}


#endif
