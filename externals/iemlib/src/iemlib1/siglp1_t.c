/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2003 */

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


#include "m_pd.h"
#include "iemlib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>


/* -- lp1_t~ - slow dynamic lowpass-filter 1. order with tau input --- */

typedef struct siglp1_t
{
	t_object x_obj;
	float	yn1;
	float	c0;
	float	c1;
	float	sr;
	float	cur_t;
	float	delta_t;
	float	end_t;
	float	ticks_per_interpol_time;
	float	rcp_ticks;
	float	interpol_time;
	int	  ticks;
	int	  counter_t;
	float	x_msi;
} t_siglp1_t;

t_class *siglp1_t_class;

static void siglp1_t_dsp_tick(t_siglp1_t *x)
{
	if(x->counter_t)
	{
		if(x->counter_t <= 1)
		{
			x->cur_t = x->end_t;
			x->counter_t = 0;
		}
		else
		{
			x->counter_t--;
			x->cur_t += x->delta_t;
		}
		if(x->cur_t == 0.0f)
			x->c1 = 0.0f;
		else
			x->c1 = exp((x->sr)/x->cur_t);
		x->c0 = 1.0f - x->c1;
	}
}

static t_int *siglp1_t_perform(t_int *w)
{
	float *in = (float *)(w[1]);
	float *out = (float *)(w[2]);
	t_siglp1_t *x = (t_siglp1_t *)(w[3]);
	int i, n = (t_int)(w[4]);
	float yn0, yn1=x->yn1;
	float c0=x->c0, c1=x->c1;

	siglp1_t_dsp_tick(x);
	for(i=0; i<n; i++)
	{
		yn0 = (*in++)*c0 + yn1*c1;
		*out++ = yn0;
		yn1 = yn0;
	}
	/* NAN protect */
	if(IEM_DENORMAL(yn1))
		yn1 = 0.0f;
	x->yn1 = yn1;
	return(w+5);
}

static t_int *siglp1_t_perf8(t_int *w)
{
	float *in = (float *)(w[1]);
	float *out = (float *)(w[2]);
	t_siglp1_t *x = (t_siglp1_t *)(w[3]);
	int i, n = (t_int)(w[4]);
	float yn[9];
	float c0=x->c0, c1=x->c1;

	siglp1_t_dsp_tick(x);
	yn[0] = x->yn1;
	for(i=0; i<n; i+=8, in+=8, out+=8)
	{
		yn[1] = in[0]*c0 + yn[0]*c1;
		out[0] = yn[1];
		yn[2] = in[1]*c0 + yn[1]*c1;
		out[1] = yn[2];
		yn[3] = in[2]*c0 + yn[2]*c1;
		out[2] = yn[3];
		yn[4] = in[3]*c0 + yn[3]*c1;
		out[3] = yn[4];
		yn[5] = in[4]*c0 + yn[4]*c1;
		out[4] = yn[5];
		yn[6] = in[5]*c0 + yn[5]*c1;
		out[5] = yn[6];
		yn[7] = in[6]*c0 + yn[6]*c1;
		out[6] = yn[7];
		yn[8] = in[7]*c0 + yn[7]*c1;
		out[7] = yn[8];
		yn[0] = yn[8];
	}
	/* NAN protect */
	if(IEM_DENORMAL(yn[0]))
		yn[0] = 0.0f;

	x->yn1 = yn[0];
	return(w+5);
}

static void siglp1_t_ft2(t_siglp1_t *x, t_floatarg t)
{
	int i = (int)((x->ticks_per_interpol_time)*t);

	x->interpol_time = t;
	if(i <= 0)
		i = 1;
	x->ticks = i;
	x->rcp_ticks = 1.0f / (float)i;
}

static void siglp1_t_ft1(t_siglp1_t *x, t_floatarg time_const)
{
	if(time_const < 0.0f)
		time_const = 0.0f;
	if(time_const != x->cur_t)
	{
		x->end_t = time_const;
		x->counter_t = x->ticks;
		x->delta_t = (time_const - x->cur_t) * x->rcp_ticks;
	}
}

static void siglp1_t_dsp(t_siglp1_t *x, t_signal **sp)
{
	int i, n=(int)sp[0]->s_n;

	x->sr = -1000.0f / (float)(sp[0]->s_sr);
	x->ticks_per_interpol_time = 0.001f * (float)(sp[0]->s_sr) / (float)n;
	i = (int)((x->ticks_per_interpol_time)*(x->interpol_time));
	if(i <= 0)
		i = 1;
	x->ticks = i;
	x->rcp_ticks = 1.0f / (float)i;
	if(x->cur_t == 0.0f)
		x->c1 = 0.0f;
	else
		x->c1 = exp((x->sr)/x->cur_t);
	x->c0 = 1.0f - x->c1;
	if(n&7)
		dsp_add(siglp1_t_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
	else
		dsp_add(siglp1_t_perf8, 4, sp[0]->s_vec, sp[1]->s_vec, x, n);
}

static void *siglp1_t_new(t_symbol *s, int argc, t_atom *argv)
{
	t_siglp1_t *x = (t_siglp1_t *)pd_new(siglp1_t_class);
	int i;
	float time_const=0.0f, interpol=0.0f;

	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft2"));
	outlet_new(&x->x_obj, &s_signal);
	x->x_msi = 0;
	x->counter_t = 1;
	x->delta_t = 0.0f;
	x->interpol_time = 0.0f;
	x->yn1 = 0.0f;
	x->sr = -1.0f / 44.1f;
	if((argc >= 1)&&IS_A_FLOAT(argv,0))
		time_const = (float)atom_getfloatarg(0, argc, argv);
	if((argc >= 2)&&IS_A_FLOAT(argv,1))
		interpol = (float)atom_getfloatarg(1, argc, argv);
	if(time_const < 0.0f)
		time_const = 0.0f;
	x->cur_t = time_const;
	if(time_const == 0.0f)
		x->c1 = 0.0f;
	else
		x->c1 = exp((x->sr)/time_const);
	x->c0 = 1.0f - x->c1;
	if(interpol < 0.0f)
		interpol = 0.0f;
	x->interpol_time = interpol;
	x->ticks_per_interpol_time = 0.5f;
	i = (int)((x->ticks_per_interpol_time)*(x->interpol_time));
	if(i <= 0)
		i = 1;
	x->ticks = i;
	x->rcp_ticks = 1.0f / (float)i;
	x->end_t = x->cur_t;
	return (x);
}

void siglp1_t_setup(void)
{
	siglp1_t_class = class_new(gensym("lp1_t~"), (t_newmethod)siglp1_t_new,
				0, sizeof(t_siglp1_t), 0, A_GIMME, 0);
	CLASS_MAINSIGNALIN(siglp1_t_class, t_siglp1_t, x_msi);
	class_addmethod(siglp1_t_class, (t_method)siglp1_t_dsp, gensym("dsp"), 0);
	class_addmethod(siglp1_t_class, (t_method)siglp1_t_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addmethod(siglp1_t_class, (t_method)siglp1_t_ft2, gensym("ft2"), A_FLOAT, 0);
	class_sethelpsymbol(siglp1_t_class, gensym("iemhelp/help-lp1_t~"));
}
