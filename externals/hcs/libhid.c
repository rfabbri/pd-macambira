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

#include <usb.h>
#include <hid.h>
#include <stdio.h>
#include <string.h>
#include "m_pd.h"


/*------------------------------------------------------------------------------
 *  INCLUDE HACK
 */

/* NOTE: included from libusb/usbi.h. UGLY, i know, but so is libusb! */
struct usb_dev_handle {
  int fd;
  struct usb_bus *bus;
  struct usb_device *device;
  int config;
  int interface;
  int altsetting;
  void *impl_info;
};


/*------------------------------------------------------------------------------
 *  GLOBAL VARIABLES
 */

/*
 * count the number of instances of this object so that certain free()
 * functions can be called only after the final instance is detroyed.
 */
t_int libhid_instance_count;

char *hid_id[32]; /* FIXME: 32 devices MAX */
t_int hid_id_count;

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */

typedef struct _libhid 
{
	t_object            x_obj;
/* libhid types */
	HIDInterface        *x_hidinterface;
	u_int8_t            x_iSerialNumber;
	hid_return          x_hid_return;
/* internal state */
	t_int               x_device_number;
	t_int               x_read_element_count;
	t_int               *x_read_elements;
	t_int               x_write_element_count;
	t_int               *x_write_elements;
/* clock support */
	t_clock             *x_clock;
	t_int               x_delay;
	t_int               x_started;
/* outlets */
	t_outlet            *x_data_outlet;
	t_outlet            *x_control_outlet;
} t_libhid;




/*------------------------------------------------------------------------------
 * LOCAL DEFINES
 */

#define LIBHID_MAJOR_VERSION 0
#define LIBHID_MINOR_VERSION 0

//#define DEBUG(x)
#define DEBUG(x) x 

static t_class *libhid_class;

#define SEND_PACKET_LENGTH 1
#define RECEIVE_PACKET_LENGTH 6
#define PATH_LENGTH 3

/*------------------------------------------------------------------------------
 * SUPPORT FUNCTIONS
 */

/* 
 * This function is used in a HIDInterfaceMatcher to iterate thru all of the
 * HID devices on the USB bus
 */
static bool device_iterator (struct usb_dev_handle const* usbdev, void* custom, 
					  unsigned int len)
{
	bool ret = false;
	t_int i;
	char current_dev_path[10];
  
	/* only here to prevent the unused warning */
	/* TODO remove */
	len = *((unsigned long*)custom);
 
	/* Obtain the device's full path */
	sprintf(current_dev_path, "%s/%s", usbdev->bus->dirname, usbdev->device->filename);

	/* Check if we already saw this dev */
	for ( i = 0 ; ( hid_id[i] != NULL ) ; i++ )
	{
		if (!strcmp(hid_id[i], current_dev_path ) )
			break;
	}
  
	/* Append device to the list if needed */
	if (hid_id[i] == NULL)
	{
		hid_id[i] = (char *) malloc(strlen(usbdev->device->filename) + strlen(usbdev->bus->dirname) );
		sprintf(hid_id[i], "%s/%s", usbdev->bus->dirname, usbdev->device->filename);
		post("bus %s device %s: %d %d",
			 usbdev->bus->dirname, 
			 usbdev->device->filename,
			 usbdev->device->descriptor.idVendor,
			 usbdev->device->descriptor.idProduct);
	}
	else /* device already seen */
	{
		return false;
	}
  
	/* Filter non HID device */
	if ( (usbdev->device->descriptor.bDeviceClass == 0) /* Class defined at interface level */
		 && usbdev->device->config
		 && usbdev->device->config->interface->altsetting->bInterfaceClass == USB_CLASS_HID)
		ret = true;
	else
		ret = false;
  
	return ret;
}

/* -------------------------------------------------------------------------- */
/* This function is used in a HIDInterfaceMatcher in order to match devices by
 * serial number.
 */
/* static bool match_serial_number(struct usb_dev_handle* usbdev, void* custom, unsigned int len) */
/* { */
/*   bool ret; */
/*   char* buffer = (char*)malloc(len); */
/*   usb_get_string_simple(usbdev, usb_device(usbdev)->descriptor.iSerialNumber, */
/*       buffer, len); */
/*   ret = strncmp(buffer, (char*)custom, len) == 0; */
/*   free(buffer); */
/*   return ret; */
/* } */


/* -------------------------------------------------------------------------- */
/* static HIDInterface* get_device_by_number(t_int device_number) */
/* { */
/* 	HIDInterface* return_hid; */
	
/* 	return return_hid; */
/* } */


/* -------------------------------------------------------------------------- */
static t_int* make_hid_packet(t_int element_count, t_int argc, t_atom *argv)
{
	DEBUG(post("make_hid_packet"););
	t_int i;
	t_int *return_array = NULL;
	
	post("element_count %d",element_count);
	return_array = (t_int *) getbytes(sizeof(t_int) * element_count);
 	for(i=0; i < element_count; ++i) 
	{
/*
 * A libhid path component is 32 bits, the high 16 bits identify the usage page,
 * and the low 16 bits the item number.
 */
		return_array[i] = 
			(atom_getintarg(i*2,argc,argv) << 16) + atom_getintarg(i*2+1,argc,argv);
/* TODO: print error if a symbol is found in the data list */
	}
	return return_array;
}


