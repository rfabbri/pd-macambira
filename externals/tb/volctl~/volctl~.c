/* Copyright (c) 2004 Tim Blechmann.
 *For information on usage and redistribution, and for a DISCLAIMER OF ALL
 *WARRANTIES, see the file, "gpl.txt" in this distribution.
 *
 *This program is free software; you can redistribute it and/or
 *modify it under the terms of the GNU General Public License
 *as published by the Free Software Foundation; either version 2
 *of the License, or (at your option) any later version.
 *
 *See file LICENSE for further informations on licensing terms.
 *
 *This program is distributed in the hope that it will be useful,
 *but WITHOUT ANY WARRANTY; without even the implied warranty of
 *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *GNU General Public License for more details.
 *
 *You should have received a copy of the GNU General Public License
 *along with this program; if not, write to the Free Software
 *Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *Based on PureData by Miller Puckette and others.
 *
 *  coded while listening to: Julien Ottavi: Nervure Magnetique
 *                                                                    */


#include "m_pd.h"

#include "m_simd.h"


/* ----------------------------- volctl ----------------------------- */

static t_class *volctl_class;

typedef struct _volctl
{
    t_object x_obj;
    float x_f;

    float x_h; //interpolation time
    float x_value; //current factor
    
    int x_ticksleft; //ticks to go
    float x_mspersample; //ms per sample
    float x_slope; //slope

    int x_line; 

} t_volctl;

void *volctl_new(t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 2) post("volctl~: extra arguments ignored");

    t_volctl *x = (t_volctl *)pd_new(volctl_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("f1"));
    inlet_settip(x->x_obj.ob_inlet,gensym("factor"));
    x->x_value = atom_getfloatarg(0, argc, argv);
    
    t_inlet * time = floatinlet_new(&x->x_obj, &x->x_h);
    inlet_settip(time,gensym("interpolation_time"));
    x->x_h = atom_getfloatarg(1, argc, argv);

    x->x_mspersample = 1000.f / 44100; // assume default samplerate

    outlet_new(&x->x_obj, &s_signal);
    x->x_f = 0;
    return (x);
}

