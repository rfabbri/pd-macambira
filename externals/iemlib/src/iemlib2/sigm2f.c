/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2005 */

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#include "m_pd.h"
#include "iemlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ----------- m2f~ ----------- */
/* --------- obsolete --------- */

#define M2FTABSIZE 2048

float *iem_m2f_table=(float *)0L;

static t_class *sigm2f_class;

typedef struct _sigm2f
{
  t_object x_obj;
  float x_msi;
} t_sigm2f;

static void *sigm2f_new(void)
{
  t_sigm2f *x = (t_sigm2f *)pd_new(sigm2f_class);
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_msi = 0;
  return (x);
}

static t_int *sigm2f_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_sigm2f *x = (t_sigm2f *)(w[3]);
  int n = (int)(w[4]);
  float *tab = iem_m2f_table, *addr, f1, f2, frac, iinn;
  double dphase;
  int normhipart;
  union tabfudge tf;
  
  tf.tf_d = UNITBIT32;
  normhipart = tf.tf_i[HIOFFSET];
  
#if 0       /* this is the readable version of the code. */
  while (n--)
  {
    iinn = (*in++)*10.0+670.0;
    dphase = (double)iinn + UNITBIT32;
    tf.tf_d = dphase;
    addr = tab + (tf.tf_i[HIOFFSET] & (M2FTABSIZE-1));
    tf.tf_i[HIOFFSET] = normhipart;
    frac = tf.tf_d - UNITBIT32;
    f1 = addr[0];
    f2 = addr[1];
    *out++ = f1 + frac * (f2 - f1);
  }
#endif
#if 1       /* this is the same, unwrapped by hand. */
  iinn = (*in++)*10.0+670.0;
  dphase = (double)iinn + UNITBIT32;
  tf.tf_d = dphase;
  addr = tab + (tf.tf_i[HIOFFSET] & (M2FTABSIZE-1));
  tf.tf_i[HIOFFSET] = normhipart;
  while (--n)
  {
    iinn = (*in++)*10.0+670.0;
    dphase = (double)iinn + UNITBIT32;
    frac = tf.tf_d - UNITBIT32;
    tf.tf_d = dphase;
    f1 = addr[0];
    f2 = addr[1];
    addr = tab + (tf.tf_i[HIOFFSET] & (M2FTABSIZE-1));
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

static void sigm2f_dsp(t_sigm2f *x, t_signal **sp)
{
  dsp_add(sigm2f_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

static void sigm2f_maketable(void)
{
  union tabfudge tf;
  
  if(!iem_m2f_table)
  {
    int i;
    float *fp, midi, refexp=440.0*exp(-5.75*log(2.0));
    
    iem_m2f_table = (float *)getbytes(sizeof(float) * (M2FTABSIZE+1));
    for(i=0, fp=iem_m2f_table, midi=-67.0; i<=M2FTABSIZE; i++, fp++, midi+=0.1)
      *fp = refexp * exp(0.057762265047 * midi);
  }
  tf.tf_d = UNITBIT32 + 0.5;
  if((unsigned)tf.tf_i[LOWOFFSET] != 0x80000000)
    bug("m2f~: unexpected machine alignment");
}

void sigm2f_setup(void)
{
  sigm2f_class = class_new(gensym("m2f~"), (t_newmethod)sigm2f_new, 0,
    sizeof(t_sigm2f), 0, 0);
  CLASS_MAINSIGNALIN(sigm2f_class, t_sigm2f, x_msi);
  class_addmethod(sigm2f_class, (t_method)sigm2f_dsp, gensym("dsp"), 0);
  sigm2f_maketable();
  class_sethelpsymbol(sigm2f_class, gensym("iemhelp/help-m2f~"));
}
