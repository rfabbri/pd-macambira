/*
 *   Pure Data Packet module.
 *   Copyright (c) 2003 by martin pi <pi@attacksyour.net>
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
//#include <unistd.h>
//#include <sys/ipc.h>
//#include <sys/shm.h>

#include <SDL/SDL.h>

//#include <quicktime/lqt.h>
//#include <quicktime/colormodels.h>

#include "pdp.h"
#include "pdp_llconv.h"


/* initial image dimensions */
#define PDP_SDL_W 320
#define PDP_SDL_H 240


typedef struct pdp_sdl_struct {
    t_object x_obj;
    t_float x_f;

    int x_packet0;
    int x_queue_id;

	SDL_Surface *x_sdl_surface;
	SDL_Overlay *x_sdl_overlay;
	SDL_Rect x_sdl_rect;
	
	int x_xid;

    Uint32 x_sdl_format;

    int x_winwidth;
    int x_winheight;

    unsigned int x_width;
    unsigned int x_height;
    int x_last_encoding;
    int x_cursor;

    int x_initialized;
    int x_backfromthread;

    int x_fullscreen;

} t_pdp_sdl;

static SDL_Surface *pdp_sdl_getSurface(int xid, char* title, int width, int height, int bits, int fullscreenflag, int cursorflag) {
	Uint32 flags;
	int size,i;
	SDL_Surface *screen;
	char SDL_hack[32];

	/* next lines from gstreamer plugin sdlvideosink */
	if (xid < 0) unsetenv("SDL_WINDOWID");
	else {
		sprintf(SDL_hack, "%d", xid);
		setenv("SDL_WINDOWID", SDL_hack, 1);
	}

	/* Initialize SDL */
	if (!SDL_WasInit(SDL_INIT_VIDEO)) {
		if (SDL_Init (SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE)) {
			post("SDL: Initializing of SDL failed: %s.\n", SDL_GetError());
			return NULL;
		}
		/* ignore events :: only keys and wm_quit */
		for ( i=SDL_NOEVENT; i<SDL_NUMEVENTS; ++i )
			if( !(i & (SDL_KEYDOWN|SDL_VIDEORESIZE)) ) 
				SDL_EventState(i, SDL_IGNORE);

		
	}

/*	
gem :    SDL_OPENGL|SDL_DOUBLEBUF|SDL_HWSURFACE|SDL_ANYFORMAT|SDL_OPENGLBLIT; 
working: SDL_ANYFORMAT|SDL_RESIZABLE|SDL_RLEACCEL; 
*/
	flags = SDL_SWSURFACE | SDL_RESIZABLE;
//	flags = SDL_HWSURFACE|SDL_RESIZABLE|SDL_ASYNCBLIT|SDL_HWACCEL|SDL_ANYFORMAT|SDL_RLEACCEL;
	if ( fullscreenflag>0 ) {
		flags |= SDL_FULLSCREEN|SDL_DOUBLEBUF;
	}

	screen = SDL_SetVideoMode(width, height, bits, flags);
	if ( screen == NULL ) {
		post("Couldn't set video mode: %s\n", SDL_GetError());
		return NULL;
	}

	SDL_WM_SetCaption (title, title);
  
	SDL_ShowCursor(cursorflag);

	return screen;		
}

