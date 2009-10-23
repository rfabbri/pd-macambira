

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
  paramId_class = class_new(gensym("param id"),
    (t_newmethod)paramId_new, (t_method)paramId_free,
    sizeof(t_paramId), 0, A_GIMME, 0);

 class_addbang(paramId_class, paramId_bang);
 
 class_sethelpsymbol(paramId_class, gensym("param"));
 
}
