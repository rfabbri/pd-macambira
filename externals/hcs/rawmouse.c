/*
rawmouse - an external object for Miller Puckette's Pure Data

This object gets the raw data from the mouse for use in Pd
It is based on J. Sarlo's joystick

Copyright (C) 2003 Hans-Christoph Steiner <hans@eds.org>

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

*/

#include "m_pd.h"

#include <linux/input.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define RAWMOUSE_DEVICE   "/dev/input/event0"

// scaling factor for output
#define RAWMOUSE_SCALE         1
// delay/refresh time in milliseconds
#define RAWMOUSE_DELAYTIME       5

/* total supported number of axes and buttons */
#define RAWMOUSE_AXES     3
#define RAWMOUSE_BUTTONS  7

/* from <linux/input.h>
 * supported buttons
#define BTN_LEFT		0x110
#define BTN_RIGHT		0x111
#define BTN_MIDDLE		0x112
#define BTN_SIDE		0x113
#define BTN_EXTRA		0x114
#define BTN_FORWARD		0x115
#define BTN_BACK		0x116

 * supported axes
#define REL_X			0x00
#define REL_Y			0x01
#define REL_WHEEL		0x08
*/

/*------------------------------------------------------------------------------
 * from evtest.c from the ff-utils package
 */

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)


char *events[EV_MAX + 1] = { "Reset", "Key", "Relative", "Absolute", NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, "LED", "Sound", NULL, "Repeat", "ForceFeedback", NULL, "ForceFeedbackStatus"};
char *keys[KEY_MAX + 1] = { "Reserved", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "Minus", "Equal", "Backspace",
"Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "LeftBrace", "RightBrace", "Enter", "LeftControl", "A", "S", "D", "F", "G",
"H", "J", "K", "L", "Semicolon", "Apostrophe", "Grave", "LeftShift", "BackSlash", "Z", "X", "C", "V", "B", "N", "M", "Comma", "Dot",
"Slash", "RightShift", "KPAsterisk", "LeftAlt", "Space", "CapsLock", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10",
"NumLock", "ScrollLock", "KP7", "KP8", "KP9", "KPMinus", "KP4", "KP5", "KP6", "KPPlus", "KP1", "KP2", "KP3", "KP0", "KPDot", "103rd",
"F13", "102nd", "F11", "F12", "F14", "F15", "F16", "F17", "F18", "F19", "F20", "KPEnter", "RightCtrl", "KPSlash", "SysRq",
"RightAlt", "LineFeed", "Home", "Up", "PageUp", "Left", "Right", "End", "Down", "PageDown", "Insert", "Delete", "Macro", "Mute",
"VolumeDown", "VolumeUp", "Power", "KPEqual", "KPPlusMinus", "Pause", "F21", "F22", "F23", "F24", "KPComma", "LeftMeta", "RightMeta",
"Compose", "Stop", "Again", "Props", "Undo", "Front", "Copy", "Open", "Paste", "Find", "Cut", "Help", "Menu", "Calc", "Setup",
"Sleep", "WakeUp", "File", "SendFile", "DeleteFile", "X-fer", "Prog1", "Prog2", "WWW", "MSDOS", "Coffee", "Direction",
"CycleWindows", "Mail", "Bookmarks", "Computer", "Back", "Forward", "CloseCD", "EjectCD", "EjectCloseCD", "NextSong", "PlayPause",
"PreviousSong", "StopCD", "Record", "Rewind", "Phone", "ISOKey", "Config", "HomePage", "Refresh", "Exit", "Move", "Edit", "ScrollUp",
"ScrollDown", "KPLeftParenthesis", "KPRightParenthesis",
"International1", "International2", "International3", "International4", "International5",
"International6", "International7", "International8", "International9",
"Language1", "Language2", "Language3", "Language4", "Language5", "Language6", "Language7", "Language8", "Language9",
NULL, 
"PlayCD", "PauseCD", "Prog3", "Prog4", "Suspend", "Close",
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
"Btn0", "Btn1", "Btn2", "Btn3", "Btn4", "Btn5", "Btn6", "Btn7", "Btn8", "Btn9",
NULL, NULL,  NULL, NULL, NULL, NULL,
"LeftBtn", "RightBtn", "MiddleBtn", "SideBtn", "ExtraBtn", "ForwardBtn", "BackBtn",
NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
"Trigger", "ThumbBtn", "ThumbBtn2", "TopBtn", "TopBtn2", "PinkieBtn",
"BaseBtn", "BaseBtn2", "BaseBtn3", "BaseBtn4", "BaseBtn5", "BaseBtn6",
NULL, NULL, NULL, "BtnDead",
"BtnA", "BtnB", "BtnC", "BtnX", "BtnY", "BtnZ", "BtnTL", "BtnTR", "BtnTL2", "BtnTR2", "BtnSelect", "BtnStart", "BtnMode",
"BtnThumbL", "BtnThumbR", NULL,
"ToolPen", "ToolRubber", "ToolBrush", "ToolPencil", "ToolAirbrush", "ToolFinger", "ToolMouse", "ToolLens", NULL, NULL,
"Touch", "Stylus", "Stylus2" };

