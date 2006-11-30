/* --------------------------------------------------------------------------*/
/*                                                                           */
/* interface to native HID (Human Interface Devices) API                     */
/* Written by Hans-Christoph Steiner <hans@at.or.at>                         */
/*                                                                           */
/* Copyright (c) 2004-2006 Hans-Christoph Steiner                            */
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

#include "hidio.h"

/*------------------------------------------------------------------------------
 * LOCAL DEFINES
 */

//#define DEBUG(x)
#define DEBUG(x) x 

unsigned short global_debug_level = 0;

static t_class *hidio_class;

/*------------------------------------------------------------------------------
 * FUNCTION PROTOTYPES
 */

//static void hidio_poll(t_hidio *x, t_float f);
static void hidio_open(t_hidio *x, t_symbol *s, int argc, t_atom *argv);
//static t_int hidio_close(t_hidio *x);
//static t_int hidio_read(t_hidio *x,int fd);
//static void hidio_float(t_hidio* x, t_floatarg f);


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

void debug_error(t_hidio *x, t_int message_debug_level, const char *fmt, ...)
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


static void output_status(t_hidio *x, t_symbol *selector, t_float output_value)
{
	t_atom *output_atom = getbytes(sizeof(t_atom));
	SETFLOAT(output_atom, output_value);
	outlet_anything( x->x_status_outlet, selector, 1, output_atom);
	freebytes(output_atom,sizeof(t_atom));
}

static void output_open_status(t_hidio *x)
{
	output_status(x, gensym("open"), x->x_device_open);
}

static void output_device_number(t_hidio *x)
{
	output_status(x, gensym("device"), x->x_device_number);
}

static void output_poll_time(t_hidio *x)
{
	output_status(x, gensym("poll"), x->x_delay);
}

static void output_device_count(t_hidio *x)
{
	output_status(x, gensym("total"), device_count);
}

