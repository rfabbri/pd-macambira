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


typedef struct pdp_affine_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;

    int x_packet0;
    int x_queue_id;
    int x_dropped;          // indicate if a packet was dropped during register_rw cycle

    int x_channel;

    void *x_affine;

} t_pdp_affine;



static void pdp_affine_process_yv12(t_pdp_affine *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    void  *data0   = pdp_packet_data  (x->x_packet0);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    unsigned int size = w*h;
    unsigned int v_offset = size;
    unsigned int u_offset = size + (size >> 2) ;
        unsigned int i,j;

    short int * idata = (short int *)data0;
    int ch = x->x_channel;
    
    if((ch == 0) || (ch==1)) pdp_imageproc_affine_process(x->x_affine, &idata[0], w, h);
    if((ch == 0) || (ch==2)) pdp_imageproc_affine_process(x->x_affine, &idata[v_offset], w>>1, h>>1);
    if((ch == 0) || (ch==3)) pdp_imageproc_affine_process(x->x_affine, &idata[u_offset], w>>1, h>>1);

    return;


}

static void pdp_affine_process_grey(t_pdp_affine *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);
    void  *data0   = pdp_packet_data  (x->x_packet0);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    short int * idata = (short int *)data0;
    int ch = x->x_channel;

    if((ch == 0) || (ch==1)) pdp_imageproc_affine_process(x->x_affine, &idata[0], w, h);

    return;


}

static void pdp_affine_process(t_pdp_affine *x)
{
    t_pdp *header0 = pdp_packet_header(x->x_packet0);

    /* check data packets */

    if ((header0) && (PDP_IMAGE == header0->type)){

	/* pdp_affine_process inputs and write into active inlet */
	switch(header0->info.image.encoding){

	case PDP_IMAGE_YV12:
	    pdp_affine_process_yv12(x);
	    break;

	case PDP_IMAGE_GREY:
	    pdp_affine_process_grey(x);
	    break;

	default:
	    break;
	    /* don't know the type, so dont pdp_affine_process */
	    
	}
    }

}

static void pdp_affine_sendpacket(t_pdp_affine *x)
{
    /* unregister and propagate if valid packet */
    pdp_pass_if_valid(x->x_outlet0, &x->x_packet0);
}

static void pdp_affine_input_0(t_pdp_affine *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;

    if (s== gensym("register_rw"))  x->x_dropped = pdp_packet_copy_rw_or_drop(&x->x_packet0, p);


    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped)){


	/* add the process method and callback to the process queue */

	pdp_queue_add(x, pdp_affine_process, pdp_affine_sendpacket, &x->x_queue_id);
    }

}



static void pdp_affine_gain(t_pdp_affine *x, t_floatarg f)
{
    pdp_imageproc_affine_setgain(x->x_affine, f);

}

static void pdp_affine_offset(t_pdp_affine *x, t_floatarg f)
{
    pdp_imageproc_affine_setoffset(x->x_affine, f);
}

static void pdp_affine_channel(t_pdp_affine *x, t_floatarg f)
{
  int ch = (int)f;


  if ((ch < 1) || (ch > 3)) ch = 0;

  x->x_channel = ch;


}
static void pdp_affine_free(t_pdp_affine *x)
{
    pdp_queue_finish(x->x_queue_id);
    pdp_imageproc_affine_delete(x->x_affine);
    pdp_packet_mark_unused(x->x_packet0);

}

t_class *pdp_affine_class;



void *pdp_affine_new(t_floatarg f)
{
    t_pdp_affine *x = (t_pdp_affine *)pd_new(pdp_affine_class);

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("gain"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("offset"));

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;
    x->x_queue_id = -1;

    x->x_affine = pdp_imageproc_affine_new();

    pdp_affine_gain(x, 1.0f);
    pdp_affine_offset(x, 0.0f);
    pdp_affine_channel(x, f);


    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_affine_setup(void)
{


    pdp_affine_class = class_new(gensym("pdp_affine"), (t_newmethod)pdp_affine_new,
    	(t_method)pdp_affine_free, sizeof(t_pdp_affine), 0, A_DEFFLOAT, A_NULL);


    class_addmethod(pdp_affine_class, (t_method)pdp_affine_gain, gensym("gain"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_affine_class, (t_method)pdp_affine_offset, gensym("offset"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_affine_class, (t_method)pdp_affine_channel, gensym("chan"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_affine_class, (t_method)pdp_affine_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
