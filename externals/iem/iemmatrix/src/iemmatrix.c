/*
 *  iemmatrix
 *
 *  objects for manipulating simple matrices
 *  mostly refering to matlab/octave matrix functions
 *
 * Copyright (c) IOhannes m zmölnig, forum::für::umläute
 * IEM, Graz, Austria
 *
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.
 *
 */
#include "iemmatrix.h"

void mtx_mul_setup();
void mtx_div_setup();
void mtx_add_setup();
void mtx_sub_setup();
void mtx_pow_setup();
void mtx_col_setup();
void mtx_cholesky_setup();
void mtx_dbtorms_setup();
void mtx_rmstodb_setup();
void mtx_diag_setup();
void mtx_diegg_setup();
void mtx_distance2_setup();
void mtx_egg_setup();
void mtx_element_setup();
void mtx_exp_setup();
void mtx_eye_setup();
void mtx_gauss_setup();
void mtx_inverse_setup();
void mtx_log_setup();
void mtx_matrix_setup();
void mtx_mean_setup();
void mtx_check_setup();
void mtx_print_setup();
void mtx_prod_setup();
void mtx_ones_setup();
void mtx_pivot_setup();
void mtx_rand_setup();
void mtx_resize_setup();
void mtx_roll_setup();
void mtx_row_setup();
void mtx_scroll_setup();
void mtx_size_setup();
void mtx_sum_setup();
void mtx_trace_setup();
void mtx_transpose_setup();
void mtx_zeros_setup();
void mtx_mul_tilde_setup();

void iemtx_setup(){
  mtx_mul_setup();
  mtx_div_setup();
  mtx_add_setup();
  mtx_sub_setup();
  mtx_pow_setup();
  mtx_col_setup();
  mtx_cholesky_setup();
  mtx_diag_setup();
  mtx_diegg_setup();
  mtx_distance2_setup();
  mtx_egg_setup();
  mtx_element_setup();
  mtx_exp_setup();
  mtx_eye_setup();
  mtx_gauss_setup();
  mtx_inverse_setup();
  mtx_log_setup();
  mtx_dbtorms_setup();
  mtx_rmstodb_setup();  
  mtx_matrix_setup();
  mtx_mean_setup();
  mtx_check_setup();
  mtx_print_setup();
  mtx_prod_setup();
  mtx_ones_setup();
  mtx_pivot_setup();
  mtx_rand_setup();
  mtx_resize_setup();
  mtx_roll_setup();
  mtx_row_setup();
  mtx_scroll_setup();
  mtx_size_setup();
  mtx_sum_setup();
  mtx_trace_setup();
  mtx_transpose_setup();
  mtx_zeros_setup();
  mtx_mul_tilde_setup();
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
