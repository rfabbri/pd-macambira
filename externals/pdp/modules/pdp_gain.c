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



typedef struct pdp_gain_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;


    int x_packet0;
    int x_dropped;
    int x_queue_id;

    void *x_gain_y;
    void *x_gain_v;
    void *x_gain_u;

} t_pdp_gain;



static void pdp_gain_process_yv12(t_pdp_gain *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    void  *data0   = pdp_packet_data  (x->x_packet0);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    short int * idata = (short int *)data0;
    unsigned int size = w*h;

    pdp_imageproc_gain_process(x->x_gain_y, idata, w, h);
    pdp_imageproc_gain_process(x->x_gain_u, idata + size, w>>1, h>>1);
    pdp_imageproc_gain_process(x->x_gain_v, idata + size + (size>>2) , w>>1, h>>1);


    return;


}
static void pdp_gain_process_grey(t_pdp_gain *x)
{

    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    void  *data0   = pdp_packet_data  (x->x_packet0);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    unsigned int size = w*h;
    short int * idata = (short int *)data0;

    pdp_imageproc_gain_process(x->x_gain_y, idata, w, h);

    return;


}

static void pdp_gain_process(t_pdp_gain *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);

    /* check data packets */

    if ((header0) && (PDP_IMAGE == header0->type)){

	/* pdp_gain_process inputs and write into active inlet */
	switch(header0->info.image.encoding){

	case PDP_IMAGE_YV12:
	    pdp_gain_process_yv12(x);
	    break;

	case PDP_IMAGE_GREY:
	    pdp_gain_process_grey(x);
	    break;

	default:
	    break;
	    /* don't know the type, so dont process */
	    
	}
    }

}


static void pdp_gain_sendpacket(t_pdp_gain *x)
{
    /* unregister and propagate if valid packet */
    pdp_pass_if_valid(x->x_outlet0, &x->x_packet0);
}

static void pdp_gain_input_0(t_pdp_gain *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;
    int passes, i;

    if (s== gensym("register_rw"))  x->x_dropped = pdp_packet_copy_rw_or_drop(&x->x_packet0, p);


    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped)){

	/* add the process method and callback to the process queue */

	pdp_queue_add(x, pdp_gain_process, pdp_gain_sendpacket, &x->x_queue_id);
    }

}




static void pdp_gain_gainy(t_pdp_gain *x, t_floatarg f)
{
    pdp_imageproc_gain_setgain(x->x_gain_y, f);
}

static void pdp_gain_gainv(t_pdp_gain *x, t_floatarg f)
{
    pdp_imageproc_gain_setgain(x->x_gain_v, f);
}

static void pdp_gain_gainu(t_pdp_gain *x, t_floatarg f)
{
    pdp_imageproc_gain_setgain(x->x_gain_u, f);
}

static void pdp_gain_gain(t_pdp_gain *x, t_floatarg f)
{
    pdp_gain_gainy(x, f);
    pdp_gain_gainv(x, f);
    pdp_gain_gainu(x, f);

}



t_class *pdp_gain_class;



void pdp_gain_free(t_pdp_gain *x)
{
    pdp_queue_finish(x->x_queue_id);
    pdp_imageproc_gain_delete(x->x_gain_y);
    pdp_imageproc_gain_delete(x->x_gain_u);
    pdp_imageproc_gain_delete(x->x_gain_v);
}

void *pdp_gain_new(t_floatarg f)
{
    t_pdp_gain *x = (t_pdp_gain *)pd_new(pdp_gain_class);


    /* no arg, or zero -> gain = 1 */
    if (f==0.0f) f = 1.0f;

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("gain"));

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;
    x->x_queue_id = -1;

    x->x_gain_y = pdp_imageproc_gain_new();
    x->x_gain_u = pdp_imageproc_gain_new();
    x->x_gain_v = pdp_imageproc_gain_new();
    pdp_gain_gain(x, f);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_gain_setup(void)
{


    pdp_gain_class = class_new(gensym("pdp_gain"), (t_newmethod)pdp_gain_new,
    	(t_method)pdp_gain_free, sizeof(t_pdp_gain), 0, A_DEFFLOAT, A_NULL);


    class_addmethod(pdp_gain_class, (t_method)pdp_gain_gain, gensym("gain"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_gain_class, (t_method)pdp_gain_gainy, gensym("y"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_gain_class, (t_method)pdp_gain_gainu, gensym("v"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_gain_class, (t_method)pdp_gain_gainv, gensym("u"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_gain_class, (t_method)pdp_gain_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