static SDL_Surface *pdp_sdl_recreateSurface(SDL_Surface *old, int xid, char* title, int width, int height, int bits, int fullscreenflag, int cursorflag) {
	SDL_Surface *new = pdp_sdl_getSurface(xid, title, width, height, bits, fullscreenflag, cursorflag);
	if (new != NULL) SDL_FreeSurface(old);
	return new;
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

static void pdp_sdl_fullscreen(t_pdp_sdl *x, t_floatarg f);

static inline void pdp_sdl_recreate(t_pdp_sdl *x) {
	x->x_sdl_surface = pdp_sdl_recreateSurface(x->x_sdl_surface, x->x_xid,"pdp-sdl", x->x_winwidth, x->x_winheight, 16, x->x_fullscreen,x->x_cursor);
}

static int pdp_sdl_create(t_pdp_sdl *x) {
	if (x->x_initialized){
		return 0;
	}
	x->x_initialized = 0;
	
	x->x_sdl_surface = pdp_sdl_getSurface(x->x_xid, "pdp-sdl", x->x_winwidth, x->x_winheight, 16, x->x_fullscreen,x->x_cursor);
	if (x->x_sdl_surface != NULL) {
		pdp_sdl_getOverlay(x);
		if (x->x_sdl_overlay != NULL) {
			x->x_sdl_rect.x = 0;
			x->x_sdl_rect.y = 0;
			x->x_sdl_rect.w = x->x_winwidth;
			x->x_sdl_rect.h = x->x_winheight;
			x->x_initialized = 1;
			post("created successfully");
		}
	}
	x->x_backfromthread = 1;
	
	return x->x_initialized;
}

static void pdp_sdl_destroy(t_pdp_sdl *x);
static void pdp_sdl_resize(t_pdp_sdl *x,t_floatarg,t_floatarg);

static void pdp_sdl_checkEvents(t_pdp_sdl *x) {
	Uint8 *keys;
	SDL_Event event;

	if (SDL_PollEvent(&event)!=1) return;

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
		case SDL_VIDEORESIZE:
			pdp_sdl_resize(x,(t_floatarg)event.resize.w,(t_floatarg)event.resize.h);
			break;
		default:
			break;
	}


}

static void pdp_sdl_bang(t_pdp_sdl *x);

