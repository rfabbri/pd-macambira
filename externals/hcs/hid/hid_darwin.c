#ifdef __APPLE__
/*
 *  Apple Darwin HID Manager support for Pd [hid] object
 *
 *  some code from SuperCollider3's SC_HID.cpp by Jan Truetzschler v. Falkenstein
 *
 *  Copyright (c) 2004 Hans-Christoph All rights reserved.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 */

/* struct IOHIDEventStruct */
/* { */
/*     IOHIDElementType	type; */
/*     IOHIDElementCookie	elementCookie; */
/*     SInt32		value; */
/*     AbsoluteTime	timestamp; */
/*     UInt32		longValueSize; */
/*     void *		longValue; */
/* }; */

/* typedef struct { */
/*         natural_t hi; */
/*         natural_t lo; */
/* } AbsoluteTime; */



#include <Carbon/Carbon.h>

#include "HID_Utilities_External.h"

#include <IOKit/hid/IOHIDUsageTables.h>
#include <ForceFeedback/ForceFeedback.h>

#include <mach/mach.h>
#include <mach/mach_error.h>

#include "hid.h"

//#define DEBUG(x)
#define DEBUG(x) x 

/*==============================================================================
 *  GLOBAL VARS
 *======================================================================== */

extern t_int hid_instance_count;

/*==============================================================================
 * FUNCTION PROTOTYPES
 *==============================================================================
 */

/* conversion functions */
char *convertEventsFromDarwinToLinux(pRecElement element);

/*==============================================================================
 * EVENT TYPE/CODE CONVERSION FUNCTIONS
 *==============================================================================
 */

/*
 * This function is needed to translate the USB HID relative flag into the
 * [hid]/linux style events
 */
void convertAxis(pRecElement element, char *linux_type, char *linux_code, char *axis) 
{
	if (element->relative) 
	{ 
		sprintf(linux_type,"rel"); 
		sprintf(linux_code,"rel_%s",axis); 
	}
	else 
	{ 
		sprintf(linux_type,"abs"); 
		sprintf(linux_code,"abs_%s",axis); 
	}
}


void convertDarwinElementToLinuxTypeCode(pRecElement element, char *linux_type, char *linux_code) 
{
	t_int button_offset = 0;

	switch(element->type)
	{
		case kIOHIDElementTypeInput_Button:
			sprintf(linux_type, "key");
			break;
	}

	switch (element->usagePage)
	{
		case kHIDPage_GenericDesktop:
			switch (element->usage)
			{
				case kHIDUsage_GD_X: convertAxis(element, linux_type, linux_code, "x"); break;
				case kHIDUsage_GD_Y: convertAxis(element, linux_type, linux_code, "y"); break;
				case kHIDUsage_GD_Z: convertAxis(element, linux_type, linux_code, "z"); break;
				case kHIDUsage_GD_Rx: convertAxis(element, linux_type, linux_code, "rx"); break;
				case kHIDUsage_GD_Ry: convertAxis(element, linux_type, linux_code, "ry"); break;
				case kHIDUsage_GD_Rz: convertAxis(element, linux_type, linux_code, "rz"); break;
				case kHIDUsage_GD_Wheel: 
					sprintf(linux_type,"rel");sprintf(linux_code,"rel_wheel");break;
				case kHIDUsage_GD_Slider:
					sprintf(linux_type,"abs");sprintf(linux_code,"abs_throttle");break;
			}
			break;
		case kHIDPage_Simulation:
			switch (element->usage)
			{
				case kHIDUsage_Sim_Rudder: 
					sprintf(linux_type,"abs");sprintf(linux_code,"abs_rz");break;
				case kHIDUsage_Sim_Throttle:
					sprintf(linux_type,"abs");sprintf(linux_code,"abs_throttle");break;
			}
			break;
		case kHIDPage_KeyboardOrKeypad:
			sprintf(linux_type, "key"); 
			/* temporary kludge until I feel like writing the translation table */
			sprintf(linux_code, "key_%ld", element->usage); 
			break;
		case kHIDPage_Button:
			sprintf(linux_type, "key"); 
			/* HID Manager button numbers start at 1, [hid] start at 0 */
			sprintf(linux_code, "btn_%ld", element->usage - 1); 
			break;
		case kHIDPage_LEDs:
			/* temporary kludge until I feel like writing the translation table */
			sprintf(linux_type, "led"); 
			sprintf(linux_code, "led_%ld", element->usage - 1); 
			break;
		case kHIDPage_PID:
			/* temporary kludge until I feel like writing the translation table */
			sprintf(linux_type, "ff"); 
			sprintf(linux_code, "ff_%ld", element->usage - 1); 
			break;
		default:
			/* temporary kludge until I feel like writing the translation table */
			sprintf(linux_type, "not_implemented"); 
			sprintf(linux_code, "notimp_%ld", element->usage - 1); 
	}
}

/*
 * Linux input events report hatswitches as absolute axes with -1, 0, 1 as
 * possible values.  MacOS X HID Manager reports hatswitches as a specific
 * hatswitch type with each direction represented by a unique number.  This
 * function converts the unique number to the Linux style axes.
 */
void hid_convert_hatswitch_values(IOHIDEventStruct event, char *linux_type, char *linux_code)
{
	/* 
	 * hmm, not sure how to implement this cleanly yet, so I left the code
	 * inline in hid_get_events().
	 */
}

/* ============================================================================== */
/* DARWIN-SPECIFIC SUPPORT FUNCTIONS */
/* ============================================================================== */

pRecDevice hid_get_device_by_number(t_int device_number)
{
	pRecDevice pCurrentHIDDevice;
	t_int i, numdevs;

/*
 *	If the specified device is greater than the total number of devices, return
 *	an error.
 */
	numdevs = (t_int) HIDCountDevices();
	if (device_number >= numdevs) {
		error("[hid]: no such device, \"%d\", only %d devices found\n",device_number,numdevs);
		return (NULL);
	}
/*
 *	The most recently discovered HID is the first element of the list here.  I
 *	want the oldest to be number 0 rather than the newest, so I use (numdevs -
 *	device_number - 1).
 */
	pCurrentHIDDevice = HIDGetFirstDevice();
	for(i=0; i < numdevs - device_number - 1; ++i)
		pCurrentHIDDevice = HIDGetNextDevice(pCurrentHIDDevice);

	return pCurrentHIDDevice;
}


void hid_build_element_list(t_hid *x) 
{
	
}


t_int hid_print_element_list(t_hid *x)
{
	DEBUG(post("hid_build_element_list"););

	UInt32 i;
	pRecElement	pCurrentHIDElement;
	pRecDevice pCurrentHIDDevice;
	UInt32 numElements;
	char type[256];
	char code[256];
	char type_name[256];
	char usage_name[256];

	pCurrentHIDDevice = hid_get_device_by_number(x->x_device_number);
	if ( ! HIDIsValidDevice(pCurrentHIDDevice) )
	{
		error("[hid]: device %d is not a valid device\n",x->x_device_number);
		return(1);
	}

	pCurrentHIDElement = HIDGetFirstDeviceElement(pCurrentHIDDevice, kHIDElementTypeInput);
	numElements = HIDCountDeviceElements(pCurrentHIDDevice, kHIDElementTypeInput);
	
	DEBUG(post("[hid] found %d elements:",numElements););
	
	post("");
	post("  TYPE\tCODE\tEVENT NAME");
	post("-----------------------------------------------------------");
	for(i=0; i<numElements; i++)
	{
		HIDGetTypeName((IOHIDElementType) pCurrentHIDElement->type, type_name);
		HIDGetUsageName(pCurrentHIDElement->usagePage, 
							 pCurrentHIDElement->usage, usage_name);

		/* some events need more processing than others */
		switch(pCurrentHIDElement->usage)
		{
			case kHIDUsage_GD_Hatswitch:
				post("  %s\t%s\t%s, %s","abs","abs_hat0x",type_name,usage_name);
				post("  %s\t%s\t%s, %s","abs","abs_hat0y",type_name,usage_name);
				break;
			default:
				convertDarwinElementToLinuxTypeCode(pCurrentHIDElement,type,code);
				post("  %s\t%s\t%s, %s",type,code,type_name,usage_name);
		}
		
		pCurrentHIDElement = HIDGetNextDeviceElement (pCurrentHIDElement, kHIDElementTypeInput);
	}
	post("");

	return (0);	
}


