
/*
 *   Pure Data Packet header file: xwindow glue code
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


// x stuff
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#include "pdp.h"

// image formats for communication with the X Server
#define FOURCC_YV12 0x32315659  /* YV12   YUV420P */
#define FOURCC_YUV2 0x32595559  /* YUV2   YUV422 */
#define FOURCC_I420 0x30323449  /* I420   Intel Indeo 4 */

/* TODO: finish this */

/* remarks:
   this class does not own the display connection */




/* x window class */
typedef struct _pdp_xwindow
{
    Display *dpy;
    int screen;
    Window win;
    GC gc;
    Atom WM_DELETE_WINDOW;


    int winwidth;
    int winheight;
    int winxoffset;
    int winyoffset;

    int  initialized;
    int  autocreate;
    t_symbol *dragbutton;
    
    float cursor;

} t_pdp_xwindow;

/* cons */
void pdp_xwindow_init(t_pdp_xwindow *b);

/* des */    
void pdp_xwindow_free(t_pdp_xwindow *b);

/* fullscreen message */
void pdp_xwindow_fullscreen(t_pdp_xwindow *xwin);

/* resize window */
void pdp_xwindow_resize(t_pdp_xwindow *b, int width, int height);

/* resize window */
void pdp_xwindow_moveresize(t_pdp_xwindow *b, int xoffset, int yoffset, int width, int height);

/* fill a tile of the screen */
void pdp_xwindow_tile(t_pdp_xwindow *xwin, int x_tiles, int y_tiles, int i, int j);

/* move window */
void pdp_xwindow_move(t_pdp_xwindow *xwin, int xoffset, int yoffset);

/* receive events */
void pdp_xwindow_send_events(t_pdp_xwindow *b, t_outlet *outlet);

/* enable/disable cursor */
void pdp_xwindow_cursor(t_pdp_xwindow *b, t_floatarg f);

/* create xwindow. return code != NULL on succes */
int pdp_xwindow_create_on_display(t_pdp_xwindow *b, Display *dpy);

/* close window */    
void pdp_xwindow_close(t_pdp_xwindow *b);



/* xvideo class */
typedef struct _pdp_xvideo
{

    Display *dpy;
    int screen;
    Window win;

    int xv_format;
    int xv_port;

    XvImage *xvi;
    unsigned char *data;
    unsigned int width;
    unsigned int height;
    int last_encoding;

    int  initialized;

} t_pdp_xvideo;


/* cons */
void pdp_xvideo_init(t_pdp_xvideo *x);

/* des */
void pdp_xvideo_free(t_pdp_xvideo* x);


/* open an xv port (and create XvImage) */
int pdp_xvideo_open_on_display(t_pdp_xvideo *x, Display *dpy);

/* close xv port (and delete XvImage */
void pdp_xvideo_close(t_pdp_xvideo* x);

/* display a packet */
void pdp_xvideo_display_packet(t_pdp_xvideo *x, t_pdp_xwindow *w, int packet);


#if 0
/* xwindow event handler */
typedef struct _pdp_xwin_handler
{
    t_pdp_xwindow *win;
    t_outlet *outlet;
}
#endif
