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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <dlfcn.h>
#include <ctype.h>

#include "pdp.h"

#ifndef _EiC
#include "cv.h"
#endif

typedef struct pdp_opencv_bm_struct
{
  t_object x_obj;
  t_float x_f;

  t_outlet *x_outlet0;
  t_outlet *x_outlet1;
  t_atom x_list[3];

  int x_packet0;
  int x_packet1;
  int x_dropped;
  int x_queue_id;

  int x_width;
  int x_height;
  int x_size;

  // OpenCv structures
  IplImage *image, *grey, *prev_grey, *swap_temp;
  IplImage *x_velx, *x_vely;
  CvSize x_blocksize, x_shiftsize, x_maxrange, x_velsize;

  int x_nightmode;
  int x_threshold;
  CvFont font;

} t_pdp_opencv_bm;

static void pdp_opencv_bm_process_rgb(t_pdp_opencv_bm *x)
{
    t_pdp     *header = pdp_packet_header(x->x_packet0);
    short int *data   = (short int *)pdp_packet_data(x->x_packet0);
    t_pdp     *newheader = pdp_packet_header(x->x_packet1);
    short int *newdata = (short int *)pdp_packet_data(x->x_packet1); 
    int i,j,k,im;
    int marked;
    int px,py;

    if ((x->x_width != (t_int)header->info.image.width) || 
        (x->x_height != (t_int)header->info.image.height) || (!x->image)) 
    {

      post("pdp_opencv_bm :: resizing plugins");
  
      x->x_width = header->info.image.width;
      x->x_height = header->info.image.height;
      x->x_size = x->x_width*x->x_height;

      x->x_velsize.width = (x->x_width-x->x_blocksize.width)/x->x_shiftsize.width; 
      x->x_velsize.height = (x->x_height-x->x_blocksize.height)/x->x_shiftsize.height; 
      x->x_maxrange.width = x->x_width;
      x->x_maxrange.height = x->x_height;
    
      //Destroy cv_images
      cvReleaseImage( &x->image );
      cvReleaseImage( &x->grey );
      cvReleaseImage( &x->prev_grey );
      cvReleaseImage( &x->x_velx );
      cvReleaseImage( &x->x_vely );
   
      //Create cv_images 
      x->image = cvCreateImage( cvSize(x->x_width, x->x_height), 8, 3 );
      x->grey = cvCreateImage( cvSize(x->x_width, x->x_height), 8, 1 );
      x->prev_grey = cvCreateImage( cvSize(x->x_width, x->x_height), 8, 1 );

      x->x_velx = cvCreateImage( x->x_velsize, IPL_DEPTH_32F, 1 );
      x->x_vely = cvCreateImage( x->x_velsize, IPL_DEPTH_32F, 1 );
    }
    
    newheader->info.image.encoding = header->info.image.encoding;
    newheader->info.image.width = x->x_width;
    newheader->info.image.height = x->x_height;

    memcpy( newdata, data, x->x_size*3 );
    
    memcpy( x->image->imageData, data, x->x_size*3 );
        
    cvCvtColor( x->image, x->grey, CV_RGB2GRAY );

    if( x->x_nightmode )
        cvZero( x->image );
        
    cvCalcOpticalFlowBM( x->prev_grey, x->grey, 
                         x->x_blocksize, x->x_shiftsize,
                         x->x_maxrange, 1,
                         x->x_velx, x->x_vely  );

    for( py=0; py<x->x_velsize.height; py++ ) 
    {
      for( px=0; px<x->x_velsize.width; px++ )
      {
        float velxf = (float)*( x->x_velx->imageData + py * x->x_velx->widthStep + px);
        float velyf = (float)*( x->x_vely->imageData + py * x->x_vely->widthStep + px);

        if ( sqrt( velxf*velxf + velyf*velyf ) > x->x_threshold )
        {
          // post( "pdp_opencv_bm : (%d,%d) values (%f,%f)", px, py, velxf, velyf );
          CvPoint orig, dest;
          orig.x = px*x->x_shiftsize.width;
          orig.y = py*x->x_shiftsize.height;
          dest.x = px*x->x_shiftsize.width+(int)velxf;
          dest.y = py*x->x_shiftsize.height+(int)velyf;

          cvLine( x->image, orig, dest, CV_RGB(0,255,0), 1, 8 );
        }
      }
    }

    CV_SWAP( x->prev_grey, x->grey, x->swap_temp );

    memcpy( newdata, x->image->imageData, x->x_size*3 );
    return;
}

