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

#include "ode_base.hpp"

//  lorenz model: dx1/dt = sigma * (x2 - x1)
//                dx2/dt = - x1 * x3 + r * x1 - x2
//                dx3/dt = x1 * x2 - b * x3
//  taken from Willi-Hans Steeb: Chaos and Fractals

class lorenz
	: protected ode_base
{
public:
	logistic()
		: m_sigma(16), m_b(4), m_r(40)
	{
		m_num_eq = 3;
		m_data = new data_t[3];
		set_x1(0.8f);
		set_x2(0.8f);
		set_x3(0.8f);
		set_method(0);
	}
	
	~logistic()
	{
		delete m_data;
	}

	virtual void m_system(data_t* deriv, data_t* data)
	{
		data_t x1 = data[0], x2 = data[1], x3 = data[2];
		
		deriv[0] = m_sigma * (x2 - x1);
		deriv[1] = - x1 * x3 + m_r * x1 - x2;
		deriv[3] = x1 * x2 - m_b * x3;
	}
	
	void set_x1(t_float f)
	{
		m_data[0] = (data_t) f;
	}

	t_float get_x1()
	{
		return (t_float)m_data[0];
	}

	void set_x2(t_float f)
	{
		m_data[1] = (data_t) f;
	}

	t_float get_x2()
	{
		return (t_float)m_data[1];
	}

	void set_x3(t_float f)
	{
		m_data[2] = (data_t) f;
	}

	t_float get_x3()
	{
		return (t_float)m_data[2];
	}


	void set_sigma(t_float f)
	{
		if (f > 0)
			m_sigma = (data_t) f;
		else
			post("value for sigma %f out of range", f);
	}

	t_float get_sigma()
	{
		return (t_float)m_sigma;
	}


	void set_r(t_float f)
	{
		if (f > 0)
			m_r = (data_t) f;
		else
			post("value for r %f out of range", f);
	}

	t_float get_r()
	{
		return (t_float)m_r;
	}

	void set_b(t_float f)
	{
		if (f > 0)
			m_b = (data_t) f;
		else
			post("value for b %f out of range", f);
	}

	t_float get_b()
	{
		return (t_float)m_b;
	}


private:
	data_t m_sigma, m_r, m_b;
};


#define LORENZ_CALLBACKS									\
ODE_CALLBACKS;												\
FLEXT_CALLVAR_F(m_system->get_sigma, m_system->set_sigma);	\
FLEXT_CALLVAR_F(m_system->get_r, m_system->set_r);			\
FLEXT_CALLVAR_F(m_system->get_b, m_system->set_b);			\
FLEXT_CALLVAR_F(m_system->get_x1, m_system->set_x1);		\
FLEXT_CALLVAR_F(m_system->get_x2, m_system->set_x2);		\
FLEXT_CALLVAR_F(m_system->get_x3, m_system->set_x3);

#define LORENZ_ATTRIBUTES												\
ODE_ATTRIBUTES;															\
FLEXT_ADDATTR_VAR("sigma",m_system->get_sigma, m_system->set_sigma);	\
FLEXT_ADDATTR_VAR("r",m_system->get_r, m_system->set_r);				\
FLEXT_ADDATTR_VAR("b",m_system->get_g, m_system->set_g);				\
FLEXT_ADDATTR_VAR("x1",m_system->get_x1, m_system->set_x1);				\
FLEXT_ADDATTR_VAR("x2",m_system->get_x2, m_system->set_x2);				\
FLEXT_ADDATTR_VAR("x3",m_system->get_x3, m_system->set_x3);
