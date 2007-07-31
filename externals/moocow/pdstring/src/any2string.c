/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: any2string_dynamic.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: convert pd messages to strings (dynamic allocation)
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/* black magic */
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
/*#define ANY2STRING_DEBUG 1*/

#ifdef ANY2STRING_DEBUG
# define A2SDEBUG(x) x
#else
# define A2SDEBUG(x)
#endif

#define ANY2STRING_DEFAULT_BUFLEN 256


/*=====================================================================
 * Structures and Types: any2string
 *=====================================================================*/
static t_class *any2string_class;

typedef struct _any2string
{
  t_object       x_obj;
  int            x_alloc;
  int            x_argc;
  t_float        x_eos;      //-- EOS character to add (<0 for none)
  char          *x_text;
  t_atom        *x_argv;
  t_binbuf      *x_binbuf;
  t_inlet       *x_eos_in;
  t_outlet      *x_outlet;
} t_any2string;


/*=====================================================================
 * Constants
 *=====================================================================*/
static char *any2string_banner = "any2string: pdstring version " PACKAGE_VERSION " by Bryan Jurish";

/*=====================================================================
 * Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * anything
 */
static void any2string_anything(t_any2string *x, t_symbol *sel, int argc, t_atom *argv)
{
  t_atom *ap;
  char *s, *s_max;
  int len;

  A2SDEBUG(post("-------any2string_anything(%p,...)---------", x));

  /*-- set up binbuf --*/
  A2SDEBUG(post("any2string[%p]: binbuf_clear()", x));
  binbuf_clear(x->x_binbuf);

  /*-- binbuf_add(): selector --*/
  if (sel != &s_float && sel != &s_list && sel != &s_) {
    t_atom a;
    A2SDEBUG(post("any2string[%p]: binbuf_add(): selector: '%s'", x, sel->s_name));
    SETSYMBOL((&a), sel);
    binbuf_add(x->x_binbuf, 1, &a);
  }
  A2SDEBUG(else { post("any2string[%p]: selector: '%s': IGNORED", x, sel->s_name); });

  /*-- binbuf_add(): arg list --*/
  A2SDEBUG(post("any2string[%p]: binbuf_add(): arg list", x));
  binbuf_add(x->x_binbuf, argc, argv);
  A2SDEBUG(post("any2string[%p]: binbuf_print: ", x));
  A2SDEBUG(binbuf_print(x->x_binbuf));

  A2SDEBUG(post("any2string[%p]: binbuf_gettext()", x));
  binbuf_gettext(x->x_binbuf, &(x->x_text), &len);
  A2SDEBUG(post("any2string[%p]: binbuf_gettext() = \"%s\" ; len=%d", x, x->x_text, len));
  /*text[len] = 0;*/ /*-- ? avoid errors: "free(): invalid next size(fast): 0x..." */

  /*-- get output atom-list length --*/
  x->x_argc = len;
  if (x->x_eos >= 0) { x->x_argc++; }
  A2SDEBUG(post("any2string[%p]: argc=%d", x, x->x_argc));

  /*-- (re-)allocate (maybe) --*/
  if (x->x_alloc < x->x_argc) {
    A2SDEBUG(post("any2string[%p]: reallocate(%d->%d)", x, x->x_alloc, x->x_argc));
    freebytes(x->x_argv, x->x_alloc*sizeof(t_atom));
    x->x_argv = (t_atom*)getbytes(x->x_argc * sizeof(t_atom));
    x->x_alloc = x->x_argc;
  }

  /*-- atom buffer: binbuf text --*/
  A2SDEBUG(post("any2string[%p]: atom buffer: for {...}", x));
  ap    = x->x_argv;
  s_max = x->x_text+len;
  for (s=x->x_text; s < s_max; s++, ap++) {
    A2SDEBUG(post("any2string[%p]: atom buffer[%d]: SETFLOAT(a,%d='%c')", x, (ap-x->x_argv), *s, *s));
    SETFLOAT(ap,*s);
  }
  A2SDEBUG(post("any2string: atom buffer: DONE"));

  /*-- add EOS character (maybe) --*/
  if (x->x_eos >= 0) { SETFLOAT(ap, ((int)x->x_eos)); }

  A2SDEBUG(post("any2string: outlet_list(..., %d, ...)", x->x_argc));
  outlet_list(x->x_outlet, &s_list, x->x_argc, x->x_argv);
}


/*--------------------------------------------------------------------
 * new
 */
static void *any2string_new(t_symbol *sel, int argc, t_atom *argv)
{
    t_any2string *x = (t_any2string *)pd_new(any2string_class);

    //-- defaults
    x->x_eos      = 0;       
    x->x_alloc    = ANY2STRING_DEFAULT_BUFLEN;

    //-- args: 0: bufsize
    if (argc > 0) {
      int bufsize = atom_getintarg(0, argc, argv);
      if (bufsize > 0) { x->x_alloc = bufsize; }
    }
    //-- args: 1: eos
    if (argc > 1) {
      x->x_eos = atom_getfloatarg(1, argc, argv);
    }

    //-- allocate
    x->x_text   = (char *)getbytes(x->x_alloc*sizeof(char));
    x->x_argc   = 0;
    x->x_argv   = (t_atom *)getbytes(x->x_alloc*sizeof(t_atom));
    x->x_binbuf = binbuf_new();

    //-- inlets
    x->x_eos_in = floatinlet_new(&x->x_obj, &x->x_eos);

    //-- outlets
    x->x_outlet = outlet_new(&x->x_obj, &s_list);

    //-- report
    A2SDEBUG(post("any2string_new(): x=%p, alloc=%d, eos=%d, text=%p, argv=%p, binbuf=%p", x, x->x_alloc, x->x_eos, x->x_text, x->x_argv, x->x_binbuf));

    return (void *)x;
}

/*--------------------------------------------------------------------
 * free
 */
static void any2string_free(t_any2string *x)
{
  if (x->x_text) {
    freebytes(x->x_text, x->x_alloc*sizeof(char));
    x->x_text = NULL;
  }
  if (x->x_argv) {
    freebytes(x->x_argv, x->x_alloc*sizeof(t_atom));
    x->x_argv = NULL;
  }
  binbuf_free(x->x_binbuf);
  inlet_free(x->x_eos_in);
  outlet_free(x->x_outlet);
  return;
}

/*--------------------------------------------------------------------
 * setup (guts)
 */
void any2string_setup_guts(void)
{
  //-- class
  any2string_class = class_new(gensym("any2string"),
			       (t_newmethod)any2string_new,
			       (t_method)any2string_free,
			       sizeof(t_any2string),
			       CLASS_DEFAULT,
			       A_GIMME,                   //-- initial_bufsize, eos_char
			       0);
  
  //-- methods
  class_addanything(any2string_class, (t_method)any2string_anything);

  
  //-- help symbol
  class_sethelpsymbol(any2string_class, gensym("any2string-help.pd"));
}

/*--------------------------------------------------------------------
 * setup
 */
void any2string_setup(void)
{
  post(any2string_banner);
  any2string_setup_guts();
}
