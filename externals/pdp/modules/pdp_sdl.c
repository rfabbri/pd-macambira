/*
 *   Pure Data Packet module.
 *   Copyright (c) by martin pi <pi@attacksyour.net>
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

pdp sdl output

DONE:

TODO:
 * close window (event)
 * fullscreen chose resolution
 * event handling in different object (and look at mplayer for that!)

*/



#include <stdio.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <SDL/SDL.h>

#include <quicktime/lqt.h>
#include <quicktime/colormodels.h>

#include "pdp.h"
#include "pdp_llconv.h"


/* initial image dimensions */
#define PDP_SDL_W 320
#define PDP_SDL_H 240

#define PDP_AUTOCREATE_RETRY	3


typedef struct pdp_sdl_struct {
    t_object x_obj;
    t_float x_f;

    int x_packet0;
    int x_queue_id;

	SDL_Surface *x_sdl_surface;
	SDL_Overlay *x_sdl_overlay;
	SDL_Rect x_sdl_rect;

    Uint32 x_sdl_format;

    int x_winwidth;
    int x_winheight;

    unsigned int x_width;
    unsigned int x_height;
    int x_last_encoding;

    int x_initialized;
    int x_backfromthread;
    int x_autocreate;
    

    int x_fullscreen;

} t_pdp_sdl;

static SDL_Surface *pdp_sdl_getSurface(char* title, int width, int height, int bits, int fullscreenflag) {
	Uint32 flags;
	int size,i;
	SDL_Surface *screen;

	/* Initialize SDL */
	if (!SDL_WasInit(SDL_INIT_VIDEO)) {
		if (SDL_Init (SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE)) {
			printf("SDL: Initializing of SDL failed: %s.\n", SDL_GetError());
			return (SDL_Surface *)-1;
		}
	}



/*	
gem :    SDL_OPENGL|SDL_DOUBLEBUF|SDL_HWSURFACE|SDL_ANYFORMAT|SDL_OPENGLBLIT; 
working: SDL_ANYFORMAT|SDL_RESIZABLE|SDL_RLEACCEL; 
*/

	flags = SDL_HWSURFACE|SDL_RESIZABLE|SDL_ASYNCBLIT|SDL_HWACCEL;
	if ( fullscreenflag>0 ) {
		flags |= SDL_FULLSCREEN|SDL_DOUBLEBUF;
	}

	/* Have a preference for 8-bit, but accept any depth */
	screen = SDL_SetVideoMode(width, height, bits, flags);
	if ( screen == NULL ) {
		fprintf(stderr, "Couldn't set video mode: %s\n",
                        SDL_GetError());
		return NULL;
	}

	SDL_WM_SetCaption (title, title);
  
	/* ignore events :: only keys and wm_quit */
	for ( i=SDL_NOEVENT; i<SDL_NUMEVENTS; ++i )
		if( !(i & (SDL_KEYDOWN|SDL_QUIT)) ) 
			SDL_EventState(i, SDL_IGNORE);

	SDL_ShowCursor(1);

	return screen;								// Success
}

static inline void pdp_sdl_getOverlay(t_pdp_sdl* x) {
	x->x_sdl_overlay = SDL_CreateYUVOverlay(x->x_width, x->x_height, x->x_sdl_format, x->x_sdl_surface);
}

static inline void pdp_sdl_freeOverlay(t_pdp_sdl* x) {
	SDL_FreeYUVOverlay(x->x_sdl_overlay);
}

static int pdp_sdl_drawImage(t_pdp_sdl* x, t_image *image, short int *pixels) {

	unsigned int width = image->width;
	unsigned int height = image->height;
	int encoding = image->encoding;
	unsigned int* uintdata;
	int i;


	/* 8bit y fulscale and 8bit u,v 2x2 subsampled */
	//static short int gain[4] = {0x0100, 0x0100, 0x0100, 0x0100}; 
	int nbpixels = width * height;
 	long size = (width * height + (((width>>1)*(height>>1))<<1));

	/* check if xvimage needs to be recreated */
	if ((width != x->x_width) || (height != x->x_height)){
		post("pdp_xv: replace image");
		x->x_width = width;
		x->x_height = height;
		pdp_sdl_freeOverlay(x);
		pdp_sdl_getOverlay(x);
	}

	SDL_LockYUVOverlay(x->x_sdl_overlay);
	if (pixels) {
        	pdp_llconv(pixels,RIF_YVU__P411_S16, (Uint8 *)(* x->x_sdl_overlay->pixels), RIF_YVU__P411_U8, x->x_width, x->x_height);  
	} else bzero((Uint8 *)(* x->x_sdl_overlay->pixels), size);
	SDL_UnlockYUVOverlay(x->x_sdl_overlay);
	
	return 1;
}

