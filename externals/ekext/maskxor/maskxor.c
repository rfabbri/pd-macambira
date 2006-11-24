#include "m_pd.h"
#include <math.h>
#define MAXENTRIES 512

static t_class *maskxor_class;

typedef struct _mask
{
  t_atom maskxor[MAXENTRIES];
  t_atom maskl[MAXENTRIES];
  t_atom maskr[MAXENTRIES];
} t_mask;

typedef struct _maskxor
{
  t_object x_obj;
  t_mask masking;
  t_float total;
  t_float f_in, yes;
  t_outlet *thru, *bool, *mask;
} t_maskxor;

void maskxor_float(t_maskxor *x, t_floatarg fin)
{
  int input = (int)fin;
  if(input>=0 && input<MAXENTRIES)
    {
      x->f_in = fin;
      x->yes = atom_getfloatarg(input, MAXENTRIES, x->masking.maskxor);
      outlet_float(x->bool, x->yes);
      if(x->yes) outlet_float(x->thru, x->f_in);
    }
}

void maskxor_bang(t_maskxor *x, t_symbol *s)
{
  outlet_list(x->mask, &s_list, x->total, x->masking.maskxor);
  outlet_float(x->bool, x->yes);
  if(x->yes)outlet_float(x->thru, x->f_in);
}

void maskxor_listl(t_maskxor *x, t_symbol *s, int argc, t_atom *argv)
{
  int i;
  float listr_element, listl_element, xor_element;
  if(x->total < argc && argc < MAXENTRIES) x->total = argc;
  for(i=0;i<x->total;i++)
    {
      listl_element = atom_getfloat(argv+i);
      SETFLOAT(&x->masking.maskl[i], listl_element);
      listr_element = atom_getfloatarg(i,MAXENTRIES,x->masking.maskr); 
      xor_element = (float)((int)listl_element ^ (int)listr_element);
      SETFLOAT(&x->masking.maskxor[i], xor_element);
    }
  outlet_list(x->mask, &s_list, x->total, x->masking.maskxor);
}

void maskxor_listr(t_maskxor *x, t_symbol *s, int argc, t_atom *argv)
{
  int i;
  float listr_element;
  if(x->total < argc && argc < MAXENTRIES) 
    {
      x->total = argc;
    }
  for(i=0;i<x->total;i++)
    {
      listr_element = atom_getfloat(argv+i);
      SETFLOAT(&x->masking.maskr[i], listr_element);
    }
}

void maskxor_clear(t_maskxor *x)
{
  int i;
  for(i=0;i<=MAXENTRIES;i++)
    {
      SETFLOAT(&x->masking.maskl[i], 0);
      SETFLOAT(&x->masking.maskr[i], 0);
      SETFLOAT(&x->masking.maskxor[i], 0);
    }
  x->total=1;
  x->yes = x->f_in = 0;
}

void *maskxor_new(t_symbol *s)
{
  int i;
  t_maskxor *x = (t_maskxor *)pd_new(maskxor_class);
  x->total = 1;
  for(i=0;i<MAXENTRIES;i++)
    {
      SETFLOAT(&x->masking.maskr[i], 0);
      SETFLOAT(&x->masking.maskl[i], 0);
      SETFLOAT(&x->masking.maskxor[i], 0);
    }
  x->thru = outlet_new(&x->x_obj, &s_float);
  x->bool = outlet_new(&x->x_obj, &s_float);
  x->mask = outlet_new(&x->x_obj, &s_list);
  return (void *)x;
}

void maskxor_setup(void) 
{
  maskxor_class = class_new(gensym("maskxor"),
  (t_newmethod)maskxor_new,
  0, sizeof(t_maskxor),
  CLASS_DEFAULT, A_DEFFLOAT, 0);
  post("|..-.--.-..-maskxor.-...--.-..|");
  post("|    exclusive-or mask-map    |");
  post("|.--.- Edward Kelly 2006 ---.-|");
  class_sethelpsymbol(maskxor_class, gensym("help-maskxor"));
  class_addfloat(maskxor_class, maskxor_float);
  class_addmethod(maskxor_class, (t_method)maskxor_listl, gensym("listl"), A_GIMME, 0, 0);
  class_addmethod(maskxor_class, (t_method)maskxor_listr, gensym("listr"), A_GIMME, 0, 0);
  class_addbang(maskxor_class, (t_method)maskxor_bang);
  class_addmethod(maskxor_class, (t_method)maskxor_clear, gensym("clear"), A_DEFFLOAT, 0);
}
