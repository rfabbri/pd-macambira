/* Copyright 2003 Hans-Christoph Steiner <hans@eds.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * TODO
 * -obj argument selects device# (event.button.which/event.motion.which)
 *
 * $Id $
 */
static char *version = "$Revision $";

#include <SDL/SDL.h>
#include <m_pd.h>
#include "m_imp.h"

#if PD_MINOR_VERSION >= 37 
#include "s_stuff.h"
#endif

/* #define DEBUG(x) */
#define DEBUG(x) x 

#define RAWMOUSE_AXES     2
#define RAWMOUSE_BUTTONS  9

#define RAWMOUSE_XAXIS    0
#define RAWMOUSE_YAXIS    1

/*------------------------------------------------------------------------------
 *  SDL FUNCTIONS
 */

/* Is the cursor visible? */
static int visible = 1;

void HotKey_ToggleFullScreen(void)
{
	SDL_Surface *screen;

	screen = SDL_GetVideoSurface();
	if ( SDL_WM_ToggleFullScreen(screen) ) {
		printf("Toggled fullscreen mode - now %s\n",
		    (screen->flags&SDL_FULLSCREEN) ? "fullscreen" : "windowed");
	} else {
		printf("Unable to toggle fullscreen mode\n");
	}
}

void HotKey_ToggleGrab(void)
{
	SDL_GrabMode mode;

	printf("Ctrl-G: toggling input grab!\n");
	mode = SDL_WM_GrabInput(SDL_GRAB_QUERY);
	if ( mode == SDL_GRAB_ON ) {
		printf("Grab was on\n");
	} else {
		printf("Grab was off\n");
	}
	mode = SDL_WM_GrabInput(mode ? SDL_GRAB_OFF : SDL_GRAB_ON);
	if ( mode == SDL_GRAB_ON ) {
		printf("Grab is now on\n");
	} else {
		printf("Grab is now off\n");
	}
}

void HotKey_Iconify(void)
{
	printf("Ctrl-Z: iconifying window!\n");
	SDL_WM_IconifyWindow();
}

void HotKey_Quit(void)
{
	SDL_Event event;

	printf("Posting internal quit request\n");
	event.type = SDL_USEREVENT;
	SDL_PushEvent(&event);
}

int FilterEvents(const SDL_Event *event)
{
	static int reallyquit = 0;
	
	switch (event->type) {
		
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			/* We want to toggle visibility on buttonpress */
			if ( event->button.state == SDL_PRESSED ) {
				visible = !visible;
				SDL_ShowCursor(visible);
			}
			printf("Mouse button %d has been %s\n",
				event->button.button,
				(event->button.state == SDL_PRESSED) ?
						"pressed" : "released");
			return(0);

		/* Show relative mouse motion */
		case SDL_MOUSEMOTION:
#if 1
			printf("Mouse relative motion: {%d,%d}\n",
				event->motion.xrel, event->motion.yrel);
#endif
			return(0);

		case SDL_KEYDOWN:
			if ( event->key.keysym.sym == SDLK_ESCAPE ) {
				HotKey_Quit();
			}
			if ( (event->key.keysym.sym == SDLK_g) &&
			     (event->key.keysym.mod & KMOD_CTRL) ) {
				HotKey_ToggleGrab();
			}
			if ( (event->key.keysym.sym == SDLK_z) &&
			     (event->key.keysym.mod & KMOD_CTRL) ) {
				HotKey_Iconify();
			}
			if ( (event->key.keysym.sym == SDLK_RETURN) &&
			     (event->key.keysym.mod & KMOD_ALT) ) {
				HotKey_ToggleFullScreen();
			}
			return(0);

		/* Pass the video resize event through .. */
		case SDL_VIDEORESIZE:
			return(1);

		/* This is important!  Queue it if we want to quit. */
		case SDL_QUIT:
			if ( ! reallyquit ) {
				reallyquit = 1;
				printf("Quit requested\n");
				return(0);
			}
			printf("Quit demanded\n");
			return(1);

		/* This will never happen because events queued directly
		   to the event queue are not filtered.
		 */
		case SDL_USEREVENT:
			return(1);

		/* Drop all other events */
		default:
			return(0);
	}
}



/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *rawmouse_class;

typedef struct _rawmouse  {
		t_object            x_obj;
		SDL_Cursor          *x_mouse;
		int                 read_ok;
		int                 started;
		int                 relative;
		t_outlet            *x_axis_out[RAWMOUSE_AXES];
		t_outlet            *x_button_num_out;
		t_outlet            *x_button_val_out;
		t_clock             *x_clock;
		double              x_delaytime;
		int                 x_buttons;
		int                 x_axes;
} t_rawmouse;

/*------------------------------------------------------------------------------
  */

static int rawmouse_close(t_rawmouse *x)  {
    DEBUG(post("rawmouse_CLOSE"));

}

static int rawmouse_open(t_rawmouse *x)  {
  rawmouse_close(x);

  DEBUG(post("rawmouse_OPEN"));

  post ("   device has %i axes and %i buttons.\n",x->x_axes,x->x_buttons);
  post ("WARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING");
  post ("This object is under development!  The interface could change at anytime!");
  post ("As I write cross-platform versions, the interface might have to change.");
  post ("WARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING");
    
  return 1;
}

void rawmouse_start(t_rawmouse* x)
{
  DEBUG(post("rawmouse_START"));

  post("started: %f",x->started);
  if ( ! x->started ) {
/*   SDL_ShowCursor(0); */
	  SDL_WM_GrabInput(SDL_GRAB_ON);
	  
	  clock_delay(x->x_clock, 0);
	  
	  x->started = 1;
  }
}


