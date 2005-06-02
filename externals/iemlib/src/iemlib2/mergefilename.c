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
#include <math.h>

/* -------------------------- mergefilename ------------------------------ */
/* ------------ concatenates a list of symbols to one symbol ------------- */
/* ----- between the linked symbols, there is a variable character ------- */

static t_class *mergefilename_class;

typedef struct _mergefilename
{
  t_object x_obj;
  char     x_sep[2];
} t_mergefilename;

static void mergefilename_separator(t_mergefilename *x, t_symbol *s, int ac, t_atom *av)
{
  if(ac > 0)
  {
    if(IS_A_SYMBOL(av, 0))
    {
      if(strlen(av->a_w.w_symbol->s_name) == 1)
        x->x_sep[0] = av->a_w.w_symbol->s_name[0];
      else if(!strcmp(av->a_w.w_symbol->s_name, "backslash"))
        x->x_sep[0] = '\\';
      else if(!strcmp(av->a_w.w_symbol->s_name, "slash"))
        x->x_sep[0] = '/';
      else if(!strcmp(av->a_w.w_symbol->s_name, "blank"))
        x->x_sep[0] = ' ';
      else if(!strcmp(av->a_w.w_symbol->s_name, "space"))
        x->x_sep[0] = ' ';
      else if(!strcmp(av->a_w.w_symbol->s_name, "dollar"))
        x->x_sep[0] = '$';
      else if(!strcmp(av->a_w.w_symbol->s_name, "comma"))
        x->x_sep[0] = ',';
      else if(!strcmp(av->a_w.w_symbol->s_name, "semi"))
        x->x_sep[0] = ';';
      else if(!strcmp(av->a_w.w_symbol->s_name, "leftbrace"))
        x->x_sep[0] = '{';
      else if(!strcmp(av->a_w.w_symbol->s_name, "rightbrace"))
        x->x_sep[0] = '}';
      else
        x->x_sep[0] = 0;
    }
    else if(IS_A_FLOAT(av, 0))
    {
      int i;
      float f=fabs(av->a_w.w_float);
      
      while(f >= 10.0)
        f *= 0.1;
      i = (int)f;
      x->x_sep[0] = (char)i + '0';
    }
  }
  else
    x->x_sep[0] = 0;
}

static void mergefilename_float(t_mergefilename *x, t_floatarg f)
{
  char fbuf[30];
  
  fbuf[0] = 0;
  sprintf(fbuf, "%g", f);
  outlet_symbol(x->x_obj.ob_outlet, gensym(fbuf));
}

static void mergefilename_symbol(t_mergefilename *x, t_symbol *s)
{
  outlet_symbol(x->x_obj.ob_outlet, s);
}

static void mergefilename_list(t_mergefilename *x, t_symbol *s, int ac, t_atom *av)
{
  char *cbeg, fbuf[30];
  int size=400, i, len, cursize=0;
  
  cbeg = (char *)getbytes(size * sizeof(char));
  cbeg[0] = 0;
  if(ac > 0)
  {
    if(IS_A_SYMBOL(av, 0))
    {
      len = strlen(av->a_w.w_symbol->s_name);
      if((len + cursize) >= (size-1))
      {
        cbeg = (char *)resizebytes(cbeg, size*sizeof(char), 2*(len + cursize)*sizeof(char));
        size = 2*(len + cursize);
      }
      strcat(cbeg, av->a_w.w_symbol->s_name);
      cursize += len;
    }
    else if(IS_A_FLOAT(av, 0))
    {
      sprintf(fbuf, "%g", av->a_w.w_float);
      len = strlen(fbuf);
      if((len + cursize) >= (size-1))
      {
        cbeg = (char *)resizebytes(cbeg, size*sizeof(char), 2*(len + cursize)*sizeof(char));
        size = 2*(len + cursize);
      }
      strcat(cbeg, fbuf);
      cursize += len;
    }
  }
  
  if(ac > 1)
  {
    for(i=1; i<ac; i++)
    {
      av++;
      strcat(cbeg, x->x_sep);
      if(IS_A_SYMBOL(av, 0))
      {
        len = strlen(av->a_w.w_symbol->s_name);
        if((len + cursize) >= (size-1))
        {
          cbeg = (char *)resizebytes(cbeg, size*sizeof(char), 2*(len + cursize)*sizeof(char));
          size = 2*(len + cursize);
        }
        strcat(cbeg, av->a_w.w_symbol->s_name);
        cursize += len;
      }
      else if(IS_A_FLOAT(av, 0))
      {
        sprintf(fbuf, "%g", av->a_w.w_float);
        len = strlen(fbuf);
        if((len + cursize) >= (size-1))
        {
          cbeg = (char *)resizebytes(cbeg, size*sizeof(char), 2*(len + cursize)*sizeof(char));
          size = 2*(len + cursize);
        }
        strcat(cbeg, fbuf);
        cursize += len;
      }
    }
  }
  outlet_symbol(x->x_obj.ob_outlet, gensym(cbeg));
  freebytes(cbeg, size * sizeof(char));
}

