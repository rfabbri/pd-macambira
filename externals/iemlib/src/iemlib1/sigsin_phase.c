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


/* --- sin_phase~ - output the phase-difference between 2 sinewaves in samples (sig) ---- */

typedef struct sigsin_phase
{
	t_object x_obj;
	float x_prev1;
	float x_prev2;
	float x_cur_out;
	int	  x_counter1;
	int	  x_counter2;
	int		x_state1;
	int		x_state2;
	float	x_msi;
} t_sigsin_phase;

t_class *sigsin_phase_class;

static t_int *sigsin_phase_perform(t_int *w)
{
	float *in1 = (float *)(w[1]);
	float *in2 = (float *)(w[2]);
	float *out = (float *)(w[3]);
	t_sigsin_phase *x = (t_sigsin_phase *)(w[4]);
	int i, n = (t_int)(w[5]);
	float prev1=x->x_prev1;
	float prev2=x->x_prev2;
	float cur_out=x->x_cur_out;
	int counter1=x->x_counter1;
	int counter2=x->x_counter2;
	int state1=x->x_state1;
	int state2=x->x_state2;

	for(i=0; i<n; i++)
	{
		if((in1[i] >= 0.0f) && (prev1 < 0.0f))
		{
			state1 = 1;
			counter1 = 0;
		}
		else if((in1[i] < 0.0f) && (prev1 >= 0.0f))
		{
			state2 = 1;
			counter2 = 0;
		}

		if((in2[i] >= 0.0f) && (prev2 < 0.0f))
		{
			state1 = 0;
			cur_out = (float)(counter1);
			counter1 = 0;
		}
		else if((in2[i] < 0.0f) && (prev2 >= 0.0f))
		{
			state2 = 0;
			cur_out = (float)(counter2);
			counter2 = 0;
		}

		if(state1)
			counter1++;
		if(state2)
			counter2++;

		prev1 = in1[i];
		prev2 = in2[i];
		out[i] = cur_out;
	}

	x->x_prev1 = prev1;
	x->x_prev2 = prev2;
	x->x_cur_out = cur_out;
	x->x_counter1 = counter1;
	x->x_counter2 = counter2;
	x->x_state1 = state1;
	x->x_state2 = state2;

	return(w+6);
}

static void sigsin_phase_dsp(t_sigsin_phase *x, t_signal **sp)
{
	dsp_add(sigsin_phase_perform, 5, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, x, sp[0]->s_n);
}

static void *sigsin_phase_new(void)
{
	t_sigsin_phase *x = (t_sigsin_phase *)pd_new(sigsin_phase_class);

	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	outlet_new(&x->x_obj, &s_signal);

	x->x_prev1 = 0.0f;
	x->x_prev2 = 0.0f;
	x->x_cur_out = 0.0f;
	x->x_counter1 = 0;
	x->x_counter2 = 0;
	x->x_state1 = 0;
	x->x_state2 = 0;
	x->x_msi = 0;

	return (x);
}

void sigsin_phase_setup(void)
{
	sigsin_phase_class = class_new(gensym("sin_phase~"), (t_newmethod)sigsin_phase_new,
				0, sizeof(t_sigsin_phase), 0, 0);
	CLASS_MAINSIGNALIN(sigsin_phase_class, t_sigsin_phase, x_msi);
	class_addmethod(sigsin_phase_class, (t_method)sigsin_phase_dsp, gensym("dsp"), 0);
	class_sethelpsymbol(sigsin_phase_class, gensym("iemhelp/help-sin_phase~"));
}
