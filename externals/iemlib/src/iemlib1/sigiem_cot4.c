/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2005 */

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


#include "m_pd.h"
#include "iemlib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/* ------------------------ iem_cot4~ ----------------------------- */

float *iem_cot4_table_cos=(float *)0L;
float *iem_cot4_table_sin=(float *)0L;

static t_class *sigiem_cot4_class;

typedef struct _sigiem_cot4
{
  t_object  x_obj;
  float     x_sr;
  float     x_msi;
} t_sigiem_cot4;

static t_int *sigiem_cot4_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_float norm_freq;
  t_float hout;
  t_sigiem_cot4 *x = (t_sigiem_cot4 *)(w[3]);
  t_float sr=x->x_sr;
  int n = (int)(w[4]);
  float *ctab = iem_cot4_table_cos, *stab = iem_cot4_table_sin;
  float *caddr, *saddr, cf1, cf2, sf1, sf2, frac;
  double dphase;
  int normhipart;
  int32 mytfi;
  union tabfudge tf;
  
  tf.tf_d = UNITBIT32;
  normhipart = tf.tf_i[HIOFFSET];
  
#if 0     /* this is the readable version of the code. */
  while (n--)
  {
    norm_freq = *in * sr;
    if(norm_freq < 0.0001f)
      norm_freq = 0.0001f;
    else if(norm_freq > 0.9f)
      norm_freq = 0.9f;
    dphase = (double)(norm_freq * (float)(COSTABSIZE)) + UNITBIT32;
    tf.tf_d = dphase;
    mytfi = tf.tf_i[HIOFFSET] & (COSTABSIZE-1);
    saddr = stab + (mytfi);
    caddr = ctab + (mytfi);
    tf.tf_i[HIOFFSET] = normhipart;
    frac = tf.tf_d - UNITBIT32;
    sf1 = saddr[0];
    sf2 = saddr[1];
    cf1 = caddr[0];
    cf2 = caddr[1];
    in++;
    *out++ = (cf1 + frac * (cf2 - cf1))/(sf1 + frac * (sf2 - sf1));
  }
#endif
#if 1     /* this is the same, unwrapped by hand. prolog beg*/
  n /= 4;
  norm_freq = *in * sr;
  if(norm_freq < 0.0001f)
    norm_freq = 0.0001f;
  else if(norm_freq > 0.9f)
    norm_freq = 0.9f;
  dphase = (double)(norm_freq * (float)(COSTABSIZE)) + UNITBIT32;
  tf.tf_d = dphase;
  mytfi = tf.tf_i[HIOFFSET] & (COSTABSIZE-1);
  saddr = stab + (mytfi);
  caddr = ctab + (mytfi);
  tf.tf_i[HIOFFSET] = normhipart;   
  in += 4;                 /*prolog end*/
  while (--n)
  {
    norm_freq = *in * sr;
    if(norm_freq < 0.0001f)
      norm_freq = 0.0001f;
    else if(norm_freq > 0.9f)
      norm_freq = 0.9f;
    dphase = (double)(norm_freq * (float)(COSTABSIZE)) + UNITBIT32;
    frac = tf.tf_d - UNITBIT32;
    tf.tf_d = dphase;
    sf1 = saddr[0];
    sf2 = saddr[1];
    cf1 = caddr[0];
    cf2 = caddr[1];
    mytfi = tf.tf_i[HIOFFSET] & (COSTABSIZE-1);
    saddr = stab + (mytfi);
    caddr = ctab + (mytfi);
    hout = (cf1 + frac * (cf2 - cf1))/(sf1 + frac * (sf2 - sf1));
    *out++ = hout;
    *out++ = hout;
    *out++ = hout;
    *out++ = hout;
    in += 4;
    tf.tf_i[HIOFFSET] = normhipart;
  }/*epilog beg*/
  frac = tf.tf_d - UNITBIT32;
  sf1 = saddr[0];
  sf2 = saddr[1];
  cf1 = caddr[0];
  cf2 = caddr[1];
  hout = (cf1 + frac * (cf2 - cf1))/(sf1 + frac * (sf2 - sf1));
  *out++ = hout;
  *out++ = hout;
  *out++ = hout;
  *out++ = hout;
  /*epilog end*/
#endif
  return (w+5);
}

static void sigiem_cot4_dsp(t_sigiem_cot4 *x, t_signal **sp)
{
  x->x_sr = 2.0f / (float)sp[0]->s_sr;
  dsp_add(sigiem_cot4_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

static void sigiem_cot4_maketable(void)
{
  int i;
  float *fp, phase, fff, phsinc = 0.5*3.141592653 / ((float)COSTABSIZE);
  union tabfudge tf;
  
  if(!iem_cot4_table_sin)
  {
    iem_cot4_table_sin = (float *)getbytes(sizeof(float) * (COSTABSIZE+1));
    for(i=COSTABSIZE+1, fp=iem_cot4_table_sin, phase=0; i--; fp++, phase+=phsinc)
      *fp = sin(phase);
  }
  if(!iem_cot4_table_cos)
  {
    iem_cot4_table_cos = (float *)getbytes(sizeof(float) * (COSTABSIZE+1));
    for(i=COSTABSIZE+1, fp=iem_cot4_table_cos, phase=0; i--; fp++, phase+=phsinc)
      *fp = cos(phase);
  }
  tf.tf_d = UNITBIT32 + 0.5;
  if((unsigned)tf.tf_i[LOWOFFSET] != 0x80000000)
    bug("iem_cot4~: unexpected machine alignment");
}

static void *sigiem_cot4_new(void)
{
  t_sigiem_cot4 *x = (t_sigiem_cot4 *)pd_new(sigiem_cot4_class);
  
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_msi = 0;
  return (x);
}

void sigiem_cot4_setup(void)
{
  sigiem_cot4_class = class_new(gensym("iem_cot4~"), (t_newmethod)sigiem_cot4_new, 0,
    sizeof(t_sigiem_cot4), 0, 0);
  class_addcreator((t_newmethod)sigiem_cot4_new, gensym("iem_cot~"), 0);
  CLASS_MAINSIGNALIN(sigiem_cot4_class, t_sigiem_cot4, x_msi);
  class_addmethod(sigiem_cot4_class, (t_method)sigiem_cot4_dsp, gensym("dsp"), 0);
  sigiem_cot4_maketable();
  class_sethelpsymbol(sigiem_cot4_class, gensym("iemhelp/help-iem_cot4~"));
}