/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

/* -------------------------------------------------------------------------- */
static void libhid_open(t_libhid *x, t_float vendor_id, t_float product_id)
{
	DEBUG(post("libhid_open"););
	
	HIDInterfaceMatcher matcher = { (unsigned short)vendor_id, 
									(unsigned short)product_id, 
									NULL, 
									NULL, 
									0 };

 	if ( !hid_is_opened(x->x_hidinterface) ) 
	{
		x->x_hid_return = hid_force_open(x->x_hidinterface, 0, &matcher, 3);
		if (x->x_hid_return != HID_RET_SUCCESS) {
			error("[libhid] hid_force_open failed with return code %d\n", x->x_hid_return);
		}
	}
}



/* -------------------------------------------------------------------------- */
static void libhid_read(t_libhid *x)
{
	DEBUG(post("libhid_read"););
/* int const PATH_IN[PATH_LENGTH] = { 0xffa00001, 0xffa00002, 0xffa10003 }; */
	int const PATH_OUT[PATH_LENGTH] = { 0x00010030, 0x00010031, 0x00010038 };

	char packet[RECEIVE_PACKET_LENGTH];

/* 	if ( !hid_is_opened(x->x_hidinterface) ) */
/* 	{ */
/* 		libhid_open(x); */
/* 	} */
/* 	else */
/* 	{ */
		x->x_hid_return = hid_get_input_report(x->x_hidinterface, 
											   PATH_OUT, 
											   PATH_LENGTH, 
											   packet, 
											   RECEIVE_PACKET_LENGTH);
		if (x->x_hid_return != HID_RET_SUCCESS) 
			error("[libhid] hid_get_input_report failed with return code %d\n", 
				  x->x_hid_return);
/* 	} */
}



/* -------------------------------------------------------------------------- */
/* set the HID packet for which elements to read */
static void libhid_set_read(t_libhid *x, int argc, t_atom *argv)
{
	DEBUG(post("libhid_set_read"););
	t_int i;

	x->x_read_element_count = argc / 2;
	x->x_read_elements = make_hid_packet(x->x_read_element_count, argc, argv);
	post("x_read_element_count %d",x->x_read_element_count);
	for(i=0;i<x->x_read_element_count;++i)
		post("x_read_elements %d: %d",i,x->x_read_elements[i]);
}


/* -------------------------------------------------------------------------- */
/* set the HID packet for which elements to write */
static void libhid_set_write(t_libhid *x, int argc, t_atom *argv)
{
	DEBUG(post("libhid_set_write"););
	t_int i;

	x->x_write_element_count = argc / 2;
	x->x_write_elements = make_hid_packet(x->x_write_element_count, argc, argv);
	post("x_write_element_count %d",x->x_write_element_count);
	for(i=0;i<x->x_write_element_count;++i)
		post("x_write_elements %d: %d",i,x->x_write_elements[i]);
}



/* -------------------------------------------------------------------------- */
/* convert a list to a HID packet and set it */
static void libhid_set(t_libhid *x, t_symbol *s, int argc, t_atom *argv)
{
	DEBUG(post("libhid_set"););
	t_symbol *subselector;

	subselector = atom_getsymbol(&argv[0]);
	if(strcmp(subselector->s_name,"read") == 0)
		libhid_set_read(x,argc-1,argv+1); 
	if(strcmp(subselector->s_name,"write") == 0)
		libhid_set_write(x,argc-1,argv+1);
}


/* -------------------------------------------------------------------------- */
static void libhid_get(t_libhid *x, t_symbol *s, int argc, t_atom *argv)
{
	DEBUG(post("libhid_get"););
	t_symbol *subselector;

	subselector = atom_getsymbol(&argv[0]);
/* 	if(strcmp(subselector->s_name,"read") == 0) */

/* 	if(strcmp(subselector->s_name,"write") == 0) */

}



/* -------------------------------------------------------------------------- */
static void libhid_close(t_libhid *x) 
{
	DEBUG(post("libhid_close"););

/* just to be safe, stop it first */
//	libhid_stop(x);

	if ( hid_is_opened(x->x_hidinterface) ) 
	{
		x->x_hid_return = hid_close(x->x_hidinterface);
		if (x->x_hid_return == HID_RET_SUCCESS) 
			post("[libhid] closed device %d",x->x_device_number);
		else
			error("[libhid] could not close %d, error #%d",x->x_device_number,x->x_hid_return);
	}
}


