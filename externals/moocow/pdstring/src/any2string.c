/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: any2string.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: convert pd messages to strings
 *
 * Copyright (c) 2004 - 2007 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file "COPYING", in this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *=============================================================================*/

#include <string.h>

#include <m_pd.h>

/* black magic */
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
#define ANY2STRING_DEBUG 1

#ifdef ANY2STRING_DEBUG
# define A2SDEBUG(x) x
#else
# define A2SDEBUG(x)
#endif

#define ANY2STRING_INITIAL_BUFLEN 32


/*=====================================================================
 * Structures and Types: any2string
 *=====================================================================*/
static t_class *any2string_class;

typedef struct _any2string
{
  t_object       x_obj;
  int            x_alloc;
  int            x_argc;
  t_atom        *x_argv;
  t_binbuf      *x_binbuf;
  t_outlet      *x_outlet;
} t_any2string;


/*=====================================================================
 * Constants
 *=====================================================================*/

/*=====================================================================
 * Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * anything
 */
static void any2string_anything(t_any2string *x, t_symbol *sel, int argc, t_atom *argv)
{
  t_atom *a;
  char *text=NULL, *s, *s_max;
  int len;

  A2SDEBUG(post("-------any2string_anything()---------"));

  /*-- set up binbuf --*/
  A2SDEBUG(post("any2string: binbuf_clear()"));
  binbuf_clear(x->x_binbuf);
  A2SDEBUG(post("any2string: binbuf_add()"));
  binbuf_add(x->x_binbuf, argc, argv);
  A2SDEBUG(startpost("any2string: binbuf_print: "));
  A2SDEBUG(binbuf_print(x->x_binbuf));

  A2SDEBUG(post("any2string: binbuf_gettext()"));
  binbuf_gettext(x->x_binbuf, &text, &len);
  A2SDEBUG(post("any2string: binbuf_gettext() = \"%s\" ; len=%d", text, len));
  /*text[len] = 0;*/ /*-- ? avoid errors: free(): invalid next size(fast): [HEX_ADDRESS] */

  /*-- get string length --*/
  x->x_argc = len+1;
  if (sel != &s_float && sel != &s_list && sel != &s_) {
    x->x_argc += strlen(sel->s_name);
    if (argc > 0) x->x_argc++;
  }
  A2SDEBUG(post("any2string: argc=%d", x->x_argc));

  /*-- (re-)allocate --*/
  if (x->x_alloc < x->x_argc) {
    A2SDEBUG(post("any2string: reallocate(%d->%d)", x->x_alloc, x->x_argc));
    freebytes(x->x_argv, x->x_alloc * sizeof(t_atom));
    x->x_argv = (t_atom *)getbytes(x->x_argc * sizeof(t_atom));
    x->x_alloc = x->x_argc;
  }

  /*-- add selector (maybe) --*/
  a=x->x_argv;
  if (sel != &s_float && sel != &s_list && sel != &s_) {
    A2SDEBUG(post("any2string: for {...} //sel"));
    for (s=sel->s_name; *s; s++, a++) {
      A2SDEBUG(post("any2string: for: SETFLOAT(a,'%c'=%d) //sel", *s, *s));
      SETFLOAT(a,*s);
    }
    A2SDEBUG(post("any2string: /for {...} //sel"));

    if (argc > 0) {
      SETFLOAT(a,' ');
      a++;
    }
  }

  /*-- add binbuf text --*/
  A2SDEBUG(post("any2string: for {...}"));
  s_max = text+len;
  for (s=text; s < s_max; s++, a++) {
    A2SDEBUG(post("any2string: for: //SETFLOAT(a,'%c'=%d)", *s, *s));
    SETFLOAT(a,*s);
  }
  A2SDEBUG(post("any2string: /for {...}"));

  /*-- add terminating NUL --*/
  SETFLOAT(a,0);

  A2SDEBUG(post("any2string: freebytes()"));
  freebytes(text, len);

  /*
    A2SDEBUG(post("any2string: binbuf_free()"));
    binbuf_free(x->x_binbuf);
  */

  A2SDEBUG(post("any2string: outlet_list(..., %d, ...)", x->x_argc));
  outlet_list(x->x_outlet, &s_list, x->x_argc, x->x_argv);
}


/*--------------------------------------------------------------------
 * new
 */
static void *any2string_new(void)
{
    t_any2string *x = (t_any2string *)pd_new(any2string_class);

    //-- defaults
    x->x_alloc = ANY2STRING_INITIAL_BUFLEN;
    x->x_argc = 0;
    x->x_argv = (t_atom *)getbytes(x->x_alloc * sizeof(t_atom));
    x->x_binbuf = binbuf_new();

    //-- outlets
    x->x_outlet = outlet_new(&x->x_obj, &s_list);

    return (void *)x;
}

/*--------------------------------------------------------------------
 * free
 */
static void any2string_free(t_any2string *x)
{
  if (x->x_argv) {
    freebytes(x->x_argv, x->x_alloc * sizeof(t_atom));
    x->x_argv = NULL;
  }
  binbuf_free(x->x_binbuf);
  outlet_free(x->x_outlet);
  return;
}

/*--------------------------------------------------------------------
 * setup
 */
void any2string_setup(void)
{
  //-- class
  any2string_class = class_new(gensym("any2string"),
			    (t_newmethod)any2string_new,
			    (t_method)any2string_free,
			    sizeof(t_any2string),
			    CLASS_DEFAULT,
			    0);
  
  //-- methods
  /*class_addmethod(any2string_class, (t_method)any2string_symbol, &s_symbol,
		  A_DEFSYMBOL, 0);
  */
  class_addanything(any2string_class,
		    (t_method)any2string_anything);

  
  //-- help symbol
  class_sethelpsymbol(any2string_class, gensym("pdstring-help.pd"));
}
