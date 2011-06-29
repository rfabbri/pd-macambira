/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

eval_sizeof written by Thomas Musil (c) IEM KUG Graz Austria 2000 - 2011

evaluetes the size of differnt data types */

#include "m_pd.h"

/* -------------------------- eval_sizeof ------------------------------ */
static t_class *eval_sizeof_class;

typedef struct _eval_sizeof
{
  t_object x_ob;
} t_eval_sizeof;

static void *eval_sizeof_new(void)
{
  t_eval_sizeof *x = (t_eval_sizeof *)pd_new(eval_sizeof_class);
  char sc=0;
  unsigned char uc=0;
  int si=0;
  unsigned int ui=0;
  long sl=0;
  unsigned long ul=0;
  long long sll=0;
  unsigned long long ull=0;
  float f=0.0;
  double d=0.0;
  long double ld=0.0;
  void *ptr=(void *)0;
  
  post("signed char = %d bytes\n", sizeof(sc));
  post("unsigned char = %d bytes\n", sizeof(uc));
  post("signed int = %d bytes\n", sizeof(si));
  post("unsigned int = %d bytes\n", sizeof(ui));
  post("signed long = %d bytes\n", sizeof(sl));
  post("unsigned long = %d bytes\n", sizeof(ul));
  post("signed long long = %d bytes\n", sizeof(sll));
  post("unsigned long long = %d bytes\n", sizeof(ull));
  post("float = %d bytes\n", sizeof(f));
  post("double = %d bytes\n", sizeof(d));
  post("long double = %d bytes\n", sizeof(ld));
  post("void* = %d bytes\n", sizeof(ptr));
  return (x);
}

void eval_sizeof_setup(void)
{
  eval_sizeof_class = class_new(gensym("eval_sizeof"), (t_newmethod)eval_sizeof_new,
    0, sizeof(t_eval_sizeof), 0, 0);
}

