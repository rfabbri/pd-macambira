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
  t_float total, suml, sumr, mode, lengthr;
  t_float f_in, yes;
  t_outlet *thru, *bool, *mask;
} t_maskxor;

void maskxor_float(t_maskxor *x, t_floatarg fin)
{
  int input = fin > 0 ? fin < MAXENTRIES ? (int)fin : MAXENTRIES : 0;
  if(x->mode == 0)
    {
      x->f_in = fin;
      x->yes = atom_getfloatarg(input, MAXENTRIES, x->masking.maskxor);
      outlet_float(x->bool, x->yes);
      if(x->yes != 0)
	{
	  outlet_float(x->thru, x->f_in);
	}
    }
  else if(x->sumr>x->suml)
    {
      x->f_in = fin;
      x->yes = atom_getfloatarg(input, MAXENTRIES, x->masking.maskxor);
      outlet_float(x->bool, x->yes);
      if(x->yes != 0)
	{
	  outlet_float(x->thru, x->f_in);
	}
    }
}

void maskxor_bang(t_maskxor *x, t_symbol *s)
{
  outlet_list(x->mask, &s_list, x->total, x->masking.maskxor);
  outlet_float(x->bool, x->yes);
  if(x->yes != 0)
    {
      outlet_float(x->thru, x->f_in);
    }
}

void maskxor_listl(t_maskxor *x, t_symbol *s, int argc, t_atom *argv)
{
  int i;
  x->suml = 0;
  float listr_element, listl_element, xor_element;
  if(x->total < argc && argc < MAXENTRIES) 
    {
      x->total = argc;
    }
  if((float)argc < x->lengthr) 
    {
      for(i=argc;i<x->lengthr;i++) SETFLOAT(&x->masking.maskl[i], 0);
    }
  for(i=0;i<x->total;i++)
    {
      listl_element = atom_getfloat(argv+i);
      SETFLOAT(&x->masking.maskl[i], listl_element);
      if(listl_element != 0) 
	{
	  x->suml++;
	}
      if(i>=x->lengthr) 
	{
	  listr_element = 0;
	}
      else 
	{
	  listr_element = atom_getfloatarg(i,MAXENTRIES,x->masking.maskr);
	}
      xor_element = (float)((int)listl_element ^ (int)listr_element);
      SETFLOAT(&x->masking.maskxor[i], xor_element);
    }
  outlet_list(x->mask, &s_list, x->total, x->masking.maskxor);
}

void maskxor_listr(t_maskxor *x, t_symbol *s, int argc, t_atom *argv)
{
  int i;
  x->sumr = 0;
  x->lengthr = (float)argc;
  float listr_element;
  if(x->total < argc && argc < MAXENTRIES) 
    {
      x->total = argc;
    }
  for(i=0;i<x->total;i++)
    {
      if(i>=argc) 
	{
	  listr_element = 0;
	}
      else listr_element = atom_getfloat(argv+i);
      SETFLOAT(&x->masking.maskr[i], listr_element);
      if(listr_element != 0) 
	{
	  x->sumr++;
	}
    }
}

void maskxor_mode(t_maskxor *x, t_symbol *s, t_floatarg fmode)
{
  x->mode = fmode;
  //  post("mode = %d", x->mode);
  post("mode = %d", fmode);
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

void maskxor_print(t_maskxor *x)
{
  float le0, le1, le2, le3, le4, le5, le6, le7, le8;
  float re0, re1, re2, re3, re4, re5, re6, re7, re8;
  float xe0, xe1, xe2, xe3, xe4, xe5, xe6, xe7, xe8;
  le0 = atom_getfloatarg(0, MAXENTRIES, x->masking.maskl);
  le1 = atom_getfloatarg(1, MAXENTRIES, x->masking.maskl);
  le2 = atom_getfloatarg(2, MAXENTRIES, x->masking.maskl);
  le3 = atom_getfloatarg(3, MAXENTRIES, x->masking.maskl);
  le4 = atom_getfloatarg(4, MAXENTRIES, x->masking.maskl);
  le5 = atom_getfloatarg(5, MAXENTRIES, x->masking.maskl);
  le6 = atom_getfloatarg(6, MAXENTRIES, x->masking.maskl);
  le7 = atom_getfloatarg(7, MAXENTRIES, x->masking.maskl);
  le8 = atom_getfloatarg(8, MAXENTRIES, x->masking.maskl);
  re0 = atom_getfloatarg(0, MAXENTRIES, x->masking.maskr);
  re1 = atom_getfloatarg(1, MAXENTRIES, x->masking.maskr);
  re2 = atom_getfloatarg(2, MAXENTRIES, x->masking.maskr);
  re3 = atom_getfloatarg(3, MAXENTRIES, x->masking.maskr);
  re4 = atom_getfloatarg(4, MAXENTRIES, x->masking.maskr);
  re5 = atom_getfloatarg(5, MAXENTRIES, x->masking.maskr);
  re6 = atom_getfloatarg(6, MAXENTRIES, x->masking.maskr);
  re7 = atom_getfloatarg(7, MAXENTRIES, x->masking.maskr);
  re8 = atom_getfloatarg(8, MAXENTRIES, x->masking.maskr);
  xe0 = atom_getfloatarg(0, MAXENTRIES, x->masking.maskxor);
  xe1 = atom_getfloatarg(1, MAXENTRIES, x->masking.maskxor);
  xe2 = atom_getfloatarg(2, MAXENTRIES, x->masking.maskxor);
  xe3 = atom_getfloatarg(3, MAXENTRIES, x->masking.maskxor);
  xe4 = atom_getfloatarg(4, MAXENTRIES, x->masking.maskxor);
  xe5 = atom_getfloatarg(5, MAXENTRIES, x->masking.maskxor);
  xe6 = atom_getfloatarg(6, MAXENTRIES, x->masking.maskxor);
  xe7 = atom_getfloatarg(7, MAXENTRIES, x->masking.maskxor);
  xe8 = atom_getfloatarg(8, MAXENTRIES, x->masking.maskxor);
  post("right mask = %d, %d, %d, %d, %d, %d, %d, %d, %d", re0, re1, re2, re3, re4, re5, re6, re7, re8);
  post(" left mask = %d, %d, %d, %d, %d, %d, %d, %d, %d", le0, le1, le2, le3, le4, le5, le6, le7, le8);
  post("  xor mask = %d, %d, %d, %d, %d, %d, %d, %d, %d", xe0, xe1, xe2, xe3, xe4, xe5, xe6, xe7, xe8);
}

void *maskxor_new(t_symbol *s, t_floatarg fmode)
{
  int i;
  t_maskxor *x = (t_maskxor *)pd_new(maskxor_class);
  x->total = 1;
  x->mode = fmode != 0 ? 1 : 0;
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
  class_addmethod(maskxor_class, (t_method)maskxor_mode, gensym("mode"), A_DEFFLOAT, 0);
  class_addmethod(maskxor_class, (t_method)maskxor_clear, gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(maskxor_class, (t_method)maskxor_print, gensym("print"), A_DEFFLOAT, 0);
}
