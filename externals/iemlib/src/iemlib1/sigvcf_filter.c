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

/* ---------- vcf_filter~ - slow dynamic vcf_filter 1. and 2. order ----------- */

typedef struct sigvcf_filter
{
	t_object x_obj;
	float	x_wn1;
	float	x_wn2;
	float	x_msi;
	char	x_filtname[6];
} t_sigvcf_filter;

t_class *sigvcf_filter_class;

static t_int *sigvcf_filter_perform_snafu(t_int *w)
{
	float *in = (float *)(w[1]);
	float *out = (float *)(w[4]);
	int n = (t_int)(w[6]);

	while(n--)
		*out++ = *in++;
	return(w+7);
}

/*
lp2
	wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*(wn0 + 2.0f*wn1 + wn2);
	wn2 = wn1;
	wn1 = wn0;

bp2
	wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*al*(wn0 - wn2);
	wn2 = wn1;
	wn1 = wn0;

rbp2
	wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*l*(wn0 - wn2);
	wn2 = wn1;
	wn1 = wn0;

hp2
	wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*(wn0 - 2.0f*wn1 + wn2);
	wn2 = wn1;
	wn1 = wn0;

*/

static t_int *sigvcf_filter_perform_lp2(t_int *w)
{
	float *in = (float *)(w[1]);
	float *lp = (float *)(w[2]);
	float *q = (float *)(w[3]);
	float *out = (float *)(w[4]);
	t_sigvcf_filter *x = (t_sigvcf_filter *)(w[5]);
	int i, n = (t_int)(w[6]);
	float wn0, wn1=x->x_wn1, wn2=x->x_wn2;
	float l, al, l2, rcp;

	for(i=0; i<n; i+=4)
	{
		l = lp[i];
		if(q[i] < 0.000001)
			al = 1000000.0*l;
		else if(q[i] > 1000000.0)
			al = 0.000001*l;
		else
			al = l/q[i];
		l2 = l*l + 1.0f;
		rcp = 1.0f/(al + l2);

		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*(wn0 + 2.0f*wn1 + wn2);
		wn2 = wn1;
		wn1 = wn0;

		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*(wn0 + 2.0f*wn1 + wn2);
		wn2 = wn1;
		wn1 = wn0;

		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*(wn0 + 2.0f*wn1 + wn2);
		wn2 = wn1;
		wn1 = wn0;

		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*(wn0 + 2.0f*wn1 + wn2);
		wn2 = wn1;
		wn1 = wn0;
	}
	/* NAN protect */
	if(IEM_DENORMAL(wn2))
		wn2 = 0.0f;
	if(IEM_DENORMAL(wn1))
		wn1 = 0.0f;

	x->x_wn1 = wn1;
	x->x_wn2 = wn2;
	return(w+7);
}

static t_int *sigvcf_filter_perform_bp2(t_int *w)
{
	float *in = (float *)(w[1]);
	float *lp = (float *)(w[2]);
	float *q = (float *)(w[3]);
	float *out = (float *)(w[4]);
	t_sigvcf_filter *x = (t_sigvcf_filter *)(w[5]);
	int i, n = (t_int)(w[6]);
	float wn0, wn1=x->x_wn1, wn2=x->x_wn2;
	float l, al, l2, rcp;

	for(i=0; i<n; i+=4)
	{
		l = lp[i];
		if(q[i] < 0.000001)
			al = 1000000.0*l;
		else if(q[i] > 1000000.0)
			al = 0.000001*l;
		else
			al = l/q[i];
		l2 = l*l + 1.0f;
		rcp = 1.0f/(al + l2);


		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*al*(wn0 - wn2);
		wn2 = wn1;
		wn1 = wn0;

		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*al*(wn0 - wn2);
		wn2 = wn1;
		wn1 = wn0;

		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*al*(wn0 - wn2);
		wn2 = wn1;
		wn1 = wn0;

		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*al*(wn0 - wn2);
		wn2 = wn1;
		wn1 = wn0;
	}
	/* NAN protect */
	if(IEM_DENORMAL(wn2))
		wn2 = 0.0f;
	if(IEM_DENORMAL(wn1))
		wn1 = 0.0f;

	x->x_wn1 = wn1;
	x->x_wn2 = wn2;
	return(w+7);
}

static t_int *sigvcf_filter_perform_rbp2(t_int *w)
{
	float *in = (float *)(w[1]);
	float *lp = (float *)(w[2]);
	float *q = (float *)(w[3]);
	float *out = (float *)(w[4]);
	t_sigvcf_filter *x = (t_sigvcf_filter *)(w[5]);
	int i, n = (t_int)(w[6]);
	float wn0, wn1=x->x_wn1, wn2=x->x_wn2;
	float al, l, l2, rcp;

	for(i=0; i<n; i+=4)
	{
		l = lp[i];
		if(q[i] < 0.000001)
			al = 1000000.0*l;
		else if(q[i] > 1000000.0)
			al = 0.000001*l;
		else
			al = l/q[i];
		l2 = l*l + 1.0f;
		rcp = 1.0f/(al + l2);


		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*l*(wn0 - wn2);
		wn2 = wn1;
		wn1 = wn0;

		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*l*(wn0 - wn2);
		wn2 = wn1;
		wn1 = wn0;

		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*l*(wn0 - wn2);
		wn2 = wn1;
		wn1 = wn0;

		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = rcp*l*(wn0 - wn2);
		wn2 = wn1;
		wn1 = wn0;
	}
	/* NAN protect */
	if(IEM_DENORMAL(wn2))
		wn2 = 0.0f;
	if(IEM_DENORMAL(wn1))
		wn1 = 0.0f;

	x->x_wn1 = wn1;
	x->x_wn2 = wn2;
	return(w+7);
}

