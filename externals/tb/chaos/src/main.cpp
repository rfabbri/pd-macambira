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

#include "chaos.hpp"

void chaos_library_setup()
{
	post("chaos~ version "PACKAGE_VERSION"\n");

	FLEXT_DSP_SETUP(bernoulli_dsp);
	FLEXT_SETUP(bernoulli_msg);

	FLEXT_DSP_SETUP(bungalow_tent_dsp);
	FLEXT_SETUP(bungalow_tent_msg);

	FLEXT_DSP_SETUP(circle_map_dsp);
	FLEXT_SETUP(circle_map_msg);

	FLEXT_DSP_SETUP(gauss_map_dsp);
	FLEXT_SETUP(gauss_map_msg);

	FLEXT_DSP_SETUP(henon_dsp);
	FLEXT_SETUP(henon_msg);

	FLEXT_DSP_SETUP(ikeda_laser_map_dsp);
	FLEXT_SETUP(ikeda_laser_map_msg);
	
	FLEXT_DSP_SETUP(logistic_dsp);
	FLEXT_SETUP(logistic_msg);

	FLEXT_DSP_SETUP(lorenz_dsp);
	FLEXT_SETUP(lorenz_msg);

	FLEXT_DSP_SETUP(lozi_map_dsp);
	FLEXT_SETUP(lozi_map_msg);

	FLEXT_DSP_SETUP(sine_map_dsp);
	FLEXT_SETUP(sine_map_msg);

	FLEXT_DSP_SETUP(standard_map_dsp);
	FLEXT_SETUP(standard_map_msg);

	FLEXT_DSP_SETUP(tent_map_dsp);
	FLEXT_SETUP(tent_map_msg);
}

FLEXT_LIB_SETUP(chaos, chaos_library_setup);
