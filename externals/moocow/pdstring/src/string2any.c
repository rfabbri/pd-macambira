/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: string2any.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: convert strings to pd messages
 *
 * Copyright (c) 2004 Bryan Jurish.
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
/*#define STRING2ANY_DEBUG 1*/
/*#undef STRING2ANY_DEBUG*/

#ifdef STRING2ANY_DEBUG
# define S2ADEBUG(x) x
#else
# define S2ADEBUG(x)
#endif

#define STRING2ANY_DEFAULT_BUFLEN 256


/*=====================================================================
 * Constants
 *=====================================================================*/
static char *string2any_banner = "string2any: pdstring version " PACKAGE_VERSION " by Bryan Jurish";

/*=====================================================================
 * Structures and Types: any2string
 *=====================================================================*/

static t_class *string2any_class;

typedef struct _string2any
{
  t_object       x_obj;
  size_t         x_size;
  t_float        x_eos;   //-- eos character
  char          *x_text;
  t_binbuf      *x_binbuf;
  t_inlet       *x_eos_in;
  t_outlet      *x_outlet;
  t_outlet      *x_outlet_done;
} t_string2any;


/*=====================================================================
 * Utilities
 *=====================================================================*/

/*--------------------------------------------------------------------
 * string2any_atoms()
 */
static void string2any_atoms(t_string2any *x, int argc, t_atom *argv)
{
  char *s;
  int x_argc, a_argc=argc;
  t_atom *x_argv;

  /*-- allocate --*/
  if (x->x_size <= (argc+1)) {
    freebytes(x->x_text, x->x_size*sizeof(char));
    x->x_size = argc+1;
    x->x_text = (char *)getbytes(x->x_size*sizeof(char));
  }

  /*-- get text --*/
  for (s=x->x_text; argc > 0; argc--, argv++, s++) {
    *s = atom_getfloat(argv);
  }
  *s = 0;
  S2ADEBUG(post("string2any[%p]: text: \"%s\", strlen=%d, argc=%d", x, x->x_text, strlen(x->x_text), a_argc));

  /*-- clear and fill binbuf --*/
  binbuf_clear(x->x_binbuf);
  binbuf_text(x->x_binbuf, x->x_text, a_argc); //-- handle NULs if binbuf will (but it won't) ?
  S2ADEBUG(post("string2any[%p]: binbuf_print: ", x));
  S2ADEBUG(binbuf_print(x->x_binbuf));

  /*-- output --*/
  x_argc = binbuf_getnatom(x->x_binbuf);
  x_argv = binbuf_getvec(x->x_binbuf);
  if (x_argc && x_argv->a_type == A_SYMBOL) {
    outlet_anything(x->x_outlet,
		    x_argv->a_w.w_symbol,
		    x_argc-1,
		    x_argv+1);
  }
  else {
    outlet_anything(x->x_outlet,
		    &s_list,
		    x_argc,
		    x_argv);
  }
}


/*=====================================================================
 * Methods
 *=====================================================================*/

/*--------------------------------------------------------------------
 * anything
 */
static void string2any_anything(t_string2any *x, t_symbol *sel, int argc, t_atom *argv)
{
  int i0=0, i;

  /*-- scan & output --*/
  if (x->x_eos >= 0) {
    for (i=i0; i < argc; i++) {
      if (((int)atom_getfloatarg(i,argc,argv))==((int)x->x_eos)) {
	string2any_atoms(x, i-i0, argv+i0);
	i0=i+1;
      }
    }
  }

  if (i0 < argc) {
    string2any_atoms(x, argc-i0, argv+i0);
  }

  outlet_bang(x->x_outlet_done);
}


/*--------------------------------------------------------------------
 * new
 */
static void *string2any_new(t_symbol *sel, int argc, t_atom *argv)
{
    t_string2any *x = (t_string2any *)pd_new(string2any_class);

    //-- defaults
    x->x_binbuf = binbuf_new();
    x->x_size   = STRING2ANY_DEFAULT_BUFLEN;
    x->x_eos    = -1;

    //-- args: 0: bufsize
    if (argc > 0) {
      int initial_bufsize = atom_getintarg(0,argc,argv);
      if (initial_bufsize > 0) x->x_size = initial_bufsize;
      x->x_eos = -1;   //-- backwards-compatibility hack: no default eos character if only bufsize is specified
    } 
    //-- args: 1: separator
    if (argc > 1) {
      x->x_eos = atom_getfloatarg(1,argc,argv);
    }

    //-- allocate
    x->x_text = (char *)getbytes(x->x_size*sizeof(char));

    //-- inlets
    x->x_eos_in = floatinlet_new(&x->x_obj, &x->x_eos);

    //-- outlets
    x->x_outlet      = outlet_new(&x->x_obj, &s_list);
    x->x_outlet_done = outlet_new(&x->x_obj, &s_bang);

    //-- debug
    S2ADEBUG(post("string2any_new: x=%p, size=%d, eos=%d, binbuf=%p, text=%p", x, x->x_size, x->x_eos, x->x_binbuf, x->x_text));

    return (void *)x;
}

/*--------------------------------------------------------------------
 * free
 */
static void string2any_free(t_string2any *x)
{
  if (x->x_text) {
    freebytes(x->x_text, x->x_size*sizeof(char));
    x->x_text = NULL;
  }
  binbuf_free(x->x_binbuf);
  inlet_free(x->x_eos_in);
  outlet_free(x->x_outlet_done);
  outlet_free(x->x_outlet);
  return;
}

/*--------------------------------------------------------------------
 * setup: guts
 */
void string2any_setup_guts(void)
{
  //-- class
  string2any_class = class_new(gensym("string2any"),
			       (t_newmethod)string2any_new,
			       (t_method)string2any_free,
			       sizeof(t_string2any),
			       CLASS_DEFAULT,
			       A_GIMME,                     //-- initial_bufsize, eos_char
			       0);
  
  //-- methods
  class_addanything(string2any_class, (t_method)string2any_anything);

  
  //-- help symbol
  class_sethelpsymbol(string2any_class, gensym("string2any-help.pd"));
}

/*--------------------------------------------------------------------
 * setup
 */
void string2any_setup(void)
{
  post(string2any_banner);
  string2any_setup_guts();
}
