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

#include "pdp.h"

#ifndef _EiC
#include "cv.h"
#endif



typedef struct pdp_opencv_floodfill_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;
    int x_packet0;
    int x_packet1;
    int x_dropped;
    int x_queue_id;

    int x_width;
    int x_height;
    int x_size;

    int x_infosok; 

    int up_diff;
    int lo_diff;
    int ffill_case;
    int connectivity;
    int is_color;
    int is_mask;
    int new_mask_val;

    // The output and temporary images

    IplImage* color_img0;
    IplImage* mask;
    IplImage* color_img;
    IplImage* gray_img0;
    IplImage* gray_img;
    
} t_pdp_opencv_floodfill;



static void pdp_opencv_floodfill_process_rgb(t_pdp_opencv_floodfill *x)
{
    t_pdp     *header = pdp_packet_header(x->x_packet0);
    short int *data   = (short int *)pdp_packet_data(x->x_packet0);
    t_pdp     *newheader = pdp_packet_header(x->x_packet1);
    short int *newdata = (short int *)pdp_packet_data(x->x_packet1); 
    int i;
      

    if ((x->x_width != (t_int)header->info.image.width) || 
        (x->x_height != (t_int)header->info.image.height)) 
    {

    	post("pdp_opencv_floodfill :: resizing plugins");
	
    	//cv_freeplugins(x);

    	x->x_width = header->info.image.width;
    	x->x_height = header->info.image.height;
    	x->x_size = x->x_width*x->x_height;
    
    	//Destroy cv_images
    	cvReleaseImage( &x->color_img );
    	cvReleaseImage( &x->color_img0 );
    	cvReleaseImage( &x->mask );
    	cvReleaseImage( &x->gray_img );
    	cvReleaseImage( &x->gray_img0 );

   
	//Create cv_images 
    	x->color_img0 = cvCreateImage( cvSize(x->x_width,x->x_height), 8, 3 );
    	x->color_img = cvCreateImage( cvSize(x->x_width,x->x_height), 8, 3 );
    	x->gray_img0 = cvCreateImage( cvSize(x->x_width, x->x_height), 8, 1 );
    	x->gray_img = cvCreateImage( cvSize(x->x_width, x->x_height), 8, 1 );
    	x->mask = cvCreateImage( cvSize(x->x_width + 2, x->x_height + 2), 8, 1 );
    }
    
    newheader->info.image.encoding = header->info.image.encoding;
    newheader->info.image.width = x->x_width;
    newheader->info.image.height = x->x_height;

    memcpy( newdata, data, x->x_size*3 );
    // FEM UNA COPIA DEL PACKET A x->grey->imageData ... http://www.cs.iit.edu/~agam/cs512/lect-notes/opencv-intro/opencv-intro.html aqui veiem la estructura de IplImage
    memcpy( x->color_img->imageData, data, x->x_size*3 );

    
    cvCvtColor(x->color_img, x->gray_img, CV_BGR2GRAY);


	int px = 0;       
	int py = 0;       
	int biggestNum=0,biggestLocX=0,biggestLocY=0;
	int haveOne=0;
	CvPixelPosition8u sil;
	int stride = x->gray_img->widthStep;
	unsigned char * pI = (unsigned char *)x->gray_img->imageData;
	CV_INIT_PIXEL_POS(sil, pI, x->gray_img->widthStep, cvSize(x->gray_img->width, x->gray_img->height), 0, 0,IPL_ORIGIN_TL);
	CvPoint xy;
        CvPoint seed = cvPoint(px,py);
        int lo = x->ffill_case == 0 ? 0 : x->lo_diff;
        int up = x->ffill_case == 0 ? 0 : x->up_diff;
        int flags = x->connectivity + (x->new_mask_val << 8) +
                        (x->ffill_case == 1 ? CV_FLOODFILL_FIXED_RANGE : 0);
        CvConnectedComp comp;
	int min_area_size = 200;

	for(py=0; py<x->gray_img->height; py++)
	{
		for(px=0; px<x->gray_img->width; px++)
		{
 			if(*(sil.currline + sil.x) != 0) // check if used yet
			{
				xy.x = px;
				xy.y = py;
				cvFloodFill ( x->gray_img, xy, cvRealScalar(100), cvRealScalar(lo), cvRealScalar(up), &comp, flags, NULL );	
				// if size is too small remove that region
				// Also, keep only the biggest region!!!	
				if( ((int)(comp.area)<min_area_size) || ((int)(comp.area)<biggestNum) )
				{
					// remove it
					//cvFloodFill ( x->gray_img, xy, cvRealScalar(0), cvRealScalar(lo), cvRealScalar(up), &comp, flags, NULL );
				} else { // for keeping just the largest
					// remove previous max
					if(haveOne)
					{
						xy.x = biggestLocX;
						xy.y = biggestLocY;
						//cvFloodFill ( x->gray_img, xy, cvRealScalar(0), cvRealScalar(lo), cvRealScalar(up), &comp, flags, NULL );
					} else haveOne=1;
					biggestNum=(int)(comp.area);
					biggestLocX=px;
					biggestLocY=py;
				}
			}
		CV_MOVE_RIGHT_WRAP(sil, 1);
		}
	CV_MOVE_DOWN(sil, 1);
	}
	if(haveOne)
	{
		xy.x = biggestLocX;
		xy.y = biggestLocY;
		//cvFloodFill ( x->gray_img, xy, cvRealScalar(255), cvRealScalar(lo), cvRealScalar(up), &comp, flags, NULL );
	}
            
                

		//CvScalar brightness = cvRealScalar(255);
                //cvFloodFill( x->color_img, seed, CV_RGB(255,255,255), CV_RGB(lo,lo,lo),
                //             CV_RGB(up,up,up), &comp, flags, NULL );

    cvCvtColor(x->gray_img, x->color_img, CV_GRAY2BGR);

    memcpy( newdata, x->color_img->imageData, x->x_size*3 );
    //printf("%g pixels were repainted\n", comp.area );
    return;
}


