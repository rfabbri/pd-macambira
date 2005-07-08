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

#define IEMSQRT4TAB1SIZE 256
#define IEMSQRT4TAB2SIZE 1024

/* ------------------------ iem_sqrt4~ ----------------------------- */

float *iem_sqrt4_exptab=(float *)0L;
float *iem_sqrt4_mantissatab=(float *)0L;

static t_class *sigiem_sqrt4_class;

typedef struct _sigiem_sqrt4
{
  t_object  x_obj;
  float     x_msi;
} t_sigiem_sqrt4;

static t_int *sigiem_sqrt4_perform(t_int *w)
{
  t_float *in = (t_float *)(w[1]);
  t_float *out = (t_float *)(w[2]);
  t_int n = (t_int)(w[3])/4;
  
  while(n--)
  { 
    float f = *in;
    float g, h;
    long l = *(long *)(in);
    
    if(f < 0.0f)
    {
      *out++ = 0.0f;
      *out++ = 0.0f;
      *out++ = 0.0f;
      *out++ = 0.0f;
    }
    else
    {
      g = iem_sqrt4_exptab[(l >> 23) & 0xff] * iem_sqrt4_mantissatab[(l >> 13) & 0x3ff];
      h = f * (1.5f * g - 0.5f * g * g * g * f);
      *out++ = h;
      *out++ = h;
      *out++ = h;
      *out++ = h;
    }
    in += 4;
  }
  return(w+4);
}

static void sigiem_sqrt4_dsp(t_sigiem_sqrt4 *x, t_signal **sp)
{
  dsp_add(sigiem_sqrt4_perform, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void sigiem_sqrt4_maketable(void)
{
  int i;
  float f;
  long l;
  
  if(!iem_sqrt4_exptab)
  {
    iem_sqrt4_exptab = (float *)getbytes(sizeof(float) * IEMSQRT4TAB1SIZE);
    for(i=0; i<IEMSQRT4TAB1SIZE; i++)
    {
      l = (i ? (i == IEMSQRT4TAB1SIZE-1 ? IEMSQRT4TAB1SIZE-2 : i) : 1)<< 23;
      *(long *)(&f) = l;
      iem_sqrt4_exptab[i] = 1.0f/sqrt(f); 
    }
  }
  if(!iem_sqrt4_mantissatab)
  {
    iem_sqrt4_mantissatab = (float *)getbytes(sizeof(float) * IEMSQRT4TAB2SIZE);
    for(i=0; i<IEMSQRT4TAB2SIZE; i++)
    {
      f = 1.0f + (1.0f/(float)IEMSQRT4TAB2SIZE) * (float)i;
      iem_sqrt4_mantissatab[i] = 1.0f/sqrt(f);  
    }
  }
}

static void *sigiem_sqrt4_new(void)
{
  t_sigiem_sqrt4 *x = (t_sigiem_sqrt4 *)pd_new(sigiem_sqrt4_class);
  
  outlet_new(&x->x_obj, gensym("signal"));
  x->x_msi = 0;
  return (x);
}

void sigiem_sqrt4_setup(void)
{
  sigiem_sqrt4_class = class_new(gensym("iem_sqrt4~"), (t_newmethod)sigiem_sqrt4_new, 0,
    sizeof(t_sigiem_sqrt4), 0, 0);
  CLASS_MAINSIGNALIN(sigiem_sqrt4_class, t_sigiem_sqrt4, x_msi);
  class_addmethod(sigiem_sqrt4_class, (t_method)sigiem_sqrt4_dsp, gensym("dsp"), 0);
  sigiem_sqrt4_maketable();
  class_sethelpsymbol(sigiem_sqrt4_class, gensym("iemhelp/help-iem_sqrt4~"));
}
