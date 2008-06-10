// GPL
// most of this code comes from pd. just the interpolation shematic is diferent.
// and the modulo for the position is made for all sample, so you do'nt need to copy the last 3 point in the beggining


#include "m_pd.h"

/******************** tabosc4cloop~ ***********************/

/* this is all copied from d_osc.c... what include file could this go in? */
#define UNITBIT32 1572864.  /* 3*2^19; bit 32 has place value 1 */

    /* machine-dependent definitions.  These ifdefs really
    should have been by CPU type and not by operating system! */
#ifdef IRIX
    /* big-endian.  Most significant byte is at low address in memory */
#define HIOFFSET 0    /* word offset to find MSB */
#define LOWOFFSET 1    /* word offset to find LSB */
#define int32 long  /* a data type that has 32 bits */
#endif /* IRIX */

#ifdef MSW
    /* little-endian; most significant byte is at highest address */
#define HIOFFSET 1
#define LOWOFFSET 0
#define int32 long
#endif

#if defined(__FreeBSD__) || defined(__APPLE__)
#include <machine/endian.h>
#endif

#ifdef __linux__
#include <endian.h>
#endif

#if defined(__unix__) || defined(__APPLE__)
#if !defined(BYTE_ORDER) || !defined(LITTLE_ENDIAN)                         
#error No byte order defined                                                    
#endif                                                                          

#if BYTE_ORDER == LITTLE_ENDIAN                                             
#define HIOFFSET 1                                                              
#define LOWOFFSET 0                                                             
#else                                                                           
#define HIOFFSET 0    /* word offset to find MSB */                             
#define LOWOFFSET 1    /* word offset to find LSB */                            
#endif /* __BYTE_ORDER */                                                       
#include <sys/types.h>
#define int32 int32_t
#endif /* __unix__ or __APPLE__*/

union tabfudge
{
    double tf_d;
    int32 tf_i[2];
};

static t_class *tabosc4cloop_tilde_class;

typedef struct _tabosc4cloop_tilde
{
    t_object x_obj;
    t_float x_fnpoints;
    t_float x_finvnpoints;
    t_word *x_vec;
    t_symbol *x_arrayname;
    t_float x_f;
    double x_phase;
    t_float x_conv;
} t_tabosc4cloop_tilde;

static void *tabosc4cloop_tilde_new(t_symbol *s)
{
    t_tabosc4cloop_tilde *x = (t_tabosc4cloop_tilde *)pd_new(tabosc4cloop_tilde_class);
    x->x_arrayname = s;
    x->x_vec = 0;
    x->x_fnpoints = 512.;
    x->x_finvnpoints = (1./512.);
    outlet_new(&x->x_obj, gensym("signal"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
    x->x_f = 0;
    return (x);
}

static t_int *tabosc4cloop_tilde_perform(t_int *w)
{
    t_tabosc4cloop_tilde *x = (t_tabosc4cloop_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);
    int normhipart;
    union tabfudge tf;
    double a0,a1,a2,a3; // CH
    t_float fnpoints = x->x_fnpoints;
    int mask = fnpoints -1;
    t_float conv = fnpoints * x->x_conv;
    int maxindex;
    t_word *tab = x->x_vec, *addr;
    int i;
    double dphase = fnpoints * x->x_phase + UNITBIT32;

    if (!tab) goto zero;
    tf.tf_d = UNITBIT32;
    normhipart = tf.tf_i[HIOFFSET];

#if 1
    while (n--)
    {
        t_sample frac,  a,  b,  c,  d, cminusb;
        tf.tf_d = dphase;
        dphase += *in++ * conv;
        addr = tab + (tf.tf_i[HIOFFSET] & mask); 
// thx to dh
        a = addr[0].w_float;
        addr = tab + ((1+tf.tf_i[HIOFFSET]) & mask);
        b = addr[0].w_float;
        addr = tab + ((2+tf.tf_i[HIOFFSET]) & mask);
        c = addr[0].w_float;
        addr = tab + ((3+tf.tf_i[HIOFFSET]) & mask);
        d = addr[0].w_float;
        tf.tf_i[HIOFFSET] = normhipart;
        frac = tf.tf_d - UNITBIT32;
// CH
	a0 = d - c - a + b;
	a1 = a - b - a0;
	a2 = c - a;
	a3 = b;
   *out++ = ((a0*frac+a1)*frac+a2)*frac+a3;

    }
#endif

    tf.tf_d = UNITBIT32 * fnpoints;
    normhipart = tf.tf_i[HIOFFSET];
    tf.tf_d = dphase + (UNITBIT32 * fnpoints - UNITBIT32);
    tf.tf_i[HIOFFSET] = normhipart;
    x->x_phase = (tf.tf_d - UNITBIT32 * fnpoints)  * x->x_finvnpoints;
    return (w+5);
 zero:
    while (n--) *out++ = 0;

    return (w+5);
}

void tabosc4cloop_tilde_set(t_tabosc4cloop_tilde *x, t_symbol *s)
{
    t_garray *a;
    int npoints, pointsinarray;

    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name)
            pd_error(x, "tabosc4cloop~: %s: no such array", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getfloatwords(a, &pointsinarray, &x->x_vec))
    {
        pd_error(x, "%s: bad template for tabosc4cloop~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (pointsinarray != (1 << ilog2(pointsinarray)))
    {
        pd_error(x, "%s: number of points (%d) not a power of 2",
        x->x_arrayname->s_name, pointsinarray);
        x->x_vec = 0;
        garray_usedindsp(a);
    }
	else
    {
        x->x_fnpoints = pointsinarray;
        x->x_finvnpoints = 1./pointsinarray;
        garray_usedindsp(a);
    }
}

static void tabosc4cloop_tilde_ft1(t_tabosc4cloop_tilde *x, t_float f)
{
    x->x_phase = f;
}

static void tabosc4cloop_tilde_dsp(t_tabosc4cloop_tilde *x, t_signal **sp)
{
    x->x_conv = 1. / sp[0]->s_sr;
    tabosc4cloop_tilde_set(x, x->x_arrayname);

    dsp_add(tabosc4cloop_tilde_perform, 4, x,
        sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void tabosc4cloop_tilde_setup(void)
{
    tabosc4cloop_tilde_class = class_new(gensym("tabosc4cloop~"),
        (t_newmethod)tabosc4cloop_tilde_new, 0,
        sizeof(t_tabosc4cloop_tilde), 0, A_DEFSYM, 0);
    CLASS_MAINSIGNALIN(tabosc4cloop_tilde_class, t_tabosc4cloop_tilde, x_f);
    class_addmethod(tabosc4cloop_tilde_class, (t_method)tabosc4cloop_tilde_dsp,
        gensym("dsp"), 0);
    class_addmethod(tabosc4cloop_tilde_class, (t_method)tabosc4cloop_tilde_set,
        gensym("set"), A_SYMBOL, 0);
    class_addmethod(tabosc4cloop_tilde_class, (t_method)tabosc4cloop_tilde_ft1,
        gensym("ft1"), A_FLOAT, 0);
}