static void pdp_sdl_process(t_pdp_sdl *x)
{
    t_pdp *header = pdp_packet_header(x->x_packet0);
    void  *data   = pdp_packet_data  (x->x_packet0);


    if (!x->x_backfromthread) return;

    /* check if window is initialized */
    if (!(x->x_initialized)){
	if (!pdp_sdl_create(x)) return;
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
		SDL_Quit();
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
	if (SDL_DisplayYUVOverlay(x->x_sdl_overlay, &(* x).x_sdl_rect) <0) 
		post("pdp_sdl: __LINE__ cannot display");
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

static void pdp_sdl_resize(t_pdp_sdl* x, t_floatarg width, t_floatarg height) {

    if (x->x_initialized && (!x->x_fullscreen) && (width>0) && (height>0)) {
	post("should get %d/%d",(int)width,(int) height);
	x->x_winwidth=(int)width;
	x->x_winheight=(int)height;
	pdp_sdl_recreate(x);
    }
}


static void pdp_sdl_fullscreen(t_pdp_sdl *x, t_floatarg f) {
	if (f == x->x_fullscreen) return;
    
	x->x_fullscreen = (f != 0.0f);
	x->x_cursor=0;

	pdp_sdl_recreate(x);
}

static void pdp_sdl_cursor(t_pdp_sdl *x, t_floatarg f) {
	if (f == x->x_cursor) return;
    
	x->x_cursor = (f != 0.0f);
	SDL_ShowCursor(x->x_cursor);
}


/* sets new target window */

static void pdp_sdl_win(t_pdp_sdl *x, t_floatarg *f) {
	pdp_queue_finish(x->x_queue_id);
	x->x_queue_id = -1;
	x->x_xid = (int)f;
	pdp_sdl_recreate(x);
}

/* be very carefule not to set DGA fro here! */
/* use export SDL_VIDEODRIVER=dga (or equivalent for your shell) instead */

static void pdp_sdl_renderer(t_pdp_sdl *x, t_symbol *s) {
	char SDL_hack[32];

	pdp_sdl_destroy(x);

	/* next lines from gstreamer plugin sdlvideosink */
	unsetenv("SDL_VIDEODRIVER");

	sprintf(SDL_hack, "%s", s->s_name);
	setenv("SDL_VIDEODRIVER", SDL_hack, 1);

	pdp_sdl_create(x);
}

static void pdp_sdl_free(t_pdp_sdl *x)
{
	pdp_queue_finish(x->x_queue_id);
	pdp_sdl_destroy(x);
	pdp_packet_mark_unused(x->x_packet0);
//	SDL_Quit();
}

static void pdp_sdl_listmodes(const char* title, Uint32 flags) {
	SDL_Rect ** modes;
	int i;
	
	/* Get available modes */
	modes = SDL_ListModes(NULL, flags);

	/* Check is there are any modes available */
	if(modes == (SDL_Rect **)0){
		printf("%s : No modes available!", title);
		return;
	}

	/* Check if our resolution is restricted */
	if(modes == (SDL_Rect **)-1){
		post("%s : All resolutions available.", title);
	} else {
		/* Print valid modes */
		for(i=0;modes[i];++i)
			post("%s : %d x %d", title, modes[i]->w, modes[i]->h);
	}

}

static void pdp_sdl_modes(t_pdp_sdl *x) {
	pdp_sdl_listmodes("FULL|HWSURF|||||||||||||||||||||||||", SDL_FULLSCREEN|SDL_HWSURFACE);
	pdp_sdl_listmodes("HWSURF|RESIZ|ASYNC|HWACCEL||||||||||", SDL_HWSURFACE|SDL_RESIZABLE|SDL_ASYNCBLIT|SDL_HWACCEL);
	pdp_sdl_listmodes("HWSURF|RESIZ|ASYNC|HWACCEL|FULL|DBUF", SDL_HWSURFACE|SDL_RESIZABLE|SDL_ASYNCBLIT|SDL_HWACCEL|SDL_FULLSCREEN|SDL_DOUBLEBUF);
	pdp_sdl_listmodes("OPENGL|DBUF|HWSURF|ANYF|GLBLIT||||||", SDL_OPENGL|SDL_DOUBLEBUF|SDL_HWSURFACE|SDL_ANYFORMAT|SDL_OPENGLBLIT);
	pdp_sdl_listmodes("ANYF|RESIZ|RLEA|||||||||||||||||||||", SDL_ANYFORMAT|SDL_RESIZABLE|SDL_RLEACCEL);
}

static void pdp_sdl_info(t_pdp_sdl *x) {
	const SDL_VideoInfo *narf;
	post("\nSDL video info: note that this only works under dga mode\n");
	narf = SDL_GetVideoInfo();
	post("Is it possible to  create hardware surfaces?\t\thw_available=%d",narf->hw_available);
	post("Is there a window manager available?\t\t\twm_available=%d",narf->wm_available);
	post("Are hardware to hardware blits accelerated?\t\tblit_hw=%d",narf->blit_hw);
	post("Are hardware to hardware colorkey blits accelerated?\tblit_hw_CC=%d",narf->blit_hw_CC);
	post("Are hardware to hardware alpha bits accelerated?\tblit_hw_A=%d",narf->blit_hw_A);
	post("Are software to hardware blits accelerated?\t\tblit_sw=%d",narf->blit_sw);
	post("Are software to hardware colorkey blits accelerated?\tblit_sw_CC=%d",narf->blit_sw_CC);
	post("Are software to hardware alpha blits accelerated?\tblit_sw_A=%d",narf->blit_sw_A);
	post("Are color fills accelerated?\t\t\t\tblit_fill=%d",narf->blit_fill);
	post("Total amount of video_mem: %d",narf->video_mem);

}

t_class *pdp_sdl_class;

void *pdp_sdl_new(void) {


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

	x->x_fullscreen = 0;
	x->x_cursor=1;
	
	x->x_xid = -1;

    pdp_sdl_create(x);

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
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_info, gensym("info"), A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_modes, gensym("modes"), A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_destroy, gensym("destroy"), A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_destroy, gensym("close"), A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_resize, gensym("dim"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_resize, gensym("size"), A_FLOAT, A_FLOAT, A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_fullscreen, gensym("fullscreen"), A_FLOAT, A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_cursor, gensym("cursor"), A_FLOAT, A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_win, gensym("window"), A_FLOAT, A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_renderer, gensym("renderer"), A_SYMBOL, A_NULL);
    class_addmethod(pdp_sdl_class, (t_method)pdp_sdl_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif

