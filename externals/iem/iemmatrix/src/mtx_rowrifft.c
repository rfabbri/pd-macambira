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
  MTXRowrifft *mtx_rowrifft_obj = (MTXRowrifft *) pd_new (mtx_rowrifft_class);
  inlet_new(&mtx_rowrifft_obj->x_obj, &mtx_rowrifft_obj->x_obj.ob_pd, gensym("matrix"),gensym(""));
  mtx_rowrifft_obj->list_re_out = outlet_new (&mtx_rowrifft_obj->x_obj, gensym("matrix"));
  return ((void *) mtx_rowrifft_obj);
} 


static void mTXrowrifftMatrixCold (MTXRowrifft *mtx_rowrifft_obj, t_symbol *s, 
				   int argc, t_atom *argv)
{
  //mTXrowrifftList (mtx_rowrifft_obj, s, argc-2, argv+2);
  int rows = atom_getint (argv++);
  int columns_re = atom_getint (argv++);
  int in_size = argc-2;
  int columns = (columns_re-1)<<1;
  int size2 = columns_re * rows;
  int size = rows * columns;
  int ifft_count;
  t_atom *ptr_re = mtx_rowrifft_obj->list_re;
  t_float *f_re = mtx_rowrifft_obj->f_re;
  t_float *f_im = mtx_rowrifft_obj->f_im;

  // ifftsize check
  if (!size)
    post("mtx_rowrifft: invalid dimensions");
  else if (in_size < size2)
    post("mtx_rowrifft: sparse matrix not yet supported: use \"mtx_check\"");
  else if (columns == (1 << ilog2(columns))) {
    // memory things
    if (f_re) {
      if (size != mtx_rowrifft_obj->size) {
	f_re = (t_float *) resizebytes (f_re, 
					sizeof (t_float) * mtx_rowrifft_obj->size,
					sizeof (t_float) * size);
	f_im = (t_float *) resizebytes (f_im, 
					sizeof (t_float) * mtx_rowrifft_obj->size,
					sizeof (t_float) * size);
	ptr_re = (t_atom *) resizebytes (ptr_re,
					 sizeof (t_atom) * (mtx_rowrifft_obj->size + 2),
					 sizeof (t_atom) * (size + 2));
      }
    }
    else {
      f_re = (t_float *) getbytes (sizeof (t_float) * size);
      f_im = (t_float *) getbytes (sizeof (t_float) * size);
      ptr_re = (t_atom *) getbytes (sizeof (t_atom) * (size + 2));
    }
    mtx_rowrifft_obj->size = size;
    mtx_rowrifft_obj->size2 = size2;
    mtx_rowrifft_obj->rows = rows;
    mtx_rowrifft_obj->columns = columns;
    mtx_rowrifft_obj->columns_re = columns_re;
    mtx_rowrifft_obj->list_re = ptr_re;
    mtx_rowrifft_obj->f_re = f_re;
    mtx_rowrifft_obj->f_im = f_im;
      
    // main part: reading imaginary part
    ifft_count = rows;
    mtx_rowrifft_obj->renorm_fac = 1.0f / columns;
    while (ifft_count--) {
      readFloatFromList (columns_re, argv, f_im);
      argv += columns_re;
      f_im += columns;
    }
    // do nothing else!
  }
  else
    post("mtx_rowrifft: rowvector size no power of 2!");
}

static void mTXrowrifftMatrixHot (MTXRowrifft *mtx_rowrifft_obj, t_symbol *s, 
				  int argc, t_atom *argv)
{
  //mTXrowrifftList (mtx_rowrifft_obj, s, argc-2, argv+2);
  int rows = atom_getint (argv++);
  int columns_re = atom_getint (argv++);
  int columns = mtx_rowrifft_obj->columns;
  int size = mtx_rowrifft_obj->size;
  int in_size = argc-2;
  int size2 = mtx_rowrifft_obj->size2;
  int ifft_count;
  t_atom *ptr_re = mtx_rowrifft_obj->list_re;
  t_float *f_re = mtx_rowrifft_obj->f_re;
  t_float *f_im = mtx_rowrifft_obj->f_im;
  t_float renorm_fac;

  // ifftsize check
  if ((rows != mtx_rowrifft_obj->rows) || 
      (columns_re != mtx_rowrifft_obj->columns_re))
    post("mtx_rowrifft: matrix dimensions do not match");
  else if (in_size<size2)
    post("mtx_rowrifft: sparse matrix not yet supported: use \"mtx_check\"");
  else if (!mtx_rowrifft_obj->size2)
    post("mtx_rowrifft: invalid right side matrix");
  else { // main part
    ifft_count = rows;
    ptr_re += 2;
    renorm_fac = mtx_rowrifft_obj->renorm_fac;
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
    ptr_re = mtx_rowrifft_obj->list_re;
    f_re = mtx_rowrifft_obj->f_re;
    size2 = mtx_rowrifft_obj->size2;

    SETSYMBOL(ptr_re, gensym("matrix"));
    SETFLOAT(ptr_re, rows);
    SETFLOAT(&ptr_re[1], mtx_rowrifft_obj->columns);
    writeFloatIntoList (size, ptr_re+2, f_re);
    outlet_anything(mtx_rowrifft_obj->list_re_out, gensym("matrix"), size+2, ptr_re);
  }
}

static void mTXrowrifftBang (MTXRowrifft *mtx_rowrifft_obj)
{
  if (mtx_rowrifft_obj->list_re)
    outlet_anything(mtx_rowrifft_obj->list_re_out, gensym("matrix"), 
		    mtx_rowrifft_obj->size+2, mtx_rowrifft_obj->list_re);
}


static void deleteMTXRowrifft (MTXRowrifft *mtx_rowrfft_obj) 
{
  if (mtx_rowrfft_obj->f_re)
    freebytes (mtx_rowrfft_obj->f_re, sizeof(t_float)*mtx_rowrfft_obj->size);
  if (mtx_rowrfft_obj->f_im)
    freebytes (mtx_rowrfft_obj->f_im, sizeof(t_float)*mtx_rowrfft_obj->size);
  if (mtx_rowrfft_obj->list_re)
    freebytes (mtx_rowrfft_obj->list_re, sizeof(t_atom)*(mtx_rowrfft_obj->size2+2));
  if (mtx_rowrfft_obj->list_im)
    freebytes (mtx_rowrfft_obj->list_im, sizeof(t_atom)*(mtx_rowrfft_obj->size2+2));
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
