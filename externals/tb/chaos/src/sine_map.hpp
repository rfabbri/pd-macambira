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

//  sine map: x[n+1] = sin(pi * x)
//                     0 <= x[n] <=  1
//  taken from Willi-Hans Steeb: Chaos and Fractals

class sine:
	protected map_base
{
	
public:
	sine()
	{
		m_num_eq = 1;
		m_data = new data_t[1];
		set_x(0.5);
	}

	~sine()
	{
		delete m_data;
	}

	virtual void m_step()
	{
		m_data[0] = sin (M_PI * m_data[0]);
	}

	void set_x(t_float f)
	{
		if ( (f >= 0) && (f <= 1))
			m_data[0] = (data_t) f;
		else
			post("value for x %f out of range", f);
	}

	t_float get_x()
	{
		return (t_float)m_data[0];
	}

};

#define SINE_CALLBACKS								\
MAP_CALLBACKS										\
FLEXT_CALLVAR_F(m_system->get_x, m_system->set_x);

#define SINE_ATTRIBUTES										\
MAP_ATTRIBUTES												\
FLEXT_ADDATTR_VAR("x",m_system->get_x, m_system->set_x);

