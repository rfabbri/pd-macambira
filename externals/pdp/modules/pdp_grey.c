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


typedef struct pdp_grey_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;


    int x_packet0;

} t_pdp_grey;



static void pdp_grey_process(t_pdp_grey *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    t_pdp *headernew;
    int newpacket;
    int w;
    int h;

    /* check data packets */

    if ((header0) && (PDP_IMAGE == header0->type)){

	/* pdp_grey_process inputs and write into active inlet */
	switch(header0->info.image.encoding){

	case PDP_IMAGE_YV12:
	    w = header0->info.image.width;
	    h = header0->info.image.height;
	    newpacket = pdp_packet_new(PDP_IMAGE, w*h*2);
	    headernew = pdp_packet_header(newpacket);
	    headernew->info.image.encoding = PDP_IMAGE_GREY;
	    headernew->info.image.width = w;
	    headernew->info.image.height = h;
	    memcpy(pdp_packet_data(newpacket), pdp_packet_data(x->x_packet0), 2*w*h);
	    pdp_packet_mark_unused(x->x_packet0);
	    x->x_packet0 = newpacket;
	    break;

	case PDP_IMAGE_GREY:
	    /* just pass along */
	    break;

	default:
	    break;
	    /* don't know the type, so dont process */
	    
	}
    }

}


static void pdp_grey_sendpacket(t_pdp_grey *x)
{
    /* unregister and propagate if valid packet */
    pdp_pass_if_valid(x->x_outlet0, &x->x_packet0);
}

static void pdp_grey_input_0(t_pdp_grey *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;
    int passes, i;

    if (s== gensym("register_rw"))  pdp_packet_copy_rw_or_drop(&x->x_packet0, p);


    if ((s == gensym("process")) && (-1 != x->x_packet0)){

	pdp_grey_process(x);
	pdp_grey_sendpacket(x);
    }

}




short int pdp_grey_fixedpoint(float f)
{

  float min = (float)-0x7fff;
  float max = (float)0x7fff;
  f *= (float)0x100;  
  if (f>max) f = max;
  if (f<min) f = min;

  return (short int)f;
  
}


t_class *pdp_grey_class;



void pdp_grey_free(t_pdp_grey *x)
{
}

void *pdp_grey_new(t_floatarg f)
{
    t_pdp_grey *x = (t_pdp_grey *)pd_new(pdp_grey_class);


    /* no arg, or zero -> gain = 1 */
    if (f==0.0f) f = 1.0f;

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("gain"));

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_grey_setup(void)
{


    pdp_grey_class = class_new(gensym("pdp_grey"), (t_newmethod)pdp_grey_new,
    	(t_method)pdp_grey_free, sizeof(t_pdp_grey), 0, A_DEFFLOAT, A_NULL);


    class_addmethod(pdp_grey_class, (t_method)pdp_grey_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
