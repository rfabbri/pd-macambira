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

#define RAWEVENT_DEVICE   "/dev/input/event0"
#define RAWEVENT_OUTLETS  4

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
static t_class *rawevent_class;

typedef struct _rawevent {
  t_object            x_obj;
  t_int               x_fd;
  t_symbol*           x_devname;
  int                 read_ok;
  int                 started;
  struct input_event  x_input_event; 
  t_outlet            *x_input_event_time_outlet;
  t_outlet            *x_input_event_type_outlet;
  t_outlet            *x_input_event_code_outlet;
  t_outlet            *x_input_event_value_outlet;
}t_rawevent;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

//DONE
static int rawevent_close(t_rawevent *x)
{
    DEBUG(post("rawevent_close");)

     if (x->x_fd <0) return 0;

     close (x->x_fd);

     return 1;
}

//DONE
static int rawevent_open(t_rawevent *x,t_symbol* s)
{
  int eventType, eventCode, buttons, rel_axes, abs_axes, ff;
  unsigned long bitmask[EV_MAX][NBITS(KEY_MAX)];
  char devicename[256] = "Unknown";
  DEBUG(post("rawevent_open");)

  rawevent_close(x);

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
    /* open the rawevent device read-only, non-exclusive */
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
  
  /* read input_events from the RAWEVENT_DEVICE stream 
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
    
  rel_axes = 0;
  abs_axes = 0;
  buttons = 0;
  ff = 0;
    
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

	  switch(eventType) {
	  case EV_RST:
	    break;
	  case EV_KEY:
	    buttons++;
	    break;
	  case EV_REL:
	    rel_axes++;
	    break;
	  case EV_ABS:
	    abs_axes++;
	    break;
	  case EV_MSC:
	    break;
	  case EV_LED:
	    break;
	  case EV_SND:
	    break;
	  case EV_REP:
	    break;
	  case EV_FF:
	    ff++;
	    break;
	  }
	}
    }        
  }
    
  post ("\nUsing %d relative axes, %d absolute axes, and %d buttons.", rel_axes, abs_axes, buttons);
  if (ff > 0) post ("Detected %d force feedback types",ff);
  post ("");
  post ("WARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING");
  post ("This object is under development!  The interface could change at anytime!");
  post ("As I write cross-platform versions, the interface might have to change.");
  post ("WARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING");
    
  return 1;
}



static int rawevent_read(t_rawevent *x,int fd)
{
  int readBytes;
  int axis_num = 0;
  t_float button_num = 0;
    
  if (x->x_fd < 0) return 0;
  if (x->read_ok) {
    readBytes = read(x->x_fd, &(x->x_input_event), sizeof(struct input_event));
    DEBUG(post("reading %d",readBytes);)
    if ( readBytes < 0 ) {
      post("rawevent: read failed");
      x->read_ok = 0;
      return 0;
    }
  }
  /*   outlet_float (x->x_input_event_time_outlet, x->x_input_event.time); */
  outlet_float (x->x_input_event_type_outlet, x->x_input_event.type);
  outlet_float (x->x_input_event_code_outlet, x->x_input_event.code);
  outlet_float (x->x_input_event_value_outlet, (int)x->x_input_event.value);
  
  return 1;    
}



/* Actions */

static void rawevent_bang(t_rawevent* x)
{
    DEBUG(post("rawevent_bang");)
   
}

static void rawevent_float(t_rawevent* x)
{
    DEBUG(post("rawevent_float");)
   
}

// DONE
void rawevent_start(t_rawevent* x)
{
  DEBUG(post("rawevent_start");)

    if (x->x_fd >= 0 && !x->started) {
       sys_addpollfn(x->x_fd, (t_fdpollfn)rawevent_read, x);
       post("rawevent: start");
       x->started = 1;
    }
}


// DONE
void rawevent_stop(t_rawevent* x)
{
  DEBUG(post("rawevent_stop");)

    if (x->x_fd >= 0 && x->started) { 
        sys_rmpollfn(x->x_fd);
        post("rawevent: stop");
        x->started = 0;
    }
}

/* Misc setup functions */


static void rawevent_free(t_rawevent* x)
{
  DEBUG(post("rawevent_free");)
    
    if (x->x_fd < 0) return;
  
  rawevent_stop(x);
  
  close (x->x_fd);
}

static void *rawevent_new(t_symbol *s)
{
  int i;
  t_rawevent *x = (t_rawevent *)pd_new(rawevent_class);

  DEBUG(post("rawevent_new");)
  
  /* init vars */
  x->x_fd = -1;
  x->read_ok = 1;
  x->started = 0;
  
  /* create outlets for each axis */
  x->x_input_event_time_outlet = outlet_new(&x->x_obj, &s_float);
  x->x_input_event_type_outlet = outlet_new(&x->x_obj, &s_float);
  x->x_input_event_code_outlet = outlet_new(&x->x_obj, &s_float);
  x->x_input_event_value_outlet = outlet_new(&x->x_obj, &s_float);
  
  if (s != &s_)
    x->x_devname = s;
  
  /* Open the device and save settings */
  
  if (!rawevent_open(x,s)) return x;
  
  return (x);
}


void rawevent_setup(void)
{
  DEBUG(post("rawevent_setup");)
  rawevent_class = class_new(gensym("rawevent"), 
			     (t_newmethod)rawevent_new, 
			     (t_method)rawevent_free,
			     sizeof(t_rawevent),0,A_DEFSYM,0);

  /* add inlet datatype methods */
  class_addfloat(rawevent_class,(t_method) rawevent_float);
  class_addbang(rawevent_class,(t_method) rawevent_bang);

  /* add inlet message methods */
  class_addmethod(rawevent_class, (t_method) rawevent_open,gensym("open"),A_DEFSYM);
  class_addmethod(rawevent_class,(t_method) rawevent_close,gensym("close"),0);
  class_addmethod(rawevent_class,(t_method) rawevent_start,gensym("start"),0);
  class_addmethod(rawevent_class,(t_method) rawevent_stop,gensym("stop"),0);
  
}

