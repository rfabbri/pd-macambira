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


typedef struct yv12_palette_struct
{
    short int y;
    short int u;
    short int v;
} t_yv12_palette;

typedef struct pdp_gradient_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;

    int x_packet0;
    int x_dropped;
    int x_queue_id;

    t_yv12_palette x_palette[512];

} t_pdp_gradient;


unsigned int float2fixed(float f)
{
    f *= 0x8000;
    if(f>0x7fff) f = 0x7fff;
    if(f<-0x7fff) f = -0x7fff;
    return (short int)f;
}

static void pdp_gradient_yuv_index(t_pdp_gradient *x, 
			     t_floatarg index, 
			     t_floatarg y,
			     t_floatarg u,
			     t_floatarg v)
{
    int i = ((int)index) & 511;
    x->x_palette[i].y = float2fixed(y);
    x->x_palette[i].u = float2fixed(u);
    x->x_palette[i].v = float2fixed(v);
}

static void pdp_gradient_yuv(t_pdp_gradient *x, 
			     t_floatarg y,
			     t_floatarg u,
			     t_floatarg v)
{
    int i;
    float inc = 1.0f / 256.0f;
    float frac = -1.0f;
    for(i=-256; i<256; i++) {
	pdp_gradient_yuv_index(x, i, frac*y, frac*u, frac*v);
	frac += inc;
    }
}

static void pdp_gradient_rgb_index(t_pdp_gradient *x, 
			     t_floatarg index, 
			     t_floatarg r,
			     t_floatarg g,
			     t_floatarg b)
{
    float y,u,v;


    y = 0.299f * r + 0.587f * g + 0.114f * b;
    u = (r-y) * 0.713f;
    v = (b-y) * 0.565f;

    pdp_gradient_yuv_index(x, index, y,u,v);
}

static void pdp_gradient_rgb(t_pdp_gradient *x, 
			     t_floatarg r,
			     t_floatarg g,
			     t_floatarg b)
{
    int i;
    float inc = 1.0f / 256.0f;
    float frac = -1.0f;
    for(i=-256; i<256; i++) {
	pdp_gradient_rgb_index(x, i, frac*r, frac*g, frac*b);
	frac += inc;
    }
}

/*
static void pdp_gradient_hsv_index(t_pdp_gradient *x, 
			     t_floatarg index, 
			     t_floatarg h,
			     t_floatarg s,
			     t_floatarg v)
{
    float r,g,b;

    r = h;
    g = s;
    b = v;

    pdp_gradient_rgb_index(x, index, r,g,b);
}

static void pdp_gradient_hsv(t_pdp_gradient *x, 
			     t_floatarg h,
			     t_floatarg s,
			     t_floatarg v)
{
    int i;
    float inc = 1.0f / 256.0f;
    float frac = -1.0f;
    for(i=-256; i<256; i++) {
	pdp_gradient_hsv_index(x, i, h, s, frac*v);
	frac += inc;
    }
}
*/

static void pdp_gradient_process_grey(t_pdp_gradient *x)
{
    t_pdp     *header = pdp_packet_header(x->x_packet0);
    short int *data   = (short int *)pdp_packet_data  (x->x_packet0);
    t_pdp     *newheader = 0;
    short int *newdata = 0;
    int       newpacket = -1;

    unsigned int w = header->info.image.width;
    unsigned int h = header->info.image.height;

    unsigned int size = w*h;
    unsigned int totalnbpixels = size;
    unsigned int u_offset = size;
    unsigned int v_offset = size + (size>>2);

    unsigned int row, col;

    newpacket = pdp_packet_new(PDP_IMAGE, (size + (size>>1))<<1);
    newheader = pdp_packet_header(newpacket);
    newdata = (short int *)pdp_packet_data(newpacket);

    newheader->info.image.encoding = PDP_IMAGE_YV12;
    newheader->info.image.width = w;
    newheader->info.image.height = h;

    /* convert every pixel according to palette */
    for(row=0; row < size; row += (w<<1)){
	for(col=0; col < w; col += 2){
	    short int u_acc, v_acc;
	    short int grey;

	    /* top left pixel */
	    grey = ((data[row+col])>>7) & 511;
	    newdata[row+col] = x->x_palette[grey].y;
	    u_acc = (x->x_palette[grey].u)>>2;
	    v_acc = (x->x_palette[grey].v)>>2;

	    /* top right pixel */
	    grey = ((data[row+col+1])>>7) & 511;
	    newdata[row+col+1] = x->x_palette[grey].y;
	    u_acc += (x->x_palette[grey].u)>>2;
	    v_acc += (x->x_palette[grey].v)>>2;	    

	    /* bottom left pixel */
	    grey = ((data[row+col+w])>>7) & 511;
	    newdata[row+col+w] = x->x_palette[grey].y;
	    u_acc += (x->x_palette[grey].u)>>2;
	    v_acc += (x->x_palette[grey].v)>>2;

	    /* bottom right pixel */
	    grey = ((data[row+col+w+1])>>7) & 511;
	    newdata[row+col+w+1] = x->x_palette[grey].y;
	    u_acc += (x->x_palette[grey].u)>>2;
	    v_acc += (x->x_palette[grey].v)>>2;

	    /* store uv comp */
	    newdata[u_offset + (row>>2) + (col>>1)] = u_acc;
	    newdata[v_offset + (row>>2) + (col>>1)] = v_acc;
	}
    }    
    


    /* delete source packet and replace with new packet */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = newpacket;
    return;
}

