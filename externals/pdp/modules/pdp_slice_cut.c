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


/* this module converts a greyscale image or the luma channel of a colour image
   to a colour image intensity mask, usable for multiplication */

#include "pdp.h"
#include "pdp_resample.h"
#include "pdp_imageproc.h"

typedef struct pdp_slice_cut_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;

    int x_packet0;
    //int x_dropped;
    //int x_queue_id;

    unsigned int x_slice_height;


} t_pdp_slice_cut;



static void pdp_slice_cut_process_grey(t_pdp_slice_cut *x)
{
    t_pdp     *header = pdp_packet_header(x->x_packet0);
    short int *data   = (short int *)pdp_packet_data  (x->x_packet0);
    t_pdp     *newheader = 0;
    short int *newdata = 0;
    int       newpacket = -1;

    unsigned int w = header->info.image.width;
    unsigned int h = header->info.image.height;

    unsigned int size = w*h;
    unsigned int height_left = h;
    unsigned int slice_height = 0;
    unsigned int slice_yoffset = 0;
    unsigned int slice_offset = 0;
    unsigned int slice_size_bytes = 0;

    #define min(x,y) (((x)<(y)) ? (x) : (y))

    while(height_left){

	/* compute slice size */
	slice_height = min(x->x_slice_height, height_left);
	height_left -= slice_height;
	slice_size_bytes =  (w << 1) * slice_height;

	/* create new slice packet */
	newpacket = pdp_packet_new(PDP_IMAGE, slice_size_bytes);
	newheader = pdp_packet_header(newpacket);
	newdata = (s16*)pdp_packet_data(newpacket);
	
	newheader->info.image.encoding = PDP_IMAGE_GREY;
	newheader->info.image.width = w;
	newheader->info.image.height = slice_height;

	newheader->info.image.orig_height = h;
	newheader->info.image.slice_yoff = slice_yoffset;

	if (slice_height + height_left == h)   newheader->info.image.slice_sync = PDP_IMAGE_SLICE_FIRST;
	else if (height_left == 0)             newheader->info.image.slice_sync = PDP_IMAGE_SLICE_LAST;
	else                                   newheader->info.image.slice_sync = PDP_IMAGE_SLICE_BODY;
	
	/* copy slice data */
	memcpy(newdata, data+slice_offset, slice_size_bytes);

	/* unregister and propagate if valid packet */
	pdp_packet_mark_unused(newpacket);
	outlet_pdp(x->x_outlet0, newpacket);

	/* advance pointer stuff */
	slice_offset += (slice_size_bytes>>1);
	slice_yoffset += slice_height;
	
    }

   

    /* delete source packet when finished */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = -1;
    return;
}

static void pdp_slice_cut_process_yv12(t_pdp_slice_cut *x)
{

    t_pdp     *header = pdp_packet_header(x->x_packet0);
    short int *data   = (short int *)pdp_packet_data  (x->x_packet0);
    t_pdp     *newheader = 0;
    short int *newdata = 0;
    int       newpacket = -1;

    unsigned int w = header->info.image.width;
    unsigned int h = header->info.image.height;

    unsigned int size = w*h;
    unsigned int height_left = h;
    unsigned int slice_height = 0;
    unsigned int sequence_number = 0;
    unsigned int slice_offset = 0;
    unsigned int slice_yoffset = 0;
    unsigned int slice_size_bytes = 0;
    unsigned int slice_size = 0;

    #define min(x,y) (((x)<(y)) ? (x) : (y))

    while(height_left){

	/* compute slice size */
	slice_height = min(x->x_slice_height, height_left);
	height_left -= slice_height;
	slice_size =  w  * slice_height;
	slice_size_bytes =  slice_size << 1;

	/* create new slice packet */
	newpacket = pdp_packet_new(PDP_IMAGE, slice_size_bytes +  (slice_size_bytes >> 1));
	newheader = pdp_packet_header(newpacket);
	newdata = (s16*)pdp_packet_data(newpacket);	
	newheader->info.image.encoding = PDP_IMAGE_YV12;
	newheader->info.image.width = w;
	newheader->info.image.height = slice_height;
	newheader->info.image.orig_height = h;
	newheader->info.image.slice_yoff = slice_yoffset;

	if (slice_height + height_left == h)   newheader->info.image.slice_sync = PDP_IMAGE_SLICE_FIRST;
	else if (height_left == 0)             newheader->info.image.slice_sync = PDP_IMAGE_SLICE_LAST;
	else                                   newheader->info.image.slice_sync = PDP_IMAGE_SLICE_BODY;
	
	
	/* copy slice data */
	memcpy(newdata, 
	       data + slice_offset, 
	       slice_size_bytes);

	memcpy(newdata + slice_size, 
	       data + size + (slice_offset>>2), 
	       slice_size_bytes>>2);

	memcpy(newdata + slice_size + (slice_size >> 2), 
	       data + size + (size >> 2) + (slice_offset >> 2), 
	       slice_size_bytes>>2);

	/* unregister and propagate if valid packet */
	pdp_packet_mark_unused(newpacket);
	outlet_pdp(x->x_outlet0, newpacket);

	/* advance pointer stuff */
	slice_offset += (slice_size_bytes>>1);
	slice_yoffset += slice_height;
	
    }

   

    /* delete source packet when finished */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = -1;
    return;

}



