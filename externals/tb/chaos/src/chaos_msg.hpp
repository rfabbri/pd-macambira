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

#include "chaos_base.hpp"

template <class system> class chaos_msg
	: public flext_base
{
	FLEXT_HEADER(chaos_msg, flext_base);

public:

	/* local data for system, output and interpolation */
	system * m_system; /* the system */

	void m_bang()
	{
		m_system->m_perform();
		int outlets = m_system->get_num_eq();

		while (outlets--)
		{
			ToOutFloat(outlets, m_system->get_data(outlets));
		}
	}

	FLEXT_CALLBACK(m_bang);
};


/* create constructor / destructor */
#define CHAOS_MSG_INIT(SYSTEM, ATTRIBUTES)		\
FLEXT_HEADER(SYSTEM##_msg, chaos_msg<SYSTEM>)	\
												\
SYSTEM##_msg(int argc, t_atom* argv )			\
{												\
	m_system = new SYSTEM;						\
												\
	int size = m_system->get_num_eq();			\
												\
    for (int i = 0; i != size; ++i)				\
        AddOutFloat();							\
												\
    ATTRIBUTES;									\
    FLEXT_ADDBANG(0, m_bang);              	\
}												\
												\
~SYSTEM##_msg()									\
{												\
	delete m_system;							\
}
