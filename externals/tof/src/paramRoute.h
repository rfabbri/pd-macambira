

static t_class *paramRoute_class;

typedef struct _paramRoute
{
  t_object                      x_obj;
  t_symbol*                     path;
  t_outlet*                     x_outlet;
  t_symbol*                     s_save;
  t_symbol*                     s_load;
  t_symbol*					    s_empty;
  t_canvas*					    canvas;
  t_symbol*					    root;
 
} t_paramRoute;


static void paramRoute_anything(t_paramRoute *x, t_symbol *s, int ac, t_atom *av) { 
  
    
 // t_symbol* path;
  t_symbol* target;
  
 
	  if (ac) {
			   if ( s->s_name[0] == '/' && strlen(s->s_name) > 1) {
                    strcpy(param_buf_temp_a, x->root->s_name);
                    strcpy(param_buf_temp_b, x->path->s_name);

                    strcat(param_buf_temp_b, s->s_name+1);
                    //path = gensym(param_buf_temp_b);
                    strcat(param_buf_temp_a, param_buf_temp_b);
                    target = gensym(param_buf_temp_a);

                    if (target->s_thing) {
                        pd_forwardmess(target->s_thing, ac, av);
                    } else {
                        outlet_anything(x->x_outlet,s,ac,av);
                    }
                } else {
                    outlet_anything(x->x_outlet,s,ac,av);
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
