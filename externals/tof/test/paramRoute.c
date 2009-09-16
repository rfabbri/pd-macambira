/*
 *      paramRoute.c
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


static t_class *paramRoute_class;

typedef struct _paramRoute
{
  t_object                    x_obj;
  t_symbol                    *basepath;
  t_outlet					  *x_outlet;
  t_symbol 					*s_save;
  t_symbol 					*s_load;
  t_symbol*					s_empty;
  t_canvas*					canvas;
  t_symbol*					id;
 
} t_paramRoute;


static t_symbol* paramRoute_makefilename( t_symbol *basename, t_float n) {
	
	
	
	int d = (int) n;
	int length = strlen(basename->s_name);
	char* buf = getbytes( (length + 6) * sizeof (*buf));	
	char* number =  getbytes( 6 * sizeof (*number));
		
	int i;
	int j = 0;
	for ( i=0; i< length; i++ ) {
		if ( (basename->s_name)[i] != '/' ) {
				buf[j] = (basename->s_name)[i];
				j++;
			}
    }
	buf[j] = '\0';
		
	if ( strlen(buf) == 0 ) {
		sprintf(number,"p%03d",d);
	} else {
		sprintf(number,".p%03d",d);
	}
	
	strcat(buf,number);
	t_symbol* filename = gensym(buf);
	freebytes(number, 6 * sizeof (*number));
	freebytes(buf, (length + 6) * sizeof (*buf));
	
	post("File name:%s",filename->s_name);
	return filename;
}

static void paramRoute_anything(t_paramRoute *x, t_symbol *s, int ac, t_atom *av) { 
  
  //I DEACTIVATED THE SAVE & LOAD FEATURES UNTIL I BETTER DEFINE PARAMROUTE'S STATE SAVING
  
  if (s == x->s_save) { // Save
  
	//~ if (x->id) {
      //~ int n = 0;
	  //~ if ( ac  ) n = atom_getfloat(av);
	  //~ t_symbol* filename = paramRoute_makefilename(x->id,n);
	  //~ if ( param_write(x->canvas,filename, x->id) ) 
		//~ pd_error("[paramRoute] could not write %s",filename->s_name);
	//~ } else {
		//~ pd_error(x,"[paramRoute] requires an /id");
	//~ }
  } else if ( s == x->s_load) { // Load
	//~ if (x->id) {
	  //~ int n = 0;
	  //~ if ( ac  ) n = atom_getfloat(av);
	  //~ t_symbol* filename = paramRoute_makefilename(x->id,n);
	  //~ if ( param_read(x->canvas,  paramRoute_makefilename(x->id,n)) )
		//~ pd_error("[paramRoute] could not read %s",filename->s_name);;
	//~ } else {
		//~ pd_error(x,"[paramRoute] requires an /id");
	//~ }
  } else { // Try to send
	  
	  if (ac) {
		int sendBufLength = strlen(x->basepath->s_name) + strlen(s->s_name) + 1;
		char *sendBuf = (char*)getbytes((sendBufLength)*sizeof(char));
	   
		strcpy(sendBuf, x->basepath->s_name);
		strcat(sendBuf, s->s_name);
		t_symbol* target = gensym(sendBuf);
		freebytes(sendBuf, (sendBufLength)*sizeof(char));
		
		if (target->s_thing) {
			if (target->s_thing) pd_forwardmess(target->s_thing, ac, av);
		   } else {
			 outlet_anything(x->x_outlet,s,ac,av);
		   }
		
	  }
	
	  }
	
}

// DECONSTRUCTOR
static void paramRoute_free(t_paramRoute*x)
{
	
}

// CONSTRUCTOR
static void *paramRoute_new(t_symbol *s, int ac, t_atom *av) {
 t_paramRoute *x = (t_paramRoute *)pd_new(paramRoute_class);

     x->s_save = gensym("save");
	 x->s_load = gensym("load");
     x->s_empty = gensym("");

    // String variables
	char *separator = "/";
	char sbuf_name[MAXPDSTRING];
	char sbuf_temp[MAXPDSTRING];
	sbuf_name[0] = '\0';
	sbuf_temp[0] = '\0';
	
	// GET THE CURRENT CANVAS
    t_canvas *canvas=tof_get_canvas();
   
   // Get the root  canvas
   x->canvas = tof_get_root_canvas(canvas);
   
   struct param_build_info build_info;
   get_param_build_info(canvas,ac,av,&build_info,0);
	
	//param_get_id_stuff(canvas,sbuf_name, sbuf_temp, &(x->x_saveable));
    x->id = build_info.id;
  	x->basepath = build_info.basepath;
   //if (x->basepath)  post("PARAMROUTE BASEPATH: %s",x->basepath->s_name);
   //if (x->id)  post("PARAMROUTE ID: %s",x->id->s_name);
    // INLETS AND OUTLETS
    x->x_outlet = outlet_new(&x->x_obj, &s_list);
  return (x);
}

void paramRoute_setup(void) {
  paramRoute_class = class_new(gensym("paramRoute"),
    (t_newmethod)paramRoute_new, (t_method)paramRoute_free,
    sizeof(t_paramRoute), 0, A_GIMME, 0);

 class_addanything(paramRoute_class, paramRoute_anything);
 
}
