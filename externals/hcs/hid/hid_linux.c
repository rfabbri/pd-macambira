#ifdef __linux__


#include <linux/input.h>
#include <sys/ioctl.h>

#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include "hid.h"

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 * from evtest.c from the ff-utils package
 */

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)


/*
 * from an email from Vojtech:
 *
 * The application reading the device is supposed to queue all events up to 
 * the SYN_REPORT event, and then process them, so that a mouse pointer
 * will move diagonally instead of following the sides of a rectangle, 
 * which would be very annoying. 
 */

t_int hid_output_events(t_hid *x)
{
	DEBUG(post("hid_output_events"););

/*	for debugging, counts how many events are processed each time hid_read() is called */
	t_int i;
	DEBUG(t_int event_counter = 0;);
	t_int read_bytes;
	t_atom event_data[4];

/* this will go into the generic read function declared in hid.h and
 * implemented in hid_linux.c 
 */
	struct input_event hid_input_event;

	if (x->x_fd < 0) return 0;

	while (read (x->x_fd, &(hid_input_event), sizeof(struct input_event)) > -1)
	{
		/* build event_data list from event data */
		/* type */
		SETSYMBOL(event_data, gensym(ev[hid_input_event.type]));
		/* code */
		SETSYMBOL(event_data + 1, gensym(event_names[hid_input_event.type][hid_input_event.code]));
		/* value */
		SETFLOAT(event_data + 2, (t_float)hid_input_event.value);
		/* time */
		SETFLOAT(event_data + 3, (t_float)(hid_input_event.time).tv_sec);
		outlet_anything(x->x_obj.te_outlet,atom_gensym(event_data),3,event_data+1); 
		DEBUG(++event_counter;);
	}
	DEBUG(
	if (event_counter > 0)
	    post("output %d events",event_counter);
	);
	
	return (0);
}


