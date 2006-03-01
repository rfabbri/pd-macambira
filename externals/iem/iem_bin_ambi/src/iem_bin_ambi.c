/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_bin_ambi written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2003 */

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


#include "m_pd.h"
#include "iemlib.h"

static t_class *iem_bin_ambi_class;

static void *iem_bin_ambi_new(void)
{
	t_object *x = (t_object *)pd_new(iem_bin_ambi_class);
    
	return (x);
}

void bin_ambi_calc_HRTF_setup(void);
void bin_ambi_reduced_decode_setup(void);

/* ------------------------ setup routine ------------------------- */

void iem_bin_ambi_setup(void)
{
	bin_ambi_calc_HRTF_setup();
	bin_ambi_reduced_decode_setup();

	post("iem_bin_ambi (R-1.15) library loaded!");
}
