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

/*
  mtx_add
  mtx_+
  mtx_mul
  mtx_*
  mtx_.*
  mtx_./
*/

/* -­------------------------------------------------------------- */
/* matrix math */

typedef struct _mtx_binscalar
{
  t_object x_obj;

  t_matrix m; // the output matrix
  t_float f;  // the second input
} t_mtx_binscalar;

typedef struct _mtx_binmtx
{
  t_object x_obj;

  t_matrix m;  // the output matrix
  t_matrix m2; // the second input
} t_mtx_binmtx;

static void mtx_bin_matrix2(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row = atom_getfloat(argv);
  int col = atom_getfloat(argv+1);
  if (argc<2){post("mtx_bin2: crippled matrix"); return;}
  if ((col<1)||(row<1)) {post("mtx_bin2: invalid dimensions %dx%d", row,col); return;}
  if (col*row+2>argc){ post("mtx_bin2: sparse matrix not yet supported : use \"mtx_check\""); return;}

  if (row*col!=x->m2.row*x->m2.col) {
    freebytes(x->m2.atombuffer, (x->m2.row*x->m2.col+2)*sizeof(t_atom));
    x->m2.atombuffer=copybytes(argv,(row*col+2)*sizeof(t_atom));
  }else memcpy(x->m2.atombuffer, argv, (row*col+2)*sizeof(t_atom));
  setdimen(&x->m2, row, col);
}

static void mtx_binmtx_bang(t_mtx_binmtx *x)
{
  if((&x->m)&&(x->m.atombuffer))
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), x->m.col*x->m.row+2, x->m.atombuffer);
}


static void mtx_binmtx_free(t_mtx_binmtx *x)
{
  matrix_free(&x->m);
  matrix_free(&x->m2);
}
static void mtx_binscalar_bang(t_mtx_binscalar *x)
{
  if((&x->m)&&(x->m.atombuffer))
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), x->m.col*x->m.row+2, x->m.atombuffer);
}
static void mtx_binscalar_free(t_mtx_binscalar *x)
{
  matrix_free(&x->m);
}



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

static void mtx_add_setup(void)
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

/* mtx_sub */
static t_class *mtx_sub_class, *mtx_subscalar_class;

static void mtx_subscalar_matrix(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc-2;
  int row=atom_getfloat(argv), col=atom_getfloat(argv+1);
  
  t_float offset=x->f;
  t_atom *buf;
  t_atom *ap=argv+2;

  if(argc<2){post("mtx_sub: crippled matrix");return; }
  adjustsize(&x->m, row, col);

  buf=x->m.atombuffer+2;

  while(n--){
    buf->a_type = A_FLOAT;
    buf++->a_w.w_float = atom_getfloat(ap++) - offset;
  }
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_subscalar_list(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc;
  t_atom *m;
  t_float offset = x->f;
  adjustsize(&x->m, 1, argc);
  m = x->m.atombuffer;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++) - offset;
  }
  outlet_list(x->x_obj.ob_outlet, gensym("list"), argc, x->m.atombuffer);
}

static void mtx_sub_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv);
  int col=atom_getfloat(argv+1);
  t_atom *m;
  t_atom *m1 = argv+2;
  t_atom *m2 = x->m2.atombuffer+2;
  int n = argc-2;

  if (argc<2){    post("mtx_sub: crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    post("mtx_sub: invalid dimensions");    return;  }
  if (col*row>argc-2){    post("sparse matrix not yet supported : use \"mtx_check\"");    return;  }

  if (!(x->m2.col*x->m2.row)) {
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, argv);
    return;
  }

  if ((col!=x->m2.col)||(row!=x->m2.row)){ 
    post("mtx_sub: matrix dimensions do not match");
    /* LATER SOLVE THIS */    
    return;
  }
  adjustsize(&x->m, row, col);
  m = x->m.atombuffer+2;

  while(n--){
    t_float f = atom_getfloat(m1++)-atom_getfloat(m2++);
    SETFLOAT(m, f);
    m++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_sub_float(t_mtx_binmtx *x, t_float f)
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
    SETFLOAT(ap, f-atom_getfloat(ap2++));
    ap++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), m->row*m->col+2, m->atombuffer);
}
static void *mtx_sub_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc>1) post("mtx_sub : extra arguments ignored");
  if (argc) {
    t_mtx_binscalar *x = (t_mtx_binscalar *)pd_new(mtx_subscalar_class);
    floatinlet_new(&x->x_obj, &x->f);
    x->f = atom_getfloatarg(0, argc, argv);
    outlet_new(&x->x_obj, 0);
    return(x);
  } else {
    t_mtx_binmtx *x = (t_mtx_binmtx *)pd_new(mtx_sub_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
    outlet_new(&x->x_obj, 0);
    x->m.col = x->m.row =  x->m2.col = x->m2.row = 0;
    x->m.atombuffer = x->m2.atombuffer = 0;
    return(x);
  }
}

