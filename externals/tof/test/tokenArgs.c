

#include "../src/tof.h"
 

static t_class *tokenArgs_class;
 

typedef struct _tokenArgs
{
  t_object                  	x_obj;
  t_outlet*			       	 	outlet;
  t_canvas*						canvas;
  char							token;
  int                           size;
  int                           ac;
  t_atom*                       av;
} t_tokenArgs;



// Dump out
static void tokenArgs_bang(t_tokenArgs *x) {
	
    int ac_a;
    t_atom* av_a;
    t_symbol* selector_a;
    int iter = 0 ;
	while ( tof_next_tagged_argument(x->token, x->ac, x->av, &ac_a, &av_a,&iter)) {
        tof_set_selector(&selector_a,&ac_a,&av_a);
        outlet_anything(x->outlet, selector_a, ac_a, av_a);
    }
    
}


static void tokenArgs_free(t_tokenArgs *x)
{
    freebytes(x->av,x->ac*sizeof(*(x->av)));
}


static void *tokenArgs_new(t_symbol *s) {
  t_tokenArgs *x = (t_tokenArgs *)pd_new(tokenArgs_class);
  
  
  x->token = s->s_name[0];
  
  //x->canvas = tof_get_canvas();
  // I guess the canvas is currently set :)
  int ac;
  t_atom* av;
  canvas_getargs(&ac, &av);
   
   x->ac = ac;
   x->av = getbytes(x->ac * sizeof(*(x->av)));
   tof_copy_atoms(av,x->av,x->ac);
   
   x->outlet = outlet_new(&x->x_obj, &s_list);
    
  return (x);
}

void tokenArgs_setup(void) {
  tokenArgs_class = class_new(gensym("tokenArgs"),
    (t_newmethod)tokenArgs_new, (t_method)tokenArgs_free,
    sizeof(t_tokenArgs), 0, A_DEFSYMBOL, 0);

 class_addbang(tokenArgs_class, tokenArgs_bang);
 //class_addsymbol(tokenArgs_class, tokenArgs_symbol);

 //class_addmethod(tokenArgs_class, (t_method) tokenArgs_values, gensym("values"), A_DEFSYMBOL,0);
 
 //class_addmethod(tokenArgs_class, (t_method) tokenArgs_guis, gensym("guis"), A_DEFSYMBOL,0);
 //class_addmethod(tokenArgs_class, (t_method) tokenArgs_updateguis, gensym("updateguis"), A_DEFSYMBOL,0);

 
 //class_addmethod(tokenArgs_class, (t_method) tokenArgs_update_guis, gensym("update"), A_DEFSYMBOL,0);

}
