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

/* ---------------- peakenv~ - simple peak-envelope-converter. ----------------- */

typedef struct sigpeakenv
{
	t_object	x_obj;
	float	   x_sr;
	float	   x_old_peak;
	float	   x_c1;
	float	   x_releasetime;
	float	   x_msi;
} t_sigpeakenv;

t_class *sigpeakenv_class;

static void sigpeakenv_reset(t_sigpeakenv *x)
{
	x->x_old_peak = 0.0f;
}

static void sigpeakenv_ft1(t_sigpeakenv *x, t_floatarg f)/* release-time in ms */
{
	if(f < 0.0f)
		f = 0.0f;
	x->x_releasetime = f;
	x->x_c1 = exp(-1.0/(x->x_sr*0.001*x->x_releasetime));
}

static t_int *sigpeakenv_perform(t_int *w)
{
	float *in = (float *)(w[1]);
	float *out = (float *)(w[2]);
	t_sigpeakenv *x = (t_sigpeakenv *)(w[3]);
	int n = (int)(w[4]);
	float peak = x->x_old_peak;
	float c1 = x->x_c1;
	float absolute;
	int i;

	for(i=0; i<n; i++)
	{
		absolute = fabs(*in++);
		peak *= c1;
		if(absolute > peak)
			peak = absolute;
		*out++ = peak;
	}
	/* NAN protect */
	if(PD_BADFLOAT(peak))
		peak = 0.0f;
	x->x_old_peak = peak;
	return(w+5);
}

static void sigpeakenv_dsp(t_sigpeakenv *x, t_signal **sp)
{
	x->x_sr = (float)sp[0]->s_sr;
	sigpeakenv_ft1(x, x->x_releasetime);
	dsp_add(sigpeakenv_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

static void *sigpeakenv_new(t_floatarg f)
{
	t_sigpeakenv *x = (t_sigpeakenv *)pd_new(sigpeakenv_class);

	if(f <= 0.0f)
		f = 0.0f;
	x->x_sr = 44100.0f;
	sigpeakenv_ft1(x, f);
	x->x_old_peak = 0.0f;
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
	outlet_new(&x->x_obj, &s_signal);
	x->x_msi = 0;
	return(x);
}

void sigpeakenv_setup(void)
{
	sigpeakenv_class = class_new(gensym("peakenv~"), (t_newmethod)sigpeakenv_new,
				 0, sizeof(t_sigpeakenv), 0, A_DEFFLOAT, 0);
	CLASS_MAINSIGNALIN(sigpeakenv_class, t_sigpeakenv, x_msi);
	class_addmethod(sigpeakenv_class, (t_method)sigpeakenv_dsp, gensym("dsp"), 0);
	class_addmethod(sigpeakenv_class, (t_method)sigpeakenv_ft1, gensym("ft1"), A_FLOAT, 0);
	class_addmethod(sigpeakenv_class, (t_method)sigpeakenv_reset, gensym("reset"), 0);
	class_sethelpsymbol(sigpeakenv_class, gensym("iemhelp/help-peakenv~"));
}
