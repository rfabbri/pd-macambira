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
  char     *x_mem;
  t_int    x_size;
} t_mergefilename;

static void mergefilename_separator(t_mergefilename *x, t_symbol *s, int ac, t_atom *av)
{
  if(ac > 0)
  {
    if(IS_A_SYMBOL(av, 0))
    {
      char *name=av->a_w.w_symbol->s_name;

      if(strlen(name) == 1)
        x->x_sep[0] = name[0];
      else if(!strcmp(name, "backslash"))
        x->x_sep[0] = '\\';
      else if(!strcmp(name, "slash"))
        x->x_sep[0] = '/';
      else if(!strcmp(name, "blank"))
        x->x_sep[0] = ' ';
      else if(!strcmp(name, "space"))
        x->x_sep[0] = ' ';
      else if(!strcmp(name, "dollar"))
        x->x_sep[0] = '$';
      else if(!strcmp(name, "comma"))
        x->x_sep[0] = ',';
      else if(!strcmp(name, "semi"))
        x->x_sep[0] = ';';
      else if(!strcmp(name, "leftbrace"))
        x->x_sep[0] = '{';
      else if(!strcmp(name, "rightbrace"))
        x->x_sep[0] = '}';
      else
        x->x_sep[0] = 0;
    }
    else if(IS_A_FLOAT(av, 0))
    {
      t_int i=atom_getintarg(0, ac, av);
      
      x->x_sep[0] = (char)i + '0';
    }
  }
  else
    x->x_sep[0] = 0;
}

static void mergefilename_float(t_mergefilename *x, t_floatarg f)
{
  char flt_buf[30];
  
  flt_buf[0] = 0;
  sprintf(flt_buf, "%g", f);
  outlet_symbol(x->x_obj.ob_outlet, gensym(flt_buf));
}

static void mergefilename_symbol(t_mergefilename *x, t_symbol *s)
{
  outlet_symbol(x->x_obj.ob_outlet, s);
}

static void mergefilename_list(t_mergefilename *x, t_symbol *s, int ac, t_atom *av)
{
  char flt_buf[30];
  t_int i, length, accu_size=0;
  
  x->x_mem[0] = 0;
  if(ac > 0)
  {
    if(IS_A_SYMBOL(av, 0))
    {
      length = strlen(av->a_w.w_symbol->s_name);
      if((length + accu_size) >= (x->x_size-2))
      {
        x->x_mem = (char *)resizebytes(x->x_mem, x->x_size*sizeof(char), 2*(length + accu_size)*sizeof(char));
        x->x_size = 2*(length + accu_size);
      }
      strcat(x->x_mem, av->a_w.w_symbol->s_name);
      accu_size += length;
    }
    else if(IS_A_FLOAT(av, 0))
    {
      sprintf(flt_buf, "%g", av->a_w.w_float);
      length = strlen(flt_buf);
      if((length + accu_size) >= (x->x_size-2))
      {
        x->x_mem = (char *)resizebytes(x->x_mem, x->x_size*sizeof(char), 2*(length + accu_size)*sizeof(char));
        x->x_size = 2*(length + accu_size);
      }
      strcat(x->x_mem, flt_buf);
      accu_size += length;
    }
  }
  
  if(ac > 1)
  {
    for(i=1; i<ac; i++)
    {
      av++;
      strcat(x->x_mem, x->x_sep);
      if(IS_A_SYMBOL(av, 0))
      {
        length = strlen(av->a_w.w_symbol->s_name);
        if((length + accu_size) >= (x->x_size-2))
        {
          x->x_mem = (char *)resizebytes(x->x_mem, x->x_size*sizeof(char), 2*(length + accu_size)*sizeof(char));
          x->x_size = 2*(length + accu_size);
        }
        strcat(x->x_mem, av->a_w.w_symbol->s_name);
        accu_size += length;
      }
      else if(IS_A_FLOAT(av, 0))
      {
        sprintf(flt_buf, "%g", av->a_w.w_float);
        length = strlen(flt_buf);
        if((length + accu_size) >= (x->x_size-2))
        {
          x->x_mem = (char *)resizebytes(x->x_mem, x->x_size*sizeof(char), 2*(length + accu_size)*sizeof(char));
          x->x_size = 2*(length + accu_size);
        }
        strcat(x->x_mem, flt_buf);
        accu_size += length;
      }
    }
  }
  outlet_symbol(x->x_obj.ob_outlet, gensym(x->x_mem));
}

static void mergefilename_anything(t_mergefilename *x, t_symbol *s, int ac, t_atom *av)
{
  char flt_buf[30];
  t_int i, length, accu_size=0;
  
  x->x_mem[0] = 0;
  length = strlen(s->s_name);
  if((length + accu_size) >= (x->x_size-2))
  {
    x->x_mem = (char *)resizebytes(x->x_mem, x->x_size*sizeof(char), 2*(length + accu_size)*sizeof(char));
    x->x_size = 2*(length + accu_size);
  }
  strcat(x->x_mem, s->s_name);
  accu_size += length;
  if(ac > 0)
  {
    for(i=0; i<ac; i++)
    {
      strcat(x->x_mem, x->x_sep);
      if(IS_A_SYMBOL(av, 0))
      {
        length = strlen(av->a_w.w_symbol->s_name);
        if((length + accu_size) >= (x->x_size-2))
        {
          x->x_mem = (char *)resizebytes(x->x_mem, x->x_size*sizeof(char), 2*(length + accu_size)*sizeof(char));
          x->x_size = 2*(length + accu_size);
        }
        strcat(x->x_mem, av->a_w.w_symbol->s_name);
        accu_size += length;
      }
      else if(IS_A_FLOAT(av, 0))
      {
        sprintf(flt_buf, "%g", av->a_w.w_float);
        length = strlen(flt_buf);
        if((length + accu_size) >= (x->x_size-2))
        {
          x->x_mem = (char *)resizebytes(x->x_mem, x->x_size*sizeof(char), 2*(length + accu_size)*sizeof(char));
          x->x_size = 2*(length + accu_size);
        }
        strcat(x->x_mem, flt_buf);
        accu_size += length;
      }
      av++;
    }
  }
  outlet_symbol(x->x_obj.ob_outlet, gensym(x->x_mem));
}

static void mergefilename_free(t_mergefilename *x)
{
  freebytes(x->x_mem, x->x_size*sizeof(char));
}

static void *mergefilename_new(t_symbol *s, int ac, t_atom *av)
{
  t_mergefilename *x = (t_mergefilename *)pd_new(mergefilename_class);
  
  x->x_sep[0] = 0;
  x->x_sep[1] = 0;
  if(ac > 0)
    mergefilename_separator(x, s, ac, av);
  x->x_size = 400;
  x->x_mem = (char *)getbytes(x->x_size*sizeof(char));
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
