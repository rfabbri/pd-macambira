/* ************************************* */
/* iemmatrix                             */
/* ************************************* */
/*  objects for simple matrix operations */
/* ************************************* */

/*
 * IEMMATRIX is a runtime-library 
 * for miller s. puckette's realtime-computermusic-software "pure data"
 * therefore you NEED "pure data" to make any use of the IEMMATRIX external
 * (except if you want to use the code for other things)
 *
 * you can get "pure data" at
 *   http://pd.iem.at
 *   ftp://iem.at/pd
 */

/*
 * Copyright (c) Thomas Musil; IEM KUG, Graz, Austria; 2001-2005
 * Copyright (c) IOhannes m zmölnig (forum::für::umläute), IEM KUG, Graz, Austria; 2001-2005
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 *
 * "pure data" has it's own license, that comes shipped with "pure data".
 *
 * there are ABSOLUTELY NO WARRANTIES for anything
 */

#ifndef INCLUDE_IEMMATRIX_H__
#define INCLUDE_IEMMATRIX_H__


#ifdef __WIN32__
/* MinGW automaticaly defines __WIN32__
 * other windos compilers might have to define it by hand
 */

/* m_pd.h expexts MSW rather than __WIN32__ */
# ifndef MSW
#  define MSW
# endif

# pragma warning( disable : 4244 )
# pragma warning( disable : 4305 )

#endif /* __WIN32__ */


#include "m_pd.h"

#define VERSION "0.1"


#include <math.h>
#include <stdio.h>

#include <string.h>
#include <memory.h>

#ifdef __WIN32__
# define fabsf fabs
# define sqrtf sqrt
#endif

typedef double t_matrixfloat;


/* the main class...*/
typedef struct _matrix
{
  t_object x_obj;

  int      row;
  int      col;

  t_atom *atombuffer;

  int     current_row, current_col;  /* this makes things easy for the mtx_row & mtx_col...*/
  t_float f;

  t_canvas *x_canvas;
} t_matrix;

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


void matrix_free(t_matrix*x);

/* utility function */
void setdimen(t_matrix *x, int row, int col);
void adjustsize(t_matrix *x, int desiredRow, int desiredCol);
void debugmtx(int argc, t_float *buf, int id);
t_matrixfloat *matrix2float(t_atom *ap);
void float2matrix(t_atom *ap, t_matrixfloat *buffer);

/* basic I/O functions */
void matrix_bang(t_matrix *x); /* output the matrix stored in atombuffer */
void matrix_matrix2(t_matrix *x, t_symbol *s, int argc, t_atom *argv); /* store the matrix in atombuffer */

/* set data */
void matrix_set(t_matrix *x, t_float f); /* set the entire matrix to "f" */
void matrix_zeros(t_matrix *x, t_symbol *s, int argc, t_atom *argv);
void matrix_ones(t_matrix *x, t_symbol *s, int argc, t_atom *argv);
void matrix_eye(t_matrix *x, t_symbol *s, int argc, t_atom *argv);
void matrix_egg(t_matrix *x, t_symbol *s, int argc, t_atom *argv);
void matrix_diag(t_matrix *x, t_symbol *s, int argc, t_atom *argv);
void matrix_diegg(t_matrix *x, t_symbol *s, int argc, t_atom *argv);

/* get/set data */
void matrix_row(t_matrix *x, t_symbol *s, int argc, t_atom *argv);
void matrix_col(t_matrix *x, t_symbol *s, int argc, t_atom *argv);
void matrix_element(t_matrix *x, t_symbol *s, int argc, t_atom *argv);


void mtx_bin_matrix2(t_mtx_binmtx *x, t_symbol *s, int argc, t_atom *argv);
void mtx_binmtx_bang(t_mtx_binmtx *x);
void mtx_binmtx_free(t_mtx_binmtx *x);
void mtx_binscalar_bang(t_mtx_binscalar *x);
void mtx_binscalar_free(t_mtx_binscalar *x);


/* some math */

/*  invert a square matrix (row=col=rowcol) */
t_matrixfloat*mtx_doInvert(t_matrixfloat*input, int rowcol);
/*  transpose a matrix */
t_matrixfloat*mtx_doTranspose(t_matrixfloat*output, int row, int col);
/*  multiply matrix A=[rowA*colA] with matrix B=[rowB*colB]; C=A*B; colA=rowB=colArowB */
t_matrixfloat*mtx_doMultiply(int rowA, t_matrixfloat*A, int colArowB, t_matrixfloat*B, int colB);

#endif
