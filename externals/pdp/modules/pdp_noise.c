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



typedef struct pdp_noise_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;

    void *x_noisegen;
    
    int x_packet0;
    int x_queue_id;

    int x_pdp_image_type;

    unsigned int x_width;
    unsigned int x_height;
 
} t_pdp_noise;



void pdp_noise_type(t_pdp_noise *x, t_symbol *s)
{
    if (gensym("yv12") == s) {x->x_pdp_image_type = PDP_IMAGE_YV12; return;}
    if (gensym("grey") == s) {x->x_pdp_image_type = PDP_IMAGE_GREY; return;}

    x->x_pdp_image_type = -1;
    
}


void pdp_noise_random(t_pdp_noise *x, t_floatarg seed)
{
    if (seed == 0.0f) seed = (float)random();
    pdp_imageproc_random_setseed(x->x_noisegen, seed);

}



static void pdp_noise_createpacket_yv12(t_pdp_noise *x)
{
    /* create new packet */
    x->x_packet0 = pdp_packet_new_image_yv12(x->x_width, x->x_height);

    /* seed the 16 bit rng with a new random number from the clib */
    pdp_noise_random(x, 0.0f);
}

static void pdp_noise_generate_yv12(t_pdp_noise *x)
{
    short int *data;
    unsigned int w = x->x_width;
    unsigned int h = x->x_height;

    h = h + (h>>1);

    data = (short int *) pdp_packet_data(x->x_packet0);

    pdp_noise_random(x, 0.0f);
    pdp_imageproc_random_process(x->x_noisegen, data, w, h);

    return;

}

static void pdp_noise_createpacket_grey(t_pdp_noise *x)
{
    /* create new packet */
    x->x_packet0 = pdp_packet_new_image_grey(x->x_width, x->x_height);

    /* seed the 16 bit rng with a new random number from the clib */
    pdp_noise_random(x, 0.0f);
}

static void pdp_noise_generate_grey(t_pdp_noise *x)
{
    unsigned int w = x->x_width;
    unsigned int h = x->x_height;
    short int *data = (short int *) pdp_packet_data(x->x_packet0);

    data = (short int *) pdp_packet_data(x->x_packet0);

    pdp_noise_random(x, 0.0f);
    pdp_imageproc_random_process(x->x_noisegen, data, w, h);


    return;
}

static void pdp_noise_sendpacket(t_pdp_noise *x, t_floatarg w, t_floatarg h)
{
    /* propagate if valid */
    pdp_pass_if_valid(x->x_outlet0, &x->x_packet0);
}


static void pdp_noise_bang(t_pdp_noise *x)
{

    int encoding;

    /* if we have an active packet, don't do anything */
    if (-1 != x->x_packet0) return;

    switch(x->x_pdp_image_type){

    case PDP_IMAGE_YV12:
	pdp_noise_createpacket_yv12(x); // don't create inside thread!!!
	pdp_queue_add(x, pdp_noise_generate_yv12, pdp_noise_sendpacket, &x->x_queue_id);
	break;
      
    case PDP_IMAGE_GREY:
	pdp_noise_createpacket_grey(x); // don't create inside thread!!!
	pdp_queue_add(x, pdp_noise_generate_grey, pdp_noise_sendpacket, &x->x_queue_id);
	break;

    default:
	break;
	
    }


    /* release the packet */

}


static void pdp_noise_dim(t_pdp_noise *x, t_floatarg w, t_floatarg h)
{
    if (w<32.0f) w = 32.0f;
    if (h<32.0f) h = 32.0f;

    x->x_width = (unsigned int)w;
    x->x_height = (unsigned int)h;
}


static void pdp_noise_free(t_pdp_noise *x)
{

    /* remove callback from process queue */
    pdp_queue_finish(x->x_queue_id);


    /* tidy up */
    pdp_packet_mark_unused(x->x_packet0);
    pdp_imageproc_random_delete(x->x_noisegen);

}

t_class *pdp_noise_class;




void *pdp_noise_new(void)
{
    int i;

    t_pdp_noise *x = (t_pdp_noise *)pd_new(pdp_noise_class);

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;
    x->x_queue_id = -1;
    x->x_width = 320;
    x->x_height = 240;

    x->x_noisegen = pdp_imageproc_random_new();

    pdp_noise_random(x, 0.0f);

    pdp_noise_type(x, gensym("yv12"));

    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif



void pdp_noise_setup(void)
{


    pdp_noise_class = class_new(gensym("pdp_noise"), (t_newmethod)pdp_noise_new,
    	(t_method)pdp_noise_free, sizeof(t_pdp_noise), 0, A_NULL);

    class_addmethod(pdp_noise_class, (t_method)pdp_noise_random, gensym("seed"), A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_noise_class, (t_method)pdp_noise_type, gensym("type"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_noise_class, (t_method)pdp_noise_dim, gensym("dim"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_noise_class, (t_method)pdp_noise_bang, gensym("bang"), A_NULL);

}

#ifdef __cplusplus
}
#endif
