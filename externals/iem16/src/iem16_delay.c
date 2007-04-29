/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_delay.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* del16read~, del16write~, vd16~ */

#include "iem16_table.h"

#if defined __WIN32 || defined __WIN32__
static int ugen_getsortno(void){return 0;}
#else
extern int ugen_getsortno(void);
#endif

#define DEFDELVS 64	    	/* LATER get this from canvas at DSP time */

/* ----------------------------- del16write~ ----------------------------- */
static t_class *sigdel16write_class;

typedef struct del16writectl{
  int c_n;
  t_iem16_16bit *c_vec;
  int c_phase;
} t_del16writectl;

typedef struct _sigdel16write{
  t_object x_obj;
  t_symbol *x_sym;
  t_del16writectl x_cspace;
  int x_sortno;   /* DSP sort number at which this was last put on chain */
  int x_rsortno;  /* DSP sort # for first del16read or write in chain */
  int x_vecsize;  /* vector size for del16read~ to use */
  float x_f;
} t_sigdel16write;

#define XTRASAMPS 4
#define SAMPBLK 4

/* routine to check that all del16writes/del16reads/vds have same vecsize */
static void sigdel16write_checkvecsize(t_sigdel16write *x, int vecsize){
  if (x->x_rsortno != ugen_getsortno())    {
    x->x_vecsize = vecsize;
    x->x_rsortno = ugen_getsortno();
  }
  else if (vecsize != x->x_vecsize)
    pd_error(x, "del16read/del16write/vd vector size mismatch");
}

static void *sigdel16write_new(t_symbol *s, t_floatarg msec){
  int nsamps;
  t_sigdel16write *x = (t_sigdel16write *)pd_new(sigdel16write_class);
  if (!*s->s_name) s = gensym("del16write~");
  pd_bind(&x->x_obj.ob_pd, s);
  x->x_sym = s;
  nsamps = msec * sys_getsr() * (float)(0.001f);
  if (nsamps < 1) nsamps = 1;
  nsamps += ((- nsamps) & (SAMPBLK - 1));
  nsamps += DEFDELVS;
  x->x_cspace.c_n = nsamps;
  x->x_cspace.c_vec =
    (t_iem16_16bit *)getbytes((nsamps + XTRASAMPS) * sizeof(t_iem16_16bit));
  x->x_cspace.c_phase = XTRASAMPS;
  x->x_sortno = 0;
  x->x_vecsize = 0;
  x->x_f = 0;
  return (x);
}

static t_int *sigdel16write_perform(t_int *w){
  t_float *in = (t_float *)(w[1]);
  t_del16writectl *c = (t_del16writectl *)(w[2]);
  int n = (int)(w[3]);
  int phase = c->c_phase, nsamps = c->c_n;
  t_iem16_16bit *vp = c->c_vec, *bp = vp + phase, *ep = vp + (c->c_n + XTRASAMPS);
  phase += n;
  while (n--)    {
    *bp++ = (*in++*IEM16_SCALE_UP);
    if (bp == ep)  	{
      vp[0] = ep[-4];
      vp[1] = ep[-3];
      vp[2] = ep[-2];
      vp[3] = ep[-1];
      bp = vp + XTRASAMPS;
      phase -= nsamps;
    }
  }
  c->c_phase = phase; 
  return (w+4);
}

static void sigdel16write_dsp(t_sigdel16write *x, t_signal **sp){
  dsp_add(sigdel16write_perform, 3, sp[0]->s_vec, &x->x_cspace, sp[0]->s_n);
  x->x_sortno = ugen_getsortno();
  sigdel16write_checkvecsize(x, sp[0]->s_n);
}

static void sigdel16write_free(t_sigdel16write *x){
  pd_unbind(&x->x_obj.ob_pd, x->x_sym);
  freebytes(x->x_cspace.c_vec,
	    (x->x_cspace.c_n + XTRASAMPS) * sizeof(t_iem16_16bit));
}

