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

/* mtx_inverse */
static t_class *mtx_inverse_class;

static void mtx_inverse_matrix(t_matrix *x, t_symbol *s, int argc, t_atom *argv)
{
  /* maybe we should do this in double or long double ? */
  int row=atom_getfloat(argv);
  int col=atom_getfloat(argv+1);
  int i, k, row2=row*row;

  t_matrixfloat *original, *inverted;
  t_matrixfloat *a1, *a2, *b1, *b2;  // dummy pointers

  int ok = 0;

  if(row*col+2>argc){
    post("mtx_print : sparse matrices not yet supported : use \"mtx_check\"");
    return;
  }
  if (row!=col){
    post("mtx_inverse: only square matrices can be inverted");
    return;
  }

  // reserve memory for outputting afterwards
  adjustsize(x, row, row);
  // 1. get the 2 matrices : orig; invert (create as eye, but will be orig^(-1))
  inverted = (t_matrixfloat *)getbytes(sizeof(t_matrixfloat)*row2);
  // 1a extract values of A to float-buf
  original=matrix2float(argv);
  // 1b make an eye-shaped float-buf for B
  i=row2;
  b1=inverted;
  while(i--)*b1++=0;
  i=row;
  b1=inverted;
  while(i--)b1[i*(row+1)]=1;

  // 2. do the Gauss-Jordan
  for (k=0;k<row;k++) {
    // 2. adjust current row
    t_matrixfloat diagel = original[k*(col+1)];
    t_matrixfloat i_diagel = diagel?1./diagel:0;
    if (!diagel)ok++;


    /* normalize current row (set the diagonal-element to 1 */
    a2=original+k*col;
    b2=inverted+k*col;
    i=row;
    while(i--){
      *a2++*=i_diagel;
      *b2++*=i_diagel;
    }
    /* eliminate the k-th element in each row by adding the weighted normalized row */

    a2=original+k*row;
    b2=inverted+k*row;
    for(i=0;i<row;i++)
      if (i-k) {
	t_matrixfloat f=-*(original+i*row+k);
	int j = row;
	a1=original+i*row;
	b1=inverted+i*row;
	while (j--) {
	  *(a1+j)+=f**(a2+j);
	  *(b1+j)+=f**(b2+j);
	}
      }
  }
  // 3. output the matrix
  // 3a convert the floatbuf to an atombuf;
  float2matrix(x->atombuffer, inverted);
  // 3b destroy the buffers
  freebytes(original, sizeof(t_matrixfloat)*row2);

  if (ok)post("mtx_inverse: couldn't really invert the matrix !!! %d error%c", ok, (ok-1)?'s':0);

  // 3c output the atombuf;
  matrix_bang(x);
}

static void *mtx_inverse_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix *x = (t_matrix *)pd_new(mtx_inverse_class);
  outlet_new(&x->x_obj, 0);
  x->col=x->row=0;
  x->atombuffer=0;

  return (x);
}
void mtx_inverse_setup(void)
{
  mtx_inverse_class = class_new(gensym("mtx_inverse"), (t_newmethod)mtx_inverse_new, 
				(t_method)matrix_free, sizeof(t_matrix), 0, A_GIMME, 0);
  class_addbang  (mtx_inverse_class, matrix_bang);
  class_addmethod(mtx_inverse_class, (t_method)mtx_inverse_matrix, gensym("matrix"), A_GIMME, 0);
  class_sethelpsymbol(mtx_inverse_class, gensym("iemmatrix/mtx_inverse"));
}

void iemtx_inverse_setup(void){
  mtx_inverse_setup();
}