/* -------------------------------------------------------------------------- */
static void libhid_print(t_libhid *x)
{
	DEBUG(post("libhid_print"););
	t_int i;
	t_atom event_data[3];

	for ( i = 0 ; ( hid_id[i] != NULL ) ; i++ )
	{
		if( hid_id[i] != NULL )
			post("hid_id[%d]: %s",i,hid_id[i]);
	}
/* 	SETSYMBOL(event_data, gensym(type));	   /\* type *\/ */
/* 	SETSYMBOL(event_data + 1, gensym(code));	/\* code *\/ */
/* 	SETSYMBOL(event_data + 2, value);	         /\* value *\/ */
//	outlet_list(x->x_control_outlet, &s_list,
}


/* -------------------------------------------------------------------------- */
static void libhid_reset(t_libhid *x)
{
	DEBUG(post("libhid_reset"););
	
	hid_reset_HIDInterface(x->x_hidinterface);
}


/* -------------------------------------------------------------------------- */
static void libhid_free(t_libhid* x) 
{
	DEBUG(post("libhid_free"););
		
	libhid_close(x);
	clock_free(x->x_clock);

	freebytes(x->x_read_elements,sizeof(t_int) * x->x_read_element_count);
	freebytes(x->x_write_elements,sizeof(t_int) * x->x_write_element_count);

	hid_delete_HIDInterface(&(x->x_hidinterface));
	x->x_hid_return = hid_cleanup();
	if (x->x_hid_return != HID_RET_SUCCESS) 
		error("[libhid] hid_cleanup failed with return code %d\n", x->x_hid_return);

	libhid_instance_count--;
}



/* -------------------------------------------------------------------------- */
static void *libhid_new(t_float f) 
{
	t_int i;
	HIDInterfaceMatcher matcher;
	t_libhid *x = (t_libhid *)pd_new(libhid_class);
	
	DEBUG(post("libhid_new"););
	
/* only display the version when the first instance is loaded */
	if(!libhid_instance_count)
		post("[libhid] %d.%d, written by Hans-Christoph Steiner <hans@eds.org>",
			 LIBHID_MAJOR_VERSION, LIBHID_MINOR_VERSION);  
	
/*   x->x_clock = clock_new(x, (t_method)libhid_read); */

	/* create anything outlet used for HID data */ 
	x->x_data_outlet = outlet_new(&x->x_obj, 0);
	x->x_control_outlet = outlet_new(&x->x_obj, 0);
	
	/* hid_write_library_config(stdout); */
	/* hid_set_debug(HID_DEBUG_NOTRACES); */
	// hid_set_debug(HID_DEBUG_NONE);
/* 	hid_set_debug(HID_DEBUG_ALL); */
	hid_set_debug_stream(stderr);
	hid_set_usb_debug(0);
	
	/* data init */
	for (i = 0 ; i < 32 ; i++)
		hid_id[i] = NULL;

	if (! hid_is_initialised() )
		x->x_hid_return = hid_init();
	
	x->x_hidinterface = hid_new_HIDInterface();
	matcher.vendor_id = HID_ID_MATCH_ANY;
	matcher.product_id = HID_ID_MATCH_ANY;
	matcher.matcher_fn = device_iterator;
	
	x->x_device_number = f;

	/* find and report the list of devices */
	/* open recursively all HID devices found */
	while ( (x->x_hid_return = hid_force_open(x->x_hidinterface, 0, &matcher, 2)) != HID_RET_DEVICE_NOT_FOUND)
	{
/* 		printf("************************************************************************\n"); */
		
/* 		hid_write_identification(stdout, x->x_hidinterface); */
		
		/* Only dump HID tree if asked */
		/* hid_dump_tree(stdout, x->x_hidinterface); */
		
		hid_close(x->x_hidinterface);
	}
	

  
	/* Open the device and save settings.  If there is an error, return the object
	 * anyway, so that the inlets and outlets are created, thus not breaking the
	 * patch.   */
/* 	if (libhid_open(x,f)) */
/* 		error("[libhid] device %d did not open",(t_int)f); */

	libhid_instance_count++;

	return (x);
}

void libhid_setup(void) 
{
	DEBUG(post("libhid_setup"););
	libhid_class = class_new(gensym("libhid"), 
							 (t_newmethod)libhid_new, 
							 (t_method)libhid_free,
							 sizeof(t_libhid),
							 CLASS_DEFAULT,
							 A_DEFFLOAT,
							 NULL);
	
/* add inlet datatype methods */
	class_addbang(libhid_class,(t_method) libhid_read);

/* add inlet message methods */
	class_addmethod(libhid_class,(t_method) libhid_print,gensym("print"),0);
	class_addmethod(libhid_class,(t_method) libhid_reset,gensym("reset"),0);
	class_addmethod(libhid_class,(t_method) libhid_set,gensym("set"),A_GIMME,0);
	class_addmethod(libhid_class,(t_method) libhid_get,gensym("get"),A_DEFSYM,0);
	class_addmethod(libhid_class,(t_method) libhid_open,gensym("open"),A_DEFFLOAT,A_DEFFLOAT,0);
	class_addmethod(libhid_class,(t_method) libhid_close,gensym("close"),0);
}

