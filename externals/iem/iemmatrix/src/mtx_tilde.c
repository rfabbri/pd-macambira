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


/*
  the sigmatrix objects ::
  mtx_*~      : multiply a n-vector of in~ with a matrix to get a m-vector of out~
                 line~ between the 2 matrices, to make it useable as a mixer
*/


/* --------------------------- matrix~ ----------------------------------
 *
 * multiply a n-vector of signals with a (n*m) matrix, to get m output-streams.
 * make the (n*m)-matrix of scalars to be liny~
 *
 *  1703:forum::für::umläute:2001
 */

static t_class *mtx_multilde_class;

typedef struct _mtx_multilde {
  t_object x_obj;

  t_float time;
  int ticksleft;
  int retarget;
  t_float msec2tick;

  t_float**value;
  t_float**target;
  t_float**increment; /* single precision is really a bad, especially when doing long line~s.
			* but the biginc (like in msp's line~ (d_ctl.c) is far too expensive... */
  t_float**sigIN;
  t_float**sigOUT;
  t_float *sigBUF;

  int      n_sigIN;    /* columns */
  int      n_sigOUT;   /* rows    */

  int      compatibility; /* 0=default; 1=zexy; 2=iemlib */
} t_mtx_multilde;

/* the message thing */

static void mtx_multilde_matrix_default(t_mtx_multilde *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row, c=0, r=0;

  if (argc<2){
    post("matrix~ : bad matrix !");
    return;
  }

  row = atom_getfloat(argv++);
  col = atom_getfloat(argv++);
  argc-=2;

  if((col!=x->n_sigOUT)||(row!=x->n_sigIN)){
    post("matrix~ : matrix dimensions do not match !!");
    return;
  }
  if(argc<row*col){
    post("matrix~ : reduced matrices not yet supported");
    return;
  }

  if (x->time<=0) {
    for(r=0; r<row; r++)
      for(c=0; c<col; c++)
	x->target[c][r]=x->value[c][r]=atom_getfloat(argv++);
    x->time=x->ticksleft=x->retarget=0;
  } else {
    for(r=0; r<row; r++)
      for(c=0; c<col; c++)
	x->target[c][r]=atom_getfloat(argv++);
    x->retarget=1;
  }
}

static void mtx_multilde_matrix_zexy(t_mtx_multilde *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row, c=0, r=0;

  if (argc<2){
    post("matrix~ : bad matrix !");
    return;
  }

  row = atom_getfloat(argv++);
  col = atom_getfloat(argv++);
  argc-=2;

  if((col!=x->n_sigOUT)||(row!=x->n_sigIN)){
    post("matrix~ : matrix dimensions do not match !!");
    return;
  }
  if(argc<row*col){
    post("matrix~ : reduced matrices not yet supported");
    return;
  }

  if (x->time<=0) {
    for(r=0; r<row; r++)
      for(c=0; c<col; c++)
	x->target[c][r]=x->value[c][r]=atom_getfloat(argv++);
    x->time=x->ticksleft=x->retarget=0;
  } else {
    for(r=0; r<row; r++)
      for(c=0; c<col; c++)
	x->target[c][r]=atom_getfloat(argv++);
    x->retarget=1;
  }
}

static void mtx_multilde_matrix_iemlib(t_mtx_multilde *x, t_symbol *s, int argc, t_atom *argv)
{
  int col, row, c=0, r=0;

  if (argc<2){
    post("matrix~ : bad matrix !");
    return;
  }

  row = atom_getfloat(argv++);
  col = atom_getfloat(argv++);
  argc-=2;

  if((col!=x->n_sigOUT)||(row!=x->n_sigIN)){
    post("matrix~ : matrix dimensions do not match !!");
    return;
  }
  if(argc<row*col){
    post("matrix~ : reduced matrices not yet supported");
    return;
  }

  if (x->time<=0) {
    for(r=0; r<row; r++)
      for(c=0; c<col; c++)
	x->target[c][r]=x->value[c][r]=atom_getfloat(argv++);
    x->time=x->ticksleft=x->retarget=0;
  } else {
    for(r=0; r<row; r++)
      for(c=0; c<col; c++)
	x->target[c][r]=atom_getfloat(argv++);
    x->retarget=1;
  }
}
static void mtx_multilde_matrix(t_mtx_multilde *x, t_symbol *s, int argc, t_atom *argv){
  switch (x->compatibility){
  default:mtx_multilde_matrix_default(x,s,argc,argv);
    break;
  case 1:mtx_multilde_matrix_zexy(x,s,argc,argv);
    break;
  case 2:mtx_multilde_matrix_iemlib(x,s,argc,argv);
    break;
  }
}

