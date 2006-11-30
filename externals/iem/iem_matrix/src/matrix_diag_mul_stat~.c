/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_matrix written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"


/* ---------- matrix_diag_mul_stat~ - signal matrix multiplication object with message matrix-coeff. ----------- */

typedef struct matrix_diag_mul_stat_tilde
{
  t_object  x_obj;
  t_float   *x_matbuf;
  t_float   **x_io;
  t_float   *x_buf;
  int       x_bufsize;
  int       x_n_io;
  t_float   x_msi;
} t_matrix_diag_mul_stat_tilde;

t_class *matrix_diag_mul_stat_tilde_class;

static void matrix_diag_mul_stat_tilde_list(t_matrix_diag_mul_stat_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
  int i, n=x->x_n_io;
  t_float *matrix = x->x_matbuf;
  
  if(argc < n)
  {
    post("matrix_diag_mul_stat~ : dimensions do not match !!");
    return;
  }
  
  for(i=0; i<n; i++)
  {
    *matrix++ = atom_getfloat(argv);
    argv++;
  }
}

static void matrix_diag_mul_stat_tilde_diag(t_matrix_diag_mul_stat_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
  matrix_diag_mul_stat_tilde_list(x, &s_list, argc, argv);
}

static void matrix_diag_mul_stat_tilde_element(t_matrix_diag_mul_stat_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
  int i, j, n=x->x_n_io;
  t_float *matrix = x->x_matbuf;
  
  if(argc == 2)
  {
    i = (int)(atom_getint(argv));
    argv++;
    if((i >= 1) && (i <= n))
      matrix[i-1] = atom_getfloat(argv);
  }
  else if(argc == 3)
  {
    i = (int)(atom_getint(argv));
    argv++;
    j = (int)(atom_getint(argv));
    argv++;
    if((i >= 1) && (i <= n) && (i == j))
      matrix[i-1] = atom_getfloat(argv);
  }
}

/* the dsp thing */

static t_int *matrix_diag_mul_stat_tilde_perform(t_int *w)
{
  t_matrix_diag_mul_stat_tilde *x = (t_matrix_diag_mul_stat_tilde *)(w[1]);
  int n = (int)(w[2]);
  t_float **io = x->x_io;
  t_float *buf = x->x_buf;
  t_float *mat = x->x_matbuf;
  int n_io = x->x_n_io;
  int hn_io = n_io / 2;
  int dn_io = 2 * n_io;
  t_float *in, *out1, *out2, mul;
  int i, j;
  
  for(j=0; j<n_io; j++)
  {
    mul = mat[j];
    in = io[j];
    for(i=0; i<n; i++)
    {
      in[i] *= mul;
    }
  }
  for(j=0; j<hn_io; j++)
  {
    in = io[j];
    for(i=0; i<n; i++)
    {
      buf[i] = in[i];
    }
    in = io[n_io-j-1];
    out1 = io[j+n_io];
    out2 = io[dn_io-j-1];
    for(i=0; i<n; i++)
    {
      out1[i] = in[i];
      out2[i] = buf[i];
    }
  }
  return (w+3);
}

static t_int *matrix_diag_mul_stat_tilde_perf8(t_int *w)
{
  t_matrix_diag_mul_stat_tilde *x = (t_matrix_diag_mul_stat_tilde *)(w[1]);
  int n = (int)(w[2]);
  t_float **io = x->x_io;
  t_float *buf;
  t_float *mat = x->x_matbuf;
  int n_io = x->x_n_io;
  int hn_io = n_io / 2;
  int dn_io = 2 * n_io;
  t_float *in, *out1, *out2, mul;
  int i, j;
  
  for(j=0; j<n_io; j++)
  {
    mul = mat[j];
    in = io[j];
    for(i=n; i; i -= 8, in += 8)
    {
      in[0] *= mul;
      in[1] *= mul;
      in[2] *= mul;
      in[3] *= mul;
      in[4] *= mul;
      in[5] *= mul;
      in[6] *= mul;
      in[7] *= mul;
    }
  }
  for(j=0; j<hn_io; j++)
  {
    in = io[j];
    buf = x->x_buf;
    for(i=n; i; i -= 8, buf += 8, in += 8)
    {
      buf[0] = in[0];
      buf[1] = in[1];
      buf[2] = in[2];
      buf[3] = in[3];
      buf[4] = in[4];
      buf[5] = in[5];
      buf[6] = in[6];
      buf[7] = in[7];
    }
    in = io[n_io-j-1];
    out1 = io[j+n_io];
    out2 = io[dn_io-j-1];
    buf = x->x_buf;
    for(i=n; i; i -= 8, buf += 8, in += 8, out1 += 8, out2 += 8)
    {
      out2[0] = in[0];
      out2[1] = in[1];
      out2[2] = in[2];
      out2[3] = in[3];
      out2[4] = in[4];
      out2[5] = in[5];
      out2[6] = in[6];
      out2[7] = in[7];
      out1[0] = buf[0];
      out1[1] = buf[1];
      out1[2] = buf[2];
      out1[3] = buf[3];
      out1[4] = buf[4];
      out1[5] = buf[5];
      out1[6] = buf[6];
      out1[7] = buf[7];
    }
  }
  return (w+3);
}