static t_int *sigvcf_filter_perform_hp2(t_int *w)
{
	float *in = (float *)(w[1]);
	float *lp = (float *)(w[2]);
	float *q = (float *)(w[3]);
	float *out = (float *)(w[4]);
	t_sigvcf_filter *x = (t_sigvcf_filter *)(w[5]);
	int i, n = (t_int)(w[6]);
	float wn0, wn1=x->x_wn1, wn2=x->x_wn2;
	float l, al, l2, rcp, forw;

	for(i=0; i<n; i+=4)
	{
		l = lp[i];
		if(q[i] < 0.000001)
			al = 1000000.0*l;
		else if(q[i] > 1000000.0)
			al = 0.000001*l;
		else
			al = l/q[i];
		l2 = l*l + 1.0f;
		rcp = 1.0f/(al + l2);
		forw = rcp * (l2 - 1.0f);

		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = forw*(wn0 - 2.0f*wn1 + wn2);
		wn2 = wn1;
		wn1 = wn0;

		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = forw*(wn0 - 2.0f*wn1 + wn2);
		wn2 = wn1;
		wn1 = wn0;

		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = forw*(wn0 - 2.0f*wn1 + wn2);
		wn2 = wn1;
		wn1 = wn0;

		wn0 = *in++ - rcp*(2.0f*(2.0f - l2)*wn1 + (l2 - al)*wn2);
		*out++ = forw*(wn0 - 2.0f*wn1 + wn2);
		wn2 = wn1;
		wn1 = wn0;
	}
	/* NAN protect */
	if(IEM_DENORMAL(wn2))
		wn2 = 0.0f;
	if(IEM_DENORMAL(wn1))
		wn1 = 0.0f;

	x->x_wn1 = wn1;
	x->x_wn2 = wn2;
	return(w+7);
}

static void sigvcf_filter_dsp(t_sigvcf_filter *x, t_signal **sp)
{
	if(!strcmp(x->x_filtname,"bp2"))
		dsp_add(sigvcf_filter_perform_bp2, 6, sp[0]->s_vec, sp[1]->s_vec, 
			sp[2]->s_vec, sp[3]->s_vec, x, sp[0]->s_n);
	else if(!strcmp(x->x_filtname,"rbp2"))
		dsp_add(sigvcf_filter_perform_rbp2, 6, sp[0]->s_vec, sp[1]->s_vec, 
			sp[2]->s_vec, sp[3]->s_vec, x, sp[0]->s_n);
	else if(!strcmp(x->x_filtname,"lp2"))
		dsp_add(sigvcf_filter_perform_lp2, 6, sp[0]->s_vec, sp[1]->s_vec,
			sp[2]->s_vec, sp[3]->s_vec, x, sp[0]->s_n);
	else if(!strcmp(x->x_filtname,"hp2"))
		dsp_add(sigvcf_filter_perform_hp2, 6, sp[0]->s_vec, sp[1]->s_vec,
			sp[2]->s_vec, sp[3]->s_vec, x, sp[0]->s_n);
	else
	{
		dsp_add(sigvcf_filter_perform_snafu, 6, sp[0]->s_vec, sp[1]->s_vec,
			sp[2]->s_vec, sp[3]->s_vec, x, sp[0]->s_n);
		post("vcf_filter~-Error: 1. initial-arguments: <sym> kind: lp2, bp2, rbp2, hp2!");
	}
}

static void *sigvcf_filter_new(t_symbol *filt_typ)
{
	t_sigvcf_filter *x = (t_sigvcf_filter *)pd_new(sigvcf_filter_class);
	char *c;

	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	outlet_new(&x->x_obj, &s_signal);
	x->x_msi = 0;
	x->x_wn1 = 0.0;
	x->x_wn2 = 0.0;
	c = (char *)filt_typ->s_name;
	c[5] = 0;
	strcpy(x->x_filtname, c);
	return(x);
}

void sigvcf_filter_setup(void)
{
	sigvcf_filter_class = class_new(gensym("vcf_filter~"), (t_newmethod)sigvcf_filter_new,
					0, sizeof(t_sigvcf_filter), 0, A_DEFSYM, 0);
	CLASS_MAINSIGNALIN(sigvcf_filter_class, t_sigvcf_filter, x_msi);
	class_addmethod(sigvcf_filter_class, (t_method)sigvcf_filter_dsp, gensym("dsp"), 0);
	class_sethelpsymbol(sigvcf_filter_class, gensym("iemhelp/help-vcf_filter~"));
}
