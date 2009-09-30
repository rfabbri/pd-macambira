#include "putget~.h"

static t_class *get_tilde_class;

typedef struct _get_tilde {
  t_object  		x_obj;
  t_sample 			f_put;
  struct putget*	pg; 
  t_sample 			f;
} t_get_tilde;

t_int *get_tilde_perform(t_int *w)
{
  t_get_tilde *x = (t_get_tilde *)(w[1]);
  t_sample  *out =    (t_sample *)(w[2]);
  int          n =           (int)(w[3]);
  
   if ( x->pg) {
   
	putget_arm( x->pg);
	 
   
	   t_sample *samples = x->pg->r;
		
		while (n--) {
		  *out++ = *samples++;
		}
	} else {
		while (n--) {
		  *out++ = 0;
		}
	}
	
  return (w+4);
}

static void get_tilde_set(t_get_tilde *x, t_symbol* s) {
	
	if (gensym("") != s ) {
		if ( x->pg ) {
			if ( x->pg->name != s) {
				 putget_unregister(x->pg,0);
				 x->pg = putget_register(s,0);
			}
		} else {
			x->pg = putget_register(s,0);
		}
	}
	
}

void get_tilde_dsp(t_get_tilde *x, t_signal **sp)
{
	
	if ( sp[0]->s_n == 64 ) {
		dsp_add(get_tilde_perform, 3, x,sp[0]->s_vec, sp[0]->s_n);
	  } else {
		  pd_error(x,"get~ only works with a block size of 64");
	  }
}

static void get_tilde_free( t_get_tilde *x) {
	
	 if (x->pg) putget_unregister(x->pg,0);
}


void *get_tilde_new(t_symbol* s)
{
  t_get_tilde *x = (t_get_tilde *)pd_new(get_tilde_class);

  
 if (gensym("") != s ) x->pg = putget_register(s,0);
 
  outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

void get_tilde_setup(void) {
  get_tilde_class = class_new(gensym("get~"),
        (t_newmethod)get_tilde_new,
        (t_method)get_tilde_free, sizeof(t_get_tilde),
        CLASS_DEFAULT, 
        A_DEFSYMBOL, 0);

  class_addmethod(get_tilde_class,
        (t_method)get_tilde_set, gensym("set"), A_SYMBOL, 0);

  class_addmethod(get_tilde_class,
        (t_method)get_tilde_dsp, gensym("dsp"), 0);
  //CLASS_MAINSIGNALIN(get_tilde_class, t_get_tilde, f);
}
