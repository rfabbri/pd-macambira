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

/* ---------------- pvu~ - simple peak-vu-meter. ----------------- */

typedef struct sigpvu
{
	t_object	x_obj;
	void			*x_outlet_meter;
	void			*x_outlet_over;
	void			*x_clock;
	float			x_cur_peak;
	float			x_old_peak;
	float			x_threshold_over;
	float			x_c1;
	int				x_metro_time;
	float			x_release_time;
	int				x_overflow_counter;
	int				x_started;
	float			x_msi;
} t_sigpvu;

t_class *sigpvu_class;
static void sigpvu_tick(t_sigpvu *x);

static void sigpvu_reset(t_sigpvu *x)
{
	outlet_float(x->x_outlet_over, (t_float)0);
	outlet_float(x->x_outlet_meter, -199.9);
	x->x_overflow_counter = 0;
	x->x_cur_peak = 0.0f;
	x->x_old_peak = 0.0f;
	clock_delay(x->x_clock, x->x_metro_time);
}

static void sigpvu_stop(t_sigpvu *x)
{
	clock_unset(x->x_clock);
	x->x_started = 0;
}

static void sigpvu_start(t_sigpvu *x)
{
	clock_delay(x->x_clock, x->x_metro_time);
	x->x_started = 1;
}

static void sigpvu_float(t_sigpvu *x, t_floatarg f)
{
	if(f == 0.0)
	{
		clock_unset(x->x_clock);
		x->x_started = 0;
	}
	else
	{
		clock_delay(x->x_clock, x->x_metro_time);
		x->x_started = 1;
	}
}

static void sigpvu_t_release(t_sigpvu *x, t_floatarg release_time)
{
	if(release_time <= 20.0f)
		release_time = 20.0f;
	x->x_release_time = release_time;
	x->x_c1 = exp(-(float)x->x_metro_time/(float)release_time);
}

static void sigpvu_t_metro(t_sigpvu *x, t_floatarg metro_time)
{
	if(metro_time <= 20.0f)
		metro_time = 20.0f;
	x->x_metro_time = (int)metro_time;
	x->x_c1 = exp(-(float)metro_time/(float)x->x_release_time);
}

static void sigpvu_threshold(t_sigpvu *x, t_floatarg thresh)
{
	x->x_threshold_over = thresh;
}

static t_int *sigpvu_perform(t_int *w)
{
	float *in = (float *)(w[1]);
	t_sigpvu *x = (t_sigpvu *)(w[2]);
	int n = (int)(w[3]);
	float peak = x->x_cur_peak;
	float absolute;
	int i;

	if(x->x_started)
	{
		for(i=0; i<n; i++)
		{
			absolute = fabs(*in++);
			if(absolute > peak)
				peak = absolute;
		}
		x->x_cur_peak = peak;
	}
	return(w+4);
}

static void sigpvu_dsp(t_sigpvu *x, t_signal **sp)
{
	dsp_add(sigpvu_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
	clock_delay(x->x_clock, x->x_metro_time);
}

static void sigpvu_tick(t_sigpvu *x)
{
	float db;
	int i;

	x->x_old_peak *= x->x_c1;
	/* NAN protect */
	if(PD_BADFLOAT(x->x_old_peak))
		x->x_old_peak = 0.0f;

	if(x->x_cur_peak > x->x_old_peak)
		x->x_old_peak = x->x_cur_peak;
	if(x->x_old_peak <= 0.0000000001f)
		db = -199.9f;
	else if(x->x_old_peak > 1000000.0f)
	{
		db = 120.0f;
		x->x_old_peak = 1000000.0f;
	}
	else
		db = 8.6858896381f*log(x->x_old_peak);
	if(db >= x->x_threshold_over)
	{
		x->x_overflow_counter++;
		outlet_float(x->x_outlet_over, (t_float)x->x_overflow_counter);
	}
	outlet_float(x->x_outlet_meter, db);
	x->x_cur_peak = 0.0f;
	clock_delay(x->x_clock, x->x_metro_time);
}

static void *sigpvu_new(t_floatarg metro_time, t_floatarg release_time, t_floatarg threshold)
{
	t_sigpvu *x;
	float t;

	x = (t_sigpvu *)pd_new(sigpvu_class);
	if(metro_time <= 0.0f)
		metro_time = 300.0f;
	if(metro_time <= 20.0f)
		metro_time = 20.0f;
	if(release_time <= 0.0f)
		release_time = 300.0f;
	if(release_time <= 20.0f)
		release_time = 20.0f;
	if(threshold == 0.0f)
		threshold = -0.01f;
	x->x_threshold_over = threshold;
	x->x_overflow_counter = 0;
	x->x_metro_time = (int)metro_time;
	x->x_release_time = release_time;
	x->x_c1 = exp(-(float)metro_time/(float)release_time);
	x->x_cur_peak = 0.0f;
	x->x_old_peak = 0.0f;
	x->x_clock = clock_new(x, (t_method)sigpvu_tick);
	x->x_outlet_meter = outlet_new(&x->x_obj, &s_float);/* left */
	x->x_outlet_over = outlet_new(&x->x_obj, &s_float); /* right */
	x->x_started = 1;
	x->x_msi = 0;
	return(x);
}

static void sigpvu_ff(t_sigpvu *x)
{
	clock_free(x->x_clock);
}

void sigpvu_setup(void )
{
	sigpvu_class = class_new(gensym("pvu~"), (t_newmethod)sigpvu_new,
				 (t_method)sigpvu_ff, sizeof(t_sigpvu), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	CLASS_MAINSIGNALIN(sigpvu_class, t_sigpvu, x_msi);
	class_addmethod(sigpvu_class, (t_method)sigpvu_dsp, gensym("dsp"), 0);
	class_addfloat(sigpvu_class, sigpvu_float);
	class_addmethod(sigpvu_class, (t_method)sigpvu_reset, gensym("reset"), 0);
	class_addmethod(sigpvu_class, (t_method)sigpvu_start, gensym("start"), 0);
	class_addmethod(sigpvu_class, (t_method)sigpvu_stop, gensym("stop"), 0);
	class_addmethod(sigpvu_class, (t_method)sigpvu_t_release, gensym("t_release"), A_FLOAT, 0);
	class_addmethod(sigpvu_class, (t_method)sigpvu_t_metro, gensym("t_metro"), A_FLOAT, 0);
	class_addmethod(sigpvu_class, (t_method)sigpvu_threshold, gensym("threshold"), A_FLOAT, 0);
	class_sethelpsymbol(sigpvu_class, gensym("iemhelp/help-pvu~"));
}