static void mtx_multilde_stop(t_mtx_multilde *x)
{
  int c = x->n_sigOUT, r;
  t_float *tgt, *val;
  while(c--){
    tgt=x->target[c];
    val=x->value [c];
    r=x->n_sigIN;
    while(r--)*tgt++=*val++;
  }
  x->ticksleft = x->retarget = 0;
}


/* the dsp thing */

static t_int *mtx_multilde_perform(t_int *w)
{
  t_mtx_multilde *x = (t_mtx_multilde *)(w[1]);
  int n = (int)(w[2]);

  int r, c;

  t_float **out = x->sigOUT;
  t_float **in  = x->sigIN;

  t_float  *buf = x->sigBUF, *sigBUF = buf;

  t_float **value     = x->value;
  t_float **target    = x->target;
  t_float **increment = x->increment;

  t_float *inc, *val, *tgt;

  int n_IN=x->n_sigIN, n_OUT=x->n_sigOUT;

  if (x->retarget) {
    int nticks = x->time * x->msec2tick;
    t_float oneovernos;

    if (!nticks) nticks = 1;
    oneovernos = 1./(nticks*n);
    x->ticksleft = nticks;

    c = n_OUT;
    while(c--) {
      inc=increment[c];
      val=value[c];
      tgt=target[c];
      r=n_IN;
      while(r--)*inc++=(*tgt++-*val++)*oneovernos;
    }

    x->retarget = 0;
  }

  if (x->ticksleft) {
    int N=n-1;
    n=-1;
    //    while (n--) {
    while(n++<N){
      c = n_OUT;
      while(c--) {
	t_float sum = 0;
	val = value[c]+n_IN-1;
	inc = increment[c]+n_IN-1;
	r=n_IN;

	while(r--)sum+=in[r][n]*(*val--+=*inc--);

	sigBUF[c]=sum;
      }
      buf = sigBUF;
      c = n_OUT;
      while(c--)out[c][n]=*buf++;
    }
    if (!--x->ticksleft) {
      c = n_OUT;
      while(c--){
	val=value[c];
	tgt=target[c];
	r=n_IN;
	while(r--)*val++=*tgt++;
      }
    }
  } else { /* no ticks left */
    while (n--) {
      c = n_OUT;
      while(c--) {
	t_float sum = 0;
	val = value[c]+n_IN-1;
	r = n_IN;
	while(r--)sum+=in[r][n]**val--; 
	sigBUF[c]=sum;
      }
      buf = sigBUF;
      c = n_OUT;
      while(c--)out[c][n]=*buf++;
    }
  }
  return (w+3);
}

static void mtx_multilde_dsp(t_mtx_multilde *x, t_signal **sp)
{
  int o = x->n_sigOUT, i=x->n_sigIN, n=0;
  t_float **dummy = x->sigIN;

  while(i--)*dummy++=sp[n++]->s_vec;

  dummy =x->sigOUT;
  while(o--)dummy[o]=sp[n++]->s_vec;

  x->msec2tick = sp[0]->s_sr / (1000.f * sp[0]->s_n);
  dsp_add(mtx_multilde_perform, 2, x, sp[0]->s_n);
}


/* setup/setdown things */

static void mtx_multilde_free(t_mtx_multilde *x)
{
  int i = x->n_sigOUT;
  while(i--) {
    freebytes(x->value    [i], x->n_sigOUT * sizeof(t_float *));
    freebytes(x->target   [i], x->n_sigOUT * sizeof(t_float *));
    freebytes(x->increment[i], x->n_sigOUT * sizeof(t_float *));
  }

  freebytes(x->value,     sizeof(x->value));
  freebytes(x->target,    sizeof(x->target));
  freebytes(x->increment, sizeof(x->increment));

  freebytes(x->sigIN,    x->n_sigIN  * sizeof(t_float *));
  freebytes(x->sigOUT,   x->n_sigOUT * sizeof(t_float *));
  freebytes(x->sigBUF,   x->n_sigOUT * sizeof(t_float  ));
}

