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

typedef struct pdp_slice_glue_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;

    int x_packet0; //input packet
    int x_packet1; //output packet

} t_pdp_slice_glue;



static void pdp_slice_glue_process_grey(t_pdp_slice_glue *x)
{
    t_pdp     *header0 = pdp_packet_header(x->x_packet0);
    t_pdp     *header1 = pdp_packet_header(x->x_packet1);
    short int *data0   = (short int *)pdp_packet_data  (x->x_packet0);
    short int *data1   = (short int *)pdp_packet_data  (x->x_packet1);


    unsigned int offset = header0->info.image.width * header0->info.image.slice_yoff;
    unsigned int slice_size = header0->info.image.width *  header0->info.image.height;

    memcpy(data1 + offset, data0, slice_size << 1);


    return;
}

static void pdp_slice_glue_process_yv12(t_pdp_slice_glue *x)
{

    t_pdp     *header0 = pdp_packet_header(x->x_packet0);
    t_pdp     *header1 = pdp_packet_header(x->x_packet1);
    short int *data0   = (short int *)pdp_packet_data  (x->x_packet0);
    short int *data1   = (short int *)pdp_packet_data  (x->x_packet1);

    unsigned int w = header0->info.image.width;
    unsigned int h = header0->info.image.height;

    unsigned int dw = header1->info.image.width;
    unsigned int dh = header1->info.image.height;
    unsigned int dsize = dh*dw;

    unsigned int offset = w * header0->info.image.slice_yoff;
    unsigned int slice_size = w *  h;

    memcpy(data1 + offset, data0, slice_size << 1);
    memcpy(data1 + dsize + (offset >> 2), data0 + slice_size, slice_size >> 1);
    memcpy(data1 + dsize + (dsize >>2) + (offset>>2), data0 + slice_size + (slice_size >> 2), slice_size >> 1);


    return;

}



static void pdp_slice_glue_process(t_pdp_slice_glue *x)
{
   int encoding;
   int newpacket;
   t_pdp *header = 0;
   t_pdp *dest_header = 0;


   /* check if the packet is a valid slice packet */
   /* if not pass it along */
   if (!((header = pdp_packet_header(x->x_packet0)) && 
	 (PDP_IMAGE == header->type) && (header->info.image.orig_height))) goto passalong;

   /* if this is the first slice of a sequence, or we don't have a dest packet yet */
   /* create a new destination packet */
   if ((x->x_packet1 == -1) || (header->info.image.slice_sync & PDP_IMAGE_SLICE_FIRST)){
       pdp_packet_mark_unused(x->x_packet1);
       x->x_packet1 = pdp_packet_new(PDP_IMAGE, ((header->size - PDP_HEADER_SIZE) / header->info.image.height) 
				     * header->info.image.orig_height);
       dest_header = pdp_packet_header(x->x_packet1);
       dest_header->info.image.encoding = header->info.image.encoding;
       dest_header->info.image.width = header->info.image.width;
       dest_header->info.image.height = header->info.image.orig_height;
   }

   /* if this is a body or last slice, it has to be compatible with the current dest packet */
   /* else ignore it */
   else{
       dest_header = pdp_packet_header(x->x_packet1);
       if (!((dest_header->info.image.encoding == header->info.image.encoding) && 
	     ((dest_header->info.image.width == header->info.image.width)))) goto dispose;
   }

   /* now we have a dest packet and the source packet is compatible with it, so copy the data */
       

   switch(pdp_packet_header(x->x_packet0)->info.image.encoding){
       
   case PDP_IMAGE_YV12:
       pdp_slice_glue_process_yv12(x);
       break;
       
   case PDP_IMAGE_GREY:
       pdp_slice_glue_process_grey(x);
       break;
       
   default:
       /* don't know the type, so dont pdp_slice_glue_process */
       
       break;
   }

   /* if the source packet was a final slice, pass on the constructed packet */
   if (header->info.image.slice_sync & PDP_IMAGE_SLICE_LAST){
       pdp_pass_if_valid(x->x_outlet0, &x->x_packet1);
   }

   
   /* processing is done so we delete the source packet */
 dispose:
   pdp_packet_mark_unused(x->x_packet0);
   x->x_packet0 = -1;
   return;


 passalong:
   pdp_pass_if_valid(x->x_outlet0, &x->x_packet0);
   return;

}

static void pdp_slice_glue_input_0(t_pdp_slice_glue *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;

    if (s== gensym("register_ro"))  x->x_packet0 = pdp_packet_copy_ro(p);
    if ((s == gensym("process")) && (-1 != x->x_packet0)) pdp_slice_glue_process(x);

}




static void pdp_slice_glue_free(t_pdp_slice_glue *x)
{
    pdp_packet_mark_unused(x->x_packet0);
}

t_class *pdp_slice_glue_class;



void *pdp_slice_glue_new(void)
{
    int i;

    t_pdp_slice_glue *x = (t_pdp_slice_glue *)pd_new(pdp_slice_glue_class);

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 
    x->x_packet0 = -1;

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_slice_glue_setup(void)
{


    pdp_slice_glue_class = class_new(gensym("pdp_slice_glue"), (t_newmethod)pdp_slice_glue_new,
    	(t_method)pdp_slice_glue_free, sizeof(t_pdp_slice_glue), 0, A_NULL);


    class_addmethod(pdp_slice_glue_class, (t_method)pdp_slice_glue_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
