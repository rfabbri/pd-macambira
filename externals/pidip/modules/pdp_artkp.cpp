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

#include "ARToolKitPlus/TrackerMultiMarkerImpl.h"

#include "pdp.h"

class PdLogger : public ARToolKitPlus::Logger
{
    void artLog(const char* msg)
    {
        post("pdp_artkp : %s", msg);
    }
};

typedef struct pdp_artkp
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet0;
    t_outlet *x_outlet1;
    t_outlet *x_outlet2;
    int x_packet0;
    int x_packet1;
    int x_dropped;
    int x_queue_id;

    int x_width;
    int x_height;
    int x_size;

    // ARToolKitPlus data
    ARToolKitPlus::TrackerMultiMarker *x_tracker; /// the ARToolKitPlus marker tracker
    PdLogger *x_logger; // tracker logger
    int x_nummarkers;   // number of detected markers
    t_atom x_mdata[3];   // marker data ( list )

} t_pdp_artkp;

static void pdp_artkp_init(t_pdp_artkp *x)
{
   if( (x->x_width>1024) || (x->x_height>1024) )
   {
     post( "pdp_artkp : warning : detection might not work well with resolution > 1024");
   } 

   x->x_tracker = new ARToolKitPlus::TrackerMultiMarkerImpl<6,6,6, 1, 16>(x->x_width, x->x_height);

   if( !x->x_tracker )
   {
     post( "pdp_artkp : FATAL... could not allocate tracker");
     exit(-1);
   }

   x->x_logger = new PdLogger;
   if( !x->x_logger )
   {
     post( "pdp_artkp : FATAL... could not allocate logger");
     exit(-1);
   }
   else
   {
     x->x_tracker->setLogger(x->x_logger);
   }

   // setting pixel format
   x->x_tracker->setPixelFormat(ARToolKitPlus::PIXEL_FORMAT_RGB);

   if(!x->x_tracker->init("/usr/share/ARToolKitPlus/camera.dat", "/usr/share/ARToolKitPlus/markerboard_480-499.cfg", 1.0f, 1000.0f))
   {
        post( "pdp_artkp : FATAL... ARToolKitPlus init() failed.");
        post( "pdp_artkp : did you install artkp data files in /usr/share/ARToolKitPlus ?");
        exit(-1);
   } 

   // the marker in the BCH test image has a thiner border...
   x->x_tracker->setBorderWidth(0.125f);

   // activate automatic thresholding
   x->x_tracker->activateAutoThreshold(true);

   // let's use lookup-table undistortion for high-speed
   // note: LUT only works with images up to 1024x1024
   x->x_tracker->setUndistortionMode(ARToolKitPlus::UNDIST_LUT);

   // RPP is more robust than ARToolKit's standard pose estimator
   x->x_tracker->setPoseEstimator(ARToolKitPlus::POSE_ESTIMATOR_RPP);

   // switch to simple ID based markers
   // use the tool in tools/IdPatGen to generate markers
   x->x_tracker->setMarkerMode(ARToolKitPlus::MARKER_ID_SIMPLE);

}

