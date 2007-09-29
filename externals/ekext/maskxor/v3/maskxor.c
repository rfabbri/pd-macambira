#include "m_pd.h"
#include <math.h>
//#define MAXENTRIES 512

static t_class *maskxor_class;

typedef struct _mask
{
  //  t_atom maskxor[MAXENTRIES];
  //  t_atom maskl[MAXENTRIES];
  //  t_atom maskr[MAXENTRIES];
  t_atom *maskxor, *maskl, *maskr;
} t_mask;

typedef struct _maskxor
{
  t_object x_obj;
  t_mask masking;
  t_atom *buffer;

  t_float total, suml, sumr, mode, lengthl, lengthr, ltot, rgreater;
  t_float f_in, yes, firstl, firstr, firstx;
  t_outlet *thru, *bool, *mask;
} t_maskxor;

static void maskxor_makelbuffer(t_maskxor *x, int n, t_atom *list)
{
  int i;
  float maskel;
  if (x->masking.maskl && x->firstl) {
    freebytes(x->masking.maskl, x->lengthl * sizeof(t_atom));
    x->masking.maskl = 0;
    x->lengthl = 0;
  }

  x->masking.maskl = getbytes(n * sizeof(t_atom)); 
  x->masking.maskl = copybytes(list, n * sizeof(t_atom));
  x->lengthl = n;
  x->suml = 0;
  for (i=0;i<n;i++)
    {
      maskel = atom_getfloatarg(i, x->lengthl, x->masking.maskl);
      if (maskel != 0) x->suml++;
    }
  x->firstl = 1;
  x->rgreater = x->suml <= x->sumr ? 1 : 0;
}

static void maskxor_makerbuffer(t_maskxor *x, int n, t_atom *list)
{
  int i;
  float maskel;
  if (x->masking.maskr && x->firstr) {
    freebytes(x->masking.maskr, x->lengthr * sizeof(t_atom));
    x->masking.maskr = 0;
    x->lengthr = 0;
  }

  x->masking.maskr = getbytes(n * sizeof(t_atom));
  x->masking.maskr = copybytes(list, n * sizeof(t_atom));
  x->lengthr = n;
  x->sumr = 0;
  for (i=0;i<n;i++)
    {
      maskel = atom_getfloatarg(i, x->lengthr, x->masking.maskr);
      if (maskel != 0) x->sumr++;
    }
  x->firstr = 1;
}

static void maskxor_makemaskbuffer(t_maskxor *x, int n, t_atom *list, int d, int dir)
{
  int i;
  float left, right, mask;
  if (x->masking.maskxor && x->firstx) {
    freebytes(x->masking.maskxor, x->ltot * sizeof(t_atom));
    x->buffer = 0;
    x->ltot = 0;
  }

  x->masking.maskxor = getbytes(n+d * sizeof(t_atom));
  if(dir > 0)
    {
      x->ltot = x->lengthr;
      for(i=0;i<x->ltot;i++)
	{
	  left = i >= n ? 0 : atom_getfloatarg(i, x->lengthl, x->masking.maskl);
	  right = atom_getfloatarg(i, x->lengthr, x->masking.maskr);
	  mask = (float)((int)left ^ (int)right);
	  SETFLOAT(&x->masking.maskxor[i], mask);
	}
    }
  else
    {
      x->ltot = x->lengthl;
      for(i=0;i<x->ltot;i++)
	{
	  right = i >= n ? 0 : atom_getfloatarg(i, x->lengthr, x->masking.maskr);
	  left = atom_getfloatarg(i, x->lengthl, x->masking.maskl);
	  mask = (float)((int)left ^ (int)right);
	  SETFLOAT(&x->masking.maskxor[i], mask);
	}
    }
  x->firstx = 1;
}

