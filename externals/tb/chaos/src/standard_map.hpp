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

//  standard map: I[n+1] = I[n] + k * sin(theta[n])
//                theta[n+1] = theta[n] + I[n] + k * sin(theta[n])
//                0 <= theta <= 2*pi
//  taken from Willi-Hans Steeb: Chaos and Fractals

class standard:
	protected map_base
{
public:
	standard()
		: m_k(0.8)
	{
		m_num_eq = 2;
		m_data = new data_t[2];
		set_I(0.1);
		set_theta(0.2);
	}

	~standard()
	{
		delete m_data;
	}

	virtual void m_step()
	{
		data_t I = m_data[0];
		data_t theta = m_data[1];
		
		m_data[0] = I + m_k * sin(theta);
		theta = theta + I + k * sin(theta);

		if (y > 2 * M_PI)
		{
			do
			{
				y -= 2*M_PI;
			}
			while (y > 2 * M_PI);
			goto put_data;
		}
		
		if (y < 0)
		{
			do
			{
				y += 2*M_PI;
			}
			while (y < 0);
		}
		
	put_data:
		m_data[1] = theta;
	}


	void set_I(t_float f)
	{
		m_data[0] = (data_t) f;
	}

	t_float get_I()
	{
		return (t_float)m_data[0];
	}


	void set_theta(t_float f)
	{
		if ( (f >= 0) && (f < 2*M_PI))
			m_data[1] = (data_t) f;
		else
			post("value for theta %f out of range", f);
	}

	t_float get_theta()
	{
		return (t_float)m_data[1];
	}


	void set_k(t_float f)
	{
		m_k = (data_t) f;
	}

	t_float get_k()
	{
		return (t_float)m_k;
	}

private:	
	data_t m_k;
};


#define STANDARD_CALLBACKS									\
MAP_CALLBACKS;												\
FLEXT_CALLVAR_F(m_system->get_I, m_system->set_I);			\
FLEXT_CALLVAR_F(m_system->get_theta, m_system->set_theta);	\
FLEXT_CALLVAR_F(m_system->get_k, m_system->set_k);


#define STANDARD_ATTRIBUTES												\
MAP_ATTRIBUTES;															\
FLEXT_ADDATTR_VAR("I",m_system->get_I, m_system->set_I);				\
FLEXT_ADDATTR_VAR("theta",m_system->get_theta, m_system->set_theta);	\
FLEXT_ADDATTR_VAR("k",m_system->get_k, m_system->set_k);
