

static t_class *paramRoute_class;

typedef struct _paramRoute
{
  t_object                    x_obj;
  t_symbol                    *path;
  t_outlet					  *x_outlet;
  t_symbol 					*s_save;
  t_symbol 					*s_load;
  t_symbol*					s_empty;
  t_canvas*					canvas;
  t_symbol*					root;
 
} t_paramRoute;


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
		//int sendBufLength = strlen(x->path->s_name) + strlen(s->s_name) + 1;
		//char *sendBuf = (char*)getbytes((sendBufLength)*sizeof(char));
	   if ( s->s_name[0] == '/' && strlen(s->s_name) > 1) {
	    strcpy(param_buf_temp_a, x->root->s_name);
		strcpy(param_buf_temp_b, x->path->s_name);
		
		strcat(param_buf_temp_b, s->s_name+1);
		t_symbol* path = gensym(param_buf_temp_b);
		strcat(param_buf_temp_a, param_buf_temp_b);
		t_symbol* target = gensym(param_buf_temp_a);
		//freebytes(sendBuf, (sendBufLength)*sizeof(char));
		//post("target:%s",target->s_name);
		if (target->s_thing) {
			pd_forwardmess(target->s_thing, ac, av);
		   } else {
			 outlet_anything(x->x_outlet,path,ac,av);
		   }
		} else {
            outlet_anything(x->x_outlet,path,ac,av);
			//pd_error(x,"Target name must start with a \"/\"");
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

	
	// GET THE CURRENT CANVAS
    t_canvas *canvas=tof_get_canvas();
   
   // Get the root  canvas
   x->canvas = tof_get_root_canvas(canvas);
   
   x->root = tof_get_dollarzero(x->canvas);
   
   x->path = param_get_path(canvas,NULL);
   
    // INLETS AND OUTLETS
    x->x_outlet = outlet_new(&x->x_obj, &s_list);
  return (x);
}

void paramRoute_setup(void) {
  paramRoute_class = class_new(gensym("param route"),
    (t_newmethod)paramRoute_new, (t_method)paramRoute_free,
    sizeof(t_paramRoute), 0, A_GIMME, 0);

 class_addanything(paramRoute_class, paramRoute_anything);
 
 class_sethelpsymbol(paramRoute_class, gensym("param"));
 
}
