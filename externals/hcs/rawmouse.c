/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_imp.h>

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#include <linux/input.h>

#include <sys/stat.h>

#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>

#define DEBUG(x)
/*#define DEBUG(x) x */


#define RAWMOUSE_DEVICE   "/dev/input/event0"

// scaling factor for output
#define DEF_SCALE         1
// delay/refresh time in milliseconds
#define DEF_DELTIME       5

#define RAWMOUSE_AXES     3
#define RAWMOUSE_BUTTONS  7

/* from <linux/input.h>
// button types
#define BTN_LEFT		0x110
#define BTN_RIGHT		0x111
#define BTN_MIDDLE		0x112
#define BTN_SIDE		0x113
#define BTN_EXTRA		0x114
#define BTN_FORWARD		0x115
#define BTN_BACK		0x116

// axes
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

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *rawmouse_class;

typedef struct _rawmouse {
  t_object            x_obj;
  t_int               x_fd;
  t_symbol*           x_devname;
  int                 read_ok;
  int                 started;
  struct input_event  x_input_event; 
  float               x_scale;
  float               x_translation;
  t_outlet            *x_axis_out[RAWMOUSE_AXES];
  t_outlet            *x_button_num_out;
  t_outlet            *x_button_val_out;
  t_clock             *x_clock;
  double              x_delaytime;
  unsigned char       x_buttons;
  unsigned char       x_axes;
}t_rawmouse;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

//DONE
static int rawmouse_close(t_rawmouse *x)
{
    DEBUG(post("rawmouse_close");)

     if (x->x_fd <0) return 0;

     close (x->x_fd);

     return 1;
}

//DONE
static int rawmouse_open(t_rawmouse *x,t_symbol* s)
{
  int eventType, eventCode;
  unsigned long bitmask[EV_MAX][NBITS(KEY_MAX)];
  char devicename[256] = "Unknown";
  DEBUG(post("rawmouse_open");)

  rawmouse_close(x);

  /* set obj device name to parameter 
   * otherwise set to default
   */  
  if (s != &s_)
    x->x_devname = s;
  else {
    post("You need to set a input device (i.e /dev/input/event0)");
  }
  
  /* open device */
  if (x->x_devname) {
    post("opening ...");
    /* open the rawmouse device read-only, non-exclusive */
    x->x_fd = open (x->x_devname->s_name, O_RDONLY | O_NONBLOCK);
    if (x->x_fd >= 0 ) post("done");
    else post("failed");
  }
  else {
    return 1;
  }
  
  /* test if device open */
  if (x->x_fd >= 0)
    post("%s opened",x->x_devname->s_name);
  else {
    post("unable to open %s",x->x_devname->s_name);
    x->x_fd = -1;
    return 0;
  }
  
  /* read input_events from the RAWMOUSE_DEVICE stream 
   * It seems that is just there to flush the event input buffer?
   */
  while (read (x->x_fd, &(x->x_input_event), sizeof(struct input_event)) > -1);
  
  /* get name of device */
  ioctl(x->x_fd, EVIOCGNAME(sizeof(devicename)), devicename);
  post ("configuring %s",devicename);

  /* get bitmask representing supported events (axes, buttons, etc.) */
  memset(bitmask, 0, sizeof(bitmask));
  ioctl(x->x_fd, EVIOCGBIT(0, EV_MAX), bitmask[0]);
  post("Supported events:");
    
  x->x_axes = 0;
  x->x_buttons = 0;
    
  /* cycle through all possible event types */
  for (eventType = 0; eventType < EV_MAX; eventType++) {
    if (test_bit(eventType, bitmask[0])) {
      post(" %s (type %d) ", events[eventType] ? events[eventType] : "?", eventType);
      //	post("Event type %d",eventType);

      /* get bitmask representing supported button types */
      ioctl(x->x_fd, EVIOCGBIT(eventType, KEY_MAX), bitmask[eventType]);

      /* cycle through all possible event codes (axes, keys, etc.) 
       * testing to see which are supported  
       */
      for (eventCode = 0; eventCode < KEY_MAX; eventCode++) 
	if (test_bit(eventCode, bitmask[eventType])) {
	  post("    Event code %d (%s)", eventCode, names[eventType] ? (names[eventType][eventCode] ? names[eventType][eventCode] : "?") : "?");

	  if ( eventType == EV_KEY ) 
	    x->x_buttons++;
	  else if  ( eventType == EV_REL ) 
	    x->x_axes++;
	}
    }        
  }
    
  post ("\nUsing %d axes and %d buttons.", x->x_axes, x->x_buttons);
  post ("WARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING");
  post ("This object is under development!  The interface could change at anytime!");
  post ("As I write cross-platform versions, the interface might have to change.");
  post ("WARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING");
    
  return 1;
}



