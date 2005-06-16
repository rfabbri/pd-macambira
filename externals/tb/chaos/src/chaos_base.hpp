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

#ifndef __chaos_base_hpp

#include "chaos.hpp"
#include <map>

#define MAXDIMENSION 5 // this should be enough for the first 

class chaos_base
{

public:
	inline t_sample get_data(unsigned int i)
	{
		return (t_sample)m_data[i]; /* this is not save, but fast */
	}

	inline int get_num_eq()
	{
		return m_num_eq;
	}

	inline void m_perform()
	{
		m_step();
		m_bash_denormals();
		m_verify();
	}

 	std::map<const t_symbol*,int> attr_ind;
	//	TableAnyMap attr_ind; /* thomas fragen :-) */
	
	// check the integrity of the system
	virtual void m_verify() 
	{
	}
	
	inline void m_bash_denormals()
	{
		for (int i = 0; i != get_num_eq(); ++i)
		{
#ifndef DOUBLE_PRECISION
			if (PD_BIGORSMALL(m_data[i]))
				m_data[i] = 0;
#endif
		}
	};
	
	data_t m_data[MAXDIMENSION];  // state of the system

protected:
	virtual void m_step() = 0;    // iteration
	int m_num_eq;                 // number of equations of the system
 	flext::AtomList Parameter;    // parameter
 	flext::AtomList System;       // system
};

#define CHAOS_PRECONSTRUCTOR					\
    /* dummy */	

#define CHAOS_POSTCONSTRUCTOR					\
m_num_eq = System.Count();

#define CHAOS_DESTRUCTOR						\
    


#define CHAOS_CALLBACKS							\
public:											\
void get_dimension(int &i)						\
{												\
	i = m_system->get_num_eq();					\
}												\
FLEXT_CALLGET_I(get_dimension);


#define CHAOS_ATTRIBUTES						\
FLEXT_ADDATTR_GET("dimension",get_dimension);



#define __chaos_base_hpp
#endif /* __chaos_base_hpp */