static void pdp_gradient_process_yv12(t_pdp_gradient *x)
{
    /* pdp_gradient_process only the luminance channel */
    pdp_gradient_process_grey(x);
}



static void pdp_gradient_process(t_pdp_gradient *x)
{
   int encoding;
   t_pdp *header = 0;

   /* check if image data packets are compatible */
   if ( (header = pdp_packet_header(x->x_packet0))
	&& (PDP_IMAGE == header->type)){
    
	/* pdp_gradient_process inputs and write into active inlet */
	switch(pdp_packet_header(x->x_packet0)->info.image.encoding){

	case PDP_IMAGE_YV12:
	    pdp_gradient_process_yv12(x);
	    break;

	case PDP_IMAGE_GREY:
	    pdp_gradient_process_grey(x);
	    break;

	default:
	    /* don't know the type, so dont pdp_gradient_process */
	    
	    break;
	}
    }
}

static void pdp_gradient_sendpacket(t_pdp_gradient *x)
{
    /* unregister and propagate if valid packet */
    pdp_pass_if_valid(x->x_outlet0, &x->x_packet0);
}

static void pdp_gradient_input_0(t_pdp_gradient *x, t_symbol *s, t_floatarg f)
{

    int p = (int)f;

    if (s== gensym("register_rw"))  x->x_dropped = pdp_packet_copy_rw_or_drop(&x->x_packet0, p);


    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped)){


	/* add the process method and callback to the process queue */
	// since the process method creates a packet, this is not processed in the thread
	// $$$TODO: fix this
	//pdp_queue_add(x, pdp_gradient_process, pdp_gradient_sendpacket, &x->x_queue_id);
	pdp_gradient_process(x);
	pdp_gradient_sendpacket(x);
    }

}



static void pdp_gradient_free(t_pdp_gradient *x)
{
    pdp_queue_finish(x->x_queue_id);
    pdp_packet_mark_unused(x->x_packet0);

}

t_class *pdp_gradient_class;



void *pdp_gradient_new(void)
{
    int i;

    t_pdp_gradient *x = (t_pdp_gradient *)pd_new(pdp_gradient_class);

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;
    x->x_queue_id = -1;

    for (i=-256; i<256; i++){
	x->x_palette[i&511].y = i << 7;
	x->x_palette[i&511].u = (-i) << 6;
	x->x_palette[i&511].v = (i) << 5;
    }
    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_gradient_setup(void)
{


    pdp_gradient_class = class_new(gensym("pdp_gradient"), (t_newmethod)pdp_gradient_new,
    	(t_method)pdp_gradient_free, sizeof(t_pdp_gradient), 0, A_NULL);


    class_addmethod(pdp_gradient_class, (t_method)pdp_gradient_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

    class_addmethod(pdp_gradient_class, (t_method)pdp_gradient_yuv, gensym("yuv"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_gradient_class, (t_method)pdp_gradient_rgb, gensym("rgb"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    //    class_addmethod(pdp_gradient_class, (t_method)pdp_gradient_hsv, gensym("hsv"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
