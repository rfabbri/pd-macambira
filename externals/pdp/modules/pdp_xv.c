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

/*

pdp xvideo output

*/


// x stuff
#include <X11/Xlib.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>

// image formats for communication with the X Server
#define FOURCC_YV12 0x32315659  /* YV12   YUV420P */
#define FOURCC_YUV2 0x32595559  /* YUV2   YUV422 */
#define FOURCC_I420 0x30323449  /* I420   Intel Indeo 4 */

// pdp stuff
#include "pdp.h"
#include "pdp_llconv.h"


/* initial image dimensions */
#define PDP_XV_W 320
#define PDP_XV_H 240

#define PDP_XV_AUTOCREATE_RETRY 10


typedef struct pdp_xv_struct
{
    t_object x_obj;
    t_float x_f;

    int x_packet0;
    int x_queue_id;
    t_symbol *x_display;

    Display *x_dpy;
    int x_screen;
    Window x_win;
    GC x_gc;

    int x_xv_format;
    int x_xv_port;

    int x_winwidth;
    int x_winheight;

    XvImage *x_xvi;
    unsigned char *x_data;
    unsigned int x_width;
    unsigned int x_height;
    int x_last_encoding;

    int  x_initialized;
    int  x_autocreate;

    float x_cursor;


} t_pdp_xv;


static int xv_init(t_pdp_xv *x)
{
    unsigned int ver, rel, req, ev, err, i, j;
    unsigned int adaptors;
    int formats;
    XvAdaptorInfo        *ai;
    
    if (Success != XvQueryExtension(x->x_dpy,&ver,&rel,&req,&ev,&err))	return 0;

    /* find + lock port */
    if (Success != XvQueryAdaptors(x->x_dpy,DefaultRootWindow(x->x_dpy),&adaptors,&ai))
	return 0;
    for (i = 0; i < adaptors; i++) {
	if ((ai[i].type & XvInputMask) && (ai[i].type & XvImageMask)) {
	    for (j=0; j < ai[i].num_ports; j++){
		if (Success != XvGrabPort(x->x_dpy,ai[i].base_id+j,CurrentTime)) {
		    //fprintf(stderr,"INFO: Xvideo port %ld on adapter %d: is busy, skipping\n",ai[i].base_id+j, i);
		}
		else {
		    x->x_xv_port = ai[i].base_id + j;
		    goto breakout;
		}
	    }
	}
    }


 breakout:

    XFree(ai);
    if (0 == x->x_xv_port) return 0;
    post("pdp_xv: grabbed port %d on adaptor %d", x->x_xv_port, i);
    return 1;
}


void pdp_xv_create_xvimage(t_pdp_xv *x)
{

    long size = (x->x_width * x->x_height + (((x->x_width>>1)*(x->x_height>>1))<<1));
    x->x_data = (unsigned char *)malloc(size);
    x->x_xvi = XvCreateImage(x->x_dpy, x->x_xv_port, x->x_xv_format, (char *)x->x_data, x->x_width, x->x_height);
    x->x_last_encoding = -1;
    if (!(x->x_xvi) || (!x->x_data)) post ("ERROR CREATING XVIMAGE");

}

void pdp_xv_destroy_xvimage(t_pdp_xv *x)
{
    if(x->x_data) free(x->x_data);
    if (x->x_xvi) XFree(x->x_xvi);
    x->x_xvi = 0;
    x->x_data = 0;
}


static void pdp_xv_cursor(t_pdp_xv *x, t_floatarg f)
{
    if (!x->x_initialized) return;

    if (f == 0) {
        static char data[1] = {0};

        Cursor cursor;
        Pixmap blank;
        XColor dummy;

        blank = XCreateBitmapFromData(x->x_dpy, x->x_win, data, 1, 1);
        cursor = XCreatePixmapCursor(x->x_dpy, blank, blank, &dummy,
                                     &dummy, 0, 0);
        XFreePixmap(x->x_dpy, blank);
        XDefineCursor(x->x_dpy, x->x_win,cursor);
    }
    else
        XUndefineCursor(x->x_dpy, x->x_win);

    x->x_cursor = f;
}


static void pdp_xv_destroy(t_pdp_xv* x)
{
    XEvent e;

    if (x->x_initialized){
	XFreeGC(x->x_dpy, x->x_gc);
	XDestroyWindow(x->x_dpy, x->x_win);
	while(XPending(x->x_dpy)) XNextEvent(x->x_dpy, &e);
	XvUngrabPort(x->x_dpy, x->x_xv_port, CurrentTime);
	pdp_xv_destroy_xvimage(x);
	XCloseDisplay(x->x_dpy);
	x->x_initialized = false;

    }

}

static void pdp_xv_resize(t_pdp_xv* x, t_floatarg width, t_floatarg height)
{
    if (x->x_initialized && (width>0) && (height>0)){
	XResizeWindow(x->x_dpy, x->x_win, (unsigned int)width, (unsigned int)height);
	XFlush(x->x_dpy);
    }
}