void hid_print_device_list(t_hid *x) 
{
	char cstrDeviceName [256];
	t_int i,numdevs;
	UInt32 usagePage, usage;
	pRecDevice pCurrentHIDDevice;

	if( HIDHaveDeviceList() )
	{
		numdevs = (t_int) HIDCountDevices();
		
		post("");
		/* display device list in console */
		for(i=0; i < numdevs; i++)
		{
			pCurrentHIDDevice = hid_get_device_by_number(i);
			post("Device %d: '%s' '%s' version %d",i,pCurrentHIDDevice->manufacturer,
				  pCurrentHIDDevice->product,pCurrentHIDDevice->version);
			//usage
			HIDGetUsageName (pCurrentHIDDevice->usagePage, 
								  pCurrentHIDDevice->usage, 
								  cstrDeviceName);
			DEBUG(post("       vendorID: %d   productID: %d   locID: %d",
						  pCurrentHIDDevice->vendorID,
						  pCurrentHIDDevice->productID,
						  pCurrentHIDDevice->locID););
		}
		post("");
	}
}

void hid_output_device_name(t_hid *x, char *manufacturer, char *product) 
{
	char      *device_name;
	t_symbol  *device_name_symbol;

	device_name = malloc( strlen(manufacturer) + 1 + strlen(product) + 1 );
//	device_name = malloc( 7 + strlen(manufacturer) + 1 + strlen(product) + 1 );
//	strcpy( device_name, "append " );
	strcat( device_name, manufacturer );
	strcat ( device_name, " ");
	strcat( device_name, product );
//	outlet_anything( x->x_device_name_outlet, gensym( device_name ),0,NULL );
	outlet_symbol( x->x_device_name_outlet, gensym( device_name ) );
}

/* ============================================================================== */
/* Pd [hid] FUNCTIONS */
/* ============================================================================== */

t_int hid_get_events(t_hid *x)
{
	//DEBUG(post("hid_get_events"););

	SInt32 value;
	pRecDevice  pCurrentHIDDevice;
	pRecElement pCurrentHIDElement;
	IOHIDEventStruct event;
	char type[256];
	char code[256];
	char event_output_string[256];
	t_atom event_data[4];

	int event_counter = 0;
	Boolean result;

	pCurrentHIDDevice = hid_get_device_by_number(x->x_device_number);

	if(!pCurrentHIDDevice) 
	{
		post("[hid] can't find current device %d.  Was it unplugged?",x->x_device_number);
		return (1);
	}

	while( (HIDGetEvent(pCurrentHIDDevice, (void*) &event)) && (event_counter < 64) ) 
	{
		value = event.value;
		//post("found event: %d",value);
		IOHIDElementCookie cookie = (IOHIDElementCookie) event.elementCookie;

		// find the pRecElement using the cookie
		pCurrentHIDElement =  HIDGetFirstDeviceElement (pCurrentHIDDevice, kHIDElementTypeIO); 
		while (pCurrentHIDElement && (pCurrentHIDElement->cookie != cookie))
			pCurrentHIDElement = HIDGetNextDeviceElement (pCurrentHIDElement, kHIDElementTypeIO);

		DEBUG(
		HIDGetElementNameFromVendorProductCookie(
			pCurrentHIDDevice->vendorID, pCurrentHIDDevice->productID,
			(long) pCurrentHIDElement->cookie, event_output_string);
		); //end DEBUG
		
		/* some events need more processing than others */
		switch(pCurrentHIDElement->usage)
		{
			case kHIDUsage_GD_Hatswitch:
				sprintf(type,"abs");
				switch (value)
				{
/*
 * MacOS X represents this as one event, while [hid] represents it as two
 * distinct axes.  So the conversion requires an added hid_output_event().
 */
					case 0: 
						sprintf(code,"abs_hat0y");value = 1;
						hid_output_event(x,type,code,(t_float)value);
						sprintf(code,"abs_hat0x");value = 0;
						break;
					case 1: 
						sprintf(code,"abs_hat0y");value = 1;
						hid_output_event(x,type,code,(t_float)value);
						sprintf(code,"abs_hat0x");value = 1;
						break;
					case 2: 
						sprintf(code,"abs_hat0y");value = 0;
						hid_output_event(x,type,code,(t_float)value);
						sprintf(code,"abs_hat0x");value = 1;
						break;
					case 3: 
						sprintf(code,"abs_hat0y");value = -1;
						hid_output_event(x,type,code,(t_float)value);
						sprintf(code,"abs_hat0x");value = 1;
						break;
					case 4: 
						sprintf(code,"abs_hat0y");value = -1;
						hid_output_event(x,type,code,(t_float)value);
						sprintf(code,"abs_hat0x");value = 0;
						break;
					case 5: 
						sprintf(code,"abs_hat0y");value = -1;
						hid_output_event(x,type,code,(t_float)value);
						sprintf(code,"abs_hat0x");value = -1;
						break;
					case 6: 
						sprintf(code,"abs_hat0y");value = 0;
						hid_output_event(x,type,code,(t_float)value);
						sprintf(code,"abs_hat0x");value = -1;
						break;
					case 7: 
						sprintf(code,"abs_hat0y");value = 1;
						hid_output_event(x,type,code,(t_float)value);
						sprintf(code,"abs_hat0x");value = -1;
						break;
					case 8: 
						sprintf(code,"abs_hat0y");value = 0;
						hid_output_event(x,type,code,(t_float)value);
						sprintf(code,"abs_hat0x");value = 0;
						break;
				}
				hid_output_event(x,type,code,(t_float)value);
				break;
			default:
				convertDarwinElementToLinuxTypeCode(pCurrentHIDElement,type,code);
				hid_output_event(x,type,code,(t_float)value);
		}

//		DEBUG(post("type: %s    code: %s   event name: %s",type,code,event_output_string););

		
		++event_counter;
	}
	DEBUG(
//	if(event_counter)
//		post("output %d events",event_counter);
	);
/* 	/\* get the first element *\/ */
/* 	pCurrentHIDElement =  HIDGetFirstDeviceElement (pCurrentHIDDevice, kHIDElementTypeIO); */
/* 	/\* cycle thru all elements *\/ */
/* 	while (pCurrentHIDElement) */
/* 	{ */
/* 		//value = HIDGetElementValue (pCurrentHIDDevice, pCurrentHIDElement); */
		
/* 		if (pCurrentHIDElement) */
/* 		{ */
/* 			value = HIDGetElementValue (pCurrentHIDDevice, pCurrentHIDElement); */
/* 			post("current value: %d", value); */
/* 			// if it's not a button and it's not a hatswitch then calibrate */
/* /\* 			if(( pCurrentHIDElement->type != kIOHIDElementTypeInput_Button ) && *\/ */
/* /\* 				( pCurrentHIDElement->usagePage == 0x01 && pCurrentHIDElement->usage != kHIDUsage_GD_Hatswitch))  *\/ */
/* /\* 				value = HIDCalibrateValue ( value, pCurrentHIDElement ); *\/ */

/* 			/\* type *\/ */
/* 			//HIDGetTypeName(pCurrentHIDElement->type,type); */
/* 			convertDarwinToLinuxType((IOHIDElementType) pCurrentHIDElement->type, event_output_string); */
/* 			SETSYMBOL(event_data, gensym(event_output_string)); */
/* 			/\* code *\/ */
/* 			HIDGetUsageName(pCurrentHIDElement->usagePage, pCurrentHIDElement->usage, event_output_string); */
/* 			SETSYMBOL(event_data + 1, gensym(event_output_string)); */
/* 			/\* value *\/ */
/* 			SETFLOAT(event_data + 2, (t_float)value); */
/* 			/\* time *\/ */
/* 			// TODO temp space filler for testing, got to find where I get the event time */
/* 			SETFLOAT(event_data + 3, (t_float)value); */
/* //			SETFLOAT(event_data + 3, (t_float)(hid_input_event.time).tv_sec); */
/* 			outlet_anything(x->x_obj.te_outlet,atom_gensym(event_data),3,event_data+1);  */
/* 		} */
/* 		pCurrentHIDElement = HIDGetNextDeviceElement (pCurrentHIDElement, kHIDElementTypeIO); */
/* 	} */
	return (0);	
}


