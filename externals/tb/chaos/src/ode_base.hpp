// 
//  
//  chaos~
//  Copyright (C) 2004  Tim Blechmann
//  
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//  
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//  
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.  If not, write to
//  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.


#ifndef __ode_base_hpp

#include "chaos_base.hpp"

class ode_base
	: protected chaos_base
{
public:
	void set_method(int i)
	{
		if (i >=0 && i <4)
			m_method = (unsigned char) i;
		post("no such method");
	}

	t_int get_method()
	{
		return (int) m_method;
	}

	void set_dt(t_float f)
	{
		if (f >= 0)
			m_dt = (data_t)f;
		else
			post("invalid value for dt: %f", f);
	}
	
	t_float get_dt()
	{
		return (t_float) m_dt;
	}

	void m_step();
	
protected:
	unsigned char m_method; /* 0: rk1, 1: rk2, 3: rk4 */
	data_t  m_dt;           /* step width */

	data_t* m_k[3];         /* temporary arrays for runge kutta */
	data_t* m_tmp;   

	virtual void m_system (data_t* deriv, data_t* data);

	void rk1 ();
	void rk2 ();
	void rk4 ();
};

#define ODE_CALLBACKS											\
CHAOS_CALLBACKS;												\
FLEXT_CALLVAR_I(m_system->get_method, m_system->set_method);	\
FLEXT_CALLVAR_F(m_system->get_dt, m_system->set_dt);

#define ODE_ATTRIBUTES														\
CHAOS_ATTRIBUTES;															\
FLEXT_ADDATTR_VAR("method", m_system->get_method, m_system->set_method);	\
FLEXT_ADDATTR_VAR("dt",m_system->get_dt, m_system->set_dt);


#define __ode_base_hpp
#endif /* __ode_base_hpp */
