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

//  lozi map: x[n+1] = y[n] + 1 - a * abs(x[n])
//            y[n+1] = b * x[n]
//            b != 0
//  taken from Willi-Hans Steeb: Chaos and Fractals

class lozi:
	protected map_base
{
public:
	lozi()
		: m_a(1.4), m_b(0.3)
	{
		m_num_eq = 2;
		m_data = new data_t[1];
		set_x(0.5);
		set_y(0.5);
	}

	~lozi()
	{
		delete m_data;
	}

	virtual void m_step()
	{
		data_t x = m_data[0];
		data_t y = m_data[1];
		
		if (x > 0)
			m_data[0] = 1 + y - m_a * x;
		else
			m_data[0] = 1 + y + m_a * x;
			
		m_data[1] = m_b * x;
		
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


	void set_a(t_float f)
	{
		m_a = (data_t) f;
	}

	t_float get_a()
	{
		return (t_float)m_a;
	}


	void set_b(t_float f)
	{
		if (f != 0)
			m_b = (data_t) f;
		else
			post("value for b %f out of range", f);
	}

	t_float get_b()
	{
		return (t_float)m_b;
	}


private:	
	data_t m_a;
	data_t m_b;
};


#define LOZI_CALLBACKS								\
MAP_CALLBACKS;										\
FLEXT_CALLVAR_F(m_system->get_a, m_system->set_a);	\
FLEXT_CALLVAR_F(m_system->get_b, m_system->set_b);	\
FLEXT_CALLVAR_F(m_system->get_x, m_system->set_x);	\
FLEXT_CALLVAR_F(m_system->get_y, m_system->set_y);

#define LOZI_ATTRIBUTES										\
MAP_ATTRIBUTES;												\
FLEXT_ADDATTR_VAR("a",m_system->get_a, m_system->set_a);	\
FLEXT_ADDATTR_VAR("b",m_system->get_b, m_system->set_b);	\
FLEXT_ADDATTR_VAR("x",m_system->get_x, m_system->set_x);	\
FLEXT_ADDATTR_VAR("y",m_system->get_y, m_system->set_y);
