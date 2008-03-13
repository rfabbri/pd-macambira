/* --------------------------------------------------------------------------*/
/*                                                                           */
/* read the sudden motion sensor on Apple Mac OS X                           */
/* Written by Hans-Christoph Steiner <hans@at.or.at>                         */
/*                                                                           */
/* Copyright (c) 2008 Free Software Foundation                               */
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
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA */
/*                                                                           */
/* --------------------------------------------------------------------------*/

#include <mach/mach.h> 
#include <IOKit/IOKitLib.h> 
#include <CoreFoundation/CoreFoundation.h> 
#include <m_pd.h>

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */

static t_class *sudden_motion_sensor_class;

typedef struct _sudden_motion_sensor {
    t_object            x_obj;
    t_symbol*           sensor_name;
    
    io_connect_t        io_connect;
    int                 kernel_function;
    int                 data_size;

    t_outlet*           data_outlet;
    t_outlet*           status_outlet;
} t_sudden_motion_sensor;

struct data {
	char x;
	char y;
	char z;
	char pad[57];
};

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void sudden_motion_sensor_output(t_sudden_motion_sensor* x)
{
	DEBUG(post("sudden_motion_sensor_output"););
    kern_return_t kern_result;

	IOItemCount structureInputSize;
	IOByteCount structureOutputSize;

	struct data inputStructure;
	struct data outputStructure;

    t_atom output_atoms[3];


	structureInputSize = x->data_size;	//sizeof(struct data);
	structureOutputSize = x->data_size;	//sizeof(struct data);

	memset(&inputStructure, 0, sizeof(inputStructure));
	memset(&outputStructure, 0, sizeof(outputStructure));
    
	kern_result = IOConnectMethodStructureIStructureO(x->io_connect,
                                                     x->kernel_function,	/* index to kernel function */
                                                     structureInputSize,
                                                     &structureOutputSize,
                                                     &inputStructure,
                                                     &outputStructure);
    
	IOServiceClose(x->io_connect);
    if( kern_result == KERN_SUCCESS)
    {
        SETFLOAT(output_atoms, outputStructure.x);
        SETFLOAT(output_atoms + 1, outputStructure.y);
        SETFLOAT(output_atoms + 2, outputStructure.z);
        outlet_list(x->data_outlet, &s_list, 3, output_atoms);
    }
    else if(kern_result == kIOReturnBusy)
        pd_error(x,"[sudden_motion_sensor]: device busy");
    else
        pd_error(x,"[sudden_motion_sensor]: could not read device");
}


static void sudden_motion_sensor_info(t_sudden_motion_sensor* x)
{
    t_atom output_atom;
    SETSYMBOL(&output_atom, x->sensor_name);
    outlet_anything(x->status_outlet, gensym("sensor"), 1, &output_atom);
}


static int open_sensor(t_sudden_motion_sensor* x,
                       mach_port_t mach_port, io_iterator_t *io_iterator_ptr, 
                       const char *sensor_string, int data_size, int kernel_function)
{
    kern_return_t kern_result;
    post("trying %s", sensor_string);
	kern_result = IOServiceGetMatchingServices(mach_port, IOServiceMatching("IOI2CMotionSensor"), io_iterator_ptr);
	if (kern_result == KERN_SUCCESS && *io_iterator_ptr != 0) 
    {
        post("found %s", sensor_string);
        x->sensor_name = gensym(sensor_string);
		x->data_size = data_size;
		x->kernel_function = kernel_function;
        return 1;
	}
    else
        return 0;
}

