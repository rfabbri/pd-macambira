

//#include "tof.h"
//#include "param.h"
#include "m_pd.h"
#include "g_canvas.h" 

static t_class *test_class;
 

typedef struct _test
{
  t_object					x_obj;
  t_outlet*					outlet;
  t_symbol*					name;
  t_canvas*					canvas;
  //t_symbol*					target;
  //t_class*					empty_s;
  //t_symbol*					root;

} t_test;



// Dump out
static void test_bang(t_test *x) {
	t_atom a;
	if (x->canvas) {
	  
		SETFLOAT(&a,1);
		pd_typedmess((t_pd*)x->canvas,gensym("vis"),1,&a);
   }
    
}



static void test_free(t_test *x)
{
	if (x->canvas) {
		//post("Deleting it");
		pd_free((t_pd *)x->canvas);
	}
	x->canvas = NULL;
}


static void *test_new(t_symbol *s, int ac, t_atom *av) {
  t_test *x = (t_test *)pd_new(test_class);
  
  // create a new canvas
  x->name = gensym("xiu");
  
  t_atom a;
  SETSYMBOL(&a, x->name);
  pd_typedmess(&pd_objectmaker,gensym("pd"),1,&a);
  
  x->canvas = (t_canvas*) pd_newest();
  
  // From this point on, we are hoping the "pd" object has been created
  
	// Change name to match pd's canvas naming scheme
	x->name = gensym("pd-xiu");
	
	
	
  // Hide the window (stupid way of doing this)
    if (x->canvas) {
		SETFLOAT(&a,0);
		pd_typedmess((t_pd*)x->canvas,gensym("vis"),1,&a);
   }
	
	// Try to change its name
	//canvas_setargs(int argc, t_atom *argv);
	
   x->outlet = outlet_new(&x->x_obj, &s_list);
    
  return (x);
}

void test_setup(void) {
  test_class = class_new(gensym("test"),
    (t_newmethod)test_new, (t_method)test_free,
    sizeof(t_test), 0, A_GIMME, 0);

 class_addbang(test_class, test_bang);
 //class_addsymbol(test_class, test_symbol);

 //class_addmethod(test_class, (t_method) test_values, gensym("values"), A_DEFSYMBOL,0);
 
 //class_addmethod(test_class, (t_method) test_guis, gensym("guis"), A_DEFSYMBOL,0);
 //class_addmethod(test_class, (t_method) test_updateguis, gensym("updateguis"), A_DEFSYMBOL,0);

 
 //class_addmethod(test_class, (t_method) test_update_guis, gensym("update"), A_DEFSYMBOL,0);

}