t_int hid_open_device(t_hid *x, t_int device_number)
{
	DEBUG(post("hid_open_device"););

	char arg[20];
	struct input_event hid_input_event;
	unsigned long bitmask[EV_MAX][NBITS(KEY_MAX)];
	char devicename[256] = "Unknown";
	t_int event_type, event_code;
	char *event_typeName = "";
	/* counts for various event types */
	t_int synCount,keyCount,relCount,absCount,mscCount,ledCount,sndCount,repCount,ffCount,pwrCount,ff_statusCount;

	x->x_fd = -1;
  
	x->x_device_number = device_number;
	sprintf(arg,"/dev/input/event%d",x->x_device_number);
	x->x_devname = gensym(arg);

  if (x->x_devname) 
  {
	  /* open the device read-only, non-exclusive */
	  x->x_fd = open(x->x_devname->s_name, O_RDONLY | O_NONBLOCK);
	  /* test if device open */
	  if (x->x_fd < 0 ) 
	  { 
		  error("[hid] open %s failed",x->x_devname->s_name);
		  x->x_fd = -1;
		  return 1;
	  }
  } 
  
  /* read input_events from the HID_DEVICE stream 
   * It seems that is just there to flush the input event queue
   */
  while (read (x->x_fd, &(hid_input_event), sizeof(struct input_event)) > -1);
  
  /* get name of device */
  ioctl(x->x_fd, EVIOCGNAME(sizeof(devicename)), devicename);
  post ("\nConfiguring device %d as %s (%s)",
		  x->x_device_number,devicename,x->x_devname->s_name);

  /* get bitmask representing supported events (axes, keys, etc.) */
  memset(bitmask, 0, sizeof(bitmask));
  ioctl(x->x_fd, EVIOCGBIT(0, EV_MAX), bitmask[0]);
  post("\nSupported events:");
    
/* init all count vars */
  synCount = keyCount = relCount = absCount = mscCount = ledCount = 0;
  sndCount = repCount = ffCount = pwrCount = ff_statusCount = 0;
    
  /* cycle through all possible event types */
  for (event_type = 0; event_type < EV_MAX; event_type++) 
  {
	  if (test_bit(event_type, bitmask[0])) 
	  {
		  /* make pretty names for event types */
		  switch(event_type)
		  {
			  case EV_SYN: event_typeName = "Synchronization"; break;
			  case EV_KEY: event_typeName = "Keys/Buttons"; break;
			  case EV_REL: event_typeName = "Relative Axes"; break;
			  case EV_ABS: event_typeName = "Absolute Axes"; break;
			  case EV_MSC: event_typeName = "Miscellaneous"; break;
			  case EV_LED: event_typeName = "LEDs"; break;
			  case EV_SND: event_typeName = "System Sounds"; break;
			  case EV_REP: event_typeName = "Autorepeat Values"; break;
			  case EV_FF:  event_typeName = "Force Feedback"; break;
			  case EV_PWR: event_typeName = "Power"; break;
			  case EV_FF_STATUS: event_typeName = "Force Feedback Status"; break;
		  }
		  post("  %s (%s/type %d) ", event_typeName, ev[event_type] ? ev[event_type] : "?", event_type);
		 
		  /* get bitmask representing supported button types */
		  ioctl(x->x_fd, EVIOCGBIT(event_type, KEY_MAX), bitmask[event_type]);
		 
		  /* cycle through all possible event codes (axes, keys, etc.) 
			* testing to see which are supported  
			*/
		  for (event_code = 0; event_code < KEY_MAX; event_code++) 
		  {
			  if (test_bit(event_code, bitmask[event_type])) 
			  {
				  post("    %s (%d)", event_names[event_type] ? (event_names[event_type][event_code] ? event_names[event_type][event_code] : "?") : "?", event_code);
/* 	  post("    Event code %d (%s)", event_code, names[event_type] ? (names[event_type][event_code] ? names[event_type][event_code] : "?") : "?"); */
				
				  switch(event_type) {
/* 
 * the API changed at some point...  EV_SYN seems to be the new name
 * from "Reset" events to "Syncronization" events
 */
#ifdef EV_RST
					  case EV_RST: synCount++; break;
#else 
					  case EV_SYN: synCount++; break;
#endif
					  case EV_KEY: keyCount++; break;
					  case EV_REL: relCount++; break;
					  case EV_ABS: absCount++; break;
					  case EV_MSC: mscCount++; break;
					  case EV_LED: ledCount++; break;
					  case EV_SND: sndCount++; break;
					  case EV_REP: repCount++; break;
					  case EV_FF:  ffCount++;  break;
					  case EV_PWR: pwrCount++; break;
					  case EV_FF_STATUS: ff_statusCount++; break;
				  }
			  }
		  }
	  }        
  }
    
  post("\nDetected:");
  if (synCount > 0) post ("  %d Sync types",synCount);
  if (keyCount > 0) post ("  %d Key/Button types",keyCount);
  if (relCount > 0) post ("  %d Relative Axis types",relCount);
  if (absCount > 0) post ("  %d Absolute Axis types",absCount);
  if (mscCount > 0) post ("  %d Misc types",mscCount);
  if (ledCount > 0) post ("  %d LED types",ledCount);
  if (sndCount > 0) post ("  %d System Sound types",sndCount);
  if (repCount > 0) post ("  %d Key Repeat types",repCount);
  if (ffCount > 0) post ("  %d Force Feedback types",ffCount);
  if (pwrCount > 0) post ("  %d Power types",pwrCount);
  if (ff_statusCount > 0) post ("  %d Force Feedback types",ff_statusCount);

  return (0);
}

/*
 * Under GNU/Linux, the device is a filehandle
 */
t_int hid_close_device(t_hid *x)
{
	DEBUG(post("hid_close_device"););
   if (x->x_fd <0) 
		return 0;
	else
		return (close(x->x_fd));
}

t_int hid_devicelist_refresh(t_hid *x)
{
	DEBUG(post("hid_devicelist_refresh"););
	/* the device list should be refreshed here */
/*
 *	since in GNU/Linux the device list is the input event devices 
 *	(/dev/input/event?), nothing needs to be done as of yet to refresh 
 * the device list.  Once the device name can be other things in addition
 * the current t_float, then this will probably need to be changed.
 */
	return (0);
}



#endif  /* #ifdef __linux__ */

