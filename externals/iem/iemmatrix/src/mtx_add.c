/*
 *  iemmatrix
 *
 *  objects for manipulating simple matrices
 *  mostly refering to matlab/octave matrix functions
 *
 * Copyright (c) IOhannes m zmölnig, forum::für::umläute
 * IEM, Graz, Austria
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */
#include "iemmatrix.h"

/*
  mtx_add
  mtx_+
*/

/* mtx_add */
static t_class *mtx_add_class, *mtx_addscalar_class;

static void mtx_addscalar_matrix(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc-2;
  int row=atom_getfloat(argv), col=atom_getfloat(argv+1);
  
  t_float offset=x->f;
  t_atom *buf;
  t_atom *ap=argv+2;

  if(argc<2){post("mtx_add: crippled matrix");return; }
  adjustsize(&x->m, row, col);

  buf=x->m.atombuffer+2;

  while(n--){
    buf->a_type = A_FLOAT;
    buf++->a_w.w_float = atom_getfloat(ap++) + offset;
  }
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_addscalar_list(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc;
  t_atom *m;
  t_float offset = x->f;
  adjustsize(&x->m, 1, argc);
  m = x->m.atombuffer;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++) + offset;
  }
  outlet_list(x->x_obj.ob_outlet, gensym("list"), argc, x->m.atombuffer);
}

static void mtx_add_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv);
  int col=atom_getfloat(argv+1);
  t_atom *m;
  t_atom *m1 = argv+2;
  t_atom *m2 = x->m2.atombuffer+2;
  int n = argc-2;

  if (argc<2){    post("mtx_add: crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    post("mtx_add: invalid dimensions");    return;  }
  if (col*row>argc-2){    post("sparse matrix not yet supported : use \"mtx_check\"");    return;  }

  if (!(x->m2.col*x->m2.row)) {
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, argv);
    return;
  }

  if ((col!=x->m2.col)||(row!=x->m2.row)){ 
    post("mtx_add: matrix dimensions do not match");
    /* LATER SOLVE THIS */    
    return;
  }
  adjustsize(&x->m, row, col);
  m = x->m.atombuffer+2;

  while(n--){
    t_float f = atom_getfloat(m1++)+atom_getfloat(m2++);
    SETFLOAT(m, f);
    m++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_add_float(t_mtx_binmtx *x, t_float f)
{
  t_matrix *m=&x->m, *m2=&x->m2;
  t_atom *ap, *ap2=m2->atombuffer+2;
  int row2, col2, n;

  if (!m2->atombuffer){ post("mulitply with what ?");            return; }

  row2=atom_getfloat(m2->atombuffer);
  col2=atom_getfloat(m2->atombuffer+1);
  adjustsize(m, row2, col2);
  ap=m->atombuffer+2;

  n=row2*col2;

  while(n--){
    SETFLOAT(ap, f+atom_getfloat(ap2++));
    ap++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), m->row*m->col+2, m->atombuffer);
}
static void *mtx_add_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc>1) post("mtx_add : extra arguments ignored");
  if (argc) {
    t_mtx_binscalar *x = (t_mtx_binscalar *)pd_new(mtx_addscalar_class);
    floatinlet_new(&x->x_obj, &x->f);
    x->f = atom_getfloatarg(0, argc, argv);
    outlet_new(&x->x_obj, 0);
    return(x);
  } else {
    t_mtx_binmtx *x = (t_mtx_binmtx *)pd_new(mtx_add_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
    outlet_new(&x->x_obj, 0);
    x->m.col = x->m.row =  x->m2.col = x->m2.row = 0;
    x->m.atombuffer = x->m2.atombuffer = 0;
    return(x);
  }
}

void mtx_add_setup(void)
{
  mtx_add_class = class_new(gensym("mtx_add"), (t_newmethod)mtx_add_new, (t_method)mtx_binmtx_free,
                            sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)mtx_add_new, gensym("mtx_+"), A_GIMME,0);
  class_addmethod(mtx_add_class, (t_method)mtx_add_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_add_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_add_class, mtx_add_float);
  class_addbang  (mtx_add_class, mtx_binmtx_bang);

  mtx_addscalar_class = class_new(gensym("mtx_add"), 0, (t_method)mtx_binscalar_free,
                                  sizeof(t_mtx_binscalar), 0, 0);
  class_addcreator(0, gensym("mtx_+"), 0, 0);
  class_addmethod(mtx_addscalar_class, (t_method)mtx_addscalar_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_addscalar_class, mtx_addscalar_list);
  class_addbang  (mtx_addscalar_class, mtx_binscalar_bang);

  class_sethelpsymbol(mtx_add_class, gensym("iemmatrix/mtx_binops"));
  class_sethelpsymbol(mtx_addscalar_class, gensym("iemmatrix/mtx_binops"));
}

void iemtx_add_setup(void)
{
  mtx_add_setup();
}
