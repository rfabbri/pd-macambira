#include "putget~.h"

static t_class *put_tilde_class;

typedef struct _put_tilde {
  t_object  		x_obj;
  //t_sample 			f_put;
  struct putget*	pg; 
  t_sample 			f;
} t_put_tilde;

static t_int* put_tilde_perform(t_int *w)
{
	
  t_put_tilde *x = (t_put_tilde *)(w[1]);
  
  if (x->pg && (x->pg->users > x->pg->writers)) {
	  t_sample  *in =    (t_sample *)(w[2]);
	  int          n =           (int)(w[3]);
	  t_sample *samples = x->pg->w;
	  
	  while (n--) {
		  *samples = *samples + *in;
		  samples++; in++;
		}
			
	}
  return (w+4);
}


static void put_tilde_set(t_put_tilde *x, t_symbol* s) {
	
	if (gensym("") != s ) {
		if ( x->pg ) {
			if ( x->pg->name != s) {
				 putget_unregister(x->pg,1);
				 x->pg = putget_register(s,1);
			}
		} else {
			x->pg = putget_register(s,1);
		}
	}
	
}


static  void put_tilde_dsp(t_put_tilde *x, t_signal **sp)
{
	
	if ( (int) sp[0]->s_n == 64 ) {
		dsp_add(put_tilde_perform, 3, x,sp[0]->s_vec, sp[0]->s_n);
		
	  } else {
		  error("put~ only works with a block size of 64");
	  }
	  
}

static void put_tilde_free( t_put_tilde *x) {

	 if (x->pg) putget_unregister(x->pg,1);
}


static void *put_tilde_new(t_symbol* s)
{
  t_put_tilde *x = (t_put_tilde *)pd_new(put_tilde_class);

  if (gensym("") != s ) x->pg = putget_register(s,1);


  return (void *)x;
}

void put_tilde_setup(void) {
  put_tilde_class = class_new(gensym("put~"),
        (t_newmethod)put_tilde_new,
        (t_method)put_tilde_free, sizeof(t_put_tilde),
        0, A_DEFSYM, 0);
  
  class_addmethod(put_tilde_class,
        (t_method)put_tilde_dsp, gensym("dsp"), 0);
		
  class_addmethod(put_tilde_class,
        (t_method)put_tilde_set, gensym("set"), A_SYMBOL, 0);
		
  CLASS_MAINSIGNALIN(put_tilde_class, t_put_tilde, f);
}
