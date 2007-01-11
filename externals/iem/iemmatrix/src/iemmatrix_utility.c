/*
 *  iemmatrix_utility
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

/*
  G.Holzmann: this has been in mtx_matrix.c before
              now here should be the shared code !!!
*/

#include "iemmatrix.h"


/* utility functions */

void setdimen(t_matrix *x, int row, int col)
{
  x->col = col;
  x->row = row;
  SETFLOAT(x->atombuffer,   row);
  SETFLOAT(x->atombuffer+1, col);
}

void adjustsize(t_matrix *x, int desiredRow, int desiredCol)
{
  int col=x->col, row=x->row;

  if (desiredRow<1){
    post("cannot make less than 1 rows");
    desiredRow=1;
  }
  if (desiredCol<1){
    post("cannot make less than 1 columns");
    desiredCol=1;
  }

  if (col*row!=desiredRow*desiredCol){
    if(x->atombuffer)freebytes(x->atombuffer, (col*row+2)*sizeof(t_atom));
    x->atombuffer=(t_atom *)getbytes((desiredCol*desiredRow+2)*sizeof(t_atom));
  }

  setdimen(x, desiredRow, desiredCol);
  return;
}

void debugmtx(int argc, t_float *buf, int id)
{
  int i=argc;
  while(i--){
	  int j=argc;
	  startpost("debug%d: ", id);
	  while(j--)
		  startpost("%f  ", *buf++);
	  endpost();
  }
}

t_matrixfloat *matrix2float(t_atom *ap)
{
  int row = atom_getfloat(ap++);
  int col=atom_getfloat(ap++);
  int length   = row * col;
  t_matrixfloat *buffer = (t_matrixfloat *)getbytes(sizeof(t_matrixfloat)*length);
  t_matrixfloat *buf = buffer;
  while(length--)*buf++=atom_getfloat(ap++);
  return buffer;
}

void float2matrix(t_atom *ap, t_matrixfloat *buffer)
{
  int row=atom_getfloat(ap++);
  int col=atom_getfloat(ap++);
  int length = row * col;
  t_matrixfloat*buf= buffer;
  while(length--){
    SETFLOAT(ap, *buf++);
    ap++;
  }
  freebytes(buffer, row*col*sizeof(t_matrixfloat));
}

/* core functions */
void matrix_bang(t_matrix *x)
{
  /* output the matrix */
  if (x->atombuffer)outlet_anything(x->x_obj.ob_outlet, gensym("matrix"), x->col*x->row+2, x->atombuffer);
}

void matrix_matrix2(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int row, col;

  if (argc<2){
    post("matrix : corrupt matrix passed");
    return;
  }
  row = atom_getfloat(argv);
  col = atom_getfloat(argv+1);
  if ((row<1)||(col<1)){
    post("matrix : corrupt matrix passed");
    return;
  }
  if (row*col > argc-2){
    post("matrix: sparse matrices not yet supported : use \"mtx_check\"");
    return;
  }

  /* this is fast and dirty, MAYBE make it slow and clean */
  /* or, to clean matrices, use the mtx_check object */
  if (row*col != x->row*x->col) {
    freebytes(x->atombuffer, x->row*x->col*sizeof(t_atom));
    x->atombuffer = copybytes(argv, (row*col+2)*sizeof(t_atom));
  } else memcpy(x->atombuffer, argv, (row*col+2)*sizeof(t_atom));

  setdimen(x, row, col);
}


/* set data */

void matrix_set(t_matrix *x, t_float f)
{
  int size = x->col * x->row;
  t_atom *buf=x->atombuffer+2;
  if(x->atombuffer)while(size--)SETFLOAT(&buf[size], f);
}

void matrix_zeros(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row;

  switch(argc) {
  case 0: /* zero out the actual matrix */
    matrix_set(x, 0);
    break;
  case 1:
    row=atom_getfloat(argv);
    adjustsize(x, row, row);
    matrix_set(x, 0);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
    adjustsize(x, row, col);

    matrix_set(x, 0);
  }

  matrix_bang(x);
}

void matrix_ones(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row;

  switch(argc) {
  case 0: /* zero out the actual matrix */
    matrix_set(x, 1);
    break;
  case 1:
    row=atom_getfloat(argv);
    adjustsize(x, row, row);
    matrix_set(x, 1);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
    adjustsize(x, row, col);

    matrix_set(x, 1);
  }

  matrix_bang(x);
}

void matrix_eye(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row;
  int n;

  switch(argc) {
  case 0: /* zero out the actual matrix */
    matrix_set(x, 0);
    break;
  case 1:
    row=atom_getfloat(argv);
    adjustsize(x, row, row);
    matrix_set(x, 0);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
    adjustsize(x, row, col);
    matrix_set(x, 0);
  }

  col=x->col;
  row=x->row;
  n = (col<row)?col:row;
  while(n--)SETFLOAT(x->atombuffer+2+n*(1+col), 1);
  
  matrix_bang(x);
}

void matrix_egg(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row;
  int n;

  switch(argc) {
  case 0: /* zero out the actual matrix */
    matrix_set(x, 0);
    break;
  case 1:
    row=atom_getfloat(argv);
    adjustsize(x, row, row);
    matrix_set(x, 0);
    break;
  default:
    row=atom_getfloat(argv++);
    col=atom_getfloat(argv);
    adjustsize(x, row, col);
    matrix_set(x, 0);
  }

  col=x->col;
  row=x->row;
  n = (col<row)?col:row;
  while(n--)SETFLOAT(x->atombuffer+2+(n+1)*(col-1), 1);
  
  matrix_bang(x);
}

