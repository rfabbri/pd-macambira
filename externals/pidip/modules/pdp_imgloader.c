/*
 *   PiDiP module
 *   Copyright (c) by Yves Degoyon (ydegoyon@free.fr)
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

/*  This object loads an image from a file and blends it with a video
 *  It uses imlib2 for all graphical operations
 */

#include "pdp.h"
#include "yuv.h"
#include <math.h>
#include <ctype.h>
#include <Imlib2.h>  // imlib2 is required

static char   *pdp_imgloader_version = "pdp_imgloader: version 0.1 : image loading object written by ydegoyon@free.fr ";

typedef struct pdp_imgloader_struct
{
    t_object x_obj;
    t_float x_f;

    t_int x_packet0;
    t_int x_packet1;
    t_int x_dropped;
    t_int x_queue_id;

    t_outlet *x_outlet0;
    t_int x_vwidth;
    t_int x_vheight;
    t_int x_vsize;

    t_int x_xoffset; // x offset of the image
    t_int x_yoffset; // y offset of the image

        /* imlib data */
    Imlib_Image x_image;
    DATA32     *x_imdata;
    t_int       x_iwidth;
    t_int       x_iheight;

    t_float     x_blend;

} t_pdp_imgloader;

        /* load an image */
static void pdp_imgloader_load(t_pdp_imgloader *x, t_symbol *filename, t_floatarg fx, t_floatarg fy)
{
  Imlib_Load_Error imliberr;

   post( "pdp_imgloader : loading : %s", filename->s_name );

   if ( x->x_image != NULL ) 
   {
      imlib_free_image();
   }
   x->x_image = imlib_load_image_with_error_return( filename->s_name, &imliberr );
   if ( imliberr != IMLIB_LOAD_ERROR_NONE )
   {
      post( "pdp_imgloader : severe error : could not load image (err=%d)!!", imliberr );
      x->x_image = NULL;
      return;
   }
   imlib_context_set_image(x->x_image);
   x->x_imdata = imlib_image_get_data();
   x->x_iwidth = imlib_image_get_width();
   x->x_iheight = imlib_image_get_height();
   post( "pdp_imgloader : loaded : %s (%dx%d)", filename->s_name, x->x_iwidth, x->x_iheight );
   x->x_xoffset = (int) fx;
   x->x_yoffset = (int) fy;
}

static void pdp_imgloader_xoffset(t_pdp_imgloader *x, t_floatarg fx )
{
   x->x_xoffset = (int) fx;
}

static void pdp_imgloader_yoffset(t_pdp_imgloader *x, t_floatarg fy )
{
   x->x_yoffset = (int) fy;
}

static void pdp_imgloader_blend(t_pdp_imgloader *x, t_floatarg fblend )
{
   if ( ( fblend > 0.0 ) && ( fblend < 1.0 ) )
   {
     x->x_blend = fblend;
   }
}

static void pdp_imgloader_clear(t_pdp_imgloader *x )
{
   if ( x->x_image != NULL ) 
   {
      imlib_free_image();
   }
   x->x_image = NULL;
}

static void pdp_imgloader_process_yv12(t_pdp_imgloader *x)
{
    t_pdp     *header = pdp_packet_header(x->x_packet0);
    short int *data   = (short int *)pdp_packet_data(x->x_packet0);
    t_pdp     *newheader = pdp_packet_header(x->x_packet1);
    short int *newdata = (short int *)pdp_packet_data(x->x_packet1);
    t_int     px, py;
    t_float   alpha, factor;
    unsigned  char y, u, v;
    short int *pY, *pU, *pV;

    if ( ( (int)(header->info.image.width) != x->x_vwidth ) ||
         ( (int)(header->info.image.height) != x->x_vheight ) )
    {
         x->x_vwidth = header->info.image.width;
         x->x_vheight = header->info.image.height;
         x->x_vsize = x->x_vwidth*x->x_vheight;
    }

    newheader->info.image.encoding = header->info.image.encoding;
    newheader->info.image.width = x->x_vwidth;
    newheader->info.image.height = x->x_vheight;

    memcpy( newdata, data, (x->x_vsize+(x->x_vsize>>1))<<1 );

    pY = newdata;
    pV = newdata+x->x_vsize;
    pU = newdata+x->x_vsize+(x->x_vsize>>2);
    for ( py=0; py<x->x_vheight; py++ )
    {
      for ( px=0; px<x->x_vwidth; px++ )
      {
        if ( ( x->x_image != NULL ) 
             && (px >= x->x_xoffset) && ( px < x->x_xoffset + x->x_iwidth )
	     && (py >= x->x_yoffset) && ( py < x->x_yoffset + x->x_iheight ) 
           )
        {
            y = yuv_RGBtoY(x->x_imdata[(py-x->x_yoffset)*x->x_iwidth+(px-x->x_xoffset)]);
            u = yuv_RGBtoU(x->x_imdata[(py-x->x_yoffset)*x->x_iwidth+(px-x->x_xoffset)]);
            v = yuv_RGBtoV(x->x_imdata[(py-x->x_yoffset)*x->x_iwidth+(px-x->x_xoffset)]);


	    if ( imlib_image_has_alpha() )
	    {
	      alpha = (x->x_imdata[(py-x->x_yoffset)*x->x_iwidth+(px-x->x_xoffset)] >> 24)/255; 
	    }
	    else
	    {
              alpha = 1.0;
	    }
            factor = x->x_blend*alpha;
          
            *(pY) = (int)((1-factor)*(*(pY)) + factor*(y<<7));
            if ( (px%2==0) && (py%2==0) )
            {
              *(pV) = (int)((1-factor)*(*(pV)) + factor*((v-128)<<8));
              *(pU) = (int)((1-factor)*(*(pU)) + factor*((u-128)<<8));
            }
        }
        pY++;
        if ( (px%2==0) && (py%2==0) )
        {
          pV++;pU++;
        }
      }
    }

    return;
}

