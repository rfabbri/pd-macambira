/*
 *  iemmatrix
 *
 *  objects for manipulating simple matrices
 *  mostly refering to matlab/octave matrix functions
 *
 * (c) IOhannes m zmölnig, forum::für::umläute
 * 
 * IEM, Graz
 *
 * this code is published under the LGPL
 *
 */
#include "iemmatrix.h"


#include "iemmatrix.h"

/* mtx_print */
static t_class *mtx_print_class;
static void mtx_print(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row;
  if (argc<2){
    post("mtx_print : crippled matrix");
    return;
  }
  row = atom_getfloat(argv++);
  col = atom_getfloat(argv++);
  if(row*col>argc-2){
    post("mtx_print : sparse matrices not yet supported : use \"mtx_check\"");
    return;
  }
  post("matrix:");
  while(row--){
    postatom(col, argv);
    argv+=col;
    endpost();
  }
  endpost();
}
static void *mtx_print_new(void)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_print_class);
  x->row = x->col = 0;
  x->atombuffer   = 0;
  return (x);
}
void mtx_print_setup(void)
{
  mtx_print_class = class_new(gensym("mtx_print"), (t_newmethod)mtx_print_new, 
			      0, sizeof(t_matrix), 0, 0, 0);
  class_addmethod  (mtx_print_class, (t_method)mtx_print, gensym("matrix"), A_GIMME, 0);
  class_sethelpsymbol(mtx_print_class, gensym("iemmatrix/matrix"));
}

void iemtx_print_setup(void){
  mtx_print_setup();
}

