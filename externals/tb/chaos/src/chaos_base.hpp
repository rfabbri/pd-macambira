a// 
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

class chaos_base
{

public:
	t_sample get_data(unsigned int i)
	{
		return (t_sample)m_data[i]; /* this is not save, but fast */
	}

	int get_num_eq()
	{
		return m_num_eq;
	}

	virtual void m_step();

	data_t * m_data;       // state of the system

protected:
	int m_num_eq;  // number of equations of the system
};

#define CHAOS_CALLBACKS							\
FLEXT_CALLGET_F(m_system->get_num_eq);

#define CHAOS_ATTRIBUTES								\
FLEXT_ADDATTR_GET("dimension",m_system->get_num_eq);



// macros for simplified system state functions
#define CHAOS_SYS_SETFUNC(NAME, NR)				\
	void set_##NAME(t_float f)					\
	{											\
		m_data[NR] = (data_t) f;				\
	}

#define CHAOS_SYS_SETFUNC_PRED(NAME, NR, PRED)							\
	void set_##NAME(t_float f)											\
	{																	\
		if ( PRED(f) )													\
			m_data[NR] = (data_t) f;									\
		else															\
			post("value for dimension " #NAME " %f out of range", f);	\
	}

#define CHAOS_SYS_GETFUNC(NAME, NR)				\
	t_float get_##NAME()						\
	{											\
		return (t_float)m_data[NR];				\
	}

/* to be called in the public part */			
#define CHAOS_SYSVAR_FUNCS_PRED(NAME, NR, PRED)	\
public:											\
CHAOS_SYS_SETFUNC_PRED(NAME, NR, PRED)			\
CHAOS_SYS_GETFUNC(NAME, NR)

#define CHAOS_SYSVAR_FUNCS(NAME, NR)			\
public:											\
CHAOS_SYS_SETFUNC(NAME, NR)						\
CHAOS_SYS_GETFUNC(NAME, NR)



// macros for simplified system parameter functions
#define CHAOS_PAR_SETFUNC(NAME)					\
	void set_##NAME(t_float f)					\
	{											\
		m_##NAME = (data_t) f;					\
	}

#define CHAOS_PAR_SETFUNC_PRED(NAME, PRED)								\
	void set_##NAME(t_float f)											\
	{																	\
		if ( PRED(f) )									\
			m_##NAME = (data_t) f;										\
		else															\
			post("value for parameter " #NAME " %f out of range", f);	\
	}

#define CHAOS_PAR_GETFUNC(NAME)					\
	t_float get_##NAME()						\
	{											\
		return (t_float)m_##NAME;				\
	}


#define CHAOS_SYSPAR_FUNCS_PRED(NAME, PRED)	\
public:											\
CHAOS_PAR_SETFUNC_PRED(NAME, PRED)			\
CHAOS_PAR_GETFUNC(NAME)							\
private:										\
data_t m_##NAME;								\
public:

#define CHAOS_SYSPAR_FUNCS(NAME)				\
public:											\
CHAOS_PAR_SETFUNC(NAME)							\
CHAOS_PAR_GETFUNC(NAME)							\
private:										\
data_t m_##NAME;								\
public:


#define CHAOS_SYS_CALLBACKS(NAME)								\
FLEXT_CALLVAR_F(m_system->get_##NAME, m_system->set_##NAME);

#define CHAOS_SYS_ATTRIBUTE(NAME)										\
FLEXT_ADDATTR_VAR(#NAME,m_system->get_##NAME, m_system->set_##NAME);

#define CHAOS_SYS_INIT(NAME, VALUE)				\
set_##NAME(VALUE);

#define CHAOS_PARAMETER(NAME) m_##NAME


#define __chaos_base_hpp
#endif /* __chaos_base_hpp */
