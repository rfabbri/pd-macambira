// tabreadi~
// can replace tabread4~
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

#define max(a,b) ( ((a) > (b)) ? (a) : (b) ) 
#define min(a,b) ( ((a) < (b)) ? (a) : (b) )

/******************** tabreadi~ ***********************/

static t_class *tabreadi_tilde_class;

typedef struct _tabreadi_tilde
{
    t_object x_obj;
    int x_npoints;
    t_word *x_vec;
    t_symbol *x_arrayname;
    t_float x_f;
    t_sample x_prev_in, x_last_in, x_prev_out, x_last_out;
    t_float x_fa1, x_fa2, x_fb1, x_fb2, x_fb3; 
    t_float cutoff;
    t_int upsample;
    t_float x_sr;
} t_tabreadi_tilde;

void tabreadi_tilde_cutoff(t_tabreadi_tilde *x, t_float cut)
{
    x->cutoff = cut;

    if (x->cutoff == 0)
    {
        x->x_fb1 = 1;
        x->x_fb2 = 0;
        x->x_fb3 = 0;
        x->x_fa1 = 0;
        x->x_fa2 = 0;

        x->x_prev_in = 0;
        x->x_last_in = 0;
        x->x_prev_out = 0; // reset filter memory
    }
    else
    {
        // filter coef to cut all high freq.
        t_float tmp1, tmp2;

        tmp1 = sqrt(2)/2;
        tmp1 = sinh(tmp1);

        tmp2 = x->cutoff * 2 * 3.1415926 / (x->upsample * x->x_sr);
	tmp2 = min(6.28,tmp2);

        tmp1 *= sin(tmp2);
        tmp2 = cos(tmp2);

        x->x_fb1 = (1-tmp2 ) /2;
        x->x_fb2 = (1-tmp2 );
        x->x_fb3 = (1-tmp2 ) /2;
        x->x_fa1 = -2 * tmp2;
        x->x_fa2 = 1 - tmp1;
	
        tmp1 +=1;

        x->x_fb1 /= tmp1;
        x->x_fb2 /= tmp1;
        x->x_fb3 /= tmp1;
        x->x_fa1 /= tmp1;
        x->x_fa2 /= tmp1;
    }
}

static void *tabreadi_tilde_new(t_symbol *s)
{
    t_tabreadi_tilde *x = (t_tabreadi_tilde *)pd_new(tabreadi_tilde_class);
    x->x_arrayname = s;
    x->x_vec = 0;
    outlet_new(&x->x_obj, gensym("signal"));
    x->x_f = 0;
    x->cutoff = 0;
    x->upsample = 1;
    x->x_sr = 44100;
    tabreadi_tilde_cutoff(x,0); // comput filter coef   return (x);
}

static t_int *tabreadi_tilde_perform(t_int *w)
{
    t_tabreadi_tilde *x = (t_tabreadi_tilde *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    t_sample *out = (t_sample *)(w[3]);
    int n = (int)(w[4]);    
    int maxindex;
    t_word *buf = x->x_vec, *wp;
    int i;
    double a3,a1,a2; // CH
    
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
    	    for (i=0;i<x->upsample;i++)
    	    {	
            t_sample findex = *in++; // ??? comment limiter ca pour faire un band limited????
            int index = findex;
            t_sample frac,  a,  b,  c,  d, cminusb;
            static int count;
            if (index < 1)
                index = 1, frac = 0;
            else if (index > maxindex)
                index = maxindex, frac = 1;
            else frac = findex - index;
            wp = buf + index;
            a = wp[-1].w_float;
            b = wp[0].w_float;
            c = wp[1].w_float;
            d = wp[2].w_float;
        /* if (!i && !(count++ & 1023))
            post("fp = %lx,  shit = %lx,  b = %f",  fp, buf->b_shit,  b); */
//        cminusb = c-b;
//        *out++ = b + frac * (
//            cminusb - 0.1666667f * (1.-frac) * (
//                (d - a - 3.0f * cminusb) * frac + (d + 2.0f*a - 3.0f*b)
// CH
	// 4-point, 3rd-order Hermite (x-form)
	    a1 = 0.5f * (c - a);
	    a2 = a - 2.5 * b + 2.f * c - 0.5f * d;
	    a3 = 0.5f * (d - a) + 1.5f * (b - c);


// TODO

        }

	*out++ =  ((a3 * frac + a2) * frac + a1) * frac + b;     
    }
    return (w+5);
 zero:
    while (n--) *out++ = 0;

    return (w+5);
}

void tabreadi_tilde_set(t_tabreadi_tilde *x, t_symbol *s)
{
    t_garray *a;
    
    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name)
            pd_error(x, "tabreadi~: %s: no such array", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getfloatwords(a, &x->x_npoints, &x->x_vec))
    {
        pd_error(x, "%s: bad template for tabreadi~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_usedindsp(a);
}


void tabreadi_tilde_upsample(t_tabreadi_tilde *x, t_float up)
{
    x->upsample = max(1,up);
    tabreadi_tilde_cutoff(x,x->cutoff);
}


static void tabreadi_tilde_dsp(t_tabreadi_tilde *x, t_signal **sp)
{
   if (x->x_sr != sp[0]->s_sr)
    {
        x->x_sr = sp[0]->s_sr;
        tabreadi_tilde_cutoff(x,x->cutoff);
    }

    tabreadi_tilde_set(x, x->x_arrayname);

    dsp_add(tabreadi_tilde_perform, 4, x,
        sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

static void tabreadi_tilde_free(t_tabreadi_tilde *x)
{
}

void tabreadi_tilde_setup(void)
{
    tabreadi_tilde_class = class_new(gensym("tabreadi~"),
        (t_newmethod)tabreadi_tilde_new, (t_method)tabreadi_tilde_free,
        sizeof(t_tabreadi_tilde), 0, A_DEFSYM, 0);
    CLASS_MAINSIGNALIN(tabreadi_tilde_class, t_tabreadi_tilde, x_f);
    class_addmethod(tabreadi_tilde_class, (t_method)tabreadi_tilde_dsp,
        gensym("dsp"), 0);
    class_addmethod(tabreadi_tilde_class, (t_method)tabreadi_tilde_set,
        gensym("set"), A_SYMBOL, 0);
}
