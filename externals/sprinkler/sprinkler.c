/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: sprinkler.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: message-forwarding : workaround for missing dynamic 'send'
 *
 *   + code adapted from 'send_class' in $PD_ROOT/src/x_connective.c
 *   + formerly 'forward.c'

 *
 * Copyright (c) 2002 Bryan Jurish.
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * See file LICENSE for further informations on licensing terms.
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

#include <m_pd.h>
//#include "sprinkler.h"

/* black magic */
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/*--------------------------------------------------------------------
 * DEBUG
 *--------------------------------------------------------------------*/
//#define SPRINKLER_DEBUG 1


/*=====================================================================
 * Constants
 *=====================================================================*/
#ifdef SPRINKLER_DEBUG
// error-message buffer
#define EBUFSIZE 256
static char sprinkler_errbuf[EBUFSIZE];
#endif

/*=====================================================================
 * Structures and Types
 *=====================================================================*/

static char *sprinkler_version = "\nsprinkler version 0.03 by Bryan Jurish : dynamic message dissemination";

static t_class *sprinkler_class;

typedef struct _sprinkler
{
    t_object x_obj;
} t_sprinkler;


/*--------------------------------------------------------------------
 * the guts:
 *  + send (the tail of) a list or message to the control-bus
 *    named by its initial element
 *  + HACK for single-element arglists *ONLY*:
 *    - sprinkler float- and pointer-initial arglists with 'pd_sprinklermess',
 *      everything else with 'pd_typedmess' as a *LIST*
 *--------------------------------------------------------------------*/
static void sprinkler_anything(t_sprinkler *x, t_symbol *dst, int argc, t_atom *argv)
{

#ifdef SPRINKLER_DEBUG
  atom_string(argv, sprinkler_errbuf, EBUFSIZE);
  post("sprinkler_debug : sprinkler_anything : dst=%s, argc=%d, arg1=%s", dst->s_name, argc, argc ? sprinkler_errbuf : "NULL");
#endif

  /*-----------------------------------------------------------------------
   * HACK:
   * + single-element arglists *ONLY*
   * + sprinkler float- and pointer-initial arglists with 'pd_sprinklermess',
   *   everything else with 'pd_typedmess' as a *LIST*
   *------------------------------------------------------------------------
   */
  if (dst->s_thing) {
    if (argc == 1) {
      switch (argv->a_type) {
      case A_FLOAT:
	pd_typedmess(dst->s_thing,&s_float,argc,argv);
	return;
      case A_SYMBOL:
	pd_typedmess(dst->s_thing,&s_symbol,argc,argv);
	return;
      case A_POINTER:
	pd_typedmess(dst->s_thing,&s_pointer,argc,argv);
	return;

      // everything else (stop 'gcc -Wall' from complaining)
      case A_NULL:
      case A_SEMI:
      case A_COMMA:
      case A_DEFFLOAT:
      case A_DOLLAR:
      case A_DOLLSYM:
      case A_GIMME:
      case A_CANT:
      default:
	// just fall though
		  ;	// empty statement to keep VC++ happy
      }
    }
    // default -- sprinkler anything else with 'pd_forwardmess'
    pd_forwardmess(dst->s_thing,argc,argv);
  }
}

static void sprinkler_list(t_sprinkler *x, t_symbol *s, int argc, t_atom *argv)
{
#ifdef SPRINKLER_DEBUG
  post("sprinkler_debug : sprinkler_list : argc=%d", argc);
#endif
  sprinkler_anything(x,atom_getsymbol(argv),--argc,++argv);
}

void *sprinkler_new(t_symbol *s)
{
    t_sprinkler *x = (t_sprinkler *)pd_new(sprinkler_class);
    return (x);
}

/*--------------------------------------------------------------------
 * setup
 *--------------------------------------------------------------------*/
void sprinkler_setup(void)
{
  post(sprinkler_version);
  sprinkler_class = class_new(gensym("sprinkler"), (t_newmethod)sprinkler_new, 0, sizeof(t_sprinkler), 0, 0);

#ifdef NON_MAX_FORWARD
  // add aliases [forward] and [fw]
  post("sprinkler : non-MAX [forward] alias enabled");
  class_addcreator((t_newmethod)sprinkler_new, gensym("forward"), A_DEFSYM, 0);
  class_addcreator((t_newmethod)sprinkler_new, gensym("fw"), A_DEFSYM, 0);
#endif

#ifdef SPRINKLER_DEBUG
  post("sprinkler : debugging enabled");
#endif
  
  // methods
  class_addlist(sprinkler_class, sprinkler_list);    
  class_addanything(sprinkler_class, sprinkler_anything);
  
  // help symbol
  class_sethelpsymbol(sprinkler_class, gensym("sprinkler-help.pd"));
}
