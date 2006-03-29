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

static t_class *mtx_conv_class;

typedef struct _MTXConv_ MTXConv;
struct _MTXConv_
{
   t_object x_obj;
   int size;
   int rows;
   int columns;
   int rows_k;
   int columns_k;
   int size_k;
   int rows_y;
   int columns_y;
   int size_y;
   t_float renorm_fac;

   t_float **x;
   t_float **k;
   t_float **y;

   t_outlet *list_outlet;
   
   t_atom *list;
};

static t_float **getTFloatMatrix (int rows, int columns)
{
   t_float **matrix = (t_float **) getbytes 
      (sizeof (t_float *) * columns);
   t_float **mtx = matrix;

   while (columns--)
      if (!(*matrix++ = (t_float *) getbytes
	       (sizeof (t_float) * rows)))
	 post("an error has occured :)");
   return mtx;
}

static void deleteTFloatMatrix (t_float **matrix, int rows, int columns)
{
   int n = columns;
   t_float **matr = matrix;
   if (matr) {
      while (n--)
	 if (*matr)
	    freebytes (*matr++, sizeof(t_float) * rows);
      freebytes (matrix, sizeof(t_float*) * columns);
   }
}


static t_float **resizeTFloatMatrix (t_float **old, int rows_old, int columns_old,
      int rows_new, int columns_new)
{
   t_float **mtx = old;
   int count1;
   post("resizing from %dx%d to %dx%d", rows_old, columns_old, rows_new, columns_new);
   
   if ((rows_new == 0)||(columns_new == 0)) {
      deleteTFloatMatrix (old, rows_old, columns_old);
      old = 0;
      return old;
   }
   // 1. if rows_old>rows_new: old row disposal
   if (rows_old>rows_new)
      for (count1 = (rows_old - rows_new), mtx += rows_new;
	    count1--; mtx++) 
	 freebytes (*mtx, sizeof(t_float) * columns_old);
   // 2. resize row (double) pointer
   mtx = old = (t_float **) resizebytes(old, sizeof(t_float*) * rows_old, 
	 sizeof(t_float*) * rows_new);
   // 3. resizing rows if new size is smaller
   if (rows_old>rows_new)
      for (count1 = rows_new; count1--; mtx++) 
	 *mtx = (t_float *) resizebytes (*mtx, sizeof(t_float) * columns_old,
	       sizeof(t_float) * columns_new);
   else { // 4. resizing old rows if new size is bigger, allocating new rows
      for (count1 = rows_old; count1--; mtx++)
	 *mtx = (t_float *) resizebytes (*mtx, sizeof(t_float) * columns_old,
	       sizeof(t_float) * columns_new);
      for (count1 = (rows_new - rows_old); count1--; mtx++)
	 *mtx = (t_float *) getbytes (sizeof(t_float) * columns_new);
   }
   /* post("return resize"); */
   return old;
}

static void deleteMTXConv (MTXConv *mtx_conv_obj) 
{
   deleteTFloatMatrix (mtx_conv_obj->k, mtx_conv_obj->rows_k, mtx_conv_obj->columns_k);
   deleteTFloatMatrix (mtx_conv_obj->x, mtx_conv_obj->rows, mtx_conv_obj->columns);
   deleteTFloatMatrix (mtx_conv_obj->y, mtx_conv_obj->rows_y, mtx_conv_obj->columns_y);
   if (mtx_conv_obj->list)
      freebytes (mtx_conv_obj->list, sizeof(t_float) * (mtx_conv_obj->size_y + 2));
         
   mtx_conv_obj->k = 0;
   mtx_conv_obj->x = 0;
   mtx_conv_obj->y = 0;
   mtx_conv_obj->list = 0;
}

static void *newMTXConv (t_symbol *s, int argc, t_atom *argv)
{
   MTXConv *mtx_conv_obj = (MTXConv *) pd_new (mtx_conv_class);
   mtx_conv_obj->list_outlet = outlet_new (&mtx_conv_obj->x_obj, gensym("matrix"));
   inlet_new(&mtx_conv_obj->x_obj, &mtx_conv_obj->x_obj.ob_pd, gensym("matrix"),gensym(""));
   mtx_conv_obj->size = 0;
   mtx_conv_obj->rows = 0;
   mtx_conv_obj->columns = 0;
   mtx_conv_obj->size_y = 0;
   mtx_conv_obj->rows_y = 0;
   mtx_conv_obj->columns_y = 0;
   mtx_conv_obj->size_k = 0;
   mtx_conv_obj->rows_k = 0;
   mtx_conv_obj->columns_k = 0;
   return ((void *) mtx_conv_obj);
} 

static void mTXConvBang (MTXConv *mtx_conv_obj)
{
   if (mtx_conv_obj->list) 
      outlet_anything(mtx_conv_obj->list_outlet, gensym("matrix"), mtx_conv_obj->size+2, mtx_conv_obj->list);
}

