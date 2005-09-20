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

static t_class *mtx_rowrfft_class;

typedef struct _MTXRowrfft_ MTXRowrfft;
struct _MTXRowrfft_
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
};

static void deleteMTXRowrfft (MTXRowrfft *mtx_rowrfft_obj) 
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

static void *newMTXRowrfft (t_symbol *s, int argc, t_atom *argv)
{
  MTXRowrfft *mtx_rowrfft_obj = (MTXRowrfft *) pd_new (mtx_rowrfft_class);
  mtx_rowrfft_obj->list_re_out = outlet_new (&mtx_rowrfft_obj->x_obj, gensym("matrix"));
  mtx_rowrfft_obj->list_im_out = outlet_new (&mtx_rowrfft_obj->x_obj, gensym("matrix"));
  return ((void *) mtx_rowrfft_obj);
} 

static void mTXrowrfftBang (MTXRowrfft *mtx_rowrfft_obj)
{
  if (mtx_rowrfft_obj->list_im) {
    outlet_anything(mtx_rowrfft_obj->list_im_out, gensym("matrix"), mtx_rowrfft_obj->size2+2, mtx_rowrfft_obj->list_im);
    outlet_anything(mtx_rowrfft_obj->list_re_out, gensym("matrix"), mtx_rowrfft_obj->size2+2, mtx_rowrfft_obj->list_re);
  }
}

static void fftRestoreImag (int n, t_float *re, t_float *im) 
{
  t_float *im2;
  n >>= 1;
  *im=0;
  re += n;
  im += n;
  im2 = im;
  *im=0;
  while (--n) {
    *--im = -*++re;
    *++im2 = 0;
    *re = 0;
  }
}

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

static void mTXrowrfftMatrix (MTXRowrfft *mtx_rowrfft_obj, t_symbol *s, 
			      int argc, t_atom *argv)
{
  //mTXrowrfftList (mtx_rowrfft_obj, s, argc-2, argv+2);
  int rows = atom_getint (argv++);
  int columns = atom_getint (argv++);
  int columns_re = (columns>>1)+1;
  int size = rows * columns;
  int in_size = argc-2;
  int size2 = columns_re * rows;
  int fft_count;
  t_atom *ptr_re = mtx_rowrfft_obj->list_re;
  t_atom *ptr_im = mtx_rowrfft_obj->list_im;
  t_float *f_re = mtx_rowrfft_obj->f_re;
  t_float *f_im = mtx_rowrfft_obj->f_im;

  // fftsize check
  if (!size)
    post("mtx_rowrfft: invalid dimensions");
  else if (in_size<size)
    post("mtx_rowrfft: sparse matrix not yet supported: use \"mtx_check\"");
  else if (columns == (1 << ilog2(columns))) {
    // memory things
    if (f_re) {
      if (size != mtx_rowrfft_obj->size) {
	f_re = (t_float *) resizebytes (f_re, 
					sizeof (t_float) * mtx_rowrfft_obj->size,
					sizeof (t_float) * size);
	f_im = (t_float *) resizebytes (f_im, 
					sizeof (t_float) * mtx_rowrfft_obj->size,
					sizeof (t_float) * size);
	ptr_re = (t_atom *) resizebytes (ptr_re,
					 sizeof (t_atom) * (mtx_rowrfft_obj->size2+2),
					 sizeof (t_atom) * (size2 + 2));
	ptr_im = (t_atom *) resizebytes (ptr_im,
					 sizeof (t_atom) * (mtx_rowrfft_obj->size2+2),
					 sizeof (t_atom) * (size2 + 2));
      }
    }
    else {
      f_re = (t_float *) getbytes (sizeof (t_float) * size);
      f_im = (t_float *) getbytes (sizeof (t_float) * size);
      ptr_re = (t_atom *) getbytes (sizeof (t_atom) * (size2+2));
      ptr_im = (t_atom *) getbytes (sizeof (t_atom) * (size2+2));
    }
    mtx_rowrfft_obj->size = size;
    mtx_rowrfft_obj->size2 = size2;
    mtx_rowrfft_obj->rows = rows;
    mtx_rowrfft_obj->columns = columns;
    mtx_rowrfft_obj->columns_re = columns_re;
    mtx_rowrfft_obj->list_im = ptr_im;
    mtx_rowrfft_obj->list_re = ptr_re;
    mtx_rowrfft_obj->f_re = f_re;
    mtx_rowrfft_obj->f_im = f_im;
      
    // main part
    readFloatFromList (size, argv, f_re);

    fft_count = rows;
    ptr_re += 2;
    ptr_im += 2;
    while (fft_count--){ 
      mayer_realfft (columns, f_re);
      fftRestoreImag (columns, f_re, f_im);
      writeFloatIntoList (columns_re, ptr_re, f_re);
      writeFloatIntoList (columns_re, ptr_im, f_im);
      f_im += columns;
      f_re += columns;
      ptr_re += columns_re;
      ptr_im += columns_re;
    }
    ptr_re = mtx_rowrfft_obj->list_re;
    ptr_im = mtx_rowrfft_obj->list_im;
      
    SETSYMBOL(ptr_re, gensym("matrix"));
    SETSYMBOL(ptr_im, gensym("matrix"));
    SETFLOAT(ptr_re, rows);
    SETFLOAT(ptr_im, rows);
    SETFLOAT(&ptr_re[1], columns_re);
    SETFLOAT(&ptr_im[1], columns_re);
    outlet_anything(mtx_rowrfft_obj->list_im_out, gensym("matrix"), 
		    mtx_rowrfft_obj->size2+2, ptr_im);
    outlet_anything(mtx_rowrfft_obj->list_re_out, gensym("matrix"), 
		    mtx_rowrfft_obj->size2+2, ptr_re);
  }
  else
    post("mtx_rowfft: rowvector size no power of 2!");
}

void mtx_rowrfft_setup (void)
{
  mtx_rowrfft_class = class_new 
    (gensym("mtx_rowrfft"),
     (t_newmethod) newMTXRowrfft,
     (t_method) deleteMTXRowrfft,
     sizeof (MTXRowrfft),
     CLASS_DEFAULT, A_GIMME, 0);
  class_addbang (mtx_rowrfft_class, (t_method) mTXrowrfftBang);
  class_addmethod (mtx_rowrfft_class, (t_method) mTXrowrfftMatrix, gensym("matrix"), A_GIMME,0);
  class_sethelpsymbol (mtx_rowrfft_class, gensym("iemmatrix/mtx_rowrfft"));
}

void iemtx_rowrfft_setup(void){
  mtx_rowrfft_setup();
}
