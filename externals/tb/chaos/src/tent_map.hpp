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

//  tent map: x[n+1] = 2 * x[n]       (for 0 < x <= 0.5)
//                     2 * (1 - x[n]) (else)
//            0 <= x[n] <  1
//  taken from Willi-Hans Steeb: Chaos and Fractals

class tent:
	protected map_base
{
public:
	tent()
	{
		m_num_eq = 1;
		m_data = new data_t[1];
		set_x(0.5);
	}

	~tent()
	{
		delete m_data;
	}

	virtual void m_step()
	{
		data_t data = m_data[0];

		if (data < 0.5f)
			m_data[0] = 2.f * data;
		else
			m_data[0] = 2.f * (1.f - data);
	}
};


#define LOGISTIC_CALLBACKS							\
MAP_CALLBACKS										\
FLEXT_CALLVAR_F(m_system->get_x, m_system->set_x);

#define LOGISTIC_ATTRIBUTES									\
MAP_ATTRIBUTES												\
FLEXT_ADDATTR_VAR("x",m_system->get_x, m_system->set_x);

