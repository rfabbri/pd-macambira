/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* MacOS X object to use HIDs (Human Interface Devices                          */
/* Written by Hans-Christoph Steiner <hans@at.or.at>                            */
/*                                                                              */
/* Copyright (c) 2004 Hans-Christoph Steiner                                    */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* See file LICENSE for further informations on licensing terms.                */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* ---------------------------------------------------------------------------- */

#include "hid.h"
#include "../linuxhid.h"

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

void hid_stop(t_hid* x) 
{
  DEBUG(post("hid_stop"););
  
  if (x->x_fd >= 0 && x->x_started) 
  { 
	  clock_unset(x->x_clock);
	  post("hid: polling stopped");
	  x->x_started = 0;
  }
}

static t_int hid_close(t_hid *x) 
{
	DEBUG(post("hid_close"););

/* just to be safe, stop it first */
	hid_stop(x);

   if (x->x_fd <0) return 0;
   close (x->x_fd);
	post ("[hid] closed %s",x->x_devname->s_name);
	
   return 1;
}

static t_int hid_open(t_hid *x, t_symbol *s) 
{
	t_int eventType, eventCode;
	char *eventTypeName = "";
#ifdef __linux__
	struct input_event hid_input_event;
#endif
	/* counts for various event types */
	t_int synCount,keyCount,relCount,absCount,mscCount,ledCount,sndCount,repCount,ffCount,pwrCount,ff_statusCount;
#ifdef __gnu_linux__
  unsigned long bitmask[EV_MAX][NBITS(KEY_MAX)];
#endif
  char devicename[256] = "Unknown";
  DEBUG(post("hid_open");)

  hid_close(x);

  /* set obj device name to parameter 
   * otherwise set to default
   */  
  if (s != &s_)
    x->x_devname = s;
  
#ifdef __gnu_linux__
  /* open device */
  if (x->x_devname) 
  {
	  /* open the device read-only, non-exclusive */
	  x->x_fd = open (x->x_devname->s_name, O_RDONLY | O_NONBLOCK);
	  /* test if device open */
	  if (x->x_fd < 0 ) 
	  { 
		  error("[hid] open %s failed",x->x_devname->s_name);
		  x->x_fd = -1;
		  return 0;
	  }
  } else return 1;
  
  /* read input_events from the HID_DEVICE stream 
   * It seems that is just there to flush the input event queue
   */
  while (read (x->x_fd, &(hid_input_event), sizeof(struct input_event)) > -1);
  
  /* get name of device */
  ioctl(x->x_fd, EVIOCGNAME(sizeof(devicename)), devicename);
  post ("\nConfiguring %s on %s",devicename,x->x_devname->s_name);

  /* get bitmask representing supported events (axes, keys, etc.) */
  memset(bitmask, 0, sizeof(bitmask));
  ioctl(x->x_fd, EVIOCGBIT(0, EV_MAX), bitmask[0]);
  post("\nSupported events:");
    
/* init all count vars */
  synCount=keyCount=relCount=absCount=mscCount=ledCount=0;
  sndCount=repCount=ffCount=pwrCount=ff_statusCount=0;
    
  /* cycle through all possible event types */
  for (eventType = 0; eventType < EV_MAX; eventType++) 
  {
	  if (test_bit(eventType, bitmask[0])) 
	  {
		  /* make pretty names for event types */
		  switch(eventType)
		  {
			  case EV_SYN: eventTypeName = "Synchronization"; break;
			  case EV_KEY: eventTypeName = "Keys/Buttons"; break;
			  case EV_REL: eventTypeName = "Relative Axes"; break;
			  case EV_ABS: eventTypeName = "Absolute Axes"; break;
			  case EV_MSC: eventTypeName = "Miscellaneous"; break;
			  case EV_LED: eventTypeName = "LEDs"; break;
			  case EV_SND: eventTypeName = "System Sounds"; break;
			  case EV_REP: eventTypeName = "Autorepeat Values"; break;
			  case EV_FF:  eventTypeName = "Force Feedback"; break;
			  case EV_PWR: eventTypeName = "Power"; break;
			  case EV_FF_STATUS: eventTypeName = "Force Feedback Status"; break;
		  }
		  post("  %s (%s/type %d) ", eventTypeName, ev[eventType] ? ev[eventType] : "?", eventType);
		 
		  /* get bitmask representing supported button types */
		  ioctl(x->x_fd, EVIOCGBIT(eventType, KEY_MAX), bitmask[eventType]);
		 
		  /* cycle through all possible event codes (axes, keys, etc.) 
			* testing to see which are supported  
			*/
		  for (eventCode = 0; eventCode < KEY_MAX; eventCode++) 
		  {
			  if (test_bit(eventCode, bitmask[eventType])) 
			  {
				  post("    %s (%d)", event_names[eventType] ? (event_names[eventType][eventCode] ? event_names[eventType][eventCode] : "?") : "?", eventCode);
/* 	  post("    Event code %d (%s)", eventCode, names[eventType] ? (names[eventType][eventCode] ? names[eventType][eventCode] : "?") : "?"); */
				
				  switch(eventType) {
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

  post("\nWARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING");
  post("This object is under development!  The interface could change at anytime!");
  post("As I write cross-platform versions, the interface might have to change.");
  post("WARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING");
  post("================================= [hid] =================================\n");
#endif /* #ifdef __gnu_linux__ */
  
  return 1;
}

static t_int hid_read(t_hid *x,int fd) 
{
	t_atom event_data[5];
	char *eventType;
	char *eventCode;
#ifdef __linux__
	struct input_event hid_input_event;

	if (x->x_fd < 0) return 0;

	while (read (x->x_fd, &(hid_input_event), sizeof(struct input_event)) > -1) 
	{
		/* build event_data list from event data */
		SETSYMBOL(event_data, gensym(ev[hid_input_event.type]));
		SETSYMBOL(event_data + 1, gensym(event_names[hid_input_event.type][hid_input_event.code]));
		SETFLOAT(event_data + 2, (t_float)hid_input_event.value);
		SETFLOAT(event_data + 3, (t_float)(hid_input_event.time).tv_sec);
		outlet_anything(x->x_obj.te_outlet,atom_gensym(event_data),3,event_data+1); 
	}
#endif /* #ifdef__gnu_linux__ */
#ifdef IGNOREIGNOREIGNORE
	pRecDevice pCurrentHIDDevice = GetSetCurrentDevice (gWindow);
	pRecElement pCurrentHIDElement = GetSetCurrenstElement (gWindow);

	// if we have a good device and element which is not a collecion
	if (pCurrentHIDDevice && pCurrentHIDElement && (pCurrentHIDElement->type != kIOHIDElementTypeCollection))
	{
		SInt32 value = HIDGetElementValue (pCurrentHIDDevice, pCurrentHIDElement);
		SInt32 valueCal = HIDCalibrateValue (value, pCurrentHIDElement);
		SInt32 valueScale = HIDScaleValue (valueCal, pCurrentHIDElement);
	 }
#endif  /* #ifdef __APPLE__ */

	if (x->x_started) 
	{
		clock_delay(x->x_clock, x->x_delay);
	}

	return 1;    
}

/* Actions */
void hid_delay(t_hid* x, t_float f)  
{
	DEBUG(post("hid_DELAY %f",f);)
		
/*	if the user sets the delay less than zero, reset to default */
	if ( f > 0 ) 
	{	
		x->x_delay = (t_int)f;
	} 
	else 
	{
		x->x_delay = DEFAULT_DELAY;
	}
}

void hid_start(t_hid* x) 
{
	DEBUG(post("hid_start"););
  
   if (x->x_fd >= 0 && !x->x_started) 
	{
		clock_delay(x->x_clock, DEFAULT_DELAY);
		post("hid: polling started");
		x->x_started = 1;
	} 
	else 
	{
		error("You need to set a input device (i.e /dev/input/event0)");
	}
}

static void hid_float(t_hid* x, t_floatarg f) 
{
	DEBUG(post("hid_float"););
   
	if(f == 1) 
		hid_start(x);
	else if(f == 0) 
		hid_stop(x);
}

/* setup functions */
static void hid_free(t_hid* x) 
{
	DEBUG(post("hid_free"););
	
	if (x->x_fd < 0) return;
	
	hid_stop(x);
	clock_free(x->x_clock);
	close (x->x_fd);
}

static void *hid_new(t_symbol *s) 
{
  t_int i;
  t_hid *x = (t_hid *)pd_new(hid_class);

  DEBUG(post("hid_new"););

  post("================================= [hid] =================================");
  post("[hid] %s, written by Hans-Christoph Steiner <hans@eds.org>",version);  
#ifndef __linux__
	error("    !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!");
	error("     This is a dummy, since this object only works with a Linux kernel!");
	error("    !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!");
#endif  /* __linux__ */

  /* init vars */
  x->x_fd = -1;
  x->x_read_ok = 1;
  x->x_started = 0;
  x->x_delay = DEFAULT_DELAY;
  x->x_devname = gensym("/dev/input/event0");

  x->x_clock = clock_new(x, (t_method)hid_read);

  /* create anything outlet used for HID data */ 
  outlet_new(&x->x_obj, 0);
  
  /* set to the value from the object argument, if that exists */
  if (s != &s_)
	  x->x_devname = s;
  
  /* Open the device and save settings */
  
  if (!hid_open(x,s)) return x;
  
  return (x);
}

void hid_setup(void) 
{
	DEBUG(post("hid_setup"););
	hid_class = class_new(gensym("hid"), 
								 (t_newmethod)hid_new, 
								 (t_method)hid_free,
								 sizeof(t_hid),
								 CLASS_DEFAULT,
								 A_DEFSYM,0);
	
	/* add inlet datatype methods */
	class_addfloat(hid_class,(t_method) hid_float);
	class_addbang(hid_class,(t_method) hid_read);
	
	/* add inlet message methods */
	class_addmethod(hid_class,(t_method) hid_delay,gensym("delay"),A_DEFFLOAT,0);
	class_addmethod(hid_class,(t_method) hid_open,gensym("open"),A_DEFSYM,0);
	class_addmethod(hid_class,(t_method) hid_close,gensym("close"),0);
	class_addmethod(hid_class,(t_method) hid_start,gensym("start"),0);
	class_addmethod(hid_class,(t_method) hid_start,gensym("poll"),0);
	class_addmethod(hid_class,(t_method) hid_stop,gensym("stop"),0);
	class_addmethod(hid_class,(t_method) hid_stop,gensym("nopoll"),0);
}

