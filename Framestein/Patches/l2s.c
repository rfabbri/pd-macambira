/*
  l2s is from ZEXY, the excellent pure data library by Johannes Zmoelnig.
  Get it from ftp://iem.kug.ac.at/pd/Externals/ZEXY/
*/

/* not needed #include "zexy.h" */
#include "m_pd.h"
#include <stdlib.h>

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#define sqrtf sqrt
#endif

/* ------------------------- list2symbol ------------------------------- */

/* compare 2 lists ( == for lists) */

static t_class *list2symbol_class;

typedef struct _list2symbol
{
  t_object x_obj;

  t_binbuf *bbuf;
} t_list2symbol;

static void list2symbol_bang(t_list2symbol *x)
{
  char *str=0, *s2;
  int n=0;

  binbuf_gettext(x->bbuf, &str, &n);
  /* memory bug ! detected and fixed by Jusu */
  s2 = copybytes(str, n+1);
  s2[n]=0;

  outlet_symbol(x->x_obj.ob_outlet, gensym(s2));
  freebytes(str, n);
  freebytes(s2,n+1);
}

static void list2symbol_list(t_list2symbol *x, t_symbol *s, int argc, t_atom *argv)
{
  binbuf_clear(x->bbuf);
  binbuf_add(x->bbuf, argc, argv);
  
  list2symbol_bang(x);
}
static void list2symbol_anything(t_list2symbol *x, t_symbol *s, int argc, t_atom *argv)
{
  t_atom ap;
  binbuf_clear(x->bbuf);
  SETSYMBOL(&ap, s);
  binbuf_add(x->bbuf, 1, &ap);
  binbuf_add(x->bbuf, argc, argv);
  
  list2symbol_bang(x);
}

static void *list2symbol_new(t_symbol *s, int argc, t_atom *argv)
{
  t_list2symbol *x = (t_list2symbol *)pd_new(list2symbol_class);

  outlet_new(&x->x_obj, 0);
  x->bbuf = binbuf_new();
  binbuf_add(x->bbuf, argc, argv);

  return (x);
}

static void list2symbol_free(t_list2symbol *x)
{
  binbuf_free(x->bbuf);
}

void l2s_setup(void)
{
  list2symbol_class = class_new(gensym("l2s"), (t_newmethod)list2symbol_new,
			 (t_method)list2symbol_free, sizeof(t_list2symbol), 0, A_GIMME, 0);

  class_addcreator((t_newmethod)list2symbol_new, gensym("list2symbol"), A_GIMME, 0);
  class_addbang    (list2symbol_class, list2symbol_bang);
  class_addlist    (list2symbol_class, list2symbol_list);
  class_addanything(list2symbol_class, list2symbol_anything);

  class_sethelpsymbol(list2symbol_class, gensym("zexy/list2symbol"));
}