static void mtx_sub_setup(void)
{
  mtx_sub_class = class_new(gensym("mtx_sub"), (t_newmethod)mtx_sub_new, (t_method)mtx_binmtx_free,
			    sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)mtx_sub_new, gensym("mtx_-"), A_GIMME,0);
  class_addmethod(mtx_sub_class, (t_method)mtx_sub_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_sub_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_sub_class, mtx_sub_float);
  class_addbang  (mtx_sub_class, mtx_binmtx_bang);

  mtx_subscalar_class = class_new(gensym("mtx_sub"), 0, (t_method)mtx_binscalar_free,
				  sizeof(t_mtx_binscalar), 0, 0);
  class_addcreator(0, gensym("mtx_-"), 0, 0);
  class_addmethod(mtx_subscalar_class, (t_method)mtx_subscalar_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_subscalar_class, mtx_subscalar_list);
  class_addbang  (mtx_subscalar_class, mtx_binscalar_bang);

  class_sethelpsymbol(mtx_sub_class, gensym("iemmatrix/mtx_binops"));
  class_sethelpsymbol(mtx_subscalar_class, gensym("iemmatrix/mtx_binops"));
}


/* mtx_mul */
static t_class *mtx_mul_class, *mtx_mulelement_class, *mtx_mulscalar_class;

static void mtx_mul_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *m=&x->m, *m2=&x->m2;
  t_atom *ap, *ap1=argv+2, *ap2=m2->atombuffer+2;
  int row=atom_getfloat(argv), col=atom_getfloat(argv+1);
  int row2, col2, n, r, c;

  if (!m2->atombuffer){ post("mulitply with what ?");            return; }
  if (argc<2){          post("mtx_mul: crippled matrix");        return; }
  if ((col<1)||(row<1)){post("mtx_mul: invalid dimensions");     return; }
  if (col*row>argc-2){  post("sparse matrix not yet supported : use \"mtx_check\""); return; }

  row2=atom_getfloat(m2->atombuffer);
  col2=atom_getfloat(m2->atombuffer+1);
 
  if (col!=row2) {      post("mtx_mul: matrix dimensions do not match !"); return;  }

  adjustsize(m, row, col2); 
  ap=m->atombuffer+2;

  for(r=0;r<row;r++)
    for(c=0;c<col2;c++) {
      t_matrixfloat sum = 0.f;
      for(n=0;n<col;n++)sum+=(t_matrixfloat)atom_getfloat(ap1+col*r+n)*atom_getfloat(ap2+col2*n+c);
      SETFLOAT(ap+col2*r+c,sum);
    }
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), m->row*m->col+2, m->atombuffer);
}

static void mtx_mul_float(t_mtx_binmtx *x, t_float f)
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
    SETFLOAT(ap, f*atom_getfloat(ap2++));
    ap++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), m->row*m->col+2, m->atombuffer);
}

static void mtx_mulelement_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  t_atom *m;
  t_atom *m2 = x->m2.atombuffer+2;
  int n = argc-2;

  if (argc<2){    post("mtx_mul: crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    post("mtx_mul: invalid dimensions");    return;  }
  if (col*row>argc-2){    post("sparse matrix not yet supported : use \"mtx_check\"");    return;  }
  if (!(x->m2.col*x->m2.row)) {
    adjustsize(&x->m, row, col);
    matrix_set(&x->m, 0);
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
    return;
  }
  if ((col!=x->m2.col)||(row!=x->m2.row)){    post("matrix dimension do not match");    /* LATER SOLVE THIS */    return;  }

  adjustsize(&x->m, row, col);
  m =  x->m.atombuffer+2;

  while(n--){
    t_float f = atom_getfloat(argv++)*atom_getfloat(m2++);
    SETFLOAT(m, f);
    m++;
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}

static void mtx_mulscalar_matrix(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc-2;
  t_atom *m;
  t_float factor = x->f;
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);

  if (argc<2){
    post("mtx_mul: crippled matrix");
    return;
  }
  adjustsize(&x->m, row, col);
  m = x->m.atombuffer+2;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++)*factor;
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_mulscalar_list(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc;
  t_atom *m;
  t_float factor = x->f;
  adjustsize(&x->m, 1, argc);
  m = x->m.atombuffer;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++)*factor;
  }
  outlet_list(x->x_obj.ob_outlet, gensym("list"), argc, x->m.atombuffer);
}

