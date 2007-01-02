#ifdef _WIN32
/*
 *  Microsoft Windows DDK HID support for Pd [hidio] object
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

#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <setupapi.h> 
#ifdef _MSC_VER
#include <hidsdi.h> 
#else
#include <ddk/hidsdi.h> 
#endif /* _MSC_VER */
#include "hidio.h"

//#define DEBUG(x)
#define DEBUG(x) x 

/*==============================================================================
 *  GLOBAL VARS
 *======================================================================== */

extern t_int hidio_instance_count;

/*==============================================================================
 * FUNCTION PROTOTYPES
 *==============================================================================
 */


/*==============================================================================
 * Event TYPE/CODE CONVERSION FUNCTIONS
 *==============================================================================
 */




/* ============================================================================== */
/* WINDOWS DDK HID SPECIFIC SUPPORT FUNCTIONS */
/* ============================================================================== */

void hidio_build_element_list(t_hidio *x) 
{
	
}

static void hidio_print_element_list(t_hidio *x)
{
	debug_post(LOG_DEBUG,"hidio_print_element_list");


	return EXIT_SUCCESS;	
}

static t_int hidio_print_device_list(t_hidio *x) 
{
	struct _GUID GUID;
	SP_INTERFACE_DEVICE_DATA DeviceInterfaceData;
	struct {DWORD cbSize; char DevicePath[256];} FunctionClassDeviceData;
	HIDD_ATTRIBUTES HIDAttributes;
	SECURITY_ATTRIBUTES SecurityAttributes;
	int i;
	HANDLE PnPHandle, HIDHandle;
	ULONG BytesReturned;
	int Success, ManufacturerName, ProductName;
	char ManufacturerBuffer[256];
	char ProductBuffer[256];
	const char NotSupplied[] = "NULL";
	DWORD lastError = 0;

#if 0
// Initialize the GUID array and setup the security attributes for Win2000
	HidD_GetHidGuid(&GUID);
	SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	SecurityAttributes.lpSecurityDescriptor = NULL;
	SecurityAttributes.bInheritHandle = FALSE;

// Get a handle for the Plug and Play node and request currently active devices
	PnPHandle = SetupDiGetClassDevs(&GUID, NULL, NULL, 
											  DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);

	if ((int)PnPHandle == -1) 
	{ 
		error("[hidio] ERROR: Could not attach to PnP node\n"); 
		return (t_int) GetLastError(); 
	}

// Lets look for a maximum of 32 Devices
	for (i = 0; i < 32; i++) {
// Initialize our data
		DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);
// Is there a device at this table entry
		Success = SetupDiEnumDeviceInterfaces(PnPHandle, NULL, &GUID, i, 
														  &DeviceInterfaceData);
		if (Success) {
// There is a device here, get it's name
			FunctionClassDeviceData.cbSize = 5;
			Success = SetupDiGetDeviceInterfaceDetail(PnPHandle, 
					&DeviceInterfaceData,
					(PSP_INTERFACE_DEVICE_DETAIL_DATA)&FunctionClassDeviceData, 
					256, &BytesReturned, NULL);
			if (!Success) 
			{ 
				error("[hidio] ERROR: Could not find the system name for device %d\n",i); 
				return GetLastError();
			}
// Can now open this device
			HIDHandle = CreateFile(FunctionClassDeviceData.DevicePath, 
										  0, 
										  FILE_SHARE_READ|FILE_SHARE_WRITE, 
										  &SecurityAttributes, OPEN_EXISTING, 0, NULL);
			lastError =  GetLastError();
			if (HIDHandle == INVALID_HANDLE_VALUE) 
			{
				error("[hidio] ERROR: Could not open HID #%d, Errorcode = %d\n", i, (int)lastError);
				return lastError;
			}
			
// Get the information about this HID
			Success = HidD_GetAttributes(HIDHandle, &HIDAttributes);
			if (!Success) 
			{ 
				error("[hidio] ERROR: Could not get HID attributes\n"); 
				return GetLastError(); 
			}
			ManufacturerName = HidD_GetManufacturerString(HIDHandle, ManufacturerBuffer, 256);
			ProductName = HidD_GetProductString(HIDHandle, ProductBuffer, 256);
// And display it!
			post("[hidio]: Device %d: %s %s\n",i,
				  ManufacturerName ? ManufacturerBuffer : NotSupplied,
				  ProductName ? ProductBuffer : NotSupplied);
			post("\tVenderID = %4.4x, Name = ", HIDAttributes.VendorID);
			post("%s\n", ManufacturerName ? ManufacturerBuffer : NotSupplied);
			post("\tProductID = %4.4x, Name = ", HIDAttributes.ProductID);
			post("%s\n", ProductName ? ProductBuffer : NotSupplied);

			CloseHandle(HIDHandle);
		} // if (SetupDiEnumDeviceInterfaces . .
	} // for (i = 0; i < 32; i++)
	SetupDiDestroyDeviceInfoList(PnPHandle);