static void pdp_opencv_bm_nightmode(t_pdp_opencv_bm *x, t_floatarg f)
{
  if ((f==0.0)||(f==1.0)) x->x_nightmode = (int)f;
}

static void pdp_opencv_bm_threshold(t_pdp_opencv_bm *x, t_floatarg f)
{
  if (f>=0.0) x->x_threshold = (int)f;
}

static void pdp_opencv_bm_blocksize(t_pdp_opencv_bm *x, t_floatarg fwidth, t_floatarg fheight )
{
  if (fwidth>=1.0) x->x_blocksize.width = (int)fwidth;
  if (fheight>=1.0) x->x_blocksize.height = (int)fheight;

  x->x_velsize.width = (x->x_width-x->x_blocksize.width)/x->x_shiftsize.width; 
  x->x_velsize.height = (x->x_height-x->x_blocksize.height)/x->x_shiftsize.height; 
  cvReleaseImage( &x->x_velx );
  cvReleaseImage( &x->x_vely );
  x->x_velx = cvCreateImage( x->x_velsize, IPL_DEPTH_32F, 1 );
  x->x_vely = cvCreateImage( x->x_velsize, IPL_DEPTH_32F, 1 );
    
}

static void pdp_opencv_bm_shiftsize(t_pdp_opencv_bm *x, t_floatarg fwidth, t_floatarg fheight )
{
  if (fwidth>=1.0) x->x_shiftsize.width = (int)fwidth;
  if (fheight>=1.0) x->x_shiftsize.height = (int)fheight;

  x->x_velsize.width = (x->x_width-x->x_blocksize.width)/x->x_shiftsize.width; 
  x->x_velsize.height = (x->x_height-x->x_blocksize.height)/x->x_shiftsize.height; 
  cvReleaseImage( &x->x_velx );
  cvReleaseImage( &x->x_vely );
  x->x_velx = cvCreateImage( x->x_velsize, IPL_DEPTH_32F, 1 );
  x->x_vely = cvCreateImage( x->x_velsize, IPL_DEPTH_32F, 1 );
}

static void pdp_opencv_bm_maxrange(t_pdp_opencv_bm *x, t_floatarg fwidth, t_floatarg fheight )
{
  if (fwidth>=1.0) x->x_maxrange.width = (int)fwidth;
  if (fheight>=1.0) x->x_maxrange.height = (int)fheight;
}

static void pdp_opencv_bm_sendpacket(t_pdp_opencv_bm *x)
{
  /* release the packet */
  pdp_packet_mark_unused(x->x_packet0);
  x->x_packet0 = -1;

  /* unregister and propagate if valid dest packet */
  pdp_packet_pass_if_valid(x->x_outlet0, &x->x_packet1);
}

static void pdp_opencv_bm_process(t_pdp_opencv_bm *x)
{
   int encoding;
   t_pdp *header = 0;

   /* check if image data packets are compatible */
   if ( (header = pdp_packet_header(x->x_packet0))
     && (PDP_BITMAP == header->type)){
    
     /* pdp_opencv_bm_process inputs and write into active inlet */
     switch(pdp_packet_header(x->x_packet0)->info.image.encoding){

     case PDP_BITMAP_RGB:
            x->x_packet1 = pdp_packet_clone_rw(x->x_packet0);
            pdp_queue_add(x, (void*)pdp_opencv_bm_process_rgb, (void*)pdp_opencv_bm_sendpacket, &x->x_queue_id);
      break;

     default:
      /* don't know the type, so dont pdp_opencv_bm_process */
      break;
      
     }
   }

}