static void *mtx_mul_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc>1) post("mtx_mul : extra arguments ignored");
  if (argc) {
    t_mtx_binscalar *x = (t_mtx_binscalar *)pd_new(mtx_mulscalar_class);
    floatinlet_new(&x->x_obj, &x->f);
    x->f = atom_getfloatarg(0, argc, argv);
    outlet_new(&x->x_obj, 0);
    return(x);
  } else {
    if (s->s_name[4]=='.') {
      /* element mul */

      t_matrix *x = (t_matrix *)pd_new(mtx_mulelement_class);
      inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
      outlet_new(&x->x_obj, 0);
      x->col = x->row = 0;
      x->atombuffer = 0;
      return(x);
    } else {
      t_mtx_binmtx *x = (t_mtx_binmtx *)pd_new(mtx_mul_class);
      inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
      outlet_new(&x->x_obj, 0);
      x->m.col = x->m.row = x->m2.col = x->m2.row = 0;
      x->m.atombuffer = x->m2.atombuffer = 0;
      return (x);
    }
  }
}

static void mtx_mul_setup(void)
{
  mtx_mul_class = class_new(gensym("mtx_mul"), (t_newmethod)mtx_mul_new, (t_method)mtx_binmtx_free,
			    sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)mtx_mul_new, gensym("mtx_*"), A_GIMME,0);
  class_addmethod(mtx_mul_class, (t_method)mtx_mul_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_mul_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_mul_class, mtx_mul_float);
  class_addbang  (mtx_mul_class, mtx_binmtx_bang);

  mtx_mulelement_class = class_new(gensym("mtx_.*"), (t_newmethod)mtx_mul_new, (t_method)mtx_binmtx_free,
				   sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addmethod(mtx_mulelement_class, (t_method)mtx_mulelement_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_mulelement_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_mulelement_class, mtx_mul_float);
  class_addbang  (mtx_mulelement_class, mtx_binmtx_bang);

  mtx_mulscalar_class = class_new(gensym("mtx_mul"), 0, (t_method)mtx_binscalar_free,
				  sizeof(t_mtx_binscalar), 0, 0);
  class_addcreator(0, gensym("mtx_*"), 0, 0);
  class_addcreator(0, gensym("mtx_.*"), 0, 0);
  class_addmethod(mtx_mulscalar_class, (t_method)mtx_mulscalar_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_mulscalar_class, mtx_mulscalar_list);
  class_addbang  (mtx_mulscalar_class, mtx_binscalar_bang);

  class_sethelpsymbol(mtx_mul_class, gensym("iemmatrix/mtx_binops"));
  class_sethelpsymbol(mtx_mulelement_class, gensym("iemmatrix/mtx_binops"));
  class_sethelpsymbol(mtx_mulscalar_class, gensym("iemmatrix/mtx_binops"));
}


/* mtx_div */
static t_class *mtx_divelement_class, *mtx_divscalar_class;