t_int hid_open_device(t_hid *x, t_int device_number)
{
	DEBUG(post("hid_open_device"););

	t_int result = 0;
	pRecDevice pCurrentHIDDevice = NULL;

	io_service_t hidDevice = NULL;
	FFDeviceObjectReference *pDeviceReference = NULL;

/* rebuild device list to make sure the list is current */
	if ( ! HIDHaveDeviceList() )
	{
		result = (t_int) HIDBuildDeviceList (NULL, NULL); 
		// returns false if no device found
		if(result) 
		{
			error("[hid]: no HID devices found\n");
			return(result);
		}
	}
	
	pCurrentHIDDevice = hid_get_device_by_number(device_number);
	if ( ! HIDIsValidDevice(pCurrentHIDDevice) )
	{
		error("[hid]: device %d is not a valid device\n",device_number);
		return(1);
	}

	hid_output_device_name( x, pCurrentHIDDevice->manufacturer, pCurrentHIDDevice->product );

	post("[hid] opened device %d: %s %s",
		  device_number, pCurrentHIDDevice->manufacturer, pCurrentHIDDevice->product);

	hid_build_element_list(x);

	if ( FFIsForceFeedback(hidDevice) == FF_OK ) 
	{
		post("device has Force Feedback support");
		if ( FFCreateDevice(hidDevice,pDeviceReference) == FF_OK ) 
		{
			post("created FF device");
		}
	}
	

	HIDQueueDevice(pCurrentHIDDevice);
// TODO: queue all elements except absolute axes, those can just be polled

	return(result);
}


t_int hid_close_device(t_hid *x)
{
	DEBUG(post("hid_close_device"););

	t_int result = 0;
	pRecDevice pCurrentHIDDevice = hid_get_device_by_number(x->x_device_number);

	HIDDequeueDevice(pCurrentHIDDevice);
// this doesn't seem to be needed at all
//   result = HIDCloseReleaseInterface(pCurrentHIDDevice);
	
	return(result);
}


t_int hid_build_device_list(t_hid *x)
{
	DEBUG(post("hid_build_device_list"););

	pRecDevice    pCurrentHIDDevice;
	t_atom        device_name_atoms[2];
	
// returns false if no device found
	if(HIDBuildDeviceList (NULL, NULL)) 
		error("[hid]: no HID devices found\n");

	/* send the [menu( msg to set the [hid_menu] to blank */
	outlet_anything( x->x_device_name_outlet, gensym( "menu" ),0,NULL );

	pCurrentHIDDevice = HIDGetFirstDevice();
	while ( pCurrentHIDDevice != NULL )
	{
		hid_output_device_name( x, pCurrentHIDDevice->manufacturer, pCurrentHIDDevice->product );
		pCurrentHIDDevice = HIDGetNextDevice(pCurrentHIDDevice);
	} 
	
	return (0);
}


void hid_print(t_hid *x)
{
	hid_print_device_list(x);
	
	if(x->x_device_open)
		hid_print_element_list(x);
}


void hid_platform_specific_free(t_hid *x)
{
	DEBUG(post("hid_platform_specific_free"););
/* only call this if the last instance is being freed */
	if (hid_instance_count < 1) 
	{
		DEBUG(post("RELEASE ALL hid_instance_count: %d", hid_instance_count););
		HIDReleaseAllDeviceQueues();
		HIDReleaseDeviceList();
	}
}

