/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_t3_lib written by Gerhard Eckel, Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2005 */

#include "m_pd.h"
#include "iemlib.h"

/* -------------------------- sigt3_sig~ ------------------------------ */
static t_class *sigt3_sig_class;

typedef struct _sigt3_sig
{
  t_object x_obj;
  t_clock  *x_clock;
  t_float  x_old_val;
  t_float  x_new_val;
  t_float  *x_beg;
  int      x_n;
  int      x_t3_bang_samps;
  int      x_transient;
  t_float  x_ms2samps;
  t_float  x_ticks2ms;
} t_sigt3_sig;

static void sigt3_sig_tick(t_sigt3_sig *x)
{
  t_float *trans = x->x_beg, val;
  int n = x->x_n, t3_bang_samps, i;
  
  t3_bang_samps = x->x_t3_bang_samps;
  if(!x->x_transient)
  {
    val = x->x_old_val;
    for(i=0; i<t3_bang_samps; i++)
      trans[i] = val;
    x->x_transient = 1;
  }
  val = x->x_old_val = x->x_new_val;
  for(i=t3_bang_samps; i<n; i++)
    trans[i] = val;
}

static void sigt3_sig_stop(t_sigt3_sig *x)
{
  clock_unset(x->x_clock);
  x->x_new_val = x->x_old_val;
}

static void sigt3_sig_list(t_sigt3_sig *x, t_symbol *s, int ac, t_atom *av)
{
  if((ac == 2)&&IS_A_FLOAT(av,0)&&IS_A_FLOAT(av,1))
  {
    int n = x->x_n, t3_bang_samps, ticks;
    
    t3_bang_samps = (int)((t_float)atom_getfloatarg(0, ac, av)*x->x_ms2samps);
    x->x_new_val = (t_float)atom_getfloatarg(1, ac, av);
    if(t3_bang_samps < 0)
      t3_bang_samps = 0;
    ticks = t3_bang_samps / n;
    x->x_t3_bang_samps = t3_bang_samps - n*ticks;
    if(ticks < 1)
      sigt3_sig_tick(x);
    else
      clock_delay(x->x_clock, (double)ticks * (double)x->x_ticks2ms);
  }
}

static t_int *sigt3_sig_perform(t_int *w)
{
  t_float *out = (t_float *)(w[1]);
  t_sigt3_sig *x = (t_sigt3_sig *)(w[2]);
  int n = (int)(w[3]);
  
  if(x->x_transient)
  {
    t_float *trans = x->x_beg;
    
    while(n--)
      *out++ = *trans++;
    x->x_transient = 0;
  }
  else
  {
    t_float val = x->x_new_val;
    
    while(n--)
      *out++ = val;
  }
  return (w+4);
}

static t_int *sigt3_sig_perf8(t_int *w)
{
  t_float *out = (t_float *)(w[1]);
  t_sigt3_sig *x = (t_sigt3_sig *)(w[2]);
  int n = (int)(w[3]), i;
  
  if(x->x_transient)
  {
    t_float *trans = x->x_beg;
    
    for(i=0; i<n; i+=8, out+=8, trans+=8)
    {
      out[0] = trans[0];
      out[1] = trans[1];
      out[2] = trans[2];
      out[3] = trans[3];
      out[4] = trans[4];
      out[5] = trans[5];
      out[6] = trans[6];
      out[7] = trans[7];
    }
    x->x_transient = 0;
  }
  else
  {
    t_float val = x->x_new_val;
    
    for(i=0; i<n; i+=8, out+=8)
    {
      out[0] = val;
      out[1] = val;
      out[2] = val;
      out[3] = val;
      out[4] = val;
      out[5] = val;
      out[6] = val;
      out[7] = val;
    }
  }
  return (w+4);
}

static void sigt3_sig_dsp(t_sigt3_sig *x, t_signal **sp)
{
  int i;
  t_float *trans, val;
  
  if(sp[0]->s_n > x->x_n)
  {
    freebytes(x->x_beg, x->x_n*sizeof(t_float));
    x->x_n = sp[0]->s_n;
    x->x_beg = (t_float *)getbytes(x->x_n*sizeof(t_float));
  }
  else
    x->x_n = sp[0]->s_n;
  x->x_ms2samps = 0.001*(t_float)sp[0]->s_sr;
  x->x_ticks2ms = (t_float)x->x_n / x->x_ms2samps;
  i = x->x_n;
  val = x->x_new_val;
  trans = x->x_beg;
  while(i--)
    *trans++ = val;
  
  if((sp[0]->s_n)&7)
    dsp_add(sigt3_sig_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
  else
    dsp_add(sigt3_sig_perf8, 3, sp[0]->s_vec, x, sp[0]->s_n);
}

static void sigt3_sig_free(t_sigt3_sig *x)
{
  if(x->x_beg)
    freebytes(x->x_beg, x->x_n*sizeof(t_float));
  clock_free(x->x_clock);
}

static void *sigt3_sig_new(t_floatarg init_val)
{
  t_sigt3_sig *x = (t_sigt3_sig *)pd_new(sigt3_sig_class);
  
  x->x_new_val = x->x_old_val = init_val;
  x->x_n = (int)sys_getblksize();
  x->x_beg = (t_float *)getbytes(x->x_n*sizeof(t_float));
  x->x_t3_bang_samps = x->x_transient = 0;
  x->x_ms2samps = 0.001 * (t_float)sys_getsr();
  x->x_ticks2ms = (t_float)x->x_n / x->x_ms2samps;
  x->x_clock = clock_new(x, (t_method)sigt3_sig_tick);
  outlet_new(&x->x_obj, &s_signal);
  return (x);
}

void sigt3_sig_setup(void)
{
  sigt3_sig_class = class_new(gensym("t3_sig~"), (t_newmethod)sigt3_sig_new,
        (t_method)sigt3_sig_free, sizeof(t_sigt3_sig), 0, A_DEFFLOAT, 0);
  class_addmethod(sigt3_sig_class, (t_method)sigt3_sig_dsp, gensym("dsp"), 0);
  class_addmethod(sigt3_sig_class, (t_method)sigt3_sig_stop, gensym("stop"), 0);
  class_addlist(sigt3_sig_class, (t_method)sigt3_sig_list);
  class_sethelpsymbol(sigt3_sig_class, gensym("iemhelp/help-t3_sig~"));
}
