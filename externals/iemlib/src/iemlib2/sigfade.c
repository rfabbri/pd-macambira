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
#include <string.h>
#include <stdio.h>
#include <math.h>

/* ------------------------ fade~ ----------------------------- */

float *iem_fade_table_lin=(float *)0L;
float *iem_fade_table_linsqrt=(float *)0L;
float *iem_fade_table_sqrt=(float *)0L;
float *iem_fade_table_sin=(float *)0L;
float *iem_fade_table_sinhann=(float *)0L;
float *iem_fade_table_hann=(float *)0L;

static t_class *sigfade_class;

typedef struct _sigfade
{
	t_object x_obj;
	float *x_table;
	float x_f;
} t_sigfade;

static void sigfade_set(t_sigfade *x, t_symbol *s)
{
	if(!strcmp(s->s_name, "_lin"))
		x->x_table = iem_fade_table_lin;
	else if(!strcmp(s->s_name, "_linsqrt"))
		x->x_table = iem_fade_table_linsqrt;
	else if(!strcmp(s->s_name, "_sqrt"))
		x->x_table = iem_fade_table_sqrt;
	else if(!strcmp(s->s_name, "_sin"))
		x->x_table = iem_fade_table_sin;
	else if(!strcmp(s->s_name, "_sinhann"))
		x->x_table = iem_fade_table_sinhann;
	else if(!strcmp(s->s_name, "_hann"))
		x->x_table = iem_fade_table_hann;
}

static void *sigfade_new(t_symbol *s)
{
	t_sigfade *x = (t_sigfade *)pd_new(sigfade_class);
	outlet_new(&x->x_obj, gensym("signal"));
	x->x_f = 0;
	x->x_table = iem_fade_table_lin;
	sigfade_set(x, s);
	return (x);
}

static t_int *sigfade_perform(t_int *w)
{
	t_float *in = (t_float *)(w[1]);
	t_float *out = (t_float *)(w[2]);
	t_sigfade *x = (t_sigfade *)(w[3]);
	int n = (int)(w[4]);
	float *tab = x->x_table, *addr, f1, f2, frac;
	double dphase;
	int normhipart;
	union tabfudge tf;
	
	tf.tf_d = UNITBIT32;
	normhipart = tf.tf_i[HIOFFSET];

#if 0			/* this is the readable version of the code. */
	while (n--)
	{
		dphase = (double)(*in++ * (float)(COSTABSIZE) * 0.99999) + UNITBIT32;
		tf.tf_d = dphase;
		addr = tab + (tf.tf_i[HIOFFSET] & (COSTABSIZE-1));
		tf.tf_i[HIOFFSET] = normhipart;
		frac = tf.tf_d - UNITBIT32;
		f1 = addr[0];
		f2 = addr[1];
		*out++ = f1 + frac * (f2 - f1);
	}
#endif
#if 1			/* this is the same, unwrapped by hand. */
	dphase = (double)(*in++ * (float)(COSTABSIZE) * 0.99999) + UNITBIT32;
	tf.tf_d = dphase;
	addr = tab + (tf.tf_i[HIOFFSET] & (COSTABSIZE-1));
	tf.tf_i[HIOFFSET] = normhipart;
	while (--n)
	{
		dphase = (double)(*in++ * (float)(COSTABSIZE) * 0.99999) + UNITBIT32;
		frac = tf.tf_d - UNITBIT32;
		tf.tf_d = dphase;
		f1 = addr[0];
		f2 = addr[1];
		addr = tab + (tf.tf_i[HIOFFSET] & (COSTABSIZE-1));
		*out++ = f1 + frac * (f2 - f1);
		tf.tf_i[HIOFFSET] = normhipart;
	}
	frac = tf.tf_d - UNITBIT32;
	f1 = addr[0];
	f2 = addr[1];
	*out++ = f1 + frac * (f2 - f1);
#endif
	return (w+5);
}

static void sigfade_dsp(t_sigfade *x, t_signal **sp)
{
	dsp_add(sigfade_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

static void sigfade_maketable(void)
{
	int i;
	float *fp, phase, fff,phsinc = 0.5*3.141592653 / ((float)COSTABSIZE*0.99999);
	union tabfudge tf;

	if(!iem_fade_table_sin)
	{
		iem_fade_table_sin = (float *)getbytes(sizeof(float) * (COSTABSIZE+1));
		for(i=COSTABSIZE+1, fp=iem_fade_table_sin, phase=0; i--; fp++, phase+=phsinc)
			*fp = sin(phase);
	}
	if(!iem_fade_table_sinhann)
	{
		iem_fade_table_sinhann = (float *)getbytes(sizeof(float) * (COSTABSIZE+1));
		for(i=COSTABSIZE+1, fp=iem_fade_table_sinhann, phase=0; i--; fp++, phase+=phsinc)
		{
			fff = sin(phase);
			*fp = fff*sqrt(fff);
		}
	}
	if(!iem_fade_table_hann)
	{
		iem_fade_table_hann = (float *)getbytes(sizeof(float) * (COSTABSIZE+1));
		for(i=COSTABSIZE+1, fp=iem_fade_table_hann, phase=0; i--; fp++, phase+=phsinc)
		{
			fff = sin(phase);
			*fp = fff*fff;
		}
	}
	phsinc = 1.0 / ((float)COSTABSIZE*0.99999);
	if(!iem_fade_table_lin)
	{
		iem_fade_table_lin = (float *)getbytes(sizeof(float) * (COSTABSIZE+1));
		for(i=COSTABSIZE+1, fp=iem_fade_table_lin, phase=0; i--; fp++, phase+=phsinc)
			*fp = phase;
	}
	if(!iem_fade_table_linsqrt)
	{
		iem_fade_table_linsqrt = (float *)getbytes(sizeof(float) * (COSTABSIZE+1));
		for(i=COSTABSIZE+1, fp=iem_fade_table_linsqrt, phase=0; i--; fp++, phase+=phsinc)
			*fp = pow(phase, 0.75);
	}
	if(!iem_fade_table_sqrt)
	{
		iem_fade_table_sqrt = (float *)getbytes(sizeof(float) * (COSTABSIZE+1));
		for(i=COSTABSIZE+1, fp=iem_fade_table_sqrt, phase=0; i--; fp++, phase+=phsinc)
			*fp = sqrt(phase);
	}
	tf.tf_d = UNITBIT32 + 0.5;
	if((unsigned)tf.tf_i[LOWOFFSET] != 0x80000000)
		bug("fade~: unexpected machine alignment");
}

void sigfade_setup(void)
{
	sigfade_class = class_new(gensym("fade~"), (t_newmethod)sigfade_new, 0,
		sizeof(t_sigfade), 0, A_DEFSYM, 0);
	CLASS_MAINSIGNALIN(sigfade_class, t_sigfade, x_f);
	class_addmethod(sigfade_class, (t_method)sigfade_dsp, gensym("dsp"), 0);
	class_addmethod(sigfade_class, (t_method)sigfade_set, gensym("set"), A_DEFSYM, 0);
	class_sethelpsymbol(sigfade_class, gensym("iemhelp/help-fade~"));
	sigfade_maketable();
}

