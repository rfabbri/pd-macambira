/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2005 */


#include "m_pd.h"
#include "iemlib.h"

/* ------------------- iem_blocksize~ -------------------- */
/* -- outputs the current signal-blocksize of a window --- */

static t_class *sigiem_blocksize_class;

typedef struct _sigiem_blocksize
{
  t_object  x_obj;
  t_float   x_blocksize;
  t_clock   *x_clock;
  t_float   x_f;
} t_sigiem_blocksize;

static void sigiem_blocksize_out(t_sigiem_blocksize *x)
{
  outlet_float(x->x_obj.ob_outlet, x->x_blocksize);
}

static void sigiem_blocksize_free(t_sigiem_blocksize *x)
{
  clock_free(x->x_clock);
}

static void *sigiem_blocksize_new(t_symbol *s)
{
  t_sigiem_blocksize *x = (t_sigiem_blocksize *)pd_new(sigiem_blocksize_class);
  x->x_clock = clock_new(x, (t_method)sigiem_blocksize_out);
  outlet_new(&x->x_obj, &s_float);
  x->x_blocksize = 64.0f;
  x->x_f = 0.0f;
  return (x);
}

static void sigiem_blocksize_dsp(t_sigiem_blocksize *x, t_signal **sp)
{
  x->x_blocksize = (t_float)(sp[0]->s_n);
  clock_delay(x->x_clock, 0.0f);
}

void sigiem_blocksize_setup(void)
{
  sigiem_blocksize_class = class_new(gensym("iem_blocksize~"), (t_newmethod)sigiem_blocksize_new,
    (t_method)sigiem_blocksize_free, sizeof(t_sigiem_blocksize), 0, 0);
  CLASS_MAINSIGNALIN(sigiem_blocksize_class, t_sigiem_blocksize, x_f);
  class_addmethod(sigiem_blocksize_class, (t_method)sigiem_blocksize_dsp, gensym("dsp"), 0);
}
