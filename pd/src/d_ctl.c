/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  The sig~ and line~ routines; possibly fancier envelope generators to
    come later.
*/

#include "m_pd.h"
#include "math.h"

/* -------------------------- sig~ ------------------------------ */
static t_class *sig_class;

typedef struct _sig
{
    t_object x_obj;
    float x_f;
} t_sig;

static t_int *sig_perform(t_int *w)
{
    t_float f = *(t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    while (n--) *out++ = f; 
    return (w+4);
}

static t_int *sig_perf8(t_int *w)
{
    t_float f = *(t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    
    for (; n; n -= 8, out += 8)
    {
    	out[0] = f;
    	out[1] = f;
    	out[2] = f;
    	out[3] = f;
    	out[4] = f;
    	out[5] = f;
    	out[6] = f;
    	out[7] = f;
    }
    return (w+4);
}

void dsp_add_scalarcopy(t_sample *in, t_sample *out, int n)
{
    if (n&7)
    	dsp_add(sig_perform, 3, in, out, n);
    else	
    	dsp_add(sig_perf8, 3, in, out, n);
}

static void sig_float(t_sig *x, t_float f)
{
    x->x_f = f;
}

static void sig_dsp(t_sig *x, t_signal **sp)
{
    dsp_add(sig_perform, 3, &x->x_f, sp[0]->s_vec, sp[0]->s_n);
}

static void *sig_new(t_floatarg f)
{
    t_sig *x = (t_sig *)pd_new(sig_class);
    x->x_f = f;
    outlet_new(&x->x_obj, gensym("signal"));
    return (x);
}

static void sig_setup(void)
{
    sig_class = class_new(gensym("sig~"), (t_newmethod)sig_new, 0,
    	sizeof(t_sig), 0, A_DEFFLOAT, 0);
    class_addfloat(sig_class, (t_method)sig_float);
    class_addmethod(sig_class, (t_method)sig_dsp, gensym("dsp"), 0);
}

/* -------------------------- line~ ------------------------------ */
static t_class *line_class;

typedef struct _line
{
    t_object x_obj;
    float x_target;
    float x_value;
    float x_biginc;
    float x_inc;
    float x_1overn;
    float x_msectodsptick;
    float x_inletvalue;
    float x_inletwas;
    int x_ticksleft;
    int x_retarget;
} t_line;

static t_int *line_perform(t_int *w)
{
    t_line *x = (t_line *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    float f = x->x_value;
	/* bash NANs and underflow/overflow hazards to zero */
    if (!((f > 1.0e-20f && f < 1.0e20f) || (f < -1e-20f && f > -1e20)))
	x->x_value = f = 0;
    if (x->x_retarget)
    {
    	int nticks = x->x_inletwas * x->x_msectodsptick;
    	if (!nticks) nticks = 1;
    	x->x_ticksleft = nticks;
    	x->x_biginc = (x->x_target - x->x_value)/(float)nticks;
    	x->x_inc = x->x_1overn * x->x_biginc;
    	x->x_retarget = 0;
    }
    if (x->x_ticksleft)
    {
    	float f = x->x_value;
    	while (n--) *out++ = f, f += x->x_inc;
    	x->x_value += x->x_biginc;
    	x->x_ticksleft--;
    }
    else
    {
    	x->x_value = x->x_target;
    	while (n--) *out++ = x->x_value;
    }
    return (w+4);
}

static void line_float(t_line *x, t_float f)
{
    if (x->x_inletvalue <= 0)
    {
    	x->x_target = x->x_value = f;
    	x->x_ticksleft = x->x_retarget = 0;
    }
    else
    {
    	x->x_target = f;
    	x->x_retarget = 1;
    	x->x_inletwas = x->x_inletvalue;
    	x->x_inletvalue = 0;
    }
}

static void line_stop(t_line *x)
{
    x->x_target = x->x_value;
    x->x_ticksleft = x->x_retarget = 0;
}

static void line_dsp(t_line *x, t_signal **sp)
{
    dsp_add(line_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
    x->x_1overn = 1./sp[0]->s_n;
    x->x_msectodsptick = sp[0]->s_sr / (1000 * sp[0]->s_n);
}

static void *line_new(void)
{
    t_line *x = (t_line *)pd_new(line_class);
    outlet_new(&x->x_obj, gensym("signal"));
    floatinlet_new(&x->x_obj, &x->x_inletvalue);
    x->x_ticksleft = x->x_retarget = 0;
    x->x_value = x->x_target = x->x_inletvalue = x->x_inletwas = 0;
    return (x);
}

static void line_setup(void)
{
    line_class = class_new(gensym("line~"), line_new, 0,
    	sizeof(t_line), 0, 0);
    class_addfloat(line_class, (t_method)line_float);
    class_addmethod(line_class, (t_method)line_dsp, gensym("dsp"), 0);
    class_addmethod(line_class, (t_method)line_stop, gensym("stop"), 0);
}

/* -------------------------- snapshot~ ------------------------------ */
static t_class *snapshot_class;

typedef struct _snapshot
{
    t_object x_obj;
    t_sample x_value;
    float x_f;
} t_snapshot;

static void *snapshot_new(void)
{
    t_snapshot *x = (t_snapshot *)pd_new(snapshot_class);
    x->x_value = 0;
    outlet_new(&x->x_obj, &s_float);
    x->x_f = 0;
    return (x);
}

static t_int *snapshot_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    *out = *in;
    return (w+3);
}

static void snapshot_dsp(t_snapshot *x, t_signal **sp)
{
    dsp_add(snapshot_perform, 2, sp[0]->s_vec + (sp[0]->s_n-1), &x->x_value);
}

static void snapshot_bang(t_snapshot *x)
{
    outlet_float(x->x_obj.ob_outlet, x->x_value);
}

static void snapshot_setup(void)
{
    snapshot_class = class_new(gensym("snapshot~"), snapshot_new, 0,
    	sizeof(t_snapshot), 0, 0);
    CLASS_MAINSIGNALIN(snapshot_class, t_snapshot, x_f);
    class_addmethod(snapshot_class, (t_method)snapshot_dsp, gensym("dsp"), 0);
    class_addbang(snapshot_class, snapshot_bang);
}

/* ---------------- env~ - simple envelope follower. ----------------- */

#define MAXOVERLAP 10
#define MAXVSTAKEN 64

typedef struct sigenv
{
    t_object x_obj; 	    	    /* header */
    void *x_outlet;		    /* a "float" outlet */
    void *x_clock;		    /* a "clock" object */
    float *x_buf;		    /* a Hanning window */
    int x_phase;		    /* number of points since last output */
    int x_period;		    /* requested period of output */
    int x_realperiod;		    /* period rounded up to vecsize multiple */
    int x_npoints;		    /* analysis window size in samples */
    float x_result;		    /* result to output */
    float x_sumbuf[MAXOVERLAP];	    /* summing buffer */
    float x_f;
} t_sigenv;

t_class *sigenv_class;
static void sigenv_tick(t_sigenv *x);

static void *sigenv_new(t_floatarg fnpoints, t_floatarg fperiod)
{
    int npoints = fnpoints;
    int period = fperiod;
    t_sigenv *x;
    float *buf;
    int i;

    if (npoints < 1) npoints = 1024;
    if (period < 1) period = npoints/2;
    if (period < npoints / MAXOVERLAP + 1)
	period = npoints / MAXOVERLAP + 1;
    if (!(buf = getbytes(sizeof(float) * (npoints + MAXVSTAKEN))))
    {
	error("env: couldn't allocate buffer");
	return (0);
    }
    x = (t_sigenv *)pd_new(sigenv_class);
    x->x_buf = buf;
    x->x_npoints = npoints;
    x->x_phase = 0;
    x->x_period = period;
    for (i = 0; i < MAXOVERLAP; i++) x->x_sumbuf[i] = 0;
    for (i = 0; i < npoints; i++)
	buf[i] = (1. - cos((2 * 3.14159 * i) / npoints))/npoints;
    for (; i < npoints+MAXVSTAKEN; i++) buf[i] = 0;
    x->x_clock = clock_new(x, (t_method)sigenv_tick);
    x->x_outlet = outlet_new(&x->x_obj, gensym("float"));
    x->x_f = 0;
    return (x);
}

static t_int *sigenv_perform(t_int *w)
{
    t_sigenv *x = (t_sigenv *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    int n = (int)(w[3]);
    int count;
    float *sump; 
    in += n;
    for (count = x->x_phase, sump = x->x_sumbuf;
	count < x->x_npoints; count += x->x_realperiod, sump++)
    {
	float *hp = x->x_buf + count;
	float *fp = in;
	float sum = *sump;
	int i;
	
	for (i = 0; i < n; i++)
	{
	    fp--;
	    sum += *hp++ * (*fp * *fp);
	}
	*sump = sum;
    }
    sump[0] = 0;
    x->x_phase -= n;
    if (x->x_phase < 0)
    {
	x->x_result = x->x_sumbuf[0];
	for (count = x->x_realperiod, sump = x->x_sumbuf;
	    count < x->x_npoints; count += x->x_realperiod, sump++)
		sump[0] = sump[1];
	sump[0] = 0;
	x->x_phase = x->x_realperiod - n;
	clock_delay(x->x_clock, 0L);
    }
    return (w+4);
}

static void sigenv_dsp(t_sigenv *x, t_signal **sp)
{
    if (x->x_period % sp[0]->s_n) x->x_realperiod =
	x->x_period + sp[0]->s_n - (x->x_period % sp[0]->s_n);
    else x->x_realperiod = x->x_period;
    dsp_add(sigenv_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
    if (sp[0]->s_n > MAXVSTAKEN) bug("sigenv_dsp");
}

static void sigenv_tick(t_sigenv *x)	/* callback function for the clock */
{
    outlet_float(x->x_outlet, powtodb(x->x_result));
}

static void sigenv_ff(t_sigenv *x)		/* cleanup on free */
{
    clock_free(x->x_clock);
    freebytes(x->x_buf, (x->x_npoints + MAXVSTAKEN) * sizeof(float));
}


void sigenv_setup(void )
{
    sigenv_class = class_new(gensym("env~"), (t_newmethod)sigenv_new,
    	(t_method)sigenv_ff, sizeof(t_sigenv), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(sigenv_class, t_sigenv, x_f);
    class_addmethod(sigenv_class, (t_method)sigenv_dsp, gensym("dsp"), 0);
}

/* --------------------- threshold~ ----------------------------- */

static t_class *threshold_tilde_class;

typedef struct _threshold_tilde
{
    t_object x_obj;
    t_outlet *x_outlet1;    	/* bang out for high thresh */
    t_outlet *x_outlet2;    	/* bang out for low thresh */
    t_clock *x_clock;	    	/* wakeup for message output */
    float x_f;	    	    	/* scalar inlet */
    int x_state;    		/* 1 = high, 0 = low */
    float x_hithresh;	    	/* value of high threshold */
    float x_lothresh;	    	/* value of low threshold */
    float x_deadwait;	    	/* msec remaining in dead period */
    float x_msecpertick;	/* msec per DSP tick */
    float x_hideadtime;	    	/* hi dead time in msec */
    float x_lodeadtime;	    	/* lo dead time in msec */
} t_threshold_tilde;

static void threshold_tilde_tick(t_threshold_tilde *x);
static void threshold_tilde_set(t_threshold_tilde *x,
    t_floatarg hithresh, t_floatarg hideadtime,
    t_floatarg lothresh, t_floatarg lodeadtime);

static t_threshold_tilde *threshold_tilde_new(t_floatarg hithresh,
    t_floatarg hideadtime, t_floatarg lothresh, t_floatarg lodeadtime)
{
    t_threshold_tilde *x = (t_threshold_tilde *)
    	pd_new(threshold_tilde_class);
    x->x_state = 0;		/* low state */
    x->x_deadwait = 0;		/* no dead time */
    x->x_clock = clock_new(x, (t_method)threshold_tilde_tick);
    x->x_outlet1 = outlet_new(&x->x_obj, &s_bang);
    x->x_outlet2 = outlet_new(&x->x_obj, &s_bang);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
    x->x_msecpertick = 0.;
    x->x_f = 0;
    threshold_tilde_set(x, hithresh, hideadtime, lothresh, lodeadtime);
    return (x);
}

    /* "set" message to specify thresholds and dead times */
static void threshold_tilde_set(t_threshold_tilde *x,
    t_floatarg hithresh, t_floatarg hideadtime,
    t_floatarg lothresh, t_floatarg lodeadtime)
{
    if (lothresh > hithresh)
    	lothresh = hithresh;
    x->x_hithresh = hithresh;
    x->x_hideadtime = hideadtime;
    x->x_lothresh = lothresh;
    x->x_lodeadtime = lodeadtime;
}

    /* number in inlet sets state -- note incompatible with JMAX which used
    "int" message for this, impossible here because of auto signal conversion */
static void threshold_tilde_ft1(t_threshold_tilde *x, t_floatarg f)
{
    x->x_state = (f != 0);
    x->x_deadwait = 0;
}

static void threshold_tilde_tick(t_threshold_tilde *x)	
{
    if (x->x_state)
    	outlet_bang(x->x_outlet1);
    else outlet_bang(x->x_outlet2);
}

static t_int *threshold_tilde_perform(t_int *w)
{
    float *in1 = (float *)(w[1]);
    t_threshold_tilde *x = (t_threshold_tilde *)(w[2]);
    int n = (t_int)(w[3]);
    if (x->x_deadwait > 0)
    	x->x_deadwait -= x->x_msecpertick;
    else if (x->x_state)
    {
    	    /* we're high; look for low sample */
    	for (; n--; in1++)
	{
	    if (*in1 < x->x_lothresh)
	    {
		clock_delay(x->x_clock, 0L);
		x->x_state = 0;
		x->x_deadwait = x->x_lodeadtime;
		goto done;
	    }
    	}
    }
    else
    {
    	    /* we're low; look for high sample */
    	for (; n--; in1++)
	{
	    if (*in1 >= x->x_hithresh)
	    {
		clock_delay(x->x_clock, 0L);
		x->x_state = 1;
		x->x_deadwait = x->x_hideadtime;
		goto done;
	    }
    	}
    }
done:
    return (w+4);
}

void threshold_tilde_dsp(t_threshold_tilde *x, t_signal **sp)
{
    x->x_msecpertick = 1000. * sp[0]->s_n / sp[0]->s_sr;
    dsp_add(threshold_tilde_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
}

static void threshold_tilde_ff(t_threshold_tilde *x)
{
    clock_free(x->x_clock);
}

static void threshold_tilde_setup( void)
{
    threshold_tilde_class = class_new(gensym("threshold~"),
    	(t_newmethod)threshold_tilde_new, (t_method)threshold_tilde_ff,
	sizeof(t_threshold_tilde), 0,
	    A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(threshold_tilde_class, t_threshold_tilde, x_f);
    class_addmethod(threshold_tilde_class, (t_method)threshold_tilde_set,
    	gensym("set"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(threshold_tilde_class, (t_method)threshold_tilde_ft1,
    	gensym("ft1"), A_FLOAT, 0);
    class_addmethod(threshold_tilde_class, (t_method)threshold_tilde_dsp,
    	gensym("dsp"), 0);
}

/* ------------------------ global setup routine ------------------------- */

void d_ctl_setup(void)
{
    sig_setup();
    line_setup();
    snapshot_setup();
    sigenv_setup();
    threshold_tilde_setup();
}

