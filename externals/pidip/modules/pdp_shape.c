/*
 *   PiDiP module.
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

/*  This object is a shape recognition object
 *  Written by Yves Degoyon                                  
 */

#include "pdp.h"
#include "yuv.h"
#include <math.h>

static char   *pdp_shape_version = "pdp_shape: version 0.1, shape recongnition object written by Yves Degoyon (ydegoyon@free.fr)";

typedef struct pdp_shape_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;
    t_int x_packet0;
    t_int x_packet1;
    t_int x_dropped;
    t_int x_queue_id;

    t_int x_vwidth;
    t_int x_vheight;
    t_int x_vsize;

    t_int x_red;
    t_int x_green;
    t_int x_blue;

    t_int x_cursX;
    t_int x_cursY;
    
    t_int x_colorY; // YUV components of selected color
    t_int x_colorU;
    t_int x_colorV;

    t_int x_tolerance; // tolerance
    t_int x_paint;     // paint option
    t_int x_luminosity; // use luminosity or not

    short int *x_bdata;
    short int *x_bbdata;
    char      *x_checked;

    t_outlet *x_x1; // output x1 coordinate of blob
    t_outlet *x_y1; // output y1 coordinate of blob
    t_outlet *x_x2; // output x2 coordinate of blob
    t_outlet *x_y2; // output y2 coordinate of blob
    t_int    x_vx1; // x1 coordinate of blob
    t_int    x_vx2; // x1 coordinate of blob
    t_int    x_vy1; // x1 coordinate of blob
    t_int    x_vy2; // x1 coordinate of blob


} t_pdp_shape;

static void pdp_shape_allocate(t_pdp_shape *x, t_int newsize)
{
 int i;

  if ( x->x_bdata ) freebytes( x->x_bdata, (( x->x_vsize + (x->x_vsize>>1))<<1));
  if ( x->x_bbdata ) freebytes( x->x_bbdata, (( x->x_vsize + (x->x_vsize>>1))<<1));
  if ( x->x_checked ) freebytes( x->x_checked, x->x_vsize );

  x->x_vsize = newsize;
 
  x->x_bdata = (short int *)getbytes((( x->x_vsize + (x->x_vsize>>1))<<1));
  x->x_bbdata = (short int *)getbytes((( x->x_vsize + (x->x_vsize>>1))<<1));
  x->x_checked = (char *)getbytes( x->x_vsize );
}

static void pdp_shape_tolerance(t_pdp_shape *x, t_floatarg ftolerance )
{
   if ( ftolerance >= 0 )
   {
      x->x_tolerance = (int)ftolerance;
   }
}

static void pdp_shape_luminosity(t_pdp_shape *x, t_floatarg fluminosity )
{
   if ( ( fluminosity == 0 ) || ( fluminosity == 1 ) )
   {
      x->x_luminosity = (int)fluminosity;
   }
}

static void pdp_shape_paint(t_pdp_shape *x, t_floatarg fpaint )
{
   if ( ( (t_int)fpaint == 0 ) || ( (t_int)fpaint == 1 ) )
   {
      x->x_paint = (t_int)fpaint;
   }
}

static void pdp_shape_do_detect(t_pdp_shape *x, t_floatarg X, t_floatarg Y);
static void pdp_shape_frame_detect(t_pdp_shape *x, t_floatarg X, t_floatarg Y);

static t_int pdp_shape_check_point(t_pdp_shape *x, t_int nX, t_int nY)
{
 short int  *pbY, *pbU, *pbV;
 short int  y, v, u;
 t_int      diff;

  if ( ( nX < 0 ) || ( nX >= x->x_vwidth ) || 
       ( nY < 0 ) || ( nY >= x->x_vheight ) )
  {
    return 0;
  }

  pbY = x->x_bdata;
  pbU = (x->x_bdata+x->x_vsize);
  pbV = (x->x_bdata+x->x_vsize+(x->x_vsize>>2));
  y = *(pbY+nY*x->x_vwidth+nX);
  v = *(pbU+(nY>>1)*(x->x_vwidth>>1)+(nX>>1));
  u = *(pbV+(nY>>1)*(x->x_vwidth>>1)+(nX>>1));
  diff = (abs(u-x->x_colorU)>>8)+(abs(v-x->x_colorV)>>8);
  if ( x->x_luminosity ) diff += (abs(y-x->x_colorY)>>7);
  if ( diff <= x->x_tolerance )
  {
    x->x_cursX = nX;
    x->x_cursY = nY;
    return 1;
  }
  return 0;
}

