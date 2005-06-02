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

/* ---------------- prvu~ - simple peak&rms-vu-meter. ----------------- */

typedef struct sigprvu
{
  t_object  x_obj;
  t_atom    x_at[3];
  void      *x_clock_metro;
  int       x_metro_time;
  void      *x_clock_hold;
  int       x_hold_time;
  float     x_cur_peak;
  float     x_old_peak;
  float     x_hold_peak;
  int       x_hold;
  float     x_sum_rms;
  float     x_old_rms;
  float     x_rcp;
  float     x_sr;
  float     x_threshold_over;
  int       x_overflow_counter;
  float     x_release_time;
  float     x_c1;
  int       x_started;
  float     x_msi;
} t_sigprvu;

t_class *sigprvu_class;
static void sigprvu_tick_metro(t_sigprvu *x);
static void sigprvu_tick_hold(t_sigprvu *x);

static void sigprvu_reset(t_sigprvu *x)
{
  x->x_at[0].a_w.w_float = -99.9f;
  x->x_at[1].a_w.w_float = -99.9f;
  x->x_at[2].a_w.w_float = 0.0f;
  outlet_list(x->x_obj.ob_outlet, &s_list, 3, x->x_at);
  x->x_overflow_counter = 0;
  x->x_cur_peak = 0.0f;
  x->x_old_peak = 0.0f;
  x->x_hold_peak = 0.0f;
  x->x_sum_rms = 0.0f;
  x->x_old_rms = 0.0f;
  x->x_hold = 0;
  clock_unset(x->x_clock_hold);
  clock_delay(x->x_clock_metro, x->x_metro_time);
}

static void sigprvu_stop(t_sigprvu *x)
{
  clock_unset(x->x_clock_metro);
  x->x_started = 0;
}

static void sigprvu_start(t_sigprvu *x)
{
  clock_delay(x->x_clock_metro, x->x_metro_time);
  x->x_started = 1;
}

static void sigprvu_float(t_sigprvu *x, t_floatarg f)
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

static void sigprvu_t_release(t_sigprvu *x, t_floatarg release_time)
{
  if(release_time <= 20.0f)
    release_time = 20.0f;
  x->x_release_time = release_time;
  x->x_c1 = exp(-2.0*(float)x->x_metro_time/x->x_release_time);
}

static void sigprvu_t_metro(t_sigprvu *x, t_floatarg metro_time)
{
  if(metro_time <= 20.0f)
    metro_time = 20.0f;
  x->x_metro_time = (int)metro_time;
  x->x_c1 = exp(-2.0*(float)x->x_metro_time/x->x_release_time);
  x->x_rcp = 1.0f/(x->x_sr*(float)x->x_metro_time);
}

static void sigprvu_t_hold(t_sigprvu *x, t_floatarg hold_time)
{
  if(hold_time <= 20.0f)
    hold_time = 20.0f;
  x->x_hold_time = (int)hold_time;
}

static void sigprvu_threshold(t_sigprvu *x, t_floatarg thresh)
{
  x->x_threshold_over = thresh;
}

static t_int *sigprvu_perform(t_int *w)
{
  float *in = (float *)(w[1]);
  t_sigprvu *x = (t_sigprvu *)(w[2]);
  int n = (int)(w[3]);
  float peak = x->x_cur_peak, pow, sum=x->x_sum_rms;
  int i;
  
  if(x->x_started)
  {
    for(i=0; i<n; i++)
    {
      pow = in[i]*in[i];
      if(pow > peak)
        peak = pow;
      sum += pow;
    }
    x->x_cur_peak = peak;
    x->x_sum_rms = sum;
  }
  return(w+4);
}

