///////////////////////////////////////////////////////////////////////////////////
/* Chaos Math PD Externals                                                       */
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


#ifndef __DATE__ 
#define __DATE__ "without using a gnu compiler"
#endif

typedef struct _chaos
{
     t_object x_obj;
} t_chaos;

static t_class* chaos_class;

	/* objects */
void henon_setup();
void ikeda_setup();
void lorenz_setup();
void rossler_setup();

static void* chaos_new(t_symbol* s)
{
    t_chaos *x = (t_chaos *)pd_new(chaos_class);
    return (x);
}

void chaos_setup(void) 
{
	chaos_class = class_new(gensym("chaos"), (t_newmethod)chaos_new, 0,
    	sizeof(t_chaos), 0,0);

	post("-------------------------");              /* Copyright info */
	post("Chaos PD Externals");
	post("Copyright Ben Bogart 2002");
	post("Win32 compilation by joge 2002");

	henon_setup();
	ikeda_setup();
	lorenz_setup();
	rossler_setup();
	
	post("-------------------------");	
}

