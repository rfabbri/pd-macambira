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

//  circle_map map: x[n+1] = x[n] * omega - r / (2*pi) * sin (2 * pi * x [n])
// 
//  taken from Willi-Hans Steeb: Chaos and Fractals

class circle_map:
	protected map_base
{
public:
	circle_map()
		: m_omega(0.4), m_r(1)
	{
		m_num_eq = 1;
		m_data = new data_t[1];
		set_x(0.5);
	}

	~circle_map()
	{
		delete m_data;
	}

	virtual void m_step()
	{
		data_t x = m_data[0];
		data_t omega = m_omega;
		data_t r = m_r;
		
		m_data[0] = x + omega - r / (2.f * M_PI) * sin (2.f * M_PI * x);
	}

	void set_x(t_float f)
	{
		m_data[0] = (data_t) f;
	}

	t_float get_x()
	{
		return (t_float)m_data[0];
	}

	void set_r(t_float f)
	{
		m_r = (data_t) f;
	}


	t_float get_r()
	{
		return (t_float)m_r;
	}


	void set_omega (t_float f)
	{
		m_omega = (data_t) f;
	}

	t_float get_omega()
	{
		return (t_float)m_omega;
	}
	

private:
	data_t m_omega;
	data_t m_r;
};

#define CIRCLE_MAP_CALLBACKS								\
MAP_CALLBACKS;												\
FLEXT_ATTRVAR_F(m_system->m_omega);							\
FLEXT_ATTRVAR_F(m_system->m_r);								\
FLEXT_CALLVAR_F(m_system->get_omega, m_system->set_omega);	\
FLEXT_CALLVAR_F(m_system->get_r, m_system->set_r);			\
FLEXT_CALLVAR_F(m_system->get_x, m_system->set_x);

#define CIRCLE_MAP_ATTRIBUTES											\
MAP_ATTRIBUTES;															\
FLEXT_ADDATTR_VAR1("omega",m_system->m_omega);							\
FLEXT_ADDATTR_VAR1("r",m_system->m_r);									\
FLEXT_ADDATTR_VAR("omega",m_system->get_omega, m_system->set_omega);	\
FLEXT_ADDATTR_VAR("r",m_system->get_r, m_system->set_r);				\
FLEXT_ADDATTR_VAR("x",m_system->get_x, m_system->set_x);

