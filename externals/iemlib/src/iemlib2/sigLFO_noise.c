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

/* -------------------- LFO_noise~ --------------------- */
/* ---- outputs a 2 point interpolated white noise ----- */
/* -- with lower cutoff frequency than 0.5 samplerate -- */

static t_class *sigLFO_noise_class;

typedef struct _sigLFO_noise
{
  t_object     x_obj;
  double       x_range;
  double       x_rcp_range;
  unsigned int x_state;
  float        x_fact;
  float        x_incr;
  float        x_y1;
  float        x_y2;
  float        x_phase;
} t_sigLFO_noise;

static int sigLFO_noise_makeseed(void)
{
  static unsigned int sigLFO_noise_nextseed = 1489853723;
  
  sigLFO_noise_nextseed = sigLFO_noise_nextseed * 435898247 + 938284287;
  return(sigLFO_noise_nextseed & 0x7fffffff);
}

static float sigLFO_noise_new_rand(t_sigLFO_noise *x)
{
  unsigned int state = x->x_state;
  double new_val, range = x->x_range;
  
  x->x_state = state = state * 472940017 + 832416023;
  new_val = range * ((double)state) * (1./4294967296.);
  if(new_val >= range)
    new_val = range-1;
  new_val -= 32767.0;
  return(new_val*(1.0/32767.0));
}

static void *sigLFO_noise_new(t_float freq)
{
  t_sigLFO_noise *x = (t_sigLFO_noise *)pd_new(sigLFO_noise_class);
  
  x->x_range = 65535.0;
  x->x_rcp_range =  (double)x->x_range * (1.0/4294967296.0);
  x->x_state = sigLFO_noise_makeseed();
  x->x_fact = 2. / 44100.0;
  x->x_incr = freq * x->x_fact;
  if(x->x_incr < 0.0)
    x->x_incr = 0.0;
  else if(x->x_incr > 0.1)
    x->x_incr = 0.1;
  x->x_y1 = sigLFO_noise_new_rand(x);
  x->x_y2 = sigLFO_noise_new_rand(x);
  x->x_phase = 0.0;
  outlet_new(&x->x_obj, gensym("signal"));
  return (x);
}

static t_int *sigLFO_noise_perform(t_int *w)
{
  t_float *out = (t_float *)(w[1]);
  t_sigLFO_noise *x = (t_sigLFO_noise *)(w[2]);
  int n = (int)(w[3]);
  float phase = x->x_phase;
  float x_y1 = x->x_y1;
  float x_y2 = x->x_y2;
  float incr = x->x_incr;
  
  while(n--)
  {
    if(phase > 1.0)
    {
      x_y1 = x_y2;
      x_y2 = sigLFO_noise_new_rand(x);
      phase -= 1.0;
    }
    *out++ = (x_y2 - x_y1) * phase + x_y1;
    phase += incr;
  }
  x->x_phase = phase;
  x->x_y1 = x_y1;
  x->x_y2 = x_y2;
  return (w+4);
}

static void sigLFO_noise_float(t_sigLFO_noise *x, t_float freq)
{
  x->x_incr = freq * x->x_fact;
  if(x->x_incr < 0.0)
    x->x_incr = 0.0;
  else if(x->x_incr > 0.1)
    x->x_incr = 0.1;
}

static void sigLFO_noise_dsp(t_sigLFO_noise *x, t_signal **sp)
{
  x->x_fact = 2. / sp[0]->s_sr;
  dsp_add(sigLFO_noise_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
}

void sigLFO_noise_setup(void)
{
  sigLFO_noise_class = class_new(gensym("LFO_noise~"),
    (t_newmethod)sigLFO_noise_new, 0,
    sizeof(t_sigLFO_noise), 0, A_DEFFLOAT, 0);
  class_addmethod(sigLFO_noise_class, (t_method)sigLFO_noise_dsp,
    gensym("dsp"), 0);
  class_addfloat(sigLFO_noise_class, (t_method)sigLFO_noise_float);
  class_sethelpsymbol(sigLFO_noise_class, gensym("iemhelp/help-LFO_noise~"));
}
