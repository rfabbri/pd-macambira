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

/* ------------------------ iem_pow4~ ----------------------------- */

static t_class *sigiem_pow4_class;

typedef struct _sigiem_pow4
{
  t_object  x_obj;
  float     x_exp;
  float     x_msi;
} t_sigiem_pow4;

static void sigiem_pow4_ft1(t_sigiem_pow4 *x, t_floatarg f)
{
  x->x_exp = f;
}

static t_int *sigiem_pow4_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_sigiem_pow4 *x = (t_sigiem_pow4 *)(w[3]);
  t_float y=x->x_exp;
  t_float f, g;
  int n = (int)(w[4])/4;
  
  while (n--)
  {
    f = *in;
    if(f < 0.01f)
      f = 0.01f;
    else if(f > 1000.0f)
      f = 1000.0f;
    g = log(f);
    f = exp(g*y);
    *out++ = f;
    *out++ = f;
    *out++ = f;
    *out++ = f;
    in += 4;
  }
  return (w+5);
}

static void sigiem_pow4_dsp(t_sigiem_pow4 *x, t_signal **sp)
{
  dsp_add(sigiem_pow4_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

static void *sigiem_pow4_new(t_floatarg f)
{
  t_sigiem_pow4 *x = (t_sigiem_pow4 *)pd_new(sigiem_pow4_class);
  
  x->x_exp = f;
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_msi = 0;
  return (x);
}

void sigiem_pow4_setup(void)
{
  sigiem_pow4_class = class_new(gensym("iem_pow4~"), (t_newmethod)sigiem_pow4_new, 0,
    sizeof(t_sigiem_pow4), 0, A_DEFFLOAT, 0);
  class_addcreator((t_newmethod)sigiem_pow4_new, gensym("icot~"), 0);
  CLASS_MAINSIGNALIN(sigiem_pow4_class, t_sigiem_pow4, x_msi);
  class_addmethod(sigiem_pow4_class, (t_method)sigiem_pow4_dsp, gensym("dsp"), 0);
  class_addmethod(sigiem_pow4_class, (t_method)sigiem_pow4_ft1, gensym("ft1"), A_FLOAT, 0);
  class_sethelpsymbol(sigiem_pow4_class, gensym("iemhelp/help-iem_pow4~"));
}