static void pdp_sdl_resize(t_pdp_sdl* x, t_floatarg width, t_floatarg height) {

    if (x->x_initialized && (!x->x_fullscreen) && (width>0) && (height>0)){
//	disabled for now
//	if media size is different to the one set, it will resize (yet never dither)
    }
}

static void pdp_sdl_fullscreen(t_pdp_sdl *x, t_floatarg f);

static int pdp_sdl_create(t_pdp_sdl *x) {
	if (x->x_initialized){
		return 0;
	}
	x->x_initialized = 0;
	
	x->x_sdl_surface = pdp_sdl_getSurface("pdp-sdl", x->x_winwidth, x->x_winheight, 16, x->x_fullscreen);
	if (x->x_sdl_surface != NULL) {
		pdp_sdl_getOverlay(x);
		if (x->x_sdl_overlay != NULL) {
			x->x_sdl_rect.x = 0;
			x->x_sdl_rect.y = 0;
			x->x_sdl_rect.w = x->x_width;
			x->x_sdl_rect.h = x->x_height;
			x->x_initialized = 1;
			post("created successfully");
		}
	}
	x->x_backfromthread = 1;
	
	return x->x_initialized;
}

static void pdp_sdl_destroy(t_pdp_sdl *x);

static void pdp_sdl_checkEvents(t_pdp_sdl *x) {
	Uint8 *keys;
	SDL_Event event;

	if (!SDL_PollEvent(&event)) return;

	switch( event.type ){
		case SDL_KEYDOWN:
		case SDL_KEYUP:
			keys = SDL_GetKeyState(NULL);

			if(keys[SDLK_UP]) { post("up"); }
			if(keys[SDLK_DOWN]) { post("down"); }

			if(keys[SDLK_ESCAPE]) pdp_sdl_fullscreen(x,0);
			break;
			
		case SDL_QUIT:
			pdp_sdl_destroy(x);
			break;
		default:
			break;
	}


}

static int pdp_sdl_try_autocreate(t_pdp_sdl *x)
{

    if (x->x_autocreate){
	post("pdp_sdl: autocreate window");
	pdp_sdl_create(x);
	if (!(x->x_initialized)){
	    x->x_autocreate--;
	    if (!x->x_autocreate){
		post ("pdp_sdl: autocreate failed %d times: disabled", PDP_AUTOCREATE_RETRY);
		post ("pdp_sdl: send [autocreate 1] message to re-enable");
		return 0;
	    }
	}
	else return 1;

    }
    return 0;
}

static void pdp_sdl_bang(t_pdp_sdl *x);

static void pdp_sdl_process(t_pdp_sdl *x)
{
    t_pdp *header = pdp_packet_header(x->x_packet0);
    void  *data   = pdp_packet_data  (x->x_packet0);


    if (!x->x_backfromthread) return;

    /* check if window is initialized */
    if (!(x->x_initialized)){
        post("trying to autocreate");
	if (!pdp_sdl_try_autocreate(x)) return;
    }

	/* check for pending sdl events */
	pdp_sdl_checkEvents(x);

    /* check data packet */
    if (!(header)) {
	post("pdp_sdl: invalid packet header");
	return;
    }
    if (PDP_IMAGE != header->type) {
	post("pdp_sdl: packet is not a PDP_IMAGE");
	return;
    }
    if (header->info.image.encoding != PDP_IMAGE_YV12) {
	post("pdp_sdl: packet is not a PDP_IMAGE_YV12");
	return;
    }
    
    /* copy the packet to the sdlimage */
	pdp_sdl_drawImage(x, &header->info.image, (short int *)data);

    /* display the new image */
    pdp_sdl_bang(x);
}

