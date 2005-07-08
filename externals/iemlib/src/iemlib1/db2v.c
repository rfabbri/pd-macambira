/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2005 */

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


#include "m_pd.h"
#include "iemlib.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

/* -------- db2v - a techn. dB to rms-value converter. --------- */

static t_class *db2v_class;

float db2v(float f)
{
  return (f <= -199.9 ? 0 : exp(0.11512925465 * f));
}

static void db2v_float(t_object *x, t_float f)
{
  outlet_float(x->ob_outlet, db2v(f));
}

static void *db2v_new(void)
{
  t_object *x = (t_object *)pd_new(db2v_class);
  outlet_new(x, &s_float);
  return (x);
}

void db2v_setup(void)
{
  db2v_class = class_new(gensym("db2v"), db2v_new, 0,
    sizeof(t_object), 0, 0);
  class_addfloat(db2v_class, (t_method)db2v_float);
  class_sethelpsymbol(db2v_class, gensym("iemhelp/help-db2v"));
}
