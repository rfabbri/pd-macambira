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


typedef struct pdp_mix_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;
    t_outlet *x_outlet1;

    int x_packet0;       // hot packet
    int x_packet1;       // cold packet
    int x_packet1next;   // next cold packet
    int x_dropped;

    int x_queue_id;      // identifier of method in the processing queue

    void *x_mixer;
 
    int x_extrapolate;
    
} t_pdp_mix;



static void pdp_mix_process_yv12(t_pdp_mix *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    t_pdp *header1 = pdp_packet_header(x->x_packet1);
    void  *data0   = pdp_packet_data  (x->x_packet0);
    void  *data1   = pdp_packet_data  (x->x_packet1);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;


    h = h + (h>>1);
    pdp_imageproc_mix_process(x->x_mixer,(short int*)data0, (short int*)data1, w, h);

    return;



}

static void pdp_mix_process_grey(t_pdp_mix *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    t_pdp *header1 = pdp_packet_header(x->x_packet1);
    void  *data0   = pdp_packet_data  (x->x_packet0);
    void  *data1   = pdp_packet_data  (x->x_packet1);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    pdp_imageproc_mix_process(x->x_mixer,(short int*)data0, (short int*)data1, w, h);

    return;



}

static void pdp_mix_process(t_pdp_mix *x)
{
   int encoding;

   /* check if image data packets are compatible */
    if (pdp_type_compat_image(x->x_packet0, x->x_packet1)){
    
	/* dispatch to process thread */
	switch(pdp_packet_header(x->x_packet0)->info.image.encoding){

	case PDP_IMAGE_YV12:
	    pdp_mix_process_yv12(x);
	    break;

	case PDP_IMAGE_GREY:
	    pdp_mix_process_grey(x);
	    break;

	default:
	    break;
	    /* don't know the type, so dont pdp_mix_process */
	    
	}
    }

}

static void pdp_mix_sendpacket(t_pdp_mix *x)
{
    /* unregister and propagate if valid packet */
    pdp_pass_if_valid(x->x_outlet0, &x->x_packet0);
}

static void pdp_mix_input_0(t_pdp_mix *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;

    if (s== gensym("register_rw"))  x->x_dropped =  pdp_packet_copy_rw_or_drop(&x->x_packet0, p);


    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped)){

	/* if a cold packet was received in the meantime
	   swap it in, else keep the old one */

	pdp_replace_if_valid(&x->x_packet1, &x->x_packet1next);


	/* add the process method and callback to the process queue */

	pdp_queue_add(x, pdp_mix_process, pdp_mix_sendpacket, &x->x_queue_id);
    }

}

static void pdp_mix_input_1(t_pdp_mix *x, t_symbol *s, t_floatarg f)
{
    /* store the packet and drop 
       the old one, if there is any */

    if(s == gensym("register_ro")) pdp_packet_copy_ro_or_drop(&x->x_packet1next, (int)f);

}



static void pdp_mix_mix(t_pdp_mix *x, t_floatarg f)
{
    float f2;
    if (!x->x_extrapolate){
	if (f < 0.0f) f = 0.0f;
	if (f > 1.0f) f = 1.0f;
    }

    f2 = (1.0f - f);
    pdp_imageproc_mix_setleftgain(x->x_mixer, f2);
    pdp_imageproc_mix_setrightgain(x->x_mixer, f);

}

static void pdp_mix_mix1(t_pdp_mix *x, t_floatarg f)
{
    pdp_imageproc_mix_setleftgain(x->x_mixer, f);

}
static void pdp_mix_mix2(t_pdp_mix *x, t_floatarg f2)
{
    pdp_imageproc_mix_setrightgain(x->x_mixer, f2);
}

static void pdp_mix_extrapolate(t_pdp_mix *x, t_floatarg f)
{
    if (f == 0.0f) x->x_extrapolate = 0;
    if (f == 1.0f) x->x_extrapolate = 1;
}


static void pdp_mix_free(t_pdp_mix *x)
{
    /* remove the method from the queue before deleting stuff */
    pdp_queue_finish(x->x_queue_id);

    pdp_packet_mark_unused(x->x_packet0);
    pdp_packet_mark_unused(x->x_packet1);
    pdp_packet_mark_unused(x->x_packet1next);

    pdp_imageproc_mix_delete(x->x_mixer);
}

t_class *pdp_mix_class;
t_class *pdp_mix2_class;


void *pdp_mix_common_init(t_pdp_mix *x)
{
    int i;

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("pdp"), gensym("pdp1"));

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 
    x->x_packet0 = -1;
    x->x_packet1 = -1;
    x->x_packet1next = -1;
    x->x_queue_id = -1;
    x->x_extrapolate = 0;


    x->x_mixer = pdp_imageproc_mix_new();
    pdp_mix_mix(x, 0.0f);


    return (void *)x;
}


void *pdp_mix_new(t_floatarg mix)
{
  t_pdp_mix *x = (t_pdp_mix *)pd_new(pdp_mix_class);
  pdp_mix_common_init(x);

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("mix"));
  if (mix == 0.0f) mix = 0.5f;
  pdp_mix_mix(x, mix);
  return (void *)x;
}

void *pdp_mix2_new(t_floatarg mix1, t_floatarg mix2)
{
  t_pdp_mix *x = (t_pdp_mix *)pd_new(pdp_mix2_class);
  pdp_mix_common_init(x);

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("mix1"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("mix2"));
  if ((mix1 == 0.0f) && (mix2 == 0.0f)) mix1 = mix2 = 0.5f;
  pdp_mix_mix1(x, mix1);
  pdp_mix_mix2(x, mix2);
  return (void *)x;
}

#ifdef __cplusplus
extern "C"
{
#endif



void pdp_mix_setup(void)
{


    pdp_mix_class = class_new(gensym("pdp_mix"), (t_newmethod)pdp_mix_new,
    	(t_method)pdp_mix_free, sizeof(t_pdp_mix), 0, A_DEFFLOAT, A_NULL);

    class_addmethod(pdp_mix_class, (t_method)pdp_mix_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_mix_class, (t_method)pdp_mix_input_1, gensym("pdp1"), A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_mix_class, (t_method)pdp_mix_mix, gensym("mix"), A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_mix_class, (t_method)pdp_mix_extrapolate, gensym("extrapolate"), A_DEFFLOAT, A_NULL);

    pdp_mix2_class = class_new(gensym("pdp_mix2"), (t_newmethod)pdp_mix2_new,
    	(t_method)pdp_mix_free, sizeof(t_pdp_mix), 0, A_DEFFLOAT, A_DEFFLOAT, A_NULL);

    class_addmethod(pdp_mix2_class, (t_method)pdp_mix_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_mix2_class, (t_method)pdp_mix_input_1, gensym("pdp1"), A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_mix2_class, (t_method)pdp_mix_mix1, gensym("mix1"), A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_mix2_class, (t_method)pdp_mix_mix2, gensym("mix2"), A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_mix2_class, (t_method)pdp_mix_extrapolate, gensym("extrapolate"), A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