static void matrix_diag_mul_stat_tilde_dsp(t_matrix_diag_mul_stat_tilde *x, t_signal **sp)
{
  int i, n=sp[0]->s_n;
  
  if(!x->x_buf)
  {
    x->x_bufsize = n;
    x->x_buf = (t_float *)getbytes(x->x_bufsize * sizeof(t_float));
  }
  else if(x->x_bufsize != n)
  {
    x->x_buf = (t_float *)resizebytes(x->x_buf, x->x_bufsize*sizeof(t_float), n*sizeof(t_float));
    x->x_bufsize = n;
  }
  
  n = 2 * x->x_n_io;
  for(i=0; i<n; i++)
  {
    x->x_io[i] = sp[i]->s_vec;
    //    post("iovec_addr = %d", (unsigned int)x->x_io[i]);
  }
  
  n = sp[0]->s_n;
  if(n&7)
    dsp_add(matrix_diag_mul_stat_tilde_perform, 2, x, n);
  else
    dsp_add(matrix_diag_mul_stat_tilde_perf8, 2, x, n);
}


/* setup/setdown things */

static void matrix_diag_mul_stat_tilde_free(t_matrix_diag_mul_stat_tilde *x)
{
  freebytes(x->x_matbuf, x->x_n_io * sizeof(t_float));
  freebytes(x->x_io, 2 * x->x_n_io * sizeof(t_float *));
  if(x->x_buf)
    freebytes(x->x_buf, x->x_bufsize * sizeof(t_float));
}

static void *matrix_diag_mul_stat_tilde_new(t_symbol *s, int argc, t_atom *argv)
{
  t_matrix_diag_mul_stat_tilde *x = (t_matrix_diag_mul_stat_tilde *)pd_new(matrix_diag_mul_stat_tilde_class);
  int i;
  
  switch (argc)
  {
  case 0:
    x->x_n_io = 1;
    break;
  default:
    x->x_n_io = (int)atom_getint(argv);
    break;
  }
  
  if(x->x_n_io < 1)
    x->x_n_io = 1;
  i = x->x_n_io - 1;
  while(i--)
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  i = x->x_n_io;
  while(i--)
    outlet_new(&x->x_obj, &s_signal);
  x->x_msi = 0;
  x->x_buf = (t_float *)0;
  x->x_bufsize = 0;
  x->x_matbuf = (t_float *)getbytes(x->x_n_io * sizeof(t_float));
  i = x->x_n_io;
  while(i--)
    x->x_matbuf[i] = 0.0f;
  x->x_io = (t_float **)getbytes(2 * x->x_n_io * sizeof(t_float *));
  return(x);
}

void matrix_diag_mul_stat_tilde_setup(void)
{
  matrix_diag_mul_stat_tilde_class = class_new(gensym("matrix_diag_mul_stat~"), (t_newmethod)matrix_diag_mul_stat_tilde_new, (t_method)matrix_diag_mul_stat_tilde_free,
    sizeof(t_matrix_diag_mul_stat_tilde), 0, A_GIMME, 0);
  CLASS_MAINSIGNALIN(matrix_diag_mul_stat_tilde_class, t_matrix_diag_mul_stat_tilde, x_msi);
  class_addmethod(matrix_diag_mul_stat_tilde_class, (t_method)matrix_diag_mul_stat_tilde_dsp, gensym("dsp"), 0);
  class_addmethod(matrix_diag_mul_stat_tilde_class, (t_method)matrix_diag_mul_stat_tilde_diag, gensym("diag"), A_GIMME, 0);
  class_addmethod(matrix_diag_mul_stat_tilde_class, (t_method)matrix_diag_mul_stat_tilde_element, gensym("element"), A_GIMME, 0);
  class_addlist(matrix_diag_mul_stat_tilde_class, (t_method)matrix_diag_mul_stat_tilde_list);
  class_sethelpsymbol(matrix_diag_mul_stat_tilde_class, gensym("iemhelp2/matrix_diag_mul_stat~-help"));
}
