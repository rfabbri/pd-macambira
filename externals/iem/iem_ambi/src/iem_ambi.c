/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_ambi written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2005 */

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


#include "m_pd.h"
#include "iemlib.h"

static t_class *iem_ambi_class;

static void *iem_ambi_new(void)
{
	t_object *x = (t_object *)pd_new(iem_ambi_class);
    
	return (x);
}

void ambi_encode_setup(void);
void ambi_decode_setup(void);
void ambi_decode2_setup(void);
void ambi_decode3_setup(void);
void ambi_decode_cube_setup(void);
void ambi_rot_setup(void);

/* ------------------------ setup routine ------------------------- */

void iem_ambi_setup(void)
{
	ambi_encode_setup();
	ambi_decode_setup();
	ambi_decode2_setup();
	ambi_decode3_setup();
	ambi_decode_cube_setup();
	ambi_rot_setup();

    post("iem_ambi (R-1.16) library loaded!   (c) Thomas Musil 05.2005");
	post("   musil%ciem.at iem KUG Graz Austria", '@');
}
