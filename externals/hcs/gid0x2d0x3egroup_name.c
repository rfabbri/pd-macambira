/* --------------------------------------------------------------------------*/
/*                                                                           */
/* converts a GID number to a group name symbol                              */
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
#include <lm.h>
#else
#include <stdlib.h>
#include <grp.h>
#endif

/*
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
*/

static char *version = "$Revision: 1.1 $";

t_int gid0x2d0x3egroup_name_instance_count;

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *gid0x2d0x3egroup_name_class;

typedef struct _gid0x2d0x3egroup_name {
	t_object            x_obj;
	t_float             x_gid;  
	t_outlet            *x_data_outlet;
//	t_outlet            *x_status_outlet;
} t_gid0x2d0x3egroup_name;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */


static void gid0x2d0x3egroup_name_output(t_gid0x2d0x3egroup_name* x)
{
	DEBUG(post("gid0x2d0x3egroup_name_output"););
	struct group *group_pointer;

#ifdef _WIN32
	/* TODO: implement for Windows! */
#else
	{
		group_pointer = getgrgid((gid_t)x->x_gid);
		if( group_pointer != NULL )
			outlet_symbol(x->x_data_outlet, gensym(group_pointer->gr_name));
		else /* output blank symbol if no match */
			outlet_symbol(x->x_data_outlet, &s_);
	}
#endif /* _WIN32 */
}


static void gid0x2d0x3egroup_name_set(t_gid0x2d0x3egroup_name* x, t_float f) 
{
	DEBUG(post("gid0x2d0x3egroup_name_set"););
	
	x->x_gid = f;
}


static void gid0x2d0x3egroup_name_float(t_gid0x2d0x3egroup_name *x, t_float f) 
{
   gid0x2d0x3egroup_name_set(x,f);
   gid0x2d0x3egroup_name_output(x);
}


static void *gid0x2d0x3egroup_name_new(t_float f) 
{
	DEBUG(post("gid0x2d0x3egroup_name_new"););

	t_gid0x2d0x3egroup_name *x = (t_gid0x2d0x3egroup_name *)pd_new(gid0x2d0x3egroup_name_class);

	if(!gid0x2d0x3egroup_name_instance_count) 
	{
		post("[gid->group_name] %s",version);  
		post("\twritten by Hans-Christoph Steiner <hans@at.or.at>");
		post("\tcompiled on "__DATE__" at "__TIME__ " ");
	}
	gid0x2d0x3egroup_name_instance_count++;


    floatinlet_new(&x->x_obj, &x->x_gid);
	x->x_data_outlet = outlet_new(&x->x_obj, 0);
//	x->x_status_outlet = outlet_new(&x->x_obj, 0);

	gid0x2d0x3egroup_name_set(x,f);

	return (x);
}


void gid0x2d0x3egroup_name_free(void) 
{
#ifdef _WIN32
#else
	endgrent();
#endif /* _WIN32 */	
}


void gid0x2d0x3egroup_name_setup(void) 
{
	DEBUG(post("gid0x2d0x3egroup_name_setup"););
	gid0x2d0x3egroup_name_class = class_new(gensym("gid->group_name"), 
								  (t_newmethod)gid0x2d0x3egroup_name_new, 
								  0,
								  sizeof(t_gid0x2d0x3egroup_name), 
								  0, 
								  A_DEFFLOAT, 
								  0);
	/* add inlet datatype methods */
	class_addbang(gid0x2d0x3egroup_name_class,
				  (t_method) gid0x2d0x3egroup_name_output);
	class_addfloat(gid0x2d0x3egroup_name_class,
					(t_method) gid0x2d0x3egroup_name_float);
	
	/* add inlet message methods */
	class_addmethod(gid0x2d0x3egroup_name_class,
					(t_method) gid0x2d0x3egroup_name_set,gensym("set"), 
					A_DEFSYM, 0);
}