static void pdp_sdl_destroy(t_pdp_sdl *x) {
	if (x->x_initialized){
		pdp_sdl_freeOverlay(x);
		SDL_FreeSurface(x->x_sdl_surface);
		x->x_initialized = 0;
	}
}

static void pdp_sdl_random(t_pdp_sdl *x) {
    unsigned int i;
    long *intdata = (long *)(* x->x_sdl_overlay->pixels);
    SDL_LockYUVOverlay(x->x_sdl_overlay);
    for(i=0; i<x->x_width*x->x_height/4; i++) intdata[i]=random();
    SDL_UnlockYUVOverlay(x->x_sdl_overlay);
}

/* redisplays image */
static void pdp_sdl_bang_thread(t_pdp_sdl *x) {
//	if (x->x_sdl_overlay->pixels) {
		if (SDL_DisplayYUVOverlay(x->x_sdl_overlay, &(* x).x_sdl_rect) <0) 
			post("pdp_sdl: __LINE__ cannot display");
//	} 
}

static void pdp_sdl_bang_callback(t_pdp_sdl *x)
{
    x->x_backfromthread = 1;

    /* release the packet if there is one */
    pdp_packet_mark_unused(x->x_packet0);
    x->x_packet0 = -1;}

static void pdp_sdl_bang(t_pdp_sdl *x) {

    /* if previous queued method returned
       schedule a new one, else ignore */
    if (x->x_backfromthread) {
	x->x_backfromthread = 0;
	pdp_queue_add(x, pdp_sdl_bang_thread, pdp_sdl_bang_callback, &x->x_queue_id);
    }
}

static void pdp_sdl_input_0(t_pdp_sdl *x, t_symbol *s, t_floatarg f) {

    if (s == gensym("register_ro")) pdp_packet_copy_ro_or_drop(&x->x_packet0, (int)f);
    if (s == gensym("process")) pdp_sdl_process(x);

}

static void pdp_sdl_autocreate(t_pdp_sdl *x, t_floatarg f) {
  if (f != 0.0f) x->x_autocreate = PDP_AUTOCREATE_RETRY;
  else x->x_autocreate = 0;
}

static void pdp_sdl_fullscreen(t_pdp_sdl *x, t_floatarg f) {
	if (f == x->x_fullscreen) return;
    
    x->x_fullscreen = (f != 0.0f);

	pdp_sdl_destroy(x);
	pdp_sdl_create(x);
	SDL_ShowCursor(0);
}

static void pdp_sdl_free(t_pdp_sdl *x)
{
    pdp_queue_finish(x->x_queue_id);
    pdp_sdl_destroy(x);
    pdp_packet_mark_unused(x->x_packet0);
    SDL_Quit();
}


t_class *pdp_sdl_class;

void *pdp_sdl_new(void)
{
    t_pdp_sdl *x = (t_pdp_sdl *)pd_new(pdp_sdl_class);

    x->x_packet0 = -1;
    x->x_queue_id = -1;
    
	x->x_sdl_surface = NULL;
	x->x_sdl_overlay = NULL;
	x->x_sdl_format = SDL_YV12_OVERLAY;

    x->x_winwidth = PDP_SDL_W;
    x->x_winheight = PDP_SDL_H;

    x->x_width = PDP_SDL_W;
    x->x_height = PDP_SDL_H;

    x->x_backfromthread = 1;
    x->x_initialized = 0;
    pdp_sdl_autocreate(x,1);

    x->x_fullscreen = 0;

    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif


void pdp_sdl_setup(void)
{


    pdp_sdl_class = class_new(gensym("pdp_sdl"), (t_newmethod)pdp_sdl_new,
    	(t_method)pdp_sdl_free, sizeof(t_pdp_sdl), 0, A_NULL);


    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_bang, gensym("bang"), A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_random, gensym("random"), A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_create, gensym("create"), A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_autocreate, gensym("autocreate"), A_FLOAT, A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_destroy, gensym("destroy"), A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_destroy, gensym("close"), A_NULL);
//    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_resize, gensym("dim"), A_FLOAT, A_FLOAT, A_NULL);
//    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_resize, gensym("size"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_fullscreen, gensym("fullscreen"), A_FLOAT, A_NULL);
//    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_display, gensym("display"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
