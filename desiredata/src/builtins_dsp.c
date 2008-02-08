/* Copyright (c) 2007 Mathieu Bouchard
   Copyright (c) 1997-1999 Miller Puckette.
   For information on usage and redistribution,
   and for a DISCLAIMER OF ALL WARRANTIES,
   see the file "LICENSE.txt" in this distribution.  */

/*  arithmetic binops (+, -, *, /).
If no creation argument is given, there are two signal inlets for vector/vector
operation; otherwise it's vector/scalar and the second inlet takes a float
to reset the value.
*/

//#define HAVE_LIBFFTW3F

#define PD_PLUSPLUS_FACE
#include "desire.h"
#include "m_simd.h"
#include "s_stuff.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
extern int ugen_getsortno ();
#define DEFDELVS 64             /* LATER get this from canvas at DSP time */
#ifdef HAVE_LIBFFTW3F
#include <fftw3.h>
#endif
#define DEFSENDVS 64    /* LATER get send to get this from canvas */
#define LOGTEN 2.302585092994
#define UNITBIT32 1572864.  /* 3*2^19; bit 32 has place value 1 */
#define int32 int
#ifdef BIGENDIAN
#define HIOFFSET 0    /* word offset to find MSB */
#define LOWOFFSET 1    /* word offset to find LSB */
#else
#define HIOFFSET 1
#define LOWOFFSET 0
#endif

#undef CLASS_MAINSIGNALIN
/* because C++ bitches about null pointers, we'll use 8 instead (really): */
#define CLASS_MAINSIGNALIN(c, type, field) class_domainsignalin(c, (char *)(&((type *)8)->field) - (char *)8)

#define clock_new(a,b) clock_new(a,(t_method)b)

#undef min
#undef max

/* ----------------------------- plus ----------------------------- */

struct t_dop : t_object {
    float a;
    t_float b; /* inlet value, if necessary */
};

#define DSPDECL(NAME) static t_class *NAME##_class, *scalar##NAME##_class; typedef t_dop t_##NAME, t_scalar##NAME;
DSPDECL(plus)
DSPDECL(minus)
DSPDECL(times)
DSPDECL(over)
DSPDECL(max)
DSPDECL(min)
DSPDECL(lt)
DSPDECL(gt)
DSPDECL(le)
DSPDECL(ge)
DSPDECL(eq)
DSPDECL(ne)

#define DSPNEW(NAME,SYM) \
static void *NAME##_new(t_symbol *s, int argc, t_atom *argv) { \
    if (argc > 1) error("extra arguments ignored"); \
    t_dop *x = (t_dop *)pd_new(argc ? scalar##NAME##_class : NAME##_class); \
    if (argc) { \
        floatinlet_new(x, &x->b); \
        x->b = atom_getfloatarg(0, argc, argv); \
    } else inlet_new(x, x, &s_signal, &s_signal); \
    outlet_new(x, &s_signal); \
    x->a = 0; \
    return x;} \
