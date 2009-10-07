/*
 *      paramDump.c
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


static t_class *paramDump_class;
 

typedef struct _paramDump
{
  t_object                    x_obj;
  t_outlet*			          outlet;
  //t_symbol*						s_set;
  t_symbol*					root;
} t_paramDump;



static void paramDump_guis(t_paramDump *x, t_symbol* s) {
	
	
		
		t_param* p = get_param_list(x->root);
		int ac;
		t_atom* av;
		
		while (p) {
			if (p->GUI ) {
				p->GUI(p->x,&ac,&av);
				outlet_anything(x->outlet,p->path,ac,av);
				
			}
		p = p->next;
		}
		
}


// Dump out everything (OR THE ID'S OR JUST THE NAMES?)
static void paramDump_bang(t_paramDump *x) {
	
	t_param* p = get_param_list(x->root);
	#ifdef PARAMDEBUG
	if (p == NULL) {
		post("No params found");
	} else {
		post("Found params");
	}
	#endif
	
	t_symbol* selector;
	int ac;
	t_atom* av;
	
	while (p) {
		if ( p->get ) {
			p->get(p->x, &selector, &ac, &av);
			tof_outlet_anything_prepend(x->outlet,selector,ac,av,p->path);
		}
		p = p->next;
	}
	
    
}



static void paramDump_free(t_paramDump *x)
{
	
	
}


static void *paramDump_new(t_symbol *s, int ac, t_atom *av) {
  t_paramDump *x = (t_paramDump *)pd_new(paramDump_class);
  
  x->root = tof_get_dollarzero(tof_get_root_canvas(tof_get_canvas()));
  
  
    //x->s_set = gensym("set");
	
    x->outlet = outlet_new(&x->x_obj, &s_list);
    
  return (x);
}

void paramDump_setup(void) {
  paramDump_class = class_new(gensym("paramDump"),
    (t_newmethod)paramDump_new, (t_method)paramDump_free,
    sizeof(t_paramDump), 0, A_GIMME, 0);

 class_addbang(paramDump_class, paramDump_bang);
 
 //class_addmethod(paramDump_class, (t_method) paramDump_values, gensym("values"), A_DEFSYMBOL,0);
 
 class_addmethod(paramDump_class, (t_method) paramDump_guis, gensym("guis"), A_DEFSYMBOL,0);
 //class_addmethod(paramDump_class, (t_method) paramDump_update_guis, gensym("update"), A_DEFSYMBOL,0);

}
