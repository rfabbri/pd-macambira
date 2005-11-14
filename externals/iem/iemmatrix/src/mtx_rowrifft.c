/*
 *  iemmatrix
 *
 *  objects for manipulating simple matrices
 *  mostly refering to matlab/octave matrix functions
 *
 * Copyright (c) 2005, Franz Zotter
 * IEM, Graz, Austria
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */

#include "iemmatrix.h"
#include <stdlib.h>

static t_class *mtx_rowrifft_class;

typedef struct _MTXRowrifft_
{
  t_object x_obj;
  int rows;
  int columns;
  int columns_re;
  int size;
  int size2;
  t_float renorm_fac;

  t_float *f_re;
  t_float *f_im;

  t_outlet *list_re_out;
  t_outlet *list_im_out;
   
  t_atom *list_re;
  t_atom *list_im;
} MTXRowrifft;


// helper functions: these should really go into a separate file!


static void zeroFloatArray (int n, t_float *f)
{
  while (n--)
    *f++ = 0.0f;
}

static void writeFloatIntoList (int n, t_atom *l, t_float *f) 
{
  for (;n--;f++, l++) 
    SETFLOAT (l, *f);
}
static void readFloatFromList (int n, t_atom *l, t_float *f) 
{
  while (n--) 
    *f++ = atom_getfloat (l++);
}

//--------------inverse real fft

static void multiplyVector (int n, t_float *f, t_float fac)
{
  while (n--)
    *f++ *= fac;
}


static void ifftPrepareReal (int n, t_float *re, t_float *im) 
{
  n >>= 1;
  re += n;
  im += n;
   
  while (--n) 
    *++re = -*--im;
}


static void *newMTXRowrifft (t_symbol *s, int argc, t_atom *argv)
{
  MTXRowrifft *x = (MTXRowrifft *) pd_new (mtx_rowrifft_class);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"),gensym(""));
  x->list_re_out = outlet_new (&x->x_obj, gensym("matrix"));
  return ((void *) x);
} 


static void mTXrowrifftMatrixCold (MTXRowrifft *x, t_symbol *s, 
				   int argc, t_atom *argv)
{
  //mTXrowrifftList (x, s, argc-2, argv+2);
  int rows = atom_getint (argv++);
  int columns_re = atom_getint (argv++);
  int in_size = argc-2;
  int columns = (columns_re-1)<<1;
  int size2 = columns_re * rows;
  int size = rows * columns;
  int ifft_count;
  t_atom *list_re = x->list_re;
  t_float *f_re = x->f_re;
  t_float *f_im = x->f_im;

  // ifftsize check
  if (columns_re < 3)
    post("mtx_rowrifft: matrix must have at least 3 columns");
  else if (!size) 
    post("mtx_rowrifft: invalid dimensions");
  else if (in_size < size2)
    post("mtx_rowrifft: sparse matrix not yet supported: use \"mtx_check\"");
  else if (columns<4)
    post("mtx_rowrifft: too small matrices");
  else if (columns == (1 << ilog2(columns))) {

    // memory things
    f_re=(t_float*)realloc(f_re, sizeof(t_float)*size);
    f_im=(t_float*)realloc(f_im, sizeof(t_float)*size);
    list_re=(t_atom*)realloc(list_re, sizeof(t_atom)*(size+2));

    x->size = size;
    x->size2 = size2;
    x->rows = rows;
    x->columns = columns;
    x->columns_re = columns_re;
    x->list_re = list_re;
    x->f_re = f_re;
    x->f_im = f_im;
      
    // main part: reading imaginary part
    ifft_count = rows;
    x->renorm_fac = 1.0f / columns;
    while (ifft_count--) {
      readFloatFromList (columns_re, argv, f_im);
      argv += columns_re;
      f_im += columns;
    }
    // do nothing else!
  }
  else
    post("mtx_rowrifft: rowvector 2*(size+1) no power of 2!");
}

static void mTXrowrifftMatrixHot (MTXRowrifft *x, t_symbol *s, 
				  int argc, t_atom *argv)
{
  //mTXrowrifftList (x, s, argc-2, argv+2);
  int rows = atom_getint (argv++);
  int columns_re = atom_getint (argv++);
  int columns = x->columns;
  int size = x->size;
  int in_size = argc-2;
  int size2 = x->size2;
  int ifft_count;
  t_atom *ptr_re = x->list_re;
  t_float *f_re = x->f_re;
  t_float *f_im = x->f_im;
  t_float renorm_fac = x->renorm_fac;

  // ifftsize check
  if ((rows != x->rows) || 
      (columns_re != x->columns_re))
    post("mtx_rowrifft: matrix dimensions do not match");
  else if (in_size<size2)
    post("mtx_rowrifft: sparse matrix not yet supported: use \"mtx_check\"");
  else if (!x->size2)
    post("mtx_rowrifft: invalid right side matrix");
  else { // main part
    ifft_count = rows;
    ptr_re += 2;
    while (ifft_count--){ 
      readFloatFromList (columns_re, argv, f_re);
      ifftPrepareReal (columns, f_re, f_im);
      mayer_realifft (columns, f_re);
      multiplyVector (columns, f_re, renorm_fac);
      f_im += columns;
      f_re += columns;
      ptr_re += columns;
      argv += columns_re;
    }
    ptr_re = x->list_re;
    f_re = x->f_re;
    size2 = x->size2;

    SETSYMBOL(ptr_re, gensym("matrix"));
    SETFLOAT(ptr_re, rows);
    SETFLOAT(&ptr_re[1], x->columns);
    writeFloatIntoList (size, ptr_re+2, f_re);
    outlet_anything(x->list_re_out, gensym("matrix"), size+2, ptr_re);
  }
}

static void mTXrowrifftBang (MTXRowrifft *x)
{
  if (x->list_re)
    outlet_anything(x->list_re_out, gensym("matrix"), 
		    x->size+2, x->list_re);
}


static void deleteMTXRowrifft (MTXRowrifft *x) 
{
  free(x->f_re);
  free(x->f_im);
  free(x->list_re);
  free(x->list_im);
}

static void mtx_rowrifft_setup (void)
{
  mtx_rowrifft_class = class_new 
    (gensym("mtx_rowrifft"),
     (t_newmethod) newMTXRowrifft,
     (t_method) deleteMTXRowrifft,
     sizeof (MTXRowrifft),
     CLASS_DEFAULT, A_GIMME, 0);
  class_addbang (mtx_rowrifft_class, (t_method) mTXrowrifftBang);
  class_addmethod (mtx_rowrifft_class, (t_method) mTXrowrifftMatrixHot, gensym("matrix"), A_GIMME,0);
  class_addmethod (mtx_rowrifft_class, (t_method) mTXrowrifftMatrixCold, gensym(""), A_GIMME,0);
  class_sethelpsymbol (mtx_rowrifft_class, gensym("iemmatrix/mtx_rowrfft"));
}

void iemtx_rowrifft_setup(void){
  mtx_rowrifft_setup();
}
