/*
 *   Pure Data Packet system module. - x window glue code (fairly tied to pd and pdp)
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


// this code is fairly tied to pd and pdp. serves mainly as reusable glue code
// for pdp_xv, pdp_glx, pdp_3d_windowcontext, ...


#include "pdp.h"
#include "pdp_xwindow.h"

#define D if(0)


static void pdp_xwindow_overrideredirect(t_pdp_xwindow *xwin, int b)
{
    XSetWindowAttributes new_attr;
    new_attr.override_redirect = b ? True : False;
    XChangeWindowAttributes(xwin->dpy, xwin->win, CWOverrideRedirect, &new_attr);
    //XFlush(xwin->dpy);

}


void pdp_xwindow_moveresize(t_pdp_xwindow *xwin, int xoffset, int yoffset, int width, int height)
{

    D post("_pdp_xwindow_moveresize");
    if ((width > 0) && (height > 0)){
	xwin->winwidth = width;
	xwin->winheight = height;
	xwin->winxoffset = xoffset;
	xwin->winyoffset = yoffset;

	if (xwin->initialized){
	    XMoveResizeWindow(xwin->dpy, xwin->win, xoffset, yoffset, width,  height);
	    XFlush(xwin->dpy);
	}
    }
}


void pdp_xwindow_fullscreen(t_pdp_xwindow *xwin)
{
    XWindowAttributes rootwin_attr;

    D post("pdp_xwindow_fullscreen");

    /* hmm.. fullscreen and xlib the big puzzle..
       if it looks like a hack it is a hack. */

    if (xwin->initialized){

        XGetWindowAttributes(xwin->dpy, RootWindow(xwin->dpy, xwin->screen), &rootwin_attr );

	//pdp_xwindow_overrideredirect(xwin, 0);
	pdp_xwindow_moveresize(xwin, 0, 0, rootwin_attr.width,  rootwin_attr.height);
	//pdp_xwindow_overrideredirect(xwin, 1);
	//XRaiseWindow(xwin->dpy, xwin->win);
	//pdp_xwindow_moveresize(xwin, 0, 0, rootwin_attr.width,   rootwin_attr.height);
	//pdp_xwindow_overrideredirect(xwin, 0);

 


    }
}


void pdp_xwindow_tile(t_pdp_xwindow *xwin, int x_tiles, int y_tiles, int i, int j)
{
    XWindowAttributes rootwin_attr;
    XSetWindowAttributes new_attr;

    D post("pdp_xwindow_fullscreen");

    if (xwin->initialized){
	int tile_w;
	int tile_h;
        XGetWindowAttributes(xwin->dpy, RootWindow(xwin->dpy, xwin->screen), &rootwin_attr );

	tile_w = rootwin_attr.width / x_tiles;
	tile_h = rootwin_attr.height / y_tiles;

	xwin->winwidth = (x_tiles-1) ? rootwin_attr.width - (x_tiles-1)*tile_w : tile_w;
	xwin->winheight = (y_tiles-1) ? rootwin_attr.height - (y_tiles-1)*tile_h : tile_h;
	xwin->winxoffset = i * tile_w;
	xwin->winyoffset = j * tile_h;

        //new_attr.override_redirect = True;
        //XChangeWindowAttributes(xwin->dpy, xwin->win, CWOverrideRedirect, &new_attr );
	XMoveResizeWindow(xwin->dpy, xwin->win, xwin->winxoffset, xwin->winyoffset, xwin->winwidth,  xwin->winheight);

    }
}

/* resize window */
void pdp_xwindow_resize(t_pdp_xwindow *xwin, int width, int height)
{
    D post("pdp_xwindow_resize");
    if ((width > 0) && (height > 0)){
	xwin->winwidth = width;
	xwin->winheight = height;
	if (xwin->initialized){
	    XResizeWindow(xwin->dpy, xwin->win,  width,  height);
	    XFlush(xwin->dpy);
	}
    }
    //_pdp_xwindow_moveresize(xwin, xwin->winxoffset, xwin->winyoffset, width, height);
}

/* move window */
void pdp_xwindow_move(t_pdp_xwindow *xwin, int xoffset, int yoffset)
{
    D post("pdp_xwindow_move");
    pdp_xwindow_moveresize(xwin, xoffset, yoffset, xwin->winwidth, xwin->winheight);
}

