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

void ode_base::rk1()
{
	m_system (m_k[0], m_data);
	for (int i = 0; i != m_num_eq; ++i)
		m_data[i] += m_dt * m_k[0][i];
}


void ode_base::rk2()
{
	m_system (m_k[0], m_data);
	for (int i = 0; i != m_num_eq; ++i)
		m_k[0][i] = m_k[0][i] * 0.5 * m_dt + m_data[i];
	
	m_system (m_k[1], m_k[0]);
	for (int i = 0; i != m_num_eq; ++i)
		m_data[i] += m_dt * m_k[1][i];
}


void ode_base::rk4()
{
	m_system (m_k[0], m_data);
	for (int i = 0; i != m_num_eq; ++i)
	{
		m_k[0][i] *= m_dt;
		m_tmp[i] = m_data[i] + 0.5 * m_k[0][i];
	}

	m_system (m_k[1], m_tmp);
	for (int i = 0; i != m_num_eq; ++i)
	{
		m_k[1][i] *= m_dt;
		m_tmp[i] = m_data[i] + 0.5 * m_k[1][i];
	}
	
	m_system (m_k[2], m_tmp);
	for (int i = 0; i != m_num_eq; ++i)
	{
		m_k[2][i] *= m_dt;
		m_tmp[i] = m_data[i] + m_k[2][i];
	}

	m_system (m_k[3], m_tmp);
	for (int i = 0; i != m_num_eq; ++i)
		m_k[3][i] *= m_dt;

	for (int i = 0; i != m_num_eq; ++i)
		m_data[i] += (m_k[0][i] + 2. * (m_k[1][i] + m_k[2][i]) + m_k[3][i]) 
			/ 6.;
}

