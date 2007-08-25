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

static t_class *mtx_rowrfft_class;

typedef struct _MTXRowrfft_ MTXRowrfft;
struct _MTXRowrfft_
{
  t_object x_obj;
  int size;
  int size2;

  t_float *f_re;
  t_float *f_im;

  t_outlet *list_re_out;
  t_outlet *list_im_out;
   
  t_atom *list_re;
  t_atom *list_im;
};

static void deleteMTXRowrfft (MTXRowrfft *x) 
{
  if (x->f_re)
     free (x->f_re);
  if (x->f_im) 
     free (x->f_im);
  if (x->list_re)
     free (x->list_re);
  if (x->list_im)
     free (x->list_im);
}

static void *newMTXRowrfft (t_symbol *s, int argc, t_atom *argv)
{
  MTXRowrfft *x = (MTXRowrfft *) pd_new (mtx_rowrfft_class);
  x->list_re_out = outlet_new (&x->x_obj, gensym("matrix"));
  x->list_im_out = outlet_new (&x->x_obj, gensym("matrix"));

  x->size=x->size2=0;
  x->f_re=x->f_im=0;
  x->list_re=x->list_im=0;
  
  return ((void *) x);
} 

static void mTXrowrfftBang (MTXRowrfft *x)
{
  if (x->list_im) {
    outlet_anything(x->list_im_out, gensym("matrix"), x->size2, x->list_im);
    outlet_anything(x->list_re_out, gensym("matrix"), x->size2, x->list_re);
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

static void mTXrowrfftMatrix (MTXRowrfft *x, t_symbol *s, 
			      int argc, t_atom *argv)
{
  //mTXrowrfftList (x, s, argc-2, argv+2);
  int rows = atom_getint (argv++);
  int columns = atom_getint (argv++);
  int columns_re = (columns>>1)+1; /* N/2+1 samples needed for real part of realfft */
  int size = rows * columns;
  int in_size = argc-2;
  int size2 = columns_re * rows + 2; /* +2 since the list also contains matrix row+col */
  int fft_count;
  t_atom *list_re = x->list_re;
  t_atom *list_im = x->list_im;
  t_float *f_re = x->f_re;
  t_float *f_im = x->f_im;

  // fftsize check
  if (!size)
    post("mtx_rowrfft: invalid dimensions");
  else if (in_size<size)
    post("mtx_rowrfft: sparse matrix not yet supported: use \"mtx_check\"");
  else if (columns < 4){
    post("mtx_rowrfft: matrix must have at least 4 columns");
  }
  else if (columns == (1 << ilog2(columns))) {
    /* ok, do the FFT! */

    // memory things
    f_re=(t_float*)realloc(f_re, sizeof(t_float)*size);
    f_im=(t_float*)realloc(f_im, sizeof(t_float)*size);
    list_re=(t_atom*)realloc(list_re, sizeof(t_atom)*size2);
    list_im=(t_atom*)realloc(list_im, sizeof(t_atom)*size2);

    x->size = size;
    x->size2 = size2;
    x->list_im = list_im;
    x->list_re = list_re;
    x->f_re = f_re;
    x->f_im = f_im;

    // main part
    readFloatFromList (size, argv, f_re);

    fft_count = rows;
    list_re += 2;
    list_im += 2;
    while (fft_count--){ 
      mayer_realfft (columns, f_re);
      fftRestoreImag (columns, f_re, f_im);
      writeFloatIntoList (columns_re, list_re, f_re);
      writeFloatIntoList (columns_re, list_im, f_im);
      f_im += columns;
      f_re += columns;
      list_re += columns_re;
      list_im += columns_re;
    }

    list_re = x->list_re;
    list_im = x->list_im;
      
    SETSYMBOL(list_re, gensym("matrix"));
    SETSYMBOL(list_im, gensym("matrix"));
    SETFLOAT(list_re, rows);
    SETFLOAT(list_im, rows);
    SETFLOAT(list_re+1, columns_re);
    SETFLOAT(list_im+1, columns_re);
    outlet_anything(x->list_im_out, gensym("matrix"), 
		    x->size2, list_im);
    outlet_anything(x->list_re_out, gensym("matrix"), 
		    x->size2, list_re);
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
}

void iemtx_rowrfft_setup(void){
  mtx_rowrfft_setup();
}
