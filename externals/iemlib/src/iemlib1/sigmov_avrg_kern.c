/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2005 */

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


#include "m_pd.h"
#include "iemlib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>


/* -- mov_avrg_kern~ - kernel for a moving-average-filter with IIR - */

typedef struct sigmov_avrg_kern
{
  t_object x_obj;
  double   x_wn1;
  double   x_a0;
  double   x_sr;
  double   x_mstime;
  int      x_nsamps;
  int      x_counter;
  float    x_msi;
} t_sigmov_avrg_kern;

t_class *sigmov_avrg_kern_class;

static t_int *sigmov_avrg_kern_perform(t_int *w)
{
  float *in_direct = (float *)(w[1]);
  float *in_delayed = (float *)(w[2]);
  float *out = (float *)(w[3]);
  t_sigmov_avrg_kern *x = (t_sigmov_avrg_kern *)(w[4]);
  int i, n = (int)(w[5]);
  double wn0, wn1=x->x_wn1, a0=x->x_a0;
  
  if(x->x_counter)
  {
    int counter = x->x_counter;
    
    if(counter >= n)
    {
      x->x_counter = counter - n;
      for(i=0; i<n; i++)
      {
        wn0 = wn1 + a0*(double)(*in_direct++);
        *out++ = (float)wn0;
        wn1 = wn0;
      }
    }
    else
    {
      x->x_counter = 0;
      for(i=0; i<counter; i++)
      {
        wn0 = wn1 + a0*(double)(*in_direct++);
        *out++ = (float)wn0;
        wn1 = wn0;
      }
      for(i=counter; i<n; i++)
      {
        wn0 = wn1 + a0*(double)(*in_direct++ - *in_delayed++);
        *out++ = (float)wn0;
        wn1 = wn0;
      }
    }
  }
  else
  {
    for(i=0; i<n; i++)
    {
      wn0 = wn1 + a0*(double)(*in_direct++ - *in_delayed++);
      *out++ = (float)wn0;
      wn1 = wn0;
    }
  }
  x->x_wn1 = wn1;
  return(w+6);
}

static void sigmov_avrg_kern_ft1(t_sigmov_avrg_kern *x, t_floatarg mstime)
{
  if(mstime < 0.04)
    mstime = 0.04;
  x->x_mstime = (double)mstime;
  x->x_nsamps = (int)(x->x_sr * x->x_mstime);
  x->x_counter = x->x_nsamps;
  x->x_wn1 = 0.0;
  x->x_a0 = 1.0/(double)(x->x_nsamps);
}

static void sigmov_avrg_kern_reset(t_sigmov_avrg_kern *x)
{
  x->x_counter = x->x_nsamps;
  x->x_wn1 = 0.0;
}

static void sigmov_avrg_kern_dsp(t_sigmov_avrg_kern *x, t_signal **sp)
{
  x->x_sr = 0.001*(double)(sp[0]->s_sr);
  x->x_nsamps = (int)(x->x_sr * x->x_mstime);
  x->x_counter = x->x_nsamps;
  x->x_wn1 = 0.0;
  x->x_a0 = 1.0/(double)(x->x_nsamps);
  dsp_add(sigmov_avrg_kern_perform, 5, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, x, sp[0]->s_n);
}

static void *sigmov_avrg_kern_new(t_floatarg mstime)
{
  t_sigmov_avrg_kern *x = (t_sigmov_avrg_kern *)pd_new(sigmov_avrg_kern_class);
  
  if(mstime < 0.04)
    mstime = 0.04;
  x->x_mstime = (double)mstime;
  x->x_sr = 44.1;
  x->x_nsamps = (int)(x->x_sr * x->x_mstime);
  x->x_counter = x->x_nsamps;
  x->x_wn1 = 0.0;
  x->x_a0 = 1.0/(double)(x->x_nsamps);
  
  inlet_new(&x->x_obj,  &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ft1"));
  outlet_new(&x->x_obj, &s_signal);
  x->x_msi = 0;
  return(x);
}

void sigmov_avrg_kern_setup(void)
{
  sigmov_avrg_kern_class = class_new(gensym("mov_avrg_kern~"), (t_newmethod)sigmov_avrg_kern_new,
        0, sizeof(t_sigmov_avrg_kern), 0, A_FLOAT, 0);
  CLASS_MAINSIGNALIN(sigmov_avrg_kern_class, t_sigmov_avrg_kern, x_msi);
  class_addmethod(sigmov_avrg_kern_class, (t_method)sigmov_avrg_kern_dsp, gensym("dsp"), 0);
  class_addmethod(sigmov_avrg_kern_class, (t_method)sigmov_avrg_kern_ft1, gensym("ft1"), A_FLOAT, 0);
  class_addmethod(sigmov_avrg_kern_class, (t_method)sigmov_avrg_kern_reset, gensym("reset"), 0);
}