void rawmouse_stop(t_rawmouse* x)  {
  DEBUG(post("rawmouse_STOP");)

  if ( x->started ) {
/*   SDL_ShowCursor(1); */
	  SDL_WM_GrabInput(SDL_GRAB_OFF);

	  clock_unset(x->x_clock);
	  x->started = 0;
  }
}

static int rawmouse_read(t_rawmouse *x,int fd)  {
  SDL_Event     event; 

  DEBUG(post("rawmouse_READ"));   

  while (SDL_PollEvent(&event)) {
	  post("event type: %s", event.type);
	  switch (event.type) {
		  case SDL_KEYDOWN:
			  post("SDL_KEYDOWN");
			  if ( event.key.keysym.sym == SDLK_ESCAPE )
				  rawmouse_stop(x);
			  break;		  
/* 		  case SDL_MOUSEMOTION: */
/* 			  if (x->relative) { */
/* 				  outlet_float (x->x_axis_out[RAWMOUSE_XAXIS], event.motion.xrel);	 */
/* 				  outlet_float (x->x_axis_out[RAWMOUSE_YAXIS], event.motion.yrel);	 */
/* 			  } else { */
/* 				  outlet_float (x->x_axis_out[RAWMOUSE_XAXIS], event.motion.xrel);	 */
/* 				  outlet_float (x->x_axis_out[RAWMOUSE_YAXIS], event.motion.yrel);	 */
/* 			  } */
/* 			  break; */
/* 		  case SDL_MOUSEBUTTONDOWN: */
/* 			  outlet_float (x->x_button_val_out, 1); */
/* 			  outlet_float (x->x_button_num_out, event.button.button); */
/* 			  break; */
/* 		  case SDL_MOUSEBUTTONUP: */
/* 			  outlet_float (x->x_button_val_out, 0); */
/* 			  outlet_float (x->x_button_num_out, event.button.button); */
/* 			  break; */
		  default:
			  DEBUG(post("Unhandled event."));
	  }
  } 
  
  if (x->started) 
	  clock_delay(x->x_clock, x->x_delaytime);

  return NULL;    
}

/* Actions */

static void rawmouse_bang(t_rawmouse* x)  {
    DEBUG(post("rawmouse_bang"));   
}

static void rawmouse_float(t_rawmouse* x)  {
    DEBUG(post("rawmouse_float"));   
}

void rawmouse_delay(t_rawmouse* x, t_float f)  {
	DEBUG(post("rawmouse_DELAY %f",f);)
	  
   x->x_delaytime = f;
}
void rawmouse_absolute(t_rawmouse* x)  {
	DEBUG(post("rawmouse_ABSOLUTE"));
	  
   x->relative = 0;
}
void rawmouse_relative(t_rawmouse* x)  {
	DEBUG(post("rawmouse_RELATIVE"));
	  
   x->relative = 1;
}


/* SETUP FUNCTIONS */

static void rawmouse_free(t_rawmouse* x) {
  DEBUG(post("rawmouse_free"));
    
  rawmouse_stop(x);
  
  SDL_Quit();

  clock_free(x->x_clock);
}

static void *rawmouse_new(t_float argument) {
  int i;
  t_rawmouse *x = (t_rawmouse *)pd_new(rawmouse_class);

  DEBUG(post("rawmouse_NEW"));
  post("rawHID(e) rawmouse  %s, <hans@eds.org>", version);

  /* init vars */
  x->read_ok = 1;
  x->started = 0;
  x->relative = 1;

  x->x_delaytime = 5;

  x->x_clock = clock_new(x, (t_method)rawmouse_read);

  /* Note: Video is required to start Event Loop !! */
  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) { 
	  post("Could not initialize SDL: %s\n", SDL_GetError());
	  // exit(-1);
	  return (0);	/* changed by olafmatt */
  }    
  atexit(SDL_Quit);
  
  SDL_WM_SetCaption(title, "rawmouse");
  
  /* create outlets for each axis */
  for (i = 0; i < RAWMOUSE_AXES; i++) 
    x->x_axis_out[i] = outlet_new(&x->x_obj, &s_float);
  
  /* create outlets for buttons */
  x->x_button_num_out = outlet_new(&x->x_obj, &s_float);
  x->x_button_val_out = outlet_new(&x->x_obj, &s_float);
  
  /* Open the device and save settings */
  if ( ! rawmouse_open(x) ) return x;
  
  return (x);
}


void rawmouse_setup(void) {
  DEBUG(post("rawmouse_setup");)
  rawmouse_class = class_new(gensym("rawmouse"), 
			     (t_newmethod)rawmouse_new, 
			     (t_method)rawmouse_free,
			     sizeof(t_rawmouse),0,A_DEFFLOAT,0);

  /* add inlet datatype methods */
  class_addfloat(rawmouse_class,(t_method) rawmouse_float);
  class_addbang(rawmouse_class,(t_method) rawmouse_bang);

  /* add inlet message methods */
  class_addmethod(rawmouse_class,(t_method) rawmouse_open,gensym("open"),0);
  class_addmethod(rawmouse_class,(t_method) rawmouse_close,gensym("close"),0);

  class_addmethod(rawmouse_class,(t_method) rawmouse_start,gensym("start"),0);
  class_addmethod(rawmouse_class,(t_method) rawmouse_stop,gensym("stop"),0);

  class_addmethod(rawmouse_class,(t_method) rawmouse_read,gensym("read"),0);

  class_addmethod(rawmouse_class,(t_method) rawmouse_delay,gensym("delay"),A_FLOAT,0);

  class_addmethod(rawmouse_class,(t_method) rawmouse_absolute,gensym("absolute"),0);
  class_addmethod(rawmouse_class,(t_method) rawmouse_relative,gensym("relative"),0);
}