/* send events to a pd outlet (don't call this outside the pd thread) */
void pdp_xwindow_send_events(t_pdp_xwindow *xwin, t_outlet *outlet)
{

    unsigned int i;
    XEvent e;
    XConfigureEvent *ce = (XConfigureEvent *)&e;
    XButtonEvent *be = (XButtonEvent *)&e;
    XMotionEvent *me = (XMotionEvent *)&e;
    t_symbol *s;
    t_atom atom[2];
    char nextdrag[]="drag0";
    char but='0';
    float inv_x = 1.0f / (float)(xwin->winwidth);
    float inv_y = 1.0f / (float)(xwin->winheight);
    int nbEvents =  XEventsQueued(xwin->dpy, QueuedAlready);
    


#if 0
    while (nbEvents && XPending(xwin->dpy)){
	XNextEvent(xwin->dpy, &e);
	nbEvents--;
        if (e.xany.window != xwin->win) {
            XPutBackEvent(xwin->dpy, &e);
            continue;
        }
#else

    while (XPending(xwin->dpy)){
	XNextEvent(xwin->dpy, &e);
        //if (e.xany.window != xwin->win) continue;
#endif

	switch(e.type){
	case ConfigureNotify:
	    xwin->winwidth = ce->width;
	    xwin->winheight = ce->height;
	    inv_x = 1.0f / (float)(xwin->winwidth);
	    inv_y = 1.0f / (float)(xwin->winheight);
	    break;
	    
	case ClientMessage:
	    if ((Atom)e.xclient.data.l[0] == xwin->WM_DELETE_WINDOW) {
		//post("pdp_xv: button disabled, please send a \"close\" message to close the window");
		//pdp_xv_kaboom(x);
		//destroy = 1;
	    }
	    break;

	case ButtonPress:
	    //post("pdp_xv: press %f %f", inv_x * (float)be->x, inv_y * (float)be->y);
	    SETFLOAT(atom+0,inv_x * (float)be->x);
	    SETFLOAT(atom+1,inv_y * (float)be->y);
	    outlet_anything(outlet, gensym("press"), 2, atom);
	    switch(be->button){
	    case Button1: outlet_anything(outlet, gensym("press1"), 2, atom); but='1'; break;
	    case Button2: outlet_anything(outlet, gensym("press2"), 2, atom); but='2'; break;
	    case Button3: outlet_anything(outlet, gensym("press3"), 2, atom); but='3'; break;
	    case Button4: outlet_anything(outlet, gensym("press4"), 2, atom); but='4'; break;
	    case Button5: outlet_anything(outlet, gensym("press5"), 2, atom); but='5'; break;
	    default: break;
	    }
	    nextdrag[4]=but;
	    xwin->dragbutton = gensym(nextdrag);
	    break;

	case ButtonRelease:
	    //post("pdp_xv: release %f %f", inv_x * (float)be->x, inv_y * (float)be->y);
	    SETFLOAT(atom+0,inv_x * (float)be->x);
	    SETFLOAT(atom+1,inv_y * (float)be->y);
	    outlet_anything(outlet, gensym("release"), 2, atom);
	    switch(be->button){
	    case Button1: outlet_anything(outlet, gensym("release1"), 2, atom); break;
	    case Button2: outlet_anything(outlet, gensym("release2"), 2, atom); break;
	    case Button3: outlet_anything(outlet, gensym("release3"), 2, atom); break;
	    case Button4: outlet_anything(outlet, gensym("release4"), 2, atom); break;
	    case Button5: outlet_anything(outlet, gensym("release5"), 2, atom); break;
	    default: break;
	    }

	    break;
	case MotionNotify:
	    //post("pdp_xv: drag %f %f", inv_x * (float)be->x, inv_y * (float)be->y);
	    SETFLOAT(atom+0,inv_x * (float)be->x);
	    SETFLOAT(atom+1,inv_y * (float)be->y);
	    outlet_anything(outlet, gensym("drag"), 2, atom);
	    outlet_anything(outlet, xwin->dragbutton, 2, atom);
	    break;

	default:
	    //post("pdp_xv: unknown event");
	    break;
	}
	
    }

}



void pdp_xwindow_cursor(t_pdp_xwindow *xwin, t_floatarg f)
{
    if (!xwin->initialized) return;

    if (f == 0) {
        static char data[] = {0};

        Cursor cursor;
        Pixmap blank;
        XColor dummy;

        blank = XCreateBitmapFromData(xwin->dpy, xwin->win, data, 1, 1);
        cursor = XCreatePixmapCursor(xwin->dpy, blank, blank, &dummy,
                                     &dummy, 0, 0);
        XFreePixmap(xwin->dpy, blank);
        XDefineCursor(xwin->dpy, xwin->win,cursor);
    }
    else
        XUndefineCursor(xwin->dpy, xwin->win);

    xwin->cursor = f;
}

/* create xwindow */
int pdp_xwindow_create_on_display(t_pdp_xwindow *xwin, Display *dpy)
{
    XEvent e;
    unsigned int i;

    /* check if already opened */
    if(  xwin->initialized ){
	post("pdp_xwindow_create_on_display: window already created");
	goto exit;
    }
    
    xwin->dpy = dpy;

    /* create a window */
    xwin->screen = DefaultScreen(xwin->dpy);


    xwin->win = XCreateSimpleWindow(
	xwin->dpy, 
	RootWindow(xwin->dpy, xwin->screen), xwin->winxoffset, xwin->winyoffset, xwin->winwidth, xwin->winheight, 0, 
	BlackPixel(xwin->dpy, xwin->screen),
	BlackPixel(xwin->dpy, xwin->screen));


    /* enable handling of close window event */
    xwin->WM_DELETE_WINDOW = XInternAtom(xwin->dpy, "WM_DELETE_WINDOW", True);
    (void)XSetWMProtocols(xwin->dpy, xwin->win, &xwin->WM_DELETE_WINDOW, 1);

    if(!(xwin->win)){
	/* clean up mess */
	post("pdp_xwindow_create_on_display: could not create window. closing.\n");
	//XCloseDisplay(xwin->dpy); NOT OWNER
	xwin->dpy = 0;
	xwin->initialized = false;
	goto exit;
    }

    /* select input events */
    XSelectInput(xwin->dpy, xwin->win, StructureNotifyMask  | ButtonPressMask | ButtonReleaseMask | MotionNotify | ButtonMotionMask);
    //XSelectInput(xwin->dpy, xwin->win, StructureNotifyMask);


    /* set window title */
    XStoreName(xwin->dpy, xwin->win, "pdp");


    /* map */
    XMapWindow(xwin->dpy, xwin->win);

    /* create graphics context */
    xwin->gc = XCreateGC(xwin->dpy, xwin->win, 0, 0);

    /* catch mapnotify */
    for(;;){
	XNextEvent(xwin->dpy, &e);
	if (e.type == MapNotify) break;
    }


    /* we're done initializing */
    xwin->initialized = true;

    /* disable/enable cursor */
    pdp_xwindow_cursor(xwin, xwin->cursor);

 exit:
    return xwin->initialized;

}

void pdp_xwindow_init(t_pdp_xwindow *xwin)
{
    xwin->dpy = 0;
    xwin->screen = -1;

    xwin->winwidth = 320;
    xwin->winheight = 240;
    xwin->winxoffset = 0;
    xwin->winyoffset = 0;

    xwin->initialized = 0;

    xwin->cursor = 0;
    xwin->dragbutton = gensym("drag1");

}
    

void pdp_xwindow_close(t_pdp_xwindow *xwin)
{
    
    XEvent e;

    if (xwin->initialized){
	XFreeGC(xwin->dpy, xwin->gc);
	XDestroyWindow(xwin->dpy, xwin->win);
	while(XPending(xwin->dpy)) XNextEvent(xwin->dpy, &e);
	xwin->dpy = 0;
	xwin->initialized = false;
    }

}

void pdp_xwindow_free(t_pdp_xwindow *x)
{
    // close win
    pdp_xwindow_close(x);

    // no more dynamic data to free
}



// some more x specific stuff: XVideo

static void pdp_xvideo_create_xvimage(t_pdp_xvideo *xvid, int width, int height)
{
    int i;
    long size;
    
    //post("pdp_xvideo_create_xvimage");

    xvid->width = width;
    xvid->height = height;
    size = (xvid->width * xvid->height + (((xvid->width>>1)*(xvid->height>>1))<<1));
    //post("create xvimage %d %d", xvid->width, xvid->height);
    xvid->data = (unsigned char *)pdp_alloc(size);
    for (i=0; i<size; i++) xvid->data[i] = i;
    xvid->xvi = XvCreateImage(xvid->dpy, xvid->xv_port, xvid->xv_format, (char *)xvid->data, xvid->width, xvid->height);
    xvid->last_encoding = -1;
    if ((!xvid->xvi) || (!xvid->data)) post ("ERROR CREATING XVIMAGE");
    //post("created xvimag data:%x xvi:%x",xvid->data,xvid->xvi);

}

static void pdp_xvideo_destroy_xvimage(t_pdp_xvideo *xvid)
{
    if(xvid->data) pdp_dealloc(xvid->data);
    if (xvid->xvi) XFree(xvid->xvi);
    xvid->xvi = 0;
    xvid->data = 0;
}

void pdp_xvideo_display_packet(t_pdp_xvideo *xvid, t_pdp_xwindow *xwin, int packet)
{
    t_pdp *header = pdp_packet_header(packet);
    void *data = pdp_packet_data(packet);
    t_bitmap * bm = pdp_packet_bitmap_info(packet);
    unsigned int width, height, encoding, size, nbpixels;

    /* some checks: only display when initialized and when pacet is bitmap YV12 */
    if (!xvid->initialized) return;
    if (!header) return;
    if (!bm) return;

    width = bm->width;
    height = bm->height;
    encoding = bm->encoding;
    size = (width * height + (((width>>1)*(height>>1))<<1));
    nbpixels = width * height;

    if (PDP_BITMAP != header->type) return;
    if (PDP_BITMAP_YV12 != encoding) return;

    /* check if xvimage needs to be recreated */
    if ((width != xvid->width) || (height != xvid->height)){
	//post("pdp_xv: replace image");
	pdp_xvideo_destroy_xvimage(xvid);
	pdp_xvideo_create_xvimage(xvid, width, height);
    }

    /* copy the data to the XvImage buffer */
    memcpy(xvid->data, data, size);

    /* display */
    XvPutImage(xvid->dpy,xvid->xv_port, xwin->win,xwin->gc,xvid->xvi, 
               0,0,xvid->width,xvid->height, 0,0,xwin->winwidth,xwin->winheight);
    XFlush(xvid->dpy);


   
}



void pdp_xvideo_close(t_pdp_xvideo* xvid)
{
    if (xvid->initialized){
	if (xvid->xvi) pdp_xvideo_destroy_xvimage(xvid);
	XvUngrabPort(xvid->dpy, xvid->xv_port, CurrentTime);
	xvid->xv_port = 0;
	xvid->dpy = 0;
	xvid->screen = -1;
	xvid->last_encoding = -1;
	xvid->initialized = false;
    }
}

void pdp_xvideo_free(t_pdp_xvideo* xvid)
{
    // close xvideo port (and delete XvImage)
    pdp_xvideo_close(xvid);

    // no more dynamic data to free

}

void pdp_xvideo_init(t_pdp_xvideo *xvid)
{

    xvid->dpy = 0;
    xvid->screen = -1;

    xvid->xv_format = FOURCC_YV12;
    xvid->xv_port      = 0;

    xvid->width = 320;
    xvid->height = 240;

    xvid->data = 0;
    xvid->xvi = 0;

    xvid->initialized = 0;
    xvid->last_encoding = -1;

}

int pdp_xvideo_open_on_display(t_pdp_xvideo *xvid, Display *dpy)
{
    unsigned int ver, rel, req, ev, err, i, j;
    unsigned int adaptors;
    int formats;
    XvAdaptorInfo        *ai;

    if (xvid->initialized) return 1;
    if (!dpy) return 0;
    xvid->dpy = dpy;
    
    if (Success != XvQueryExtension(xvid->dpy,&ver,&rel,&req,&ev,&err))	return 0;

    /* find + lock port */
    if (Success != XvQueryAdaptors(xvid->dpy,DefaultRootWindow(xvid->dpy),&adaptors,&ai))
	return 0;
    for (i = 0; i < adaptors; i++) {
	if ((ai[i].type & XvInputMask) && (ai[i].type & XvImageMask)) {
	    for (j=0; j < ai[i].num_ports; j++){
		if (Success != XvGrabPort(xvid->dpy,ai[i].base_id+j,CurrentTime)) {
		    //fprintf(stderr,"INFO: Xvideo port %ld on adapter %d: is busy, skipping\n",ai[i].base_id+j, i);
		}
		else {
		    xvid->xv_port = ai[i].base_id + j;
		    goto breakout;
		}
	    }
	}
    }


 breakout:

    XFree(ai);
    if (0 == xvid->xv_port) return 0;
    post("pdp_xvideo: grabbed port %d on adaptor %d", xvid->xv_port, i);
    xvid->initialized = 1;
    pdp_xvideo_create_xvimage(xvid, xvid->width, xvid->height);
    return 1;
}


