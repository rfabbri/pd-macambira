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
#include "pdp_mmx.h"
#include <math.h>


typedef struct pdp_chrot_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;

    int x_packet0;
    int x_queue_id;
    int x_dropped;          // indicate if a packet was dropped during register_rw cycle

    int x_channel;

    float x_matrix[4];

    void *x_crot2d;

} t_pdp_chrot;



static void pdp_chrot_process_yv12(t_pdp_chrot *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    void  *data0   = pdp_packet_data  (x->x_packet0);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    unsigned int size = w*h;
    unsigned int v_offset = size;

    short int * idata = (short int *)data0;

    
    /* color rotation for 2 colour planes */
    pdp_imageproc_crot2d_process(x->x_crot2d, idata + v_offset, w>>1, h>>1);


    return;


}

static void pdp_chrot_process(t_pdp_chrot *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);

    /* check data packets */

    if ((header0) && (PDP_IMAGE == header0->type)){

	/* pdp_chrot_process inputs and write into active inlet */
	switch(header0->info.image.encoding){

	case PDP_IMAGE_YV12:
	    pdp_chrot_process_yv12(x);
	    break;

	case PDP_IMAGE_GREY:
	    break;

	default:
	    break;
	    /* don't know the type, so dont pdp_chrot_process */
	    
	}
    }

}

static void pdp_chrot_sendpacket(t_pdp_chrot *x)
{
    /* unregister and propagate if valid packet */
    pdp_pass_if_valid(x->x_outlet0, &x->x_packet0);
}

static void pdp_chrot_input_0(t_pdp_chrot *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;

    if (s== gensym("register_rw"))  x->x_dropped = pdp_packet_copy_rw_or_drop(&x->x_packet0, p);


    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped)){


	/* add the process method and callback to the process queue */

	pdp_queue_add(x, pdp_chrot_process, pdp_chrot_sendpacket, &x->x_queue_id);
    }

}


static void pdp_chrot_setelement(t_pdp_chrot *x, int element, float f)
{
    x->x_matrix[element] = f;
	
}

static void pdp_chrot_angle_radians(t_pdp_chrot *x, t_floatarg angle)
{
    float c = cos(angle);
    float s = sin(angle);

    pdp_chrot_setelement(x, 0, c);
    pdp_chrot_setelement(x, 1, s);
    pdp_chrot_setelement(x, 2, -s);
    pdp_chrot_setelement(x, 3, c);

    pdp_imageproc_crot2d_setmatrix(x->x_crot2d, x->x_matrix);
}

static void pdp_chrot_angle_degrees(t_pdp_chrot *x, t_floatarg angle)
{
    pdp_chrot_angle_radians(x, (angle * (M_PI / 180.f)));

}

static void pdp_chrot_free(t_pdp_chrot *x)
{
    pdp_queue_finish(x->x_queue_id);
    pdp_imageproc_crot2d_delete(x->x_crot2d);
    pdp_packet_mark_unused(x->x_packet0);

}

t_class *pdp_chrot_class;



void *pdp_chrot_new(t_floatarg f)
{
    t_pdp_chrot *x = (t_pdp_chrot *)pd_new(pdp_chrot_class);

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("angle"));

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;
    x->x_queue_id = -1;

    x->x_crot2d = pdp_imageproc_crot2d_new();
    pdp_chrot_angle_radians(x, 0.0f);


    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_chrot_setup(void)
{


    pdp_chrot_class = class_new(gensym("pdp_chrot"), (t_newmethod)pdp_chrot_new,
    	(t_method)pdp_chrot_free, sizeof(t_pdp_chrot), 0, A_DEFFLOAT, A_NULL);


    class_addmethod(pdp_chrot_class, (t_method)pdp_chrot_angle_degrees, gensym("angle"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_chrot_class, (t_method)pdp_chrot_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