static void output_element_ranges(t_hidio *x)
{
	if( (x->x_device_number > -1) && (x->x_device_open) )
	{
		unsigned int i;
		t_atom output_data[4];
		
		for(i=0;i<element_count[x->x_device_number];++i)
		{
			SETSYMBOL(output_data, element[x->x_device_number][i]->type);
			SETSYMBOL(output_data + 1, element[x->x_device_number][i]->name);
			SETFLOAT(output_data + 2, element[x->x_device_number][i]->min);
			SETFLOAT(output_data + 3, element[x->x_device_number][i]->max);
			outlet_anything(x->x_status_outlet, gensym("range"), 4, output_data);
		}
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


static short get_device_number_from_arguments(int argc, t_atom *argv)
{
	short device_number = -1;
	unsigned short device_type_instance;
	unsigned int usage;
	unsigned short vendor_id;
	unsigned short product_id;
	char device_type_string[MAXPDSTRING] = "";
	t_symbol *first_argument;
	t_symbol *second_argument;

	if(argc == 1)
	{
		first_argument = atom_getsymbolarg(0,argc,argv);
		if(first_argument == &s_) 
		{ // single float arg means device #
			post("first_argument == &s_");
			device_number = (short) atom_getfloatarg(0,argc,argv);
			if(device_number < 0) device_number = -1;
			debug_print(LOG_DEBUG,"[hidio] setting device# to %d",device_number);
		}
		else
		{ // single symbol arg means first instance of a device type
			atom_string(argv, device_type_string, MAXPDSTRING-1);
			usage = name_to_usage(device_type_string);
			device_number = get_device_number_from_usage(0, usage >> 16, 
														 usage & 0xffff);
			debug_print(LOG_INFO,"[hidio] using 0x%04x 0x%04x for %s",
						usage >> 16, usage & 0xffff, device_type_string);
		}
	}
	else if(argc == 2)
	{ 
		first_argument = atom_getsymbolarg(0,argc,argv);
		second_argument = atom_getsymbolarg(1,argc,argv);
		if( second_argument == &s_ ) 
		{ /* a symbol then a float means match on usage */
			atom_string(argv, device_type_string, MAXPDSTRING-1);
			usage = name_to_usage(device_type_string);
			device_type_instance = atom_getfloatarg(1,argc,argv);
			debug_print(LOG_DEBUG,"[hidio] looking for %s at #%d",
						device_type_string, device_type_instance);
			device_number = get_device_number_from_usage(device_type_instance,
															  usage >> 16, 
															  usage & 0xffff);
		}
		else
		{ /* two symbols means idVendor and idProduct in hex */
			vendor_id = 
				(unsigned short) strtol(first_argument->s_name, NULL, 16);
			product_id = 
				(unsigned short) strtol(second_argument->s_name, NULL, 16);
			device_number = get_device_number_by_id(vendor_id,product_id);
		}
	}
	return(device_number);
}


void hidio_output_event(t_hidio *x, t_hid_element *output_data)
{
	if( (output_data->value != output_data->previous_value) ||
		(output_data->relative) )  // relative data should always be output
	{
		t_atom event_data[3];
		SETSYMBOL(event_data, output_data->name);
		SETFLOAT(event_data + 1, output_data->instance);
		SETFLOAT(event_data + 2, output_data->value);
		outlet_anything(x->x_data_outlet,output_data->type,3,event_data);
	} 
}


/* stop polling the device */
static void stop_poll(t_hidio* x) 
{
  debug_print(LOG_DEBUG,"stop_poll");
  
  if (x->x_started) 
  { 
	  clock_unset(x->x_clock);
	  debug_print(LOG_INFO,"[hidio] polling stopped");
	  x->x_started = 0;
  }
}

/*------------------------------------------------------------------------------
 * METHODS FOR [hidio]'s MESSAGES                    
 */


void hidio_poll(t_hidio* x, t_float f) 
{
	debug_print(LOG_DEBUG,"hidio_poll");
  
/*	if the user sets the delay less than 2, set to block size */
	if( f > 2 )
		x->x_delay = (t_int)f;
	else if( f > 0 ) //TODO make this the actual time between message processing
		x->x_delay = 1.54; 
	if(x->x_device_number > -1) 
	{
		if(!x->x_device_open) 
			hidio_open(x,gensym("open"),0,NULL);
		if(!x->x_started) 
		{
			clock_delay(x->x_clock, x->x_delay);
			debug_print(LOG_DEBUG,"[hidio] polling started");
			x->x_started = 1;
		} 
	}
}

static void hidio_set_from_float(t_hidio *x, t_floatarg f)
{
/* values greater than 1 set the polling delay time */
/* 1 and 0 for start/stop so you can use a [tgl] */
	if(f > 1)
	{
		x->x_delay = (t_int)f;
		hidio_poll(x,f);
	}
	else if(f == 1) 
	{
		if(! x->x_started)
			hidio_poll(x,f);
	}
	else if(f == 0) 		
	{
		stop_poll(x);
	}
}

/* close the device */
t_int hidio_close(t_hidio *x) 
{
	debug_print(LOG_DEBUG,"hidio_close");

/* just to be safe, stop it first */
	stop_poll(x);

	if(! hidio_close_device(x)) 
	{
		debug_print(LOG_INFO,"[hidio] closed device %d",x->x_device_number);
		x->x_device_open = 0;
		return (0);
	}

	return (1);
}


/* hidio_open behavoir
 * current state                 action
 * ---------------------------------------
 * closed / same device          open 
 * open / same device            no action 
 * closed / different device     open 
 * open / different device       close open 
 */
static void hidio_open(t_hidio *x, t_symbol *s, int argc, t_atom *argv) 
{
	debug_print(LOG_DEBUG,"hid_%s",s->s_name);
/* store running state to be restored after the device has been opened */
	t_int started = x->x_started;
	
	short device_number = get_device_number_from_arguments(argc, argv);
	if(device_number > -1)
	{
		if( (device_number != x->x_device_number) && (x->x_device_open) ) 
			hidio_close(x);
		if(! x->x_device_open)
		{
			if(hidio_open_device(x,device_number))
				error("[hidio] can not open device %d",device_number);
			else
				x->x_device_open = 1;
		}
	}
	else debug_print(LOG_WARNING,"[hidio] device does not exist");
/* restore the polling state so that when I [tgl] is used to start/stop [hidio],
 * the [tgl]'s state will continue to accurately reflect [hidio]'s state  */
	if(started)
		hidio_set_from_float(x,x->x_delay);
	debug_print(LOG_DEBUG,"[hidio] set device# to %d",device_number);
	output_open_status(x);
	output_device_number(x);
}


t_int hidio_read(t_hidio *x, int fd) 
{
//	debug_print(LOG_DEBUG,"hidio_read");
	unsigned int i;
	double right_now = clock_getlogicaltime();
	t_hid_element *current_element;
	
	if(right_now > last_execute_time[x->x_device_number])
	{
		hidio_get_events(x);
		last_execute_time[x->x_device_number] = right_now;
/*		post("executing: instance %d/%d at %ld", 
		x->x_instance, hidio_instance_count, right_now);*/
	}
	for(i=0; i< element_count[x->x_device_number]; ++i)
	{
		current_element = element[x->x_device_number][i];
		if(current_element->previous_value != current_element->value)
		{
			hidio_output_event(x, current_element);
			if(!current_element->relative)
				current_element->previous_value = current_element->value;
		}
	}
	if (x->x_started) 
	{
		clock_delay(x->x_clock, x->x_delay);
	}
	
	// TODO: why is this 1? 
	return 1; 
}

static void hidio_info(t_hidio *x)
{
	output_open_status(x);
	output_device_number(x);
	output_device_count(x);
	output_poll_time(x);
	output_element_ranges(x);
	hidio_platform_specific_info(x);
}

static void hidio_float(t_hidio* x, t_floatarg f) 
{
	debug_print(LOG_DEBUG,"hid_float");

	hidio_set_from_float(x,f);
}

static void hidio_debug(t_hidio *x, t_float f)
{
	global_debug_level = f;
}


/*------------------------------------------------------------------------------
 * system functions 
 */
static void hidio_free(t_hidio* x) 
{
	debug_print(LOG_DEBUG,"hidio_free");
		
	hidio_close(x);
	clock_free(x->x_clock);
	hidio_instance_count--;

	hidio_platform_specific_free(x);
}

/* create a new instance of this class */
static void *hidio_new(t_symbol *s, int argc, t_atom *argv) 
{
	DEBUG(post("hidio_new"););
	t_hidio *x = (t_hidio *)pd_new(hidio_class);
	unsigned int i;
	
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
  for(i=0; i<MAX_DEVICES; ++i) last_execute_time[i] = 0;

  x->x_clock = clock_new(x, (t_method)hidio_read);

  /* create anything outlet used for HID data */ 
  x->x_data_outlet = outlet_new(&x->x_obj, 0);
  x->x_status_outlet = outlet_new(&x->x_obj, 0);

  x->x_device_number = get_device_number_from_arguments(argc, argv);
  
  x->x_instance = hidio_instance_count;
  hidio_instance_count++;

  return (x);
}

void hidio_setup(void) 
{
	hidio_class = class_new(gensym("hidin"), 
								 (t_newmethod)hidio_new, 
								 (t_method)hidio_free,
								 sizeof(t_hidio),
								 CLASS_DEFAULT,
								 A_GIMME,0);
	
	/* add inlet datatype methods */
	class_addfloat(hidio_class,(t_method) hidio_float);
	class_addbang(hidio_class,(t_method) hidio_read);
/* 	class_addanything(hidio_class,(t_method) hidio_anything); */
	
	/* add inlet message methods */
/*	class_addmethod(hidio_class,(t_method) hidio_debug,gensym("debug"),A_DEFFLOAT,0);
	class_addmethod(hidio_class,(t_method) hidio_build_device_list,gensym("refresh"),0);
	class_addmethod(hidio_class,(t_method) hidio_print,gensym("print"),0);
	class_addmethod(hidio_class,(t_method) hidio_info,gensym("info"),0);
	class_addmethod(hidio_class,(t_method) hidio_open,gensym("open"),A_GIMME,0);
	class_addmethod(hidio_class,(t_method) hidio_close,gensym("close"),0);
	class_addmethod(hidio_class,(t_method) hidio_poll,gensym("poll"),A_DEFFLOAT,0);
   /* force feedback messages */
/*	class_addmethod(hidio_class,(t_method) hidio_ff_autocenter,
						 gensym("ff_autocenter"),A_DEFFLOAT,0);
	class_addmethod(hidio_class,(t_method) hidio_ff_gain,gensym("ff_gain"),A_DEFFLOAT,0);
	class_addmethod(hidio_class,(t_method) hidio_ff_motors,gensym("ff_motors"),A_DEFFLOAT,0);
	class_addmethod(hidio_class,(t_method) hidio_ff_continue,gensym("ff_continue"),0);
	class_addmethod(hidio_class,(t_method) hidio_ff_pause,gensym("ff_pause"),0);
	class_addmethod(hidio_class,(t_method) hidio_ff_reset,gensym("ff_reset"),0);
	class_addmethod(hidio_class,(t_method) hidio_ff_stopall,gensym("ff_stopall"),0);
	/* ff tests */
/*	class_addmethod(hidio_class,(t_method) hidio_ff_fftest,gensym("fftest"),A_DEFFLOAT,0);
	class_addmethod(hidio_class,(t_method) hidio_ff_print,gensym("ff_print"),0);
*/

	post("[hidio] %d.%d, written by Hans-Christoph Steiner <hans@eds.org>",
		 HIDIO_MAJOR_VERSION, HIDIO_MINOR_VERSION);  
	post("\tcompiled on "__DATE__" at "__TIME__ " ");
	post("arg!!!!!!");
}

