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
  t_symbol*						s_set;
  t_symbol*					root;
} t_paramDump;

// Dump out the values of a specific id
//static void paramDump_values(t_paramDump *x, t_symbol* s, int a_ac, t_atom* a_av) {

/*
static void paramDump_values(t_paramDump *x, t_symbol* s) {
	
	//if ( s == &s_list && a_ac > 0 && IS_A_SYMBOL(a_av,0)  ) s = atom_getsymbol(a_av);
	
	//if ( !(s == &s_list || s == &s_float) ) {
		char* star = "*";
		int all = !(strcmp(s->s_name, star));
		
		
		struct param* pp = get_param_list();
		while (pp) {
			if (pp->root == x->root && (all || pp->id == s) ) {
				param_output_prepend(pp,x->outlet,pp->path);
				
			}
		pp = pp->next;
		}
	//}
}

*/

static void paramDump_guis(t_paramDump *x, t_symbol* s) {
	
	//char* star = "*";
	//int all = !(strcmp(s->s_name, star));
		
		struct param* pp = get_param_list(x->root);
		while (pp) {
			if (pp->ac_g ) {
				
				outlet_anything(x->outlet,pp->path,pp->ac_g,pp->av_g);
				
			}
		pp = pp->next;
		}
}

/*
static void paramDump_update_guis(t_paramDump *x, t_symbol* s) {
	
	char* star = "*";
	int all = !(strcmp(s->s_name, star));
		
		struct param* pp = get_param_list();
		while (pp) {
			if (pp->ac_g && pp->root == x->root && (all || pp->id == s) ) {
				
				param_send_prepend(pp, pp->path_ ,x->s_set );
			    //if ( pp->path_g->s_thing) 
				//	pd_typedmess(pp->path_g->s_thing, pp->selector,pp->ac, pp->av);
			       
				//outlet_anything(x->outlet,pp->path,pp->ac_g,pp->av_g);
				
			}
		pp = pp->next;
		}
}
*/


// Dump out everything (OR THE ID'S OR JUST THE NAMES?)
static void paramDump_bang(t_paramDump *x) {
	
	
	struct param* pp = get_param_list(x->root);
	#ifdef PARAMDEBUG
	if (pp == NULL) {
		post("No params found");
	} else {
		post("Found params");
	}
	#endif
	while (pp) {
		//if (pp->root == x->root) {
			param_output_prepend(pp,x->outlet,pp->path);
			
		//}
		pp = pp->next;
	}
	
    
}



static void paramDump_free(t_paramDump *x)
{
	
	
}


static void *paramDump_new(t_symbol *s, int ac, t_atom *av) {
  t_paramDump *x = (t_paramDump *)pd_new(paramDump_class);
  
  x->root = tof_get_dollarzero(tof_get_root_canvas(tof_get_canvas()));
  
  
    x->s_set = gensym("set");
	
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
