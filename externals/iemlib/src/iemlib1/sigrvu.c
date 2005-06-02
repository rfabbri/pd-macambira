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

/* ---------------- rvu~ - simple peak&rms-vu-meter. ----------------- */

typedef struct sigrvu
{
  t_object  x_obj;
  void      *x_clock_metro;
  int       x_metro_time;
  float     x_sum_rms;
  float     x_old_rms;
  float     x_rcp;
  float     x_sr;
  float     x_release_time;
  float     x_c1;
  int       x_started;
  float     x_msi;
} t_sigrvu;

t_class *sigrvu_class;
static void sigrvu_tick_metro(t_sigrvu *x);

static void sigrvu_reset(t_sigrvu *x)
{
  outlet_float(x->x_obj.ob_outlet, -99.9f);
  x->x_sum_rms = 0.0f;
  x->x_old_rms = 0.0f;
  clock_delay(x->x_clock_metro, x->x_metro_time);
}

static void sigrvu_stop(t_sigrvu *x)
{
  clock_unset(x->x_clock_metro);
  x->x_started = 0;
}

static void sigrvu_start(t_sigrvu *x)
{
  clock_delay(x->x_clock_metro, x->x_metro_time);
  x->x_started = 1;
}

static void sigrvu_float(t_sigrvu *x, t_floatarg f)
{
  if(f == 0.0)
  {
    clock_unset(x->x_clock_metro);
    x->x_started = 0;
  }
  else
  {
    clock_delay(x->x_clock_metro, x->x_metro_time);
    x->x_started = 1;
  }
}

static void sigrvu_t_release(t_sigrvu *x, t_floatarg release_time)
{
  if(release_time <= 20.0f)
    release_time = 20.0f;
  x->x_release_time = release_time;
  x->x_c1 = exp(-2.0*(float)x->x_metro_time/x->x_release_time);
}

static void sigrvu_t_metro(t_sigrvu *x, t_floatarg metro_time)
{
  if(metro_time <= 20.0f)
    metro_time = 20.0f;
  x->x_metro_time = (int)metro_time;
  x->x_c1 = exp(-2.0*(float)x->x_metro_time/x->x_release_time);
  x->x_rcp = 1.0f/(x->x_sr*(float)x->x_metro_time);
}

static t_int *sigrvu_perform(t_int *w)
{
  float *in = (float *)(w[1]);
  t_sigrvu *x = (t_sigrvu *)(w[2]);
  int n = (int)(w[3]);
  float pow, sum=x->x_sum_rms;
  int i;
  
  if(x->x_started)
  {
    for(i=0; i<n; i++)
    {
      sum += in[i]*in[i];
    }
    x->x_sum_rms = sum;
  }
  return(w+4);
}

static void sigrvu_dsp(t_sigrvu *x, t_signal **sp)
{
  x->x_sr = 0.001*(float)sp[0]->s_sr;
  x->x_rcp = 1.0f/(x->x_sr*(float)x->x_metro_time);
  dsp_add(sigrvu_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
  clock_delay(x->x_clock_metro, x->x_metro_time);
}

static void sigrvu_tick_metro(t_sigrvu *x)
{
  float dbr, cur_rms, c1=x->x_c1;
  
  cur_rms = (1.0f - c1)*x->x_sum_rms*x->x_rcp + c1*x->x_old_rms;
  /* NAN protect */
  if(IEM_DENORMAL(cur_rms))
    cur_rms = 0.0f;
  
  if(cur_rms <= 0.0000000001f)
    dbr = -99.9f;
  else if(cur_rms > 1000000.0f)
  {
    dbr = 60.0f;
    x->x_old_rms = 1000000.0f;
  }
  else
    dbr = 4.3429448195f*log(cur_rms);
  x->x_sum_rms = 0.0f;
  x->x_old_rms = cur_rms;
  outlet_float(x->x_obj.ob_outlet, dbr);
  clock_delay(x->x_clock_metro, x->x_metro_time);
}

static void sigrvu_ff(t_sigrvu *x)
{
  clock_free(x->x_clock_metro);
}

static void *sigrvu_new(t_floatarg metro_time, t_floatarg release_time)
{
  t_sigrvu *x=(t_sigrvu *)pd_new(sigrvu_class);
  float t;
  int i;
  
  if(metro_time <= 0.0f)
    metro_time = 300.0f;
  if(metro_time <= 20.0f)
    metro_time = 20.0f;
  if(release_time <= 0.0f)
    release_time = 300.0f;
  if(release_time <= 20.0f)
    release_time = 20.0f;
  x->x_metro_time = (int)metro_time;
  x->x_release_time = release_time;
  x->x_c1 = exp(-2.0*(float)x->x_metro_time/x->x_release_time);
  x->x_sum_rms = 0.0f;
  x->x_old_rms = 0.0f;
  x->x_sr = 44.1f;
  x->x_rcp = 1.0f/(x->x_sr*(float)x->x_metro_time);
  x->x_clock_metro = clock_new(x, (t_method)sigrvu_tick_metro);
  x->x_started = 1;
  outlet_new(&x->x_obj, &s_float);
  x->x_msi = 0.0f;
  return(x);
}

void sigrvu_setup(void)
{
  sigrvu_class = class_new(gensym("rvu~"), (t_newmethod)sigrvu_new,
    (t_method)sigrvu_ff, sizeof(t_sigrvu), 0,
    A_DEFFLOAT, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(sigrvu_class, t_sigrvu, x_msi);
  class_addmethod(sigrvu_class, (t_method)sigrvu_dsp, gensym("dsp"), 0);
  class_addfloat(sigrvu_class, sigrvu_float);
  class_addmethod(sigrvu_class, (t_method)sigrvu_reset, gensym("reset"), 0);
  class_addmethod(sigrvu_class, (t_method)sigrvu_start, gensym("start"), 0);
  class_addmethod(sigrvu_class, (t_method)sigrvu_stop, gensym("stop"), 0);
  class_addmethod(sigrvu_class, (t_method)sigrvu_t_release, gensym("t_release"), A_FLOAT, 0);
  class_addmethod(sigrvu_class, (t_method)sigrvu_t_metro, gensym("t_metro"), A_FLOAT, 0);
  class_sethelpsymbol(sigrvu_class, gensym("iemhelp/help-rvu~"));
}
