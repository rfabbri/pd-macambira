/* --------------------------------------------------------------------------*/
/*                                                                           */
/* MacOS X object to use HIDs (Human Interface Devices)                      */
/* Written by Hans-Christoph Steiner <hans@at.or.at>                         */
/*                                                                           */
/* Copyright (c) 2004 Hans-Christoph Steiner                                 */
/*                                                                           */
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU General Public License               */
/* as published by the Free Software Foundation; either version 2            */
/* of the License, or (at your option) any later version.                    */
/*                                                                           */
/* See file LICENSE for further informations on licensing terms.             */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software Foundation,   */
/* Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           */
/*                                                                           */
/* --------------------------------------------------------------------------*/

#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "hid.h"

/*------------------------------------------------------------------------------
 * LOCAL DEFINES
 */

#define DEBUG(x)
//#define DEBUG(x) x 

unsigned short global_debug_level = 0;

static t_class *hid_class;

/*------------------------------------------------------------------------------
 * FUNCTION PROTOTYPES
 */

static void hid_poll(t_hid *x, t_float f);
static t_int hid_open(t_hid *x, t_symbol *s, t_int argc, t_atom *argv);
static t_int hid_close(t_hid *x);
static t_int hid_read(t_hid *x,int fd);
static void hid_float(t_hid* x, t_floatarg f);


/*------------------------------------------------------------------------------
 * SUPPORT FUNCTIONS
 */

void debug_print(t_int message_debug_level, const char *fmt, ...)
{
	if(message_debug_level <= global_debug_level)
	{
		char buf[MAXPDSTRING];
		va_list ap;
		//t_int arg[8];
		va_start(ap, fmt);
		vsnprintf(buf, MAXPDSTRING-1, fmt, ap);
		post(buf);
		va_end(ap);
	}
}

void debug_error(t_hid *x, t_int message_debug_level, const char *fmt, ...)
{
	if(message_debug_level <= global_debug_level)
	{
		char buf[MAXPDSTRING];
		va_list ap;
		//t_int arg[8];
		va_start(ap, fmt);
		vsnprintf(buf, MAXPDSTRING-1, fmt, ap);
		pd_error(x, buf);
		va_end(ap);
	}
}

static unsigned int name_to_usage(char *usage_name)
{ // output usagepage << 16 + usage
	if(strcmp(usage_name,"pointer") == 0)   return(0x00010001);
	if(strcmp(usage_name,"mouse") == 0)     return(0x00010002);
	if(strcmp(usage_name,"joystick") == 0)  return(0x00010004);
	if(strcmp(usage_name,"gamepad") == 0)   return(0x00010005);
	if(strcmp(usage_name,"keyboard") == 0)  return(0x00010006);
	if(strcmp(usage_name,"keypad") == 0)    return(0x00010007);
	if(strcmp(usage_name,"multiaxiscontroller") == 0) return(0x00010008);
	return(0);
}

void hid_output_event(t_hid *x, char *type, char *code, t_float value)
{
	t_atom event_data[3];
	
	SETSYMBOL(event_data, gensym(type));	   /* type */
	SETSYMBOL(event_data + 1, gensym(code));	/* code */
	SETFLOAT(event_data + 2, value);	         /* value */

	outlet_anything(x->x_data_outlet,atom_gensym(event_data),2,event_data+1);
}

/* stop polling the device */
void stop_poll(t_hid* x) 
{
  debug_print(LOG_DEBUG,"stop_poll");
  
  if (x->x_started) 
  { 
	  clock_unset(x->x_clock);
	  debug_print(LOG_INFO,"[hid] polling stopped");
	  x->x_started = 0;
  }
}

void hid_set_from_float(t_hid *x, t_floatarg f)
{
/* values greater than 1 set the polling delay time */
/* 1 and 0 for start/stop so you can use a [tgl] */
	if (f > 1)
	{
		x->x_delay = (t_int)f;
		hid_poll(x,f);
	}
	else if (f == 1) 
	{
		if (! x->x_started)
		hid_poll(x,f);
	}
	else if (f == 0) 		
	{
		stop_poll(x);
	}
}

/*------------------------------------------------------------------------------
 * METHODS FOR [hid]'s MESSAGES                    
 */


/* close the device */
t_int hid_close(t_hid *x) 
{
	debug_print(LOG_DEBUG,"hid_close");

/* just to be safe, stop it first */
	stop_poll(x);

	if(! hid_close_device(x)) 
	{
		debug_print(LOG_INFO,"[hid] closed device %d",x->x_device_number);
		x->x_device_open = 0;
		return (0);
	}

	return (1);
}