static void *mtx_multilde_new(t_symbol *s, int argc, t_atom *argv)
{
    t_mtx_multilde *x = (t_mtx_multilde *)pd_new(mtx_multilde_class);
    int i;
    int InOut=0;
    x->compatibility=0;

    if(s==gensym("matrix~")){ // zexy-compat
      error("[matrix~] is deprecated");//; use [mtx_*~] instead");
      x->compatibility=1;
      InOut=1;
    }

    if(s==gensym("matrix_mul~")){ // iemlib-compat
      error("[matrix_mul~] is deprecated; use [mtx_*~] instead");
      x->compatibility=2;
      InOut=1;
    }

    x->time = 0;

    switch (argc) {
    case 0:
      x->n_sigIN = x->n_sigOUT = 1;
     break;
    case 1:
      x->n_sigIN = x->n_sigOUT = atom_getfloat(argv);
      break;
    default:
      x->time= atom_getfloat(argv+2);
    case 2:
      if(InOut){
	x->n_sigIN  = atom_getfloat(argv);
	x->n_sigOUT = atom_getfloat(argv+1);
      } else {
	x->n_sigOUT = atom_getfloat(argv);
	x->n_sigIN  = atom_getfloat(argv+1);
      }
      break;
    }

    if (x->time<0) x->time=0;
    if (x->n_sigIN <1) x->n_sigIN =1;
    if (x->n_sigOUT<1) x->n_sigOUT=1;

    /* the inlets */
    i=x->n_sigIN-1;
    while(i--)inlet_new(&x->x_obj,&x->x_obj.ob_pd,&s_signal,&s_signal);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("matrix"), gensym(""));
    floatinlet_new(&x->x_obj, &x->time);

    /* the outlets */
    i=x->n_sigOUT;
    while(i--)outlet_new(&x->x_obj,&s_signal);

    /* make all the buffers */
    x->sigIN  = (t_float**)getbytes(x->n_sigIN  * sizeof(t_float*));
    x->sigOUT = (t_float**)getbytes(x->n_sigOUT * sizeof(t_float*));
    x->sigBUF = (t_float *)getbytes(x->n_sigOUT * sizeof(t_float ));

    x->value     = (t_float **)getbytes(x->n_sigOUT * sizeof(t_float));
    x->target    = (t_float **)getbytes(x->n_sigOUT * sizeof(t_float));
    x->increment = (t_float **)getbytes(x->n_sigOUT * sizeof(t_float));

    i = x->n_sigOUT;
    while(i--){
      int j = x->n_sigIN;
      x->sigOUT   [i] = 0;
      x->value    [i] = (t_float *)getbytes(x->n_sigIN * sizeof(t_float));
      x->target   [i] = (t_float *)getbytes(x->n_sigIN * sizeof(t_float));
      x->increment[i] = (t_float *)getbytes(x->n_sigIN * sizeof(t_float));

      while(j--)x->value[i][j]=x->target[i][j]=x->increment[i][j]=0;
    }

    i = x->n_sigIN;
    while(i--)x->sigIN[i] = 0;    

    x->msec2tick = x->ticksleft = x->retarget = 0;
    return (x);
}

static void mtx_multilde_setup(void)
{
  mtx_multilde_class = class_new(gensym("mtx_*~"), (t_newmethod)mtx_multilde_new, 
				 (t_method)mtx_multilde_free,
				 sizeof(t_mtx_multilde), 0, A_GIMME, 0);

  class_addcreator((t_newmethod)mtx_multilde_new, gensym("matrix~"), A_GIMME,0);

  class_addmethod(mtx_multilde_class, (t_method)mtx_multilde_dsp, gensym("dsp"), 0);
  class_addmethod(mtx_multilde_class, nullfn, gensym("signal"), 0);

  class_addmethod(mtx_multilde_class, (t_method)mtx_multilde_matrix, gensym(""), A_GIMME, 0);
  class_addmethod(mtx_multilde_class, (t_method)mtx_multilde_stop, gensym("stop"), 0);

  class_sethelpsymbol(mtx_multilde_class, gensym("iemmatrix/matrix~"));
}

void mtx_tilde_setup(void)
{
  mtx_multilde_setup();
}