void maskxor_float(t_maskxor *x, t_floatarg fin)
{
  int input = fin > 0 ? (int)fin : 0;
  if(x->mode == 0)
    {
      x->f_in = fin;
      x->yes = atom_getfloatarg(input, x->ltot, x->masking.maskxor);
      outlet_float(x->bool, x->yes);
      if(x->yes != 0) outlet_float(x->thru, x->f_in);
    }
  else if (x->rgreater)
    {
      x->f_in = fin;
      x->yes = atom_getfloatarg(input, x->ltot, x->masking.maskxor);
      outlet_float(x->bool, x->yes);
      if(x->yes != 0) outlet_float(x->thru, x->f_in);
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
  int difflen;
  maskxor_makelbuffer(x, argc, argv);
  if(x->lengthl >= x->lengthr)
    {
      difflen = (int)(x->lengthl - x->lengthr);
      maskxor_makemaskbuffer(x, argc, argv, difflen, 0);
    }
 else 
   {
     difflen = (int)(x->lengthr - x->lengthl);
     maskxor_makemaskbuffer(x, argc, argv, difflen, 1);
   }

  //      listl_element = atom_getfloat(argv+i);
  //      SETFLOAT(&x->masking.maskl[i], listl_element);
  //      if(listl_element != 0) x->suml++;
  //      listr_element = atom_getfloatarg(i,MAXENTRIES,x->masking.maskr); 
  //      xor_element = (float)((int)listl_element ^ (int)listr_element);
  //      SETFLOAT(&x->masking.maskxor[i], xor_element);

  outlet_list(x->mask, &s_list, x->total, x->masking.maskxor);
}

void maskxor_listr(t_maskxor *x, t_symbol *s, int argc, t_atom *argv)
{
  maskxor_makerbuffer(x, argc, argv);
}

void maskxor_mode(t_maskxor *x, t_floatarg fmode)
{
  x->mode = fmode != 0 ? 1 : 0;
}

/* substitute zexy code */
void maskxor_clear(t_maskxor *x)
{
  freebytes(x->masking.maskl, x->lengthl * sizeof(t_atom));
  freebytes(x->masking.maskr, x->lengthr * sizeof(t_atom));
  freebytes(x->masking.maskxor, x->ltot * sizeof(t_atom));
  //  x->masking.maskl = getbytes(sizeof(t_atom));
  //  x->masking.maskr = getbytes(sizeof(t_atom));
  //  x->masking.maskxor = getbytes(sizeof(t_atom));

  //  SETFLOAT(&x->masking.maskr[0], 0);
  //  SETFLOAT(&x->masking.maskl[0], 0);
  //  SETFLOAT(&x->masking.maskxor[0], 0);
  x->lengthl = x->lengthr = x->ltot = 0;
  x->total=1;
  x->yes = x->f_in = 0;
}

void maskxor_print(t_maskxor *x)
{
  float le0, le1, le2, le3, le4, le5, le6, le7, le8;
  float re0, re1, re2, re3, re4, re5, re6, re7, re8;
  float xe0, xe1, xe2, xe3, xe4, xe5, xe6, xe7, xe8;
  le0 = atom_getfloatarg(0, x->ltot, x->masking.maskl);
  le1 = atom_getfloatarg(1, x->ltot, x->masking.maskl);
  le2 = atom_getfloatarg(2, x->ltot, x->masking.maskl);
  le3 = atom_getfloatarg(3, x->ltot, x->masking.maskl);
  le4 = atom_getfloatarg(4, x->ltot, x->masking.maskl);
  le5 = atom_getfloatarg(5, x->ltot, x->masking.maskl);
  le6 = atom_getfloatarg(6, x->ltot, x->masking.maskl);
  le7 = atom_getfloatarg(7, x->ltot, x->masking.maskl);
  le8 = atom_getfloatarg(8, x->ltot, x->masking.maskl);
  re0 = atom_getfloatarg(0, x->ltot, x->masking.maskr);
  re1 = atom_getfloatarg(1, x->ltot, x->masking.maskr);
  re2 = atom_getfloatarg(2, x->ltot, x->masking.maskr);
  re3 = atom_getfloatarg(3, x->ltot, x->masking.maskr);
  re4 = atom_getfloatarg(4, x->ltot, x->masking.maskr);
  re5 = atom_getfloatarg(5, x->ltot, x->masking.maskr);
  re6 = atom_getfloatarg(6, x->ltot, x->masking.maskr);
  re7 = atom_getfloatarg(7, x->ltot, x->masking.maskr);
  re8 = atom_getfloatarg(8, x->ltot, x->masking.maskr);
  xe0 = atom_getfloatarg(0, x->ltot, x->masking.maskxor);
  xe1 = atom_getfloatarg(1, x->ltot, x->masking.maskxor);
  xe2 = atom_getfloatarg(2, x->ltot, x->masking.maskxor);
  xe3 = atom_getfloatarg(3, x->ltot, x->masking.maskxor);
  xe4 = atom_getfloatarg(4, x->ltot, x->masking.maskxor);
  xe5 = atom_getfloatarg(5, x->ltot, x->masking.maskxor);
  xe6 = atom_getfloatarg(6, x->ltot, x->masking.maskxor);
  xe7 = atom_getfloatarg(7, x->ltot, x->masking.maskxor);
  xe8 = atom_getfloatarg(8, x->ltot, x->masking.maskxor);
  post("right mask = %d, %d, %d, %d, %d, %d, %d, %d, %d", re0, re1, re2, re3, re4, re5, re6, re7, re8);
  post(" left mask = %d, %d, %d, %d, %d, %d, %d, %d, %d", le0, le1, le2, le3, le4, le5, le6, le7, le8);
  post("  xor mask = %d, %d, %d, %d, %d, %d, %d, %d, %d", xe0, xe1, xe2, xe3, xe4, xe5, xe6, xe7, xe8);
}

void *maskxor_new(t_symbol *s, t_floatarg fmode)
{
  int i;
  t_maskxor *x = (t_maskxor *)pd_new(maskxor_class);
  x->total = 1;
  x->firstl = x->firstr = x->firstx = 0;
  x->mode = fmode != 0 ? 1 : 0;
   
  //  SETFLOAT(&x->masking.maskr[0], 0);
  //  SETFLOAT(&x->masking.maskl[0], 0);
  //  SETFLOAT(&x->masking.maskxor[0], 0);
  x->lengthl = x->lengthr = x->ltot = 0;

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

  class_addfloat(maskxor_class, maskxor_float);
  class_addmethod(maskxor_class, (t_method)maskxor_listl, gensym("listl"), A_GIMME, 0, 0);
  class_addmethod(maskxor_class, (t_method)maskxor_listr, gensym("listr"), A_GIMME, 0, 0);
  class_addbang(maskxor_class, (t_method)maskxor_bang);
  class_addmethod(maskxor_class, (t_method)maskxor_mode, gensym("mode"), A_DEFFLOAT, 0);
  class_addmethod(maskxor_class, (t_method)maskxor_clear, gensym("clear"), A_DEFFLOAT, 0);
  class_addmethod(maskxor_class, (t_method)maskxor_print, gensym("print"), A_DEFFLOAT, 0);
}