void matrix_diag(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int col=argc;
  argv+=argc-1;
  if (argc<1) {
    post("matrix: no diagonale present");
    return;
  }
  adjustsize(x, argc, argc);
  matrix_set(x, 0);

  while(argc--)SETFLOAT(x->atombuffer+2+argc*(1+col), atom_getfloat(argv--));

  matrix_bang(x);
}

void matrix_diegg(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  int col=argc;
  argv+=argc-1;
  if (argc<1) {
    post("matrix: no dieggonale present");
    return;
  }
  adjustsize(x, argc, argc);
  matrix_set(x, 0);

  while(argc--){
	  t_atom *ap=x->atombuffer+2+(argc+1)*(col-1);
	  SETFLOAT(ap, atom_getfloat(argv--));
  }

  matrix_bang(x);
}


/* the rest */

void matrix_row(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  t_atom *ap;
  int row=x->row, col=x->col;
  int r;
  t_float f;

  switch (argc){
  case 0:
    for (r=0;r<row;r++)outlet_list(x->x_obj.ob_outlet, gensym("row"), col, x->atombuffer+r*col+2);
    break;
  case 1:
    r=atom_getfloat(argv)-1;
    if ((r<0)||(r>=row)){
      post("matrix: row index %d is out of range", r+1);
      return;
    }
    outlet_list(x->x_obj.ob_outlet, gensym("row"), col, x->atombuffer+r*col+2);
    break;
  case 2:
    r=atom_getfloat(argv)-1;
    f=atom_getfloat(argv+1);
    if ((r<0)||(r>=row)){
      post("matrix: row index %d is out of range", r+1);
      return;
    }
    
    
  default:
    r=atom_getfloat(argv++)-1;
    if (argc--<col){
      post("matrix: sparse rows not yet supported : use \"mtx_check\"");
      return;
    }
    if ((r<0)||(r>=row)){
      post("matrix: row index %d is out of range", r+1);
      return;
    }
    if (r==row) {
    } else {
      ap=x->atombuffer+2+col*r;
      memcpy(ap, argv, col*sizeof(t_atom));
    }
  }
}

void matrix_col(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  t_atom *ap;
  int row=x->row, col=x->col;
  int c, r;

  switch (argc){
  case 0:
    ap=(t_atom *)getbytes(row*sizeof(t_atom));
    for (c=0;c<col;c++) {
      for (r=0;r<row;r++)SETFLOAT(&ap[r], atom_getfloat(x->atombuffer+2+c+col*r));
      outlet_list(x->x_obj.ob_outlet, gensym("col"), row, ap);
    }
    freebytes(ap, row*sizeof(t_atom));
    break;
  case 1:
    ap=(t_atom *)getbytes(row*sizeof(t_atom));
    c=atom_getfloat(argv)-1;
    if ((c<0)||(c>=col)){
      post("matrix: col index %d is out of range", c+1);
      return;
    }
    for (r=0;r<row;r++)SETFLOAT(&ap[r], atom_getfloat(x->atombuffer+2+c+col*r));
    outlet_list(x->x_obj.ob_outlet, gensym("col"), row, ap);
    freebytes(ap, row*sizeof(t_atom));
    break;
  default:
    c=atom_getfloat(argv++)-1;
    if (argc--<row){
      post("matrix: sparse cols not yet supported : use \"mtx_check\"");
      return;
    }
    if ((c<0)||(c>=col)){
      post("matrix: col index %d is out of range", c+1);
      return;
    }
    argv+=argc-1;
    if (argc>row)argc=row;
    while(argc--){
      ap=x->atombuffer+2+c+col*argc;
      SETFLOAT(ap, atom_getfloat(argv--));
    }
  }
}

void matrix_element(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  t_atom *ap=x->atombuffer+2;
  int row=x->row, col=x->col;
  int r, c, i=row*col;

  switch (argc){
  case 0:
    while(i--)outlet_float(x->x_obj.ob_outlet, atom_getfloat(ap++));
    break;
  case 1:
    r=c=atom_getfloat(argv)-1;
    if ((r<0)||(r>=row)){
      post("matrix: row index %d is out of range", r+1);
      return;
    }
    if ((c<0)||(c>=col)){
      post("matrix: col index %d is out of range", c+1);
      return;
    }
    outlet_float(x->x_obj.ob_outlet, atom_getfloat(x->atombuffer+2+c+r*col));
    break;
  case 2:
    r=atom_getfloat(argv++)-1;
    c=atom_getfloat(argv++)-1;
    if ((r<0)||(r>=row)){      post("matrix: row index %d is out of range", r+1);      return;    }
    if ((c<0)||(c>=col)){      post("matrix: col index %d is out of range", c+1);      return;    }
    outlet_float(x->x_obj.ob_outlet, atom_getfloat(x->atombuffer+2+c+r*col));
    break;
  default:
    r=atom_getfloat(argv++)-1;
    c=atom_getfloat(argv++)-1;
    if ((r<0)||(r>=row)){      post("matrix: row index %d is out of range", r+1);      return;    }
    if ((c<0)||(c>=col)){      post("matrix: col index %d is out of range", c+1);      return;    }
    SETFLOAT(x->atombuffer+2+c+r*col, atom_getfloat(argv));
  }
}


/* destructor */

void matrix_free(t_matrix *x)
{
  freebytes(x->atombuffer, (x->col*x->row+2)*sizeof(t_atom));
  x->atombuffer=0;
  x->col=x->row=0;
}
