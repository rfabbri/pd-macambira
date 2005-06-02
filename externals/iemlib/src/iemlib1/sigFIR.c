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


/* ---------- FIR~ - FIR-filter with table-coef ----------- */

typedef struct sigFIR
{
  t_object  x_obj;
  float     *x_coef_beg;
  float     *x_history_beg;
  int       x_rw_index;
  int       x_fir_order;
  t_symbol  *x_table_name;
  float     x_msi;
} t_sigFIR;

t_class *sigFIR_class;

static t_int *sigFIR_perform(t_int *w)
{
  float *in = (float *)(w[1]);
  float *out = (float *)(w[2]);
  t_sigFIR *x = (t_sigFIR *)(w[3]);
  int n = (t_int)(w[4]);
  int rw_index = x->x_rw_index;
  int i, j;
  int order = x->x_fir_order;
  int ord16 = order / 16;
  float sum=0.0f;
  float *coef = x->x_coef_beg;
  float *write_hist1=x->x_history_beg;
  float *write_hist2;
  float *read_hist;
  float *coef_vec;
  float *hist_vec;
  
  if(!coef)
    goto sigFIRperfzero;
  
  write_hist1 = x->x_history_beg;
  write_hist2 = write_hist1 + order;
  read_hist = write_hist2;
  
  for(i=0; i<n; i++)
  {
    write_hist1[rw_index] = in[i];
    write_hist2[rw_index] = in[i];
    
    sum = 0.0f;
    coef_vec = coef;
    hist_vec = &read_hist[rw_index];
    for(j=0; j<ord16; j++)
    {
      sum += coef_vec[0] * hist_vec[0];
      sum += coef_vec[1] * hist_vec[-1];
      sum += coef_vec[2] * hist_vec[-2];
      sum += coef_vec[3] * hist_vec[-3];
      sum += coef_vec[4] * hist_vec[-4];
      sum += coef_vec[5] * hist_vec[-5];
      sum += coef_vec[6] * hist_vec[-6];
      sum += coef_vec[7] * hist_vec[-7];
      sum += coef_vec[8] * hist_vec[-8];
      sum += coef_vec[9] * hist_vec[-9];
      sum += coef_vec[10] * hist_vec[-10];
      sum += coef_vec[11] * hist_vec[-11];
      sum += coef_vec[12] * hist_vec[-12];
      sum += coef_vec[13] * hist_vec[-13];
      sum += coef_vec[14] * hist_vec[-14];
      sum += coef_vec[15] * hist_vec[-15];
      coef_vec += 16;
      hist_vec -= 16;
    }
    for(j=ord16*16; j<order; j++)
    {
      sum += coef[j] * read_hist[rw_index-j];
    }
    out[i] = sum;
    
    rw_index++;
    if(rw_index >= order)
      rw_index -= order;
  }
  
  x->x_rw_index = rw_index;
  return(w+5);
  
sigFIRperfzero:
  
  while(n--)
    *out++ = 0.0f;
  return(w+5);
}

void sigFIR_set(t_sigFIR *x, t_symbol *table_name, t_floatarg forder)
{
  t_garray *ga;
  int table_size;
  int order = (int)forder;
  
  x->x_table_name = table_name;
  if(!(ga = (t_garray *)pd_findbyclass(x->x_table_name, garray_class)))
  {
    if(*table_name->s_name)
      error("FIR~: %s: no such table~", x->x_table_name->s_name);
    x->x_coef_beg = 0;
  }
  else if(!garray_getfloatarray(ga, &table_size, &x->x_coef_beg))
  {
    error("%s: bad template for FIR~", x->x_table_name->s_name);
    x->x_coef_beg = 0;
  }
  else if(table_size < order)
  {
    error("FIR~: tablesize %d < order %d !!!!", table_size, order);
    x->x_coef_beg = 0;
  }
  else
    garray_usedindsp(ga);
  x->x_rw_index = 0;
  if(order > x->x_fir_order)/* resize */
    x->x_history_beg =  (float *)resizebytes(x->x_history_beg, 2*x->x_fir_order*sizeof(float), 2*order*sizeof(float));
  x->x_fir_order = order;
}

static void sigFIR_dsp(t_sigFIR *x, t_signal **sp)
{
  sigFIR_set(x, x->x_table_name, x->x_fir_order);
  dsp_add(sigFIR_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

static void *sigFIR_new(t_symbol *ref, t_floatarg np)
{
  t_sigFIR *x = (t_sigFIR *)pd_new(sigFIR_class);
  
  outlet_new(&x->x_obj, &s_signal);
  x->x_msi = 0;
  x->x_table_name = ref;
  x->x_coef_beg = 0;
  if((int)np < 1)
    np = 1.0;
  x->x_fir_order = (int)np;
  x->x_history_beg = (float *)getbytes((2*x->x_fir_order)*sizeof(float));
  x->x_rw_index = 0;
  return(x);
}

static void sigFIR_free(t_sigFIR *x)
{
  if(x->x_history_beg)
    freebytes(x->x_history_beg, (2*x->x_fir_order)*sizeof(float));
}

void sigFIR_setup(void)
{
  sigFIR_class = class_new(gensym("FIR~"), (t_newmethod)sigFIR_new,
    (t_method)sigFIR_free, sizeof(t_sigFIR), 0, A_DEFSYM, A_DEFFLOAT, 0);
  CLASS_MAINSIGNALIN(sigFIR_class, t_sigFIR, x_msi);
  class_addmethod(sigFIR_class, (t_method)sigFIR_dsp, gensym("dsp"), 0);
  class_addmethod(sigFIR_class, (t_method)sigFIR_set,
    gensym("set"), A_SYMBOL, A_FLOAT, 0);
  class_sethelpsymbol(sigFIR_class, gensym("iemhelp/help-FIR~"));
}
