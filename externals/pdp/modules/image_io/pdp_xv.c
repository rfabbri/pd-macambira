/*
 *   Pure Data Packet module. Xvideo image packet output
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

// pdp stuff
#include "pdp.h"
#include "pdp_base.h"

// some x window glue code
#include "pdp_xwindow.h"


#define PDP_XV_AUTOCREATE_RETRY 10


typedef struct pdp_xv_struct
{
    t_object x_obj;

    t_pdp_xwindow x_xwin;
    t_pdp_xvideo x_xvid;

    t_outlet *x_outlet;

    int x_packet0;
    int x_queue_id;
    t_symbol *x_display;

    Display *x_dpy;

    int  x_initialized;
    int  x_autocreate;


} t_pdp_xv;


static void pdp_xv_cursor(t_pdp_xv *x, t_floatarg f)
{
    pdp_xwindow_cursor(&x->x_xwin, f);
}


static void pdp_xv_destroy(t_pdp_xv* x)
{
    t_pdp_procqueue *q = pdp_queue_get_queue();
    if (x->x_initialized){
	
	pdp_procqueue_finish(q, x->x_queue_id);   // wait for thread to finish
	x->x_queue_id = -1;
	pdp_xvideo_close(&x->x_xvid);      // close xvideo connection
	pdp_xwindow_close(&x->x_xwin);     // close the window
	XCloseDisplay(x->x_dpy);           // close the display connection
	x->x_dpy = 0;
	pdp_packet_mark_unused(x->x_packet0);
	x->x_packet0 = -1;
	x->x_initialized = 0;

    }
}



static void pdp_xv_create(t_pdp_xv* x)
{
    int i;
    if(x->x_initialized) return;

    /* manually open a display */
    if (NULL == (x->x_dpy = XOpenDisplay(x->x_display->s_name))){
	post("pdp_xv: cant open display %s\n",x->x_display->s_name);
	x->x_initialized = false;
	return;
    }

    /* open an xv port on the display */
    if (!(x->x_initialized = pdp_xvideo_open_on_display(&x->x_xvid, x->x_dpy))) goto exit_close_dpy;

    /* create a window on the display */
    if (!(x->x_initialized = pdp_xwindow_create_on_display(&x->x_xwin, x->x_dpy))) goto exit_close_win;

    /* done */
    return;

    /* cleanup exits */
 exit_close_win:
    pdp_xwindow_close(&x->x_xwin); // cose window
 exit_close_dpy:
    XCloseDisplay(x->x_dpy);       // close display
    x->x_dpy = 0;
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

static void pdp_xv_bang_thread(t_pdp_xv *x)
{
    pdp_xvideo_display_packet(&x->x_xvid, &x->x_xwin, x->x_packet0);
}


static void pdp_xv_bang_callback(t_pdp_xv *x)
{
    /* receive events & output them */
    pdp_xwindow_send_events(&x->x_xwin, x->x_outlet);


    /* release the packet if there is one */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = -1;

}

static void pdp_xv_bang(t_pdp_xv *x)
{
    t_pdp_procqueue *q = pdp_queue_get_queue();

    /* check if window is initialized */
    if (!(x->x_initialized)){
        if (!pdp_xv_try_autocreate(x)) return;
    }

    /* check if we can proceed */
    if (-1 != x->x_queue_id) return;
    if (-1 == x->x_packet0) return;

    /* if previous queued method returned
       schedule a new one, else ignore */
    if (-1 == x->x_queue_id) {
	pdp_procqueue_add(q, x, pdp_xv_bang_thread, pdp_xv_bang_callback, &x->x_queue_id);
    }
    
}

static void pdp_xv_input_0(t_pdp_xv *x, t_symbol *s, t_floatarg f)
{

    if (s == gensym("register_ro")) pdp_packet_convert_ro_or_drop(&x->x_packet0, (int)f, pdp_gensym("bitmap/yv12/*"));
    if (s == gensym("process")) pdp_xv_bang(x);

}



static void pdp_xv_autocreate(t_pdp_xv *x, t_floatarg f)
{
  if (f != 0.0f) x->x_autocreate = PDP_XV_AUTOCREATE_RETRY;
  else x->x_autocreate = 0;
}

static void pdp_xv_display(t_pdp_xv *x, t_symbol *s)
{
    t_pdp_procqueue *q = pdp_queue_get_queue();
    pdp_procqueue_finish(q, x->x_queue_id);
    x->x_queue_id = -1;
    x->x_display = s;
    if (x->x_initialized){
	pdp_xv_destroy(x);
	pdp_xv_create(x);
    }
}

static void pdp_xv_fullscreen(t_pdp_xv *x)
{
    pdp_xwindow_fullscreen(&x->x_xwin);
}

static void pdp_xv_resize(t_pdp_xv* x, t_floatarg width, t_floatarg height)
{
    pdp_xwindow_resize(&x->x_xwin, width, height);
}

static void pdp_xv_move(t_pdp_xv* x, t_floatarg width, t_floatarg height)
{
    pdp_xwindow_move(&x->x_xwin, width, height);
}

static void pdp_xv_moveresize(t_pdp_xv* x, t_floatarg xoff, t_floatarg yoff, t_floatarg width, t_floatarg height)
{
    pdp_xwindow_moveresize(&x->x_xwin, xoff, yoff, width, height);
}

static void pdp_xv_tile(t_pdp_xv* x, t_floatarg xtiles, t_floatarg ytiles, t_floatarg i, t_floatarg j)
{
    pdp_xwindow_tile(&x->x_xwin, xtiles, ytiles, i, j);
}

static void pdp_xv_vga(t_pdp_xv *x)
{
    pdp_xv_resize(x, 640, 480);
}

static void pdp_xv_free(t_pdp_xv *x)
{
    t_pdp_procqueue *q = pdp_queue_get_queue();
    pdp_procqueue_finish(q, x->x_queue_id);
    pdp_xv_destroy(x);
    pdp_xvideo_free(&x->x_xvid);
    pdp_xwindow_free(&x->x_xwin);
}

t_class *pdp_xv_class;



void *pdp_xv_new(void)
{
    t_pdp_xv *x = (t_pdp_xv *)pd_new(pdp_xv_class);
    x->x_outlet = outlet_new(&x->x_obj, &s_anything);

    pdp_xwindow_init(&x->x_xwin);
    pdp_xvideo_init(&x->x_xvid);

    x->x_packet0 = -1;
    x->x_queue_id = -1;
    x->x_display = gensym(":0");
    x->x_dpy = 0;
    pdp_xv_autocreate(x,1);

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
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_create, gensym("open"), A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_create, gensym("create"), A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_autocreate, gensym("autocreate"), A_FLOAT, A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_destroy, gensym("destroy"), A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_destroy, gensym("close"), A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_resize, gensym("dim"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_move, gensym("move"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_move, gensym("pos"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_resize, gensym("size"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_display, gensym("display"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_cursor, gensym("cursor"), A_FLOAT, A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_fullscreen, gensym("fullscreen"), A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_moveresize, gensym("posdim"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_tile, gensym("tile"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);

    /* some shortcuts for the lazy */
    class_addmethod(pdp_xv_class, (t_method)pdp_xv_vga, gensym("vga"), A_NULL);

}

#ifdef __cplusplus
}
#endif


