///////////////////////////////////////////////////////////////////////////////////
/* Henon's Attractor PD External                                                 */
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

t_class *myclass;

typedef struct thisismystruct
{
	t_object myobj;
	double a,b,lx0,ly0;
	t_outlet *y_outlet;
}	mystruct;

static void calculate(mystruct *x)
{
	double lx0,ly0,lx1,ly1;
	double a,b;

	a = x->a;
	b = x->b;
	lx0 = x->lx0;
	ly0 = x->ly0;

	lx1 = (ly0 + 1) - (a * pow(lx0,2));
	ly1 = b * lx0;
	x->lx0 = lx1;
	x->ly0 = ly1;

	outlet_float(x->myobj.ob_outlet, (t_float)lx1);
	outlet_float(x->y_outlet, (t_float)ly1);
}

static void reset(mystruct *x)
{
	x->lx0 = 1;
        x->ly0 = 1;
}

static void param(mystruct *x, t_floatarg a, t_floatarg b)
{
        x->a = (double)a;
        x->b = (double)b;
}

void *henon_new(void)
{
        mystruct *x = (mystruct *)pd_new(myclass);
	x->a = 1.4;
	x->b = 0.3;
	x->lx0 = 1;
	x->ly0 = 1;
     
        outlet_new(&x->myobj, &s_float);		/* Default float outlet */
	x->y_outlet = outlet_new(&x->myobj, &s_float);  /* New Outlet */
        return (void *)x;
}


void henon_setup(void)
{
	post("-------------------------");		/* Copyright info */
	post("Chaos PD Externals");
	post("Copyright Ben Bogart 2002");
	post("-------------------------");

	myclass = class_new(gensym("henon"),		/* symname is the symbolic name */
	(t_newmethod)henon_new,				/* Constructor Function */
	0,						/* Destructor Function */
	sizeof(mystruct),				/* Size of the structure */
	CLASS_DEFAULT,					/* Graphical Representation */
	0);						/* 0 Terminates Argument List */

	class_addbang(myclass, (t_method)calculate);
	class_addmethod(myclass,
			(t_method)reset,
			gensym("reset"),
			0);

        class_addmethod(myclass,
                        (t_method)param,
                        gensym("param"),
                        A_DEFFLOAT,
                        A_DEFFLOAT,
                        0);
}


