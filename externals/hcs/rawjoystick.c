/*
'pd_joystick' (An external library for Miller Puckette's 'PD' software
adding PC and/or USB joystick control capabilities)

Copyright (C) 2001 Joseph A. Sarlo

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA  

jsarlo@peabody.jhu.edu
*/

#include "m_pd.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#ifdef USB
#define JOYSTICK_DEVICE   "/dev/input/js0"
#else
#define JOYSTICK_DEVICE   "dev/js0"
#endif

// scaling factor for output
#define DEF_SCALE         1
// delay/refresh time in milliseconds
#define DEF_DELTIME       5
// this object handles a maximum of 10 axes
#define MAX_AXIS_OUTS     10
// input event types
#define JS_EVENT_BUTTON   0x01
#define JS_EVENT_AXIS     0x02
#define JS_EVENT_INIT     0x80

// ise ioctl() to get number of axes and buttons from device
#define JSIOCGAXES       _IOR('j', 0x11, unsigned char)
#define JSIOCGBUTTONS    _IOR('j', 0x12, unsigned char)

struct js_event
{
    unsigned int time;
    signed short value;
    unsigned char type;
    unsigned char number;
};

typedef struct _joystick
{
    t_object t_ob;
    struct js_event x_joy_e;
    int x_joy_fd;
    float x_scale;
    float x_translation;
    t_outlet *x_axis_out[10];
    t_outlet *x_button_num_out;
    t_outlet *x_button_val_out;
    t_clock *x_clock;
    double x_deltime;
    unsigned char x_joy_buttons;
    unsigned char x_joy_axes;
}t_joystick;

t_class *joystick_class;

void *joystick_new(t_float deltime, t_float scale, t_float translation); 
void joystick_setup(void);
void joystick_read(t_joystick *x);
void joystick_change_deltime(t_joystick *x, t_float deltime);
void joystick_free(t_joystick *x);

void *joystick_new(t_float deltime, t_float scale, t_float translation)
{  
    int i, num_axes;   
     
    t_joystick *x = (t_joystick *)pd_new(joystick_class);

    // open the joystick device read-only, non-exclusive
    x->x_joy_fd = open (JOYSTICK_DEVICE, O_RDONLY | O_NONBLOCK);

    // read js_events from the JOYSTICK_DEVICE stream
    while (read (x->x_joy_fd, &(x->x_joy_e), sizeof(struct js_event)) > -1);

    // if deltime is set in the object's creation arguments
    // use that value, otherwise use the default
    if (deltime == 0)
      x->x_deltime = DEF_DELTIME;
    else
      x->x_deltime = (int)deltime;

    // if scale is set in the object's creation arguments
    // use that value, otherwise use the default
    if (scale == 0)
      x->x_scale = DEF_SCALE;
    else
      x->x_scale = scale;
    
    // get translation from object arguments
    x->x_translation = translation;
    x->x_joy_axes = 0;
    x->x_joy_buttons = 0;

    // get number of axes and buttons from device
    ioctl (x->x_joy_fd, JSIOCGAXES, &(x->x_joy_axes));
    ioctl (x->x_joy_fd, JSIOCGBUTTONS, &(x->x_joy_buttons));

    // this object handles a maximum of MAX_AXIS_OUTS axes
    // if there are more, limit to MAX_AXIS_OUTS
    if (x->x_joy_axes > MAX_AXIS_OUTS)
    	num_axes = MAX_AXIS_OUTS;
    else
        num_axes = x->x_joy_axes;

    // create outlets for each axis
    for (i = 0; i < num_axes; i++)
        x->x_axis_out[i] = outlet_new(&x->t_ob, &s_float);

    // create outlets for buttons
    x->x_button_num_out = outlet_new(&x->t_ob, &s_float);
    x->x_button_val_out = outlet_new(&x->t_ob, &s_float);


    x->x_clock = clock_new (x, (t_method)joystick_read);

    clock_delay (x->x_clock, x->x_deltime);

    post ("configuring joystick:");
    post ("  found %d axes", x->x_joy_axes);
    post ("  found %d buttons", x->x_joy_buttons);
    return (void *)x;
}

void joystick_setup(void)
{
    post ("joystick object loaded (J. Sarlo)");
    joystick_class = class_new(gensym("joystick"),(t_newmethod)joystick_new, 
                              (t_method)joystick_free, sizeof(t_joystick), 0, A_DEFFLOAT, A_DEFFLOAT, 
                               A_DEFFLOAT, 0);
    class_addfloat(joystick_class, joystick_change_deltime);
}

void joystick_read(t_joystick *x)
{
 	int rt;
    while (read (x->x_joy_fd, &(x->x_joy_e), sizeof(struct js_event)) > -1)
    {
       if (x->x_joy_e.type == JS_EVENT_AXIS)
       {
            outlet_float (x->x_axis_out[x->x_joy_e.number], 
                          ((int)x->x_joy_e.value + x->x_translation) / x->x_scale);
       }
       else if (x->x_joy_e.type == JS_EVENT_BUTTON)
            {
                outlet_float (x->x_button_val_out, x->x_joy_e.value);
                outlet_float (x->x_button_num_out, x->x_joy_e.number);
            }
    }
    clock_delay (x->x_clock, x->x_deltime);
}

void joystick_change_deltime(t_joystick *x, t_float deltime)
{
    if (deltime < 0)
        deltime = 0;
    x->x_deltime = deltime;
}

void joystick_free(t_joystick *x)
{
    close (x->x_joy_fd);
    clock_free (x->x_clock);
}
