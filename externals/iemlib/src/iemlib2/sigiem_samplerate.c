/* Copyright (c) 1997-2003 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil (c) IEM KUG Graz Austria 2000 - 2003 */

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#include "m_pd.h"
#include "iemlib.h"
#include <stdlib.h>

/* ------------------------ iem_samplerate~ ----------------------------- */

static t_class *sigiem_samplerate_class;

typedef struct _sigiem_samplerate
{
	t_object	x_obj;
	t_float		x_samplerate;
	t_clock		*x_clock;
	t_float		x_f;
} t_sigiem_samplerate;

static void sigiem_samplerate_out(t_sigiem_samplerate *x)
{
	outlet_float(x->x_obj.ob_outlet, x->x_samplerate);
}

static void sigiem_samplerate_free(t_sigiem_samplerate *x)
{
	clock_free(x->x_clock);
}

static void *sigiem_samplerate_new(t_symbol *s)
{
	t_sigiem_samplerate *x = (t_sigiem_samplerate *)pd_new(sigiem_samplerate_class);
	x->x_clock = clock_new(x, (t_method)sigiem_samplerate_out);
	outlet_new(&x->x_obj, &s_float);
	x->x_samplerate = 44100.0f;
	x->x_f = 0.0f;
	return (x);
}

static void sigiem_samplerate_dsp(t_sigiem_samplerate *x, t_signal **sp)
{
	x->x_samplerate = (t_float)(sp[0]->s_sr);
	clock_delay(x->x_clock, 0.0f);
}

void sigiem_samplerate_setup(void)
{
	sigiem_samplerate_class = class_new(gensym("iem_samplerate~"), (t_newmethod)sigiem_samplerate_new,
		(t_method)sigiem_samplerate_free, sizeof(t_sigiem_samplerate), 0, 0);
	CLASS_MAINSIGNALIN(sigiem_samplerate_class, t_sigiem_samplerate, x_f);
	class_addmethod(sigiem_samplerate_class, (t_method)sigiem_samplerate_dsp, gensym("dsp"), 0);
}

