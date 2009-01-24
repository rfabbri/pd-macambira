/* -*- Mode: C -*- */
/*=============================================================================*\
 * File: pdstring.c
 * Author: Bryan Jurish <moocow@ling.uni-potsdam.de>
 * Description: pd string conversions : library
 *
 * Copyright (c) 2004-2009 Bryan Jurish.
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
#include <m_pd.h>
#include "mooPdUtils.h"

/* black magic */
#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/*=====================================================================
 * Constants
 *=====================================================================*/

/*=====================================================================
 * Structures and Types: pdstring [dummy]
 *=====================================================================*/
static t_class *pdstring_class;

typedef struct _pdstring
{
  t_object       x_obj;
} t_pdstring;


/*=====================================================================
 * External declarations
 *=====================================================================*/
#ifndef PDSTRING_OBJECT_EXTERNALS
# include "any2bytes.c"
# include "bytes2any.c"
# include "bytes2wchars.c"
#endif

/*--------------------------------------------------------------------
 * new
 */
static void *pdstring_new(void)
{
  t_pdstring *x = (t_pdstring *)pd_new(pdstring_class);
  return x;
}

/*--------------------------------------------------------------------
 * help
 */
static void pdstring_help(MOO_UNUSED t_pdstring *x)
{
  post("");
  post("pdstring: byte-string externals version " PACKAGE_VERSION " by Bryan Jurish");
  post("pdstring: compiled by " PDSTRING_USER " on " PDSTRING_DATE);
}

/*=====================================================================
 * Setup
 *=====================================================================*/
void pdstring_setup(void)
{
  pdstring_help(NULL);

#ifndef PDSTRING_OBJECT_EXTERNALS
  any2bytes_setup_guts();
  bytes2any_setup_guts();
  bytes2wchars_setup_guts();
#endif

  pdstring_class = class_new(gensym("pdstring"),
			     (t_newmethod)pdstring_new,
			     0,
			     sizeof(t_pdstring),
			     CLASS_DEFAULT,
			     0);

  //-- help method
  class_addmethod(pdstring_class, (t_method)pdstring_help, gensym("help"), A_NULL);

  //-- help symbol
  //class_sethelpsymbol(pdstring_class, gensym("pdstring-help.pd")); //-- breaks pd-extended help lookup
}
