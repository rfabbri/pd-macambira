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
  
  //t_symbol 					*x_path;
  t_symbol	                 *s_PARAM;
  //t_symbol                   *x_update_gui;
  t_symbol                   *s_set;
  struct param				  *x_param;
} t_param;

typedef struct _param_inlet2
{
  t_object        x_obj;
  t_param  *p_owner;
} t_param_inlet2;


static void param_bang(t_param *x)
{
	if ( x->x_param) {
		param_output(x->x_param,x->x_obj.ob_outlet);
	
		//if (PARAMECHO) param_send_prepend(x->x_param, x->s_PARAM ,x->x_param->path );
		if(x->x_param->selector != &s_bang ) param_send_prepend(x->x_param, x->x_param->send ,x->s_set );
   }
}

static void param_anything(t_param *x, t_symbol *s, int ac, t_atom *av)
{
 #ifdef PARAMDEBUG
  post("RECEIVING SOMETHING");
 #endif
  if ( x->x_param) set_param_anything(x->x_param,s,ac,av);
  
  param_bang(x);
    
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

	if (x->x_param) {
		pd_unbind(&x->x_obj.ob_pd, x->x_param->receive);
		param_unregister(x->x_param);
	}
    
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
  
  // GET THE CURRENT CANVAS
  t_canvas* canvas=tof_get_canvas();
  
  // GET THE NAME
  t_symbol* name = param_get_name(ac,av);
  
  if (name) {
	  
	  t_symbol* path = param_get_path(canvas,name);
	  t_symbol* root = tof_get_dollarzero(tof_get_root_canvas(canvas));
	  
	  
	  // FIND PARAM VALUE
	  // A. In canvas' arguments
	  // B. In object's arguments
	  // C. Defaults to a bang
	  
	   int ac_p = 0;
		t_atom* av_p = NULL;
	  
			  
	   // A. In canvas' arguments
		int ac_c = 0;
		t_atom* av_c = NULL;
		
		t_canvas * before = tof_get_canvas_before_root(canvas);
		tof_get_canvas_arguments(before,&ac_c , &av_c);
		tof_find_tagged_argument('/',name, ac_c, av_c,&ac_p,&av_p);
	  
		// B. I object's arguments
	  if ( ac_p == 0  && ac > 1) {
		int ac_a = 0;
		t_atom* av_a = NULL;
		tof_find_tagged_argument('/',name, ac, av,&ac_p,&av_p);
		//tof_get_tagged_argument('/',ac,av,&start,&count);
		//if (count > 1) {
		//	ac_p = ac_a;
		//	av_p = av_a + 1;
		//}
	  }
		  
	  
	  
	  
	  //FIND THE GUI TAGS
	  int ac_g = 0;
	  t_atom* av_g = NULL;
	 // There could be a problem if the the name is also /gui
	 tof_find_tagged_argument('/',gensym("/gui"), ac, av,&ac_g,&av_g);
	  
	  x->x_param = param_register(root,path,ac_p,av_p,ac_g,av_g);
	  
	  #ifdef PARAMDEBUG
			post("receive:%s",x->x_param->receive->s_name);
			post("send:%s",x->x_param->send->s_name);
	  #endif
	  
	  // BIND RECEIVER
  	  pd_bind(&x->x_obj.ob_pd, x->x_param->receive );
      // CREATE INLETS AND OUTLETS
      inlet_new((t_object *)x, (t_pd *)p, 0, 0);
      outlet_new(&x->x_obj, &s_list);
	  
	  
  } else {
	  
	   pd_error(x,"Could not create param. See possible errors above.");
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
