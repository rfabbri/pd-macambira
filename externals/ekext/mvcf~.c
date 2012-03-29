/*
 * moog vcf, 4-pole lowpass resonant filter
 *
 * (c) Edward Kelly 2012
 * BSD License
 */
#include "m_pd.h"
#include <math.h>
#define _limit 0.95

static t_class *mvcf_tilde_class;

typedef struct _mvcf_tilde {
  t_object x_obj;
  t_float b0, b1, b2, b3, b4;  //filter buffers to keep (beware denormals!)
  t_float token, debug, safety;
  t_outlet *lp;
} t_mvcf_tilde;

/* We could have a mode where the fc and res are only registered at the start of the block (quick) or are registered in signal mode (slow) - i.e. a flag */


static inline float saturate( float input ) { //clamp without branching
  float x1 = fabsf( input + _limit );
  float x2 = fabsf( input - _limit );
  return 0.5 * (x1 - x2);
}


t_int *mvcf_tilde_perform(t_int *w) {
  t_mvcf_tilde   *x =   (t_mvcf_tilde *)(w[1]);
  t_sample      *in =       (t_sample *)(w[2]);
  t_sample      *fc =       (t_sample *)(w[3]);
  t_sample     *res =       (t_sample *)(w[4]);
  t_sample     *out =       (t_sample *)(w[5]);
  int             n =              (int)(w[6]);

  //x *b0 =

  /*
  q = 1.0f - frequency;
  p = frequency + 0.8f * frequency * q;
  f = p + p - 1.0f;
  q = resonance * (1.0f + 0.5f * q * (1.0f - q + 5.6f * q * q));
  */

  //	while (n--) {
  //		float f = *(in++);
  //		setup_svf(obj, *(freq++), *(res++));
  //		*(out++) = run_svf(obj, f + ((obj->b) * (*(res++))));
  //	}

  float t1 = 0;
  float t2 = 0;
  float xb0 = x->b0;
  float xb1 = x->b1;
  float xb2 = x->b2;
  float xb3 = x->b3;
  float xb4 = x->b4;
  float i1 = 0;
  float fc1 = 0;
  float res1 = 0;
  float q = 0;
  float p = 0;
  float fcoeff = 0;

  //  while (n-=4) {
  while (n--) {
    i1=(*in++);
    fc1 = (*fc++);
    /* This failsafe line stops the filter bursting
     * ...but it is expensive! */
    if(x->safety) {
      fc1 = fc1 <= 1 ? fc1 >= 0 ? fc1 : 0 : 1;
    }
    res1 = (*res++);
    q = 1.0f - fc1;
    p = fc1 + 0.8f * fc1 * q;
    fcoeff = p + p - 1.0f;
    q = res1 * (1.0f + 0.5f * q * (1.0f - q + 5.6f * q * q));
    i1 -= q * xb4;                          //feedback
    t1 = xb1;
    xb1 = (i1 + xb0) * p - xb1 * fcoeff;
    t2 = xb2;
    xb2 = (xb1 + t1) * p - xb2 * fcoeff;
    t1 = xb3;
    xb3 = (xb2 + t2) * p - xb3 * fcoeff;
    xb4 = (xb3 + t1) * p - xb4 * fcoeff;
    xb4 = saturate(xb4);
    xb4 = xb4 - xb4 * xb4 * xb4 * 0.01f;
    xb0 = i1;
    *out++ = xb4; // lowpass mode
    // *out++ = i1 - x->b4; // highpass mode
// Lowpass  output:  xb4
// Highpass output:  in - xb4;
// Bandpass output:  3.0f * (b3 - xb4);

  }
  x->b0 = xb0;
  x->b1 = xb1;
  x->b2 = xb2;
  x->b3 = xb3;
  x->b4 = xb4;
  if(x->debug != 0) {
    x->token +=1;
    if(x->token == 15) {
      post("q = %f, p=%f, fcoeff=%f, b0=%f, b1=%f, b2=%f, b3=%f, b4=%f",q,p,fcoeff,xb0,xb1,xb2,xb3,xb4);
      x->token = 0;
    }
  }
  return (w+7);
}

void mvcf_tilde_dsp(t_mvcf_tilde *x, t_signal **sp) {
  dsp_add(mvcf_tilde_perform, 6, x, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec, sp[0]->s_n);
}

void mvcf_tilde_safety(t_mvcf_tilde *x, t_floatarg f) {
  x->safety = f != 0 ? 1 : 0;
}

/*void mvcf_tilde_mode(t_mvcf_tilde *x, t_floatarg f) {
x->mode = f < 1 ? 0 : f > 1 ? 2 : 1;
}
 */

void mvcf_tilde_clear(t_mvcf_tilde *x) {
  x->b0 = 0;
  x->b1 = 0;
  x->b2 = 0;
  x->b3 = 0;
  x->b4 = 0;
}

void mvcf_tilde_debug(t_mvcf_tilde *x, t_floatarg f) {
  x->debug = f != 0 ? 1 : 0;
}

void *mvcf_tilde_new(t_floatarg f) {
  t_mvcf_tilde *x = (t_mvcf_tilde *)pd_new(mvcf_tilde_class);

  x->b0 = 0;
  x->b1 = 0;
  x->b2 = 0;
  x->b3 = 0;
  x->b4 = 0;
  x->token = 0;
  x->safety = 1;

  inlet_new (&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  inlet_new (&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, &s_signal);
  return (void *)x;
}

void mvcf_tilde_setup(void) {
  mvcf_tilde_class = class_new(gensym("mvcf~"), 
  (t_newmethod)mvcf_tilde_new, 
  0, sizeof(t_mvcf_tilde),
  CLASS_DEFAULT, A_DEFFLOAT, 0);

  post("~~~~~~~~~~~~~~~>mvcf~");
  post("~~~>by Ed Kelly, 2012");

  class_addmethod(mvcf_tilde_class,
  (t_method)mvcf_tilde_dsp, gensym("dsp"), 0);
  CLASS_MAINSIGNALIN(mvcf_tilde_class, t_mvcf_tilde, token);
  class_addmethod(mvcf_tilde_class, (t_method)mvcf_tilde_clear, gensym("clear"), 0);
  class_addmethod(mvcf_tilde_class, (t_method)mvcf_tilde_debug, gensym("debug"), 0);
  class_addmethod(mvcf_tilde_class, (t_method)mvcf_tilde_safety, gensym("safe"), 0);
  //  class_addmethod(mvcf_tilde_class, (t_method)mvcf_tilde_mode, gensym("mode"), A_DEFFLOAT, 0);
}
