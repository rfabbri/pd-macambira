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


typedef struct pdp_randmix_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;

    int x_packet0;
    int x_packet1;
    int x_packet1next;
    int x_queue_id;
    int x_dropped;

    void *x_randmixer;
 
} t_pdp_randmix;


void pdp_randmix_random(t_pdp_randmix *x, t_floatarg seed)
{
    pdp_imageproc_randmix_setseed(x->x_randmixer, seed);
}



static void pdp_randmix_process_yv12(t_pdp_randmix *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    t_pdp *header1 = pdp_packet_header(x->x_packet1);
    void  *data0   = pdp_packet_data  (x->x_packet0);
    void  *data1   = pdp_packet_data  (x->x_packet1);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    h = h + (h>>1);

    pdp_imageproc_randmix_process(x->x_randmixer, (short int*)data0, (short int*)data1, w, h);

    return;



}

static void pdp_randmix_process_grey(t_pdp_randmix *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    t_pdp *header1 = pdp_packet_header(x->x_packet1);
    void  *data0   = pdp_packet_data  (x->x_packet0);
    void  *data1   = pdp_packet_data  (x->x_packet1);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    pdp_imageproc_randmix_process(x->x_randmixer, (short int*)data0, (short int*)data1, w, h);

    return;

}

static void pdp_randmix_process(t_pdp_randmix *x)
{

  int encoding;

   /* check if image data packets are compatible */
    if (pdp_type_compat_image(x->x_packet0, x->x_packet1)){
    
	/* pdp_randmix_process inputs and write into active inlet */
	switch(pdp_packet_header(x->x_packet0)->info.image.encoding){

	case PDP_IMAGE_YV12:
	    pdp_randmix_process_yv12(x);
	    break;

	case PDP_IMAGE_GREY:
	    pdp_randmix_process_grey(x);
	    break;

	default:
	    break;
	    /* don't know the type, so dont pdp_randmix_process */
	    
	}
    }

}

static void pdp_randmix_sendpacket(t_pdp_randmix *x)
{
    /* unregister and propagate if valid packet */
    pdp_pass_if_valid(x->x_outlet0, &x->x_packet0);
}

static void pdp_randmix_input_0(t_pdp_randmix *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;

    if (s== gensym("register_rw"))  x->x_dropped = pdp_packet_copy_rw_or_drop(&x->x_packet0, p);


    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped)){

	/* if a cold packet was received in the meantime
	   swap it in, else keep the old one */

	pdp_replace_if_valid(&x->x_packet1, &x->x_packet1next);


	/* add the process method and callback to the process queue */

	pdp_queue_add(x, pdp_randmix_process, pdp_randmix_sendpacket, &x->x_queue_id);
    }

}

static void pdp_randmix_input_1(t_pdp_randmix *x, t_symbol *s, t_floatarg f)
{
    /* store the packet and drop 
       the old one, if there is any */

    if(s == gensym("register_ro")) pdp_packet_copy_ro_or_drop(&x->x_packet1next, (int)f);

}





static void pdp_randmix_threshold(t_pdp_randmix *x, t_floatarg f)
{
    pdp_imageproc_randmix_setthreshold(x->x_randmixer, f);

}



static void pdp_randmix_free(t_pdp_randmix *x)
{
    pdp_queue_finish(x->x_queue_id);

    pdp_packet_mark_unused(x->x_packet0);
    pdp_packet_mark_unused(x->x_packet1);

    pdp_imageproc_randmix_delete(x->x_randmixer);

}

t_class *pdp_randmix_class;


void *pdp_randmix_new(void)
{
    int i;

    t_pdp_randmix *x = (t_pdp_randmix *)pd_new(pdp_randmix_class);

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("pdp"), gensym("pdp1"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("threshold"));

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;
    x->x_packet1 = -1;
    x->x_packet1next = -1;
    x->x_queue_id = -1;

    x->x_randmixer = pdp_imageproc_randmix_new();

    pdp_randmix_threshold(x, 0.5f);
    pdp_randmix_random(x, 0.0f);

    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif



void pdp_randmix_setup(void)
{


    pdp_randmix_class = class_new(gensym("pdp_randmix"), (t_newmethod)pdp_randmix_new,
    	(t_method)pdp_randmix_free, sizeof(t_pdp_randmix), 0, A_NULL);

    class_addmethod(pdp_randmix_class, (t_method)pdp_randmix_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_randmix_class, (t_method)pdp_randmix_input_1, gensym("pdp1"), A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_randmix_class, (t_method)pdp_randmix_threshold, gensym("threshold"), A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_randmix_class, (t_method)pdp_randmix_random, gensym("seed"), A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