static void sigdel16write_setup(void){
  sigdel16write_class = class_new(gensym("del16write~"), 
				  (t_newmethod)sigdel16write_new, (t_method)sigdel16write_free,
				  sizeof(t_sigdel16write), 0, A_DEFSYM, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(sigdel16write_class, t_sigdel16write, x_f);
  class_addmethod(sigdel16write_class, (t_method)sigdel16write_dsp,
		  gensym("dsp"), 0);
}

// G.Holzmann: for PD-extended build system
void del16write_tilde_setup(void)
{
  sigdel16write_setup();
}

/* ----------------------------- del16read~ ----------------------------- */
static t_class *sigdel16read_class;

typedef struct _sigdel16read{
  t_object x_obj;
  t_symbol *x_sym;
  t_float x_deltime;	/* delay in msec */
  int x_delsamps; 	/* delay in samples */
  t_float x_sr;   	/* samples per msec */
  t_float x_n;   	/* vector size */
  int x_zerodel;  	/* 0 or vecsize depending on read/write order */
} t_sigdel16read;

static void sigdel16read_16bit(t_sigdel16read *x, t_float f);

static void *sigdel16read_new(t_symbol *s, t_floatarg f){
  t_sigdel16read *x = (t_sigdel16read *)pd_new(sigdel16read_class);
  x->x_sym = s;
  x->x_sr = 1;
  x->x_n = 1;
  x->x_zerodel = 0;
  sigdel16read_16bit(x, f);
  outlet_new(&x->x_obj, gensym("signal"));
  return (x);
}

static void sigdel16read_16bit(t_sigdel16read *x, t_float f){
  t_sigdel16write *delwriter =
    (t_sigdel16write *)pd_findbyclass(x->x_sym, sigdel16write_class);
  x->x_deltime = f;
  if (delwriter)    {
    x->x_delsamps = (int)(0.5 + x->x_sr * x->x_deltime)
      + x->x_n - x->x_zerodel;
    if (x->x_delsamps < x->x_n) x->x_delsamps = x->x_n;
    else if (x->x_delsamps > delwriter->x_cspace.c_n - DEFDELVS)
      x->x_delsamps = delwriter->x_cspace.c_n - DEFDELVS;
  }
}

static t_int *sigdel16read_perform(t_int *w){
  t_float *out = (t_float *)(w[1]);
  t_del16writectl *c = (t_del16writectl *)(w[2]);
  int delsamps = *(int *)(w[3]);
  int n = (int)(w[4]);
  int phase = c->c_phase - delsamps, nsamps = c->c_n;
  t_iem16_16bit *vp = c->c_vec, *bp, *ep = vp + (c->c_n + XTRASAMPS);

  if (phase < 0) phase += nsamps;
  bp = vp + phase;
  while (n--)    {
    *out++ = *bp++*IEM16_SCALE_DOWN;
    if (bp == ep) bp -= nsamps;
  }
  return (w+5);
}

static void sigdel16read_dsp(t_sigdel16read *x, t_signal **sp){
  t_sigdel16write *delwriter =
    (t_sigdel16write *)pd_findbyclass(x->x_sym, sigdel16write_class);
  x->x_sr = sp[0]->s_sr * 0.001;
  x->x_n = sp[0]->s_n;
  if (delwriter)    {
    sigdel16write_checkvecsize(delwriter, sp[0]->s_n);
    x->x_zerodel = (delwriter->x_sortno == ugen_getsortno() ?
		    0 : delwriter->x_vecsize);
    sigdel16read_16bit(x, x->x_deltime);
    dsp_add(sigdel16read_perform, 4,
    	    sp[0]->s_vec, &delwriter->x_cspace, &x->x_delsamps, sp[0]->s_n);
  }
  else if (*x->x_sym->s_name)
    error("delread~: %s: no such delwrite~",x->x_sym->s_name);
}

static void sigdel16read_setup(void){
  sigdel16read_class = class_new(gensym("del16read~"),
				 (t_newmethod)sigdel16read_new, 0,
				 sizeof(t_sigdel16read), 0, A_DEFSYM, A_DEFFLOAT, 0);
  class_addmethod(sigdel16read_class, (t_method)sigdel16read_dsp,
		  gensym("dsp"), 0);
  class_addfloat(sigdel16read_class, (t_method)sigdel16read_16bit);
}

// G.Holzmann: for PD-extended build system
void del16read_tilde_setup(void)
{
  sigdel16read_setup();
}

/* ----------------------------- vd~ ----------------------------- */
static t_class *sig16vd_class;

typedef struct _sig16vd{
  t_object x_obj;
  t_symbol *x_sym;
  t_float x_sr;   	/* samples per msec */
  int x_zerodel;  	/* 0 or vecsize depending on read/write order */
  float x_f;
} t_sig16vd;

static void *sig16vd_new(t_symbol *s){
  t_sig16vd *x = (t_sig16vd *)pd_new(sig16vd_class);
  if (!*s->s_name) s = gensym("vd~");
  x->x_sym = s;
  x->x_sr = 1;
  x->x_zerodel = 0;
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_f = 0;
  return (x);
}

static t_int *sig16vd_perform(t_int *w){
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_del16writectl *ctl = (t_del16writectl *)(w[3]);
  t_sig16vd *x = (t_sig16vd *)(w[4]);
  int n = (int)(w[5]);

  int nsamps = ctl->c_n;
  float limit = nsamps - n - 1;
  float fn = n-4;
  t_iem16_16bit *vp = ctl->c_vec, *bp, *wp = vp + ctl->c_phase;
  float zerodel = x->x_zerodel;
  while (n--)    {
    float delsamps = x->x_sr * *in++ - zerodel, frac;
    int idelsamps;
    float a, b, c, d, cminusb;
    if (delsamps < 1.00001f) delsamps = 1.00001f;
    if (delsamps > limit) delsamps = limit;
    delsamps += fn;
    fn = fn - 1.0f;
    idelsamps = delsamps;
    frac = delsamps - (float)idelsamps;
    bp = wp - (idelsamps + 3);
    if (bp < vp + 4) bp += nsamps;
    d = bp[-3]*IEM16_SCALE_DOWN;
    c = bp[-2]*IEM16_SCALE_DOWN;
    b = bp[-1]*IEM16_SCALE_DOWN;
    a = bp[00]*IEM16_SCALE_DOWN;
    cminusb = c-b;
    *out++ = b + frac * (
			 cminusb - 0.5f * (frac-1.) * (
						       (a - d + 3.0f * cminusb) * frac + (b - a - cminusb)
						       )
			 );
  }
  return (w+6);
}

static void sig16vd_dsp(t_sig16vd *x, t_signal **sp){
  t_sigdel16write *delwriter =
    (t_sigdel16write *)pd_findbyclass(x->x_sym, sigdel16write_class);
  x->x_sr = sp[0]->s_sr * 0.001;
  if (delwriter)    {
    sigdel16write_checkvecsize(delwriter, sp[0]->s_n);
    x->x_zerodel = (delwriter->x_sortno == ugen_getsortno() ?
		    0 : delwriter->x_vecsize);
    dsp_add(sig16vd_perform, 5,
    	    sp[0]->s_vec, sp[1]->s_vec,
	    &delwriter->x_cspace, x, sp[0]->s_n);
  }
  else error("vd~: %s: no such delwrite~",x->x_sym->s_name);
}

static void sig16vd_setup(void){
  sig16vd_class = class_new(gensym("vd16~"), (t_newmethod)sig16vd_new, 0,
			    sizeof(t_sig16vd), 0, A_DEFSYM, 0);
  class_addmethod(sig16vd_class, (t_method)sig16vd_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(sig16vd_class, t_sig16vd, x_f);
}

// G.Holzmann: for PD-extended build system
void vd16_tilde_setup(void)
{
  sig16vd_setup();
}

/* ----------------------- global setup routine ---------------- */

void iem16_delay_setup(void){
  sigdel16write_setup();
  sigdel16read_setup();
  sig16vd_setup();
}

