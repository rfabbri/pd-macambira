/* ---------------------------------------------------------------------------- */
/*                                                                              */
/* MacOS X object to use HIDs (Human Interface Devices)                         */
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
/* ---------------------------------------------------------------------------- */

#include "hid.h"


/*------------------------------------------------------------------------------
 * LOCAL DEFINES
 */

//#define DEBUG(x)
#define DEBUG(x) x 

#define DEFAULT_DELAY 500

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

void hid_stop(t_hid *x) 
{
  DEBUG(post("hid_stop"););
  
  if (x->x_started) 
  { 
	  clock_unset(x->x_clock);
	  post("[hid] polling stopped");
	  x->x_started = 0;
  }
  
  hid_devicelist_refresh(x);
}


t_int hid_close(t_hid *x) 
{
	DEBUG(post("hid_close"););

/* just to be safe, stop it first */
	hid_stop(x);

	post("[hid] closed device number %d",x->x_device_number);

	return (hid_close_device(x));
}


t_int hid_open(t_hid *x, t_float f) 
{
	DEBUG(post("hid_open"););
	
	hid_close(x);

  /* set obj device name to parameter 
   * otherwise set to default
   */  
  if (f > 0)
	  x->x_device_number = f;
  else
	  x->x_device_number = 1;
  
  if (hid_open_device(x,x->x_device_number)) 
  {
	  error("[hid] can not open device %d",x->x_device_number);
	  return (1);
  }

  post("\nWARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING");
  post("This object is under development!  The interface could change at anytime!");
  post("As I write cross-platform versions, the interface might have to change.");
  post("WARNING * WARNING * WARNING * WARNING * WARNING * WARNING * WARNING");
  post("================================= [hid] =================================\n");
  return (0);
}


t_int hid_read(t_hid *x,int fd) 
{
	hid_output_events(x);
	
	if (x->x_started) 
	{
		clock_delay(x->x_clock, x->x_delay);
	}
	
	return 1;  /* why is this 1? */
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
  
   if (!x->x_started) 
	{
		clock_delay(x->x_clock, DEFAULT_DELAY);
		post("hid: polling started");
		x->x_started = 1;
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
		
	hid_close(x);

	clock_free(x->x_clock);
}

static void *hid_new(t_float f) 
{
  t_hid *x = (t_hid *)pd_new(hid_class);

  DEBUG(post("hid_new"););

  post("================================= [hid] =================================");
  post("[hid] %s, written by Hans-Christoph Steiner <hans@eds.org>",version);  
#if !defined(__linux__) && !defined(__APPLE__)
  error("    !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!");
  error("     This is a dummy, since this object only works GNU/Linux and MacOS X!");
  error("    !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!");
#endif

  /* init vars */
  x->x_read_ok = 1;
  x->x_started = 0;
  x->x_delay = DEFAULT_DELAY;

  x->x_clock = clock_new(x, (t_method)hid_read);

  /* create anything outlet used for HID data */ 
  outlet_new(&x->x_obj, 0);
  
  /* Open the device and save settings */
  if (hid_open(x,f))
	  error("[hid] device %d did not open",(t_int)f);

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
								 A_DEFFLOAT,0);
	
	/* add inlet datatype methods */
	class_addfloat(hid_class,(t_method) hid_float);
	class_addbang(hid_class,(t_method) hid_read);
	
	/* add inlet message methods */
	class_addmethod(hid_class,(t_method) hid_delay,gensym("delay"),A_DEFFLOAT,0);
	class_addmethod(hid_class,(t_method) hid_open,gensym("open"),A_DEFFLOAT,0);
	class_addmethod(hid_class,(t_method) hid_close,gensym("close"),0);
	class_addmethod(hid_class,(t_method) hid_start,gensym("start"),0);
	class_addmethod(hid_class,(t_method) hid_start,gensym("poll"),0);
	class_addmethod(hid_class,(t_method) hid_stop,gensym("stop"),0);
	class_addmethod(hid_class,(t_method) hid_stop,gensym("nopoll"),0);
}

