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

#include "map_base.hpp"
#include <cmath>

//  ikeda laser map: z[n+1] = roh + c2 * z[n] * 
//                            exp (j * (c1 - c3 / (1 + abs(z) * abs(z))))
//                   z is complex
//
//            equal: x[n+1] = roh + c2 * (x[n] * cos(tau) - y[n] * sin (tau))
//                   y[n+1] = c2 * (x[n] * sin(tau) + y[n] * cos(tau))
//                   tau = c1 - (c2 / (1 + x*x + y*y))
//
//  taken from Willi-Hans Steeb: Chaos and Fractals

class ikeda:
	protected map_base
{
public:
	ikeda()
		: m_c1(0.4), m_c2(0.9), m_c3(9), m_roh(0.85)
	{
		m_num_eq = 2;
		m_data = new data_t[2];
		set_x(0.5);
		set_y(0.5);
	}

	~ikeda()
	{
		delete m_data;
	}

	virtual void m_step()
	{
		data_t x = m_data[0];
		data_t y = m_data[1];
		
		data_t tau = m_c1 - m_c3 / (1 + x*x + y*y);
		data_t cos_tau = cos(tau);
		data_t sin_tau = sin(tau);

		m_data[0] = m_roh + m_c2 * (x * cos_tau - y * sin_tau);
		m_data[1] = m_c2 * (x * sin_tau + y * cos_tau);
	}
	

	void set_x(t_float f)
	{
		m_data[0] = (data_t) f;
	}

	t_float get_x()
	{
		return (t_float)m_data[0];
	}

	void set_y(t_float f)
	{
		m_data[1] = (data_t) f;
	}

	t_float get_y()
	{
		return (t_float)m_data[1];
	}


	void set_c1(t_float f)
	{
		m_c1 = (data_t) f;
	}

	t_float get_c1()
	{
		return (t_float)m_c1;
	}


	void set_c2(t_float f)
	{
		m_c2[1] = (data_t) f;
	}

	t_float get_c2()
	{
		return (t_float)m_c2;
	}


	void set_c3(t_float f)
	{
		m_c3 = (data_t) f;
	}

	t_float get_c3()
	{
		return (t_float)m_c3;
	}


	void set_roh(t_float f)
	{
		m_roh = (data_t) f;
	}

	t_float get_roh()
	{
		return (t_float)m_roh;
	}

	
private:
	data_t m_c1, m_c2, m_c3, m_roh;
};


#define IKEDA_CALLBACKS									\
MAP_CALLBACKS;											\
FLEXT_CALLVAR_F(m_system->get_c1, m_system->set_c1);	\
FLEXT_CALLVAR_F(m_system->get_c2, m_system->set_c2);	\
FLEXT_CALLVAR_F(m_system->get_c3, m_system->set_c3);	\
FLEXT_CALLVAR_F(m_system->get_roh, m_system->set_roh);	\
FLEXT_CALLVAR_F(m_system->get_x, m_system->set_x);		\
FLEXT_CALLVAR_F(m_system->get_y, m_system->set_y);		\


#define IKEDA_ATTRIBUTES										\
MAP_ATTRIBUTES;													\
FLEXT_ADDATTR_VAR("c1",m_system->get_c1, m_system->set_c1);		\
FLEXT_ADDATTR_VAR("c2",m_system->get_c2, m_system->set_c2);		\
FLEXT_ADDATTR_VAR("c3",m_system->get_c3, m_system->set_c3);		\
FLEXT_ADDATTR_VAR("roh",m_system->get_roh, m_system->set_roh);	\
FLEXT_ADDATTR_VAR("x",m_system->get_x, m_system->set_x);		\
FLEXT_ADDATTR_VAR("y",m_system->get_y, m_system->set_y);
