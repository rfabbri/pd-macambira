#include "m_pd.h"
#define IS_A_SYMBOL(atom,index) ((atom+index)->a_type == A_SYMBOL)
#define IS_A_FLOAT(atom,index) ((atom+index)->a_type == A_FLOAT)


static t_class *listUnfold_class;

typedef struct _listUnfold {
  t_object  x_obj;

  //t_float start;
  //t_float step;
   t_int iterating;
  t_outlet* outlet1;
  t_outlet* outlet2;
} t_listUnfold;


void listUnfold_bang(t_listUnfold *x)
{
  //x->i_count = x->i_down;
  x->iterating = 0;
  
}


void listUnfold_anything(t_listUnfold *x, t_symbol* s, int ac, t_atom* av)
{
	int i = 0;
	int count = 0;
    x->iterating = 1;
	 
	if ( s != &s_list && s != &s_float && s != &s_symbol ) {
		outlet_float(x->outlet2,0);
		outlet_symbol(x->outlet1,s);
		count++;
	}
	
	for ( ; i < ac; i++ ) {
		if ( !(x->iterating) ) break;
		outlet_float(x->outlet2,count);
		count++;
		if ( IS_A_FLOAT(av,i) ) {
			outlet_float(x->outlet1,atom_getfloat(av+i));
		} else {
			outlet_symbol(x->outlet1,atom_getsymbol(av+i));
		}
		
	}
	  
	
}


static void listUnfold_free(t_listUnfold *x)
{
		
    
}

void *listUnfold_new(t_symbol *s, int argc, t_atom *argv)
{
  t_listUnfold *x = (t_listUnfold *)pd_new(listUnfold_class);
  
  x->iterating = 0;
 
  
  //floatinlet_new(&x->x_obj, &x->start);
  //floatinlet_new(&x->x_obj, &x->step);
  x->outlet1 = outlet_new(&x->x_obj, &s_list);
  x->outlet2 = outlet_new(&x->x_obj, &s_float);


  return (void *)x;
}

void listUnfold_setup(void) {
  listUnfold_class = class_new(gensym("listUnfold"),
        (t_newmethod)listUnfold_new,
        (t_method)listUnfold_free, sizeof(t_listUnfold),
        CLASS_DEFAULT, 
        A_GIMME, 0);

  class_addbang  (listUnfold_class, listUnfold_bang);
  class_addanything (listUnfold_class, listUnfold_anything);
  //class_addlist (listUnfold_class, listUnfold_list);
  
}
