/*
 *   abs.c  - computes absolute value of a signal 
 *   Copyright (c) 2000-2003 by Tom Schouten
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "m_pd.h"
#include <math.h>

/* ------------------------- abs~ -------------------------- */
static t_class *abs_class;

typedef struct _abs
{
    t_object x_obj;
} t_abs;

static t_int *abs_perform(t_int *w)
{
    t_abs *x = (t_abs *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
    while (n--)
    {
    	float f = *in++;
    	if (f < 0) f = -f;
    	*out++ = f;
    }
    return (w+5);
}

static void abs_dsp(t_abs *x, t_signal **sp)
{
    dsp_add(abs_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void *abs_new(void)
{
    t_abs *x = (t_abs *)pd_new(abs_class);
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

void abs_tilde_setup(void)
{
    abs_class = class_new(gensym("abs~"), (t_newmethod)abs_new, 0,
    	sizeof(t_abs), 0, A_NULL);
    class_addmethod(abs_class, (t_method)nullfn, &s_signal, A_NULL);
    class_addmethod(abs_class, (t_method)abs_dsp, gensym("dsp"), A_NULL);
}