static void pdp_xv_create(t_pdp_xv* x)
{
    unsigned int *uintdata = (unsigned int *)(x->x_data);
    XEvent e;
    unsigned int i;

    if(  x->x_initialized ){
	//post("pdp_xv: window already created");
	return;
    }

    if (NULL == (x->x_dpy = XOpenDisplay(x->x_display->s_name))){
	post("pdp_xv: cant open display %s\n",x->x_display->s_name);
	x->x_initialized = false;
	return;
    }

    /* init xvideo */
    if (xv_init(x)){

	pdp_xv_create_xvimage(x);

    }

    else {
	/* clean up mess */
	post("pdp_xv: ERROR: no xv port available. closing display.");
	XCloseDisplay(x->x_dpy);
	x->x_initialized = false;
	return;
    }


    /* create a window */
    x->x_screen = DefaultScreen(x->x_dpy);


    x->x_win = XCreateSimpleWindow(
	x->x_dpy, 
	RootWindow(x->x_dpy, x->x_screen), 0, 0, x->x_winwidth, x->x_winheight, 0, 
	BlackPixel(x->x_dpy, x->x_screen),
	BlackPixel(x->x_dpy, x->x_screen));

    if(!(x->x_win)){
	/* clean up mess */
	post("pdp_xv: could not create window\n");
	post("pdp_xv: unlocking xv port");
	XvUngrabPort(x->x_dpy, x->x_xv_port, CurrentTime);
	post("pdp_xv: freeing xvimage");
	pdp_xv_destroy_xvimage(x);
	post("pdp_xv: closing display");
	XCloseDisplay(x->x_dpy);
	x->x_initialized = false;
	return;
    }

    XSelectInput(x->x_dpy, x->x_win, StructureNotifyMask);

    XMapWindow(x->x_dpy, x->x_win);

    x->x_gc = XCreateGC(x->x_dpy, x->x_win, 0, 0);

    for(;;){
	XNextEvent(x->x_dpy, &e);
	if (e.type == MapNotify) break;
    }


    x->x_initialized = true;
    pdp_xv_cursor(x, x->x_cursor);

}

void  pdp_xv_copy_xvimage(t_pdp_xv *x, t_image *image, short int* data)
{
    unsigned int width = image->width;
    unsigned int height = image->height;
    int encoding = image->encoding;
    unsigned int* uintdata;
    int i;


    /* 8bit y fulscale and 8bit u,v 2x2 subsampled */
    //static short int gain[4] = {0x0100, 0x0100, 0x0100, 0x0100}; 
    long size = (width * height + (((width>>1)*(height>>1))<<1));
    int nbpixels = width * height;

    /* check if xvimage needs to be recreated */
    if ((width != x->x_width) || (height != x->x_height)){
	//post("pdp_xv: replace image");
	x->x_width = width;
	x->x_height = height;
	pdp_xv_destroy_xvimage(x);
	pdp_xv_create_xvimage(x);
    }


    /* data holds a 16bit version of a the xvimage, so it needs to be converted */

    

    if (data) {
	/* convert 16bit -> 8bit */
	if(PDP_IMAGE_YV12 == encoding){
	    pdp_llconv(data,RIF_YVU__P411_S16, x->x_data, RIF_YVU__P411_U8, x->x_width, x->x_height);  
	    x->x_last_encoding = PDP_IMAGE_YV12;
	}
	if(PDP_IMAGE_GREY == encoding){
	    pdp_llconv(data,RIF_GREY______S16, x->x_data, RIF_GREY______U8, x->x_width, x->x_height);  
	    if (PDP_IMAGE_GREY != x->x_last_encoding) {
		post("pdp_xv: switching to greyscale");
		/* clear u&v planes if necessary */
		uintdata = (unsigned int *)&x->x_data[nbpixels];
		for(i=0; i < nbpixels>>3; i++) uintdata[i]=0x80808080;
	    }
	    x->x_last_encoding = PDP_IMAGE_GREY;
	    
	}
    }
    else bzero(x->x_data, size);


    
}

static int pdp_xv_try_autocreate(t_pdp_xv *x)
{

    if (x->x_autocreate){
	post("pdp_xv: autocreate window");
	pdp_xv_create(x);
	if (!(x->x_initialized)){
	    x->x_autocreate--;
	    if (!x->x_autocreate){
		post ("pdp_xv: autocreate failed %d times: disabled", PDP_XV_AUTOCREATE_RETRY);
		post ("pdp_xv: send [autocreate 1] message to re-enable");
		return 0;
	    }
	}
	else return 1;

    }
    return 0;
}

static void pdp_xv_bang(t_pdp_xv *x);