static void pdp_shape_propagate(t_pdp_shape *x, t_int nX, t_int nY)
{

  if ( ( nX >= 0 ) && ( nX < x->x_vwidth ) && 
       ( nY >= 0 ) && ( nY < x->x_vheight ) &&
       ( !*(x->x_checked + nY*x->x_vwidth + nX) )  
     )
  {
    pdp_shape_do_detect( x, nX, nY );
  }
}

static void pdp_shape_do_detect(t_pdp_shape *x, t_floatarg X, t_floatarg Y)
{
 short int  *pbY, *pbU, *pbV;
 short int  *pbbY, *pbbU, *pbbV;
 short int  nX, nY, y, v, u;
 short int *data;
 t_int      diff, px, py, inc, maxXY;

  pbY = x->x_bdata;
  pbU = (x->x_bdata+x->x_vsize);
  pbV = (x->x_bdata+x->x_vsize+(x->x_vsize>>2));
  pbbY = x->x_bbdata;
  pbbU = (x->x_bbdata+x->x_vsize);
  pbbV = (x->x_bbdata+x->x_vsize+(x->x_vsize>>2));

  if ( ( (t_int)X < 0 ) || ( (t_int)X >= x->x_vwidth ) || 
       ( (t_int)Y < 0 ) || ( (t_int)Y >= x->x_vheight ) )
  {
     return;
  }

  nX = (t_int) X; 
  nY = (t_int) Y; 
  *(x->x_checked + nY*x->x_vwidth + nX) = 1;

  y = *(pbY+nY*x->x_vwidth+nX);
  v = *(pbU+(nY>>1)*(x->x_vwidth>>1)+(nX>>1));
  u = *(pbV+(nY>>1)*(x->x_vwidth>>1)+(nX>>1));
  diff = (abs(u-x->x_colorU)>>8)+(abs(v-x->x_colorV)>>8);
  if ( x->x_luminosity ) diff += (abs(y-x->x_colorY)>>7);
  if ( diff > x->x_tolerance )
  {
     // paint it white
     // post( "pdp_shape_do_detect : paint : %d %d", nX, nY );
     *(pbbY+nY*x->x_vwidth+nX) = (0xff<<7);
     *(pbbU+(nY>>1)*(x->x_vwidth>>1)+(nX>>1)) = (0xff<<8);
     *(pbbV+(nY>>1)*(x->x_vwidth>>1)+(nX>>1)) = (0xff<<8);

     if ( ( nX < x->x_vx1 ) || ( x->x_vx1 == -1 ) )
     {
        x->x_vx1 = nX;
     }
     if ( ( nX > x->x_vx2 ) || ( x->x_vx2 == -1 ) )
     {
        x->x_vx2 = nX;
     }
     if ( ( nY < x->x_vy1 ) || ( x->x_vy1 == -1 ) )
     {
        x->x_vy1 = nY;
     }
     if ( ( nY > x->x_vy2 ) || ( x->x_vy2 == -1 ) )
     {
        x->x_vy2 = nY;
     }

     maxXY = ( x->x_vwidth > x->x_vheight ) ? x->x_vwidth : x->x_vheight;

     if ( ( nX == x->x_cursX ) && ( nY == x->x_cursY ) )
     {
       for ( inc=0; inc<=maxXY; inc++ )
       {
        nX = (t_int) X+inc; 
        nY = (t_int) Y; 
        if ( pdp_shape_check_point( x, nX, nY ) )
        {
          pdp_shape_frame_detect( x, x->x_cursX, x->x_cursY );
          return;
        }
          
        nX = (t_int) X-inc; 
        nY = (t_int) Y; 
        if ( pdp_shape_check_point( x, nX, nY ) )
        {
          pdp_shape_frame_detect( x, x->x_cursX, x->x_cursY );
          return;
        }
      
        nX = (t_int) X-inc; 
        nY = (t_int) Y-inc; 
        if ( pdp_shape_check_point( x, nX, nY ) )
        {
          pdp_shape_frame_detect( x, x->x_cursX, x->x_cursY );
          return;
        }
      
        nX = (t_int) X; 
        nY = (t_int) Y-inc; 
        if ( pdp_shape_check_point( x, nX, nY ) )
        {
          pdp_shape_frame_detect( x, x->x_cursX, x->x_cursY );
          return;
        }
      
        nX = (t_int) X+inc; 
        nY = (t_int) Y-inc; 
        if ( pdp_shape_check_point( x, nX, nY ) )
        {
          pdp_shape_frame_detect( x, x->x_cursX, x->x_cursY );
          return;
        }
      
        nX = (t_int) X-inc; 
        nY = (t_int) Y+inc; 
        if ( pdp_shape_check_point( x, nX, nY ) )
        {
          pdp_shape_frame_detect( x, x->x_cursX, x->x_cursY );
          return;
        }
      
        nX = (t_int) X; 
        nY = (t_int) Y+inc; 
        if ( pdp_shape_check_point( x, nX, nY ) )
        {
          pdp_shape_frame_detect( x, x->x_cursX, x->x_cursY );
          return;
        }
      
        nX = (t_int) X+inc; 
        nY = (t_int) Y+inc; 
        if ( pdp_shape_check_point( x, nX, nY ) )
        {
          pdp_shape_frame_detect( x, x->x_cursX, x->x_cursY );
          return;
        }
      
       }
     }
     return;
  }
  else
  {
     if ( x->x_paint )
     {
       *(pbbY+nY*x->x_vwidth+nX) =
          (yuv_RGBtoY( (x->x_blue << 16) + (x->x_green << 8) + x->x_red ))<<7;
       *(pbbU+(nY>>1)*(x->x_vwidth>>1)+(nX>>1)) =
          (yuv_RGBtoU( (x->x_blue << 16) + (x->x_green << 8) + x->x_red ))-128<<8;
       *(pbbV+(nY>>1)*(x->x_vwidth>>1)+(nX>>1)) =
          (yuv_RGBtoV( (x->x_blue << 16) + (x->x_green << 8) + x->x_red ))-128<<8;
     }
  }

  nX = (t_int) X+1; 
  nY = (t_int) Y; 
  pdp_shape_propagate(x, nX, nY);
    
  nX = (t_int) X-1; 
  nY = (t_int) Y; 
  pdp_shape_propagate(x, nX, nY);

  nX = (t_int) X-1; 
  nY = (t_int) Y-1; 
  pdp_shape_propagate(x, nX, nY);

  nX = (t_int) X; 
  nY = (t_int) Y-1; 
  pdp_shape_propagate(x, nX, nY);

  nX = (t_int) X+1; 
  nY = (t_int) Y-1; 
  pdp_shape_propagate(x, nX, nY);

  nX = (t_int) X-1; 
  nY = (t_int) Y+1; 
  pdp_shape_propagate(x, nX, nY);

  nX = (t_int) X; 
  nY = (t_int) Y+1; 
  pdp_shape_propagate(x, nX, nY);

  nX = (t_int) X+1; 
  nY = (t_int) Y+1; 
  pdp_shape_propagate(x, nX, nY);

}