/* hid_open behavoir
 * current state                 action
 * ---------------------------------------
 * closed / same device          open 
 * open / same device            no action 
 * closed / different device     open 
 * open / different device       close open 
 */

t_int hid_open(t_hid *x, t_symbol *s, t_int argc, t_atom *argv) 
{
	debug_print(LOG_DEBUG,"hid_open");
	unsigned short i;
	unsigned short device_number = 0;
	unsigned short usage_number;
	unsigned int usage;
	char usage_string[MAXPDSTRING] = "";

	if(argc == 1)
	{
		if(atom_getsymbolarg(0,argc,argv) == &s_) 
		{ // single float arg means device
			debug_print(LOG_DEBUG,"[hid] setting device# to %d",device_number);
			device_number = (unsigned short) atom_getfloatarg(0,argc,argv);
		}
		else
		{ // single symbol arg means usagepage/usage
			debug_print(LOG_DEBUG,"[hid] setting device via usagepage/usage");
			atom_string(argv, usage_string, MAXPDSTRING-1);
			i = strlen(usage_string);
			do {
				--i;
			} while(isdigit(usage_string[i]));
			usage_number = strtol(usage_string + i + 1,NULL,10);
			usage_string[i+1] = '\0';
			debug_print(LOG_DEBUG,"[hid] looking for %s #%d",usage_string,usage_number);
			usage = name_to_usage(usage_string);
			debug_print(LOG_DEBUG,"[hid] usage 0x%08x 0x%04x 0x%04x",usage, usage >> 16, usage & 0xffff);
			device_number = get_device_number_from_usage_list(usage_number, 
															  usage >> 16, usage & 0xffff);
		}
	}
	else if( (argc == 2) && (atom_getsymbolarg(0,argc,argv) != NULL) 
			 && (atom_getsymbolarg(1,argc,argv) != NULL) )
	{ /* two symbols means idVendor and idProduct in hex */
	}
	
/* store running state to be restored after the device has been opened */
	t_int started = x->x_started;

/* only close the device if its different than the current and open */	
	if( (device_number != x->x_device_number) && (x->x_device_open) ) 
		hid_close(x);

	if(device_number > 0)
		x->x_device_number = device_number;
	else
		x->x_device_number = 0;

/* if device is open still, that means the same device is trying to be opened,
 * therefore ignore the redundant open request.  To reopen the same device,
 * send a [close( msg, then an [open( msg. */
	if(! x->x_device_open) 
	{
		if(hid_open_device(x,x->x_device_number))
		{
			error("[hid] can not open device %d",x->x_device_number);
			return (1);
		}
		else
		{
			x->x_device_open = 1;
		}
	}
	

/* restore the polling state so that when I [tgl] is used to start/stop [hid],
 * the [tgl]'s state will continue to accurately reflect [hid]'s state  */
	if(started)
		hid_set_from_float(x,x->x_delay);

	debug_print(LOG_DEBUG,"[hid] done device# to %d",device_number);

	return (0);
}


t_int hid_read(t_hid *x,int fd) 
{
//	debug_print(LOG_DEBUG,"hid_read");

	hid_get_events(x);
	
	if (x->x_started) 
	{
		clock_delay(x->x_clock, x->x_delay);
	}
	
	// TODO: why is this 1? 
	return 1; 
}

void hid_poll(t_hid* x, t_float f) 
{
	debug_print(LOG_DEBUG,"hid_poll");
  
/*	if the user sets the delay less than one, ignore */
	if( f > 0 ) 	
		x->x_delay = (t_int)f;

	if(!x->x_device_open)
		hid_open(x,gensym("open"),0,NULL);
	
	if(!x->x_started) 
	{
		clock_delay(x->x_clock, x->x_delay);
		debug_print(LOG_DEBUG,"[hid] polling started");
		x->x_started = 1;
	} 
}

static void hid_anything(t_hid *x, t_symbol *s, t_int argc, t_atom *argv)
{
	int i;
	t_symbol *my_symbol;
	char device_name[MAXPDSTRING];
		
	startpost("ANYTHING! selector: %s data:");
	for(i=0; i<argc; ++i)
	{
		my_symbol = atom_getsymbolarg(i,argc,argv);
		if(my_symbol != NULL)
			post(" %s",my_symbol->s_name);
		else
			post(" %f",atom_getfloatarg(i,argc,argv));
	}
}