static void pdp_artkp_process_rgb(t_pdp_artkp *x)
{
  t_pdp         *header = pdp_packet_header(x->x_packet0);
  unsigned char *data   = (unsigned char *)pdp_packet_data(x->x_packet0);
  t_pdp         *newheader = pdp_packet_header(x->x_packet1);
  unsigned char *newdata = (unsigned char *)pdp_packet_data(x->x_packet1); 
  int im;
  int numDetected;

    if ((x->x_width != (t_int)header->info.image.width) || 
        (x->x_height != (t_int)header->info.image.height)) 
    {

      post("pdp_artkp :: resizing plugins");
  
      x->x_width = header->info.image.width;
      x->x_height = header->info.image.height;
      x->x_size = x->x_width*x->x_height;

      pdp_artkp_init(x);
    }
    
    newheader->info.image.encoding = header->info.image.encoding;
    newheader->info.image.width = x->x_width;
    newheader->info.image.height = x->x_height;

    numDetected = x->x_tracker->calc((unsigned char*)data);
    if ( numDetected != x->x_nummarkers )
    {
       x->x_nummarkers = numDetected;
       outlet_float( x->x_outlet2, x->x_nummarkers );
    }

    for (im=0; im<numDetected; im++) 
    {
       ARToolKitPlus::ARMarkerInfo marker = x->x_tracker->getDetectedMarker(im);

       SETFLOAT(&x->x_mdata[0], marker.id);
       SETFLOAT(&x->x_mdata[1], marker.pos[0]);
       SETFLOAT(&x->x_mdata[2], marker.pos[1]);
       outlet_list( x->x_outlet1, 0, 3, x->x_mdata );

    }

    memcpy( newdata, data, x->x_size*3 );

    return;
}


static void pdp_artkp_sendpacket(t_pdp_artkp *x)
{
    /* release the packet */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = -1;

    /* unregister and propagate if valid dest packet */
    pdp_packet_pass_if_valid(x->x_outlet0, &x->x_packet1);
}

static void pdp_artkp_process(t_pdp_artkp *x)
{
  int encoding;
  t_pdp *header = 0;

  /* check if image data packets are compatible */
  if ( (header = pdp_packet_header(x->x_packet0))
  && (PDP_BITMAP == header->type)){
    
  /* pdp_artkp_process inputs and write into active inlet */
  switch(pdp_packet_header(x->x_packet0)->info.image.encoding){

   case PDP_BITMAP_RGB:
            x->x_packet1 = pdp_packet_clone_rw(x->x_packet0);
            pdp_queue_add(x, (void*)pdp_artkp_process_rgb, (void*)pdp_artkp_sendpacket, &x->x_queue_id);
      break;

   default:
      /* don't know the type, so dont pdp_artkp_process */
      break;
      
   }
  }

}

static void pdp_artkp_input_0(t_pdp_artkp *x, t_symbol *s, t_floatarg f)
{
    /* if this is a register_ro message or register_rw message, register with packet factory */

    if (s == gensym("register_rw")) 
       x->x_dropped = pdp_packet_convert_ro_or_drop(&x->x_packet0, (int)f, pdp_gensym((char*)"bitmap/rgb/*") );

    if ((s == gensym("process")) && (-1 != x->x_packet0) && (!x->x_dropped))
    {
        /* add the process method and callback to the process queue */
        pdp_artkp_process(x);
    }
}

static void pdp_artkp_free(t_pdp_artkp *x)
{
  int i;

    pdp_queue_finish(x->x_queue_id);
    pdp_packet_mark_unused(x->x_packet0);
    
}

t_class *pdp_artkp_class;

void *pdp_artkp_new(t_floatarg f)
{
  int i;

    t_pdp_artkp *x = (t_pdp_artkp *)pd_new(pdp_artkp_class);

    x->x_outlet0 = outlet_new(&x->x_obj, &s_anything); 
    x->x_outlet1 = outlet_new(&x->x_obj, &s_anything); 
    x->x_outlet2 = outlet_new(&x->x_obj, &s_anything); 

    x->x_packet0 = -1;
    x->x_packet1 = -1;
    x->x_queue_id = -1;

    x->x_width  = 320;
    x->x_height = 240;
    x->x_size   = x->x_width * x->x_height;

    // init the ARToolKitPlus tracker
    pdp_artkp_init(x);
    x->x_nummarkers = 0;
    
    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_artkp_setup(void)
{

    pdp_artkp_class = class_new(gensym("pdp_artkp"), (t_newmethod)pdp_artkp_new,
      (t_method)pdp_artkp_free, sizeof(t_pdp_artkp), 0, A_DEFFLOAT, A_NULL);

    class_addmethod(pdp_artkp_class, (t_method)pdp_artkp_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
