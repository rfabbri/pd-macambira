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

template <class system> class chaos_dsp
	: public flext_dsp
{
	FLEXT_HEADER(chaos_dsp, flext_dsp);

public:

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

	virtual void m_dsp(int n, t_sample *const *insigs,t_sample *const *outsigs)
	{
		m_sr = Samplerate();
	}
	
	
	/* local data for system, output and interpolation */
	system * m_system; /* the system */

	t_sample * m_values;   /* actual value */
	t_sample * m_slopes;   /* actual slope for cubic interpolation */

    t_sample * m_nextvalues;
    t_sample * m_nextmidpts;
    t_sample * m_curves;
	
	/* local data for signal functions */
	float m_freq;        /* frequency of oscillations */
	int m_phase;         /* phase counter */
	float m_sr;          /* sample rate */
	
	int m_method;       /* interpolation method */
	
};


/* create constructor / destructor */
#define CHAOS_DSP_INIT(SYSTEM, ATTRIBUTES)		\
FLEXT_HEADER(SYSTEM##_dsp, chaos_dsp<SYSTEM>)	\
												\
SYSTEM##_dsp(int argc, t_atom* argv )			\
{												\
	m_system = new SYSTEM;						\
												\
	int size = m_system->get_num_eq();			\
												\
	m_values = new t_float[size];				\
	m_slopes = new t_float[size];				\
	m_nextvalues = new t_float[size];			\
	m_nextmidpts = new t_float[size];			\
	m_curves = new t_float[size];				\
												\
    /* create inlets and zero arrays*/ 			\
    for (int i = 0; i != size; ++i)				\
	{											\
		AddOutSignal();							\
		m_values[i] = 0;						\
		m_slopes[i] = 0;						\
		m_nextvalues[i] = 0;					\
		m_nextmidpts[i] = 0;					\
		m_curves[i] = 0;						\
	}											\
												\
												\
												\
    m_freq = GetAFloat(argv[0]);				\
	m_method = (char)GetAFloat(argv[1]);		\
    m_phase = 0;								\
												\
    FLEXT_ADDATTR_VAR1("frequency",m_freq);		\
    FLEXT_ADDATTR_VAR1("method",m_method);		\
												\
    ATTRIBUTES;									\
}												\
												\
~SYSTEM##_dsp()									\
{												\
	delete m_system;							\
	delete m_values;							\
	delete m_slopes;							\
	delete m_nextvalues;						\
	delete m_nextmidpts;						\
	delete m_curves;							\
}												\
												\
FLEXT_ATTRVAR_F(m_freq);						\
FLEXT_ATTRVAR_I(m_method);



template <class system> 
void chaos_dsp<system>::m_signal(int n, t_sample *const *insigs,
								 t_sample *const *outsigs)
{
	if (m_freq >= m_sr * 0.5)
	{
		m_signal_(n, insigs, outsigs);
		return;
	}

	switch (m_method)
	{
	case 0:
		m_signal_n(n, insigs, outsigs);
		return;
	case 1:
		m_signal_l(n, insigs, outsigs);
		return;
	case 2:
		m_signal_c(n, insigs, outsigs);
		return;
	}
}

template <class system> 
void chaos_dsp<system>::m_signal_(int n, t_sample *const *insigs,
								 t_sample *const *outsigs)
{
	int outlets = m_system->get_num_eq();

	for (int i = 0; i!=n; ++i)
	{
		m_system->m_step();
		for (int j = 0; j != outlets; ++j)
		{
			outsigs[j][i] = m_system->get_data(j);
		}
	}
	
}

template <class system> 
void chaos_dsp<system>::m_signal_n(int n, t_sample *const *insigs,
								   t_sample *const *outsigs)
{
	int outlets = m_system->get_num_eq();
	
	int phase = m_phase;

	int i = 0;

	while (n)
	{
		if (phase == 0)
		{
			m_system->m_step();
			phase = int (m_sr / m_freq);
		}
		
		int next = (phase < n) ? phase : n;
		n -= next;
		phase -=next;
		
		while (next--)
		{
			for (int j = 0; j != outlets; ++j)
			{
				outsigs[j][i] = m_system->get_data(j);
			}
			++i;
		}
	}
	m_phase = phase;
}


/* linear and cubic interpolation adapted from supercollider by James McCartney */
template <class system> 
void chaos_dsp<system>::m_signal_l(int n, t_sample *const *insigs,
								   t_sample *const *outsigs)
{
	int outlets = m_system->get_num_eq();
	
	int phase = m_phase;

	int i = 0;

	while (n)
	{
		if (phase == 0)
		{
			m_system->m_step();
			phase = int (m_sr / m_freq);

			for (int j = 0; j != outlets; ++j)
				m_slopes[j] = (m_system->get_data(j) - m_values[j]) / phase;
		}
		
		int next = (phase < n) ? phase : n;
		n -= next;
		phase -=next;
		
		while (next--)
		{
			for (int j = 0; j != outlets; ++j)
			{
				outsigs[j][i] = m_values[j];
				m_values[j]+=m_slopes[j];
			}
			++i;
		}
	}
	m_phase = phase;
}


template <class system> 
void chaos_dsp<system>::m_signal_c(int n, t_sample *const *insigs,
								   t_sample *const *outsigs)
{
	int outlets = m_system->get_num_eq();
	
	int phase = m_phase;

	int i = 0;

	while (n)
	{
		if (phase == 0)
		{
			m_system->m_step();
			phase = int (m_sr / m_freq);
			phase = (phase > 2) ? phase : 2;
			
			for (int j = 0; j != outlets; ++j)
			{
				t_sample value = m_nextvalues[j];
				m_nextvalues[j]= m_system->get_data(j);
				
				m_values[j] =  m_nextmidpts[j];
				m_nextmidpts[j] = (m_nextvalues[j] + value) * 0.5f;
				
				float fseglen = (float)phase;
				m_curves[j] = 2.f * (m_nextmidpts[j] - m_values[j] - 
									 fseglen * m_slopes[j]) 
					/ (fseglen * fseglen + fseglen);
			}
		}
		
		int next = (phase < n) ? phase : n;
		n -= next;
		phase -=next;
		
		while (next--)
		{
			for (int j = 0; j != outlets; ++j)
			{
				outsigs[j][i] = m_values[j];
				m_slopes[j]+=m_curves[j];
				m_values[j]+=m_slopes[j];
			}
			++i;
		}
	}
	m_phase = phase;
}