static void mergefilename_anything(t_mergefilename *x, t_symbol *s, int ac, t_atom *av)
{
  char *cbeg, fbuf[30];
  int size=400, i, len, cursize=0;
  
  cbeg = (char *)getbytes(size * sizeof(char));
  cbeg[0] = 0;
  len = strlen(s->s_name);
  if((len + cursize) >= (size-1))
  {
    cbeg = (char *)resizebytes(cbeg, size*sizeof(char), 2*(len + cursize)*sizeof(char));
    size = 2*(len + cursize);
  }
  strcat(cbeg, s->s_name);
  cursize += len;
  if(ac > 0)
  {
    for(i=0; i<ac; i++)
    {
      strcat(cbeg, x->x_sep);
      if(IS_A_SYMBOL(av, 0))
      {
        len = strlen(av->a_w.w_symbol->s_name);
        if((len + cursize) >= (size-1))
        {
          cbeg = (char *)resizebytes(cbeg, size*sizeof(char), 2*(len + cursize)*sizeof(char));
          size = 2*(len + cursize);
        }
        strcat(cbeg, av->a_w.w_symbol->s_name);
        cursize += len;
      }
      else if(IS_A_FLOAT(av, 0))
      {
        sprintf(fbuf, "%g", av->a_w.w_float);
        len = strlen(fbuf);
        if((len + cursize) >= (size-1))
        {
          cbeg = (char *)resizebytes(cbeg, size*sizeof(char), 2*(len + cursize)*sizeof(char));
          size = 2*(len + cursize);
        }
        strcat(cbeg, fbuf);
        cursize += len;
      }
      av++;
    }
  }
  outlet_symbol(x->x_obj.ob_outlet, gensym(cbeg));
  freebytes(cbeg, size * sizeof(char));
}

static void *mergefilename_new(t_symbol *s, int ac, t_atom *av)
{
  t_mergefilename *x = (t_mergefilename *)pd_new(mergefilename_class);
  
  x->x_sep[0] = 0;
  x->x_sep[1] = 0;
  if(ac > 0)
    mergefilename_separator(x, s, ac, av);
  outlet_new(&x->x_obj, &s_symbol);
  return (x);
}

void mergefilename_setup(void)
{
  mergefilename_class = class_new(gensym("mergefilename"), (t_newmethod)mergefilename_new,
    0, sizeof(t_mergefilename), 0, A_GIMME, 0);
  class_addmethod(mergefilename_class, (t_method)mergefilename_separator, gensym("separator"), A_GIMME, 0);
  class_addmethod(mergefilename_class, (t_method)mergefilename_separator, gensym("sep"), A_GIMME, 0);
  class_addfloat(mergefilename_class, mergefilename_float);
  class_addsymbol(mergefilename_class, mergefilename_symbol);
  class_addlist(mergefilename_class, mergefilename_list);
  class_addanything(mergefilename_class, mergefilename_anything);
  class_sethelpsymbol(mergefilename_class, gensym("iemhelp/help-mergefilename"));
}
