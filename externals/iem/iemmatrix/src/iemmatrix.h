/* ************************************* */
/* iemmatrix                             */
/* ************************************* */
/*  objects for simple matrix operations */
/* ************************************* */

/* IEMMATRIX is a runtime-library for miller s. puckette's realtime-computermusic-software "pure data"
 * therefore you NEED "pure data" to make any use of the IEMMATRIX external
 * (except if you want to use the code for other things)
 * download "pure data" at
 *
 * http://pd.iem.at
 * ftp://iem.at/pd
 *
 * 
 * IEMMATRIX is published under the Lesser GNU GeneralPublicLicense (LGPL),
 * that must be shipped with IEMMATRIX.
 * if you are using Debian GNU/linux, 
 * the lesser GNU-GPL can be found under /usr/share/common-licenses/LGPL
 * if you still haven't found a copy of the lesser GNU-GPL, have a look at http://www.gnu.org
 *
 * "pure data" has it's own license, that comes shipped with "pure data".
 *
 * there are ABSOLUTELY NO WARRANTIES for anything
 */

#ifndef INCLUDE_IEMMATRIX_H__
#define INCLUDE_IEMMATRIX_H__

#include "m_pd.h"

#define VERSION "0.1"

#ifdef NT
# pragma warning( disable : 4244 )
# pragma warning( disable : 4305 )
#endif

#include <math.h>
#include <stdio.h>

#include <string.h>
#include <memory.h>

#ifdef NT
# define fabsf fabs
#endif

typedef long double t_matrixfloat;


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

#endif
