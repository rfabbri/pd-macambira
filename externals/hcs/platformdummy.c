/* -------------------------- platformdummy ----------------------------------- */
/*                                                                              */
/* Space filler object for objects that don't work on the given platform.       */
/* Written by Hans-Christoph Steiner <hans@eds.org>                             */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

#include <m_pd.h>

#define INLET_NUMBER 8
#define OUTLET_NUMBER 8
 
typedef struct platformdummy {
		t_object x_ob;
		t_symbol *x_symbol;
		t_outlet *x_outlets[OUTLET_NUMBER];
} t_platformdummy;

t_class *platformdummy_class;

void *platformdummy_new(void) {
	int i;
	
	t_platformdummy *x = (t_platformdummy *)pd_new(platformdummy_class);

/* counter starts at one since Pd makes the first inlet automatically */
	for (i=1;i<INLET_NUMBER;x++) 		
		symbolinlet_new(&x->x_ob, &x->x_symbol);
	
	for (i=0;i<OUTLET_NUMBER;x++) 		
		x->x_outlets[i] = outlet_new(&x->x_ob, gensym("float"));
	
	return (void *)x;
}

void platformdummy_setup(void) {
	platformdummy_class = class_new(gensym("platformdummy"), (t_newmethod)platformdummy_new,
											  0, sizeof(t_platformdummy), 0, 0);
	class_sethelpsymbol(platformdummy_class, gensym("platformdummy-help.pd"));
}