static void mtx_divelement_matrix(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv)
{
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);
  t_atom *m;
  t_atom *m2 = x->m2.atombuffer+2;
  int n = argc-2;

  if (argc<2){    post("mtx_div: crippled matrix");    return;  }
  if ((col<1)||(row<1)) {    post("mtx_div: invalid dimensions");    return;  }
  if (col*row>argc-2){    post("sparse matrix not yet supported : use \"mtx_check\"");    return;  }
  if (!(x->m2.col*x->m2.row)) {
    adjustsize(&x->m, row, col);
    matrix_set(&x->m, 0);
    outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
    return;
  }
  if ((col!=x->m2.col)||(row!=x->m2.row)){    post("matrix dimension do not match");    /* LATER SOLVE THIS */    return;  }

  adjustsize(&x->m, row, col);
  m =  x->m.atombuffer+2;

  while(n--){
    t_float f = atom_getfloat(argv++)/atom_getfloat(m2++);
    SETFLOAT(m, f);
    m++;
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_divelement_float(t_mtx_binmtx *x, t_float f)
{
  t_matrix *m=&x->m, *m2=&x->m2;
  t_atom *ap, *ap2=m2->atombuffer+2;
  int row2, col2, n;

  if (!m2->atombuffer){ post("divide by what ?");            return; }

  row2=atom_getfloat(m2->atombuffer);
  col2=atom_getfloat(m2->atombuffer+1);
  adjustsize(m, row2, col2);
  ap=m->atombuffer+2;

  n=row2*col2;

  while(n--){
    SETFLOAT(ap, f/atom_getfloat(ap2++));
    ap++;
  }
  
  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), m->row*m->col+2, m->atombuffer);
}
static void mtx_divscalar_matrix(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc-2;
  t_atom *m;
  t_float factor = 1.0/x->f;
  int row=atom_getfloat(argv++);
  int col=atom_getfloat(argv++);

  if (argc<2){
    post("mtx_div: crippled matrix");
    return;
  }
  adjustsize(&x->m, row, col);
  m = x->m.atombuffer+2;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++)*factor;
  }

  outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), argc, x->m.atombuffer);
}
static void mtx_divscalar_list(t_mtx_binscalar *x, t_symbol *s, int argc, t_atom *argv)
{
  int n=argc;
  t_atom *m;
  t_float factor = 1.0/x->f;

  adjustsize(&x->m, 1, argc);
  m = x->m.atombuffer;

  while(n--){
    m->a_type = A_FLOAT;
    (m++)->a_w.w_float = atom_getfloat(argv++)*factor;
  }

  outlet_list(x->x_obj.ob_outlet, gensym("list"), argc, x->m.atombuffer);
}

static void *mtx_div_new(t_symbol *s, int argc, t_atom *argv)
{
  if (argc>1) post("mtx_div : extra arguments ignored");
  if (argc) {
    /* scalar division */
    t_mtx_binscalar *x = (t_mtx_binscalar *)pd_new(mtx_divscalar_class);
    floatinlet_new(&x->x_obj, &x->f);
    x->f = atom_getfloatarg(0, argc, argv);
    outlet_new(&x->x_obj, 0);
    return(x);
  } else {
    /* element division */
    t_matrix *x = (t_matrix *)pd_new(mtx_divelement_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
    outlet_new(&x->x_obj, 0);
    x->col = x->row = 0;
    x->atombuffer = 0;
    return(x);
  }
}

static void mtx_div_setup(void)
{
  mtx_divelement_class = class_new(gensym("mtx_./"), (t_newmethod)mtx_div_new, (t_method)mtx_binmtx_free,
				   sizeof(t_mtx_binmtx), 0, A_GIMME, 0);
  class_addmethod(mtx_divelement_class, (t_method)mtx_divelement_matrix, gensym("matrix"), A_GIMME, 0);
  class_addmethod(mtx_divelement_class, (t_method)mtx_bin_matrix2, gensym(""), A_GIMME, 0);
  class_addfloat (mtx_divelement_class, mtx_divelement_float);
  class_addbang  (mtx_divelement_class, mtx_binmtx_bang);

  mtx_divscalar_class = class_new(gensym("mtx_./"), 0, (t_method)mtx_binscalar_free,
				  sizeof(t_mtx_binscalar), 0, 0);
  class_addmethod(mtx_divscalar_class, (t_method)mtx_divscalar_matrix, gensym("matrix"), A_GIMME, 0);
  class_addlist  (mtx_divscalar_class, mtx_divscalar_list);
  class_addbang  (mtx_divscalar_class, mtx_binscalar_bang);

  class_sethelpsymbol(mtx_divelement_class, gensym("iemmatrix/mtx_binops"));
  class_sethelpsymbol(mtx_divscalar_class, gensym("iemmatrix/mtx_binops"));
}

void mtx_binops_setup(void)
{
  mtx_add_setup();
  mtx_sub_setup();
  mtx_mul_setup();
  mtx_div_setup();
}
void iemtx_binops_setup(void)
{
  mtx_binops_setup();
}