static void *sudden_motion_sensor_new(void) 
{
	DEBUG(post("sudden_motion_sensor_new"););
	t_sudden_motion_sensor *x = (t_sudden_motion_sensor *)pd_new(sudden_motion_sensor_class);
	io_iterator_t io_iterator;
	io_object_t io_object;
    kern_return_t kern_result;
	mach_port_t mach_port;

/*
    kern_result = IOMasterPort(MACH_PORT_NULL, &mach_port);
	if (kern_result != KERN_SUCCESS) 
		pd_error(x, "[sudden_motion_sensor]: IOMasterPort(MACH_PORT_NULL, &mach_port)");
    else if(!open_sensor(x, mach_port, &io_iterator, "IOI2CMotionSensor", 60, 21))
        if(!open_sensor(x, mach_port, &io_iterator, "PMUMotionSensor", 60, 21))
            if(!open_sensor(x, mach_port, &io_iterator, "SMCMotionSensor", 40, 5))
                pd_error(x,"[sudden_motion_sensor]: cannot find motion sensor");

    io_object = IOIteratorNext(io_iterator);
    IOObjectRelease(io_iterator);
    if (io_object) 
    {
        kern_result = IOServiceOpen(io_object, mach_task_self(), 0, &x->io_connect);
        IOObjectRelease(io_object);
        if (kern_result != KERN_SUCCESS) 
            pd_error(x,"[sudden_motion_sensor]: could not open motion sensor");
    }
    else
        pd_error(x,"[sudden_motion_sensor]: motion sensor not available");
*/



	kern_result = IOMasterPort(MACH_PORT_NULL, &mach_port);
	if (kern_result != KERN_SUCCESS) {
		error("[sudden_motion_sensor]: IOMasterPort(MACH_PORT_NULL, &mach_port)");
	}

	//PowerBookG4, iBookG4
	kern_result = IOServiceGetMatchingServices(mach_port, IOServiceMatching("IOI2CMotionSensor"), &io_iterator);
	if (kern_result == KERN_SUCCESS && io_iterator != 0) {
        x->sensor_name = gensym("IOI2CMotionSensor");        
		x->data_size = 60;
		x->kernel_function = 21;
		goto FOUND_SENSOR;
	}
	//
	kern_result = IOServiceGetMatchingServices(mach_port, IOServiceMatching("PMUMotionSensor"), &io_iterator);
	if (kern_result == KERN_SUCCESS && io_iterator != 0) {
        x->sensor_name = gensym("PMUMotionSensor");        
		x->data_size = 60;
		x->kernel_function = 21;
		goto FOUND_SENSOR;
	}
	//
	kern_result = IOServiceGetMatchingServices(mach_port, IOServiceMatching("SMCMotionSensor"), &io_iterator);
	if (kern_result == KERN_SUCCESS && io_iterator != 0) {
        x->sensor_name = gensym("SMCMotionSensor");        
		x->data_size = 40;
		x->kernel_function = 5;
		goto FOUND_SENSOR;
	}
	
	error("can't find motionsensor\n");


FOUND_SENSOR:
	io_object = IOIteratorNext(io_iterator);
	IOObjectRelease(io_iterator);
	
	if (io_object == 0) {
		error("[sudden_motion_sensor]: No motion sensor available.");
	}

	kern_result = IOServiceOpen(io_object, mach_task_self(), 0, &x->io_connect);
	IOObjectRelease(io_object);

	if (kern_result != KERN_SUCCESS) {
		error("[sudden_motion_sensor]: Could not open motion sensor device.");
	}

    x->data_outlet = outlet_new(&x->x_obj, &s_list);
	x->status_outlet = outlet_new(&x->x_obj, &s_anything);

	return (x);
}

void sudden_motion_sensor_setup(void) 
{
	sudden_motion_sensor_class = class_new(gensym("sudden_motion_sensor"), 
                                           (t_newmethod)sudden_motion_sensor_new,
                                           NULL,
                                           sizeof(t_sudden_motion_sensor), 
                                           CLASS_DEFAULT, 
                                           0);
	/* add inlet datatype methods */
	class_addbang(sudden_motion_sensor_class,(t_method) sudden_motion_sensor_output);
	class_addmethod(sudden_motion_sensor_class,(t_method) sudden_motion_sensor_info, 
                    gensym("info"), 0);
}
