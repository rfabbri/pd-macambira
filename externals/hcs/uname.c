/* --------------------------------------------------------------------------*/
/*                                                                           */
/* object for getting file listings using wildcard patterns                  */
/* Written by Hans-Christoph Steiner <hans@at.or.at>                         */
/*                                                                           */
/* Copyright (c) 2006 Hans-Christoph Steiner                                 */
/*                                                                           */
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU General Public License               */
/* as published by the Free Software Foundation; either version 2            */
/* of the License, or (at your option) any later version.                    */
/*                                                                           */
/* See file LICENSE for further informations on licensing terms.             */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software Foundation,   */
/* Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           */
/*                                                                           */
/* --------------------------------------------------------------------------*/

#include <m_pd.h>

#ifdef _WIN32
#define _WIN32_WINNT 0x0400
#include <windows.h>
#include <stdio.h>
#else
#include <sys/utsname.h>
#include <stdlib.h>
#endif

static char *version = "$Revision: 1.1 $";

t_int uname_instance_count;

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *uname_class;

typedef struct _uname {
	  t_object            x_obj;
} t_uname;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

// TODO: regexp ~ to USERPROFILE for Windows
// TODO: make FindFirstFile display when its just a dir

static void uname_output(t_uname* x)
{
	DEBUG(post("uname_output"););

	struct utsname *utsname_struct;
	t_atom uname_data[5];
	
	utsname_struct = malloc(sizeof(utsname_struct));

	if ( uname(utsname_struct) > -1 )
	{
		SETSYMBOL(uname_data, gensym(utsname_struct->sysname));
		SETSYMBOL(uname_data + 1, gensym(utsname_struct->nodename));
		SETSYMBOL(uname_data + 2, gensym(utsname_struct->release));
		SETSYMBOL(uname_data + 3, gensym(utsname_struct->version));
		SETSYMBOL(uname_data + 4, gensym(utsname_struct->machine));
	
		outlet_anything(x->x_obj.ob_outlet,
						atom_gensym(uname_data),
						4,
						uname_data + 1);
	}
}


static void *uname_new(t_symbol *s) 
{
	DEBUG(post("uname_new"););

	t_uname *x = (t_uname *)pd_new(uname_class);
	
	if(!uname_instance_count) 
	{
		post("[uname] %s",version);  
		post("\twritten by Hans-Christoph Steiner <hans@at.or.at>");
	}
	uname_instance_count++;

	outlet_new(&x->x_obj, &s_symbol);
	
	return (x);
}

void uname_setup(void) 
{
	DEBUG(post("uname_setup"););
	uname_class = class_new(gensym("uname"), 
								  (t_newmethod)uname_new, 
								  0,
								  sizeof(t_uname), 
								  0, 
								  0);
	/* add inlet datatype methods */
	class_addbang(uname_class,(t_method) uname_output);
	
}