static void NAME##_setup() { \
    t_class *c = NAME##_class = class_new2(SYM,NAME##_new,0,sizeof(t_dop),0,"*"); \
    class_addmethod2(c, NAME##_dsp, "dsp",""); \
    CLASS_MAINSIGNALIN(c, t_dop, a); \
    class_sethelpsymbol(NAME##_class, gensym("sigbinops")); \
    c = scalar##NAME##_class = class_new2(SYM,0,0,sizeof(t_dop),0,""); \
    CLASS_MAINSIGNALIN(c, t_dop, a); \
    class_addmethod2(c, scalar##NAME##_dsp, "dsp",""); \
    class_sethelpsymbol(scalar##NAME##_class, gensym("sigbinops"));}

/* use when simd functions are present */
#define DSPDSP(NAME) \
static void NAME##_dsp(t_minus *x, t_signal **sp) { \
    const int n = sp[0]->n; \
    if(n&7) dsp_add(NAME##_perform, 4, sp[0]->v, sp[1]->v, sp[2]->v, n); \
    else if(SIMD_CHECK3(n,sp[0]->v,sp[1]->v,sp[2]->v)) \
    	 dsp_add(NAME##_perf_simd, 4, sp[0]->v, sp[1]->v, sp[2]->v, n); \
    else dsp_add(NAME##_perf8, 4, sp[0]->v, sp[1]->v, sp[2]->v, n);} \
static void scalar##NAME##_dsp(t_scalarminus *x, t_signal **sp) { \
    const int n = sp[0]->n;\
    if(n&7) dsp_add(scalar##NAME##_perform, 4, sp[0]->v, &x->b,sp[1]->v, n);\
    else if(SIMD_CHECK2(n,sp[0]->v,sp[1]->v)) \
    	 dsp_add(scalar##NAME##_perf_simd, 4, sp[0]->v, &x->b, sp[1]->v, n);\
    else dsp_add(scalar##NAME##_perf8, 4, sp[0]->v, &x->b, sp[1]->v, n);}

/* use when simd functions are missing */
#define DSPDSP2(NAME) \
static void NAME##_dsp(t_minus *x, t_signal **sp) { \
    const int n = sp[0]->n; \
    if(n&7) dsp_add(NAME##_perform, 4, sp[0]->v, sp[1]->v, sp[2]->v, n); \
    else dsp_add(NAME##_perf8, 4, sp[0]->v, sp[1]->v, sp[2]->v, n);} \
static void scalar##NAME##_dsp(t_scalarminus *x, t_signal **sp) { \
    const int n = sp[0]->n;\
    if(n&7) dsp_add(scalar##NAME##_perform, 4, sp[0]->v, &x->b,sp[1]->v, n);\
    else dsp_add(scalar##NAME##_perf8, 4, sp[0]->v, &x->b, sp[1]->v, n);}

#define PERFORM(NAME,EXPR) \
t_int *NAME##_perform(t_int *w) { \
    t_float *in1 = (t_float *)w[1], *in2 = (t_float *)w[2], *out = (t_float *)w[3]; \
    int n = (int)w[4]; \
    while (n--) {t_float a=*in1++, b=*in2++; *out++ = (EXPR);} \
    return w+5;} \
t_int *NAME##_perf8(t_int *w) { \
    t_float *in1 = (t_float *)w[1], *in2 = (t_float *)w[2], *out = (t_float *)w[3]; \
    int n = (int)w[4]; \
    for (; n; n -= 8, in1 += 8, in2 += 8, out += 8) { \
        {t_float a=in1[0], b=in2[0]; out[0] = (EXPR);} \
        {t_float a=in1[1], b=in2[1]; out[1] = (EXPR);} \
        {t_float a=in1[2], b=in2[2]; out[2] = (EXPR);} \
        {t_float a=in1[3], b=in2[3]; out[3] = (EXPR);} \
        {t_float a=in1[4], b=in2[4]; out[4] = (EXPR);} \
        {t_float a=in1[5], b=in2[5]; out[5] = (EXPR);} \
        {t_float a=in1[6], b=in2[6]; out[6] = (EXPR);} \
        {t_float a=in1[7], b=in2[7]; out[7] = (EXPR);}} \
    return w+5;} \
t_int *scalar##NAME##_perform(t_int *w) { \
    t_float *in = (t_float *)w[1]; t_float b = *(t_float *)w[2]; t_float *out = (t_float *)w[3]; \
    int n = (int)w[4]; \
    while (n--) {t_float a=*in++; *out++ = (EXPR);} \
    return w+5;} \
t_int *scalar##NAME##_perf8(t_int *w) { \
    t_float *in = (t_float *)w[1]; t_float b = *(t_float *)w[2]; t_float *out = (t_float *)w[3]; \
    int n = (int)w[4]; \
    for (; n; n -= 8, in += 8, out += 8) { \
	{t_float a=in[0]; out[0] = (EXPR);} \
	{t_float a=in[1]; out[1] = (EXPR);} \
	{t_float a=in[2]; out[2] = (EXPR);} \
	{t_float a=in[3]; out[3] = (EXPR);} \
	{t_float a=in[4]; out[4] = (EXPR);} \
	{t_float a=in[5]; out[5] = (EXPR);} \
	{t_float a=in[6]; out[6] = (EXPR);} \
	{t_float a=in[7]; out[7] = (EXPR);}} \
    return w+5;}

void dsp_add_plus(t_sample *in1, t_sample *in2, t_sample *out, int n) {
    if (n&7)                            dsp_add(plus_perform,   4, in1, in2, out, n);
    else if(SIMD_CHECK3(n,in1,in2,out)) dsp_add(plus_perf_simd, 4, in1, in2, out, n);
    else                                dsp_add(plus_perf8,     4, in1, in2, out, n);
}

/* T.Grill - squaring: optimized * for equal input signals */
t_int *sqr_perf8(t_int *w) {
    t_float *in = (t_float *)w[1];
    t_float *out = (t_float *)w[2];
    int n = (int)w[3];
    for (; n; n -= 8, in += 8, out += 8) {
    	float f0 = in[0], f1 = in[1], f2 = in[2], f3 = in[3];
    	float f4 = in[4], f5 = in[5], f6 = in[6], f7 = in[7];
    	out[0] = f0 * f0; out[1] = f1 * f1; out[2] = f2 * f2; out[3] = f3 * f3;
    	out[4] = f4 * f4; out[5] = f5 * f5; out[6] = f6 * f6; out[7] = f7 * f7;
    }
    return w+4;
}

/* T.Grill - added optimization for equal input signals */
static void times_dsp(t_times *x, t_signal **sp) {
    const int n = sp[0]->n;
    if (n&7) dsp_add(times_perform, 4, sp[0]->v, sp[1]->v, sp[2]->v, n);
    else if(sp[0]->v == sp[1]->v) {
	if(SIMD_CHECK2(n,sp[0]->v,sp[2]->v))
	     dsp_add(sqr_perf_simd, 3, sp[0]->v, sp[2]->v, n);
	else dsp_add(sqr_perf8, 3, sp[0]->v, sp[2]->v, n);
    } else {
	if(SIMD_CHECK3(n,sp[0]->v,sp[1]->v,sp[2]->v))
	     dsp_add(times_perf_simd, 4, sp[0]->v, sp[1]->v, sp[2]->v, n);
	else dsp_add(times_perf8, 4, sp[0]->v, sp[1]->v, sp[2]->v, n);
    }
}
static void scalartimes_dsp(t_scalartimes *x, t_signal **sp) {
    const int n = sp[0]->n;
    if (n&7) dsp_add(scalartimes_perform, 4, sp[0]->v, &x->b,sp[1]->v, n);
    else if(SIMD_CHECK2(n,sp[0]->v,sp[1]->v))
	 dsp_add(scalartimes_perf_simd, 4, sp[0]->v, &x->b, sp[1]->v, n);
    else dsp_add(scalartimes_perf8, 4, sp[0]->v, &x->b, sp[1]->v, n);
}

PERFORM(plus ,a+b)    DSPDSP(plus)      DSPNEW(plus ,"+~")
PERFORM(minus,a-b)    DSPDSP(minus)     DSPNEW(minus,"-~")
PERFORM(times,a*b)    /*DSPDSP(times)*/ DSPNEW(times,"*~")
/*PERFORM(over,a/b)*/ DSPDSP(over)      DSPNEW(over ,"/~")
PERFORM(min ,a<b?a:b) DSPDSP(min)       DSPNEW(min  ,"min~")
PERFORM(max ,a>b?a:b) DSPDSP(max)       DSPNEW(max  ,"max~")
PERFORM(lt  ,a<b)     DSPDSP2(lt)       DSPNEW(lt   ,"<~")
PERFORM(gt  ,a>b)     DSPDSP2(gt)       DSPNEW(gt   ,">~")
PERFORM(le  ,a<=b)    DSPDSP2(le)       DSPNEW(le   ,"<=~")
PERFORM(ge  ,a>=b)    DSPDSP2(ge)       DSPNEW(ge   ,">=~")
PERFORM(eq  ,a==b)    DSPDSP2(eq)       DSPNEW(eq   ,"==~")
PERFORM(ne  ,a!=b)    DSPDSP2(ne)       DSPNEW(ne   ,"!=~")

t_int *over_perform(t_int *w) {
    t_float *in1 = (t_float *)w[1], *in2 = (t_float *)w[2], *out = (t_float *)w[3];
    int n = (int)w[4];
    while (n--) {
        float g = *in2++;
        *out++ = (g ? *in1++ / g : 0);
    }
    return w+5;
}
t_int *over_perf8(t_int *w) {
    t_float *in1 = (t_float *)w[1];
    t_float *in2 = (t_float *)w[2];
    t_float *out = (t_float *)w[3];
    int n = (int)w[4];
    for (; n; n -= 8, in1 += 8, in2 += 8, out += 8) {
        float f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
        float f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];
        float g0 = in2[0], g1 = in2[1], g2 = in2[2], g3 = in2[3];
        float g4 = in2[4], g5 = in2[5], g6 = in2[6], g7 = in2[7];
        out[0] = (g0? f0 / g0 : 0);
        out[1] = (g1? f1 / g1 : 0);
        out[2] = (g2? f2 / g2 : 0);
        out[3] = (g3? f3 / g3 : 0);
        out[4] = (g4? f4 / g4 : 0);
        out[5] = (g5? f5 / g5 : 0);
        out[6] = (g6? f6 / g6 : 0);
        out[7] = (g7? f7 / g7 : 0);
    }
    return w+5;
}
/* T.Grill - added check for zero */
t_int *scalarover_perform(t_int *w) {
    t_float *in = (t_float *)w[1];
    t_float f = *(t_float *)w[2];
    t_float *out = (t_float *)w[3];
    int n = (int)w[4];
    if(f) f = 1./f;
    while (n--) *out++ = *in++ * f;
    return w+5;
}
t_int *scalarover_perf8(t_int *w) {
    t_float *in = (t_float *)w[1];
    t_float g = *(t_float *)w[2];
    t_float *out = (t_float *)w[3];
    int n = (int)w[4];
    if (g) g = 1.f / g;
    for (; n; n -= 8, in += 8, out += 8) {
        out[0] = in[0] * g; out[1] = in[1] * g; out[2] = in[2] * g; out[3] = in[3] * g;
        out[4] = in[4] * g; out[5] = in[5] * g; out[6] = in[6] * g; out[7] = in[7] * g;
    }
    return w+5;
}

/* ------------------------- tabwrite~ -------------------------- */
static t_class *tabwrite_tilde_class;
struct t_tabwrite_tilde : t_object {
    int phase;
    int nsampsintab;
    float *vec;
    t_symbol *arrayname;
    float a;
};
static void *tabwrite_tilde_new(t_symbol *s) {
    t_tabwrite_tilde *x = (t_tabwrite_tilde *)pd_new(tabwrite_tilde_class);
    x->phase = 0x7fffffff;
    x->arrayname = s;
    x->a = 0;
    return x;
}
static void tabwrite_tilde_redraw(t_tabwrite_tilde *x) {
    t_garray *a = (t_garray *)pd_findbyclass(x->arrayname, garray_class);
    if (!a) bug("tabwrite_tilde_redraw");
    else garray_redraw(a);
}
static t_int *tabwrite_tilde_perform(t_int *w) {
    t_tabwrite_tilde *x = (t_tabwrite_tilde *)w[1];
    t_float *in = (t_float *)w[2];
    int n = (int)w[3], phase = x->phase, endphase = x->nsampsintab;
    if (!x->vec) goto bad;
    if (endphase > phase) {
        int nxfer = endphase - phase;
        float *fp = x->vec + phase;
        if (nxfer > n) nxfer = n;
        phase += nxfer;
	testcopyvec(fp, in, nxfer);
        if (phase >= endphase) {
            tabwrite_tilde_redraw(x);
            phase = 0x7fffffff;
        }
        x->phase = phase;
    } else x->phase = 0x7fffffff;
bad:
    return w+4;
}
static t_int *tabwrite_tilde_perf_simd(t_int *w) {
    t_tabwrite_tilde *x = (t_tabwrite_tilde *)w[1];
    t_float *in = (t_float *)w[2];
    int n = (int)w[3], phase = x->phase, endphase = x->nsampsintab;
    if (!x->vec) goto bad;
    if (endphase > phase) {
        int nxfer = endphase - phase;
        float *fp = x->vec + phase;
        if (nxfer > n) nxfer = n;
        phase += nxfer;
	if (SIMD_CHKCNT(nxfer)) testcopyvec_simd(fp, in, nxfer);
	else                    testcopyvec(fp, in, nxfer);
        if (phase >= endphase) {
            tabwrite_tilde_redraw(x);
            phase = 0x7fffffff;
        }
        x->phase = phase;
    } else x->phase = 0x7fffffff;
bad:
    return w+4;
}
void tabwrite_tilde_set(t_tabwrite_tilde *x, t_symbol *s) {
    x->arrayname = s;
    t_garray *a = (t_garray *)pd_findbyclass(x->arrayname, garray_class);
    if (!a) {
        if (*s->name) error("tabwrite~: %s: no such array", x->arrayname->name);
        x->vec = 0;
    } else if (!garray_getfloatarray(a, &x->nsampsintab, &x->vec)) {
        error("%s: bad template for tabwrite~", x->arrayname->name);
        x->vec = 0;
    } else garray_usedindsp(a);
}
static void tabwrite_tilde_dsp(t_tabwrite_tilde *x, t_signal **sp) {
    tabwrite_tilde_set(x, x->arrayname);
    if (SIMD_CHECK1(sp[0]->n, sp[0]->v))
	dsp_add(tabwrite_tilde_perf_simd, 3, x, sp[0]->v, sp[0]->n);
    else  dsp_add(tabwrite_tilde_perform, 3, x, sp[0]->v, sp[0]->n);
}
static void tabwrite_tilde_bang(t_tabwrite_tilde *x) {x->phase = 0;}
static void tabwrite_tilde_start(t_tabwrite_tilde *x, t_floatarg f) {x->phase = (int)max((int)f,0);}
static void tabwrite_tilde_stop(t_tabwrite_tilde *x) {
    if (x->phase != 0x7fffffff) {
        tabwrite_tilde_redraw(x);
        x->phase = 0x7fffffff;
    }
}

/* ------------ tabplay~ - non-transposing sample playback --------------- */
static t_class *tabplay_tilde_class;
struct t_tabplay_tilde : t_object {
    int phase;
    int nsampsintab;
    int limit;
    float *vec;
    t_symbol *arrayname;
    t_clock *clock;
};
static void tabplay_tilde_tick(t_tabplay_tilde *x);
static void *tabplay_tilde_new(t_symbol *s) {
    t_tabplay_tilde *x = (t_tabplay_tilde *)pd_new(tabplay_tilde_class);
    x->clock = clock_new(x, tabplay_tilde_tick);
    x->phase = 0x7fffffff;
    x->limit = 0;
    x->arrayname = s;
    outlet_new(x, &s_signal);
    outlet_new(x, &s_bang);
    return x;
}
static t_int *tabplay_tilde_perform(t_int *w) {
    t_tabplay_tilde *x = (t_tabplay_tilde *)w[1];
    t_float *out = (t_float *)w[2], *fp;
    int n = (int)w[3], phase = x->phase, endphase = (x->nsampsintab < x->limit ? x->nsampsintab : x->limit), nxfer, n3;
    if (!x->vec || phase >= endphase) {while (n--) *out++ = 0; goto bye;}
    nxfer = min(endphase-phase,n);
    fp = x->vec + phase;
    n3 = n - nxfer;
    phase += nxfer;
    while (nxfer--) *out++ = *fp++;
    if (phase >= endphase) {
        clock_delay(x->clock, 0);
        x->phase = 0x7fffffff;
        while (n3--) *out++ = 0;
    } else x->phase = phase;
    return w+4;
bye:
    return w+4;
}
void tabplay_tilde_set(t_tabplay_tilde *x, t_symbol *s) {
    t_garray *a = (t_garray *)pd_findbyclass(x->arrayname, garray_class);
    x->arrayname = s;
    if (!a) {
        if (*s->name) error("tabplay~: %s: no such array", x->arrayname->name);
        x->vec = 0;
    } else if (!garray_getfloatarray(a, &x->nsampsintab, &x->vec)) {
        error("%s: bad template for tabplay~", x->arrayname->name);
        x->vec = 0;
    } else garray_usedindsp(a);
}
static void tabplay_tilde_dsp(t_tabplay_tilde *x, t_signal **sp) {
    tabplay_tilde_set(x, x->arrayname);
    dsp_add(tabplay_tilde_perform, 3, x, sp[0]->v, sp[0]->n);
}
static void tabplay_tilde_list(t_tabplay_tilde *x, t_symbol *s, int argc, t_atom *argv) {
    long  start = atom_getintarg(0, argc, argv);
    long length = atom_getintarg(1, argc, argv);
    if (start < 0) start = 0;
    if (length <= 0) x->limit = 0x7fffffff; else x->limit = start + length;
    x->phase = start;
}
static void tabplay_tilde_stop(t_tabplay_tilde *x) {x->phase = 0x7fffffff;}
static void tabplay_tilde_tick(t_tabplay_tilde *x) {outlet_bang(x->out(1));}
static void tabplay_tilde_free(t_tabplay_tilde *x) {clock_free(x->clock);}

/******************** tabread~ ***********************/
static t_class *tabread_tilde_class;
struct t_tabread_tilde : t_object {
    int npoints;
    float *vec;
    t_symbol *arrayname;
    float a;
};
static void *tabread_tilde_new(t_symbol *s) {
    t_tabread_tilde *x = (t_tabread_tilde *)pd_new(tabread_tilde_class);
    x->arrayname = s;
    x->vec = 0;
    outlet_new(x, &s_signal);
    x->a = 0;
    return x;
}
static t_int *tabread_tilde_perform(t_int *w) {
    t_tabread_tilde *x = (t_tabread_tilde *)w[1];
    t_float *in = (t_float *)w[2];
    t_float *out = (t_float *)w[3];
    int n = (int)w[4];
    float *buf = x->vec;
    int maxindex = x->npoints - 1;
    if (!buf) {while (n--) *out++ = 0; goto bad;}
    for (int i = 0; i < n; i++) {
        int index = (int)*in++;
        if (index < 0) index = 0; else if (index > maxindex) index = maxindex;
        *out++ = buf[index];
    }
bad:return w+5;
}
void tabread_tilde_set(t_tabread_tilde *x, t_symbol *s) {
    x->arrayname = s;
    t_garray *a = (t_garray *)pd_findbyclass(x->arrayname, garray_class);
    if (!a) {
        if (*s->name) error("tabread~: %s: no such array", x->arrayname->name);
        x->vec = 0;
    } else if (!garray_getfloatarray(a, &x->npoints, &x->vec)) {
        error("%s: bad template for tabread~", x->arrayname->name);
        x->vec = 0;
    } else garray_usedindsp(a);
}
static void tabread_tilde_dsp(t_tabread_tilde *x, t_signal **sp) {
    tabread_tilde_set(x, x->arrayname);
    dsp_add(tabread_tilde_perform, 4, x, sp[0]->v, sp[1]->v, sp[0]->n);
}
static void tabread_tilde_free(t_tabread_tilde *x) {}

/******************** tabread4~ ***********************/
static t_class *tabread4_tilde_class;
struct t_tabread4_tilde : t_object {
    int npoints;
    float *vec;
    t_symbol *arrayname;
    float a;
};
static void *tabread4_tilde_new(t_symbol *s) {
    t_tabread4_tilde *x = (t_tabread4_tilde *)pd_new(tabread4_tilde_class);
    x->arrayname = s;
    x->vec = 0;
    outlet_new(x, &s_signal);
    x->a = 0;
    return x;
}
static t_int *tabread4_tilde_perform(t_int *w) {
    t_tabread4_tilde *x = (t_tabread4_tilde *)w[1];
    t_float *in = (t_float *)w[2];
    t_float *out = (t_float *)w[3];
    int n = (int)w[4];
    int maxindex;
    float *buf = x->vec, *fp;
    maxindex = x->npoints - 3;
    if (!buf) goto zero;
#if 0       /* test for spam -- I'm not ready to deal with this */
    for (i = 0,  xmax = 0, xmin = maxindex,  fp = in1; i < n; i++,  fp++) {
        float f = *in1;
        if (f < xmin) xmin = f;
        else if (f > xmax) xmax = f;
    }
    if (xmax < xmin + x->c_maxextent) xmax = xmin + x->c_maxextent;
    for (i = 0, splitlo = xmin+ x->c_maxextent, splithi = xmax - x->c_maxextent,
        fp = in1; i < n; i++,  fp++) {
        float f = *in1;
        if (f > splitlo && f < splithi) goto zero;
    }
#endif
    for (int i=0; i<n; i++) {
        float findex = *in++;
        int index = (int)findex;
        float frac;
        if (index < 1) index = 1, frac = 0;
        else if (index > maxindex) index = maxindex, frac = 1;
        else frac = findex - index;
        fp = buf + index;
        float a=fp[-1], b=fp[0], c=fp[1], d=fp[2];
        /* if (!i && !(count++ & 1023)) post("fp = %lx,  shit = %lx,  b = %f",  fp, buf->b_shit,  b); */
        float cminusb = c-b;
        *out++ = b + frac * (cminusb - 0.1666667f * (1.-frac) * ((d - a - 3.0f * cminusb) * frac + (d + 2.0f*a - 3.0f*b)));
    }
    return w+5;
 zero:
    while (n--) *out++ = 0;
    return w+5;
}
void tabread4_tilde_set(t_tabread4_tilde *x, t_symbol *s) {
    x->arrayname = s;
    t_garray *a = (t_garray *)pd_findbyclass(x->arrayname, garray_class);
    if (!a) {
        if (*s->name) error("tabread4~: %s: no such array", x->arrayname->name);
        x->vec = 0;
    } else if (!garray_getfloatarray(a, &x->npoints, &x->vec)) {
        error("%s: bad template for tabread4~", x->arrayname->name);
        x->vec = 0;
    } else garray_usedindsp(a);
}
static void tabread4_tilde_dsp(t_tabread4_tilde *x, t_signal **sp) {
    tabread4_tilde_set(x, x->arrayname);
    dsp_add(tabread4_tilde_perform, 4, x, sp[0]->v, sp[1]->v, sp[0]->n);
}
static void tabread4_tilde_free(t_tabread4_tilde *x) {}

/******************** tabosc4~ ***********************/

#define UNITBIT32 1572864.  /* 3*2^19; bit 32 has place value 1 */

#ifdef BIGENDIAN
#define HIOFFSET 0    /* word offset to find MSB */
#define LOWOFFSET 1    /* word offset to find LSB */
#else
#define HIOFFSET 1
#define LOWOFFSET 0
#endif
#include <sys/types.h>
//#define int32 int32_t
#define int32 int

union tabfudge {
    double d;
    int32 i[2];
};
static t_class *tabosc4_tilde_class;
struct t_tabosc4_tilde : t_object {
    float fnpoints;
    float finvnpoints;
    float *vec;
    t_symbol *arrayname;
    float a;
    double phase;
    float conv;
};
static void *tabosc4_tilde_new(t_symbol *s) {
    t_tabosc4_tilde *x = (t_tabosc4_tilde *)pd_new(tabosc4_tilde_class);
    x->arrayname = s;
    x->vec = 0;
    x->fnpoints = 512.;
    x->finvnpoints = (1./512.);
    outlet_new(x, &s_signal);
    inlet_new(x, x, &s_float, gensym("ft1"));
    x->a = 0;
    return x;
}
static t_int *tabosc4_tilde_perform(t_int *w) {
    t_tabosc4_tilde *x = (t_tabosc4_tilde *)w[1];
    t_float *in = (t_float *)w[2];
    t_float *out = (t_float *)w[3];
    int n = (int)w[4];
    union tabfudge tf;
    float fnpoints = x->fnpoints;
    int mask = (int)(fnpoints-1);
    float conv = fnpoints * x->conv;
    float *tab = x->vec, *addr;
    double dphase = fnpoints * x->phase + UNITBIT32;
    if (!tab) {while (n--) *out++ = 0; return w+5;}
    tf.d = UNITBIT32;
    int normhipart = tf.i[HIOFFSET];
#if 1
    while (n--) {
        tf.d = dphase;
        dphase += *in++ * conv;
        addr = tab + (tf.i[HIOFFSET] & mask);
        tf.i[HIOFFSET] = normhipart;
        float frac = tf.d - UNITBIT32;
        float a = addr[0];
        float b = addr[1];
        float c = addr[2];
        float d = addr[3];
        float cminusb = c-b;
        *out++ = b + frac * (cminusb - 0.1666667f * (1.-frac) * ((d - a - 3.0f * cminusb) * frac + (d + 2.0f*a - 3.0f*b)));
    }
#endif
    tf.d = UNITBIT32 * fnpoints;
    normhipart = tf.i[HIOFFSET];
    tf.d = dphase + (UNITBIT32 * fnpoints - UNITBIT32);
    tf.i[HIOFFSET] = normhipart;
    x->phase = (tf.d - UNITBIT32 * fnpoints)  * x->finvnpoints;
    return w+5;
}
void tabosc4_tilde_set(t_tabosc4_tilde *x, t_symbol *s) {
    t_garray *a = (t_garray *)pd_findbyclass(x->arrayname, garray_class);
    int npoints, pointsinarray;
    x->arrayname = s;
    if (!a) {
        if (*s->name) error("tabosc4~: %s: no such array", x->arrayname->name);
        x->vec = 0;
    } else if (!garray_getfloatarray(a, &pointsinarray, &x->vec)) {
        error("%s: bad template for tabosc4~", x->arrayname->name);
        x->vec = 0;
    } else if ((npoints = pointsinarray - 3) != (1 << ilog2(pointsinarray - 3))) {
        error("%s: number of points (%d) not a power of 2 plus three", x->arrayname->name, pointsinarray);
        x->vec = 0;
        garray_usedindsp(a);
    } else {
        x->fnpoints = npoints;
        x->finvnpoints = 1./npoints;
        garray_usedindsp(a);
    }
}
static void tabosc4_tilde_ft1(t_tabosc4_tilde *x, t_float f) {
    x->phase = f;
}
static void tabosc4_tilde_dsp(t_tabosc4_tilde *x, t_signal **sp) {
    x->conv = 1. / sp[0]->sr;
    tabosc4_tilde_set(x, x->arrayname);
    dsp_add(tabosc4_tilde_perform, 4, x, sp[0]->v, sp[1]->v, sp[0]->n);
}
static void tabosc4_tilde_setup() {
}

static void tab_tilde_setup() {
    t_class *c;
    c = tabwrite_tilde_class = class_new2("tabwrite~",tabwrite_tilde_new,0,sizeof(t_tabwrite_tilde),0,"S");
    CLASS_MAINSIGNALIN(c, t_tabwrite_tilde, a);
    class_addmethod2(c, tabwrite_tilde_dsp, "dsp","");
    class_addmethod2(c, tabwrite_tilde_set, "set","s");
    class_addmethod2(c, tabwrite_tilde_stop,"stop","");
    class_addmethod2(c, tabwrite_tilde_start,"start","F");
    class_addbang(c, tabwrite_tilde_bang);
    c = tabplay_tilde_class = class_new2("tabplay~",tabplay_tilde_new,tabplay_tilde_free,sizeof(t_tabplay_tilde),0,"S");
    class_addmethod2(c, tabplay_tilde_dsp,  "dsp","");
    class_addmethod2(c, tabplay_tilde_stop, "stop","");
    class_addmethod2(c, tabplay_tilde_set,  "set","S");
    class_addlist(c, tabplay_tilde_list);
    c = tabread_tilde_class = class_new2("tabread~",tabread_tilde_new,tabread_tilde_free, sizeof(t_tabread_tilde),0,"S");
    CLASS_MAINSIGNALIN(c, t_tabread_tilde, a);
    class_addmethod2(c, tabread_tilde_dsp, "dsp","");
    class_addmethod2(c, tabread_tilde_set, "set","s");
    c = tabread4_tilde_class = class_new2("tabread4~",tabread4_tilde_new,tabread4_tilde_free,sizeof(t_tabread4_tilde),0,"S");
    CLASS_MAINSIGNALIN(c, t_tabread4_tilde, a);
    class_addmethod2(c, tabread4_tilde_dsp, "dsp","");
    class_addmethod2(c, tabread4_tilde_set, "set","s");
    c = tabosc4_tilde_class = class_new2("tabosc4~",tabosc4_tilde_new,0,sizeof(t_tabosc4_tilde),0,"S");
    CLASS_MAINSIGNALIN(c, t_tabosc4_tilde, a);
    class_addmethod2(c, tabosc4_tilde_dsp, "dsp","");
    class_addmethod2(c, tabosc4_tilde_set, "set","s");
    class_addmethod2(c, tabosc4_tilde_ft1, "ft1","f");
}

/* ------------------------ tabsend~ ------------------------- */
static t_class *tabsend_class;
struct t_tabsend : t_object {
    float *vec;
    int graphperiod;
    int graphcount;
    t_symbol *arrayname;
    float a;
};
static void *tabsend_new(t_symbol *s) {
    t_tabsend *x = (t_tabsend *)pd_new(tabsend_class);
    x->graphcount = 0;
    x->arrayname = s;
    x->a = 0;
    return x;
}
static t_int *tabsend_perform(t_int *w) {
    t_tabsend *x = (t_tabsend *)w[1];
    t_float *in = (t_float *)w[2];
    int n = w[3];
    t_float *dest = x->vec;
    int i = x->graphcount;
    if (!x->vec) goto bad;
    testcopyvec(dest,in,n);
    if (!i--) {
        t_garray *a = (t_garray *)pd_findbyclass(x->arrayname, garray_class);
        if (!a) bug("tabsend_dsp");
        else garray_redraw(a);
        i = x->graphperiod;
    }
    x->graphcount = i;
bad:
    return w+4;
}
static t_int *tabsend_perf_simd(t_int *w) {
    t_tabsend *x = (t_tabsend *)w[1];
    t_float *in = (t_float *)w[2];
    int n = w[3];
    t_float *dest = x->vec;
    int i = x->graphcount;
    if (!x->vec) goto bad;
    testcopyvec_simd(dest,in,n);
    if (!i--) {
        t_garray *a = (t_garray *)pd_findbyclass(x->arrayname, garray_class);
        if (!a) bug("tabsend_dsp");
        else garray_redraw(a);
        i = x->graphperiod;
    }
    x->graphcount = i;
bad:
    return w+4;
}
static void tabsend_dsp(t_tabsend *x, t_signal **sp) {
    int vecsize;
    t_garray *a = (t_garray *)pd_findbyclass(x->arrayname, garray_class);
    if (!a) {
        if (*x->arrayname->name) {error("tabsend~: %s: no such array", x->arrayname->name); return;}
    } else if (!garray_getfloatarray(a, &vecsize, &x->vec)) {
        error("%s: bad template for tabsend~", x->arrayname->name); return;
    } else {
        int n = sp[0]->n;
        int ticksper = (int)(sp[0]->sr/n);
        if (ticksper < 1) ticksper = 1;
        x->graphperiod = ticksper;
        if (x->graphcount > ticksper) x->graphcount = ticksper;
        if (n < vecsize) vecsize = n;
        garray_usedindsp(a);
		if(SIMD_CHECK1(vecsize,sp[0]->v))
			dsp_add(tabsend_perf_simd, 3, x, sp[0]->v, vecsize);
		else	dsp_add(tabsend_perform,   3, x, sp[0]->v, vecsize);
    }
}

static void tabsend_setup() {
    tabsend_class = class_new2("tabsend~",tabsend_new,0,sizeof(t_tabsend),0,"S");
    CLASS_MAINSIGNALIN(tabsend_class, t_tabsend, a);
    class_addmethod2(tabsend_class, tabsend_dsp, "dsp","");
}

/* ------------------------ tabreceive~ ------------------------- */
static t_class *tabreceive_class;
struct t_tabreceive : t_object {
    float *vec;
    int vecsize;
    t_symbol *arrayname;
};
static t_int *tabreceive_perform(t_int *w) {
    t_tabreceive *x = (t_tabreceive *)w[1];
    t_float *out = (t_float *)w[2];
    int n = w[3];
    t_float *from = x->vec;
    if (from) {
        int vecsize = x->vecsize; while (vecsize--) *out++ = *from++;
        vecsize = n - x->vecsize; while (vecsize--) *out++ = 0;
    } else while (n--) *out++ = 0;
    return w+4;
}
static t_int *tabreceive_perf8(t_int *w) {
    t_tabreceive *x = (t_tabreceive *)w[1];
    t_float *from = x->vec;
    if (from) copyvec_8((t_float *)w[2],from,w[3]);
    else      zerovec_8((t_float *)w[2],     w[3]);
    return w+4;
}
static t_int *tabreceive_perfsimd(t_int *w) {
    t_tabreceive *x = (t_tabreceive *)w[1];
    t_float *from = x->vec;
    if(from) copyvec_simd((t_float *)w[2],from,w[3]);
    else     zerovec_simd((t_float *)w[2],     w[3]);
    return w+4;
}
static void tabreceive_dsp(t_tabreceive *x, t_signal **sp) {
    t_garray *a;
    if (!(a = (t_garray *)pd_findbyclass(x->arrayname, garray_class))) {
        if (*x->arrayname->name) error("tabsend~: %s: no such array", x->arrayname->name);
    } else if (!garray_getfloatarray(a, &x->vecsize, &x->vec)) {
        error("%s: bad template for tabreceive~", x->arrayname->name);
    } else {
	if (x->vecsize > sp[0]->n) x->vecsize = sp[0]->n;
        garray_usedindsp(a);
        /* the array is aligned in any case */
        if(sp[0]->n&7) dsp_add(tabreceive_perform, 3, x, sp[0]->v, sp[0]->n);
        else if(SIMD_CHECK1(sp[0]->n,sp[0]->v))
    	     dsp_add(tabreceive_perfsimd, 3, x, sp[0]->v, sp[0]->n);
        else dsp_add(tabreceive_perf8,    3, x, sp[0]->v, sp[0]->n);
    }
}
static void *tabreceive_new(t_symbol *s) {
    t_tabreceive *x = (t_tabreceive *)pd_new(tabreceive_class);
    x->arrayname = s;
    outlet_new(x, &s_signal);
    return x;
}
static void tabreceive_setup() {
    tabreceive_class = class_new2("tabreceive~",tabreceive_new,0,sizeof(t_tabreceive),0,"S");
    class_addmethod2(tabreceive_class, tabreceive_dsp, "dsp","");
}

/* ---------- tabread: control, non-interpolating ------------------------ */
static t_class *tabread_class;
struct t_tabread : t_object {
    t_symbol *arrayname;
};
static void tabread_float(t_tabread *x, t_float f) {
    int npoints;
    t_float *vec;
    t_garray *a = (t_garray *)pd_findbyclass(x->arrayname, garray_class);
    if (!a) {error("%s: no such array", x->arrayname->name); return;}
    if (!garray_getfloatarray(a, &npoints, &vec)) {error("%s: bad template for tabread", x->arrayname->name); return;}
    int n = clip(int(f),0,npoints-1);
    outlet_float(x->outlet, (npoints ? vec[n] : 0));
}
static void tabread_set(t_tabread *x, t_symbol *s) {
    x->arrayname = s;
}
static void *tabread_new(t_symbol *s) {
    t_tabread *x = (t_tabread *)pd_new(tabread_class);
    x->arrayname = s;
    outlet_new(x, &s_float);
    return x;
}
static void tabread_setup() {
    tabread_class = class_new2("tabread",tabread_new,0,sizeof(t_tabread),0,"S");
    class_addfloat(tabread_class, tabread_float);
    class_addmethod2(tabread_class, tabread_set, "set","s");
}

/* ---------- tabread4: control, 4-point interpolation --------------- */
static t_class *tabread4_class;
struct t_tabread4 : t_object {
    t_symbol *arrayname;
};
static void tabread4_float(t_tabread4 *x, t_float f) {
    t_garray *ar = (t_garray *)pd_findbyclass(x->arrayname, garray_class);
    int npoints;
    t_float *vec;
    if (!ar) {error("%s: no such array", x->arrayname->name); return;}
    if (!garray_getfloatarray(ar, &npoints, &vec)) {error("%s: bad template for tabread4", x->arrayname->name); return;}
    if (npoints < 4)    {outlet_float(x->outlet, 0); return;}
    if (f <= 1)         {outlet_float(x->outlet, vec[1]); return;}
    if (f >= npoints-2) {outlet_float(x->outlet, vec[npoints-2]); return;}
    int n = min(int(f),npoints-3);
    float *fp = vec + n;
    float frac = f - n;
    float a=fp[-1], b=fp[0], c=fp[1], d=fp[2];
    float cminusb = c-b;
    outlet_float(x->outlet, b + frac * (cminusb - 0.1666667f * (1.-frac) * ((d - a - 3.0f * cminusb) * frac + (d + 2.0f*a - 3.0f*b))));
}
static void tabread4_set(t_tabread4 *x, t_symbol *s) {x->arrayname = s;}
static void *tabread4_new(t_symbol *s) {
    t_tabread4 *x = (t_tabread4 *)pd_new(tabread4_class);
    x->arrayname = s;
    outlet_new(x, &s_float);
    return x;
}
static void tabread4_setup() {
    tabread4_class = class_new2("tabread4",tabread4_new,0,sizeof(t_tabread4),0,"S");
    class_addfloat(tabread4_class, tabread4_float);
    class_addmethod2(tabread4_class, tabread4_set, "set","s");
}

/* ------------------ tabwrite: control ------------------------ */
static t_class *tabwrite_class;
struct t_tabwrite : t_object {
    t_symbol *arrayname;
    float ft1;
};
static void tabwrite_float(t_tabwrite *x, t_float f) {
    int vecsize;
    t_garray *a = (t_garray *)pd_findbyclass(x->arrayname, garray_class);
    t_float *vec;
    if (!a) {error("%s: no such array", x->arrayname->name); return;}
    if (!garray_getfloatarray(a, &vecsize, &vec)) {error("%s: bad template for tabwrite", x->arrayname->name); return;}
    int n = (int)x->ft1;
    if (n < 0) n = 0; else if (n > vecsize-1) n = vecsize-1;
    vec[n] = f;
    garray_redraw(a);
}
static void tabwrite_set(t_tabwrite *x, t_symbol *s) {x->arrayname = s;}
static void *tabwrite_new(t_symbol *s) {
    t_tabwrite *x = (t_tabwrite *)pd_new(tabwrite_class);
    x->ft1 = 0;
    x->arrayname = s;
    floatinlet_new(x, &x->ft1);
    return x;
}
void tabwrite_setup() {
    tabwrite_class = class_new2("tabwrite",tabwrite_new,0,sizeof(t_tabwrite),0,"S");
    class_addfloat(tabwrite_class, tabwrite_float);
    class_addmethod2(tabwrite_class, tabwrite_set, "set","s");
}

/* -------------------------- sig~ ------------------------------ */
static t_class *sig_tilde_class;
struct t_sig : t_object {
    float a;
};
t_int *sig_tilde_perform(t_int *w) {
    t_float f = *(t_float *)w[1];
    t_float *out = (t_float *)w[2];
    int n = (int)w[3];
    while (n--) *out++ = f;
    return w+4;
}
t_int *sig_tilde_perf8(t_int *w) {
    t_float f = *(t_float *)w[1];
    t_float *out = (t_float *)w[2];
    int n = (int)w[3];
    for (; n; n -= 8, out += 8) {
        out[0] = f; out[1] = f;
        out[2] = f; out[3] = f;
        out[4] = f; out[5] = f;
        out[6] = f; out[7] = f;
    }
    return w+4;
}
void dsp_add_scalarcopy(t_sample *in, t_sample *out, int n) {
    if (n&7)
         dsp_add(sig_tilde_perform,    3, in, out, n);
    else if(SIMD_CHECK1(n,out))
	 dsp_add(sig_tilde_perf_simd, 3, in, out, n);
    else dsp_add(sig_tilde_perf8,     3, in, out, n);
}
static void sig_tilde_float(t_sig *x, t_float f) {
    x->a = f;
}
static void sig_tilde_dsp(t_sig *x, t_signal **sp) {
/*   dsp_add(sig_tilde_perform, 3, &x->a, sp[0]->v, sp[0]->n); */
	/* T.Grill - use chance of unrolling */
	dsp_add_scalarcopy(&x->a, sp[0]->v, sp[0]->n);
}
static void *sig_tilde_new(t_floatarg f) {
    t_sig *x = (t_sig *)pd_new(sig_tilde_class);
    x->a = f;
    outlet_new(x, &s_signal);
    return x;
}
static void sig_tilde_setup() {
    sig_tilde_class = class_new2("sig~",sig_tilde_new,0,sizeof(t_sig),0,"F");
    class_addfloat(sig_tilde_class, sig_tilde_float);
    class_addmethod2(sig_tilde_class, sig_tilde_dsp,"dsp","");
}

/* -------------------------- line~ ------------------------------ */
static t_class *line_tilde_class;
struct t_line : t_object {
    float target;
    float value;
    float biginc;
    float inc;
    float oneovern;
    float dspticktomsec;
    float inletvalue;
    float inletwas;
    int ticksleft;
    int retarget;
    float* slopes; /* tb: for simd-optimized line */
    float slopestep; /* tb: 4*x->inc */
};
static t_int *line_tilde_perform(t_int *w) {
    t_line *x = (t_line *)w[1];
    t_float *out = (t_float *)w[2];
    int n = (int)w[3];
    float f = x->value;
    if (PD_BIGORSMALL(f)) x->value = f = 0;
    if (x->retarget) {
        int nticks = (int)(x->inletwas * x->dspticktomsec);
        if (!nticks) nticks = 1;
        x->ticksleft = nticks;
        x->biginc = (x->target - x->value)/(float)nticks;
        x->inc = x->oneovern * x->biginc;
        x->retarget = 0;
    }
    if (x->ticksleft) {
        float f = x->value;
		float slope = x->inc;         /* tb: make sure, x->inc is loaded to a register */
        while (n--) *out++ = f, f += slope;
        x->value += x->biginc;
        x->ticksleft--;
    } else {
        float g = x->value = x->target;
        while (n--)
            *out++ = g;
    }
    return w+4;
}
/* tb: vectorized / simd version { */
static void line_tilde_slope(t_float* out, t_int n, t_float* value, t_float* slopes, t_float* slopestep) {
	t_float slope = slopes[1];
	t_float f = *value;
	n>>=3;
	while (n--) {
		*out++ = f;f += slope;
		*out++ = f;f += slope;
		*out++ = f;f += slope;
		*out++ = f;f += slope;
		*out++ = f;f += slope;
		*out++ = f;f += slope;
		*out++ = f;f += slope;
		*out++ = f;f += slope;
	}
}
static t_int *line_tilde_perf8(t_int *w) {
    t_line *x = (t_line *)w[1];
    t_float *out = (t_float *)w[2];
    int n = (int)w[3];
    float f = x->value;
    if (PD_BIGORSMALL(f)) x->value = f = 0;
    if (x->retarget) {
        int nticks = (int)(x->inletwas * x->dspticktomsec);
        if (!nticks) nticks = 1;
        x->ticksleft = nticks;
        x->biginc = (x->target - x->value)/(float)nticks;
        x->inc = x->oneovern * x->biginc;
        x->retarget = 0;
	/* tb: rethink!!! this is ugly */
	for (int i = 0; i != 4; ++i) x->slopes[i] = i*x->inc;
	x->slopestep = 4 * x->inc;
    }
    if (x->ticksleft) {
	line_tilde_slope(out, n, &x->value, x->slopes, &x->slopestep);
        x->value += x->biginc;
        x->ticksleft--;
    } else {
        float f = x->value = x->target;
	setvec_8(out,f,n);
    }
    return w+4;
}
static t_int *line_tilde_perfsimd(t_int *w) {
    t_line *x = (t_line *)w[1];
    t_float *out = (t_float *)w[2];
    int n = (int)w[3];
    float f = x->value;
    if (PD_BIGORSMALL(f)) x->value = f = 0;
    if (x->retarget) {
        int nticks = (int)(x->inletwas * x->dspticktomsec);
        if (!nticks) nticks = 1;
        x->ticksleft = nticks;
        x->biginc = (x->target - x->value)/(float)nticks;
        x->inc = x->oneovern * x->biginc;
        x->retarget = 0;
	for (int i = 0; i != 4; ++i) x->slopes[i] = i*x->inc;
	x->slopestep = 4 * x->inc;
    }
    if (x->ticksleft) {
	line_tilde_slope_simd(out, n, &x->value, x->slopes, &x->slopestep);
        x->value += x->biginc;
        x->ticksleft--;
    } else {
        float f = x->value = x->target;
	setvec_simd(out,f,n);
    }
    return w+4;
}
/* tb } */
static void line_tilde_float(t_line *x, t_float f) {
    if (x->inletvalue <= 0) {
        x->target = x->value = f;
        x->ticksleft = x->retarget = 0;
    } else {
        x->target = f;
        x->retarget = 1;
        x->inletwas = x->inletvalue;
        x->inletvalue = 0;
    }
}
static void line_tilde_stop(t_line *x) {
    x->target = x->value;
    x->ticksleft = x->retarget = 0;
}
static void line_tilde_dsp(t_line *x, t_signal **sp) {
    if(sp[0]->n&7)
	 dsp_add(line_tilde_perform,  3, x, sp[0]->v, sp[0]->n);
    else if (SIMD_CHECK1(sp[0]->n,sp[0]->v))
	 dsp_add(line_tilde_perfsimd, 3, x, sp[0]->v, sp[0]->n);
    else dsp_add(line_tilde_perf8,    3, x, sp[0]->v, sp[0]->n);
    x->oneovern = 1./sp[0]->n;
    x->dspticktomsec = sp[0]->sr / (1000 * sp[0]->n);
}
/* tb: modified for simd-optimized line~ */
static void *line_tilde_new() {
    t_line *x = (t_line *)pd_new(line_tilde_class);
    outlet_new(x, &s_signal);
    floatinlet_new(x, &x->inletvalue);
    x->ticksleft = x->retarget = 0;
    x->value = x->target = x->inletvalue = x->inletwas = 0;
	x->slopes = (t_float *)getalignedbytes(4*sizeof(t_float));
    return x;
}

static void line_tilde_free(t_line * x) {
	freealignedbytes(x->slopes, 4*sizeof(t_float));
}
static void line_tilde_setup() {
    line_tilde_class = class_new2("line~",line_tilde_new,line_tilde_free,sizeof(t_line),0,"");
    class_addfloat(line_tilde_class, line_tilde_float);
    class_addmethod2(line_tilde_class, line_tilde_dsp, "dsp","");
    class_addmethod2(line_tilde_class, line_tilde_stop,"stop","");
}

/* -------------------------- vline~ ------------------------------ */
static t_class *vline_tilde_class;
struct t_vseg {
    double s_targettime;
    double s_starttime;
    float s_target;
    t_vseg *s_next;
};
struct t_vline : t_object {
    double value;
    double inc;
    double referencetime;
    double samppermsec;
    double msecpersamp;
    double targettime;
    float target;
    float inlet1;
    float inlet2;
    t_vseg *list;
};
static t_int *vline_tilde_perform(t_int *w) {
    t_vline *x = (t_vline *)w[1];
    t_float *out = (t_float *)w[2];
    int n = (int)w[3], i;
    double f = x->value;
    double inc = x->inc;
    double msecpersamp = x->msecpersamp;
    double timenow = clock_gettimesince(x->referencetime) - n * msecpersamp;
    t_vseg *s = x->list;
    for (i = 0; i < n; i++) {
        double timenext = timenow + msecpersamp;
    checknext:
        if (s) {
            /* has starttime elapsed?  If so update value and increment */
            if (s->s_starttime < timenext) {
                if (x->targettime <= timenext)
                    f = x->target, inc = 0;
                    /* if zero-length segment bash output value */
                if (s->s_targettime <= s->s_starttime) {
                    f = s->s_target;
                    inc = 0;
                } else {
                    double incpermsec = (s->s_target - f)/
                        (s->s_targettime - s->s_starttime);
                    f = f + incpermsec * (timenext - s->s_starttime);
                    inc = incpermsec * msecpersamp;
                }
                x->inc = inc;
                x->target = s->s_target;
                x->targettime = s->s_targettime;
                x->list = s->s_next;
                free(s);
                s = x->list;
                goto checknext;
            }
        }
        if (x->targettime <= timenext)
            f = x->target, inc = x->inc = 0, x->targettime = 1e20;
        *out++ = f;
        f = f + inc;
        timenow = timenext;
    }
    x->value = f;
    return w+4;
}
static void vline_tilde_stop(t_vline *x) {
    t_vseg *s1, *s2;
    for (s1 = x->list; s1; s1 = s2)
        s2 = s1->s_next, free(s1);
    x->list = 0;
    x->inc = 0;
    x->inlet1 = x->inlet2 = 0;
    x->target = x->value;
    x->targettime = 1e20;
}
static void vline_tilde_float(t_vline *x, t_float f) {
    double timenow = clock_gettimesince(x->referencetime);
    float inlet1 = (x->inlet1 < 0 ? 0 : x->inlet1);
    float inlet2 = x->inlet2;
    double starttime = timenow + inlet2;
    t_vseg *s1, *s2, *deletefrom = 0, *snew;
    if (PD_BIGORSMALL(f)) f = 0;
    /* negative delay input means stop and jump immediately to new value */
    if (inlet2 < 0) {
        x->value = f;
        vline_tilde_stop(x);
        return;
    }
    snew = (t_vseg *)t_getbytes(sizeof(*snew));
        /* check if we supplant the first item in the list.  We supplant
        an item by having an earlier starttime, or an equal starttime unless
        the equal one was instantaneous and the new one isn't (in which case
        we'll do a jump-and-slide starting at that time.) */
    if (!x->list || x->list->s_starttime > starttime ||
        (x->list->s_starttime == starttime &&
            (x->list->s_targettime > x->list->s_starttime || inlet1 <= 0))) {
        deletefrom = x->list;
        x->list = snew;
    } else {
        for (s1 = x->list; (s2 = s1->s_next); s1 = s2) {
            if (s2->s_starttime > starttime ||
                (s2->s_starttime == starttime &&
                    (s2->s_targettime > s2->s_starttime || inlet1 <= 0))) {
                deletefrom = s2;
                s1->s_next = snew;
                goto didit;
            }
        }
        s1->s_next = snew;
        deletefrom = 0;
    didit: ;
    }
    while (deletefrom) {
        s1 = deletefrom->s_next;
        free(deletefrom);
        deletefrom = s1;
    }
    snew->s_next = 0;
    snew->s_target = f;
    snew->s_starttime = starttime;
    snew->s_targettime = starttime + inlet1;
    x->inlet1 = x->inlet2 = 0;
}
static void vline_tilde_dsp(t_vline *x, t_signal **sp) {
    dsp_add(vline_tilde_perform, 3, x, sp[0]->v, sp[0]->n);
    x->samppermsec = ((double)(sp[0]->sr)) / 1000;
    x->msecpersamp = ((double)1000) / sp[0]->sr;
}
static void *vline_tilde_new() {
    t_vline *x = (t_vline *)pd_new(vline_tilde_class);
    outlet_new(x, &s_signal);
    floatinlet_new(x, &x->inlet1);
    floatinlet_new(x, &x->inlet2);
    x->inlet1 = x->inlet2 = 0;
    x->value = x->inc = 0;
    x->referencetime = clock_getlogicaltime();
    x->list = 0;
    x->samppermsec = 0;
    x->targettime = 1e20;
    return x;
}
static void vline_tilde_setup() {
    vline_tilde_class = class_new2("vline~",vline_tilde_new,vline_tilde_stop,sizeof(t_vline),0,"");
    class_addfloat(vline_tilde_class, vline_tilde_float);
    class_addmethod2(vline_tilde_class, vline_tilde_dsp, "dsp","");
    class_addmethod2(vline_tilde_class, vline_tilde_stop,"stop","");
}

/* -------------------------- snapshot~ ------------------------------ */
static t_class *snapshot_tilde_class;
struct t_snapshot : t_object {
    t_sample value;
    float a;
};
static void *snapshot_tilde_new() {
    t_snapshot *x = (t_snapshot *)pd_new(snapshot_tilde_class);
    x->value = 0;
    outlet_new(x, &s_float);
    x->a = 0;
    return x;
}
static t_int *snapshot_tilde_perform(t_int *w) {
    t_float *in = (t_float *)w[1];
    t_float *out = (t_float *)w[2];
    *out = *in;
    return w+3;
}
static void snapshot_tilde_dsp(t_snapshot *x, t_signal **sp) {
    dsp_add(snapshot_tilde_perform, 2, sp[0]->v + (sp[0]->n-1), &x->value);
}
static void snapshot_tilde_bang(t_snapshot *x) {
    outlet_float(x->outlet, x->value);
}
static void snapshot_tilde_set(t_snapshot *x, t_floatarg f) {
    x->value = f;
}
static void snapshot_tilde_setup() {
    snapshot_tilde_class = class_new2("snapshot~",snapshot_tilde_new,0,sizeof(t_snapshot),0,"");
    CLASS_MAINSIGNALIN(snapshot_tilde_class, t_snapshot, a);
    class_addmethod2(snapshot_tilde_class, snapshot_tilde_dsp, "dsp","");
    class_addmethod2(snapshot_tilde_class, snapshot_tilde_set, "set","s");
    class_addbang(snapshot_tilde_class, snapshot_tilde_bang);
}

/* -------------------------- vsnapshot~ ------------------------------ */
static t_class *vsnapshot_tilde_class;
struct t_vsnapshot : t_object {
    int n;
    int gotone;
    t_sample *vec;
    float a;
    float sampspermsec;
    double time;
};
static void *vsnapshot_tilde_new() {
    t_vsnapshot *x = (t_vsnapshot *)pd_new(vsnapshot_tilde_class);
    outlet_new(x, &s_float);
    x->a = 0;
    x->n = 0;
    x->vec = 0;
    x->gotone = 0;
    return x;
}
static t_int *vsnapshot_tilde_perform(t_int *w) {
    t_float *in = (t_float *)w[1];
    t_vsnapshot *x = (t_vsnapshot *)w[2];
    t_float *out = x->vec;
    int n = x->n, i;
    for (i = 0; i < n; i++) out[i] = in[i];
    x->time = clock_getlogicaltime();
    x->gotone = 1;
    return w+3;
}
static void vsnapshot_tilde_dsp(t_vsnapshot *x, t_signal **sp) {
    int n = sp[0]->n;
    if (n != x->n) {
        if (x->vec) free(x->vec);
        x->vec = (t_sample *)getbytes(n * sizeof(t_sample));
        x->gotone = 0;
        x->n = n;
    }
    x->sampspermsec = sp[0]->sr / 1000;
    dsp_add(vsnapshot_tilde_perform, 2, sp[0]->v, x);
}
static void vsnapshot_tilde_bang(t_vsnapshot *x) {
    float val;
    if (x->gotone) {
        int indx = clip((int)(clock_gettimesince(x->time) * x->sampspermsec),0,x->n-1);
        val = x->vec[indx];
    } else val = 0;
    outlet_float(x->outlet, val);
}
static void vsnapshot_tilde_ff(t_vsnapshot *x) {
    if (x->vec) free(x->vec);
}
static void vsnapshot_tilde_setup() {
    vsnapshot_tilde_class =
	class_new2("vsnapshot~",vsnapshot_tilde_new, vsnapshot_tilde_ff,sizeof(t_vsnapshot), 0,"");
    CLASS_MAINSIGNALIN(vsnapshot_tilde_class, t_vsnapshot, a);
    class_addmethod2(vsnapshot_tilde_class, vsnapshot_tilde_dsp, "dsp","");
    class_addbang(vsnapshot_tilde_class, vsnapshot_tilde_bang);
}

/* ---------------- env~ - simple envelope follower. ----------------- */
#define MAXOVERLAP 10
#define MAXVSTAKEN 64
struct t_sigenv : t_object {
    t_clock *clock;                  /* a "clock" object */
    float *buf;                   /* a Hanning window */
    int phase;                    /* number of points since last output */
    int period;                   /* requested period of output */
    int realperiod;               /* period rounded up to vecsize multiple */
    int npoints;                  /* analysis window size in samples */
    float result;                 /* result to output */
    float sumbuf[MAXOVERLAP];     /* summing buffer */
    float a;
};
t_class *env_tilde_class;
static void env_tilde_tick(t_sigenv *x);
static void *env_tilde_new(t_floatarg fnpoints, t_floatarg fperiod) {
    int npoints = (int)fnpoints;
    int period = (int)fperiod;
    float *buf;
    if (npoints < 1) npoints = 1024;
    if (period < 1) period = npoints/2;
    if (period < npoints / MAXOVERLAP + 1)
        period = npoints / MAXOVERLAP + 1;
    if (!(buf = (float *)getalignedbytes(sizeof(float) * (npoints + MAXVSTAKEN)))) {
        error("env: couldn't allocate buffer");
        return 0;
    }
    t_sigenv *x = (t_sigenv *)pd_new(env_tilde_class);
    x->buf = buf;
    x->npoints = npoints;
    x->phase = 0;
    x->period = period;
    int i;
    for (i = 0; i < MAXOVERLAP; i++) x->sumbuf[i] = 0;
    for (i = 0; i < npoints; i++) buf[i] = (1. - cos((2 * 3.14159 * i) / npoints))/npoints;
    for (; i < npoints+MAXVSTAKEN; i++) buf[i] = 0;
    x->clock = clock_new(x, env_tilde_tick);
    x->outlet = outlet_new(x, &s_float);
    x->a = 0;
    return x;
}
static t_int *env_tilde_perform(t_int *w) {
    t_sigenv *x = (t_sigenv *)w[1];
    t_float *in = (t_float *)w[2];
    int n = (int)w[3];
    float *sump = x->sumbuf;
    in += n;
    for (int count = x->phase; count < x->npoints; count += x->realperiod, sump++) {
        float *hp = x->buf + count;
        float *fp = in;
        float sum = *sump;
        for (int i = 0; i < n; i++) {
            fp--;
            sum += *hp++ * (*fp * *fp);
        }
        *sump = sum;
    }
    sump[0] = 0;
    x->phase -= n;
    if (x->phase < 0) {
        x->result = x->sumbuf[0];
	sump = x->sumbuf;
        for (int count = x->realperiod; count < x->npoints; count += x->realperiod, sump++) sump[0] = sump[1];
        sump[0] = 0;
        x->phase = x->realperiod - n;
        clock_delay(x->clock, 0L);
    }
    return w+4;
}
/* tb: loop unrolling and simd */
static float env_tilde_accum_8(t_float* in, t_float* hp, t_int n) {
	float ret = 0;
	n>>=3;
	for (int i = 0; i !=n; ++i) {
		in--; ret += *hp++ * (*in * *in);
		in--; ret += *hp++ * (*in * *in);
		in--; ret += *hp++ * (*in * *in);
		in--; ret += *hp++ * (*in * *in);
		in--; ret += *hp++ * (*in * *in);
		in--; ret += *hp++ * (*in * *in);
		in--; ret += *hp++ * (*in * *in);
		in--; ret += *hp++ * (*in * *in);
	}
	return ret;
}
static t_int *env_tilde_perf8(t_int *w) {
    t_sigenv *x = (t_sigenv *)w[1];
    t_float *in = (t_float *)w[2];
    int n = (int)w[3];
    float *sump = x->sumbuf;
    in += n;
    for (int count = x->phase; count < x->npoints; count += x->realperiod, sump++) {
	*sump += env_tilde_accum_8(in, x->buf + count, n);
    }
    sump[0] = 0;
    x->phase -= n;
    if (x->phase < 0) {
        x->result = x->sumbuf[0];
	float *sump = x->sumbuf;
        for (int count = x->realperiod; count < x->npoints; count += x->realperiod, sump++)
                sump[0] = sump[1];
        sump[0] = 0;
        x->phase = x->realperiod - n;
        clock_delay(x->clock, 0L);
    }
    return w+4;
}
static t_int *env_tilde_perf_simd(t_int *w) {
    t_sigenv *x = (t_sigenv *)w[1];
    t_float *in = (t_float *)w[2];
    int n = (int)w[3];
    float *sump = x->sumbuf;
    in += n;
    for (int count = x->phase; count < x->npoints; count += x->realperiod, sump++) {
	*sump += env_tilde_accum_simd(in, x->buf + count, n);
    }
    sump[0] = 0;
    x->phase -= n;
    if (x->phase < 0) {
        x->result = x->sumbuf[0];
	float *sump = x->sumbuf;
        for (int count = x->realperiod; count < x->npoints; count += x->realperiod, sump++) sump[0] = sump[1];
        sump[0] = 0;
        x->phase = x->realperiod - n;
        clock_delay(x->clock, 0L);
    }
    return w+4;
}
static void env_tilde_dsp(t_sigenv *x, t_signal **sp) {
    int mod = x->period % sp[0]->n;
    if (mod) x->realperiod = x->period + sp[0]->n - mod; else x->realperiod = x->period;
    if (sp[0]->n & 7)
	 dsp_add(env_tilde_perform,   3, x, sp[0]->v, sp[0]->n);
    else if (SIMD_CHECK1(sp[0]->n, sp[0]->v))
	 dsp_add(env_tilde_perf_simd, 3, x, sp[0]->v, sp[0]->n);
    else dsp_add(env_tilde_perf8,     3, x, sp[0]->v, sp[0]->n);
    if (sp[0]->n > MAXVSTAKEN) bug("env_tilde_dsp");
}
static void env_tilde_tick(t_sigenv *x) {
    outlet_float(x->outlet, powtodb(x->result));
}
static void env_tilde_ff(t_sigenv *x) {
    clock_free(x->clock);
    freealignedbytes(x->buf, (x->npoints + MAXVSTAKEN) * sizeof(float));
}
void env_tilde_setup() {
    env_tilde_class = class_new2("env~",env_tilde_new,env_tilde_ff,sizeof(t_sigenv),0,"FF");
    CLASS_MAINSIGNALIN(env_tilde_class, t_sigenv, a);
    class_addmethod2(env_tilde_class, env_tilde_dsp, "dsp","");
}

/* --------------------- threshold~ ----------------------------- */
static t_class *threshold_tilde_class;
struct t_threshold_tilde : t_object {
    t_clock *clock;           /* wakeup for message output */
    float a;                  /* scalar inlet */
    int state;                /* 1 = high, 0 = low */
    float hithresh;           /* value of high threshold */
    float lothresh;           /* value of low threshold */
    float deadwait;           /* msec remaining in dead period */
    float msecpertick;        /* msec per DSP tick */
    float hideadtime;         /* hi dead time in msec */
    float lodeadtime;         /* lo dead time in msec */
};
static void threshold_tilde_tick(t_threshold_tilde *x);
/* "set" message to specify thresholds and dead times */
static void threshold_tilde_set(t_threshold_tilde *x,
    t_floatarg hithresh, t_floatarg hideadtime,
    t_floatarg lothresh, t_floatarg lodeadtime) {
    if (lothresh > hithresh) lothresh = hithresh;
    x->hithresh = hithresh; x->hideadtime = hideadtime;
    x->lothresh = lothresh; x->lodeadtime = lodeadtime;
}
static t_threshold_tilde *threshold_tilde_new(t_floatarg hithresh,
    t_floatarg hideadtime, t_floatarg lothresh, t_floatarg lodeadtime) {
    t_threshold_tilde *x = (t_threshold_tilde *)pd_new(threshold_tilde_class);
    x->state = 0;             /* low state */
    x->deadwait = 0;          /* no dead time */
    x->clock = clock_new(x, threshold_tilde_tick);
    outlet_new(x, &s_bang);
    outlet_new(x, &s_bang);
    inlet_new(x, x, &s_float, gensym("ft1"));
    x->msecpertick = 0.;
    x->a = 0;
    threshold_tilde_set(x, hithresh, hideadtime, lothresh, lodeadtime);
    return x;
}
/* number in inlet sets state -- note incompatible with JMAX which used
   "int" message for this, impossible here because of auto signal conversion */
static void threshold_tilde_ft1(t_threshold_tilde *x, t_floatarg f) {
    x->state = (f != 0);
    x->deadwait = 0;
}
static void threshold_tilde_tick(t_threshold_tilde *x) {
  outlet_bang(x->out(x->state?0:1));
}
static t_int *threshold_tilde_perform(t_int *w) {
    float *in1 = (float *)w[1];
    t_threshold_tilde *x = (t_threshold_tilde *)w[2];
    int n = (t_int)w[3];
    if (x->deadwait > 0)
        x->deadwait -= x->msecpertick;
    else if (x->state) {
        /* we're high; look for low sample */
        for (; n--; in1++) {
            if (*in1 < x->lothresh) {
                clock_delay(x->clock, 0L);
                x->state = 0;
                x->deadwait = x->lodeadtime;
                goto done;
            }
        }
    } else {
        /* we're low; look for high sample */
        for (; n--; in1++) {
            if (*in1 >= x->hithresh) {
                clock_delay(x->clock, 0L);
                x->state = 1;
                x->deadwait = x->hideadtime;
                goto done;
            }
        }
    }
done:
    return w+4;
}
void threshold_tilde_dsp(t_threshold_tilde *x, t_signal **sp) {
    x->msecpertick = 1000. * sp[0]->n / sp[0]->sr;
    dsp_add(threshold_tilde_perform, 3, sp[0]->v, x, sp[0]->n);
}
static void threshold_tilde_ff(t_threshold_tilde *x) {clock_free(x->clock);}
static void threshold_tilde_setup() {
    t_class *c = threshold_tilde_class =
	class_new2("threshold~",threshold_tilde_new,threshold_tilde_ff,sizeof(t_threshold_tilde), 0, "FFFF");
    CLASS_MAINSIGNALIN(c, t_threshold_tilde, a);
    class_addmethod2(c, threshold_tilde_set, "set","ffff");
    class_addmethod2(c, threshold_tilde_ft1, "ft1","f");
    class_addmethod2(c, threshold_tilde_dsp, "dsp","");
}

/* ----------------------------- dac~ --------------------------- */
static t_class *dac_class;
struct t_dac : t_object {
    t_int n;
    t_int *vec;
    float a;
};
static void *dac_new(t_symbol *s, int argc, t_atom *argv) {
    t_dac *x = (t_dac *)pd_new(dac_class);
    t_atom defarg[2];
    if (!argc) {
        argv = defarg;
        argc = 2;
        SETFLOAT(&defarg[0], 1);
        SETFLOAT(&defarg[1], 2);
    }
    x->n = argc;
    x->vec = (t_int *)getbytes(argc * sizeof(*x->vec));
    for (int i = 0; i < argc; i++) x->vec[i] = atom_getintarg(i, argc, argv);
    for (int i = 1; i < argc; i++) inlet_new(x, x, &s_signal, &s_signal);
    x->a = 0;
    return x;
}
static void dac_dsp(t_dac *x, t_signal **sp) {
    t_int i, *ip;
    t_signal **sp2;
    for (i = x->n, ip = x->vec, sp2 = sp; i--; ip++, sp2++) {
        int ch = *ip - 1;
        if ((*sp2)->n != sys_dacblocksize) error("dac~: bad vector size");
    	else if (ch >= 0 && ch < sys_get_outchannels())
		if(SIMD_CHECK3(sys_dacblocksize,sys_soundout + sys_dacblocksize*ch, (*sp2)->v,sys_soundout + sys_dacblocksize*ch))
		    dsp_add(plus_perf_simd, 4, sys_soundout + sys_dacblocksize*ch,
			    (*sp2)->v, sys_soundout + sys_dacblocksize*ch, sys_dacblocksize);
		else
		    dsp_add(plus_perform, 4, sys_soundout + sys_dacblocksize*ch, (*sp2)->v, sys_soundout + sys_dacblocksize*ch,
			sys_dacblocksize);
    }
}
static void dac_free(t_dac *x) {free(x->vec);}
static void dac_setup() {
    dac_class = class_new2("dac~",dac_new,dac_free,sizeof(t_dac),0,"*");
    CLASS_MAINSIGNALIN(dac_class, t_dac, a);
    class_addmethod2(dac_class, dac_dsp, "dsp","!");
    class_sethelpsymbol(dac_class, gensym("adc~_dac~"));
}

/* ----------------------------- adc~ --------------------------- */
static t_class *adc_class;
struct t_adc : t_object {
    t_int n;
    t_int *vec;
};
static void *adc_new(t_symbol *s, int argc, t_atom *argv) {
    t_adc *x = (t_adc *)pd_new(adc_class);
    t_atom defarg[2];
    if (!argc) {
        argv = defarg;
        argc = 2;
        SETFLOAT(&defarg[0], 1);
        SETFLOAT(&defarg[1], 2);
    }
    x->n = argc;
    x->vec = (t_int *)getbytes(argc * sizeof(*x->vec));
    for (int i = 0; i < argc; i++) x->vec[i] = atom_getintarg(i, argc, argv);
    for (int i = 0; i < argc; i++) outlet_new(x, &s_signal);
    return x;
}
t_int *copy_perform(t_int *w) {
    t_float *in1 = (t_float *)w[1];
    t_float *out = (t_float *)w[2];
    int n = (int)w[3];
    while (n--) *out++ = *in1++;
    return w+4;
}
t_int *copy_perf8(t_int *w) {
    t_float *in1 = (t_float *)w[1];
    t_float *out = (t_float *)w[2];
    int n = (int)w[3];
    for (; n; n -= 8, in1 += 8, out += 8) {
        out[0] = in1[0];
        out[1] = in1[1];
        out[2] = in1[2];
        out[3] = in1[3];
        out[4] = in1[4];
        out[5] = in1[5];
        out[6] = in1[6];
        out[7] = in1[7];
    }
    return w+4;
}
void dsp_add_copy(t_sample *in, t_sample *out, int n) {
    if (n&7)
         dsp_add(copy_perform,   3, in, out, n);
    else if (SIMD_CHECK2(n,in,out))
	 dsp_add(copy_perf_simd, 3, in, out, n);
    else dsp_add(copy_perf8,     3, in, out, n);
}
static void adc_dsp(t_adc *x, t_signal **sp) {
    t_int i, *ip;
    t_signal **sp2;
    for (i = x->n, ip = x->vec, sp2 = sp; i--; ip++, sp2++) {
        int ch = *ip - 1;
        if ((*sp2)->n != sys_dacblocksize)
            error("adc~: bad vector size");
        else if (ch >= 0 && ch < sys_get_inchannels())
            dsp_add_copy(sys_soundin + sys_dacblocksize*ch, (*sp2)->v, sys_dacblocksize);
        else dsp_add_zero((*sp2)->v, sys_dacblocksize);
    }
}
static void adc_free(t_adc *x) {free(x->vec);}
static void adc_setup() {
    adc_class = class_new2("adc~",adc_new,adc_free,sizeof(t_adc),0,"*");
    class_addmethod2(adc_class, adc_dsp, "dsp","!");
    class_sethelpsymbol(adc_class, gensym("adc~_dac~"));
}

/* ----------------------------- delwrite~ ----------------------------- */
static t_class *sigdelwrite_class;
struct t_delwritectl {
    int c_n;
    float *c_vec;
    int c_phase;
};
struct t_sigdelwrite : t_object {
    t_symbol *sym;
    t_delwritectl cspace;
    int sortno;   /* DSP sort number at which this was last put on chain */
    int rsortno;  /* DSP sort # for first delread or write in chain */
    int vecsize;  /* vector size for delread~ to use */
    float a;
};
#define XTRASAMPS 4
#define SAMPBLK 4
/* routine to check that all delwrites/delreads/vds have same vecsize */
static void sigdelwrite_checkvecsize(t_sigdelwrite *x, int vecsize) {
    if (x->rsortno != ugen_getsortno()) {
        x->vecsize = vecsize;
        x->rsortno = ugen_getsortno();
    }
    /*  LATER this should really check sample rate and blocking, once that is
        supported.  Probably we don't actually care about vecsize.
        For now just suppress this check. */
#if 0
    else if (vecsize != x->vecsize)
        error("delread/delwrite/vd vector size mismatch");
#endif
}
static void *sigdelwrite_new(t_symbol *s, t_floatarg msec) {
    t_sigdelwrite *x = (t_sigdelwrite *)pd_new(sigdelwrite_class);
    if (!*s->name) s = gensym("delwrite~");
    pd_bind(x, s);
    x->sym = s;
    int nsamps = (int)(msec * sys_getsr() * (float)(0.001f));
    if (nsamps < 1) nsamps = 1;
    nsamps += ((- nsamps) & (SAMPBLK - 1));
    nsamps += DEFDELVS;
#ifdef SIMD_BYTEALIGN
	nsamps += (SIMD_BYTEALIGN - nsamps) % SIMD_BYTEALIGN; /* tb: for simd */
#endif
    x->cspace.c_n = nsamps;
    x->cspace.c_vec = (float *)getalignedbytes((nsamps + XTRASAMPS) * sizeof(float));
	x->cspace.c_phase = XTRASAMPS;
    x->sortno = 0;
    x->vecsize = 0;
    x->a = 0;
    return x;
}
static t_int *sigdelwrite_perform(t_int *w) {
    t_float *in = (t_float *)w[1];
    t_delwritectl *c = (t_delwritectl *)w[2];
    int n = (int)w[3];
    int phase = c->c_phase, nsamps = c->c_n;
    float *vp = c->c_vec, *bp = vp + phase, *ep = vp + (c->c_n + XTRASAMPS);
    phase += n;
    while (n--) {
        float f = *in++;
        if (PD_BIGORSMALL(f))
            f = 0;
        *bp++ = f;
        if (bp == ep) {
            vp[0] = ep[-4];
            vp[1] = ep[-3];
            vp[2] = ep[-2];
            vp[3] = ep[-1];
            bp = vp + XTRASAMPS;
            phase -= nsamps;
        }
    }
    c->c_phase = phase;
    return w+4;
}
static t_int *sigdelwrite_perf8(t_int *w) {
    t_float *in = (t_float *)w[1];
    t_delwritectl *c = (t_delwritectl *)w[2];
    int n = (int)w[3];
    int phase = c->c_phase, nsamps = c->c_n;
    float *vp = c->c_vec, *bp = vp + phase, *ep = vp + (c->c_n + XTRASAMPS);
    phase += n;
	if (phase > nsamps)
		while (n--) {
			float f = *in++;
			if (PD_BIGORSMALL(f)) f = 0;
			*bp++ = f;
			if (bp == ep) {
				vp[0] = ep[-4];
				vp[1] = ep[-3];
				vp[2] = ep[-2];
				vp[3] = ep[-1];
				bp = vp + XTRASAMPS;
				phase -= nsamps;
			}
		}
	else testcopyvec_8(bp, in, n);
    c->c_phase = phase;
    return w+4;
}
static t_int *sigdelwrite_perfsimd(t_int *w) {
    t_float *in = (t_float *)w[1];
    t_delwritectl *c = (t_delwritectl *)w[2];
    int n = (int)w[3];
    int phase = c->c_phase, nsamps = c->c_n;
    float *vp = c->c_vec, *bp = vp + phase, *ep = vp + (c->c_n + XTRASAMPS);
    phase += n;
	if (phase > nsamps )
		while (n--) {
			float f = *in++;
			if (PD_BIGORSMALL(f)) f = 0;
			*bp++ = f;
			if (bp == ep) {
				vp[0] = ep[-4];
				vp[1] = ep[-3];
				vp[2] = ep[-2];
				vp[3] = ep[-1];
				bp = vp + XTRASAMPS;
				phase -= nsamps;
			}
		}
	else testcopyvec_simd(bp, in, n);
    c->c_phase = phase;
    return w+4;
}
static void sigdelwrite_dsp(t_sigdelwrite *x, t_signal **sp) {
	if (sp[0]->n & 7)
		dsp_add(sigdelwrite_perform,  3, sp[0]->v, &x->cspace, sp[0]->n);
	else if (SIMD_CHECK1(sp[0]->n, sp[0]->v))
		dsp_add(sigdelwrite_perfsimd, 3, sp[0]->v, &x->cspace, sp[0]->n);
	else	dsp_add(sigdelwrite_perf8,    3, sp[0]->v, &x->cspace, sp[0]->n);
    x->sortno = ugen_getsortno();
    sigdelwrite_checkvecsize(x, sp[0]->n);
}
static void sigdelwrite_free(t_sigdelwrite *x) {
    pd_unbind(x, x->sym);
    freealignedbytes(x->cspace.c_vec, (x->cspace.c_n + XTRASAMPS) * sizeof(float));
}
static void sigdelwrite_setup() {
    sigdelwrite_class = class_new2("delwrite~",sigdelwrite_new,sigdelwrite_free,sizeof(t_sigdelwrite),0,"SF");
    CLASS_MAINSIGNALIN(sigdelwrite_class, t_sigdelwrite, a);
    class_addmethod2(sigdelwrite_class, sigdelwrite_dsp,"dsp","");
}

/* ----------------------------- delread~ ----------------------------- */
static t_class *sigdelread_class;
struct t_sigdelread : t_object {
    t_symbol *sym;
    t_float deltime;  /* delay in msec */
    int delsamps;     /* delay in samples */
    t_float sr;       /* samples per msec */
    t_float n;        /* vector size */
    int zerodel;      /* 0 or vecsize depending on read/write order */
	void (*copy_fp)(t_float*, const t_float*, int); /* tb: copy function */
};
static void sigdelread_float(t_sigdelread *x, t_float f);
static void *sigdelread_new(t_symbol *s, t_floatarg f) {
    t_sigdelread *x = (t_sigdelread *)pd_new(sigdelread_class);
    x->sym = s;
    x->sr = 1;
    x->n = 1;
    x->zerodel = 0;
    sigdelread_float(x, f);
    outlet_new(x, &s_signal);
    return x;
}
static void sigdelread_float(t_sigdelread *x, t_float f) {
    t_sigdelwrite *delwriter = (t_sigdelwrite *)pd_findbyclass(x->sym, sigdelwrite_class);
    x->deltime = f;
    if (delwriter) {
        x->delsamps = (int)(0.5 + x->sr * x->deltime + x->n - x->zerodel);
        if (x->delsamps < x->n) x->delsamps = (int)x->n;
        else if (x->delsamps > delwriter->cspace.c_n - DEFDELVS)
            x->delsamps = (int)(delwriter->cspace.c_n - DEFDELVS);
    }
    if (SIMD_CHKCNT(x->delsamps)) x->copy_fp = copyvec_simd;
    else x->copy_fp = copyvec_simd_unalignedsrc;
}
static t_int *sigdelread_perform(t_int *w) {
    t_float *out = (t_float *)w[1];
    t_delwritectl *c = (t_delwritectl *)w[2];
    int delsamps = *(int *)w[3];
    int n = (int)w[4];
    int phase = c->c_phase - delsamps, nsamps = c->c_n;
    float *vp = c->c_vec, *bp, *ep = vp + (c->c_n + XTRASAMPS);
    if (phase < 0) phase += nsamps;
    bp = vp + phase;
    while (n--) {
        *out++ = *bp++;
        if (bp == ep) bp -= nsamps;
    }
    return w+5;
}
static t_int *sigdelread_perf8(t_int *w) {
    t_float *out = (t_float *)w[1];
    t_delwritectl *c = (t_delwritectl *)w[2];
    int delsamps = *(int *)w[3];
    int n = (int)w[4];
    int phase = c->c_phase - delsamps, nsamps = c->c_n;
    float *vp = c->c_vec, *bp, *ep = vp + (c->c_n + XTRASAMPS);
    if (phase < 0) phase += nsamps;
    bp = vp + phase;
	if (phase + n > nsamps)
		while (n--) {
			*out++ = *bp++;
			if (bp == ep) bp -= nsamps;
		}
	else copyvec_8(out, bp, n);
    return w+5;
}
static t_int *sigdelread_perfsimd(t_int *w) {
    t_float *out = (t_float *)w[1];
    t_delwritectl *c = (t_delwritectl *)w[2];
    int delsamps = *(int *)w[3];
    int n = (int)w[4];
    t_sigdelread *x = (t_sigdelread *)w[5];
    int phase = c->c_phase - delsamps, nsamps = c->c_n;
    float *vp = c->c_vec, *bp, *ep = vp + (c->c_n + XTRASAMPS);
    if (phase < 0) phase += nsamps;
    bp = vp + phase;
	if (phase + n > nsamps)
		while (n--) {
			*out++ = *bp++;
			if (bp == ep) bp -= nsamps;
		}
	else x->copy_fp(out, bp, n);
    return w+6;
}
static void sigdelread_dsp(t_sigdelread *x, t_signal **sp) {
    t_sigdelwrite *delwriter =
        (t_sigdelwrite *)pd_findbyclass(x->sym, sigdelwrite_class);
    x->sr = sp[0]->sr * 0.001;
    x->n = sp[0]->n;
    if (delwriter) {
        sigdelwrite_checkvecsize(delwriter, sp[0]->n);
        x->zerodel = (delwriter->sortno == ugen_getsortno() ?
            0 : delwriter->vecsize);
        sigdelread_float(x, x->deltime);
	if (sp[0]->n & 7)
		dsp_add(sigdelread_perform, 4, sp[0]->v, &delwriter->cspace, &x->delsamps, sp[0]->n);
	else if (SIMD_CHECK1(sp[0]->n, sp[0]->v))
		dsp_add(sigdelread_perfsimd, 5, sp[0]->v, &delwriter->cspace, &x->delsamps, sp[0]->n, x);
	else	dsp_add(sigdelread_perf8,    4, sp[0]->v, &delwriter->cspace, &x->delsamps, sp[0]->n);
    } else if (*x->sym->name) error("delread~: %s: no such delwrite~",x->sym->name);
}
static void sigdelread_setup() {
    sigdelread_class = class_new2("delread~",sigdelread_new,0,sizeof(t_sigdelread),0,"SF");
    class_addmethod2(sigdelread_class, sigdelread_dsp,"dsp","");
    class_addfloat(sigdelread_class, sigdelread_float);
}

/* ----------------------------- vd~ ----------------------------- */
static t_class *sigvd_class;
struct t_sigvd : t_object {
    t_symbol *sym;
    t_float sr;       /* samples per msec */
    int zerodel;      /* 0 or vecsize depending on read/write order */
    float a;
};
static void *sigvd_new(t_symbol *s) {
    t_sigvd *x = (t_sigvd *)pd_new(sigvd_class);
    if (!*s->name) s = gensym("vd~");
    x->sym = s;
    x->sr = 1;
    x->zerodel = 0;
    outlet_new(x, &s_signal);
    x->a = 0;
    return x;
}
static t_int *sigvd_perform(t_int *w) {
    t_float *in = (t_float *)w[1];
    t_float *out = (t_float *)w[2];
    t_delwritectl *ctl = (t_delwritectl *)w[3];
    t_sigvd *x = (t_sigvd *)w[4];
    int n = (int)w[5];

    int nsamps = ctl->c_n;
    float limit = nsamps - n - 1;
    float fn = n-1;
    float *vp = ctl->c_vec, *bp, *wp = vp + ctl->c_phase;
    float zerodel = x->zerodel;
    while (n--) {
        float delsamps = x->sr * *in++ - zerodel, frac;
        int idelsamps;
        float a, b, c, d, cminusb;
        if (delsamps < 1.00001f) delsamps = 1.00001f;
        if (delsamps > limit) delsamps = limit;
        delsamps += fn;
        fn = fn - 1.0f;
        idelsamps = (int)delsamps;
        frac = delsamps - (float)idelsamps;
        bp = wp - idelsamps;
        if (bp < vp + 4) bp += nsamps;
        d = bp[-3];
        c = bp[-2];
        b = bp[-1];
        a = bp[0];
        cminusb = c-b;
        *out++ = b + frac * (
            cminusb - 0.1666667f * (1.-frac) * (
                (d - a - 3.0f * cminusb) * frac + (d + 2.0f*a - 3.0f*b)
            )
        );
    }
    return w+6;
}
static void sigvd_dsp(t_sigvd *x, t_signal **sp) {
    t_sigdelwrite *delwriter = (t_sigdelwrite *)pd_findbyclass(x->sym, sigdelwrite_class);
    x->sr = sp[0]->sr * 0.001;
    if (delwriter) {
        sigdelwrite_checkvecsize(delwriter, sp[0]->n);
        x->zerodel = (delwriter->sortno == ugen_getsortno() ? 0 : delwriter->vecsize);
        dsp_add(sigvd_perform, 5, sp[0]->v, sp[1]->v, &delwriter->cspace, x, sp[0]->n);
    } else error("vd~: %s: no such delwrite~",x->sym->name);
}
static void sigvd_setup() {
    sigvd_class = class_new2("vd~",sigvd_new,0,sizeof(t_sigvd),0,"S");
    class_addmethod2(sigvd_class, sigvd_dsp, "dsp","");
    CLASS_MAINSIGNALIN(sigvd_class, t_sigvd, a);
}

#ifndef HAVE_LIBFFTW3F
/* ------------------------ fft~ and ifft~ -------------------------------- */
/* ----------------------- rfft~ -------------------------------- */
/* ----------------------- rifft~ -------------------------------- */
static t_class *sigfft_class;   struct t_sigfft   : t_object {float a;};
static t_class *sigifft_class;  struct t_sigifft  : t_object {float a;};
static t_class *sigrfft_class;  struct t_sigrfft  : t_object {float a;};
static t_class *sigrifft_class; struct t_sigrifft : t_object {float a;};

static void *sigfft_new() {
    t_sigfft *x = (t_sigfft *)pd_new(sigfft_class);
    outlet_new(x, &s_signal);
    outlet_new(x, &s_signal);
    inlet_new(x, x, &s_signal, &s_signal); x->a=0; return x;}
static void *sigifft_new() {
    t_sigifft *x = (t_sigifft *)pd_new(sigifft_class);
    outlet_new(x, &s_signal);
    outlet_new(x, &s_signal);
    inlet_new(x, x, &s_signal, &s_signal); x->a=0; return x;}
static void *sigrfft_new() {
    t_sigrfft *x = (t_sigrfft *)pd_new(sigrfft_class);
    outlet_new(x, &s_signal);
    outlet_new(x, &s_signal);
    x->a = 0; return x;}
static void *sigrifft_new() {
    t_sigrifft *x = (t_sigrifft *)pd_new(sigrifft_class);
    inlet_new(x, x, &s_signal, &s_signal);
    outlet_new(x, &s_signal);
    x->a = 0; return x;}

static t_int *sigfft_swap(t_int *w) {
    float *in1 = (t_float *)w[1];
    float *in2 = (t_float *)w[2];
    int n = w[3];
    for (;n--; in1++, in2++) {
        float f = *in1;
	*in1 = *in2;
	*in2 = f;
    }
    return w+4;
}
static t_int *  sigfft_perform(t_int *w) {float *in1=(t_float *)w[1], *in2=(t_float *)w[2]; int n=w[3];  mayer_fft(n,in1,in2); return w+4;}
static t_int * sigifft_perform(t_int *w) {float *in1=(t_float *)w[1], *in2=(t_float *)w[2]; int n=w[3]; mayer_ifft(n,in1,in2); return w+4;}
static t_int * sigrfft_perform(t_int *w) {float *in =(t_float *)w[1]; int n = w[2]; mayer_realfft(n, in);  return w+3;}
static t_int *sigrifft_perform(t_int *w) {float *in =(t_float *)w[1]; int n = w[2]; mayer_realifft(n, in); return w+3;}

static void sigfft_dspx(t_sigfft *x, t_signal **sp, t_int *(*f)(t_int *w)) {
    int n = sp[0]->n;
    float *in1 = sp[0]->v;
    float *in2 = sp[1]->v;
    float *out1 = sp[2]->v;
    float *out2 = sp[3]->v;
    if (out1 == in2 && out2 == in1)
    	dsp_add(sigfft_swap, 3, out1, out2, n);
    else if (out1 == in2) {
    	dsp_add(copy_perform, 3, in2, out2, n);
    	dsp_add(copy_perform, 3, in1, out1, n);
    } else {
    	if (out1 != in1) dsp_add(copy_perform, 3, in1, out1, n);
    	if (out2 != in2) dsp_add(copy_perform, 3, in2, out2, n);
    }
    dsp_add(f, 3, sp[2]->v, sp[3]->v, n);
}
static void  sigfft_dsp(t_sigfft *x, t_signal **sp) {sigfft_dspx(x, sp, sigfft_perform);}
static void sigifft_dsp(t_sigfft *x, t_signal **sp) {sigfft_dspx(x, sp, sigifft_perform);}

static t_int *sigrfft_flip(t_int *w) {
    float *in = (t_float *)w[1];
    float *out = (t_float *)w[2];
    int n = w[3];
    while (n--) *(--out) = *in++;
    *(--out) = 0; /* to hell with it */
    return w+4;
}
static void sigrfft_dsp(t_sigrfft *x, t_signal **sp) {
    int n = sp[0]->n, n2 = (n>>1);
    float *in1 = sp[0]->v;
    float *out1 = sp[1]->v;
    float *out2 = sp[2]->v;
    if (n < 4) {
    	error("fft: minimum 4 points");
    	return;
    }
    /* this probably never happens */
    if (in1 == out2) {
    	dsp_add(sigrfft_perform, 2, out2, n);
    	dsp_add(copy_perform, 3, out2, out1, n2);
    	dsp_add(sigrfft_flip, 3, out2 + (n2+1), out2 + n2, n2-1);
    } else {
    	if (in1 != out1) dsp_add(copy_perform, 3, in1, out1, n);
    	dsp_add(sigrfft_perform, 2, out1, n);
    	dsp_add(sigrfft_flip, 3, out1 + (n2+1), out2 + n2, n2-1);
    }
    dsp_add_zero(out1 + n2, n2);
    dsp_add_zero(out2 + n2, n2);
}

static void sigrifft_dsp(t_sigrifft *x, t_signal **sp) {
    int n = sp[0]->n, n2 = (n>>1);
    float *in1 = sp[0]->v;
    float *in2 = sp[1]->v;
    float *out1 = sp[2]->v;
    if (n < 4) {error("fft: minimum 4 points"); return;}
    if (in2 == out1) {
    	dsp_add(sigrfft_flip, 3, out1+1, out1 + n, (n2-1));
    	dsp_add(copy_perform, 3, in1, out1, n2);
    } else {
    	if (in1 != out1) dsp_add(copy_perform, 3, in1, out1, n2);
    	dsp_add(sigrfft_flip, 3, in2+1, out1 + n, n2-1);
    }
    dsp_add(sigrifft_perform, 2, out1, n);
}
static void sigfft_setup() {
    sigfft_class   = class_new2("fft~",  sigfft_new,  0,sizeof(t_sigfft),  0,"");
    sigifft_class  = class_new2("ifft~", sigifft_new, 0,sizeof(t_sigfft),  0,"");
    sigrfft_class  = class_new2("rfft~", sigrfft_new, 0,sizeof(t_sigrfft), 0,"");
    sigrifft_class = class_new2("rifft~",sigrifft_new,0,sizeof(t_sigrifft),0,"");
    CLASS_MAINSIGNALIN(sigfft_class,  t_sigfft,  a);
    CLASS_MAINSIGNALIN(sigifft_class, t_sigfft,  a);
    CLASS_MAINSIGNALIN(sigrfft_class, t_sigrfft, a);
    CLASS_MAINSIGNALIN(sigrifft_class,t_sigrifft,a);
    class_addmethod2(sigfft_class,  sigfft_dsp,  "dsp","");
    class_addmethod2(sigifft_class, sigifft_dsp, "dsp","");
    class_addmethod2(sigrfft_class, sigrfft_dsp, "dsp","");
    class_addmethod2(sigrifft_class,sigrifft_dsp,"dsp","");
    class_sethelpsymbol(sigifft_class, gensym("fft~"));
    class_sethelpsymbol(sigrfft_class, gensym("fft~"));
    class_sethelpsymbol(sigrifft_class,gensym("fft~"));
}

#else
/* Support for fftw3 by Tim Blechmann */
/* ------------------------ fft~ and ifft~ -------------------------------- */
/* ----------------------- rfft~ --------------------------------- */
/* ----------------------- rifft~ -------------------------------- */

static t_class *sigfftw_class;  struct t_sigfftw   : t_object {float a; fftwf_plan plan; fftwf_iodim dim;};
static t_class *sigifftw_class; struct t_sigifftw  : t_object {float a; fftwf_plan plan; fftwf_iodim dim;};
static t_class *sigrfftw_class; struct t_sigrfftw  : t_object {float a; fftwf_plan plan; fftwf_iodim dim;};
static t_class *sigrifftw_class;struct t_sigrifftw : t_object {float a; fftwf_plan plan; fftwf_iodim dim;};

static void *sigfftw_new() {
    t_sigfftw *x  = (t_sigfftw *)pd_new(sigfftw_class);  outlet_new(x, &s_signal); outlet_new(x, &s_signal);
    inlet_new(x, x, &s_signal, &s_signal); x->a=0; return x;}
static void *sigifftw_new() {
    t_sigifftw *x = (t_sigifftw *)pd_new(sigfftw_class); outlet_new(x, &s_signal); outlet_new(x, &s_signal);
    inlet_new(x, x, &s_signal, &s_signal); x->a=0; return x;}
static void *sigrfftw_new() {
    t_sigrfftw *x = (t_sigrfftw *)pd_new(sigrfftw_class);
    outlet_new(x, &s_signal); outlet_new(x, &s_signal); x->a=0; return x;}
static void *sigrifftw_new() {
    t_sigrifftw *x = (t_sigrifftw *)pd_new(sigrifftw_class);
    inlet_new(x, x, &s_signal, &s_signal);
    outlet_new(x, &s_signal);                           x->a=0; return x;}

static void   sigfftw_free(  t_sigfftw *x) {fftwf_destroy_plan(x->plan);}
static void  sigifftw_free( t_sigifftw *x) {fftwf_destroy_plan(x->plan);}
static void  sigrfftw_free( t_sigrfftw *x) {fftwf_destroy_plan(x->plan);}
static void sigrifftw_free(t_sigrifftw *x) {fftwf_destroy_plan(x->plan);}

/* for compatibility reasons with the mayer fft, we'll have to invert some samples. this is ugly, but someone might rely on that. */
static void sigrfftw_invert(t_sample * s, t_int n) {
    while (n!=0) {--n; s[n]=-s[n];}
}
static t_int *sigfftw_perform(t_int *w) {
    fftwf_execute(*(fftwf_plan *)w[1]);
    return w+2;
}
static t_int *sigrfftw_perform(t_int *w) {
    fftwf_execute(*(fftwf_plan*)w[1]);
    sigrfftw_invert((t_sample*)w[2],(t_int)w[3]);
    return w+4;
}
static t_int *sigrifftw_perform(t_int *w) {
    sigrfftw_invert((t_sample *)w[2],w[3]);
    fftwf_execute(*(fftwf_plan*)w[1]);
    return w+4;
}

static void sigfftw_dsp(t_sigfftw *x, t_signal **sp) {
    int n = sp[0]->n;
    float *in1 = sp[0]->v;
    float *in2 = sp[1]->v;
    float *out1 = sp[2]->v;
    float *out2 = sp[3]->v;
    x->dim.n=n;
    x->dim.is=1;
    x->dim.os=1;
    x->plan = fftwf_plan_guru_split_dft(1, &(x->dim), 0, NULL, in1, in2, out1, out2, FFTW_ESTIMATE);
    dsp_add(sigfftw_perform, 1, &x->plan);
}
static void sigifftw_dsp(t_sigfftw *x, t_signal **sp) {
    int n = sp[0]->n;
    float *in1 = sp[0]->v;
    float *in2 = sp[1]->v;
    float *out1 = sp[2]->v;
    float *out2 = sp[3]->v;
    x->dim.n=n;
    x->dim.is=1;
    x->dim.os=1;
    x->plan = fftwf_plan_guru_split_dft(1, &(x->dim), 0, NULL, in2, in1, out2, out1, FFTW_ESTIMATE);
    dsp_add(sigfftw_perform, 1, &x->plan);
}

static void sigrfftw_dsp(t_sigrfftw *x, t_signal **sp) {
    int n = sp[0]->n, n2 = (n>>1);
    float *in = sp[0]->v;
    float *out1 = sp[1]->v;
    float *out2 = sp[2]->v;
    if (n < 4) {error("fft: minimum 4 points"); return;}
    x->dim.n=n;
    x->dim.is=1;
    x->dim.os=1;
    x->plan = fftwf_plan_guru_split_dft_r2c(1, &(x->dim), 0, NULL, in, out1, out2, FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
    dsp_add(sigrfftw_perform,3,&x->plan,out2+1,n2-1);
    dsp_add_zero(out1 + n2, n2);
    dsp_add_zero(out2 + n2, n2);
}

static void sigrifftw_dsp(t_sigrifftw *x, t_signal **sp) {
    int n = sp[0]->n, n2 = (n>>1);
    float *in1 = sp[0]->v;
    float *in2 = sp[1]->v;
    float *out = sp[2]->v;
    if (n < 4) {error("fft: minimum 4 points"); return;}
    x->dim.n=n;
    x->dim.is=1;
    x->dim.os=1;
    x->plan = fftwf_plan_guru_split_dft_c2r(1, &(x->dim), 0, NULL, in1, in2, out, FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
    dsp_add_zero(in1+ n/2, n/2);
    dsp_add(sigrifftw_perform,3,&x->plan,in2,n2);
}
static void sigfftw_setup() {
    sigfftw_class   = class_new2(  "fft~",sigfftw_new,  sigfftw_free,  sizeof(t_sigfftw),  0,"");
    sigifftw_class  = class_new2( "ifft~",sigifftw_new, sigifftw_free, sizeof(t_sigfftw),  0,"");
    sigrfftw_class  = class_new2( "rfft~",sigrfftw_new, sigrfftw_free, sizeof(t_sigrfftw), 0,"");
    sigrifftw_class = class_new2("rifft~",sigrifftw_new,sigrifftw_free,sizeof(t_sigrifftw),0,"");
    CLASS_MAINSIGNALIN(sigfftw_class,  t_sigfftw,  a);
    CLASS_MAINSIGNALIN(sigifftw_class, t_sigfftw,  a);
    CLASS_MAINSIGNALIN(sigrfftw_class, t_sigrfftw, a);
    CLASS_MAINSIGNALIN(sigrifftw_class,t_sigrifftw,a);
    class_addmethod2(sigfftw_class,   sigfftw_dsp,  "dsp","");
    class_addmethod2(sigifftw_class,  sigifftw_dsp, "dsp","");
    class_addmethod2(sigrfftw_class,  sigrfftw_dsp, "dsp","");
    class_addmethod2(sigrifftw_class, sigrifftw_dsp,"dsp","");
    class_sethelpsymbol(sigifftw_class, gensym("fft~"));
    class_sethelpsymbol(sigrfftw_class, gensym("fft~"));
    class_sethelpsymbol(sigrifftw_class,gensym("fft~"));
}
#endif /* HAVE_LIBFFTW3F */
/* end of FFTW support */

/* ----------------------- framp~ -------------------------------- */
static t_class *sigframp_class;
struct t_sigframp : t_object {
    float a;
};
static void *sigframp_new() {
    t_sigframp *x = (t_sigframp *)pd_new(sigframp_class);
    inlet_new(x, x, &s_signal, &s_signal);
    outlet_new(x, &s_signal);
    outlet_new(x, &s_signal);
    x->a = 0;
    return x;
}
static t_int *sigframp_perform(t_int *w) {
    float *inreal = (t_float *)w[1];
    float *inimag = (t_float *)w[2];
    float *outfreq = (t_float *)w[3];
    float *outamp = (t_float *)w[4];
    float lastreal = 0, currentreal = inreal[0], nextreal = inreal[1];
    float lastimag = 0, currentimag = inimag[0], nextimag = inimag[1];
    int n = w[5];
    int m = n + 1;
    float fbin = 1, oneovern2 = 1.f/((float)n * (float)n);
    inreal += 2;
    inimag += 2;
    *outamp++ = *outfreq++ = 0;
    n -= 2;
    while (n--) {
    	float re, im, pow, freq;
    	lastreal = currentreal;
    	currentreal = nextreal;
    	nextreal = *inreal++;
    	lastimag = currentimag;
    	currentimag = nextimag;
    	nextimag = *inimag++;
    	re = currentreal - 0.5f * (lastreal + nextreal);
    	im = currentimag - 0.5f * (lastimag + nextimag);
    	pow = re * re + im * im;
    	if (pow > 1e-19) {
    	    float detune = ((lastreal - nextreal) * re +
    	    	    (lastimag - nextimag) * im) / (2.0f * pow);
    	    if (detune > 2 || detune < -2) freq = pow = 0;
    	    else freq = fbin + detune;
    	}
    	else freq = pow = 0;
    	*outfreq++ = freq;
    	*outamp++ = oneovern2 * pow;
    	fbin += 1.0f;
    }
    while (m--) *outamp++ = *outfreq++ = 0;
    return w+6;
}
t_int *sigsqrt_perform(t_int *w);
static void sigframp_dsp(t_sigframp *x, t_signal **sp) {
    int n = sp[0]->n, n2 = (n>>1);
    if (n < 4) {
    	error("framp: minimum 4 points");
    	return;
    }
    dsp_add(sigframp_perform, 5, sp[0]->v, sp[1]->v,
    	sp[2]->v, sp[3]->v, n2);
    dsp_add(sigsqrt_perform, 3, sp[3]->v, sp[3]->v, n2);
}
static void sigframp_setup() {
    sigframp_class = class_new2("framp~",sigframp_new,0,sizeof(t_sigframp),0,"");
    CLASS_MAINSIGNALIN(sigframp_class, t_sigframp, a);
    class_addmethod2(sigframp_class, sigframp_dsp, "dsp","");
}

/* ---------------- hip~ - 1-pole 1-zero hipass filter. ----------------- */
/* ---------------- lop~ - 1-pole lopass filter. ----------------- */
t_class *sighip_class; struct t_hipctl {float x; float coef;};
t_class *siglop_class; struct t_lopctl {float x; float coef;};
struct t_sighip : t_object {float sr; float hz; t_hipctl cspace; t_hipctl *ctl; float a;};
struct t_siglop : t_object {float sr; float hz; t_lopctl cspace; t_lopctl *ctl; float a;};

static void sighip_ft1(t_sighip *x, t_floatarg f) {x->hz = max(f,0.f); x->ctl->coef = clip(1-f*(2*3.14159)/x->sr,0.,1.);}
static void siglop_ft1(t_siglop *x, t_floatarg f) {x->hz = max(f,0.f); x->ctl->coef = clip(  f*(2*3.14159)/x->sr,0.,1.);}

static void *sighip_new(t_floatarg f) {
    t_sighip *x = (t_sighip *)pd_new(sighip_class);
    inlet_new(x, x, &s_float, gensym("ft1")); outlet_new(x, &s_signal);
    x->sr = 44100; x->ctl = &x->cspace; x->cspace.x = 0; sighip_ft1(x, f); x->a = 0; return x;}
static void *siglop_new(t_floatarg f) {
    t_siglop *x = (t_siglop *)pd_new(siglop_class);
    inlet_new(x, x, &s_float, gensym("ft1")); outlet_new(x, &s_signal);
    x->sr = 44100; x->ctl = &x->cspace; x->cspace.x = 0; siglop_ft1(x, f); x->a = 0; return x;}

static void sighip_clear(t_sighip *x, t_floatarg q) {x->cspace.x = 0;}
static void siglop_clear(t_siglop *x, t_floatarg q) {x->cspace.x = 0;}

static t_int *sighip_perform(t_int *w) {
    float *in = (float *)w[1];
    float *out = (float *)w[2];
    t_hipctl *c = (t_hipctl *)w[3];
    int n = (t_int)w[4];
    float last = c->x;
    float coef = c->coef;
    if (coef < 1) {
        for (int i = 0; i < n; i++) {
            float noo = *in++ + coef * last;
            *out++ = noo - last;
            last = noo;
        }
        if (PD_BIGORSMALL(last)) last = 0;
        c->x = last;
    } else {
        for (int i = 0; i < n; i++) *out++ = *in++;
        c->x = 0;
    }
    return w+5;
}
static t_int *siglop_perform(t_int *w) {
    float *in = (float *)w[1];
    float *out = (float *)w[2];
    t_lopctl *c = (t_lopctl *)w[3];
    int n = (t_int)w[4];
    float last = c->x;
    float coef = c->coef;
    float feedback = 1 - coef;
    for (int i = 0; i < n; i++) last = *out++ = coef * *in++ + feedback * last;
    if (PD_BIGORSMALL(last)) last = 0;
    c->x = last;
    return w+5;
}
static void sighip_dsp(t_sighip *x, t_signal **sp) {
    x->sr = sp[0]->sr; sighip_ft1(x,  x->hz);
    dsp_add(sighip_perform, 4, sp[0]->v, sp[1]->v, x->ctl, sp[0]->n);}
static void siglop_dsp(t_siglop *x, t_signal **sp) {
    x->sr = sp[0]->sr; siglop_ft1(x, x->hz);
    dsp_add(siglop_perform, 4, sp[0]->v, sp[1]->v, x->ctl, sp[0]->n);}

void sighip_setup() {
    sighip_class = class_new2("hip~",sighip_new,0,sizeof(t_sighip),0,"F");
    CLASS_MAINSIGNALIN(sighip_class, t_sighip, a);
    class_addmethod2(sighip_class, sighip_dsp, "dsp","");
    class_addmethod2(sighip_class, sighip_ft1, "ft1","f");
    class_addmethod2(sighip_class, sighip_clear,"clear","");
}
void siglop_setup() {
    siglop_class = class_new2("lop~",siglop_new,0,sizeof(t_siglop),0,"F");
    CLASS_MAINSIGNALIN(siglop_class, t_siglop, a);
    class_addmethod2(siglop_class, siglop_dsp, "dsp","");
    class_addmethod2(siglop_class, siglop_ft1, "ft1","f");
    class_addmethod2(siglop_class, siglop_clear, "clear","");
}

/* ---------------- bp~ - 2-pole bandpass filter. ----------------- */
struct t_bpctl {
    float x1;
    float x2;
    float coef1;
    float coef2;
    float gain;
};
struct t_sigbp : t_object {
    float sr;
    float freq;
    float q;
    t_bpctl cspace;
    t_bpctl *ctl;
    float a;
};
t_class *sigbp_class;
static void sigbp_docoef(t_sigbp *x, t_floatarg f, t_floatarg q);
static void *sigbp_new(t_floatarg f, t_floatarg q) {
    t_sigbp *x = (t_sigbp *)pd_new(sigbp_class);
    inlet_new(x, x, &s_float, gensym("ft1"));
    inlet_new(x, x, &s_float, gensym("ft2"));
    outlet_new(x, &s_signal);
    x->sr = 44100;
    x->ctl = &x->cspace;
    x->cspace.x1 = 0;
    x->cspace.x2 = 0;
    sigbp_docoef(x, f, q);
    x->a = 0;
    return x;
}
static float sigbp_qcos(float f) {
    if (f >= -(0.5f*3.14159f) && f <= 0.5f*3.14159f) {
        float g = f*f;
        return ((g*g*g * (-1.0f/720.0f) + g*g*(1.0f/24.0f)) - g*0.5) + 1;
    } else return 0;
}
static void sigbp_docoef(t_sigbp *x, t_floatarg f, t_floatarg q) {
    float r, oneminusr, omega;
    if (f < 0.001) f = 10;
    if (q < 0) q = 0;
    x->freq = f;
    x->q = q;
    omega = f * (2.0f * 3.14159f) / x->sr;
    if (q < 0.001) oneminusr = 1.0f;
    else oneminusr = omega/q;
    if (oneminusr > 1.0f) oneminusr = 1.0f;
    r = 1.0f - oneminusr;
    x->ctl->coef1 = 2.0f * sigbp_qcos(omega) * r;
    x->ctl->coef2 = - r * r;
    x->ctl->gain = 2 * oneminusr * (oneminusr + r * omega);
    /* post("r %f, omega %f, coef1 %f, coef2 %f", r, omega, x->ctl->coef1, x->ctl->coef2); */
}
static void sigbp_ft1(t_sigbp *x, t_floatarg f) {sigbp_docoef(x, f, x->q);}
static void sigbp_ft2(t_sigbp *x, t_floatarg q) {sigbp_docoef(x, x->freq, q);}
static void sigbp_clear(t_sigbp *x, t_floatarg q) {x->ctl->x1 = x->ctl->x2 = 0;}
static t_int *sigbp_perform(t_int *w) {
    float *in = (float *)w[1];
    float *out = (float *)w[2];
    t_bpctl *c = (t_bpctl *)w[3];
    int n = (t_int)w[4];
    float last = c->x1;
    float prev = c->x2;
    float coef1 = c->coef1;
    float coef2 = c->coef2;
    float gain = c->gain;
    for (int i = 0; i < n; i++) {
        float output =  *in++ + coef1 * last + coef2 * prev;
        *out++ = gain * output;
        prev = last;
        last = output;
    }
    if (PD_BIGORSMALL(last)) last = 0;
    if (PD_BIGORSMALL(prev)) prev = 0;
    c->x1 = last;
    c->x2 = prev;
    return w+5;
}
static void sigbp_dsp(t_sigbp *x, t_signal **sp) {
    x->sr = sp[0]->sr;
    sigbp_docoef(x, x->freq, x->q);
    dsp_add(sigbp_perform, 4, sp[0]->v, sp[1]->v, x->ctl, sp[0]->n);

}
void sigbp_setup() {
    sigbp_class = class_new2("bp~",sigbp_new,0,sizeof(t_sigbp),0,"FF");
    CLASS_MAINSIGNALIN(sigbp_class, t_sigbp, a);
    class_addmethod2(sigbp_class, sigbp_dsp, "dsp","");
    class_addmethod2(sigbp_class, sigbp_ft1, "ft1","f");
    class_addmethod2(sigbp_class, sigbp_ft2, "ft2","f");
    class_addmethod2(sigbp_class, sigbp_clear, "clear","");
}

/* ---------------- biquad~ - raw biquad filter ----------------- */
struct t_biquadctl {
    float x1;
    float x2;
    float fb1;
    float fb2;
    float ff1;
    float ff2;
    float ff3;
};
struct t_sigbiquad : t_object {
    float a;
    t_biquadctl cspace;
    t_biquadctl *ctl;
};
t_class *sigbiquad_class;
static void sigbiquad_list(t_sigbiquad *x, t_symbol *s, int argc, t_atom *argv);
static void *sigbiquad_new(t_symbol *s, int argc, t_atom *argv) {
    t_sigbiquad *x = (t_sigbiquad *)pd_new(sigbiquad_class);
    outlet_new(x, &s_signal);
    x->ctl = &x->cspace;
    x->cspace.x1 = x->cspace.x2 = 0;
    sigbiquad_list(x, s, argc, argv);
    x->a = 0;
    return x;
}
static t_int *sigbiquad_perform(t_int *w) {
    float *in = (float *)w[1];
    float *out = (float *)w[2];
    t_biquadctl *c = (t_biquadctl *)w[3];
    int n = (t_int)w[4];
    float last = c->x1;
    float prev = c->x2;
    float fb1 = c->fb1;
    float fb2 = c->fb2;
    float ff1 = c->ff1;
    float ff2 = c->ff2;
    float ff3 = c->ff3;
    for (int i = 0; i < n; i++) {
        float output =  *in++ + fb1 * last + fb2 * prev;
        if (PD_BIGORSMALL(output)) output = 0;
        *out++ = ff1 * output + ff2 * last + ff3 * prev;
        prev = last;
        last = output;
    }
    c->x1 = last;
    c->x2 = prev;
    return w+5;
}
/* tb: some loop unrolling & do some relaxed denormal bashing */
/* (denormal bashing = non-Pentium4 penalised for Pentium4's failings) */
static t_int *sigbiquad_perf8(t_int *w) {
    float *in = (float *)w[1];
    float *out = (float *)w[2];
    t_biquadctl *c = (t_biquadctl *)w[3];
    int n = (t_int)w[4]>>3;
    float last = c->x1;
    float prev = c->x2;
    float fb1 = c->fb1;
    float fb2 = c->fb2;
    float ff1 = c->ff1;
    float ff2 = c->ff2;
    float ff3 = c->ff3;
    for (int i = 0; i < n; i++) {
        float output =  *in++ + fb1*last + fb2*prev;
        if (PD_BIGORSMALL(output)) output = 0;
	*out++ = ff1 * output + ff2*last + ff3*prev; prev=last; last=output; output = *in++ + fb1*last + fb2*prev;
        if (PD_BIGORSMALL(output)) output = 0;
        *out++ = ff1 * output + ff2*last + ff3*prev; prev=last; last=output; output = *in++ + fb1*last + fb2*prev;
        *out++ = ff1 * output + ff2*last + ff3*prev; prev=last; last=output; output = *in++ + fb1*last + fb2*prev;
        *out++ = ff1 * output + ff2*last + ff3*prev; prev=last; last=output; output = *in++ + fb1*last + fb2*prev;
	output += 1e-10;
	output -= 1e-10;
	*out++ = ff1*output + ff2*last + ff3*prev; prev=last; last=output; output = *in++ + fb1*last + fb2*prev;
        *out++ = ff1*output + ff2*last + ff3*prev; prev=last; last=output; output = *in++ + fb1*last + fb2*prev;
        *out++ = ff1*output + ff2*last + ff3*prev; prev=last; last=output; output = *in++ + fb1*last + fb2*prev;
        *out++ = ff1*output + ff2*last + ff3*prev; prev=last; last=output;
    }
    c->x1 = last;
    c->x2 = prev;
    return w+5;
}
static void sigbiquad_list(t_sigbiquad *x, t_symbol *s, int argc, t_atom *argv) {
    float fb1 = atom_getfloatarg(0, argc, argv);
    float fb2 = atom_getfloatarg(1, argc, argv);
    float ff1 = atom_getfloatarg(2, argc, argv);
    float ff2 = atom_getfloatarg(3, argc, argv);
    float ff3 = atom_getfloatarg(4, argc, argv);
    float discriminant = fb1 * fb1 + 4 * fb2;
    t_biquadctl *c = x->ctl;
    /* imaginary roots -- resonant filter */
    if (discriminant < 0) {
        /* they're conjugates so we just check that the product is less than one */
        if (fb2 >= -1.0f) goto stable;
    } else {    /* real roots */
        /* check that the parabola 1 - fb1 x - fb2 x^2 has a
           vertex between -1 and 1, and that it's nonnegative
           at both ends, which implies both roots are in [1-,1]. */
        if (fb1 <= 2.0f && fb1 >= -2.0f && 1.0f - fb1 -fb2 >= 0 && 1.0f + fb1 - fb2 >= 0) goto stable;
    }
    /* if unstable, just bash to zero */
    fb1 = fb2 = ff1 = ff2 = ff3 = 0;
stable:
    c->fb1 = fb1;
    c->fb2 = fb2;
    c->ff1 = ff1;
    c->ff2 = ff2;
    c->ff3 = ff3;
}
static void sigbiquad_set(t_sigbiquad *x, t_symbol *s, int argc, t_atom *argv) {
    t_biquadctl *c = x->ctl;
    c->x1 = atom_getfloatarg(0, argc, argv);
    c->x2 = atom_getfloatarg(1, argc, argv);
}
static void sigbiquad_dsp(t_sigbiquad *x, t_signal **sp) {
	const int n = sp[0]->n;
	if (n&7) dsp_add(sigbiquad_perform, 4, sp[0]->v, sp[1]->v, x->ctl, sp[0]->n);
	else     dsp_add(sigbiquad_perf8,   4, sp[0]->v, sp[1]->v, x->ctl, sp[0]->n);
}
void sigbiquad_setup() {
    sigbiquad_class = class_new2("biquad~",sigbiquad_new,0,sizeof(t_sigbiquad),0,"*");
    CLASS_MAINSIGNALIN(sigbiquad_class, t_sigbiquad, a);
    class_addmethod2(sigbiquad_class, sigbiquad_dsp, "dsp","");
    class_addlist(sigbiquad_class, sigbiquad_list);
    class_addmethod2(sigbiquad_class, sigbiquad_set, "set","*");
    class_addmethod2(sigbiquad_class, sigbiquad_set, "clear","*");
}

/* ---------------- samphold~ - sample and hold  ----------------- */
struct t_sigsamphold : t_object {
    float a;
    float lastin;
    float lastout;
};
t_class *sigsamphold_class;
static void *sigsamphold_new() {
    t_sigsamphold *x = (t_sigsamphold *)pd_new(sigsamphold_class);
    inlet_new(x, x, &s_signal, &s_signal);
    outlet_new(x, &s_signal);
    x->lastin = 0;
    x->lastout = 0;
    x->a = 0;
    return x;
}
static t_int *sigsamphold_perform(t_int *w) {
    float *in1 = (float *)w[1];
    float *in2 = (float *)w[2];
    float *out = (float *)w[3];
    t_sigsamphold *x = (t_sigsamphold *)w[4];
    int n = (t_int)w[5];
    float lastin = x->lastin;
    float lastout = x->lastout;
    for (int i = 0; i < n; i++, *in1++) {
        float next = *in2++;
        if (next < lastin) lastout = *in1;
        *out++ = lastout;
        lastin = next;
    }
    x->lastin = lastin;
    x->lastout = lastout;
    return w+6;
}
static void sigsamphold_dsp(t_sigsamphold *x, t_signal **sp) {
    dsp_add(sigsamphold_perform, 5, sp[0]->v, sp[1]->v, sp[2]->v, x, sp[0]->n);
}
static void sigsamphold_reset(t_sigsamphold *x, t_symbol *s, int argc, t_atom *argv) {
    x->lastin = ((argc > 0 && (argv[0].a_type == A_FLOAT)) ? argv[0].a_w.w_float : 1e20);
}
static void sigsamphold_set(t_sigsamphold *x, t_float f) {
    x->lastout = f;
}
void sigsamphold_setup() {
    sigsamphold_class = class_new2("samphold~",sigsamphold_new,0,sizeof(t_sigsamphold),0,"");
    CLASS_MAINSIGNALIN(sigsamphold_class, t_sigsamphold, a);
    class_addmethod2(sigsamphold_class, sigsamphold_set, "set","F");
    class_addmethod2(sigsamphold_class, sigsamphold_reset, "reset","*");
    class_addmethod2(sigsamphold_class, sigsamphold_dsp, "dsp","");
}

/* ---------------- rpole~ - real one-pole filter (raw) ----------------- */
/* ---------------- rzero~ - real one-zero filter (raw) ----------------- */
/* --- rzero_rev~ - real, reverse one-zero filter (raw) ----------------- */
t_class *sigrpole_class; struct t_sigrpole : t_object {float a; float last;};
t_class *sigrzero_class; struct t_sigrzero : t_object {float a; float last;};
t_class *sigrzrev_class; struct t_sigrzrev : t_object {float a; float last;};

static void *sigrpole_new(t_float f) {
    t_sigrpole *x = (t_sigrpole *)pd_new(sigrpole_class);
    pd_float((t_pd *)inlet_new(x, x, &s_signal, &s_signal), f); outlet_new(x, &s_signal); x->last=0; return x;}
static void *sigrzero_new(t_float f) {
    t_sigrzero *x = (t_sigrzero *)pd_new(sigrzero_class);
    pd_float((t_pd *)inlet_new(x, x, &s_signal, &s_signal), f); outlet_new(x, &s_signal); x->last=0; return x;}
static void *sigrzrev_new(t_float f) {
    t_sigrzrev *x = (t_sigrzrev *)pd_new(sigrzrev_class);
    pd_float((t_pd *)inlet_new(x, x, &s_signal, &s_signal), f); outlet_new(x, &s_signal); x->last=0; return x;}

static t_int *sigrpole_perform(t_int *w) {
    float *in1 = (float *)w[1]; float *in2 = (float *)w[2]; float *out = (float *)w[3];
    t_sigrpole *x = (t_sigrpole *)w[4]; int n = (t_int)w[5]; float last = x->last;
    for (int i=0; i<n; i++) {
        float next = *in1++, coef = *in2++;
        *out++ = last = coef*last + next;
    }
    if (PD_BIGORSMALL(last)) last = 0;
    x->last = last;
    return w+6;
}
static t_int *sigrzero_perform(t_int *w) {
    float *in1 = (float *)w[1]; float *in2 = (float *)w[2]; float *out = (float *)w[3];
    t_sigrzero *x = (t_sigrzero *)w[4]; int n = (t_int)w[5]; float last = x->last;
    for (int i = 0; i < n; i++) {
        float next = *in1++, coef = *in2++;
        *out++ = next - coef*last;
        last = next;
    }
    x->last = last;
    return w+6;
}
static t_int *sigrzrev_perform(t_int *w) {
    float *in1 = (float *)w[1]; float *in2 = (float *)w[2]; float *out = (float *)w[3];
    t_sigrzrev *x = (t_sigrzrev *)w[4]; int n = (t_int)w[5]; float last = x->last;
    for (int i = 0; i < n; i++) {
        float next = *in1++, coef = *in2++;
        *out++ = last - coef*next;
        last = next;
    }
    x->last = last;
    return w+6;
}

static void sigrpole_dsp(t_sigrpole *x, t_signal **sp) {dsp_add(sigrpole_perform, 5, sp[0]->v, sp[1]->v, sp[2]->v, x, sp[0]->n);}
static void sigrzero_dsp(t_sigrzero *x, t_signal **sp) {dsp_add(sigrzero_perform, 5, sp[0]->v, sp[1]->v, sp[2]->v, x, sp[0]->n);}
static void sigrzrev_dsp(t_sigrzrev *x, t_signal **sp) {dsp_add(sigrzrev_perform, 5, sp[0]->v, sp[1]->v, sp[2]->v, x, sp[0]->n);}
static void sigrpole_clear(t_sigrpole *x) {x->last = 0;}
static void sigrzero_clear(t_sigrzero *x) {x->last = 0;}
static void sigrzrev_clear(t_sigrzrev *x) {x->last = 0;}
static void sigrpole_set(t_sigrpole *x, t_float f) {x->last = f;}
static void sigrzero_set(t_sigrzero *x, t_float f) {x->last = f;}
static void sigrzrev_set(t_sigrzrev *x, t_float f) {x->last = f;}
void sigr_setup() {
    sigrpole_class = class_new2("rpole~",    sigrpole_new,0,sizeof(t_sigrpole),0,"F");
    sigrzero_class = class_new2("rzero~",    sigrzero_new,0,sizeof(t_sigrzero),0,"F");
    sigrzrev_class = class_new2("rzero_rev~",sigrzrev_new,0,sizeof(t_sigrzrev),0,"F");
    CLASS_MAINSIGNALIN(sigrpole_class, t_sigrpole, a);
    CLASS_MAINSIGNALIN(sigrzero_class, t_sigrzero, a);
    CLASS_MAINSIGNALIN(sigrzrev_class, t_sigrzrev, a);
    class_addmethod2(sigrpole_class, sigrpole_set,  "set","F");
    class_addmethod2(sigrzero_class, sigrzero_set,  "set","F");
    class_addmethod2(sigrzrev_class, sigrzrev_set,  "set","F");
    class_addmethod2(sigrpole_class, sigrpole_clear,"clear","");
    class_addmethod2(sigrzero_class, sigrzero_clear,"clear","");
    class_addmethod2(sigrzrev_class, sigrzrev_clear,"clear","");
    class_addmethod2(sigrpole_class, sigrpole_dsp,  "dsp","");
    class_addmethod2(sigrzero_class, sigrzero_dsp,  "dsp","");
    class_addmethod2(sigrzrev_class, sigrzrev_dsp,  "dsp","");
}

/* -------------- cpole~ - complex one-pole filter (raw) --------------- */
/* -------------- czero~ - complex one-pole filter (raw) --------------- */
/* ---------- czero_rev~ - complex one-pole filter (raw) --------------- */

t_class *sigcpole_class; struct t_sigcpole : t_object {float a; float lastre; float lastim;};
t_class *sigczero_class; struct t_sigczero : t_object {float a; float lastre; float lastim;};
t_class *sigczrev_class; struct t_sigczrev : t_object {float a; float lastre; float lastim;};

static void *sigcpole_new(t_float re, t_float im) {
    t_sigcpole *x = (t_sigcpole *)pd_new(sigcpole_class);
    inlet_new(x, x, &s_signal, &s_signal);
    pd_float((t_pd *)inlet_new(x, x, &s_signal, &s_signal), re);
    pd_float((t_pd *)inlet_new(x, x, &s_signal, &s_signal), im);
    outlet_new(x, &s_signal);
    outlet_new(x, &s_signal);
    x->lastre = x->lastim = 0;
    x->a = 0;
    return x;
}
static void *sigczero_new(t_float re, t_float im) {
    t_sigczero *x = (t_sigczero *)pd_new(sigczero_class);
    inlet_new(x, x, &s_signal, &s_signal);
    pd_float((t_pd *)inlet_new(x, x, &s_signal, &s_signal), re);
    pd_float((t_pd *)inlet_new(x, x, &s_signal, &s_signal), im);
    outlet_new(x, &s_signal);
    outlet_new(x, &s_signal);
    x->lastre = x->lastim = 0;
    x->a = 0;
    return x;
}
static void *sigczrev_new(t_float re, t_float im) {
    t_sigczrev *x = (t_sigczrev *)pd_new(sigczrev_class);
    inlet_new(x, x, &s_signal, &s_signal);
    pd_float((t_pd *)inlet_new(x, x, &s_signal, &s_signal), re);
    pd_float((t_pd *)inlet_new(x, x, &s_signal, &s_signal), im);
    outlet_new(x, &s_signal);
    outlet_new(x, &s_signal);
    x->lastre = x->lastim = 0;
    x->a = 0;
    return x;
}

static t_int *sigcpole_perform(t_int *w) {
    float *inre1 = (float *)w[1], *inim1 = (float *)w[2];
    float *inre2 = (float *)w[3], *inim2 = (float *)w[4];
    float *outre = (float *)w[5], *outim = (float *)w[6];
    t_sigcpole *x = (t_sigcpole *)w[7];
    int n = (t_int)w[8];
    float lastre = x->lastre;
    float lastim = x->lastim;
    for (int i = 0; i < n; i++) {
        float nextre = *inre1++, nextim = *inim1++;
        float coefre = *inre2++, coefim = *inim2++;
        float tempre = *outre++ = nextre + lastre * coefre - lastim * coefim;
        lastim = *outim++ = nextim + lastre * coefim + lastim * coefre;
        lastre = tempre;
    }
    if (PD_BIGORSMALL(lastre)) lastre = 0;
    if (PD_BIGORSMALL(lastim)) lastim = 0;
    x->lastre = lastre;
    x->lastim = lastim;
    return w+9;
}
static t_int *sigczero_perform(t_int *w) {
    float *inre1 = (float *)w[1], *inim1 = (float *)w[2];
    float *inre2 = (float *)w[3], *inim2 = (float *)w[4];
    float *outre = (float *)w[5], *outim = (float *)w[6];
    t_sigczero *x = (t_sigczero *)w[7];
    int n = (t_int)w[8];
    float lastre = x->lastre;
    float lastim = x->lastim;
    for (int i = 0; i < n; i++) {
        float nextre = *inre1++, nextim = *inim1++;
        float coefre = *inre2++, coefim = *inim2++;
        *outre++ = nextre - lastre * coefre + lastim * coefim;
        *outim++ = nextim - lastre * coefim - lastim * coefre;
        lastre = nextre;
        lastim = nextim;
    }
    x->lastre = lastre;
    x->lastim = lastim;
    return w+9;
}
static t_int *sigczrev_perform(t_int *w) {
    float *inre1 = (float *)w[1], *inim1 = (float *)w[2];
    float *inre2 = (float *)w[3], *inim2 = (float *)w[4];
    float *outre = (float *)w[5], *outim = (float *)w[6];
    t_sigczrev *x = (t_sigczrev *)w[7];
    int n = (t_int)w[8];
    float lastre = x->lastre;
    float lastim = x->lastim;
    for (int i = 0; i < n; i++) {
        float nextre = *inre1++, nextim = *inim1++;
        float coefre = *inre2++, coefim = *inim2++;
        /* transfer function is (A bar) - Z^-1, for the same frequency response as 1 - AZ^-1 from czero_tilde. */
        *outre++ = lastre - nextre * coefre - nextim * coefim;
        *outim++ = lastim - nextre * coefim + nextim * coefre;
        lastre = nextre;
        lastim = nextim;
    }
    x->lastre = lastre;
    x->lastim = lastim;
    return w+9;
}

static void sigcpole_dsp(t_sigcpole *x, t_signal **sp) {
    dsp_add(sigcpole_perform, 8, sp[0]->v, sp[1]->v, sp[2]->v, sp[3]->v, sp[4]->v, sp[5]->v, x, sp[0]->n);}
static void sigczero_dsp(t_sigczero *x, t_signal **sp) {
    dsp_add(sigczero_perform, 8, sp[0]->v, sp[1]->v, sp[2]->v, sp[3]->v, sp[4]->v, sp[5]->v, x, sp[0]->n);}
static void sigczrev_dsp(t_sigczrev *x, t_signal **sp) {
    dsp_add(sigczrev_perform, 8, sp[0]->v, sp[1]->v, sp[2]->v, sp[3]->v, sp[4]->v, sp[5]->v, x, sp[0]->n);}

static void sigcpole_clear(t_sigcpole *x) {x->lastre = x->lastim = 0;}
static void sigczero_clear(t_sigczero *x) {x->lastre = x->lastim = 0;}
static void sigczrev_clear(t_sigczrev *x) {x->lastre = x->lastim = 0;}
static void sigcpole_set(t_sigcpole *x, t_float re, t_float im) {x->lastre = re; x->lastim = im;}
static void sigczero_set(t_sigczero *x, t_float re, t_float im) {x->lastre = re; x->lastim = im;}
static void sigczrev_set(t_sigczrev *x, t_float re, t_float im) {x->lastre = re; x->lastim = im;}

void sigc_setup() {
    sigcpole_class = class_new2("cpole~",    sigcpole_new,0,sizeof(t_sigcpole),0,"FF");
    sigczero_class = class_new2("czero~",    sigczero_new,0,sizeof(t_sigczero),0,"FF");
    sigczrev_class = class_new2("czero_rev~",sigczrev_new,0,sizeof(t_sigczrev),0,"FF");
    CLASS_MAINSIGNALIN(sigcpole_class, t_sigcpole, a);
    CLASS_MAINSIGNALIN(sigczero_class, t_sigczero, a);
    CLASS_MAINSIGNALIN(sigczrev_class, t_sigczrev, a);
    class_addmethod2(sigcpole_class, sigcpole_set,  "set","FF");
    class_addmethod2(sigczero_class, sigczero_set,   "set","FF");
    class_addmethod2(sigczrev_class, sigczrev_set,   "set","FF");
    class_addmethod2(sigcpole_class, sigcpole_clear,"clear","");
    class_addmethod2(sigczero_class, sigczero_clear, "clear","");
    class_addmethod2(sigczrev_class, sigczrev_clear, "clear","");
    class_addmethod2(sigcpole_class, sigcpole_dsp,  "dsp","");
    class_addmethod2(sigczero_class, sigczero_dsp,   "dsp","");
    class_addmethod2(sigczrev_class, sigczrev_dsp,   "dsp","");
}

/* ----------------------------- send~ ----------------------------- */
static t_class *sigsend_class;
struct t_sigsend : t_object {
    t_symbol *sym;
    int n;
    float *vec;
    float a;
};
static void *sigsend_new(t_symbol *s) {
    t_sigsend *x = (t_sigsend *)pd_new(sigsend_class);
    pd_bind(x, s);
    x->sym = s;
    x->n = DEFSENDVS;
    x->vec = (float *)getalignedbytes(DEFSENDVS * sizeof(float));
    memset((char *)(x->vec), 0, DEFSENDVS * sizeof(float));
    x->a = 0;
    return x;
}
static t_int *sigsend_perform(t_int *w) {
    testcopyvec((t_float *)w[2],(t_float *)w[1],w[3]);
    return w+4;
}
/* T.Grill - SIMD version */
static t_int *sigsend_perfsimd(t_int *w) {
    testcopyvec_simd((t_float *)w[2],(t_float *)w[1],w[3]);
    return w+4;
}
static void sigsend_dsp(t_sigsend *x, t_signal **sp) {
    const int n = x->n;
    if(n != sp[0]->n) {error("sigsend %s: unexpected vector size", x->sym->name); return;}
    if(SIMD_CHECK1(n,sp[0]->v)) /* x->vec is aligned in any case */
         dsp_add(sigsend_perfsimd, 3, sp[0]->v, x->vec, n);
    else dsp_add(sigsend_perform,  3, sp[0]->v, x->vec, n);
}
static void sigsend_free(t_sigsend *x) {
    pd_unbind(x, x->sym);
    freealignedbytes(x->vec,x->n* sizeof(float));
}
static void sigsend_setup() {
    sigsend_class = class_new2("send~",sigsend_new,sigsend_free,sizeof(t_sigsend),0,"S");
    class_addcreator2("s~",sigsend_new,"S");
    CLASS_MAINSIGNALIN(sigsend_class, t_sigsend, a);
    class_addmethod2(sigsend_class, sigsend_dsp,"dsp","");
}

/* ----------------------------- receive~ ----------------------------- */
static t_class *sigreceive_class;
struct t_sigreceive : t_object {
    t_symbol *sym;
    t_float *wherefrom;
    int n;
};
static void *sigreceive_new(t_symbol *s) {
    t_sigreceive *x = (t_sigreceive *)pd_new(sigreceive_class);
    x->n = DEFSENDVS;             /* LATER find our vector size correctly */
    x->sym = s;
    x->wherefrom = 0;
    outlet_new(x, &s_signal);
    return x;
}
static t_int *sigreceive_perform(t_int *w) {
    t_sigreceive *x = (t_sigreceive *)w[1];
    t_float *out = (t_float *)w[2];
    int n = (int)w[3];
    t_float *in = x->wherefrom;
    if (in) {
        while (n--) *out++ = *in++;
    } else {
        while (n--) *out++ = 0;
    }
    return w+4;
}
/* tb: vectorized receive function */
static t_int *sigreceive_perf8(t_int *w) {
    t_sigreceive *x = (t_sigreceive *)w[1];
    t_float *in = x->wherefrom;
    if (in) copyvec_8((t_float *)w[2],in,w[3]);
    else    zerovec_8((t_float *)w[2],w[3]);
    return w+4;
}
/* T.Grill - SIMD version */
static t_int *sigreceive_perfsimd(t_int *w) {
    t_sigreceive *x = (t_sigreceive *)w[1];
    t_float *in = x->wherefrom;
    if(in) copyvec_simd((t_float *)w[2],in,w[3]);
    else   zerovec_simd((t_float *)w[2],w[3]);
    return w+4;
}
static void sigreceive_set(t_sigreceive *x, t_symbol *s) {
    t_sigsend *sender = (t_sigsend *)pd_findbyclass((x->sym = s),
        sigsend_class);
    if (sender) {
        if (sender->n == x->n)
            x->wherefrom = sender->vec;
        else {
            error("receive~ %s: vector size mismatch", x->sym->name);
            x->wherefrom = 0;
        }
    } else {
        error("receive~ %s: no matching send", x->sym->name);
        x->wherefrom = 0;
    }
}
static void sigreceive_dsp(t_sigreceive *x, t_signal **sp) {
    const int n = x->n;
    if (sp[0]->n != n) {error("receive~ %s: vector size mismatch", x->sym->name); return;}
    sigreceive_set(x, x->sym);
    /* x->wherefrom is aligned because we aligned the sender memory buffer */
    if(n&7) dsp_add(sigreceive_perform,  3, x, sp[0]->v, n);
    else if(SIMD_CHECK1(n,sp[0]->v))
	    dsp_add(sigreceive_perfsimd, 3, x, sp[0]->v, n);
    else    dsp_add(sigreceive_perf8,    3, x, sp[0]->v, n);
}
static void sigreceive_setup() {
    sigreceive_class = class_new2("receive~",sigreceive_new,0,sizeof(t_sigreceive),0,"S");
    class_addcreator2("r~",sigreceive_new,"S");
    class_addmethod2(sigreceive_class, sigreceive_set, "set","s");
    class_addmethod2(sigreceive_class, sigreceive_dsp, "dsp","");
    class_sethelpsymbol(sigreceive_class, gensym("send~"));
}

/* ----------------------------- catch~ ----------------------------- */
static t_class *sigcatch_class;
struct t_sigcatch : t_object {
    t_symbol *sym;
    int n;
    float *vec;
};
static void *sigcatch_new(t_symbol *s) {
    t_sigcatch *x = (t_sigcatch *)pd_new(sigcatch_class);
    pd_bind(x, s);
    x->sym = s;
    x->n = DEFSENDVS;
    x->vec = (float *)getalignedbytes(DEFSENDVS * sizeof(float));
    memset((char *)(x->vec), 0, DEFSENDVS * sizeof(float));
    outlet_new(x, &s_signal);
    return x;
}
static t_int *sigcatch_perform(t_int *w) {
    t_float *in = (t_float *)w[1];
    t_float *out = (t_float *)w[2];
    int n = (int)w[3];
    while (n--) *out++ = *in, *in++ = 0;
    return w+4;
}
/* tb: vectorized catch function */
static t_int *sigcatch_perf8(t_int *w) {
    copyvec_8((t_float *)w[2],(t_float *)w[1],w[3]);
    zerovec_8((t_float *)w[1],w[3]);
    return w+4;
}
/* T.Grill: SIMD catch function */
static t_int *sigcatch_perfsimd(t_int *w) {
    copyvec_simd((t_float *)w[2],(t_float *)w[1],w[3]);
    zerovec_simd((t_float *)w[1],w[3]);
    return w+4;
}
static void sigcatch_dsp(t_sigcatch *x, t_signal **sp) {
    const int n = sp[0]->n;
    if (x->n != n) {error("sigcatch %s: unexpected vector size", x->sym->name); return;}
    if(n&7) dsp_add(sigcatch_perform, 3, x->vec, sp[0]->v, n);
    else if(SIMD_CHECK2(n,x->vec,sp[0]->v))
	 dsp_add(sigcatch_perfsimd,   3, x->vec, sp[0]->v, n);
    else dsp_add(sigcatch_perf8,      3, x->vec, sp[0]->v, n);
}
static void sigcatch_free(t_sigcatch *x) {
    pd_unbind(x, x->sym);
    freealignedbytes(x->vec,x->n*sizeof(float));
}
static void sigcatch_setup() {
    sigcatch_class = class_new2("catch~",sigcatch_new,sigcatch_free,sizeof(t_sigcatch),CLASS_NOINLET,"S");
    class_addmethod2(sigcatch_class, sigcatch_dsp, "dsp","");
    class_sethelpsymbol(sigcatch_class, gensym("throw~"));
}

/* ----------------------------- throw~ ----------------------------- */
static t_class *sigthrow_class;
struct t_sigthrow : t_object {
    t_symbol *sym;
    t_float *whereto;
    int n;
    t_float a;
};
static void *sigthrow_new(t_symbol *s) {
    t_sigthrow *x = (t_sigthrow *)pd_new(sigthrow_class);
    x->sym = s;
    x->whereto  = 0;
    x->n = DEFSENDVS;
    x->a = 0;
    return x;
}
static t_int *sigthrow_perform(t_int *w) {
    t_sigthrow *x = (t_sigthrow *)w[1];
    t_float *out = x->whereto;
    if(out) testaddvec(out,(t_float *)w[2],w[3]);
    return w+4;
}
/* T.Grill - SIMD version */
static t_int *sigthrow_perfsimd(t_int *w) {
    t_sigthrow *x = (t_sigthrow *)w[1];
    t_float *out = x->whereto;
    if(out) testaddvec_simd(out,(t_float *)w[2],w[3]);
    return w+4;
}
static void sigthrow_set(t_sigthrow *x, t_symbol *s) {
    x->sym = s;
    t_sigcatch *catcher = (t_sigcatch *)pd_findbyclass(s,sigcatch_class);
    x->whereto = 0;
    if (catcher) {
        if (catcher->n == x->n) x->whereto = catcher->vec;
        else error("throw~ %s: vector size mismatch", x->sym->name);
    } else error("throw~ %s: no matching catch", x->sym->name);
}
static void sigthrow_dsp(t_sigthrow *x, t_signal **sp) {
    const int n = x->n;
    if (sp[0]->n != n) {error("throw~ %s: vector size mismatch", x->sym->name); return;}
    sigthrow_set(x, x->sym);
    if(SIMD_CHECK1(n,sp[0]->v)) /* the memory of the catcher is aligned in any case */
    	    dsp_add(sigthrow_perfsimd, 3, x, sp[0]->v, n);
    else    dsp_add(sigthrow_perform,  3, x, sp[0]->v, n);
}
static void sigthrow_setup() {
    sigthrow_class = class_new2("throw~",sigthrow_new,0,sizeof(t_sigthrow),0,"S");
    class_addmethod2(sigthrow_class, sigthrow_set, "set","s");
    CLASS_MAINSIGNALIN(sigthrow_class, t_sigthrow, a);
    class_addmethod2(sigthrow_class, sigthrow_dsp, "dsp","");
}

/* ------------------------- clip~ -------------------------- */
static t_class *clip_class;
struct t_clip : t_object {
    float a;
    t_sample lo;
    t_sample hi;
};
static void *clip_new(t_floatarg lo, t_floatarg hi) {
    t_clip *x = (t_clip *)pd_new(clip_class);
    x->lo = lo;
    x->hi = hi;
    outlet_new(x, &s_signal);
    floatinlet_new(x, &x->lo);
    floatinlet_new(x, &x->hi);
    x->a = 0;
    return x;
}
/* T.Grill - changed function interface so that class pointer needn't be passed */
t_int *clip_perform(t_int *w) {
    t_float *in = (t_float *)w[1];
    t_float *out = (t_float *)w[2];
    const t_float lo = *(t_float *)w[3],hi = *(t_float *)w[4];
    int n = (int)w[5];
    while (n--) *out++ = clip(*in++,lo,hi);
    return w+6;
}
static void clip_dsp(t_clip *x, t_signal **sp) {
    if(SIMD_CHECK2(sp[0]->n,sp[0]->v,sp[1]->v))
	dsp_add(clip_perf_simd, 5, sp[0]->v, sp[1]->v, &x->lo, &x->hi, sp[0]->n);
    else dsp_add(clip_perform,  5, sp[0]->v, sp[1]->v, &x->lo, &x->hi, sp[0]->n);
}
static void clip_setup() {
    clip_class = class_new2("clip~",clip_new,0,sizeof(t_clip),0,"FF");
    CLASS_MAINSIGNALIN(clip_class, t_clip, a);
    class_addmethod2(clip_class, clip_dsp, "dsp","");
}

/* sigrsqrt - reciprocal square root good to 8 mantissa bits  */
/* sigsqrt -  square root good to 8 mantissa bits  */

#define DUMTAB1SIZE 256
#define DUMTAB2SIZE 1024
static float rsqrt_exptab[DUMTAB1SIZE], rsqrt_mantissatab[DUMTAB2SIZE];
static void init_rsqrt() {
    for (int i = 0; i < DUMTAB1SIZE; i++) {
        float f;
        long l = (i ? (i == DUMTAB1SIZE-1 ? DUMTAB1SIZE-2 : i) : 1)<< 23;
        *(int *)(&f) = l;
        rsqrt_exptab[i] = 1./sqrt(f);
    }
    for (int i = 0; i < DUMTAB2SIZE; i++) {
        float f = 1 + (1./DUMTAB2SIZE) * i;
        rsqrt_mantissatab[i] = 1./sqrt(f);
    }
}
/* these are used in externs like "bonk" */
float q8_rsqrt(float f) {
    long l = *(long *)(&f);
    if (f < 0) return 0;
    return rsqrt_exptab[(l >> 23) & 0xff] *
      rsqrt_mantissatab[(l >> 13) & 0x3ff];
}
float q8_sqrt(float f) {
    long l = *(long *)(&f);
    if (f < 0) return 0;
    return f * rsqrt_exptab[(l >> 23) & 0xff] *
          rsqrt_mantissatab[(l >> 13) & 0x3ff];
}

/* the old names are OK unless we're in IRIX N32 */
#ifndef N32
float  qsqrt(float f) {return  q8_sqrt(f);}
float qrsqrt(float f) {return q8_rsqrt(f);}
#endif
static t_class *sigrsqrt_class; struct t_sigrsqrt : t_object {float a;};
static t_class * sigsqrt_class; struct  t_sigsqrt : t_object {float a;};
static void *sigrsqrt_new() {
    t_sigrsqrt *x = (t_sigrsqrt *)pd_new(sigrsqrt_class);
    outlet_new(x, &s_signal);
    x->a = 0;
    return x;
}
static void *sigsqrt_new() {
    t_sigsqrt *x = (t_sigsqrt *)pd_new(sigsqrt_class);
    outlet_new(x, &s_signal);
    x->a = 0;
    return x;
}

static t_int *sigrsqrt_perform(t_int *w) {
    float *in = *(t_float **)(w+1), *out = *(t_float **)(w+2);
    t_int n = *(t_int *)(w+3);
    while (n--) {
        float f = *in;
        long l = *(long *)(in++);
        if (f < 0) *out++ = 0;
        else {
            float g = rsqrt_exptab[(l >> 23) & 0xff] *
                 rsqrt_mantissatab[(l >> 13) & 0x3ff];
            *out++ = 1.5 * g - 0.5 * g * g * g * f;
        }
    }
    return w+4;
}
/* not static; also used in d_fft.c */
t_int *sigsqrt_perform(t_int *w) {
    float *in = *(t_float **)(w+1), *out = *(t_float **)(w+2);
    t_int n = *(t_int *)(w+3);
    while (n--) {
        float f = *in;
        long l = *(long *)(in++);
        if (f < 0) *out++ = 0;
        else {
            float g = rsqrt_exptab[(l >> 23) & 0xff] *
                 rsqrt_mantissatab[(l >> 13) & 0x3ff];
            *out++ = f * (1.5 * g - 0.5 * g * g * g * f);
        }
    }
    return w+4;
}

static void sigrsqrt_dsp(t_sigrsqrt *x, t_signal **sp) {
    if(SIMD_CHECK2(sp[0]->n,sp[0]->v,sp[1]->v))
	 dsp_add(sigrsqrt_perf_simd, 3, sp[0]->v, sp[1]->v, sp[0]->n);
    else dsp_add(sigrsqrt_perform,   3, sp[0]->v, sp[1]->v, sp[0]->n);
}
static void sigsqrt_dsp(t_sigsqrt *x, t_signal **sp) {
    if(SIMD_CHECK2(sp[0]->n,sp[0]->v,sp[1]->v))
	 dsp_add(sigsqrt_perf_simd, 3, sp[0]->v, sp[1]->v, sp[0]->n);
    else dsp_add(sigsqrt_perform,   3, sp[0]->v, sp[1]->v, sp[0]->n);
}

void sigsqrt_setup() {
    init_rsqrt();
    sigrsqrt_class = class_new2("rsqrt~",sigrsqrt_new,0,sizeof(t_sigrsqrt),0,"");
    sigsqrt_class  = class_new2( "sqrt~", sigsqrt_new,0, sizeof(t_sigsqrt),0,"");
    CLASS_MAINSIGNALIN(sigrsqrt_class,t_sigrsqrt,a);
    CLASS_MAINSIGNALIN( sigsqrt_class, t_sigsqrt,a);
    class_addmethod2( sigsqrt_class, sigsqrt_dsp,"dsp","");
    class_addmethod2(sigrsqrt_class,sigrsqrt_dsp,"dsp","");
    class_addcreator2("q8_rsqrt~",sigrsqrt_new,"");
    class_addcreator2("q8_sqrt~",  sigsqrt_new,"");
}

/* ------------------------------ wrap~ -------------------------- */
struct t_sigwrap : t_object {float a;};
t_class *sigwrap_class;
static void *sigwrap_new() {
    t_sigwrap *x = (t_sigwrap *)pd_new(sigwrap_class);
    outlet_new(x, &s_signal);
    x->a = 0;
    return x;
}
static t_int *sigwrap_perform(t_int *w) {
    float *in = *(t_float **)(w+1), *out = *(t_float **)(w+2);
    t_int n = *(t_int *)(w+3);
    while (n--) {
        float f = *in++;
        int k = (int)f;
        if (f > 0) *out++ = f-k;
        else *out++ = f - (k-1);
    }
    return w+4;
}
static void sigwrap_dsp(t_sigwrap *x, t_signal **sp) {
    if(SIMD_CHECK2(sp[0]->n,sp[0]->v,sp[1]->v))
	 dsp_add(sigwrap_perf_simd, 3, sp[0]->v, sp[1]->v, sp[0]->n);
    else dsp_add(sigwrap_perform,   3, sp[0]->v, sp[1]->v, sp[0]->n);
}
void sigwrap_setup() {
    sigwrap_class = class_new2("wrap~",sigwrap_new,0,sizeof(t_sigwrap),0,"");
    CLASS_MAINSIGNALIN(sigwrap_class, t_sigwrap, a);
    class_addmethod2(sigwrap_class, sigwrap_dsp, "dsp","");
}

/* ------------------------------ mtof_tilde~ and such -------------------------- */
struct t_func1 : t_object {float a;};
t_class *mtof_tilde_class, *ftom_tilde_class;
t_class *dbtorms_tilde_class, *rmstodb_tilde_class;
t_class *dbtopow_tilde_class, *powtodb_tilde_class;

#define FUNC1(NAME,EXPR) \
static t_int *NAME##_perform(t_int *w) { \
    float *in = *(t_float **)(w+1), *out = *(t_float **)(w+2); \
    for (t_int n = *(t_int *)(w+3); n--; in++, out++) { float a = *in; *out = (EXPR); } \
    return w+4;} \
static void *NAME##_new() {t_func1 *x = (t_func1 *)pd_new(NAME##_class); \
    outlet_new(x,&s_signal); x->a = 0; return x;} \
static void NAME##_dsp(t_func1 *x, t_signal **sp) { \
    dsp_add(NAME##_perform, 3, sp[0]->v, sp[1]->v, sp[0]->n);}


FUNC1(mtof_tilde, a<=-1500 ? 0 : 8.17579891564 * exp(.0577622650 * min(a,1499.f)))
FUNC1(ftom_tilde, a>0 ? 17.3123405046 * log(.12231220585 * a) : -1500)
FUNC1(dbtorms_tilde, a<=0 ? 0 : exp((LOGTEN * 0.05) * (min(a,485.f)-100.)))
FUNC1(dbtopow_tilde, a<=0 ? 0 : max(100 + 20./LOGTEN * log(a),0.))
FUNC1(rmstodb_tilde, a<=0 ? 0 : exp((LOGTEN * 0.1) * (min(a,870.f)-100.)))
FUNC1(powtodb_tilde, a<=0 ? 0 : max(100 + 10./LOGTEN * log(a),0.))

#define FUNC1DECL(NAME,SYM) \
    NAME##_class = class_new2(SYM,NAME##_new,0,sizeof(t_func1),0,""); \
    CLASS_MAINSIGNALIN(NAME##_class,t_func1,a); \
    class_addmethod2(NAME##_class, NAME##_dsp, "dsp","");

void mtof_tilde_setup() {
    FUNC1DECL(mtof_tilde,"mtof~")
    FUNC1DECL(ftom_tilde,"ftom~")
    FUNC1DECL(dbtorms_tilde,"dbtorms~")
    FUNC1DECL(dbtopow_tilde,"dbtopow~")
    FUNC1DECL(rmstodb_tilde,"rmstodb~")
    FUNC1DECL(powtodb_tilde,"powtodb~")
    t_symbol *s = gensym("acoustics~.pd");
    class_sethelpsymbol(mtof_tilde_class, s);
    class_sethelpsymbol(ftom_tilde_class, s);
    class_sethelpsymbol(dbtorms_tilde_class, s);
    class_sethelpsymbol(rmstodb_tilde_class, s);
    class_sethelpsymbol(dbtopow_tilde_class, s);
    class_sethelpsymbol(powtodb_tilde_class, s);
}

static t_class *print_class;
struct t_print : t_object {
    float a;
    t_symbol *sym;
    int count;
};
static t_int *print_perform(t_int *w) {
    t_print *x = (t_print *)w[1];
    t_float *in = (t_float *)w[2];
    int n = (int)w[3];
    if (x->count) {
        post("%s:", x->sym->name);
        if (n == 1) post("%8g", in[0]);
        else if (n == 2) post("%8g %8g", in[0], in[1]);
        else if (n == 4) post("%8g %8g %8g %8g", in[0], in[1], in[2], in[3]);
        else while (n > 0) {
            post("%-8.5g %-8.5g %-8.5g %-8.5g %-8.5g %-8.5g %-8.5g %-8.5g",
                in[0], in[1], in[2], in[3], in[4], in[5], in[6], in[7]);
            n -= 8;
            in += 8;
        }
        x->count--;
    }
    return w+4;
}
static void print_dsp(t_print *x, t_signal **sp) {
    dsp_add(print_perform, 3, x, sp[0]->v, sp[0]->n);
}
static void print_float(t_print *x, t_float f) {x->count = max(0,(int)f);}
static void print_bang(t_print *x) {x->count = 1;}
static void *print_new(t_symbol *s) {
    t_print *x = (t_print *)pd_new(print_class);
    x->sym = s->name[0] ? s : gensym("print~");
    x->count = 0;
    x->a = 0;
    return x;
}
static void print_setup() {
    print_class = class_new2("print~",print_new,0,sizeof(t_print),0,"S");
    CLASS_MAINSIGNALIN(print_class, t_print, a);
    class_addmethod2(print_class, print_dsp, "dsp","");
    class_addbang(print_class, print_bang);
    class_addfloat(print_class, print_float);
}

/* ------------------------ bang~ -------------------------- */
static t_class *bang_tilde_class;
struct t_bang : t_object {
    t_clock *clock;
};
static t_int *bang_tilde_perform(t_int *w) {
    t_bang *x = (t_bang *)w[1];
    clock_delay(x->clock, 0);
    return w+2;
}
static void bang_tilde_dsp(t_bang *x, t_signal **sp) {dsp_add(bang_tilde_perform, 1, x);}
static void bang_tilde_tick(t_bang *x) {outlet_bang(x->outlet);}
static void bang_tilde_free(t_bang *x) {clock_free(x->clock);}
static void *bang_tilde_new(t_symbol *s) {
    t_bang *x = (t_bang *)pd_new(bang_tilde_class);
    x->clock = clock_new(x, bang_tilde_tick);
    outlet_new(x, &s_bang);
    return x;
}
static void bang_tilde_setup() {
    bang_tilde_class = class_new2("bang~",bang_tilde_new,bang_tilde_free,sizeof(t_bang),0,"");
    class_addmethod2(bang_tilde_class, bang_tilde_dsp, "dsp","");
}

/* -------------------------- phasor~ ------------------------------ */
static t_class *phasor_class;
/* in the style of R. Hoeldrich (ICMC 1995 Banff) */
struct t_phasor : t_object {
    double phase;
    float conv;
    float a; /* scalar frequency */
};
static void *phasor_new(t_floatarg f) {
    t_phasor *x = (t_phasor *)pd_new(phasor_class);
    x->a = f;
    inlet_new(x, x, &s_float, gensym("ft1"));
    x->phase = 0;
    x->conv = 0;
    outlet_new(x, &s_signal);
    return x;
}
static t_int *phasor_perform(t_int *w) {
    t_phasor *x = (t_phasor *)w[1];
    t_float *in = (t_float *)w[2];
    t_float *out = (t_float *)w[3];
    int n = (int)w[4];
    double dphase = x->phase + UNITBIT32;
    union tabfudge tf;
    int normhipart;
    float conv = x->conv;
    tf.d = UNITBIT32;
    normhipart = tf.i[HIOFFSET];
    tf.d = dphase;
    while (n--) {
        tf.i[HIOFFSET] = normhipart;
        dphase += *in++ * conv;
        *out++ = tf.d - UNITBIT32;
        tf.d = dphase;
    }
    tf.i[HIOFFSET] = normhipart;
    x->phase = tf.d - UNITBIT32;
    return w+5;
}
static void phasor_dsp(t_phasor *x, t_signal **sp) {
    x->conv = 1./sp[0]->sr;
    dsp_add(phasor_perform, 4, x, sp[0]->v, sp[1]->v, sp[0]->n);
}
static void phasor_ft1(t_phasor *x, t_float f) {
    x->phase = f;
}
static void phasor_setup() {
    phasor_class = class_new2("phasor~",phasor_new,0,sizeof(t_phasor),0,"F");
    CLASS_MAINSIGNALIN(phasor_class, t_phasor, a);
    class_addmethod2(phasor_class, phasor_dsp, "dsp","");
    class_addmethod2(phasor_class, phasor_ft1,"ft1","f");
}
/* </Hoeldrich-version> */

/* ------------------------ cos~ ----------------------------- */
float *cos_table;
static t_class *cos_class;
struct t_cos : t_object {
    float a;
};
static void *cos_new() {
    t_cos *x = (t_cos *)pd_new(cos_class);
    outlet_new(x,&s_signal);
    x->a = 0;
    return x;
}
static t_int *cos_perform(t_int *w) {
    t_float *in = (t_float *)w[1];
    t_float *out = (t_float *)w[2];
    int n = (int)w[3];
    float *tab = cos_table, *addr, f1, f2, frac;
    double dphase;
    int normhipart;
    union tabfudge tf;
    tf.d = UNITBIT32;
    normhipart = tf.i[HIOFFSET];

#if 0 /* this is the readable version of the code. */
    while (n--) {
        dphase = (double)(*in++ * (float)(COSTABSIZE)) + UNITBIT32;
        tf.d = dphase;
        addr = tab + (tf.i[HIOFFSET] & (COSTABSIZE-1));
        tf.i[HIOFFSET] = normhipart;
        frac = tf.d - UNITBIT32;
        f1 = addr[0];
        f2 = addr[1];
        *out++ = f1 + frac * (f2 - f1);
    }
#else /* this is the same, unwrapped by hand. */
    dphase = (double)(*in++ * (float)(COSTABSIZE)) + UNITBIT32;
    tf.d = dphase;
    addr = tab + (tf.i[HIOFFSET] & (COSTABSIZE-1));
    tf.i[HIOFFSET] = normhipart;
    while (--n) {
        dphase = (double)(*in++ * (float)(COSTABSIZE)) + UNITBIT32;
        frac = tf.d - UNITBIT32;
        tf.d = dphase;
        f1 = addr[0];
        f2 = addr[1];
        addr = tab + (tf.i[HIOFFSET] & (COSTABSIZE-1));
        *out++ = f1 + frac * (f2 - f1);
        tf.i[HIOFFSET] = normhipart;
    }
    frac = tf.d - UNITBIT32;
    f1 = addr[0];
    f2 = addr[1];
    *out++ = f1 + frac * (f2 - f1);
#endif
    return w+4;
}
static void cos_dsp(t_cos *x, t_signal **sp) {
    dsp_add(cos_perform, 3, sp[0]->v, sp[1]->v, sp[0]->n);
}
static void cos_maketable() {
    float phsinc = (2. * 3.14159) / COSTABSIZE;
    union tabfudge tf;
    if (cos_table) return;
    cos_table = (float *)getbytes(sizeof(float) * (COSTABSIZE+1));
    float phase=0;
    float *fp = cos_table;
    for (int i = COSTABSIZE + 1; i--; fp++, phase += phsinc) *fp = cos(phase);
    /* here we check at startup whether the byte alignment
       is as we declared it.  If not, the code has to be recompiled the other way. */
    tf.d = UNITBIT32 + 0.5;
    if ((unsigned)tf.i[LOWOFFSET] != 0x80000000) bug("cos~: unexpected machine alignment");
}
static void cos_setup() {
    cos_class = class_new2("cos~",cos_new,0,sizeof(t_cos),0,"F");
    CLASS_MAINSIGNALIN(cos_class, t_cos, a);
    class_addmethod2(cos_class, cos_dsp, "dsp","");
    cos_maketable();
}

/* ------------------------ osc~ ----------------------------- */
static t_class *osc_class;
struct t_osc : t_object {
    double phase;
    float conv;
    float a;      /* frequency if scalar */
};
static void *osc_new(t_floatarg f) {
    t_osc *x = (t_osc *)pd_new(osc_class);
    x->a = f;
    outlet_new(x,&s_signal);
    inlet_new(x, x, &s_float, gensym("ft1"));
    inlet_settip(x->inlet,gensym("phase"));
    x->phase = 0;
    x->conv = 0;
    return x;
}
static t_int *osc_perform(t_int *w) {
    t_osc *x = (t_osc *)w[1];
    t_float *in = (t_float *)w[2];
    t_float *out = (t_float *)w[3];
    int n = (int)w[4];
    float *tab = cos_table, *addr, f1, f2, frac;
    double dphase = x->phase + UNITBIT32;
    int normhipart;
    union tabfudge tf;
    float conv = x->conv;
    tf.d = UNITBIT32;
    normhipart = tf.i[HIOFFSET];
#if 0
    while (n--) {
        tf.d = dphase;
        dphase += *in++ * conv;
        addr = tab + (tf.i[HIOFFSET] & (COSTABSIZE-1));
        tf.i[HIOFFSET] = normhipart;
        frac = tf.d - UNITBIT32;
        f1 = addr[0];
        f2 = addr[1];
        *out++ = f1 + frac * (f2 - f1);
    }
#else
    tf.d = dphase;
    dphase += *in++ * conv;
    addr = tab + (tf.i[HIOFFSET] & (COSTABSIZE-1));
    tf.i[HIOFFSET] = normhipart;
    frac = tf.d - UNITBIT32;
    while (--n) {
        tf.d = dphase;
        f1 = addr[0];
        dphase += *in++ * conv;
        f2 = addr[1];
        addr = tab + (tf.i[HIOFFSET] & (COSTABSIZE-1));
        tf.i[HIOFFSET] = normhipart;
            *out++ = f1 + frac * (f2 - f1);
        frac = tf.d - UNITBIT32;
    }
    f1 = addr[0];
    f2 = addr[1];
    *out++ = f1 + frac * (f2 - f1);
#endif
    tf.d = UNITBIT32 * COSTABSIZE;
    normhipart = tf.i[HIOFFSET];
    tf.d = dphase + (UNITBIT32 * COSTABSIZE - UNITBIT32);
    tf.i[HIOFFSET] = normhipart;
    x->phase = tf.d - UNITBIT32 * COSTABSIZE;
    return w+5;
}
static void osc_dsp(t_osc *x, t_signal **sp) {
    x->conv = COSTABSIZE/sp[0]->sr;
    dsp_add(osc_perform, 4, x, sp[0]->v, sp[1]->v, sp[0]->n);
}
static void osc_ft1(t_osc *x, t_float f) {
    x->phase = COSTABSIZE * f;
}
static void osc_setup() {
    osc_class = class_new2("osc~",osc_new,0,sizeof(t_osc),0,"F");
    CLASS_MAINSIGNALIN(osc_class, t_osc, a);
    class_addmethod2(osc_class, osc_dsp, "dsp","");
    class_addmethod2(osc_class, osc_ft1, "ft1","f");
    class_settip(osc_class,gensym("frequency"));
    cos_maketable();
}

/* ---------------- vcf~ - 2-pole bandpass filter. ----------------- */
struct t_vcfctl {
    float re;
    float im;
    float q;
    float isr;
};
struct t_sigvcf : t_object {
    t_vcfctl cspace;
    t_vcfctl *ctl;
    float a;
};
t_class *sigvcf_class;
static void *sigvcf_new(t_floatarg q) {
    t_sigvcf *x = (t_sigvcf *)pd_new(sigvcf_class);
    inlet_new(x, x, &s_signal, &s_signal);
    inlet_new(x, x, &s_float, gensym("ft1"));
    outlet_new(x, &s_signal);
    outlet_new(x, &s_signal);
    x->ctl = &x->cspace;
    x->cspace.re = 0;
    x->cspace.im = 0;
    x->cspace.q = q;
    x->cspace.isr = 0;
    x->a = 0;
    return x;
}
static void sigvcf_ft1(t_sigvcf *x, t_floatarg f) {
    x->ctl->q = (f > 0 ? f : 0.f);
}
static t_int *sigvcf_perform(t_int *w) {
    float *in1 = (float *)w[1];
    float *in2 = (float *)w[2];
    float *out1 = (float *)w[3];
    float *out2 = (float *)w[4];
    t_vcfctl *c = (t_vcfctl *)w[5];
    int n = (t_int)w[6];
    float re = c->re, re2;
    float im = c->im;
    float q = c->q;
    float qinv = (q > 0? 1.0f/q : 0);
    float ampcorrect = 2.0f - 2.0f / (q + 2.0f);
    float isr = c->isr;
    float coefr, coefi;
    float *tab = cos_table, *addr, f1, f2, frac;
    double dphase;
    int normhipart, tabindex;
    union tabfudge tf;
    tf.d = UNITBIT32;
    normhipart = tf.i[HIOFFSET];
    for (int i = 0; i < n; i++) {
        float cf, cfindx, r, oneminusr;
        cf = *in2++ * isr;
        if (cf < 0) cf = 0;
        cfindx = cf * (float)(COSTABSIZE/6.28318f);
        r = (qinv > 0 ? 1 - cf * qinv : 0);
        if (r < 0) r = 0;
        oneminusr = 1.0f - r;
        dphase = ((double)(cfindx)) + UNITBIT32;
        tf.d = dphase;
        tabindex = tf.i[HIOFFSET] & (COSTABSIZE-1);
        addr = tab + tabindex;
        tf.i[HIOFFSET] = normhipart;
        frac = tf.d - UNITBIT32;
        f1 = addr[0]; f2 = addr[1]; coefr = r * (f1 + frac * (f2 - f1));
        addr = tab + ((tabindex - (COSTABSIZE/4)) & (COSTABSIZE-1));
        f1 = addr[0]; f2 = addr[1]; coefi = r * (f1 + frac * (f2 - f1));
        f1 = *in1++;
        re2 = re;
        *out1++ = re = ampcorrect * oneminusr * f1 + coefr * re2 - coefi * im;
        *out2++ = im = coefi * re2 + coefr * im;
    }
    if (PD_BIGORSMALL(re)) re = 0;
    if (PD_BIGORSMALL(im)) im = 0;
    c->re = re;
    c->im = im;
    return w+7;
}
static void sigvcf_dsp(t_sigvcf *x, t_signal **sp) {
    x->ctl->isr = 6.28318f/sp[0]->sr;
    dsp_add(sigvcf_perform, 6, sp[0]->v, sp[1]->v, sp[2]->v, sp[3]->v, x->ctl, sp[0]->n);

}
void sigvcf_setup() {
    sigvcf_class = class_new2("vcf~",sigvcf_new,0,sizeof(t_sigvcf),0,"F");
    CLASS_MAINSIGNALIN(sigvcf_class, t_sigvcf, a);
    class_addmethod2(sigvcf_class, sigvcf_dsp, "dsp","");
    class_addmethod2(sigvcf_class, sigvcf_ft1, "ft1","f");
}

/* -------------------------- noise~ ------------------------------ */
static t_class *noise_class;
struct t_noise : t_object {
    int val;
};
static void *noise_new() {
    t_noise *x = (t_noise *)pd_new(noise_class);
    static int init = 307;
    x->val = (init *= 1319);
    outlet_new(x, &s_signal);
    return x;
}
static t_int *noise_perform(t_int *w) {
    t_float *out = (t_float *)w[1];
    int *vp = (int *)w[2];
    int n = (int)w[3];
    int val = *vp;
    while (n--) {
        *out++ = ((float)((val & 0x7fffffff) - 0x40000000)) * (float)(1.0 / 0x40000000);
        val = val * 435898247 + 382842987;
    }
    *vp = val;
    return w+4;
}
static void noise_dsp(t_noise *x, t_signal **sp) {
    dsp_add(noise_perform, 3, sp[0]->v, &x->val, sp[0]->n);
}
static void noise_setup() {
    noise_class = class_new2("noise~",noise_new,0,sizeof(t_noise),0,"");
    class_addmethod2(noise_class, noise_dsp, "dsp","");
}
void builtins_dsp_setup() {
    plus_setup();
    minus_setup();
    times_setup();
    over_setup();
    max_setup();
    min_setup();
    lt_setup();
    gt_setup();
    le_setup();
    ge_setup();
    eq_setup();
    ne_setup();
    //abs_setup();

    tab_tilde_setup();
    tabosc4_tilde_setup();
    tabsend_setup();
    tabreceive_setup();
    tabread_setup();
    tabread4_setup();
    tabwrite_setup();

    sig_tilde_setup();
     line_tilde_setup();  snapshot_tilde_setup();
    vline_tilde_setup(); vsnapshot_tilde_setup();
    env_tilde_setup();
    threshold_tilde_setup();

    dac_setup();
    adc_setup();

    sigdelwrite_setup();
    sigdelread_setup();
    sigvd_setup();

    sigframp_setup();
#ifdef HAVE_LIBFFTW3F
    sigfftw_setup();     /* added by Tim Blechmann to support fftw */
#else
    sigfft_setup();
#endif /* HAVE_LIBFFTW3F */

    sighip_setup();
    siglop_setup();
    sigbp_setup();
    sigbiquad_setup();
    sigsamphold_setup();
    sigr_setup();
    sigc_setup();

    sigsend_setup();
    sigreceive_setup();
    sigcatch_setup();
    sigthrow_setup();

    clip_setup();
    sigsqrt_setup();
    sigwrap_setup();
    mtof_tilde_setup();
    print_setup();
    bang_tilde_setup();

    phasor_setup();
    cos_setup();
    osc_setup();
    sigvcf_setup();
    noise_setup();
}