static void zeroFloatArray (int n, t_float *f)
{
   while (n--)
      *f++ = 0.0f;
}
static void zeroTFloatMatrix (t_float **mtx, int rows, int columns)
{
   while (rows--)
      zeroFloatArray (columns, *mtx++);
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
static void readMatrixFromList (int rows, int columns, t_atom *l, t_float **mtx) 
{
   for (;rows--; l+=columns)
      readFloatFromList (columns, l, *mtx++);
}
static void writeMatrixIntoList (int rows, int columns, t_atom *l, t_float **mtx)
{
   for (;rows--; l+=columns)
      writeFloatIntoList (columns, l, *mtx++);
}

static void mTXConvKernelMatrix (MTXConv *mtx_conv_obj, t_symbol *s, int argc,
      t_atom *argv)
{
   int rows_k = atom_getint (argv++);
   int columns_k = atom_getint (argv++);
   int in_size = argc-2;
   int size_k = rows_k * columns_k;
   t_float **k = mtx_conv_obj->k;

   if (!size_k) 
      post ("mtx_conv: invalid matrix dimensions!");
   else if (in_size < size_k)
      post("mtx_conv: sparse matrix not yet supported: use \"mtx_check\"");
   else if ((rows_k != mtx_conv_obj->rows_k) || (columns_k != mtx_conv_obj->columns_k)) {
      if (k)
	 k = resizeTFloatMatrix (k, mtx_conv_obj->rows_k, mtx_conv_obj->columns_k,
	       rows_k, columns_k);
      else
	 k = getTFloatMatrix (rows_k, columns_k);

      mtx_conv_obj->rows_k = rows_k;
      mtx_conv_obj->columns_k = columns_k;

      readMatrixFromList (rows_k, columns_k, argv, k);

      mtx_conv_obj->k = k;
      mtx_conv_obj->size_k = size_k;
   }
   else
      readMatrixFromList (rows_k, columns_k, argv, k);
}
static void convolveRow (int columns, int columns_c, t_float *x, t_float *c, t_float *y)
{
   int n,k,count;
   for (k = 0; k < columns_c; k++) 
      for (n = k, count = columns; count--; n++) 
	 y[n] += x[n-k] * c[k];
}

static void convolveMtx (int rows, int columns, int rows_c, int columns_c, 
      t_float **x, t_float **c, t_float **y)
{
   int n,k,count;
   zeroTFloatMatrix (y, rows+rows_c-1, columns+columns_c-1);
   for (k = 0; k < rows_c; k++)
      for (n = k, count = rows; count--; n++) 
	 convolveRow (columns, columns_c, x[n-k], c[k], y[n]);
}


static void mTXConvMatrix (MTXConv *mtx_conv_obj, t_symbol *s, 
      int argc, t_atom *argv)
{
   int rows = atom_getint (argv++);
   int columns = atom_getint (argv++);
   int size = rows * columns;
   int rows_k = mtx_conv_obj->rows_k;
   int columns_k = mtx_conv_obj->columns_k;
   int size_k = mtx_conv_obj->size_k;
   int in_size = argc-2;
   int rows_y;
   int columns_y;
   int size_y = mtx_conv_obj->size_y;
   t_atom *list_ptr = mtx_conv_obj->list;
   t_float **x = mtx_conv_obj->x;
   t_float **y = mtx_conv_obj->y;
   t_float **k = mtx_conv_obj->k;

   // fftsize check
   if (!size){
      post("mtx_conv: invalid dimensions");
      return;
   }  else if (in_size<size) {
      post("mtx_conv: sparse matrix not yet supported: use \"mtx_check\"");
      return;
   }  else if (!size_k) {
      post("mtx_conv: no valid filter kernel defined");
      return;
   }

   //   post("1");

   if ((mtx_conv_obj->rows != rows)||(mtx_conv_obj->columns != columns)) { 
     if (x)
       x = resizeTFloatMatrix (x, mtx_conv_obj->rows, mtx_conv_obj->columns,
                               rows, columns);
     else
       x = getTFloatMatrix (rows, columns);
     mtx_conv_obj->x = x;
     mtx_conv_obj->size = size;
     mtx_conv_obj->rows = rows;
     mtx_conv_obj->columns = columns;
   }
   //post("2");
   rows_y = rows+rows_k-1;
   columns_y = columns+columns_k-1;
   if ((mtx_conv_obj->rows_y != rows_y)||(mtx_conv_obj->columns_y != columns_y)) { 
     size_y = rows_y * columns_y;      
     if (y)
       y = resizeTFloatMatrix (y, mtx_conv_obj->rows_y, mtx_conv_obj->columns_y,
                               rows_y, columns_y);
     else
       y = getTFloatMatrix (rows_y, columns_y);
     if (list_ptr)
       list_ptr = (t_atom *) resizebytes (list_ptr, sizeof(t_atom) * (mtx_conv_obj->size_y+2),
                                          sizeof (t_atom) * (size_y+2));
     else
       list_ptr = (t_atom *) getbytes (sizeof (t_atom) * (size_y+2));
     mtx_conv_obj->size_y = size_y;
     mtx_conv_obj->rows_y = rows_y;
     mtx_conv_obj->columns_y = columns_y;
     mtx_conv_obj->y = y;
     mtx_conv_obj->list = list_ptr;
   }
   //post("3");
   // main part
   readMatrixFromList (rows, columns, argv, x); 
   //post("4");
   convolveMtx (rows, columns, rows_k, columns_k, x, k, y);
   //post("5");
   writeMatrixIntoList (rows_y, columns_y, list_ptr+2, y);
   //post("6");
   SETSYMBOL(list_ptr, gensym("matrix"));
   SETFLOAT(list_ptr, rows_y);
   SETFLOAT(&list_ptr[1], columns_y);
   outlet_anything(mtx_conv_obj->list_outlet, gensym("matrix"), 
                   size_y+2, list_ptr);
   //post("7");
}

void mtx_conv_setup (void)
{
   mtx_conv_class = class_new 
      (gensym("mtx_conv"),
       (t_newmethod) newMTXConv,
       (t_method) deleteMTXConv,
       sizeof (MTXConv),
       CLASS_DEFAULT, A_GIMME, 0);
   class_addbang (mtx_conv_class, (t_method) mTXConvBang);
   class_addmethod (mtx_conv_class, (t_method) mTXConvMatrix, gensym("matrix"), A_GIMME,0);
   class_addmethod (mtx_conv_class, (t_method) mTXConvKernelMatrix, gensym(""), A_GIMME,0);

}

void iemtx_conv_setup(void){
  mtx_conv_setup();
}