static void sigprvu_dsp(t_sigprvu *x, t_signal **sp)
{
  x->x_sr = 0.001*(float)sp[0]->s_sr;
  x->x_rcp = 1.0f/(x->x_sr*(float)x->x_metro_time);
  dsp_add(sigprvu_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
  clock_delay(x->x_clock_metro, x->x_metro_time);
}

static void sigprvu_tick_hold(t_sigprvu *x)
{
  x->x_hold = 0;
  x->x_hold_peak = x->x_old_peak;
}

static void sigprvu_tick_metro(t_sigprvu *x)
{
  float dbr, dbp, cur_rms, c1=x->x_c1;
  
  x->x_old_peak *= c1;
  /* NAN protect */
  if(IEM_DENORMAL(x->x_old_peak))
    x->x_old_peak = 0.0f;
  
  if(x->x_cur_peak > x->x_old_peak)
    x->x_old_peak = x->x_cur_peak;
  if(x->x_old_peak > x->x_hold_peak)
  {
    x->x_hold = 1;
    x->x_hold_peak = x->x_old_peak;
    clock_delay(x->x_clock_hold, x->x_hold_time);
  }
  if(!x->x_hold)
    x->x_hold_peak = x->x_old_peak;
  if(x->x_hold_peak <= 0.0000000001f)
    dbp = -99.9f;
  else if(x->x_hold_peak > 1000000.0f)
  {
    dbp = 60.0f;
    x->x_hold_peak = 1000000.0f;
    x->x_old_peak = 1000000.0f;
  }
  else
    dbp = 4.3429448195f*log(x->x_hold_peak);
  x->x_cur_peak = 0.0f;
  if(dbp >= x->x_threshold_over)
    x->x_overflow_counter++;
  x->x_at[1].a_w.w_float = dbp;
  x->x_at[2].a_w.w_float = (float)x->x_overflow_counter;
  
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
  x->x_at[0].a_w.w_float = dbr;
  outlet_list(x->x_obj.ob_outlet, &s_list, 3, x->x_at);
  clock_delay(x->x_clock_metro, x->x_metro_time);
}

static void sigprvu_ff(t_sigprvu *x)
{
  clock_free(x->x_clock_metro);
  clock_free(x->x_clock_hold);
}

static void *sigprvu_new(t_floatarg metro_time, t_floatarg hold_time,
                         t_floatarg release_time, t_floatarg threshold)
{
  t_sigprvu *x;
  float t;
  int i;
  
  x = (t_sigprvu *)pd_new(sigprvu_class);
  if(metro_time <= 0.0f)
    metro_time = 300.0f;
  if(metro_time <= 20.0f)
    metro_time = 20.0f;
  if(release_time <= 0.0f)
    release_time = 300.0f;
  if(release_time <= 20.0f)
    release_time = 20.0f;
  if(hold_time <= 0.0f)
    hold_time = 1000.0f;
  if(hold_time <= 20.0f)
    hold_time = 20.0f;
  if(threshold == 0.0f)
    threshold = -0.01f;
  x->x_metro_time = (int)metro_time;
  x->x_release_time = release_time;
  x->x_hold_time = (int)hold_time;
  x->x_threshold_over = threshold;
  x->x_c1 = exp(-2.0*(float)x->x_metro_time/x->x_release_time);
  x->x_cur_peak = 0.0f;
  x->x_old_peak = 0.0f;
  x->x_hold_peak = 0.0f;
  x->x_hold = 0;
  x->x_sum_rms = 0.0f;
  x->x_old_rms = 0.0f;
  x->x_sr = 44.1f;
  x->x_rcp = 1.0f/(x->x_sr*(float)x->x_metro_time);
  x->x_overflow_counter = 0;
  x->x_clock_metro = clock_new(x, (t_method)sigprvu_tick_metro);
  x->x_clock_hold = clock_new(x, (t_method)sigprvu_tick_hold);
  x->x_started = 1;
  outlet_new(&x->x_obj, &s_list);
  x->x_at[0].a_type = A_FLOAT;
  x->x_at[1].a_type = A_FLOAT;
  x->x_at[2].a_type = A_FLOAT;
  x->x_msi = 0.0f;
  return(x);
}

void sigprvu_setup(void)
{
  sigprvu_class = class_new(gensym("prvu~"), (t_newmethod)sigprvu_new,
    (t_method)sigprvu_ff, sizeof(t_sigprvu), 0,
    A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(sigprvu_class, t_sigprvu, x_msi);
  class_addmethod(sigprvu_class, (t_method)sigprvu_dsp, gensym("dsp"), 0);
  class_addfloat(sigprvu_class, sigprvu_float);
  class_addmethod(sigprvu_class, (t_method)sigprvu_reset, gensym("reset"), 0);
  class_addmethod(sigprvu_class, (t_method)sigprvu_start, gensym("start"), 0);
  class_addmethod(sigprvu_class, (t_method)sigprvu_stop, gensym("stop"), 0);
  class_addmethod(sigprvu_class, (t_method)sigprvu_t_release, gensym("t_release"), A_FLOAT, 0);
  class_addmethod(sigprvu_class, (t_method)sigprvu_t_metro, gensym("t_metro"), A_FLOAT, 0);
  class_addmethod(sigprvu_class, (t_method)sigprvu_t_hold, gensym("t_hold"), A_FLOAT, 0);
  class_addmethod(sigprvu_class, (t_method)sigprvu_threshold, gensym("threshold"), A_FLOAT, 0);
  class_sethelpsymbol(sigprvu_class, gensym("iemhelp/help-prvu~"));
}
