// tabread6c~
// most of this code comes from pd. just the interpolation shematic is diferent.


/* 
This software is copyrighted by Miller Puckette and others.  The following
terms (the "Standard Improved BSD License") apply to all files associated with
the software unless explicitly disclaimed in individual files:

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above  
   copyright notice, this list of conditions and the following 
   disclaimer in the documentation and/or other materials provided
   with the distribution.
3. The name of the author may not be used to endorse or promote
   products derived from this software without specific prior 
   written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,   
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
*/

// Cyrille Henry 06 2008

#include "m_pd.h"

/******************** tabread6c~ ***********************/

static t_class *tabread6c_tilde_class;

typedef struct _tabread6c_tilde
{
    t_object x_obj;
    int x_npoints;
    t_word *x_vec;
    t_symbol *x_arrayname;
    t_float x_f;
} t_tabread6c_tilde;

static void *tabread6c_tilde_new(t_symbol *s)
{
    t_tabread6c_tilde *x = (t_tabread6c_tilde *)pd_new(tabread6c_tilde_class);
    x->x_arrayname = s;
    x->x_vec = 0;
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    return (x);
}

static t_int *tabread6c_tilde_perform(t_int *w)
{
    t_tabread6c_tilde *x = (t_tabread6c_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);    
    int maxindex;
    t_word *buf = x->x_vec, *wp;
    int i;
    double a0,a1,a2,a3,a4,a5; // CH
    
    maxindex = x->x_npoints - 3;

    if (!buf) goto zero;

#if 0       /* test for spam -- I'm not ready to deal with this */
    for (i = 0,  xmax = 0, xmin = maxindex,  fp = in1; i < n; i++,  fp++)
    {
        t_sample f = *in1;
        if (f < xmin) xmin = f;
        else if (f > xmax) xmax = f;
    }
    if (xmax < xmin + x->c_maxextent) xmax = xmin + x->c_maxextent;
    for (i = 0, splitlo = xmin+ x->c_maxextent, splithi = xmax - x->c_maxextent,
        fp = in1; i < n; i++,  fp++)
    {
        t_sample f = *in1;
        if (f > splitlo && f < splithi) goto zero;
    }
#endif

    for (i = 0; i < n; i++)
    {
        t_sample findex = *in++;
        int index = findex;
        t_sample frac, a, b, c, d, e, f, cminusb;
        static int count;
        if (index < 1)
            index = 1, frac = 0;
        else if (index > maxindex)
            index = maxindex, frac = 1;
        else frac = findex - index;
        wp = buf + index;
        a = wp[-2].w_float;
        b = wp[-1].w_float;
        c = wp[0].w_float;
        d = wp[1].w_float;
		e = wp[2].w_float;
		f = wp[3].w_float;
// CH

/*	a0 = c;
	a1 = ( 1./12.)*a - ( 2./3. )*b               + ( 2./3. )*d - ( 1./12.)*e;
	a2 = (-1./24.)*a + ( 2./3. )*b - ( 5./4. )*c + ( 2./3. )*d - ( 1./24.)*e;
	a3 = (-3./8. )*a + (13./8. )*b - (35./12.)*c + (11./4. )*d - (11./8. )*e + ( 7./24.)*f;
	a4 = (13./24.)*a - ( 8./3. )*b + (21./4. )*c - (31./6. )*d + (61./24.)*e - ( 1./2. )*f;
	a5 = (-5./24.)*a + (25./24.)*b - (25./12.)*c + (25./12.)*d - (25./24.)*e + ( 5./24.)*f;

	*out++ =  ((((a5 * frac + a4 ) * frac + a3) * frac + a2) * frac + a1) * frac + a0;
*/
// same but optimized :

	t_sample a3plusa4plusa5 = 0.25f*c+0.125f*e-(1./3.)*d-(1./24.)*a;
	t_sample fminusa = f-a;
	t_sample eminusb = e-b;
	t_sample dminusc = d-c;

	a5 = (5./24.)*((fminusa-5.f*eminusb+10.f*dminusc));
	a4 = (8./3.)*eminusb-0.5f*fminusa-5.5f*dminusc-a3plusa4plusa5;
	a3 = a3plusa4plusa5-a4-a5;
	a2 = (2./3.)*(d+b)-(1./24.)*(a+e)-1.25f*c;
	a1 = (2./3.)*(d-b)+(1./12.)*(a-e);
	a0 = c;

	*out++ =  ((((a5 * frac + a4 ) * frac + a3) * frac + a2) * frac + a1)
* frac + a0;
     
    }
    return (w+5);
 zero:
    while (n--) *out++ = 0;

    return (w+5);
}

void tabread6c_tilde_set(t_tabread6c_tilde *x, t_symbol *s)
{
    t_garray *a;
    
    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name)
            pd_error(x, "tabread6c~: %s: no such array", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getfloatwords(a, &x->x_npoints, &x->x_vec))
    {
        pd_error(x, "%s: bad template for tabread6c~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_usedindsp(a);
}

static void tabread6c_tilde_dsp(t_tabread6c_tilde *x, t_signal **sp)
{
    tabread6c_tilde_set(x, x->x_arrayname);

    dsp_add(tabread6c_tilde_perform, 4, x,
        sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

static void tabread6c_tilde_free(t_tabread6c_tilde *x)
{
}

void tabread6c_tilde_setup(void)
{
    tabread6c_tilde_class = class_new(gensym("tabread6c~"),
        (t_newmethod)tabread6c_tilde_new, (t_method)tabread6c_tilde_free,
        sizeof(t_tabread6c_tilde), 0, A_DEFSYM, 0);
    CLASS_MAINSIGNALIN(tabread6c_tilde_class, t_tabread6c_tilde, x_f);
    class_addmethod(tabread6c_tilde_class, (t_method)tabread6c_tilde_dsp,
        gensym("dsp"), 0);
    class_addmethod(tabread6c_tilde_class, (t_method)tabread6c_tilde_set,
        gensym("set"), A_SYMBOL, 0);
}