static void pdp_shape_pick(t_pdp_shape *x, t_floatarg X, t_floatarg Y)
{
 t_int y,u,v;

   x->x_cursX = (t_int) (X*(t_float)x->x_vwidth);
   x->x_cursY = (t_int) (Y*(t_float)x->x_vheight);
   // post( "pdp_shape : pick color at : %d,%d", x->x_cursX, x->x_cursY );
   if ( ( x->x_cursX >= 0 ) && ( x->x_cursX < x->x_vwidth )
        && ( x->x_cursY >= 0 ) && ( x->x_cursY < x->x_vheight ) )
   {
      x->x_colorY = *(x->x_bdata + x->x_cursY*x->x_vwidth+x->x_cursX);;
      x->x_colorV = (*(x->x_bdata + x->x_vsize + (x->x_cursY>>1)*(x->x_vwidth>>1)+(x->x_cursX>>1)));
      x->x_colorU = 
            (*(x->x_bdata + x->x_vsize + (x->x_vsize>>2) + (x->x_cursY>>1)*(x->x_vwidth>>1)+(x->x_cursX>>1)));
      y = x->x_colorY>>7;
      v = (x->x_colorV>>8)+128;
      u = (x->x_colorU>>8)+128; 
      x->x_red = yuv_YUVtoR( y, u, v );
      x->x_green = yuv_YUVtoG( y, u, v );
      x->x_blue = yuv_YUVtoB( y, u, v );
      // post( "pdp_shape : picked color set to : %d,%d,%d", x->x_red, x->x_green, x->x_blue );
   }
}