//void HIDGetUsageName (const long valueUsagePage, const long valueUsage, char * cstrName)
char *convertEventsFromDarwinToLinux(pRecElement element)
{
	char *cstrName = "";
// this allows these definitions to exist in an XML .plist file
/* 	if (xml_GetUsageName(valueUsagePage, valueUsage, cstrName)) */
/* 		return; */

    switch (element->usagePage)
    {
        case kHIDPage_Undefined:
			switch (element->usage)
            {
                default: sprintf (cstrName, "Undefined Page, Usage 0x%lx", element->usage); break;
            }
            break;
        case kHIDPage_GenericDesktop:
            switch (element->usage)
            {
                case kHIDUsage_GD_Pointer: sprintf (cstrName, "Pointer"); break;
                case kHIDUsage_GD_Mouse: sprintf (cstrName, "Mouse"); break;
                case kHIDUsage_GD_Joystick: sprintf (cstrName, "Joystick"); break;
                case kHIDUsage_GD_GamePad: sprintf (cstrName, "GamePad"); break;
                case kHIDUsage_GD_Keyboard: sprintf (cstrName, "Keyboard"); break;
                case kHIDUsage_GD_Keypad: sprintf (cstrName, "Keypad"); break;
                case kHIDUsage_GD_MultiAxisController: sprintf (cstrName, "Multi-Axis Controller"); break;

                case kHIDUsage_GD_X: sprintf (cstrName, "X-Axis"); break;
                case kHIDUsage_GD_Y: sprintf (cstrName, "Y-Axis"); break;
                case kHIDUsage_GD_Z: sprintf (cstrName, "Z-Axis"); break;
                case kHIDUsage_GD_Rx: sprintf (cstrName, "X-Rotation"); break;
                case kHIDUsage_GD_Ry: sprintf (cstrName, "Y-Rotation"); break;
                case kHIDUsage_GD_Rz: sprintf (cstrName, "Z-Rotation"); break;
                case kHIDUsage_GD_Slider: sprintf (cstrName, "Slider"); break;
                case kHIDUsage_GD_Dial: sprintf (cstrName, "Dial"); break;
                case kHIDUsage_GD_Wheel: sprintf (cstrName, "Wheel"); break;
                case kHIDUsage_GD_Hatswitch: sprintf (cstrName, "Hatswitch"); break;
                case kHIDUsage_GD_CountedBuffer: sprintf (cstrName, "Counted Buffer"); break;
                case kHIDUsage_GD_ByteCount: sprintf (cstrName, "Byte Count"); break;
                case kHIDUsage_GD_MotionWakeup: sprintf (cstrName, "Motion Wakeup"); break;
                case kHIDUsage_GD_Start: sprintf (cstrName, "Start"); break;
                case kHIDUsage_GD_Select: sprintf (cstrName, "Select"); break;

                case kHIDUsage_GD_Vx: sprintf (cstrName, "X-Velocity"); break;
                case kHIDUsage_GD_Vy: sprintf (cstrName, "Y-Velocity"); break;
                case kHIDUsage_GD_Vz: sprintf (cstrName, "Z-Velocity"); break;
                case kHIDUsage_GD_Vbrx: sprintf (cstrName, "X-Rotation Velocity"); break;
                case kHIDUsage_GD_Vbry: sprintf (cstrName, "Y-Rotation Velocity"); break;
                case kHIDUsage_GD_Vbrz: sprintf (cstrName, "Z-Rotation Velocity"); break;
                case kHIDUsage_GD_Vno: sprintf (cstrName, "Vno"); break;

                case kHIDUsage_GD_SystemControl: sprintf (cstrName, "System Control"); break;
                case kHIDUsage_GD_SystemPowerDown: sprintf (cstrName, "System Power Down"); break;
                case kHIDUsage_GD_SystemSleep: sprintf (cstrName, "System Sleep"); break;
                case kHIDUsage_GD_SystemWakeUp: sprintf (cstrName, "System Wake Up"); break;
                case kHIDUsage_GD_SystemContextMenu: sprintf (cstrName, "System Context Menu"); break;
                case kHIDUsage_GD_SystemMainMenu: sprintf (cstrName, "System Main Menu"); break;
                case kHIDUsage_GD_SystemAppMenu: sprintf (cstrName, "System App Menu"); break;
                case kHIDUsage_GD_SystemMenuHelp: sprintf (cstrName, "System Menu Help"); break;
                case kHIDUsage_GD_SystemMenuExit: sprintf (cstrName, "System Menu Exit"); break;
                case kHIDUsage_GD_SystemMenu: sprintf (cstrName, "System Menu"); break;
                case kHIDUsage_GD_SystemMenuRight: sprintf (cstrName, "System Menu Right"); break;
                case kHIDUsage_GD_SystemMenuLeft: sprintf (cstrName, "System Menu Left"); break;
                case kHIDUsage_GD_SystemMenuUp: sprintf (cstrName, "System Menu Up"); break;
                case kHIDUsage_GD_SystemMenuDown: sprintf (cstrName, "System Menu Down"); break;

                case kHIDUsage_GD_DPadUp: sprintf (cstrName, "DPad Up"); break;
                case kHIDUsage_GD_DPadDown: sprintf (cstrName, "DPad Down"); break;
                case kHIDUsage_GD_DPadRight: sprintf (cstrName, "DPad Right"); break;
                case kHIDUsage_GD_DPadLeft: sprintf (cstrName, "DPad Left"); break;

                case kHIDUsage_GD_Reserved: sprintf (cstrName, "Reserved"); break;

                default: sprintf (cstrName, "Generic Desktop Usage 0x%lx", element->usage); break;
            }
            break;
        case kHIDPage_Simulation:
            switch (element->usage)
            {
                default: sprintf (cstrName, "Simulation Usage 0x%lx", element->usage); break;
            }
            break;
        case kHIDPage_VR:
            switch (element->usage)
            {
                default: sprintf (cstrName, "VR Usage 0x%lx", element->usage); break;
            }
            break;
        case kHIDPage_Sport:
            switch (element->usage)
            {
                default: sprintf (cstrName, "Sport Usage 0x%lx", element->usage); break;
            }
            break;
        case kHIDPage_Game:
            switch (element->usage)
            {
                default: sprintf (cstrName, "Game Usage 0x%lx", element->usage); break;
            }
            break;
        case kHIDPage_KeyboardOrKeypad:
            switch (element->usage)
            {
                default: sprintf (cstrName, "Keyboard Usage 0x%lx", element->usage); break;
            }
            break;
        case kHIDPage_LEDs:
            switch (element->usage)
            {
				// some LED usages
				case kHIDUsage_LED_IndicatorRed: sprintf (cstrName, "Red LED"); break;
				case kHIDUsage_LED_IndicatorGreen: sprintf (cstrName, "Green LED"); break;
				case kHIDUsage_LED_IndicatorAmber: sprintf (cstrName, "Amber LED"); break;
				case kHIDUsage_LED_GenericIndicator: sprintf (cstrName, "Generic LED"); break;
				case kHIDUsage_LED_SystemSuspend: sprintf (cstrName, "System Suspend LED"); break;
				case kHIDUsage_LED_ExternalPowerConnected: sprintf (cstrName, "External Power LED"); break;
				default: sprintf (cstrName, "LED Usage 0x%lx", element->usage); break;
            }
            break;
        case kHIDPage_Button:
            switch (element->usage)
            {
                default: sprintf (cstrName, "Button #%ld", element->usage); break;
            }
            break;
        case kHIDPage_Ordinal:
            switch (element->usage)
            {
                default: sprintf (cstrName, "Ordinal Instance %lx", element->usage); break;
            }
            break;
        case kHIDPage_Telephony:
            switch (element->usage)
            {
                default: sprintf (cstrName, "Telephony Usage 0x%lx", element->usage); break;
            }
            break;
        case kHIDPage_Consumer:
            switch (element->usage)
            {
                default: sprintf (cstrName, "Consumer Usage 0x%lx", element->usage); break;
            }
            break;
        case kHIDPage_Digitizer:
            switch (element->usage)
            {
                default: sprintf (cstrName, "Digitizer Usage 0x%lx", element->usage); break;
            }
            break;
        case kHIDPage_PID:
			if (((element->usage >= 0x02) && (element->usage <= 0x1F)) || ((element->usage >= 0x29) && (element->usage <= 0x2F)) ||
	   ((element->usage >= 0x35) && (element->usage <= 0x3F)) || ((element->usage >= 0x44) && (element->usage <= 0x4F)) ||
	   (element->usage == 0x8A) || (element->usage == 0x93)  || ((element->usage >= 0x9D) && (element->usage <= 0x9E)) ||
	   ((element->usage >= 0xA1) && (element->usage <= 0xA3)) || ((element->usage >= 0xAD) && (element->usage <= 0xFFFF)))
                sprintf (cstrName, "PID Reserved");
			else
				switch (element->usage)
				{
					case 0x00: sprintf (cstrName, "PID Undefined Usage"); break;
					case kHIDUsage_PID_PhysicalInterfaceDevice: sprintf (cstrName, "Physical Interface Device"); break;
					case kHIDUsage_PID_Normal: sprintf (cstrName, "Normal Force"); break;

					case kHIDUsage_PID_SetEffectReport: sprintf (cstrName, "Set Effect Report"); break;
					case kHIDUsage_PID_EffectBlockIndex: sprintf (cstrName, "Effect Block Index"); break;
					case kHIDUsage_PID_ParamBlockOffset: sprintf (cstrName, "Parameter Block Offset"); break;
					case kHIDUsage_PID_ROM_Flag: sprintf (cstrName, "ROM Flag"); break;

					case kHIDUsage_PID_EffectType: sprintf (cstrName, "Effect Type"); break;
					case kHIDUsage_PID_ET_ConstantForce: sprintf (cstrName, "Effect Type Constant Force"); break;
					case kHIDUsage_PID_ET_Ramp: sprintf (cstrName, "Effect Type Ramp"); break;
					case kHIDUsage_PID_ET_CustomForceData: sprintf (cstrName, "Effect Type Custom Force Data"); break;
					case kHIDUsage_PID_ET_Square: sprintf (cstrName, "Effect Type Square"); break;
					case kHIDUsage_PID_ET_Sine: sprintf (cstrName, "Effect Type Sine"); break;
					case kHIDUsage_PID_ET_Triangle: sprintf (cstrName, "Effect Type Triangle"); break;
					case kHIDUsage_PID_ET_SawtoothUp: sprintf (cstrName, "Effect Type Sawtooth Up"); break;
					case kHIDUsage_PID_ET_SawtoothDown: sprintf (cstrName, "Effect Type Sawtooth Down"); break;
					case kHIDUsage_PID_ET_Spring: sprintf (cstrName, "Effect Type Spring"); break;
					case kHIDUsage_PID_ET_Damper: sprintf (cstrName, "Effect Type Damper"); break;
					case kHIDUsage_PID_ET_Inertia: sprintf (cstrName, "Effect Type Inertia"); break;
					case kHIDUsage_PID_ET_Friction: sprintf (cstrName, "Effect Type Friction"); break;
					case kHIDUsage_PID_Duration: sprintf (cstrName, "Effect Duration"); break;
					case kHIDUsage_PID_SamplePeriod: sprintf (cstrName, "Effect Sample Period"); break;
					case kHIDUsage_PID_Gain: sprintf (cstrName, "Effect Gain"); break;
					case kHIDUsage_PID_TriggerButton: sprintf (cstrName, "Effect Trigger Button"); break;
					case kHIDUsage_PID_TriggerRepeatInterval: sprintf (cstrName, "Effect Trigger Repeat Interval"); break;

					case kHIDUsage_PID_AxesEnable: sprintf (cstrName, "Axis Enable"); break;
					case kHIDUsage_PID_DirectionEnable: sprintf (cstrName, "Direction Enable"); break;

					case kHIDUsage_PID_Direction: sprintf (cstrName, "Direction"); break;

					case kHIDUsage_PID_TypeSpecificBlockOffset: sprintf (cstrName, "Type Specific Block Offset"); break;

					case kHIDUsage_PID_BlockType: sprintf (cstrName, "Block Type"); break;

					case kHIDUsage_PID_SetEnvelopeReport: sprintf (cstrName, "Set Envelope Report"); break;
					case kHIDUsage_PID_AttackLevel: sprintf (cstrName, "Envelope Attack Level"); break;
					case kHIDUsage_PID_AttackTime: sprintf (cstrName, "Envelope Attack Time"); break;
					case kHIDUsage_PID_FadeLevel: sprintf (cstrName, "Envelope Fade Level"); break;
					case kHIDUsage_PID_FadeTime: sprintf (cstrName, "Envelope Fade Time"); break;

					case kHIDUsage_PID_SetConditionReport: sprintf (cstrName, "Set Condition Report"); break;
					case kHIDUsage_PID_CP_Offset: sprintf (cstrName, "Condition CP Offset"); break;
					case kHIDUsage_PID_PositiveCoefficient: sprintf (cstrName, "Condition Positive Coefficient"); break;
					case kHIDUsage_PID_NegativeCoefficient: sprintf (cstrName, "Condition Negative Coefficient"); break;
					case kHIDUsage_PID_PositiveSaturation: sprintf (cstrName, "Condition Positive Saturation"); break;
					case kHIDUsage_PID_NegativeSaturation: sprintf (cstrName, "Condition Negative Saturation"); break;
					case kHIDUsage_PID_DeadBand: sprintf (cstrName, "Condition Dead Band"); break;

					case kHIDUsage_PID_DownloadForceSample: sprintf (cstrName, "Download Force Sample"); break;
					case kHIDUsage_PID_IsochCustomForceEnable: sprintf (cstrName, "Isoch Custom Force Enable"); break;

					case kHIDUsage_PID_CustomForceDataReport: sprintf (cstrName, "Custom Force Data Report"); break;
					case kHIDUsage_PID_CustomForceData: sprintf (cstrName, "Custom Force Data"); break;

					case kHIDUsage_PID_CustomForceVendorDefinedData: sprintf (cstrName, "Custom Force Vendor Defined Data"); break;
					case kHIDUsage_PID_SetCustomForceReport: sprintf (cstrName, "Set Custom Force Report"); break;
					case kHIDUsage_PID_CustomForceDataOffset: sprintf (cstrName, "Custom Force Data Offset"); break;
					case kHIDUsage_PID_SampleCount: sprintf (cstrName, "Custom Force Sample Count"); break;

					case kHIDUsage_PID_SetPeriodicReport: sprintf (cstrName, "Set Periodic Report"); break;
					case kHIDUsage_PID_Offset: sprintf (cstrName, "Periodic Offset"); break;
					case kHIDUsage_PID_Magnitude: sprintf (cstrName, "Periodic Magnitude"); break;
					case kHIDUsage_PID_Phase: sprintf (cstrName, "Periodic Phase"); break;
					case kHIDUsage_PID_Period: sprintf (cstrName, "Periodic Period"); break;

					case kHIDUsage_PID_SetConstantForceReport: sprintf (cstrName, "Set Constant Force Report"); break;

					case kHIDUsage_PID_SetRampForceReport: sprintf (cstrName, "Set Ramp Force Report"); break;
					case kHIDUsage_PID_RampStart: sprintf (cstrName, "Ramp Start"); break;
					case kHIDUsage_PID_RampEnd: sprintf (cstrName, "Ramp End"); break;

					case kHIDUsage_PID_EffectOperationReport: sprintf (cstrName, "Effect Operation Report"); break;

					case kHIDUsage_PID_EffectOperation: sprintf (cstrName, "Effect Operation"); break;
					case kHIDUsage_PID_OpEffectStart: sprintf (cstrName, "Op Effect Start"); break;
					case kHIDUsage_PID_OpEffectStartSolo: sprintf (cstrName, "Op Effect Start Solo"); break;
					case kHIDUsage_PID_OpEffectStop: sprintf (cstrName, "Op Effect Stop"); break;
					case kHIDUsage_PID_LoopCount: sprintf (cstrName, "Op Effect Loop Count"); break;

					case kHIDUsage_PID_DeviceGainReport: sprintf (cstrName, "Device Gain Report"); break;
					case kHIDUsage_PID_DeviceGain: sprintf (cstrName, "Device Gain"); break;

					case kHIDUsage_PID_PoolReport: sprintf (cstrName, "PID Pool Report"); break;
					case kHIDUsage_PID_RAM_PoolSize: sprintf (cstrName, "RAM Pool Size"); break;
					case kHIDUsage_PID_ROM_PoolSize: sprintf (cstrName, "ROM Pool Size"); break;
					case kHIDUsage_PID_ROM_EffectBlockCount: sprintf (cstrName, "ROM Effect Block Count"); break;
					case kHIDUsage_PID_SimultaneousEffectsMax: sprintf (cstrName, "Simultaneous Effects Max"); break;
					case kHIDUsage_PID_PoolAlignment: sprintf (cstrName, "Pool Alignment"); break;

					case kHIDUsage_PID_PoolMoveReport: sprintf (cstrName, "PID Pool Move Report"); break;
					case kHIDUsage_PID_MoveSource: sprintf (cstrName, "Move Source"); break;
					case kHIDUsage_PID_MoveDestination: sprintf (cstrName, "Move Destination"); break;
					case kHIDUsage_PID_MoveLength: sprintf (cstrName, "Move Length"); break;

					case kHIDUsage_PID_BlockLoadReport: sprintf (cstrName, "PID Block Load Report"); break;

					case kHIDUsage_PID_BlockLoadStatus: sprintf (cstrName, "Block Load Status"); break;
					case kHIDUsage_PID_BlockLoadSuccess: sprintf (cstrName, "Block Load Success"); break;
					case kHIDUsage_PID_BlockLoadFull: sprintf (cstrName, "Block Load Full"); break;
					case kHIDUsage_PID_BlockLoadError: sprintf (cstrName, "Block Load Error"); break;
					case kHIDUsage_PID_BlockHandle: sprintf (cstrName, "Block Handle"); break;

					case kHIDUsage_PID_BlockFreeReport: sprintf (cstrName, "PID Block Free Report"); break;

					case kHIDUsage_PID_TypeSpecificBlockHandle: sprintf (cstrName, "Type Specific Block Handle"); break;

					case kHIDUsage_PID_StateReport: sprintf (cstrName, "PID State Report"); break;
					case kHIDUsage_PID_EffectPlaying: sprintf (cstrName, "Effect Playing"); break;

					case kHIDUsage_PID_DeviceControlReport: sprintf (cstrName, "PID Device Control Report"); break;

					case kHIDUsage_PID_DeviceControl: sprintf (cstrName, "PID Device Control"); break;
					case kHIDUsage_PID_DC_EnableActuators: sprintf (cstrName, "Device Control Enable Actuators"); break;
					case kHIDUsage_PID_DC_DisableActuators: sprintf (cstrName, "Device Control Disable Actuators"); break;
					case kHIDUsage_PID_DC_StopAllEffects: sprintf (cstrName, "Device Control Stop All Effects"); break;
					case kHIDUsage_PID_DC_DeviceReset: sprintf (cstrName, "Device Control Reset"); break;
					case kHIDUsage_PID_DC_DevicePause: sprintf (cstrName, "Device Control Pause"); break;
					case kHIDUsage_PID_DC_DeviceContinue: sprintf (cstrName, "Device Control Continue"); break;
					case kHIDUsage_PID_DevicePaused: sprintf (cstrName, "Device Paused"); break;
					case kHIDUsage_PID_ActuatorsEnabled: sprintf (cstrName, "Actuators Enabled"); break;
					case kHIDUsage_PID_SafetySwitch: sprintf (cstrName, "Safety Switch"); break;
					case kHIDUsage_PID_ActuatorOverrideSwitch: sprintf (cstrName, "Actuator Override Switch"); break;
					case kHIDUsage_PID_ActuatorPower: sprintf (cstrName, "Actuator Power"); break;
					case kHIDUsage_PID_StartDelay: sprintf (cstrName, "Start Delay"); break;

					case kHIDUsage_PID_ParameterBlockSize: sprintf (cstrName, "Parameter Block Size"); break;
					case kHIDUsage_PID_DeviceManagedPool: sprintf (cstrName, "Device Managed Pool"); break;
					case kHIDUsage_PID_SharedParameterBlocks: sprintf (cstrName, "Shared Parameter Blocks"); break;

					case kHIDUsage_PID_CreateNewEffectReport: sprintf (cstrName, "Create New Effect Report"); break;
					case kHIDUsage_PID_RAM_PoolAvailable: sprintf (cstrName, "RAM Pool Available"); break;
					default: sprintf (cstrName, "PID Usage 0x%lx", element->usage); break;
				}
					break;
        case kHIDPage_Unicode:
            switch (element->usage)
            {
                default: sprintf (cstrName, "Unicode Usage 0x%lx", element->usage); break;
            }
            break;
        case kHIDPage_PowerDevice:
			if (((element->usage >= 0x06) && (element->usage <= 0x0F)) || ((element->usage >= 0x26) && (element->usage <= 0x2F)) ||
	   ((element->usage >= 0x39) && (element->usage <= 0x3F)) || ((element->usage >= 0x48) && (element->usage <= 0x4F)) ||
	   ((element->usage >= 0x58) && (element->usage <= 0x5F)) || (element->usage == 0x6A) ||
	   ((element->usage >= 0x74) && (element->usage <= 0xFC)))
                sprintf (cstrName, "Power Device Reserved");
			else
				switch (element->usage)
				{
					case kHIDUsage_PD_Undefined: sprintf (cstrName, "Power Device Undefined Usage"); break;
					case kHIDUsage_PD_iName: sprintf (cstrName, "Power Device Name Index"); break;
					case kHIDUsage_PD_PresentStatus: sprintf (cstrName, "Power Device Present Status"); break;
					case kHIDUsage_PD_ChangedStatus: sprintf (cstrName, "Power Device Changed Status"); break;
					case kHIDUsage_PD_UPS: sprintf (cstrName, "Uninterruptible Power Supply"); break;
					case kHIDUsage_PD_PowerSupply: sprintf (cstrName, "Power Supply"); break;

					case kHIDUsage_PD_BatterySystem: sprintf (cstrName, "Battery System Power Module"); break;
					case kHIDUsage_PD_BatterySystemID: sprintf (cstrName, "Battery System ID"); break;
					case kHIDUsage_PD_Battery: sprintf (cstrName, "Battery"); break;
					case kHIDUsage_PD_BatteryID: sprintf (cstrName, "Battery ID"); break;
					case kHIDUsage_PD_Charger: sprintf (cstrName, "Charger"); break;
					case kHIDUsage_PD_ChargerID: sprintf (cstrName, "Charger ID"); break;
					case kHIDUsage_PD_PowerConverter: sprintf (cstrName, "Power Converter Power Module"); break;
					case kHIDUsage_PD_PowerConverterID: sprintf (cstrName, "Power Converter ID"); break;
					case kHIDUsage_PD_OutletSystem: sprintf (cstrName, "Outlet System power module"); break;
					case kHIDUsage_PD_OutletSystemID: sprintf (cstrName, "Outlet System ID"); break;
					case kHIDUsage_PD_Input: sprintf (cstrName, "Power Device Input"); break;
					case kHIDUsage_PD_InputID: sprintf (cstrName, "Power Device Input ID"); break;
					case kHIDUsage_PD_Output: sprintf (cstrName, "Power Device Output"); break;
					case kHIDUsage_PD_OutputID: sprintf (cstrName, "Power Device Output ID"); break;
					case kHIDUsage_PD_Flow: sprintf (cstrName, "Power Device Flow"); break;
					case kHIDUsage_PD_FlowID: sprintf (cstrName, "Power Device Flow ID"); break;
					case kHIDUsage_PD_Outlet: sprintf (cstrName, "Power Device Outlet"); break;
					case kHIDUsage_PD_OutletID: sprintf (cstrName, "Power Device Outlet ID"); break;
					case kHIDUsage_PD_Gang: sprintf (cstrName, "Power Device Gang"); break;
					case kHIDUsage_PD_GangID: sprintf (cstrName, "Power Device Gang ID"); break;
					case kHIDUsage_PD_PowerSummary: sprintf (cstrName, "Power Device Power Summary"); break;
					case kHIDUsage_PD_PowerSummaryID: sprintf (cstrName, "Power Device Power Summary ID"); break;

					case kHIDUsage_PD_Voltage: sprintf (cstrName, "Power Device Voltage"); break;
					case kHIDUsage_PD_Current: sprintf (cstrName, "Power Device Current"); break;
					case kHIDUsage_PD_Frequency: sprintf (cstrName, "Power Device Frequency"); break;
					case kHIDUsage_PD_ApparentPower: sprintf (cstrName, "Power Device Apparent Power"); break;
					case kHIDUsage_PD_ActivePower: sprintf (cstrName, "Power Device RMS Power"); break;
					case kHIDUsage_PD_PercentLoad: sprintf (cstrName, "Power Device Percent Load"); break;
					case kHIDUsage_PD_Temperature: sprintf (cstrName, "Power Device Temperature"); break;
					case kHIDUsage_PD_Humidity: sprintf (cstrName, "Power Device Humidity"); break;
					case kHIDUsage_PD_BadCount: sprintf (cstrName, "Power Device Bad Condition Count"); break;

					case kHIDUsage_PD_ConfigVoltage: sprintf (cstrName, "Power Device Nominal Voltage"); break;
					case kHIDUsage_PD_ConfigCurrent: sprintf (cstrName, "Power Device Nominal Current"); break;
					case kHIDUsage_PD_ConfigFrequency: sprintf (cstrName, "Power Device Nominal Frequency"); break;
					case kHIDUsage_PD_ConfigApparentPower: sprintf (cstrName, "Power Device Nominal Apparent Power"); break;
					case kHIDUsage_PD_ConfigActivePower: sprintf (cstrName, "Power Device Nominal RMS Power"); break;
					case kHIDUsage_PD_ConfigPercentLoad: sprintf (cstrName, "Power Device Nominal Percent Load"); break;
					case kHIDUsage_PD_ConfigTemperature: sprintf (cstrName, "Power Device Nominal Temperature"); break;

					case kHIDUsage_PD_ConfigHumidity: sprintf (cstrName, "Power Device Nominal Humidity"); break;
					case kHIDUsage_PD_SwitchOnControl: sprintf (cstrName, "Power Device Switch On Control"); break;
					case kHIDUsage_PD_SwitchOffControl: sprintf (cstrName, "Power Device Switch Off Control"); break;
					case kHIDUsage_PD_ToggleControl: sprintf (cstrName, "Power Device Toogle Sequence Control"); break;
					case kHIDUsage_PD_LowVoltageTransfer: sprintf (cstrName, "Power Device Min Transfer Voltage"); break;
					case kHIDUsage_PD_HighVoltageTransfer: sprintf (cstrName, "Power Device Max Transfer Voltage"); break;
					case kHIDUsage_PD_DelayBeforeReboot: sprintf (cstrName, "Power Device Delay Before Reboot"); break;
					case kHIDUsage_PD_DelayBeforeStartup: sprintf (cstrName, "Power Device Delay Before Startup"); break;
					case kHIDUsage_PD_DelayBeforeShutdown: sprintf (cstrName, "Power Device Delay Before Shutdown"); break;
					case kHIDUsage_PD_Test: sprintf (cstrName, "Power Device Test Request/Result"); break;
					case kHIDUsage_PD_ModuleReset: sprintf (cstrName, "Power Device Reset Request/Result"); break;
					case kHIDUsage_PD_AudibleAlarmControl: sprintf (cstrName, "Power Device Audible Alarm Control"); break;

					case kHIDUsage_PD_Present: sprintf (cstrName, "Power Device Present"); break;
					case kHIDUsage_PD_Good: sprintf (cstrName, "Power Device Good"); break;
					case kHIDUsage_PD_InternalFailure: sprintf (cstrName, "Power Device Internal Failure"); break;
					case kHIDUsage_PD_VoltageOutOfRange: sprintf (cstrName, "Power Device Voltage Out Of Range"); break;
					case kHIDUsage_PD_FrequencyOutOfRange: sprintf (cstrName, "Power Device Frequency Out Of Range"); break;
					case kHIDUsage_PD_Overload: sprintf (cstrName, "Power Device Overload"); break;
					case kHIDUsage_PD_OverCharged: sprintf (cstrName, "Power Device Over Charged"); break;
					case kHIDUsage_PD_OverTemperature: sprintf (cstrName, "Power Device Over Temperature"); break;
					case kHIDUsage_PD_ShutdownRequested: sprintf (cstrName, "Power Device Shutdown Requested"); break;

					case kHIDUsage_PD_ShutdownImminent: sprintf (cstrName, "Power Device Shutdown Imminent"); break;
					case kHIDUsage_PD_SwitchOnOff: sprintf (cstrName, "Power Device On/Off Switch Status"); break;
					case kHIDUsage_PD_Switchable: sprintf (cstrName, "Power Device Switchable"); break;
					case kHIDUsage_PD_Used: sprintf (cstrName, "Power Device Used"); break;
					case kHIDUsage_PD_Boost: sprintf (cstrName, "Power Device Boosted"); break;
					case kHIDUsage_PD_Buck: sprintf (cstrName, "Power Device Bucked"); break;
					case kHIDUsage_PD_Initialized: sprintf (cstrName, "Power Device Initialized"); break;
					case kHIDUsage_PD_Tested: sprintf (cstrName, "Power Device Tested"); break;
					case kHIDUsage_PD_AwaitingPower: sprintf (cstrName, "Power Device Awaiting Power"); break;
					case kHIDUsage_PD_CommunicationLost: sprintf (cstrName, "Power Device Communication Lost"); break;

					case kHIDUsage_PD_iManufacturer: sprintf (cstrName, "Power Device Manufacturer String Index"); break;
					case kHIDUsage_PD_iProduct: sprintf (cstrName, "Power Device Product String Index"); break;
					case kHIDUsage_PD_iserialNumber: sprintf (cstrName, "Power Device Serial Number String Index"); break;
					default: sprintf (cstrName, "Power Device Usage 0x%lx", element->usage); break;
				}
					break;
        case kHIDPage_BatterySystem:
			if (((element->usage >= 0x0A) && (element->usage <= 0x0F)) || ((element->usage >= 0x1E) && (element->usage <= 0x27)) ||
	   ((element->usage >= 0x30) && (element->usage <= 0x3F)) || ((element->usage >= 0x4C) && (element->usage <= 0x5F)) ||
	   ((element->usage >= 0x6C) && (element->usage <= 0x7F)) || ((element->usage >= 0x90) && (element->usage <= 0xBF)) ||
	   ((element->usage >= 0xC3) && (element->usage <= 0xCF)) || ((element->usage >= 0xDD) && (element->usage <= 0xEF)) ||
	   ((element->usage >= 0xF2) && (element->usage <= 0xFF)))
                sprintf (cstrName, "Power Device Reserved");
			else
				switch (element->usage)
				{
					case kHIDUsage_BS_Undefined: sprintf (cstrName, "Battery System Undefined"); break;
					case kHIDUsage_BS_SMBBatteryMode: sprintf (cstrName, "SMB Mode"); break;
					case kHIDUsage_BS_SMBBatteryStatus: sprintf (cstrName, "SMB Status"); break;
					case kHIDUsage_BS_SMBAlarmWarning: sprintf (cstrName, "SMB Alarm Warning"); break;
					case kHIDUsage_BS_SMBChargerMode: sprintf (cstrName, "SMB Charger Mode"); break;
					case kHIDUsage_BS_SMBChargerStatus: sprintf (cstrName, "SMB Charger Status"); break;
					case kHIDUsage_BS_SMBChargerSpecInfo: sprintf (cstrName, "SMB Charger Extended Status"); break;
					case kHIDUsage_BS_SMBSelectorState: sprintf (cstrName, "SMB Selector State"); break;
					case kHIDUsage_BS_SMBSelectorPresets: sprintf (cstrName, "SMB Selector Presets"); break;
					case kHIDUsage_BS_SMBSelectorInfo: sprintf (cstrName, "SMB Selector Info"); break;
					case kHIDUsage_BS_OptionalMfgFunction1: sprintf (cstrName, "Battery System Optional SMB Mfg Function 1"); break;
					case kHIDUsage_BS_OptionalMfgFunction2: sprintf (cstrName, "Battery System Optional SMB Mfg Function 2"); break;
					case kHIDUsage_BS_OptionalMfgFunction3: sprintf (cstrName, "Battery System Optional SMB Mfg Function 3"); break;
					case kHIDUsage_BS_OptionalMfgFunction4: sprintf (cstrName, "Battery System Optional SMB Mfg Function 4"); break;
					case kHIDUsage_BS_OptionalMfgFunction5: sprintf (cstrName, "Battery System Optional SMB Mfg Function 5"); break;
					case kHIDUsage_BS_ConnectionToSMBus: sprintf (cstrName, "Battery System Connection To System Management Bus"); break;
					case kHIDUsage_BS_OutputConnection: sprintf (cstrName, "Battery System Output Connection Status"); break;
					case kHIDUsage_BS_ChargerConnection: sprintf (cstrName, "Battery System Charger Connection"); break;
					case kHIDUsage_BS_BatteryInsertion: sprintf (cstrName, "Battery System Battery Insertion"); break;
					case kHIDUsage_BS_Usenext: sprintf (cstrName, "Battery System Use Next"); break;
					case kHIDUsage_BS_OKToUse: sprintf (cstrName, "Battery System OK To Use"); break;
					case kHIDUsage_BS_BatterySupported: sprintf (cstrName, "Battery System Battery Supported"); break;
					case kHIDUsage_BS_SelectorRevision: sprintf (cstrName, "Battery System Selector Revision"); break;
					case kHIDUsage_BS_ChargingIndicator: sprintf (cstrName, "Battery System Charging Indicator"); break;
					case kHIDUsage_BS_ManufacturerAccess: sprintf (cstrName, "Battery System Manufacturer Access"); break;
					case kHIDUsage_BS_RemainingCapacityLimit: sprintf (cstrName, "Battery System Remaining Capacity Limit"); break;
					case kHIDUsage_BS_RemainingTimeLimit: sprintf (cstrName, "Battery System Remaining Time Limit"); break;
					case kHIDUsage_BS_AtRate: sprintf (cstrName, "Battery System At Rate..."); break;
					case kHIDUsage_BS_CapacityMode: sprintf (cstrName, "Battery System Capacity Mode"); break;
					case kHIDUsage_BS_BroadcastToCharger: sprintf (cstrName, "Battery System Broadcast To Charger"); break;
					case kHIDUsage_BS_PrimaryBattery: sprintf (cstrName, "Battery System Primary Battery"); break;
					case kHIDUsage_BS_ChargeController: sprintf (cstrName, "Battery System Charge Controller"); break;
					case kHIDUsage_BS_TerminateCharge: sprintf (cstrName, "Battery System Terminate Charge"); break;
					case kHIDUsage_BS_TerminateDischarge: sprintf (cstrName, "Battery System Terminate Discharge"); break;
					case kHIDUsage_BS_BelowRemainingCapacityLimit: sprintf (cstrName, "Battery System Below Remaining Capacity Limit"); break;
					case kHIDUsage_BS_RemainingTimeLimitExpired: sprintf (cstrName, "Battery System Remaining Time Limit Expired"); break;
					case kHIDUsage_BS_Charging: sprintf (cstrName, "Battery System Charging"); break;
					case kHIDUsage_BS_Discharging: sprintf (cstrName, "Battery System Discharging"); break;
					case kHIDUsage_BS_FullyCharged: sprintf (cstrName, "Battery System Fully Charged"); break;
					case kHIDUsage_BS_FullyDischarged: sprintf (cstrName, "Battery System Fully Discharged"); break;
					case kHIDUsage_BS_ConditioningFlag: sprintf (cstrName, "Battery System Conditioning Flag"); break;
					case kHIDUsage_BS_AtRateOK: sprintf (cstrName, "Battery System At Rate OK"); break;
					case kHIDUsage_BS_SMBErrorCode: sprintf (cstrName, "Battery System SMB Error Code"); break;
					case kHIDUsage_BS_NeedReplacement: sprintf (cstrName, "Battery System Need Replacement"); break;
					case kHIDUsage_BS_AtRateTimeToFull: sprintf (cstrName, "Battery System At Rate Time To Full"); break;
					case kHIDUsage_BS_AtRateTimeToEmpty: sprintf (cstrName, "Battery System At Rate Time To Empty"); break;
					case kHIDUsage_BS_AverageCurrent: sprintf (cstrName, "Battery System Average Current"); break;
					case kHIDUsage_BS_Maxerror: sprintf (cstrName, "Battery System Max Error"); break;
					case kHIDUsage_BS_RelativeStateOfCharge: sprintf (cstrName, "Battery System Relative State Of Charge"); break;
					case kHIDUsage_BS_AbsoluteStateOfCharge: sprintf (cstrName, "Battery System Absolute State Of Charge"); break;
					case kHIDUsage_BS_RemainingCapacity: sprintf (cstrName, "Battery System Remaining Capacity"); break;
					case kHIDUsage_BS_FullChargeCapacity: sprintf (cstrName, "Battery System Full Charge Capacity"); break;
					case kHIDUsage_BS_RunTimeToEmpty: sprintf (cstrName, "Battery System Run Time To Empty"); break;
					case kHIDUsage_BS_AverageTimeToEmpty: sprintf (cstrName, "Battery System Average Time To Empty"); break;
					case kHIDUsage_BS_AverageTimeToFull: sprintf (cstrName, "Battery System Average Time To Full"); break;
					case kHIDUsage_BS_CycleCount: sprintf (cstrName, "Battery System Cycle Count"); break;
					case kHIDUsage_BS_BattPackModelLevel: sprintf (cstrName, "Battery System Batt Pack Model Level"); break;
					case kHIDUsage_BS_InternalChargeController: sprintf (cstrName, "Battery System Internal Charge Controller"); break;
					case kHIDUsage_BS_PrimaryBatterySupport: sprintf (cstrName, "Battery System Primary Battery Support"); break;
					case kHIDUsage_BS_DesignCapacity: sprintf (cstrName, "Battery System Design Capacity"); break;
					case kHIDUsage_BS_SpecificationInfo: sprintf (cstrName, "Battery System Specification Info"); break;
					case kHIDUsage_BS_ManufacturerDate: sprintf (cstrName, "Battery System Manufacturer Date"); break;
					case kHIDUsage_BS_SerialNumber: sprintf (cstrName, "Battery System Serial Number"); break;
					case kHIDUsage_BS_iManufacturerName: sprintf (cstrName, "Battery System Manufacturer Name Index"); break;
					case kHIDUsage_BS_iDevicename: sprintf (cstrName, "Battery System Device Name Index"); break;
					case kHIDUsage_BS_iDeviceChemistry: sprintf (cstrName, "Battery System Device Chemistry Index"); break;
					case kHIDUsage_BS_ManufacturerData: sprintf (cstrName, "Battery System Manufacturer Data"); break;
					case kHIDUsage_BS_Rechargable: sprintf (cstrName, "Battery System Rechargable"); break;
					case kHIDUsage_BS_WarningCapacityLimit: sprintf (cstrName, "Battery System Warning Capacity Limit"); break;
					case kHIDUsage_BS_CapacityGranularity1: sprintf (cstrName, "Battery System Capacity Granularity 1"); break;
					case kHIDUsage_BS_CapacityGranularity2: sprintf (cstrName, "Battery System Capacity Granularity 2"); break;
					case kHIDUsage_BS_iOEMInformation: sprintf (cstrName, "Battery System OEM Information Index"); break;
					case kHIDUsage_BS_InhibitCharge: sprintf (cstrName, "Battery System Inhibit Charge"); break;
					case kHIDUsage_BS_EnablePolling: sprintf (cstrName, "Battery System Enable Polling"); break;
					case kHIDUsage_BS_ResetToZero: sprintf (cstrName, "Battery System Reset To Zero"); break;
					case kHIDUsage_BS_ACPresent: sprintf (cstrName, "Battery System AC Present"); break;
					case kHIDUsage_BS_BatteryPresent: sprintf (cstrName, "Battery System Battery Present"); break;
					case kHIDUsage_BS_PowerFail: sprintf (cstrName, "Battery System Power Fail"); break;
					case kHIDUsage_BS_AlarmInhibited: sprintf (cstrName, "Battery System Alarm Inhibited"); break;
					case kHIDUsage_BS_ThermistorUnderRange: sprintf (cstrName, "Battery System Thermistor Under Range"); break;
					case kHIDUsage_BS_ThermistorHot: sprintf (cstrName, "Battery System Thermistor Hot"); break;
					case kHIDUsage_BS_ThermistorCold: sprintf (cstrName, "Battery System Thermistor Cold"); break;
					case kHIDUsage_BS_ThermistorOverRange: sprintf (cstrName, "Battery System Thermistor Over Range"); break;
					case kHIDUsage_BS_VoltageOutOfRange: sprintf (cstrName, "Battery System Voltage Out Of Range"); break;
					case kHIDUsage_BS_CurrentOutOfRange: sprintf (cstrName, "Battery System Current Out Of Range"); break;
					case kHIDUsage_BS_CurrentNotRegulated: sprintf (cstrName, "Battery System Current Not Regulated"); break;
					case kHIDUsage_BS_VoltageNotRegulated: sprintf (cstrName, "Battery System Voltage Not Regulated"); break;
					case kHIDUsage_BS_MasterMode: sprintf (cstrName, "Battery System Master Mode"); break;
					case kHIDUsage_BS_ChargerSelectorSupport: sprintf (cstrName, "Battery System Charger Support Selector"); break;
					case kHIDUsage_BS_ChargerSpec: sprintf (cstrName, "attery System Charger Specification"); break;
					case kHIDUsage_BS_Level2: sprintf (cstrName, "Battery System Charger Level 2"); break;
					case kHIDUsage_BS_Level3: sprintf (cstrName, "Battery System Charger Level 3"); break;
					default: sprintf (cstrName, "Battery System Usage 0x%lx", element->usage); break;
				}
					break;
        case kHIDPage_AlphanumericDisplay:
            switch (element->usage)
            {
                default: sprintf (cstrName, "Alphanumeric Display Usage 0x%lx", element->usage); break;
            }
            break;
		case kHIDPage_BarCodeScanner:
            switch (element->usage)
            {
                default: sprintf (cstrName, "Bar Code Scanner Usage 0x%lx", element->usage); break;
            }
            break;
        case kHIDPage_Scale:
            switch (element->usage)
            {
                default: sprintf (cstrName, "Scale Usage 0x%lx", element->usage); break;
            }
            break;
        case kHIDPage_CameraControl:
            switch (element->usage)
            {
                default: sprintf (cstrName, "Camera Control Usage 0x%lx", element->usage); break;
            }
            break;
        case kHIDPage_Arcade:
            switch (element->usage)
            {
                default: sprintf (cstrName, "Arcade Usage 0x%lx", element->usage); break;
            }
            break;
        default:
			if (element->usagePage > kHIDPage_VendorDefinedStart)
				sprintf (cstrName, "Vendor Defined Usage 0x%lx", element->usage);
			else
				sprintf (cstrName, "Page: 0x%lx, Usage: 0x%lx", element->usagePage, element->usage);
            break;
    }
	 
	 return(cstrName);
}




#endif  /* #ifdef __APPLE__ */
