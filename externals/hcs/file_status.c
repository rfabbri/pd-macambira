/* --------------------------------------------------------------------------*/
/*                                                                           */
/* object for getting file type (dir, link, exe, etc) using a filename       */
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
#include <stdlib.h>
#endif

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>

static char *version = "$Revision: 1.1 $";

t_int file_status_instance_count;

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *file_status_class;

typedef struct _file_status {
	t_object            x_obj;
	t_symbol            *x_filename;
	t_outlet            *x_data_outlet;
	t_outlet            *x_status_outlet;
} t_file_status;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void file_status_output_error(t_file_status *x, int error_number)
{
	t_atom output_atoms[2];
	switch(error_number)
	{
	case EACCES:
		error("[file_status]: access denied: %s", x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("access"));
		break;
	case EIO:
		error("[file_status]: An error occured while reading %s", 
			  x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("io"));
		break;
	case ELOOP:
		error("[file_status]: A loop exists in symbolic links in %s", 
			  x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("loop"));
		break;
	case ENAMETOOLONG:
		error("[file_status]: The filename %s is too long", 
			  x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("name_too_long"));
		break;
	case ENOENT:
		error("[file_status]: %s does not exist", x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("does_not_exist"));
		break;
	case ENOTDIR:
		error("[file_status]: A component of %s is not a existing folder", 
			  x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("not_folder"));
		break;
	case EOVERFLOW:
		error("[file_status]: %s caused overflow in stat struct", 
			  x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("overflow"));
		break;
	case EFAULT:
		error("[file_status]: fault in stat struct (%s)", x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("fault"));
		break;
	case EINVAL:
		error("[file_status]: invalid argument to stat() (%s)", 
			  x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("invalid"));
		break;
	default:
		error("[file_status]: unknown error %d: %s", 
			  error_number, x->x_filename->s_name);
		SETSYMBOL(output_atoms, gensym("unknown"));
	}
	SETSYMBOL(output_atoms + 2, x->x_filename);
	outlet_anything(x->x_status_outlet, gensym("error"), 2, output_atoms);
}

static void file_status_output(t_file_status* x)
{
	DEBUG(post("file_status_output"););
	struct stat stat_buffer;
	int result;
	t_atom output_atoms[7];

#ifdef _WIN32
	result = _stat(x->x_filename, &stat_buffer);
#else
	result = stat(x->x_filename->s_name, &stat_buffer);
#endif /* _WIN32 */
	if(result != 0)
	{
		file_status_output_error(x, result);
	}
	else
	{
		/* TODO: output time stamps, in which format? */
		SETFLOAT(output_atoms, (t_float) stat_buffer.st_nlink);
		SETFLOAT(output_atoms + 1, (t_float) stat_buffer.st_uid);
		SETFLOAT(output_atoms + 2, (t_float) stat_buffer.st_gid);
		SETFLOAT(output_atoms + 3, (t_float) stat_buffer.st_rdev);
		SETFLOAT(output_atoms + 4, (t_float) stat_buffer.st_size);
		SETFLOAT(output_atoms + 5, (t_float) stat_buffer.st_blocks);
		SETFLOAT(output_atoms + 6, (t_float) stat_buffer.st_blksize);
		outlet_anything(x->x_data_outlet,x->x_filename,7,output_atoms);
	}
}


static void file_status_set(t_file_status* x, t_symbol *s) 
{
	DEBUG(post("file_status_set"););
#ifdef _WIN32
	char string_buffer[MAX_PATH];
	ExpandEnvironmentStrings(s->s_name, string_buffer, MAX_PATH);
	x->x_filename = gensym(string_buffer);
#else
	x->x_filename = s;
#endif	
}


static void file_status_symbol(t_file_status *x, t_symbol *s) 
{
   file_status_set(x,s);
   file_status_output(x);
}


static void *file_status_new(t_symbol *s) 
{
	DEBUG(post("file_status_new"););

	t_file_status *x = (t_file_status *)pd_new(file_status_class);

	if(!file_status_instance_count) 
	{
		post("[file_status] %s",version);  
		post("\twritten by Hans-Christoph Steiner <hans@at.or.at>");
		post("\tcompiled on "__DATE__" at "__TIME__ " ");
	}
	file_status_instance_count++;


    symbolinlet_new(&x->x_obj, &x->x_filename);
	x->x_data_outlet = outlet_new(&x->x_obj, 0);
	x->x_status_outlet = outlet_new(&x->x_obj, 0);

	/* set to the value from the object argument, if that exists */
	if (s != &s_)
	{
		x->x_filename = s;
	}
	else
	{
		x->x_filename = canvas_getcurrentdir();
		post("setting pattern to default: %s",x->x_filename->s_name);
	}

	return (x);
}

void file_status_setup(void) 
{
	DEBUG(post("file_status_setup"););
	file_status_class = class_new(gensym("file_status"), 
								  (t_newmethod)file_status_new, 
								  0,
								  sizeof(t_file_status), 
								  0, 
								  A_DEFSYM, 
								  0);
	/* add inlet datatype methods */
	class_addbang(file_status_class,(t_method) file_status_output);
	class_addsymbol(file_status_class,(t_method) file_status_symbol);
	
	/* add inlet message methods */
	class_addmethod(file_status_class,(t_method) file_status_set,gensym("set"), 
					A_DEFSYM, 0);
}

