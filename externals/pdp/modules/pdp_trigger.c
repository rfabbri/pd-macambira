/*
 *   Pure Data Packet module.
 *   Copyright (c) by Tom Schouten <pdp@zzz.kotnet.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */



#include "pdp.h"





typedef struct pdp_trigger_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;
    t_outlet *x_outlet1;
    int x_packet;


} t_pdp_trigger;




static void pdp_trigger_input_0(t_pdp_trigger *x, t_symbol *s, t_floatarg f)
{
    t_atom atom[2];
    t_symbol *pdp = gensym("pdp");


    /* trigger on register_ro */
    if (s == gensym("register_ro")){
	outlet_bang(x->x_outlet1);

    }

    /* propagate the pdp message */
    SETSYMBOL(atom+0, s);
    SETFLOAT(atom+1, f);
    outlet_anything(x->x_outlet0, pdp, 2, atom);

}



static void pdp_trigger_free(t_pdp_trigger *x)
{

}

t_class *pdp_trigger_class;



void *pdp_trigger_new(void)
{
    t_pdp_trigger *x = (t_pdp_trigger *)pd_new(pdp_trigger_class);

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 
    x->x_outlet1 = outlet_new(&x->x_obj, &s_bang); 

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_trigger_setup(void)
{


    pdp_trigger_class = class_new(gensym("pdp_trigger"), (t_newmethod)pdp_trigger_new,
    	(t_method)pdp_trigger_free, sizeof(t_pdp_trigger), 0, A_NULL);
    
    class_addmethod(pdp_trigger_class, (t_method)pdp_trigger_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);


}

#ifdef __cplusplus
}
#endif
