
/*
 *      paramId.c
 *      
 *      Copyright 2009 Thomas O Fredericks <tom@hp>
 *      
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *      
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *      
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */


#include "tof.h"
#include "param.h"


static t_class *paramId_class;
 

typedef struct _paramId
{
  t_object                    x_obj;
  t_outlet*			          outlet;
  t_symbol*						path;
  //t_symbol*					root;
} t_paramId;



// Dump out everything (OR THE ID'S OR JUST THE NAMES?)
static void paramId_bang(t_paramId *x) {
	
	outlet_symbol(x->outlet,x->path);
	
    
}



static void paramId_free(t_paramId *x)
{
	
	
}


static void *paramId_new(t_symbol *s, int ac, t_atom *av) {
  t_paramId *x = (t_paramId *)pd_new(paramId_class);
  
  //x->root = tof_get_dollarzero(tof_get_root_canvas(tof_get_canvas()));
   t_canvas* canvas = tof_get_canvas();
	x->path = param_get_path(canvas,NULL);
	
    x->outlet = outlet_new(&x->x_obj, &s_list);
    
  return (x);
}

void paramId_setup(void) {
  paramId_class = class_new(gensym("paramId"),
    (t_newmethod)paramId_new, (t_method)paramId_free,
    sizeof(t_paramId), 0, A_GIMME, 0);

 class_addbang(paramId_class, paramId_bang);
 
}
