///////////////////////////////////////////////////////////////////////////////////
/* Rossler's Attractor PD External                                               */
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

t_class *rossler_class;

typedef struct rossler_struct
{
	t_object myobj;
	double h,a,b,c,lx0,ly0,lz0;
	t_outlet *y_outlet;
	t_outlet *z_outlet;
}	rossler_struct;

static void calculate(rossler_struct *x)
{
	double lx0,ly0,lz0,lx1,ly1,lz1;
	double h,a,b,c;

	h = x->h;
	a = x->a;
	b = x->b;
	c = x->c;
	lx0 = x->lx0;
	ly0 = x->ly0;
	lz0 = x->lz0;

	lx1 = lx0 + h * (-ly0 - lz0);
	ly1 = ly0 + h * (lx0 + (a * ly0));
	lz1 = lz0 + h * (b + (lx0*lz0) - (c * lz0));;
	x->lx0 = lx1;
	x->ly0 = ly1;
	x->lz0 = lz1;

	outlet_float(x->myobj.ob_outlet, (t_float)lx1);
	outlet_float(x->y_outlet, (t_float)ly1);
	outlet_float(x->z_outlet, (t_float)lz1);
}

static void reset(rossler_struct *x)
{
		x->lx0 = 0.1;
        x->ly0 = 0;
        x->lz0 = 0;
}

static void param(rossler_struct *x, t_floatarg h, t_floatarg a, t_floatarg b, t_floatarg c)
{
        x->h = (double)h;
        x->a = (double)a;
        x->b = (double)b;
        x->c = (double)c;
}

void *rossler_new(void)
{
    rossler_struct *x = (rossler_struct *)pd_new(rossler_class);
	x->h = 0.01;
	x->a = 0.2;
	x->b = 0.2;
	x->c = 5.7;
	x->lx0 = 0.1;
	x->ly0 = 0;
	x->lz0 = 0;
     
    outlet_new(&x->myobj, &s_float);				/* Default float outlet */
	x->y_outlet = outlet_new(&x->myobj, &s_float);  /* Two new Outlets */
	x->z_outlet = outlet_new(&x->myobj, &s_float);
        return (void *)x;
}


void rossler_setup(void)
{
    post("rossler");
        
	rossler_class = class_new(gensym("rossler"),	/* symname is the symbolic name */
	(t_newmethod)rossler_new,						/* Constructor Function */
	0,												/* Destructor Function */
	sizeof(rossler_struct),							/* Size of the structure */
	CLASS_DEFAULT,									/* Graphical Representation */
	0);												/* 0 Terminates Argument List */

	class_addbang(rossler_class, (t_method)calculate);

	class_addmethod(rossler_class,
		(t_method)reset,
		gensym("reset"),
		0);

	class_addmethod(rossler_class,
		(t_method)param,
		gensym("param"),
		A_DEFFLOAT,
		A_DEFFLOAT,
		A_DEFFLOAT,
		A_DEFFLOAT,
		0);
}