char *absval[5] = { "Value", "Min  ", "Max  ", "Fuzz ", "Flat " };
char *relatives[REL_MAX + 1] = { "X", "Y", "Z", NULL, NULL, NULL, "HWheel", "Dial", "Wheel" };
char *absolutes[ABS_MAX + 1] = { "X", "Y", "Z", "Rx", "Ry", "Rz", "Throttle", "Rudder", "Wheel", "Gas", "Brake",
NULL, NULL, NULL, NULL, NULL,
"Hat0X", "Hat0Y", "Hat1X", "Hat1Y", "Hat2X", "Hat2Y", "Hat3X", "Hat 3Y", "Pressure", "Distance", "XTilt", "YTilt"};
char *leds[LED_MAX + 1] = { "NumLock", "CapsLock", "ScrollLock", "Compose", "Kana", "Sleep", "Suspend", "Mute" };
char *repeats[REP_MAX + 1] = { "Delay", "Period" };
char *sounds[SND_MAX + 1] = { "Bell", "Click" };

char **names[EV_MAX + 1] = { events, keys, relatives, absolutes, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
NULL, NULL, leds, sounds, NULL, repeats, NULL, NULL, NULL };

/*------------------------------------------------------------------------------
 */


/*
 * The event structure itself from <linux/input.h>
 * only here as a reference

struct input_event {
	struct timeval time;
	unsigned short type;
	unsigned short code;
	unsigned int value;
};
*/

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */

typedef struct _rawmouse {
  t_object t_ob;
  struct input_event x_mouse_event[64]; /* the events (up to 64 at once) */
  int x_mouse_fd;
  float x_scale;
  float x_translation;
  t_outlet *x_axis_out[RAWMOUSE_AXES];
  t_outlet *x_button_num_out;
  t_outlet *x_button_val_out;
  t_clock *x_clock;
  double x_delaytime;
  unsigned char x_mouse_buttons;
  unsigned char x_mouse_axes;
}t_rawmouse;

t_class *rawmouse_class;

 
/*------------------------------------------------------------------------------
 * INTERFACE                   
 */

void *rawmouse_new(t_float delaytime, t_float scale, t_float translation); 
void rawmouse_setup(void);
void rawmouse_read(t_rawmouse *x);
void rawmouse_change_delaytime(t_rawmouse *x, t_float delaytime);
void rawmouse_free(t_rawmouse *x);


/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

void *rawmouse_new(t_float delaytime, t_float scale, t_float translation)
{  
    int i,eventtype, eventcode, num_axes;   
    unsigned long bitmask[EV_MAX][NBITS(KEY_MAX)];
    char devicename[256] = "Unknown";
	
     
    t_rawmouse *x = (t_rawmouse *)pd_new(rawmouse_class);

    /* open the rawmouse device read-only, non-exclusive */
    x->x_mouse_fd = open (RAWMOUSE_DEVICE, O_RDONLY | O_NONBLOCK);

    /* read input_events from the RAWMOUSE_DEVICE stream 
     *
     * what is this doing?  its doesn't make sense
     * there seems to be two while loops in joystick.c reading
     * the input device, one in _new, and the other in _read
    while (read (x->x_mouse_fd, &(x->x_mouse_event), sizeof(struct input_event)) > -1);
    */
    while ( read(x->x_mouse_fd, &(x->x_mouse_event), sizeof(struct input_event) * 64) > -1);
    
    /* if delaytime is set in the object's creation arguments
     * use that value, otherwise use the default  */
    if (delaytime == 0)
      x->x_delaytime = RAWMOUSE_DELAYTIME;
    else
      x->x_delaytime = (int)delaytime;

    /* if scale is set in the object's creation arguments
     * use that value, otherwise use the default */
    if (scale == 0)
      x->x_scale = RAWMOUSE_SCALE;
    else
      x->x_scale = scale;
    
    /* get translation from object arguments */
    x->x_translation = translation;

    /* from evtest.c in the ff-utils distro
     * get the number of supported axes and buttons
     * EVIOCGBIT (EV_KEY) for buttons
     * EVIOCGBIT (EV_REL) for axes
     */

    /* get name of device */
    ioctl(x->x_mouse_fd, EVIOCGNAME(sizeof(devicename)), devicename);
    post ("configuring %s",devicename);

    /* get bitmask representing supported events (axes, buttons, etc.) */
    memset(bitmask, 0, sizeof(bitmask));
    ioctl(x->x_mouse_fd, EVIOCGBIT(0, EV_MAX), bitmask[0]);
    post("Supported events:");
    
    x->x_mouse_axes = 0;
    x->x_mouse_buttons = 0;
    
    /* cycle through all possible event types */
    for (eventtype = 0; eventtype < EV_MAX; eventtype++) {
      if (test_bit(eventtype, bitmask[0])) {
	post(" %s (type %d) ", events[eventtype] ? events[eventtype] : "?", eventtype);
	//	post("Event type %d",eventtype);

	/* get bitmask representing supported button types */
	ioctl(x->x_mouse_fd, EVIOCGBIT(eventtype, KEY_MAX), bitmask[eventtype]);

	/* cycle through all possible event codes (axes, keys, etc.) 
	 * testing to see which are supported  
	 */
	for (eventcode = 0; eventcode < KEY_MAX; eventcode++) 
	  if (test_bit(eventcode, bitmask[eventtype])) {
	    post("    Event code %d (%s)", eventcode, names[eventtype] ? (names[eventtype][eventcode] ? names[eventtype][eventcode] : "?") : "?");
	    //	    post ("    Event code %d", eventcode); 	    
	    x->x_mouse_buttons += 1;
	    post(" number: %d   eventtype: %d    eventcode: %d",x->x_mouse_buttons, eventtype, eventcode);
	  }
      }        
    }
    
    /* create outlets for each axis */
    for (i = 0; i < RAWMOUSE_AXES; i++)
      x->x_axis_out[i] = outlet_new(&x->t_ob, &s_float);

    /* create outlets for buttons */
    x->x_button_num_out = outlet_new(&x->t_ob, &s_float);
    x->x_button_val_out = outlet_new(&x->t_ob, &s_float);

    /* get pd clock */
    x->x_clock = clock_new (x, (t_method)rawmouse_read);

    /* set refresh time */
    clock_delay (x->x_clock, x->x_delaytime);

    post ("  found %d axes", x->x_mouse_axes);
    post ("  found %d buttons", x->x_mouse_buttons);

    return (void *)x;
}

