/*
 *      param.c
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




static t_class *param_class;
static t_class *param_inlet2_class;
struct _param_inlet2;

typedef struct _param
{
  t_object                    x_obj;
  struct _param_inlet2       *x_param_inlet2;
  
  t_symbol 					*x_path;
  t_symbol	                 *s_PARAM;
  t_symbol                   *x_update_gui;
  t_symbol                   *s_set;
  struct param				  *x_param;
} t_param;

typedef struct _param_inlet2
{
  t_object        x_obj;
  t_param  *p_owner;
} t_param_inlet2;

/*
static void output_param(t_param *x) {
	if (x->x_param) {
		if((x->x_param->selector == &s_bang) ) {
			outlet_bang(x->x_obj.ob_outlet);
		} else {
			outlet_anything(x->x_obj.ob_outlet, x->x_param->selector, x->x_param->ac, x->x_param->av);
		}
	}	
}
*/
/*
static void send_param(t_param *x, t_symbol* s,t_symbol* prepend ) {
	if (x->x_param) {
	   if((x->x_param->selector == &s_bang)) {
			 if (s->s_thing) 
			     pd_bang(s->s_thing);
	  } else {
		if (s->s_thing) {
			if ( x->x_param->selector == &s_list || x->x_param->selector == &s_float || x->x_param->selector == &s_symbol ) {
				typedmess(s->s_thing, prepend, x->x_param->ac, x->x_param->av);
			} else {
			int ac = x->x_param->ac + 1;
			t_atom *av = getbytes(ac*sizeof(*av));	
			tof_copy_atoms(x->x_param->av,av+1,x->x_param->ac);
			SETSYMBOL(av, x->x_param->selector);
			typedmess(s->s_thing, prepend, ac, av);
			freebytes(av, ac*sizeof(*av));
		}
		}
	  }
	}
	
}
*/

static void param_bang(t_param *x)
{
	param_output(x->x_param,x->x_obj.ob_outlet);
	
	param_send_prepend(x->x_param, x->s_PARAM ,x->x_path );
    if(x->x_param->selector != &s_bang ) param_send_prepend(x->x_param, x->x_update_gui ,x->s_set );
 
}

static void param_anything(t_param *x, t_symbol *s, int ac, t_atom *av)
{
 
  if ( x->x_param) set_param_anything(x->x_param,s,ac,av);
  
  param_bang(x);
  
  /*
  param_output(x->x_param,x->x_obj.ob_outlet);
 
  param_send_prepend(x->x_param, x->s_PARAM ,x->x_path );
  if(x->x_param->selector != &s_bang ) param_send_prepend(x->x_param, x->x_update_gui ,x->s_set );
  */
  
}




// SECOND INLET METHOD

static void param_inlet2_anything(t_param_inlet2 *p, t_symbol *s, int ac, t_atom *av)
{
 
  if ( p->p_owner->x_param ) set_param_anything(p->p_owner->x_param, s,ac,av);
}  

// DECONSTRUCTOR

static void param_free(t_param *x)
{
	
	if(x->x_param_inlet2) pd_free((t_pd *)x->x_param_inlet2);
	
	
	if (x->x_param) unregister_param(x->x_param);
	
	if ( x->x_path) pd_unbind(&x->x_obj.ob_pd, x->x_path);
    
}

// CONSTRUCTOR
static void *param_new(t_symbol *s, int ac, t_atom *av)
{
  t_param *x = (t_param *)pd_new(param_class);
  t_param_inlet2 *p = (t_param_inlet2 *)pd_new(param_inlet2_class);
   
  // Stuff
  x->s_set = gensym("set");
  x->s_PARAM = gensym("PARAM");
  
  // Set up second inlet proxy
  x->x_param_inlet2 = p;
  p->p_owner = x;
  

   x->x_param = NULL;


	// GET THE CURRENT CANVAS
	t_canvas *canvas=tof_get_canvas();
   
   struct param_build_info build_info;
   get_param_build_info(canvas,ac,av,&build_info,1);
	
 
  if ( build_info.path  ) {
	x->x_path = build_info.path;
	x->x_update_gui = build_info.path_g;
	// BIND RECEIVER
  	pd_bind(&x->x_obj.ob_pd, build_info.path );
	// REGISTER PARAM
	x->x_param = register_param(&build_info);
    // CREATE INLETS AND OUTLETS
    inlet_new((t_object *)x, (t_pd *)p, 0, 0);
    outlet_new(&x->x_obj, &s_list);
   } else {
	   pd_error(x,"[param] requires a name(first argument) that starts with a /");
   }
  
  
  
  return (x);
}

void param_setup(void)
{
  param_class = class_new(gensym("param"),
    (t_newmethod)param_new, (t_method)param_free,
    sizeof(t_param), 0, A_GIMME, 0);


  class_addanything(param_class, param_anything);
  class_addbang(param_class, param_bang);
  
  param_inlet2_class = class_new(gensym("_param_inlet2"),
    0, 0, sizeof(t_param_inlet2), CLASS_PD | CLASS_NOINLET, 0);
	
  class_addanything(param_inlet2_class, param_inlet2_anything);
  
}