static void pdp_opencv_floodfill_diff(t_pdp_opencv_floodfill *x, t_floatarg f)
{
	if ((f==1)||(f==3)||(f==5)||(f==7)) x->up_diff = (int)f;
}

static void pdp_opencv_floodfill_sendpacket(t_pdp_opencv_floodfill *x)
{
    /* release the packet */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = -1;

    /* unregister and propagate if valid dest packet */
    pdp_packet_pass_if_valid(x->x_outlet0, &x->x_packet1);
}

static void pdp_opencv_floodfill_process(t_pdp_opencv_floodfill *x)
{
   int encoding;
   t_pdp *header = 0;
   char *parname;
   unsigned pi;
   int partype;
   float pardefault;
   t_atom plist[2];
   t_atom tlist[2];
   t_atom vlist[2];

   /* check if image data packets are compatible */
   if ( (header = pdp_packet_header(x->x_packet0))
	&& (PDP_BITMAP == header->type)){
    
	/* pdp_opencv_floodfill_process inputs and write into active inlet */
	switch(pdp_packet_header(x->x_packet0)->info.image.encoding){

	case PDP_BITMAP_RGB:
            x->x_packet1 = pdp_packet_clone_rw(x->x_packet0);
            pdp_queue_add(x, pdp_opencv_floodfill_process_rgb, pdp_opencv_floodfill_sendpacket, &x->x_queue_id);
	    break;

	default:
	    /* don't know the type, so dont pdp_opencv_floodfill_process */
	    break;
	    
	}
    }

}

static void pdp_opencv_floodfill_input_0(t_pdp_opencv_floodfill *x, t_symbol *s, t_floatarg f)
{
    /* if this is a register_ro message or register_rw message, register with packet factory */

    if (s == gensym("register_rw")) 
       x->x_dropped = pdp_packet_convert_ro_or_drop(&x->x_packet0, (int)f, pdp_gensym("bitmap/rgb/*") );

    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped))
    {
        /* add the process method and callback to the process queue */
        pdp_opencv_floodfill_process(x);
    }
}

static void pdp_opencv_floodfill_free(t_pdp_opencv_floodfill *x)
{
  int i;

    pdp_queue_finish(x->x_queue_id);
    pdp_packet_mark_unused(x->x_packet0);
    //cv_freeplugins(x);
    
    	//Destroy cv_images
    	cvReleaseImage( &x->color_img );
    	cvReleaseImage( &x->color_img0 );
    	cvReleaseImage( &x->mask );
    	cvReleaseImage( &x->gray_img );
    	cvReleaseImage( &x->gray_img0 );
}

t_class *pdp_opencv_floodfill_class;


void *pdp_opencv_floodfill_new(t_floatarg f)
{
    int i;

    t_pdp_opencv_floodfill *x = (t_pdp_opencv_floodfill *)pd_new(pdp_opencv_floodfill_class);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("lo_diff"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("up_diff"));

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;
    x->x_packet1 = -1;
    x->x_queue_id = -1;

    x->x_width  = 320;
    x->x_height = 240;
    x->x_size   = x->x_width * x->x_height;

    x->x_infosok = 0;

    x->ffill_case = 1;
    x->lo_diff = 20; 
    x->up_diff = 20;
    x->connectivity = 4;
    x->is_color = 1;
    x->is_mask = 0;
    x->new_mask_val = 255;

    x->color_img0 = cvCreateImage( cvSize(x->x_width,x->x_height), 8, 3 );
    x->color_img = cvCreateImage( cvSize(x->x_width,x->x_height), 8, 3 );
    x->mask = cvCreateImage( cvSize(x->x_width + 2, x->x_height + 2), 8, 1 );
    x->gray_img0 = cvCreateImage( cvSize(x->x_width, x->x_height), 8, 1 );
    x->gray_img = cvCreateImage( cvSize(x->x_width, x->x_height), 8, 1 );


    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_opencv_floodfill_setup(void)
{

    post( "		pdp_opencv_floodfill");
    pdp_opencv_floodfill_class = class_new(gensym("pdp_opencv_floodfill"), (t_newmethod)pdp_opencv_floodfill_new,
    	(t_method)pdp_opencv_floodfill_free, sizeof(t_pdp_opencv_floodfill), 0, A_DEFFLOAT, A_NULL);

    class_addmethod(pdp_opencv_floodfill_class, (t_method)pdp_opencv_floodfill_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    //class_addmethod(pdp_opencv_floodfill_class, (t_method)pdp_opencv_floodfill_up_diff, gensym("up_diff"),  A_FLOAT, A_NULL );   
    //class_addmethod(pdp_opencv_floodfill_class, (t_method)pdp_opencv_floodfill_lo_diff, gensym("lo_diff"),  A_FLOAT, A_NULL );   

}

#ifdef __cplusplus
}
#endif
