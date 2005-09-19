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

static t_class *mtx_minmax_class;

typedef struct _MTXminmax_ MTXminmax;
struct _MTXminmax_
{
   t_object x_obj;
   int size;
   int outsize;
   int minmax_dimension;
   int minmax_direction;
  int operator_minimum; // 1 if we are [mtx_min], 0 if we are [mtx_max]

   t_outlet *list_outlet;

   t_atom *list_out;
   t_atom *list_in;
};

static void deleteMTXMinMax (MTXminmax *mtx_minmax_obj) 
{
   if (mtx_minmax_obj->list_out)
      freebytes (mtx_minmax_obj->list_out, sizeof(t_atom)*(mtx_minmax_obj->size+2));
}

static void mTXSetMinMaxDirection (MTXminmax *mtx_minmax_obj, t_float c_dir)
{
   int direction = (int) c_dir;
   if ((direction != -1) && (direction != 1))
      direction = 1;
   mtx_minmax_obj->minmax_direction = direction;
}

static void mTXSetMinMaxDimension (MTXminmax *mtx_minmax_obj, t_float c_dim)
{
   int dimension = (int) c_dim;
   dimension = (dimension > 0)?dimension:0;
   dimension = (dimension < 3)?dimension:3;
   mtx_minmax_obj->minmax_dimension = dimension;
}

static void *newMTXMin (t_symbol *s, int argc, t_atom *argv)
{
   MTXminmax *mtx_minmax_obj = (MTXminmax *) pd_new (mtx_minmax_class);
   int c_dim = 1;
   int c_dir = 1;

   switch ((argc>2)?2:argc) {
      case 2:
	 c_dir = atom_getint(argv+1);
      case 1:
	 c_dim = atom_getint(argv);
   }
   mtx_minmax_obj->operator_minimum = 1;
   mTXSetMinMaxDimension (mtx_minmax_obj, (t_float) c_dim);
   mTXSetMinMaxDirection (mtx_minmax_obj, (t_float) c_dir);

   mtx_minmax_obj->list_outlet = outlet_new (&mtx_minmax_obj->x_obj, gensym("matrix"));
   return ((void *) mtx_minmax_obj);
} 
static void *newMTXMax (t_symbol *s, int argc, t_atom *argv)
{
   MTXminmax *mtx_minmax_obj = (MTXminmax *) pd_new (mtx_minmax_class);
   int c_dim = 1;
   int c_dir = 1;

   switch ((argc>2)?2:argc) {
      case 2:
	 c_dir = atom_getint(argv+1);
      case 1:
	 c_dim = atom_getint(argv);
   }
   mtx_minmax_obj->operator_minimum = 0;
   mTXSetMinMaxDimension (mtx_minmax_obj, (t_float) c_dim);
   mTXSetMinMaxDirection (mtx_minmax_obj, (t_float) c_dir);

   mtx_minmax_obj->list_outlet = outlet_new (&mtx_minmax_obj->x_obj, gensym("matrix"));
   return ((void *) mtx_minmax_obj);
} 

static void mTXMinMaxBang (MTXminmax *mtx_minmax_obj)
{
   if (mtx_minmax_obj->list_out) 
      outlet_anything(mtx_minmax_obj->list_outlet, gensym("matrix"), 
	    mtx_minmax_obj->outsize+2, mtx_minmax_obj->list_out);
}
/*
static void copyList (int size, t_atom *x, t_atom *y)
{
   while(size--)
 *y++=*x++;
 }
 */

static t_float minList (int n, t_atom *x)
{
   t_float min=atom_getfloat(x);
   t_float f;
   for (;n--;x++) {
      f = atom_getfloat(x);
      min = (min < f)?min:f;
   }
   return min;
}

static t_float minListStep (int n, const int step, t_atom *x)
{
   t_float min=atom_getfloat(x);
   t_float f;
   for (;n--;x+=step) {
      f = atom_getfloat(x);
      min = (min < f)?min:f;
   }
   return min;
}

static t_float maxList (int n, t_atom *x)
{
   t_float max=atom_getfloat(x);
   t_float f;
   for (;n--;x++) {
      f = atom_getfloat(x);
      max = (max > f)?max:f;
   }
   return max;
}
static t_float maxListStep (int n, const int step, t_atom *x)
{
   t_float max=atom_getfloat(x);
   t_float f;
   for (;n--;x+=step) {
      f = atom_getfloat(x);
      max = (max > f)?max:f;
   }
   return max;
}

