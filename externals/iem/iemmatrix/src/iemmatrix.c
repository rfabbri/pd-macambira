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

void mtx_binops_setup();
void mtx_col_setup();
void mtx_diag_setup();
void mtx_diegg_setup();
void mtx_distance2_setup();
void mtx_egg_setup();
void mtx_element_setup();
void mtx_eye_setup();
void mtx_inverse_setup();
void mtx_matrix_setup();
void mtx_mean_setup();
void mtx_check_setup();
void mtx_print_setup();
void mtx_ones_setup();
void mtx_pivot_setup();
void mtx_rand_setup();
void mtx_resize_setup();
void mtx_roll_setup();
void mtx_row_setup();
void mtx_scroll_setup();
void mtx_size_setup();
void mtx_trace_setup();
void mtx_transpose_setup();
void mtx_zeros_setup();
void mtx_tilde_setup();

void iemtx_setup(){
  mtx_binops_setup();
  mtx_col_setup();
  mtx_diag_setup();
  mtx_diegg_setup();
  mtx_distance2_setup();
  mtx_egg_setup();
  mtx_element_setup();
  mtx_eye_setup();
  mtx_inverse_setup();
  mtx_matrix_setup();
  mtx_mean_setup();
  mtx_check_setup();
  mtx_print_setup();
  mtx_ones_setup();
  mtx_pivot_setup();
  mtx_rand_setup();
  mtx_resize_setup();
  mtx_roll_setup();
  mtx_row_setup();
  mtx_scroll_setup();
  mtx_size_setup();
  mtx_trace_setup();
  mtx_transpose_setup();
  mtx_zeros_setup();
  mtx_tilde_setup();
}

void iemmatrix_setup(){
  post("");
  post("iemmatrix "VERSION);
  post("\tobjects for manipulating 2d-matrices");
  post("\t(c) IOhannes m zmölnig, Thomas Musil :: iem, 2001-2005");
  post("\tcompiled "__DATE__" : "__TIME__);
  post("");

  iemtx_setup();
}
