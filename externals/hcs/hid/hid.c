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

/* 
 * Various Max Refresh Rates: 
      ADB = 15.5 ms
		USB mice on Windows = 8-10 ms
		USB mice on GNU/Linux = 1-10 ms
		USB wireless mice = 12-20 ms
		PS/2 mouse = 5-25 ms
		P5 Globe = 22 ms
 */

#define DEFAULT_DELAY 5

/*------------------------------------------------------------------------------
 * FUNCTION PROTOTYPES
 */

void hid_start(t_hid *x, t_float f);
void hid_stop(t_hid *x);
t_int hid_open(t_hid *x, t_float f);
t_int hid_close(t_hid *x);
t_int hid_read(t_hid *x,int fd);
static void hid_float(t_hid* x, t_floatarg f);


/*------------------------------------------------------------------------------
 * SUPPORT FUNCTIONS
 */

void hid_output_event(t_hid *x, 
							  char *type, char *code, t_float value, t_float timestamp)
{
	t_atom event_data[4];
	
	SETSYMBOL(event_data, gensym(type));	   /* type */
	SETSYMBOL(event_data + 1, gensym(code));	/* code */
	SETFLOAT(event_data + 2, value);	         /* value */
	SETFLOAT(event_data + 3, timestamp); 	   /* timestamp */

	outlet_anything(x->x_obj.te_outlet,atom_gensym(event_data),3,event_data+1);
}

void hid_set_from_float(t_hid *x, t_floatarg f)
{
/* values greater than 1 set the polling delay time */
/* 1 and 0 for start/stop so you can use a [tgl] */
	if (f > 1)
	{
		x->x_delay = (t_int)f;
		hid_start(x,f);
	}
	else if (f == 1) 
	{
		if (! x->x_started)
		hid_start(x,f);
	}
	else if (f == 0) 		
	{
		hid_stop(x);
	}
}

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

/* stop polling the device */
void hid_stop(t_hid* x) 
{
  DEBUG(post("hid_stop"););
  
  if (x->x_started) 
  { 
	  clock_unset(x->x_clock);
	  post("[hid] polling stopped");
	  x->x_started = 0;
  }
}

/* close the device */
t_int hid_close(t_hid *x) 
{
	DEBUG(post("hid_close"););

/* just to be safe, stop it first */
	hid_stop(x);

	if(! hid_close_device(x)) 
	{
		post("[hid] closed device number %d",x->x_device_number);
		x->x_device_open = 0;
		return (0);
	}

	return (1);
}


/* closed same device          open */
/* open same device            no action */
/* closed different device     open */
/* open different device       close open */

t_int hid_open(t_hid *x, t_float f) 
{
	DEBUG(post("hid_open"););

/* store running state to be restored after the device has been opened */
		t_int started = x->x_started;

		if ( (f != x->x_device_number) && (x->x_device_open) ) hid_close(x);

		/* set obj device name to parameter otherwise set to default   */  
		if (f > 0)
			x->x_device_number = f;
		else
			x->x_device_number = 0;
		
		if (! x->x_device_open) 
			if (hid_open_device(x,x->x_device_number))
			{
				error("[hid] can not open device %d",x->x_device_number);
				post("\\=========================== [hid] ===========================/\n");
				return (1);
			}
			else
			{
				x->x_device_open = 1;
			}

/* restore the polling state so that when I [tgl] is used to start/stop [hid],
 * the [tgl]'s state will continue to accurately reflect [hid]'s state  */
		hid_set_from_float(x,started);

		
	post("\\=========================== [hid] ===========================/\n");
	return (0);
}


t_int hid_read(t_hid *x,int fd) 
{
//	DEBUG(post("hid_read"););

	hid_get_events(x);
	
	if (x->x_started) 
	{
		clock_delay(x->x_clock, x->x_delay);
	}
	
	// TODO: why is this 1? 
	return 1; 
}

void hid_start(t_hid* x, t_float f) 
{
	DEBUG(post("hid_start"););
  
/*	if the user sets the delay less than one, ignore */
	if ( f >= 1 ) 	
		x->x_delay = (t_int)f;
	
   if (!x->x_started) 
	{
		clock_delay(x->x_clock, x->x_delay);
		post("[hid]: polling started");
		x->x_started = 1;
	} 
}

static void hid_float(t_hid* x, t_floatarg f) 
{
	DEBUG(post("hid_float"););

	hid_set_from_float(x,f);
}

/* setup functions */
static void hid_free(t_hid* x) 
{
	DEBUG(post("hid_free"););
		
	hid_close(x);
	
	hid_platform_specific_free(x);
	
	clock_free(x->x_clock);
}

/* create a new instance of this class */
static void *hid_new(t_float f) 
{
  t_hid *x = (t_hid *)pd_new(hid_class);

  DEBUG(post("hid_new"););

  post("/=========================== [hid] ===========================\\");
  post("[hid] %d.%d, written by Hans-Christoph Steiner <hans@eds.org>",
		 HID_MAJOR_VERSION, HID_MINOR_VERSION);  
#if !defined(__linux__) && !defined(__APPLE__)
  error("    !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!");
  error("     This is a dummy, since this object only works GNU/Linux and MacOS X!");
  error("    !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!");
#endif

  /* init vars */
  x->x_device_open = 0;
  x->x_started = 0;
  x->x_delay = DEFAULT_DELAY;

  x->x_clock = clock_new(x, (t_method)hid_read);

  /* create anything outlet used for HID data */ 
  outlet_new(&x->x_obj, 0);

  /* find and report the list of devices */
  hid_build_device_list(x);
  
  /* Open the device and save settings.  If there is an error, return the object
   * anyway, so that the inlets and outlets are created, thus not breaking the
   * patch.   */
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
	class_addmethod(hid_class,(t_method) hid_build_device_list,gensym("refresh"),0);
	class_addmethod(hid_class,(t_method) hid_open,gensym("open"),A_DEFFLOAT,0);
	class_addmethod(hid_class,(t_method) hid_close,gensym("close"),0);
	class_addmethod(hid_class,(t_method) hid_start,gensym("start"),A_DEFFLOAT,0);
	class_addmethod(hid_class,(t_method) hid_start,gensym("poll"),A_DEFFLOAT,0);
	class_addmethod(hid_class,(t_method) hid_stop,gensym("stop"),0);
	class_addmethod(hid_class,(t_method) hid_stop,gensym("nopoll"),0);
}