void rawmouse_setup(void)
{
  post ("rawmouse object loaded using %s",RAWMOUSE_DEVICE);
  rawmouse_class = class_new(gensym("rawmouse"),(t_newmethod)rawmouse_new, 
			     (t_method)rawmouse_free, sizeof(t_rawmouse), 0, A_DEFFLOAT, A_DEFFLOAT, 
			     A_DEFFLOAT, 0);
  class_addfloat(rawmouse_class, rawmouse_change_delaytime);
}

void rawmouse_read(t_rawmouse *x)
{
  /* Currently, the mouse button #s are directly taken from the 
   * sequence in the Linux input event system <linux/input.h>
   * this might have to change when creating the MacOS X and 
   * Windows objects in order to keep it consistent across
   * all platforms.  Let's hope the order is the same on all...
   */
  int i;              /* loop counter */
  int axis_num;
  t_float button_num;
  size_t read_bytes;  /* how many bytes were read */

  while (1) {
    read_bytes = read(x->x_mouse_fd, &(x->x_mouse_event), sizeof(struct input_event) * 64);
    
    if (read_bytes < (int) sizeof(struct input_event)) {
      post("rawmouse: short read");
      break;
    }
  
    for (i = 0; i < (int) (read_bytes / sizeof(struct input_event)); i++)  {
      post("Event: time %ld.%06ld, type %d, code %d, value %d",
	   x->x_mouse_event[i].time.tv_sec, x->x_mouse_event[i].time.tv_usec, 
	   x->x_mouse_event[i].type,x->x_mouse_event[i].code, x->x_mouse_event[i].value);

      if ( x->x_mouse_event[i].type == EV_KEY ) {
	/* key/button event type */
	switch ( x->x_mouse_event[i].code ) {
	case BTN_LEFT:
	  button_num = 0;
	  break;
	case BTN_RIGHT:
	  button_num = 1;
	  break;
	case BTN_MIDDLE:
	  button_num = 2;
	  break;
	case BTN_SIDE:
	  button_num = 3;
	  break;
	case BTN_EXTRA:
	  button_num = 4;
	  break;
	case BTN_FORWARD:
	  button_num = 5;
	  break;
	case BTN_BACK:
	  button_num = 6;
	  break;
	}
	outlet_float (x->x_button_val_out, x->x_mouse_event[i].value);
	outlet_float (x->x_button_num_out, button_num);
      }
      else if  ( x->x_mouse_event[i].type == EV_REL ) {
	/* Relative Axes Event Type */
	switch ( x->x_mouse_event[i].code ) {
	case REL_X:
	  axis_num = 0;
	  break;
	case REL_Y:
	  axis_num = 1;
	  break;
	case REL_WHEEL:
	  axis_num = 2;
	  break;
	}
	outlet_float (x->x_axis_out[axis_num], x->x_mouse_event[i].value);	
      }
    }
  }
  clock_delay (x->x_clock, x->x_delaytime);
}

void rawmouse_change_delaytime(t_rawmouse *x, t_float delaytime)
{
  if (delaytime < 0)
    delaytime = 0;
  x->x_delaytime = delaytime;
}

void rawmouse_free(t_rawmouse *x)
{
  close (x->x_mouse_fd);
  clock_free (x->x_clock);
}