static void hid_float(t_hid* x, t_floatarg f) 
{
	debug_print(LOG_DEBUG,"hid_float");

	hid_set_from_float(x,f);
}

static void hid_debug(t_hid *x, t_float f)
{
	global_debug_level = f;
}


/*------------------------------------------------------------------------------
 * system functions 
 */
static void hid_free(t_hid* x) 
{
	debug_print(LOG_DEBUG,"hid_free");
		
	hid_close(x);
	clock_free(x->x_clock);
	hid_instance_count--;

	hid_platform_specific_free(x);
}

/* create a new instance of this class */
static void *hid_new(t_float f) 
{
  t_hid *x = (t_hid *)pd_new(hid_class);
  
  debug_print(LOG_DEBUG,"hid_new");
  
/* only display the version when the first instance is loaded */
  if(!hid_instance_count)
  {
	  post("[hid] %d.%d, written by Hans-Christoph Steiner <hans@eds.org>",
			 HID_MAJOR_VERSION, HID_MINOR_VERSION);  
	  post("\tcompiled on "__DATE__" at "__TIME__ " ");
  }

#if !defined(__linux__) && !defined(__APPLE__)
  error("    !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!");
  error("     This is a dummy, since this object only works GNU/Linux and MacOS X!");
  error("    !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !! WARNING !!");
#endif

  /* init vars */
  global_debug_level = 9; /* high numbers here means see more messages */
  x->x_has_ff = 0;
  x->x_device_open = 0;
  x->x_started = 0;
  x->x_delay = DEFAULT_DELAY;

  x->x_clock = clock_new(x, (t_method)hid_read);

  /* create anything outlet used for HID data */ 
  x->x_data_outlet = outlet_new(&x->x_obj, 0);
  x->x_device_name_outlet = outlet_new(&x->x_obj, 0);

  x->x_device_number = 0;

  if(f > 0)
	  x->x_device_number = f;
  else
	  x->x_device_number = 0;
  
  hid_instance_count++;

  return (x);
}

void hid_setup(void) 
{
	debug_print(LOG_DEBUG,"hid_setup");
	hid_class = class_new(gensym("hid"), 
								 (t_newmethod)hid_new, 
								 (t_method)hid_free,
								 sizeof(t_hid),
								 CLASS_DEFAULT,
								 A_DEFFLOAT,0);
	
	/* add inlet datatype methods */
	class_addfloat(hid_class,(t_method) hid_float);
	class_addbang(hid_class,(t_method) hid_read);
	class_addanything(hid_class,(t_method) hid_anything);
	
	/* add inlet message methods */
	class_addmethod(hid_class,(t_method) hid_debug,gensym("debug"),A_DEFFLOAT,0);
	class_addmethod(hid_class,(t_method) hid_build_device_list,gensym("refresh"),0);
	class_addmethod(hid_class,(t_method) hid_print,gensym("print"),0);
	class_addmethod(hid_class,(t_method) hid_open,gensym("open"),A_GIMME,0);
	class_addmethod(hid_class,(t_method) hid_close,gensym("close"),0);
	class_addmethod(hid_class,(t_method) hid_poll,gensym("poll"),A_DEFFLOAT,0);
   /* force feedback messages */
	class_addmethod(hid_class,(t_method) hid_ff_autocenter,
						 gensym("ff_autocenter"),A_DEFFLOAT,0);
	class_addmethod(hid_class,(t_method) hid_ff_gain,gensym("ff_gain"),A_DEFFLOAT,0);
	class_addmethod(hid_class,(t_method) hid_ff_motors,gensym("ff_motors"),A_DEFFLOAT,0);
	class_addmethod(hid_class,(t_method) hid_ff_continue,gensym("ff_continue"),0);
	class_addmethod(hid_class,(t_method) hid_ff_pause,gensym("ff_pause"),0);
	class_addmethod(hid_class,(t_method) hid_ff_reset,gensym("ff_reset"),0);
	class_addmethod(hid_class,(t_method) hid_ff_stopall,gensym("ff_stopall"),0);
	/* ff tests */
	class_addmethod(hid_class,(t_method) hid_ff_fftest,gensym("fftest"),A_DEFFLOAT,0);
	class_addmethod(hid_class,(t_method) hid_ff_print,gensym("ff_print"),0);
}