static void minListColumns (const int rows, const int columns, t_atom *x, t_atom *y)
{
   int count;
   t_float f;
   for (count=0; count < columns; count++, x++, y++) {
      f=minListStep (rows, columns, x);
      SETFLOAT(y,f);
   }
}
static void minListRows (int rows, int columns, t_atom *x, t_atom *y)
{
   int count;
   t_float f;
   for (count=0; count < rows; count++, x+=columns, y++) {
      f=minList (columns, x);
      SETFLOAT(y,f);
   }
}
static void maxListColumns (const int rows, const int columns, t_atom *x, t_atom *y)
{
   int count;
   t_float f;
   for (count=0; count < columns; count++, x++, y++) {
      f=maxListStep (rows, columns, x);
      SETFLOAT(y,f);
   }
}
static void maxListRows (int rows, int columns, t_atom *x, t_atom *y)
{
   int count;
   t_float f;
   for (count=0; count < rows; count++, x+=columns, y++) {
      f=maxList (columns, x);
      SETFLOAT(y,f);
   }
}


static void mTXMinMaxMatrix (MTXminmax *mtx_minmax_obj, t_symbol *s, 
      int argc, t_atom *argv)
{
   int rows = atom_getint (argv++);
   int columns = atom_getint (argv++);
   int size = rows * columns;
   int list_size = argc - 2;
   t_atom *list_in = argv;
   t_atom *list_out = mtx_minmax_obj->list_out;
   int count;
   int rows_out;
   int columns_out;

   // size check
   if (!size) {
      post("mtx_minmax: invalid dimensions");
      return;
   }
   else if (list_size<size) {
      post("mtx_minmax: sparse matrix not yet supported: use \"mtx_check\"");
      return;
   }
   
   if (size != mtx_minmax_obj->size) {
      if (!list_out)
	 list_out = (t_atom *) getbytes (sizeof (t_atom) * (size + 2));
      else
	 list_out = (t_atom *) resizebytes (list_out,
	       sizeof (t_atom) * (mtx_minmax_obj->size+2),
	       sizeof (t_atom) * (size + 2));
   }

   mtx_minmax_obj->size = size;
   mtx_minmax_obj->list_out = list_out;

   // main part
   list_out += 2;
   //copyList (size, argv, list_out);
   switch (mtx_minmax_obj->minmax_dimension) {
      case 0:
	 columns_out = 1;
	 rows_out = 1;
	 if (mtx_minmax_obj->operator_minimum)
	    minListRows (1, size, list_in, list_out); 
	 else
	    maxListRows (1, size, list_in, list_out);
	 break;
      case 1:
	 rows_out = rows;
	 columns_out = 1;
	 if (mtx_minmax_obj->operator_minimum)
	    minListRows (rows, columns, list_in, list_out); 
	 else
	    maxListRows (rows, columns, list_in, list_out);
	 break;
      case 2:
	 rows_out = 1;
	 columns_out = columns;
	 if (mtx_minmax_obj->operator_minimum)
	    minListColumns (rows, columns, list_in, list_out); 
	 else
	    maxListColumns (rows, columns, list_in, list_out);
	 break;
   }
   mtx_minmax_obj->outsize = columns_out * rows_out;
   list_out = mtx_minmax_obj->list_out;

   SETSYMBOL(list_out, gensym("matrix"));
   SETFLOAT(list_out, rows_out);
   SETFLOAT(&list_out[1], columns_out);
   outlet_anything(mtx_minmax_obj->list_outlet, gensym("matrix"), 
	 mtx_minmax_obj->outsize+2, list_out);
}

void mtx_minmax_setup (void)
{
   mtx_minmax_class = class_new 
      (gensym("mtx_min"),
       (t_newmethod) newMTXMin,
       (t_method) deleteMTXMinMax,
       sizeof (MTXminmax),
       CLASS_DEFAULT, A_GIMME, 0);
   class_addbang (mtx_minmax_class, (t_method) mTXMinMaxBang);
   class_addmethod (mtx_minmax_class, (t_method) mTXMinMaxMatrix, gensym("matrix"), A_GIMME,0);
   class_addmethod (mtx_minmax_class, (t_method) mTXSetMinMaxDimension, gensym("dimension"), A_DEFFLOAT,0);
   class_addmethod (mtx_minmax_class, (t_method) mTXSetMinMaxDirection, gensym("direction"), A_DEFFLOAT,0);
   class_addcreator ((t_newmethod) newMTXMax, gensym("mtx_max"), A_GIMME,0);
   class_sethelpsymbol (mtx_minmax_class, gensym("iemmatrix/mtx_minmax"));
}

void iemtx_minmax_setup(void){
  mtx_minmax_setup();
}