static int rawmouse_read(t_rawmouse *x,int fd)
{
  int readBytes;
  int axis_num = 0;
  t_float button_num = 0;
    
  if (x->x_fd < 0) return 0;
  if (x->read_ok) {
    readBytes = read(x->x_fd, &(x->x_input_event), sizeof(struct input_event));
    DEBUG(post("reading %d",readBytes);)
    if ( readBytes < 0 ) {
      post("rawmouse: read failed");
      x->read_ok = 0;
      return 0;
    }
  }
  if ( x->x_input_event.type == EV_KEY ) {
    /* key/button event type */
    switch ( x->x_input_event.code ) {
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
    outlet_float (x->x_button_val_out, x->x_input_event.value);
    outlet_float (x->x_button_num_out, button_num);
  }
  else if  ( x->x_input_event.type == EV_REL ) {
    /* Relative Axes Event Type */
    switch ( x->x_input_event.code ) {
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
    outlet_float (x->x_axis_out[axis_num], (int)x->x_input_event.value);	
  }

  return 1;    
}



/* Actions */

static void rawmouse_bang(t_rawmouse* x)
{
    DEBUG(post("rawmouse_bang");)
   
}

static void rawmouse_float(t_rawmouse* x)
{
    DEBUG(post("rawmouse_float");)
   
}

// DONE
void rawmouse_start(t_rawmouse* x)
{
  DEBUG(post("rawmouse_start");)

    if (x->x_fd >= 0 && !x->started) {
       sys_addpollfn(x->x_fd, (t_fdpollfn)rawmouse_read, x);
       post("rawmouse: start");
       x->started = 1;
    }
}


// DONE
void rawmouse_stop(t_rawmouse* x)
{
  DEBUG(post("rawmouse_stop");)

    if (x->x_fd >= 0 && x->started) { 
        sys_rmpollfn(x->x_fd);
        post("rawmouse: stop");
        x->started = 0;
    }
}

/* Misc setup functions */


static void rawmouse_free(t_rawmouse* x)
{
  DEBUG(post("rawmouse_free");)
    
    if (x->x_fd < 0) return;
  
  rawmouse_stop(x);
  
  close (x->x_fd);
}

static void *rawmouse_new(t_symbol *s)
{
  int i;
  t_rawmouse *x = (t_rawmouse *)pd_new(rawmouse_class);

  DEBUG(post("rawmouse_new");)
  
  /* init vars */
  x->x_fd = -1;
  x->read_ok = 1;
  x->started = 0;
  
  /* create outlets for each axis */
  for (i = 0; i < RAWMOUSE_AXES; i++) 
    x->x_axis_out[i] = outlet_new(&x->x_obj, &s_float);
  
  /* create outlets for buttons */
  x->x_button_num_out = outlet_new(&x->x_obj, &s_float);
  x->x_button_val_out = outlet_new(&x->x_obj, &s_float);
  
  if (s != &s_)
    x->x_devname = s;
  
  /* Open the device and save settings */
  
  if (!rawmouse_open(x,s)) return x;
  
  return (x);
}


void rawmouse_setup(void)
{
  DEBUG(post("rawmouse_setup");)
  rawmouse_class = class_new(gensym("rawmouse"), 
			     (t_newmethod)rawmouse_new, 
			     (t_method)rawmouse_free,
			     sizeof(t_rawmouse),0,A_DEFSYM,0);

  /* add inlet datatype methods */
  class_addfloat(rawmouse_class,(t_method) rawmouse_float);
  class_addbang(rawmouse_class,(t_method) rawmouse_bang);

  /* add inlet message methods */
  class_addmethod(rawmouse_class, (t_method) rawmouse_open,gensym("open"),A_DEFSYM);
  class_addmethod(rawmouse_class,(t_method) rawmouse_close,gensym("close"),0);
  class_addmethod(rawmouse_class,(t_method) rawmouse_start,gensym("start"),0);
  class_addmethod(rawmouse_class,(t_method) rawmouse_stop,gensym("stop"),0);
  
}