static void pdp_imgloader_sendpacket(t_pdp_imgloader *x)
{
    /* delete source packet */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = -1;

    /* unregister and propagate if valid dest packet */
    pdp_packet_pass_if_valid(x->x_outlet0, &x->x_packet1);
}

static void pdp_imgloader_process(t_pdp_imgloader *x)
{
   int encoding;
   t_pdp *header = 0;

   /* check if image data packets are compatible */
   if ( (header = pdp_packet_header(x->x_packet0))
	&& (PDP_IMAGE == header->type)){
    
	/* pdp_imgloader_process inputs and write into active inlet */
	switch(pdp_packet_header(x->x_packet0)->info.image.encoding)
        {

	  case PDP_IMAGE_YV12:
            x->x_packet1 = pdp_packet_clone_rw(x->x_packet0);
            pdp_queue_add(x, pdp_imgloader_process_yv12, pdp_imgloader_sendpacket, &x->x_queue_id);
	    break;

	  case PDP_IMAGE_GREY:
            // should write something to handle these one day
            // but i don't use this mode                      
	    break;

	  default:
	    /* don't know the type, so dont pdp_imgloader_process */
	    break;
	    
	}
    }

}

static void pdp_imgloader_input_0(t_pdp_imgloader *x, t_symbol *s, t_floatarg f)
{

    /* if this is a register_ro message or register_rw message, register with packet factory */

    if (s== gensym("register_rw")) 
       x->x_dropped = pdp_packet_convert_ro_or_drop(&x->x_packet0, (int)f, pdp_gensym("image/YCrCb/*") );

    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped)){

        /* add the process method and callback to the process queue */
        pdp_imgloader_process(x);

    }

}

static void pdp_imgloader_free(t_pdp_imgloader *x)
{
  int i;

    pdp_queue_finish(x->x_queue_id);
    pdp_packet_mark_unused(x->x_packet0);
}

t_class *pdp_imgloader_class;

void *pdp_imgloader_new(void)
{
    int i;

    t_pdp_imgloader *x = (t_pdp_imgloader *)pd_new(pdp_imgloader_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("xoffset"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("yoffset"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("blend"));

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 
    x->x_packet0 = -1;
    x->x_packet1 = -1;
    x->x_queue_id = -1;
    x->x_image = NULL;

    x->x_blend = 1;

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_imgloader_setup(void)
{

    post( pdp_imgloader_version );
    pdp_imgloader_class = class_new(gensym("pdp_imgloader"), (t_newmethod)pdp_imgloader_new,
    	(t_method)pdp_imgloader_free, sizeof(t_pdp_imgloader), 0, A_NULL);

    class_addmethod(pdp_imgloader_class, (t_method)pdp_imgloader_input_0, gensym("pdp"),  
                             A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_imgloader_class, (t_method)pdp_imgloader_load, gensym("load"),  A_SYMBOL, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_imgloader_class, (t_method)pdp_imgloader_clear, gensym("clear"),  A_NULL);
    class_addmethod(pdp_imgloader_class, (t_method)pdp_imgloader_xoffset, gensym("xoffset"),  A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_imgloader_class, (t_method)pdp_imgloader_yoffset, gensym("yoffset"),  A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_imgloader_class, (t_method)pdp_imgloader_blend, gensym("blend"),  A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