static void pdp_opencv_bm_input_0(t_pdp_opencv_bm *x, t_symbol *s, t_floatarg f)
{
    /* if this is a register_ro message or register_rw message, register with packet factory */

    if (s == gensym("register_rw")) 
       x->x_dropped = pdp_packet_convert_ro_or_drop(&x->x_packet0, (int)f, pdp_gensym((char*)"bitmap/rgb/*") );

    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped))
    {
        /* add the process method and callback to the process queue */
        pdp_opencv_bm_process(x);
    }
}

static void pdp_opencv_bm_free(t_pdp_opencv_bm *x)
{
  int i;

    pdp_queue_finish(x->x_queue_id);
    pdp_packet_mark_unused(x->x_packet0);
    //cv_freeplugins(x);
    
    //Destroy cv_images
    cvReleaseImage( &x->image );
    cvReleaseImage( &x->grey );
    cvReleaseImage( &x->prev_grey );
    cvReleaseImage( &x->x_velx );
    cvReleaseImage( &x->x_vely );
}

t_class *pdp_opencv_bm_class;

void *pdp_opencv_bm_new(t_floatarg f)
{
  int i;

  t_pdp_opencv_bm *x = (t_pdp_opencv_bm *)pd_new(pdp_opencv_bm_class);

  x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 
  x->x_outlet1 = outlet_new(&x->x_obj, &s_anything);

  x->x_packet0 = -1;
  x->x_packet1 = -1;
  x->x_queue_id = -1;

  x->x_width  = 320;
  x->x_height = 240;
  x->x_size   = x->x_width * x->x_height;

  x->x_nightmode=0;
  x->x_threshold=10;
  x->x_blocksize.width = 10;
  x->x_blocksize.height = 10;
  x->x_shiftsize.width = 10;
  x->x_shiftsize.height = 10;
  x->x_maxrange.width = x->x_width;
  x->x_maxrange.height = x->x_height;
  x->x_velsize.width = (x->x_width-x->x_blocksize.width)/x->x_shiftsize.width; 
  x->x_velsize.height = (x->x_height-x->x_blocksize.height)/x->x_shiftsize.height; 

  // initialize font
  cvInitFont( &x->font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0, 1, 8 );
    
  x->image = cvCreateImage( cvSize(x->x_width, x->x_height), 8, 3 );
  x->grey = cvCreateImage( cvSize(x->x_width, x->x_height), 8, 1 );
  x->prev_grey = cvCreateImage( cvSize(x->x_width, x->x_height), 8, 1 );

  x->x_velx = cvCreateImage( x->x_velsize, IPL_DEPTH_32F, 1 );
  x->x_vely = cvCreateImage( x->x_velsize, IPL_DEPTH_32F, 1 );

  return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_opencv_bm_setup(void)
{

    post( "    pdp_opencv_bm");
    pdp_opencv_bm_class = class_new(gensym("pdp_opencv_bm"), (t_newmethod)pdp_opencv_bm_new,
      (t_method)pdp_opencv_bm_free, sizeof(t_pdp_opencv_bm), 0, A_DEFFLOAT, A_NULL);

    class_addmethod(pdp_opencv_bm_class, (t_method)pdp_opencv_bm_input_0, gensym("pdp"), A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_opencv_bm_class, (t_method)pdp_opencv_bm_nightmode, gensym("nightmode"), A_FLOAT, A_NULL );   
    class_addmethod(pdp_opencv_bm_class, (t_method)pdp_opencv_bm_threshold, gensym("threshold"), A_FLOAT, A_NULL );   
    class_addmethod(pdp_opencv_bm_class, (t_method)pdp_opencv_bm_blocksize, gensym("blocksize"), A_FLOAT, A_FLOAT, A_NULL );   
    class_addmethod(pdp_opencv_bm_class, (t_method)pdp_opencv_bm_shiftsize, gensym("shiftsize"), A_FLOAT, A_FLOAT, A_NULL );   
    class_addmethod(pdp_opencv_bm_class, (t_method)pdp_opencv_bm_maxrange, gensym("maxrange"), A_FLOAT, A_FLOAT, A_NULL );   

}

#ifdef __cplusplus
}
#endif
