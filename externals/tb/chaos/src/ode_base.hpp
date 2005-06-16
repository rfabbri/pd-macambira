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
	: public chaos_base
{
public:
	void set_method(int i)
	{
		if (i >=0 && i <4)
		{
			m_method = (unsigned char) i;
			switch (i)
			{
			case 0:
				m_routine = &ode_base::rk1;
				break;
			case 1:
				m_routine = &ode_base::rk2;
				break;
			case 2:
				m_routine = &ode_base::rk4;
			}
		}
		else
			post("no such method");
	}

	t_int get_method()
	{
		return (int) m_method;
	}

	CHAOS_SYSPAR_FUNCS_PRED(dt, m_pred_dt);

	bool m_pred_dt(t_float f)
	{
		return (f >= 0);
	}

	virtual void m_step()
	{
		(this->*m_routine)();
	}
	
	void ode_base_alloc()
	{
		int dimension = get_num_eq();
		
		for (int i = 0; i != 3; ++i)
		{
			m_k[i] = new data_t[dimension];
		}

		m_tmp = new data_t[dimension];
	}
	
	void ode_base_free()
	{
		for (int i = 0; i != 3; ++i)
		{
			delete m_k[i];
		}
		delete m_tmp;
	}

protected:
 	void (ode_base::*m_routine)(void);

	unsigned char m_method; /* 0: rk1, 1: rk2, 3: rk4 */

	data_t* m_k[3];         /* temporary arrays for runge kutta */
	data_t* m_tmp;   

	virtual void m_system (data_t* deriv, data_t* data) = 0;

	void rk1 ();
	void rk2 ();
	void rk4 ();
};

#define ODE_CALLBACKS							\
CHAOS_CALLBACKS;								\
CHAOS_SYS_CALLBACKS_I(method);					\
CHAOS_SYS_CALLBACKS(dt);

#define ODE_ATTRIBUTES							\
CHAOS_ATTRIBUTES;								\
CHAOS_SYS_ATTRIBUTE(method);					\
CHAOS_SYS_ATTRIBUTE(dt);


#define __ode_base_hpp
#endif /* __ode_base_hpp */
