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
 */
/* 
 * $Id: rawjoystick.c,v 1.2 2003-08-20 16:20:43 eighthave Exp $
 */
static char *version = "$Revision: 1.2 $";

#include "SDL.h"
#include "m_imp.h"

//#define DEBUG(x)
#define DEBUG(x) x 

#define RAWJOYSTICK_AXES     6
#define RAWJOYSTICK_BUTTONS  9


/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *rawjoystick_class;

typedef struct _rawjoystick  {
  t_object            x_obj;
  SDL_Joystick        *x_joystick;
  t_int               x_devnum;
  int                 read_ok;
  int                 started;
  t_outlet            *x_axis_out[RAWJOYSTICK_AXES];
  t_outlet            *x_button_num_out;
  t_outlet            *x_button_val_out;
  t_clock             *x_clock;
  double              x_delaytime;
  int                 x_buttons;
  int                 x_hats;
  int                 x_axes;
} t_rawjoystick;

/*------------------------------------------------------------------------------
  */

static int rawjoystick_close(t_rawjoystick *x)  {
    DEBUG(post("rawjoystick_CLOSE"));

    if ( SDL_JoystickOpened(x->x_devnum) ) {
      SDL_JoystickClose(x->x_joystick);
      return 1;
    }
    else {	 
      return 0;
    }
}

static int rawjoystick_open(t_rawjoystick *x)  {
  rawjoystick_close(x);

  DEBUG(post("rawjoystick_OPEN"));
  
  /* open device */
  SDL_JoystickEventState(SDL_ENABLE);
  x->x_joystick = SDL_JoystickOpen(x->x_devnum);

  /* test if device open */
  /* get name of device */  
  if ( SDL_JoystickOpened(x->x_devnum) ) {
    post ("Configuring %s",SDL_JoystickName(x->x_devnum));
  }
  else {	 
    return 0;
  }

  x->x_axes = SDL_JoystickNumAxes(x->x_joystick);
  x->x_hats = SDL_JoystickNumHats(x->x_joystick);
  x->x_buttons = SDL_JoystickNumButtons(x->x_joystick);

  post ("   device has %i axes, %i hats, and %i buttons.\n",x->x_axes,x->x_hats,x->x_buttons);
  post ("WARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING");
  post ("This object is under development!  The interface could change at anytime!");
  post ("As I write cross-platform versions, the interface might have to change.");
  post ("WARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING");
    
  return 1;
}

static int rawjoystick_read(t_rawjoystick *x,int fd)  {
  SDL_Event           event; 

  DEBUG(post("rawjoystick_READ"));   

  if ( ! SDL_JoystickOpened(x->x_devnum) ) {
    return 0;
  }

  post("Joystick read: %s",SDL_JoystickName(x->x_devnum));

  if ( SDL_PollEvent(&event) ) {
    post("SDL_Event.type: %i",event.type);
    post("SDL_JoyAxisEvent.value: %i",event.jaxis.value);
    post("SDL_JoyButtonEvent.value: %i",event.jbutton.state);
    switch (event.type) {
    case SDL_JOYAXISMOTION:
      outlet_float (x->x_axis_out[event.jaxis.axis], event.jaxis.value);	
      break;
    case SDL_JOYHATMOTION:
      break;
    case SDL_JOYBUTTONDOWN:
      outlet_float (x->x_button_val_out, 1);
      outlet_float (x->x_button_num_out, event.jaxis.axis);
      break;
    case SDL_JOYBUTTONUP:
      outlet_float (x->x_button_val_out, 0);
      outlet_float (x->x_button_num_out, event.jaxis.axis);
      break;
    default:
      DEBUG(post("Unhandled event."));
    }
  }
  return 1;    
}

/* Actions */

static void rawjoystick_bang(t_rawjoystick* x)  {
    DEBUG(post("rawjoystick_bang"));   
}