static void pdp_xv_process(t_pdp_xv *x)
{
    t_pdp *header = pdp_packet_header(x->x_packet0);
    void  *data   = pdp_packet_data  (x->x_packet0);


    if (-1 != x->x_queue_id) return;

    /* check if window is initialized */
    if (!(x->x_initialized)){
        if (!pdp_xv_try_autocreate(x)) return;
    }

    /* check data packet */
    if (!(header)) {
	post("pdp_xv: invalid packet header");
	return;
    }
    if (PDP_IMAGE != header->type) {
	post("pdp_xv: packet is not a PDP_IMAGE");
	return;
    }
    if ((PDP_IMAGE_YV12 != header->info.image.encoding)
	&& (PDP_IMAGE_GREY != header->info.image.encoding)) {
	post("pdp_xv: packet is not a PDP_IMAGE_YV12/GREY");
	return;
    }
    
    /* copy the packet to the xvimage */
    pdp_xv_copy_xvimage(x, &header->info.image, (short int *)data);


    /* display the new image */
    pdp_xv_bang(x);


}


static void pdp_xv_random(t_pdp_xv *x)
{
    unsigned int i;
    long *intdata = (long *)(x->x_data);
    for(i=0; i<x->x_width*x->x_height/4; i++) intdata[i]=random();
}

/* redisplays image */
static void pdp_xv_bang_thread(t_pdp_xv *x)
{

    XEvent e;
    unsigned int i;

    //while (XEventsQueued(x->x_dpy, QueuedAlready)){
    while (XPending(x->x_dpy)){
        //post("pdp_xv: waiting for event");
	XNextEvent(x->x_dpy, &e);
	//post("pdp_xv: XEvent %d", e.type);
	if(e.type == ConfigureNotify){
	    XConfigureEvent *ce = (XConfigureEvent *)&e;
	    x->x_winwidth = ce->width;
	    x->x_winheight = ce->height;

	}
        //post("pdp_xv: received event");
	
    }

    XvPutImage(x->x_dpy,x->x_xv_port,x->x_win,x->x_gc,x->x_xvi, 
	       0,0,x->x_width,x->x_height, 0,0,x->x_winwidth,x->x_winheight);
    XFlush(x->x_dpy);
}

static void pdp_xv_bang_callback(t_pdp_xv *x)
{
    /* release the packet if there is one */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = -1;}

static void pdp_xv_bang(t_pdp_xv *x)
{


    /* if previous queued method returned
       schedule a new one, else ignore */
    if (-1 == x->x_queue_id) {
	pdp_queue_add(x, pdp_xv_bang_thread, pdp_xv_bang_callback, &x->x_queue_id);
    }
    
}

static void pdp_xv_input_0(t_pdp_xv *x, t_symbol *s, t_floatarg f)
{

    if (s == gensym("register_ro")) pdp_packet_copy_ro_or_drop(&x->x_packet0, (int)f);
    if (s == gensym("process")) pdp_xv_process(x);

}


static void pdp_xv_autocreate(t_pdp_xv *x, t_floatarg f)
{
  if (f != 0.0f) x->x_autocreate = PDP_XV_AUTOCREATE_RETRY;
  else x->x_autocreate = 0;
}

static void pdp_xv_display(t_pdp_xv *x, t_symbol *s)
{
    pdp_queue_finish(x->x_queue_id);
    x->x_queue_id = -1;
    x->x_display = s;
    if (x->x_initialized){
	pdp_xv_destroy(x);
	pdp_xv_create(x);
    }
}



static void pdp_xv_free(t_pdp_xv *x)
{
    pdp_queue_finish(x->x_queue_id);

    pdp_xv_destroy(x);
    pdp_packet_mark_unused(x->x_packet0);
}

t_class *pdp_xv_class;



void *pdp_xv_new(void)
{
    t_pdp_xv *x = (t_pdp_xv *)pd_new(pdp_xv_class);


    x->x_packet0 = -1;
    x->x_queue_id = -1;
    x->x_display = gensym(":0");


    x->x_dpy = 0;
    x->x_screen = -1;

    x->x_xv_format = FOURCC_YV12;
    x->x_xv_port      = 0;

    x->x_winwidth = PDP_XV_W;
    x->x_winheight = PDP_XV_H;

    x->x_width = PDP_XV_W;
    x->x_height = PDP_XV_H;

    x->x_data = 0;
    x->x_xvi = 0;

    x->x_initialized = 0;
    pdp_xv_autocreate(x,1);
    x->x_last_encoding = -1;

    x->x_cursor = 0;

    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif


void pdp_xv_setup(void)
{


    pdp_xv_class = class_new(gensym("pdp_xv"), (t_newmethod)pdp_xv_new,
    	(t_method)pdp_xv_free, sizeof(t_pdp_xv), 0, A_NULL);


    class_addmethod(pdp_xv_class, (t_method)pdp_xv_bang, gensym("bang"), A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_random, gensym("random"), A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_create, gensym("create"), A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_autocreate, gensym("autocreate"), A_FLOAT, A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_destroy, gensym("destroy"), A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_destroy, gensym("close"), A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_resize, gensym("dim"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_resize, gensym("size"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_display, gensym("display"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_cursor, gensym("cursor"), A_FLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