static void pdp_shape_frame_detect(t_pdp_shape *x, t_floatarg X, t_floatarg Y)
{
   if ( x->x_bdata == NULL ) return;

   // post( "pdp_shape : detect %d %d", (t_int)x->x_cursX, (t_int)x->x_cursY );
   x->x_vx1 = -1; 
   x->x_vx2 = -1; 
   x->x_vy1 = -1; 
   x->x_vy2 = -1; 
   memset( x->x_checked, 0x0, x->x_vsize );
   pdp_shape_do_detect( x, x->x_cursX, x->x_cursY );

   outlet_float( x->x_x1, x->x_vx1 );
   outlet_float( x->x_y1, x->x_vy1 );
   outlet_float( x->x_x2, x->x_vx2 );
   outlet_float( x->x_y2, x->x_vy2 );
}

static void pdp_shape_detect(t_pdp_shape *x, t_floatarg X, t_floatarg Y)
{
 t_int wX, wY;

   wX = (t_int) (X*(t_float)x->x_vwidth);
   wY = (t_int) (Y*(t_float)x->x_vheight);
   // post( "pdp_shape : detect %d %d", wX, wY );
   if ( (wX<0) || (wX>x->x_vwidth) )
   {
     // post( "pdp_shape : fill : wrong X position : %f", wX );
     return;
   }  
   if ( (wY<0) || (wY>x->x_vheight) )
   {
     // post( "pdp_shape : fill : wrong Y position : %f", wY );
     return;
   }  

   x->x_cursX = wX;
   x->x_cursY = wY;
}

static void pdp_shape_rgb(t_pdp_shape *x, t_floatarg r, t_floatarg g, t_floatarg b)
{
  if ( ( r >= 0. ) && ( r <= 255. ) &&
       ( g >= 0. ) && ( g <= 255. ) &&
       ( b >= 0. ) && ( b <= 255. ) ) 
  {
    x->x_red = (int) r;
    x->x_green = (int) g;
    x->x_blue = (int) b;
  }
}

static void pdp_shape_process_yv12(t_pdp_shape *x)
{
    t_pdp     *header = pdp_packet_header(x->x_packet0);
    short int *data   = (short int *)pdp_packet_data(x->x_packet0);
    t_pdp     *newheader = pdp_packet_header(x->x_packet1);
    short int *newdata = (short int *)pdp_packet_data(x->x_packet1);
    short int  *pbbY, *pbbU, *pbbV;

    /* allocate all ressources */
    if ( ((t_int)header->info.image.width != x->x_vwidth ) ||
         ((t_int)header->info.image.height != x->x_vheight ) ) 
    {
        pdp_shape_allocate(x, header->info.image.width*header->info.image.height );
        post( "pdp_shape : reallocating buffers" );
    }

    x->x_vwidth = header->info.image.width;
    x->x_vheight = header->info.image.height;
    x->x_vsize = x->x_vwidth*x->x_vheight;

    newheader->info.image.encoding = header->info.image.encoding;
    newheader->info.image.width = x->x_vwidth;
    newheader->info.image.height = x->x_vheight;

    memcpy( x->x_bdata, data, (x->x_vsize+(x->x_vsize>>1))<<1 );
    memcpy( x->x_bbdata, data, (x->x_vsize+(x->x_vsize>>1))<<1 );

    if ( x->x_cursX != -1 ) pdp_shape_frame_detect( x, x->x_cursX, x->x_cursY );
  
    // paint cursor in red for debug purpose
    pbbY = x->x_bbdata;
    pbbU = (x->x_bbdata+x->x_vsize);
    pbbV = (x->x_bbdata+x->x_vsize+(x->x_vsize>>2));
    *(pbbY+x->x_cursY*x->x_vwidth+x->x_cursX) = (yuv_RGBtoY( 0xff ))<<7;
    *(pbbU+(x->x_cursY>>1)*(x->x_vwidth>>1)+(x->x_cursX>>1)) = ((yuv_RGBtoU( 0xff )-128)<<8); 
    *(pbbV+(x->x_cursY>>1)*(x->x_vwidth>>1)+(x->x_cursX>>1)) = ((yuv_RGBtoV( 0xff )-128)<<8);

    memcpy( newdata, x->x_bbdata, (x->x_vsize+(x->x_vsize>>1))<<1 );

    return;
}

