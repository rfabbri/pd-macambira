///////////////////////////////////////////////////////////////////////////////////
/* Ikeda Attractor PD External                                                   */
/* Copyright Ben Bogart 2002                                                     */
/* This program is distributed under the terms of the GNU General Public License */
///////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
/* This file is part of Chaos PD Externals.                                      */
/*                                                                               */
/* Chaos PD Externals are free software; you can redistribute them and/or modify */
/* them under the terms of the GNU General Public License as published by        */
/* the Free Software Foundation; either version 2 of the License, or             */
/* (at your option) any later version.                                           */
/*                                                                               */
/* Chaos PD Externals are distributed in the hope that they will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of                */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                 */
/* GNU General Public License for more details.                                  */
/*                                                                               */
/* You should have received a copy of the GNU General Public License             */
/* along with the Chaos PD Externals; if not, write to the Free Software         */
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA     */
///////////////////////////////////////////////////////////////////////////////////

#include "m_pd.h"
#include <math.h>


t_class *ikeda_class;

typedef struct ikeda_struct
{
	t_object ikeda_obj;
	double a, b, c, rho, lx0, ly0;
	t_outlet *y_outlet;
}	ikeda_struct;

static void calculate(ikeda_struct *x)
{
	double lx0, ly0, lx1, ly1;
	double a, b, c, rho, tmp, cos_tmp, sin_tmp;

	a = x->a;
	b = x->b;
	c = x->c;
	rho = x->rho;
	lx0 = x->lx0;
	ly0 = x->ly0;


	tmp = a - c / ( 1.0 + lx0*lx0 + ly0*ly0);
	sin_tmp = sin(tmp);
	cos_tmp = cos(tmp);

	lx1 = rho + b * ( lx0 * cos_tmp - ly0 * sin_tmp);
	ly1 = b * ( lx0 * sin_tmp + ly0 * cos_tmp);
	x->lx0 = lx1;
	x->ly0 = ly1;

	outlet_float(x->ikeda_obj.ob_outlet, (t_float)lx1);
	outlet_float(x->y_outlet, (t_float)ly1);
}

static void reset(ikeda_struct *x)
{
	x->lx0 = 0.1;
	x->ly0 = 0.1;
}

static void param(ikeda_struct *x, t_floatarg a, t_floatarg b, t_floatarg c, t_floatarg rho)
{
	x->a = (double)a;
	x->b = (double)b;
	x->c = (double)c;
	x->rho = (double)rho;
}

void *ikeda_new(void)
{
	ikeda_struct *x = (ikeda_struct *)pd_new(ikeda_class);
	x->a = 0.4;
	x->b = 0.9;
	x->c = 6.0;
	x->rho = 1.0;
	x->lx0 = 0.1;
	x->ly0 = 0.1;
     
	outlet_new(&x->ikeda_obj, &s_float);				/* Default float outlet */
	x->y_outlet = outlet_new(&x->ikeda_obj, &s_float);  /* New Outlet */

	return (void *)x;
}


void ikeda_setup(void)
{
	post("ikeda");

	ikeda_class = class_new(gensym("ikeda"),		/* symname is the symbolic name */
	(t_newmethod)ikeda_new,							/* Constructor Function */
	0,												/* Destructor Function */
	sizeof(ikeda_struct),							/* Size of the structure */
	CLASS_DEFAULT,									/* Graphical Representation */
	0);												/* 0 Terminates Argument List */

	class_addbang(ikeda_class, (t_method)calculate);

	class_addmethod(ikeda_class,
		(t_method)reset,
		gensym("reset"),
		0);

	class_addmethod(ikeda_class,
		(t_method)param,
		gensym("param"),
		A_DEFFLOAT,
		A_DEFFLOAT,
		A_DEFFLOAT,
		A_DEFFLOAT,
		0);
}
