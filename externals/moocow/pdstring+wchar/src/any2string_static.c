/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: any2string_static.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: convert pd messages to strings (static buffer allocation)
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
#include <stdio.h>

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
/*#undef ANY2STRING_DEBUG*/

#ifdef ANY2STRING_DEBUG
# define A2SDEBUG(x) x
#else
# define A2SDEBUG(x)
#endif

#define ANY2STRING_DEFAULT_BUFLEN 512


/*=====================================================================
 * Structures and Types: any2string
 *=====================================================================*/
static t_class *any2string_class;

typedef struct _any2string
{
  t_object       x_obj;
  int            x_alloc;  //-- buffer size (text, x_argv)
  int            x_argc;   //-- current number of atoms to outlet
  t_atom        *x_argv;   //-- float-list to outlet
  t_outlet      *x_outlet; //-- outlet
} t_any2string;


/*=====================================================================
 * Utilities
 *=====================================================================*/

/*--------------------------------------------------------------------
 * append_string
 */
static void any2string_append_string(t_any2string *x, char *s, unsigned int maxlen, int doescape)
{
  char *sp;
  char *ep = s+maxlen;

  for (sp=s; *sp && sp<ep && *sp && x->x_argc<x->x_alloc; sp++, x->x_argc++) {
    if (doescape && (*sp==';' || *sp==',' || *sp=='\\'
		     || (*sp == '$' && sp<(ep-1) && sp[1] >= '0' && sp[1] <= '9')))
      {
	A2SDEBUG(post("any2string_append_string: ESCAPE: x_argv[%d] = '%c' = %d", x->x_argc, '\\', '\\'));
	x->x_argv[x->x_argc++].a_w.w_float = '\\';
	if (x->x_argc >= x->x_alloc) break;
      }
    A2SDEBUG(post("any2string_append_string: x_argv[%d] = '%c' = %d", x->x_argc, *sp, *sp));
    x->x_argv[x->x_argc].a_w.w_float = *sp;
  }
}

/*--------------------------------------------------------------------
 * append_atom
 */
#define ANY2STRING_APPEND_BUFSIZE 30
static void any2string_append_atom(t_any2string *x, t_atom *a)
{
  char buf[ANY2STRING_APPEND_BUFSIZE];
  A2SDEBUG(post("~~ any2string_append_atom(%p,...) ~~", x));

  if (x->x_argc >= x->x_alloc) { return; }

  /*-- stringify a single atom (inspired by atom_string() from m_atom.c) --*/
  switch (a->a_type) {
  case A_SEMI:    any2string_append_string(x, ";", 1, 0); break;
  case A_COMMA:   any2string_append_string(x, ",", 1, 0); break;
  case A_POINTER: any2string_append_string(x, "(pointer)", 9, 0); break;
  case A_FLOAT:
    snprintf(buf, ANY2STRING_APPEND_BUFSIZE, "%g", a->a_w.w_float);
    any2string_append_string(x, buf, ANY2STRING_APPEND_BUFSIZE, 0);
    break;
  case A_SYMBOL:
    any2string_append_string(x, a->a_w.w_symbol->s_name, strlen(a->a_w.w_symbol->s_name), 1);
    break;
  case A_DOLLAR:
    snprintf(buf, ANY2STRING_APPEND_BUFSIZE, "$%d", a->a_w.w_index);
    any2string_append_string(x, buf, ANY2STRING_APPEND_BUFSIZE, 0);
    break;
  case A_DOLLSYM:
    any2string_append_string(x, a->a_w.w_symbol->s_name, strlen(a->a_w.w_symbol->s_name), 0);
    break;
  default:
    pd_error(x,"any2string_append_atom: unknown atom type '%d'", a->a_type);
    break;
  }

  if (x->x_argc < x->x_alloc) {
    A2SDEBUG(post("any2string_append_atom[%p]: x_argv[%d] = '%c' = %d", x, x->x_argc, ' ', ' '));
    x->x_argv[x->x_argc++].a_w.w_float = ' ';
  }
}

/*=====================================================================
 * Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * anything
 */
static void any2string_anything(t_any2string *x, t_symbol *sel, int argc, t_atom *argv)
{
  t_atom *argv_end = argv+argc;
  x->x_argc=0;

  A2SDEBUG(post("-------any2string_anything(%p,...) ---------", x));

  /*-- stringify selector (maybe) --*/
  if (sel != &s_float && sel != &s_list && sel != &s_) {
    t_atom a;
    SETSYMBOL(&a,sel);
    any2string_append_atom(x, &a);
  }

  /*-- stringify arg list --*/
  for ( ; argv<argv_end && x->x_argc<x->x_alloc; argv++) {
    any2string_append_atom(x, argv);
  }

  /*-- add terminating NUL (if we can) --*/
  A2SDEBUG(post("any2string[%p]: terminating NUL: x_argv[%d]=0", x, x->x_argc-1));
  if (x->x_argc >= x->x_alloc) {
    pd_error(x, "any2string: input length exceeds buffer size!");
    x->x_argc = x->x_alloc;
    x->x_argv[x->x_argc-1].a_w.w_float = '*'; //-- simulate atom_string() behavior
  } else if (x->x_argc > 0) {
    x->x_argv[x->x_argc-1].a_w.w_float = 0;
  }

  A2SDEBUG(post("any2string[%p]: outlet_list(..., %d, ...)", x, x->x_argc));
  outlet_list(x->x_outlet, &s_list, x->x_argc, x->x_argv);
}


/*--------------------------------------------------------------------
 * new
 */
static void *any2string_new(t_floatarg bufsize)
{
    t_any2string *x = (t_any2string *)pd_new(any2string_class);
    int i;

    //-- bufsize
    if (bufsize <= 0) {
      x->x_alloc = ANY2STRING_DEFAULT_BUFLEN;
    } else {
      x->x_alloc = bufsize;
    }
    A2SDEBUG(post("any2string_new: buf_req=%g, alloc=%d", bufsize, x->x_alloc));

    //-- defaults
    x->x_argc  = 0;
    x->x_argv  = (t_atom*)getbytes(x->x_alloc*sizeof(t_atom));

    //-- initialize (set a_type)
    for (i=0; i < x->x_alloc; i++) {
      SETFLOAT((x->x_argv+i),0);
    }

    //-- outlets
    x->x_outlet = outlet_new(&x->x_obj, &s_list);
    A2SDEBUG(post("any2string_new: x=%p, alloc=%d, argv=%p, nbytes=%d", x, x->x_alloc,x->x_argv,x->x_alloc*sizeof(t_atom)));
    return (void *)x;
}

/*--------------------------------------------------------------------
 * free
 */
static void any2string_free(t_any2string *x)
{
  A2SDEBUG(post("any2string_free(x=%p)", x));
  if (x->x_argv) {
    A2SDEBUG(post("any2string_free(x=%p): x_argv=%p (size=%d)", x, x->x_argv, x->x_alloc*sizeof(t_atom)));
    freebytes(x->x_argv, x->x_alloc*sizeof(t_atom));
  }
  A2SDEBUG(post("any2string_free(x=%p): x_outlet=%p", x, x->x_outlet));
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
			       A_DEFFLOAT,
			       0);
  
  //-- methods
  class_addanything(any2string_class,
		    (t_method)any2string_anything);

  
  //-- help symbol
  class_sethelpsymbol(any2string_class, gensym("pdstring-help.pd"));
}
