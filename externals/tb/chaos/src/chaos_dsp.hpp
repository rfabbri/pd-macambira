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

class chaos_dsp
	: public flext_dsp
{
	FLEXT_HEADER(chaos_dsp, flext_dsp);

protected:

	/* signal functions: */
	/* for frequency = sr/2 */
	void m_signal_(int n, t_sample *const *insigs,t_sample *const *outsigs);
	/* sample & hold */
	void m_signal_n(int n, t_sample *const *insigs,t_sample *const *outsigs);
	/* linear interpolation */
	void m_signal_l(int n, t_sample *const *insigs,t_sample *const *outsigs);
	/* cubic interpolatio */
	void m_signal_c(int n, t_sample *const *insigs,t_sample *const *outsigs);
	
	virtual void m_signal(int n, t_sample *const *insigs,t_sample *const *outsigs);
	virtual void m_dsp(int n, t_sample *const *insigs,t_sample *const *outsigs);


	/* local data for system, output and interpolation */
	chaos_base * m_system; /* the system */

	t_sample * m_values;   /* actual value */
	t_sample * m_slopes;   /* actual slope for cubic interpolation */

    t_sample * m_nextvalues;
    t_sample * m_nextmidpts;
    t_sample * m_curves;
	
	/* local data for signal functions */
	float m_freq;        /* frequency of oscillations */
	int m_phase;         /* phase counter */
	float m_sr;          /* sample rate */
	
	char m_method;       /* interpolation method */
	
};
