#include "m_pd.h"
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ dspobj~ ----------------------------- */

/* tilde object to take absolute value. */

static t_class *dspobj_class;

typedef struct _dspobj
{
    t_object x_obj;
} t_dspobj;

static t_int *dspobj_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    while (n--)
    {
    	float f = *(in++);
	*out++ = (f > 0 ? f : -f);
    }
    return (w+4);
}

static void dspobj_dsp(t_dspobj *x, t_signal **sp)
{
    dsp_add(dspobj_perform, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void *dspobj_new(void)
{
    t_dspobj *x = (t_dspobj *)pd_new(dspobj_class);
    outlet_new(&x->x_obj, gensym("signal"));
    return (x);
}

void dspobj_tilde_setup(void)
{
    dspobj_class = class_new(gensym("dspobj~"), (t_newmethod)dspobj_new, 0,
    	sizeof(t_dspobj), 0, A_DEFFLOAT, 0);
    class_addmethod(dspobj_class, nullfn, gensym("signal"), 0);
    class_addmethod(dspobj_class, (t_method)dspobj_dsp, gensym("dsp"), 0);
}