static void pdp_shape_sendpacket(t_pdp_shape *x)
{
    /* release the packet */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = -1;

    /* unregister and propagate if valid dest packet */
    pdp_packet_pass_if_valid(x->x_outlet0, &x->x_packet1);
}

static void pdp_shape_process(t_pdp_shape *x)
{
   int encoding;
   t_pdp *header = 0;

   /* check if image data packets are compatible */
   if ( (header = pdp_packet_header(x->x_packet0))
	&& (PDP_IMAGE == header->type))
   {
	/* pdp_shape_process inputs and write into active inlet */
	switch(pdp_packet_header(x->x_packet0)->info.image.encoding){

	case PDP_IMAGE_YV12:
            x->x_packet1 = pdp_packet_clone_rw(x->x_packet0);
            pdp_queue_add(x, pdp_shape_process_yv12, pdp_shape_sendpacket, &x->x_queue_id);
	    break;

	case PDP_IMAGE_GREY:
	    // pdp_shape_process_packet(x);
	    break;

	default:
	    /* don't know the type, so dont pdp_shape_process */
	    break;
	    
	}
    }
}

static void pdp_shape_input_0(t_pdp_shape *x, t_symbol *s, t_floatarg f)
{
    /* if this is a register_ro message or register_rw message, register with packet factory */

    if (s== gensym("register_rw"))
    {
       x->x_dropped = pdp_packet_convert_ro_or_drop(&x->x_packet0, (int)f, pdp_gensym("image/YCrCb/*") );
    }

    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped))
    {
        /* add the process method and callback to the process queue */
        pdp_shape_process(x);
    }
}

static void pdp_shape_free(t_pdp_shape *x)
{
  int i;

    pdp_queue_finish(x->x_queue_id);
    pdp_packet_mark_unused(x->x_packet0);

}

t_class *pdp_shape_class;

void *pdp_shape_new(void)
{
    int i;

    t_pdp_shape *x = (t_pdp_shape *)pd_new(pdp_shape_class);

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_x1 = outlet_new(&x->x_obj, &s_float);
    x->x_y1 = outlet_new(&x->x_obj, &s_float);
    x->x_x2 = outlet_new(&x->x_obj, &s_float);
    x->x_y2 = outlet_new(&x->x_obj, &s_float);

    x->x_packet0 = -1;
    x->x_packet1 = -1;
    x->x_queue_id = -1;

    x->x_vsize = -1;

    x->x_red = 255;
    x->x_green = 255;
    x->x_blue = 255;

    x->x_tolerance = 20;
    x->x_paint = 0;
    x->x_luminosity = 1;

    x->x_cursX = -1;
    x->x_cursY = -1;

    x->x_bdata = NULL;
    x->x_bbdata = NULL;
    x->x_checked = NULL;

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_shape_setup(void)
{
//    post( pdp_shape_version );
    pdp_shape_class = class_new(gensym("pdp_shape"), (t_newmethod)pdp_shape_new,
    	(t_method)pdp_shape_free, sizeof(t_pdp_shape), 0, A_NULL);
    class_sethelpsymbol( pdp_shape_class, gensym("pdp_shape.pd") );

    class_addmethod(pdp_shape_class, (t_method)pdp_shape_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_shape_class, (t_method)pdp_shape_pick, gensym("pick"), A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_shape_class, (t_method)pdp_shape_detect, gensym("detect"), A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_shape_class, (t_method)pdp_shape_rgb, gensym("rgb"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_shape_class, (t_method)pdp_shape_tolerance, gensym("tolerance"), A_FLOAT, A_NULL);
    class_addmethod(pdp_shape_class, (t_method)pdp_shape_paint, gensym("paint"), A_FLOAT, A_NULL);
    class_addmethod(pdp_shape_class, (t_method)pdp_shape_luminosity, gensym("luminosity"), A_FLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
