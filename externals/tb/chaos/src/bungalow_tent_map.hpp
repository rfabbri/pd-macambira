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

//  bungalow_tent map: x[n+1] = 1 + 2 * r + 2 * (r + 1) * x[n] 
//                                                (for -1 <= x[n] < -0.5f)
//                              1 + 2 * (1 - r) * (x[n])
//                                                (for -0.5 <= x[n] < 0)
//                              1 + 2 * (r - 1) * (x[n])
//                                                (for 0 <= x[n] < 0.5)
//                              1 + 2 * r - 2 * (r + 1) * (x[n])
//                                                (for 0.5 <= x[n] < 1)
//                              -1 <= x[n] <  1
//                              -0.5 <= r < 1
//  taken from Willi-Hans Steeb: Chaos and Fractals

class bungalow_tent:
	protected map_base
{
public:
	bungalow_tent()
		: m_r(0.5)
	{
		m_num_eq = 1;
		m_data = new data_t[1];
		set_x(0.5f);
	}

	~bungalow_tent()
	{
		delete m_data;
	}

	virtual void m_step()
	{
		data_t x = m_data[0];
		data_t r = m_r;
		
		if ( x < - 0.5)
		{
			m_data[0] = 1 + 2 * r + 2 * (r + 1) * x;
			return;
		}
		if ( x < 0)
		{
			m_data[0] = 1 + 2 * (1 - r) * x;
			return;
		}
		if ( x < 0.5)
		{
			m_data[0] = 1 + 2 * (r - 1) * x;
			return;
		}
		else
		{
			m_data[0] = 1 + 2 * r - 2 * (r + 1) * x;
			return;
		}

	}

	void set_x(t_float f)
	{
		if ( (f > -1) && (f < 1))
			m_data[0] = (data_t) f;
		else
			post("value for x %f out of range", f);
	}

	t_float get_x()
	{
		return (t_float)m_data[0];
	}

	void set_r(t_float f)
	{
		if ( (f > -.5) && (f < 1))
			m_data[0] = (data_t) f;
		else
			post("value for r %f out of range", f);
	}

	t_float get_r()
	{
		return (t_float)m_data[0];
	}

private:
	data_t m_r;

};


#define BUNGALOW_TENT_CALLBACKS						\
MAP_CALLBACKS;										\
FLEXT_CALLVAR_F(m_system->get_r, m_system->set_r);	\
FLEXT_CALLVAR_F(m_system->get_x, m_system->set_x);	\


#define BUNGALOW_TENT_ATTRIBUTES							\
MAP_ATTRIBUTES;												\
FLEXT_ADDATTR_VAR("r",m_system->get_r, m_system->set_r);	\
FLEXT_ADDATTR_VAR("x",m_system->get_x, m_system->set_x);