static void rawjoystick_float(t_rawjoystick* x)  {
    DEBUG(post("rawjoystick_float"));   
}

// DONE
void rawjoystick_start(t_rawjoystick* x)
{
  DEBUG(post("rawjoystick_START"));

  if ( ( SDL_JoystickOpened(x->x_devnum) ) && ( ! x->started ) ) {
    sys_addpollfn(x->x_devnum, (t_fdpollfn)rawjoystick_read, x);
    x->started = 1;
  }
}


// DONE
void rawjoystick_stop(t_rawjoystick* x)  {
  DEBUG(post("rawjoystick_STOP");)

  if ( ( SDL_JoystickOpened(x->x_devnum) ) && ( x->started ) ) {
    sys_rmpollfn(x->x_devnum);
    x->started = 0;
  }
}

/* Misc setup functions */


static void rawjoystick_free(t_rawjoystick* x) {
  DEBUG(post("rawjoystick_free"));
    
  rawjoystick_stop(x);
  
  if ( SDL_JoystickOpened(x->x_devnum)) 
    SDL_JoystickClose(x->x_joystick);
  
  SDL_Quit();
}

static void *rawjoystick_new(t_float argument) {
  int i,joystickNumber;
  t_rawjoystick *x = (t_rawjoystick *)pd_new(rawjoystick_class);

  DEBUG(post("rawjoystick_NEW"));
  post("rawHID objects, %s", version);
  post("       by Hans-Christoph Steiner <hans@eds.org>");

  /* init vars */
  x->x_devnum = 0;
  x->read_ok = 1;
  x->started = 0;

  /* INIT SDL using joystick layer  */  
  /* Note: Video is required to start Event Loop !! */
  if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) == -1 ) { 
    post("Could not initialize SDL: %s.\n", SDL_GetError());
    // exit(-1);
	return (0);	/* changed by olafmatt */
  }    

  post("%i joysticks were found:", SDL_NumJoysticks() );
  
  for( i=0; i < SDL_NumJoysticks(); i++ ) {
    post("    %s", SDL_JoystickName(i));
  }  

  joystickNumber = (int)argument;

  if ( (joystickNumber >= 0) && (joystickNumber < SDL_NumJoysticks() ) )
    x->x_devnum = joystickNumber;
  else 
    post("Joystick %i does not exist!",joystickNumber);
  
  /* create outlets for each axis */
  for (i = 0; i < RAWJOYSTICK_AXES; i++) 
    x->x_axis_out[i] = outlet_new(&x->x_obj, &s_float);
  
  /* create outlets for buttons */
  x->x_button_num_out = outlet_new(&x->x_obj, &s_float);
  x->x_button_val_out = outlet_new(&x->x_obj, &s_float);
  
  /* Open the device and save settings */
  
  if ( ! rawjoystick_open(x) ) return x;
  
  return (x);
}


void rawjoystick_setup(void)
{
  DEBUG(post("rawjoystick_setup");)
  rawjoystick_class = class_new(gensym("rawjoystick"), 
			     (t_newmethod)rawjoystick_new, 
			     (t_method)rawjoystick_free,
			     sizeof(t_rawjoystick),0,A_DEFFLOAT,0);

  /* add inlet datatype methods */
  class_addfloat(rawjoystick_class,(t_method) rawjoystick_float);
  class_addbang(rawjoystick_class,(t_method) rawjoystick_bang);

  /* add inlet message methods */
  class_addmethod(rawjoystick_class,(t_method) rawjoystick_open,gensym("open"),0);
  class_addmethod(rawjoystick_class,(t_method) rawjoystick_close,gensym("close"),0);
  class_addmethod(rawjoystick_class,(t_method) rawjoystick_start,gensym("start"),0);
  class_addmethod(rawjoystick_class,(t_method) rawjoystick_stop,gensym("stop"),0);
  class_addmethod(rawjoystick_class,(t_method) rawjoystick_read,gensym("read"),0);
  
}