static void pdp_slice_cut_process(t_pdp_slice_cut *x)
{
   int encoding;
   t_pdp *header = 0;


   /* check if image data packets are compatible */
   if ( (header = pdp_packet_header(x->x_packet0))
	&& (PDP_IMAGE == header->type)){
    
	/* pdp_slice_cut_process inputs and write into active inlet */
	switch(pdp_packet_header(x->x_packet0)->info.image.encoding){

	case PDP_IMAGE_YV12:
	    pdp_slice_cut_process_yv12(x);
	    break;

	case PDP_IMAGE_GREY:
	    pdp_slice_cut_process_grey(x);
	    break;

	default:
	    /* don't know the type, so dont pdp_slice_cut_process */
	    
	    break;
	}
    }
}

static void pdp_slice_cut_input_0(t_pdp_slice_cut *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;

    if (s== gensym("register_ro"))  x->x_packet0 = pdp_packet_copy_ro(p);
    if ((s == gensym("process")) && (-1 != x->x_packet0)) pdp_slice_cut_process(x);

}



static void pdp_slice_cut_height(t_pdp_slice_cut *x, t_floatarg f)
{
    int i = (int)f;
    x->x_slice_height = pdp_imageproc_legalheight(i);
    post("pdp_slice: height set to %d", x->x_slice_height);
    
}
static void pdp_slice_cut_forceheight(t_pdp_slice_cut *x, t_floatarg f)
{
    int i = (int)f;
    if (i<1) i = 1;
    x->x_slice_height = i;
    post("pdp_slice: WARNING: forceheight is a debug message. setting this to an abritrary");
    post("pdp_slice: WARNING: value can crash pdp. set it to a multiple of 2 and only use pixel");
    post("pdp_slice: WARNING: operations (no convolution or biquad) in the mmx version.");
    post("pdp_slice: height forced to %d", x->x_slice_height);
    
}


static void pdp_slice_cut_free(t_pdp_slice_cut *x)
{
    pdp_packet_mark_unused(x->x_packet0);
}

t_class *pdp_slice_cut_class;



void *pdp_slice_cut_new(void)
{
    int i;

    t_pdp_slice_cut *x = (t_pdp_slice_cut *)pd_new(pdp_slice_cut_class);

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 
    x->x_packet0 = -1;
    //x->x_queue_id = -1;

    x->x_slice_height = 8;

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_slice_cut_setup(void)
{


    pdp_slice_cut_class = class_new(gensym("pdp_slice_cut"), (t_newmethod)pdp_slice_cut_new,
    	(t_method)pdp_slice_cut_free, sizeof(t_pdp_slice_cut), 0, A_NULL);


    class_addmethod(pdp_slice_cut_class, (t_method)pdp_slice_cut_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_slice_cut_class, (t_method)pdp_slice_cut_height, gensym("height"),  A_FLOAT, A_NULL);
    class_addmethod(pdp_slice_cut_class, (t_method)pdp_slice_cut_forceheight, gensym("forceheight"),  A_FLOAT, A_NULL);


}

#ifdef __cplusplus
}
#endif