t_int *volctl_perform(t_int *w)
{
    t_volctl * x = (t_volctl *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    
    float f = x->x_value;

    if (x->x_ticksleft)
    {
	float x_slope = x->x_slope;
	if (x->x_ticksleft < n)
	{
	    int remain = x->x_ticksleft;
	    n-=remain;
	    while (remain--)
	    {
		f+=x_slope;
		*out++ = *in++ * f;
	    }
	    while (n--)
	    {
		*out++ = *in++ * f;
	    }
	    x->x_value = f;
	    x->x_ticksleft = 0;
	}
	else
	{
	    x->x_ticksleft -=n;
	    while (n--)
	    {
		f+=x_slope;
		*out++ = *in++ * f;
	    }
	    x->x_value = f;
	}
    }
    else
	while (n--) *out++ = *in++ * f; 
	
    return (w+5);
}
    

t_int *volctl_perf8(t_int *w)
{
    t_volctl * x = (t_volctl *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);

    float f = x->x_value;

    if (x->x_ticksleft)
    {
	float x_slope = x->x_slope;
	if (x->x_ticksleft < n)
	{
	    int remain = x->x_ticksleft;
	    n-=remain;
	    while (remain--)
	    {
		*out++ = *in++ * f;
		f+=x_slope;
	    }
	    while (n--)
	    {
		*out++ = *in++ * f;
	    }
	    x->x_value = f;
	    x->x_ticksleft = 0;
	}
	else
	{
	    x->x_ticksleft -= n;
	    while (n--)
	    {
		*out++ = *in++ * f;
		f+=x_slope;
	    }
	    x->x_value = f;
	}
    }
    else
    {
	for (; n; n -= 8, in += 8, out += 8)
	{
	    float f0 = in[0], f1 = in[1], f2 = in[2], f3 = in[3];
	    float f4 = in[4], f5 = in[5], f6 = in[6], f7 = in[7];
	    
	    out[0] = f0 * f; out[1] = f1 * f; out[2] = f2 * f; out[3] = f3 * f;
	    out[4] = f4 * f; out[5] = f5 * f; out[6] = f6 * f; out[7] = f7 * f;
	}
    }
    return (w+5);
}

t_int *volctl_perf_simd(t_int *w)
{
    t_volctl * x = (t_volctl *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);

    if (x->x_ticksleft)
    {
	int n = (int)(w[4]);
	
	float f = x->x_value;

	float x_slope = x->x_slope;
	if (x->x_ticksleft < n)
	{
	    int remain = x->x_ticksleft;
	    n-=remain;
	    while (remain--)
	    {
		*out++ = *in++ * f;
		f+=x_slope;
	    }
	    while (n--)
	    {
		*out++ = *in++ * f;
	    }
	    x->x_value = f;
	    x->x_ticksleft = 0;
	}
	else
	{
	    x->x_ticksleft -= n;
	    while (n--)
	    {
		*out++ = *in++ * f;
		f+=x_slope;
	    }
	    x->x_value = f;
	}
    }
    else
    {
	asm(
	    ".set T_FLOAT,4                            \n"

	    "movss     (%3), %%xmm0                   \n"
	    "shufps    $0, %%xmm0, %%xmm0                \n"
	    "shrl      $4, %2                        \n"
  
	    "volctl_loop:                              \n"
	    "movaps    (%0), %%xmm1                   \n"
	    "mulps     %%xmm0, %%xmm1                    \n"
	    "movaps    %%xmm1, (%1)                   \n" 
	    "movaps    4*T_FLOAT(%0), %%xmm2          \n"
	    "mulps     %%xmm0, %%xmm2                    \n"
	    "movaps    %%xmm2, 4*T_FLOAT(%1)          \n"
	    "movaps    8*T_FLOAT(%0), %%xmm3          \n"
	    "mulps     %%xmm0, %%xmm3                    \n"
	    "movaps    %%xmm3, 8*T_FLOAT(%1)          \n"
	    "movaps    12*T_FLOAT(%0), %%xmm4         \n"
	    "mulps     %%xmm0, %%xmm4                    \n"
	    "movaps    %%xmm4, 12*T_FLOAT(%1)         \n"
	    "addl      $64, %0                       \n"
	    "addl      $64, %1                       \n"
	    "loop      volctl_loop                     \n"
	    :
	    : "r"(in), "r"(out),
	    "a"(w[4]),"r"(&(x->x_value))
	    : "%xmm0","%xmm1","%xmm2","%xmm3","%xmm4");
    }
    return (w+5);
}


void volctl_set(t_volctl *x, t_float f)
{
    x->x_ticksleft = x->x_h / x->x_mspersample;
    x->x_slope = (f - x->x_value) / x->x_ticksleft;
}

void volctl_dsp(t_volctl *x, t_signal **sp)
{
    const int n = sp[0]->s_n;
    if (n&7)
    	dsp_add(volctl_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, n);
    else 
    {
	if(SIMD_CHECK2(n,sp[0]->s_vec,sp[1]->s_vec))
	    dsp_add(volctl_perf_simd, 4, x, sp[0]->s_vec, sp[1]->s_vec, n);
	else
	dsp_add(volctl_perf8, 4, x, sp[0]->s_vec, sp[1]->s_vec, n);
    }
    x->x_mspersample = 1000.f / sp[0]->s_sr;
}

void volctl_tilde_setup(void)
{
    volctl_class = class_new(gensym("volctl~"), (t_newmethod)volctl_new, 0,
			     sizeof(t_volctl), 0, A_GIMME, 0);
    CLASS_MAINSIGNALIN(volctl_class, t_volctl, x_f);
    class_addmethod(volctl_class, (t_method)volctl_dsp, gensym("dsp"), 0);
    class_addmethod(volctl_class, (t_method)volctl_set, gensym("f1"),A_FLOAT,0);
    class_settip(volctl_class,gensym("signal"));
}
