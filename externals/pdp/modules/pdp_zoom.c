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
#include "pdp_resample.h"



typedef struct pdp_zoom_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;


    int x_packet0;
    int x_packet1;
    int x_dropped;
    int x_queue_id;

    float x_zoom_x;
    float x_zoom_y;

    float x_center_x;
    float x_center_y;

    int x_quality; //not used

    
} t_pdp_zoom;


static void pdp_zoom_process_yv12(t_pdp_zoom *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    t_pdp *header1 = pdp_packet_header(x->x_packet1);
    void  *data0   = pdp_packet_data  (x->x_packet0);
    void  *data1   = pdp_packet_data  (x->x_packet1);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    short int *src_image = (short int *)data0;
    short int *dst_image = (short int *)data1;

    unsigned int size = w * h;
    unsigned int voffset = size;
    unsigned int uoffset = size + (size>>2);


    pdp_resample_zoom_tiled_bilin(src_image, dst_image, w, h, x->x_zoom_x, x->x_zoom_y, x->x_center_x, x->x_center_y);
    pdp_resample_zoom_tiled_bilin(src_image+voffset, dst_image+voffset, w>>1, h>>1, x->x_zoom_x, x->x_zoom_y, x->x_center_x, x->x_center_y);
    pdp_resample_zoom_tiled_bilin(src_image+uoffset, dst_image+uoffset, w>>1, h>>1, x->x_zoom_x, x->x_zoom_y, x->x_center_x, x->x_center_y);

    return;
}

static void pdp_zoom_process_grey(t_pdp_zoom *x)
{

    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    t_pdp *header1 = pdp_packet_header(x->x_packet1);
    void  *data0   = pdp_packet_data  (x->x_packet0);
    void  *data1   = pdp_packet_data  (x->x_packet1);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    short int *src_image = (short int *)data0;
    short int *dst_image = (short int *)data1;

    pdp_resample_zoom_tiled_bilin(src_image, dst_image, w, h, x->x_zoom_x, x->x_zoom_y, x->x_center_x, x->x_center_y);

    return;

}

static void pdp_zoom_sendpacket(t_pdp_zoom *x)
{
    /* delete source packet */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = -1;

    /* unregister and propagate if valid dest packet */
    pdp_pass_if_valid(x->x_outlet0, &x->x_packet1);
}

static void pdp_zoom_process(t_pdp_zoom *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);

    /* check data packets */

    if ((header0) && (PDP_IMAGE == header0->type)){


	/* type hub */
	switch(header0->info.image.encoding){

	case PDP_IMAGE_YV12:
	    x->x_packet1 = pdp_packet_clone_rw(x->x_packet0);
	    pdp_queue_add(x, pdp_zoom_process_yv12, pdp_zoom_sendpacket, &x->x_queue_id);
	    break;

	case PDP_IMAGE_GREY:
	    x->x_packet1 = pdp_packet_clone_rw(x->x_packet0);
	    pdp_queue_add(x, pdp_zoom_process_grey, pdp_zoom_sendpacket, &x->x_queue_id);
	    break;

	default:
	    break;
	    /* don't know the type, so dont process */
	    
	}
    }

}




static void pdp_zoom_input_0(t_pdp_zoom *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;
    int passes, i;

    if (s== gensym("register_rw"))  x->x_dropped = pdp_packet_copy_ro_or_drop(&x->x_packet0, p);


    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped)){

	/* add the process method and callback to the process queue */
	pdp_zoom_process(x);

    }

}




static void pdp_zoom_x(t_pdp_zoom *x, t_floatarg f)
{
    x->x_zoom_x = f;
}

static void pdp_zoom_y(t_pdp_zoom *x, t_floatarg f)
{
    x->x_zoom_y = f;
}

static void pdp_zoom(t_pdp_zoom *x, t_floatarg f)
{
    pdp_zoom_x(x, f);
    pdp_zoom_y(x, f);
}

static void pdp_zoom_center_x(t_pdp_zoom *x, t_floatarg f)
{
    x->x_center_x = (f + 0.5f);
}

static void pdp_zoom_center_y(t_pdp_zoom *x, t_floatarg f)
{
    x->x_center_y = (f + 0.5f);
}
static void pdp_zoom_center(t_pdp_zoom *x, t_floatarg fx, t_floatarg fy)
{
    pdp_zoom_center_x(x, fx);
    pdp_zoom_center_y(x, fy);
}

static void pdp_zoom_quality(t_pdp_zoom *x, t_floatarg f)
{
    if (f==0) x->x_quality = 0;
    if (f==1) x->x_quality = 1;
}


t_class *pdp_zoom_class;



void pdp_zoom_free(t_pdp_zoom *x)
{
    pdp_queue_finish(x->x_queue_id);
    pdp_packet_mark_unused(x->x_packet0);
    pdp_packet_mark_unused(x->x_packet1);
}

void *pdp_zoom_new(t_floatarg fw, t_floatarg zoom)
{
    t_pdp_zoom *x = (t_pdp_zoom *)pd_new(pdp_zoom_class);

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("zoom"));

  

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;
    x->x_packet1 = -1;
    x->x_queue_id = -1;

    pdp_zoom_quality(x, 1);
    pdp_zoom_center_x(x, 0);
    pdp_zoom_center_y(x, 0);

    if (zoom = 0.0f) zoom = 1.0f;
    pdp_zoom(x, zoom);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_zoom_setup(void)
{


    pdp_zoom_class = class_new(gensym("pdp_zoom"), (t_newmethod)pdp_zoom_new,
    	(t_method)pdp_zoom_free, sizeof(t_pdp_zoom), 0, A_DEFFLOAT, A_DEFFLOAT, A_NULL);


    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom_quality, gensym("quality"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom_center_x, gensym("centerx"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom_center_y, gensym("centery"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom_center, gensym("center"),  A_FLOAT, A_FLOAT, A_NULL);   
    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom_x, gensym("zoomx"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom_y, gensym("zoomy"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom, gensym("zoom"),  A_FLOAT, A_NULL);   
    class_addmethod(pdp_zoom_class, (t_method)pdp_zoom_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