#endif
	return EXIT_SUCCESS;
}

void hidio_output_device_name(t_hidio *x, char *manufacturer, char *product) 
{
	char      *device_name;
	t_symbol  *device_name_symbol;

	device_name = malloc( strlen(manufacturer) + 1 + strlen(product) + 1 );
//	device_name = malloc( 7 + strlen(manufacturer) + 1 + strlen(product) + 1 );
//	strcpy( device_name, "append " );
	strcat( device_name, manufacturer );
	strcat ( device_name, " ");
	strcat( device_name, product );
//	outlet_anything( x->x_status_outlet, gensym( device_name ),0,NULL );
	outlet_symbol( x->x_status_outlet, gensym( device_name ) );
}

/* ------------------------------------------------------------------------------ */
/*  FORCE FEEDBACK FUNCTIONS */
/* ------------------------------------------------------------------------------ */

/* cross-platform force feedback functions */
t_int hidio_ff_autocenter( t_hidio *x, t_float value )
{
	return EXIT_SUCCESS;
}


t_int hidio_ff_gain( t_hidio *x, t_float value )
{
	return EXIT_SUCCESS;
}


t_int hidio_ff_motors( t_hidio *x, t_float value )
{
	return EXIT_SUCCESS;
}


t_int hidio_ff_continue( t_hidio *x )
{
	return EXIT_SUCCESS;
}


t_int hidio_ff_pause( t_hidio *x )
{
	return EXIT_SUCCESS;
}


t_int hidio_ff_reset( t_hidio *x )
{
	return EXIT_SUCCESS;
}


t_int hidio_ff_stopall( t_hidio *x )
{
	return EXIT_SUCCESS;
}



// these are just for testing...
t_int hidio_ff_fftest ( t_hidio *x, t_float value)
{
	return EXIT_SUCCESS;
}


void hidio_ff_print( t_hidio *x )
{
}

/* ============================================================================== */
/* Pd [hidio] FUNCTIONS */
/* ============================================================================== */

void hidio_get_events(t_hidio *x)
{
	//debug_post(LOG_DEBUG,"hidio_get_events");
}


t_int hidio_open_device(t_hidio *x, t_int device_number)
{
	debug_post(LOG_DEBUG,"hidio_open_device");

	return EXIT_SUCCESS;
}


t_int hidio_close_device(t_hidio *x)
{
	debug_post(LOG_DEBUG,"hidio_close_device");

	return EXIT_SUCCESS;
}


void hidio_build_device_list(t_hidio *x)
{
	debug_post(LOG_DEBUG,"hidio_build_device_list");
	
/*
 * The Windows DDK "hid.dll" has to be loaded manually because Windows gets
 * confused by this object, which is also named "hid.dll".  This is the first
 * platform-specific function called in hid.c, so that's why this is happening
 * here.
 */
	TCHAR hidDllPath[MAX_PATH];
	UINT hidDllPathLength;
	HMODULE hModule = NULL;

	hidDllPathLength = GetSystemDirectory(hidDllPath, MAX_PATH);
	if( !hidDllPathLength  )
	{
		error("[hidio] ERROR: cannot get SystemRoot");
	}
	strcat(hidDllPath,"\\hid.dll");
	post("hidDllPath: %s",hidDllPath);
	hModule = LoadLibrary(hidDllPath);
	if ( !hModule )
	{
		error("[hidio] ERROR: couldn't load %s: error %d",hidDllPath,GetLastError());
	}
}


void hidio_print(t_hidio *x)
{
	hidio_print_device_list(x);
	
	if(x->x_device_open) 
	{
		hidio_print_element_list(x);
		hidio_ff_print( x );
	}
}


void hidio_platform_specific_free(t_hidio *x)
{
	debug_post(LOG_DEBUG,"hidio_platform_specific_free");
/* only call this if the last instance is being freed */
	if (hidio_instance_count < 1) 
	{
		DEBUG(post("RELEASE ALL hidio_instance_count: %d", hidio_instance_count););
	}
}






#endif  /* _WIN32 */
