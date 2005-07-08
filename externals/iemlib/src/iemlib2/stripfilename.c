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

/* -------------------------- stripfilename ----------------------- */
/* -- strips the first n or last n characters, depending on sign -- */
/* ------- of the initial argument (set message argument) --------- */

static t_class *stripfilename_class;

typedef struct _stripfilename
{
  t_object x_obj;
  int      x_nr_char;
} t_stripfilename;

static void stripfilename_symbol(t_stripfilename *x, t_symbol *s)
{
  if(x->x_nr_char < 0)
  {
    int len = strlen(s->s_name);
    char *str=(char *)getbytes((len+2)*sizeof(char));
    int i=len + x->x_nr_char;
    
    strcpy(str, s->s_name);
    if(i < 0)
      i = 0;
    str[i] = 0;
    outlet_symbol(x->x_obj.ob_outlet, gensym(str));
    freebytes(str, (len+2)*sizeof(char));
  }
  else if(x->x_nr_char > 0)
  {
    int len = strlen(s->s_name);
    char *str=(char *)getbytes((len+2)*sizeof(char));
    int i=x->x_nr_char;
    
    strcpy(str, s->s_name);
    if(i > len)
      i = len;
    outlet_symbol(x->x_obj.ob_outlet, gensym(str+i));
    freebytes(str, (len+2)*sizeof(char));
  }
  else
    outlet_symbol(x->x_obj.ob_outlet, s);
}

static void stripfilename_set(t_stripfilename *x, t_floatarg nr_char)
{
  x->x_nr_char = (int)nr_char;
}

static void *stripfilename_new(t_floatarg nr_char)
{
  t_stripfilename *x = (t_stripfilename *)pd_new(stripfilename_class);
  
  stripfilename_set(x, nr_char);
  outlet_new(&x->x_obj, &s_symbol);
  return (x);
}

void stripfilename_setup(void)
{
  stripfilename_class = class_new(gensym("stripfilename"), (t_newmethod)stripfilename_new,
    0, sizeof(t_stripfilename), 0, A_DEFFLOAT, 0);
  class_addsymbol(stripfilename_class, stripfilename_symbol);
  class_addmethod(stripfilename_class, (t_method)stripfilename_set, gensym("set"), A_FLOAT, 0);
  class_sethelpsymbol(stripfilename_class, gensym("iemhelp/help-stripfilename"));
}
