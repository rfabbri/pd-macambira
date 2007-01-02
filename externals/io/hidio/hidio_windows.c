#ifdef _WINDOWS
/*
 *  Microsoft Windows DDK HID support for Pd/Max [hidio] object
 *
 *  Copyright (c) 2006 Olaf Matthes. All rights reserved.
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

/*
 * Please note that this file needs the Microsoft Driver Developent Kit (DDK)
 * to be installed in order to compile!
 */

#ifdef _MSC_VER
#include <hidsdi.h> 
#else
#include <ddk/hidsdi.h> 
#endif /* _MSC_VER */

#include "hidio.h"

//#define DEBUG(x)
#define DEBUG(x) x 

#define debug_post(d, p) post(p)

typedef struct _hid_data
{
   BOOLEAN			IsButtonData;
   unsigned char	Reserved;
   USAGE			UsagePage;   // The usage page for which we are looking.
   unsigned long	Status;      // The last status returned from the accessor function
                            // when updating this field.
   unsigned long	ReportID;    // ReportID for this given data structure
   BOOLEAN			IsDataSet;   // Variable to track whether a given data structure
                            //  has already been added to a report structure

   union
   {
      struct
	  {
         unsigned long	UsageMin;       // Variables to track the usage minimum and max
         unsigned long	UsageMax;       // If equal, then only a single usage
         unsigned long	MaxUsageLength; // Usages buffer length.
         PUSAGE			Usages;         // list of usages (buttons ``down'' on the device.

      } ButtonData;
      struct
	  {
         USAGE			Usage; // The usage describing this value;
         unsigned short	Reserved;

         unsigned long	Value;
         long			ScaledValue;
      } ValueData;
   };
} t_hid_data;

typedef struct _hid_device
{   
    char                 *devicePath;
    HANDLE               device; // A file handle to the hid device.
	HANDLE               event;
	OVERLAPPED           overlapped;

    BOOL                 openedForRead;
    BOOL                 openedForWrite;
    BOOL                 openedOverlapped;
    BOOL                 openedExclusive;

    PHIDP_PREPARSED_DATA ppd; // The opaque parser info describing this device
    HIDP_CAPS            caps; // The Capabilities of this hid device.
    HIDD_ATTRIBUTES      attributes;
    char                 *inputReportBuffer;
    t_hid_data           *inputData; // array of hid data structures
    unsigned long        inputDataLength; // Num elements in this array.
    PHIDP_BUTTON_CAPS    inputButtonCaps;
    PHIDP_VALUE_CAPS     inputValueCaps;

    char                 *outputReportBuffer;
    t_hid_data           *outputData;
    unsigned long        outputDataLength;
    PHIDP_BUTTON_CAPS    outputButtonCaps;
    PHIDP_VALUE_CAPS     outputValueCaps;

    char                 *featureReportBuffer;
    t_hid_data           *featureData;
    unsigned long        featureDataLength;
    PHIDP_BUTTON_CAPS    featureButtonCaps;
    PHIDP_VALUE_CAPS     featureValueCaps;
} t_hid_device;


/*==============================================================================
 *  GLOBAL VARS
 *======================================================================== */

extern t_int hidio_instance_count; // in hidio.c

/* store device pointers so I don't have to query them all the time */
// t_hid_devinfo device_pointer[MAX_DEVICES];

/*==============================================================================
 * FUNCTION PROTOTYPES
 *==============================================================================
 */


/*==============================================================================
 * Event TYPE/CODE CONVERSION FUNCTIONS
 *==============================================================================
 */

/* ============================================================================== */
/* WINDOWS DDK HID SPECIFIC REALLY LOW-LEVEL STUFF */
/* ============================================================================== */

/* count devices by looking into the registry */
short _hid_count_devices(void)
{
    short	i, gNumDevices = 0;
	long	ret;

	HKEY	hKey;
    long	DeviceNameLen, KeyNameLen;
    char	KeyName[MAXPDSTRING];
	char	DeviceName[MAXPDSTRING];

	/* Search in Windows Registry for enumerated HID devices */
    if ((ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\hidusb\\Enum", 0, KEY_QUERY_VALUE, &hKey)) != ERROR_SUCCESS)
	{
		error("hidio: failed to get list of HID devices from registry");
        return -1;
    }

    for (i = 0; i < MAX_DEVICES + 3; i++)	/* there are three entries that are no devices */
	{
        DeviceNameLen = 80;
        KeyNameLen = 100;
		ret = RegEnumValue(hKey, i, KeyName, &KeyNameLen, NULL, NULL, DeviceName, &DeviceNameLen);
        if (ret == ERROR_SUCCESS)
		{
			if (!strncmp(KeyName, "Count", 5))
			{
				/* this is the number of devices as HEX DWORD */
				continue;
			}
			else if (!strncmp(DeviceName, "USB\\Vid", 7))
			{
				/* we found a device, DeviceName contains the path */
				// post("device #%d: %s = %s", gNumDevices, KeyName, DeviceName);
				gNumDevices++;
				continue;
			}
		}
		else if (ret == ERROR_NO_MORE_ITEMS)	/* no more entries in registry */
		{
			break;
		}
		else	/* any other error while looking into registry */
		{
			char errbuf[MAXPDSTRING];
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, ret, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							errbuf, MAXPDSTRING, NULL);
			error("hidio: $s", errbuf);
			break;
		}
	}
	RegCloseKey(hKey);
    return gNumDevices;		/* return number of devices */
}

/* get device path for a HID specified by number */
static short _hid_get_device_path(short device_number, char **path, short length)
{
	GUID guid;
	HDEVINFO DeviceInfo;
	SP_DEVICE_INTERFACE_DATA DeviceInterface;
	PSP_INTERFACE_DEVICE_DETAIL_DATA DeviceDetail;
	unsigned long iSize;
	
	HidD_GetHidGuid(&guid);

	DeviceInfo = SetupDiGetClassDevs(&guid, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));

	DeviceInterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

	if (!SetupDiEnumDeviceInterfaces(DeviceInfo, NULL, &guid, device_number, &DeviceInterface))
	{
		SetupDiDestroyDeviceInfoList(DeviceInfo);
		return -1;
	}

	SetupDiGetDeviceInterfaceDetail( DeviceInfo, &DeviceInterface, NULL, 0, &iSize, 0 );

	DeviceDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(iSize);
	DeviceDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

	if (SetupDiGetDeviceInterfaceDetail(DeviceInfo, &DeviceInterface, DeviceDetail, iSize, &iSize, NULL))
	{
		if (!*path && !length)	/* we got no memory passed in, allocate some */
		{						/* WARNING: caller has to free this memory!! */
			*path = (char *)getbytes((short)(strlen(DeviceDetail->DevicePath) * sizeof(char)));
		}
		/* copy path */
		strcpy(*path, DeviceDetail->DevicePath);
	}
	free(DeviceDetail);

	SetupDiDestroyDeviceInfoList(DeviceInfo);
	return EXIT_SUCCESS;
}


/* get capabilities (usage page & usage ID) of an already opened device */
static short _hid_get_capabilities(HANDLE fd, HIDP_CAPS *capabilities)
{
	PHIDP_PREPARSED_DATA	preparsedData;

	if (fd == INVALID_HANDLE_VALUE)
	{
		error("hidio: couldn't get device capabilities due to an invalid handle");
		return -1;
	}

	/* returns and allocates a buffer with device info */
	HidD_GetPreparsedData(fd, &preparsedData);

	/* get capabilities of device from above buffer */
	HidP_GetCaps(preparsedData, capabilities);

	/* no need for PreparsedData any more, so free the memory it's using */
	HidD_FreePreparsedData(preparsedData);
	return EXIT_SUCCESS;
}


/* ============================================================================== */
/* WINDOWS DDK HID SPECIFIC SUPPORT FUNCTIONS */
/* ============================================================================== */

short get_device_number_by_id(unsigned short vendor_id, unsigned short product_id)
{
	return -1;
}

short get_device_number_from_usage(short device_number, 
										unsigned short usage_page, 
										unsigned short usage)
{
	HANDLE fd = INVALID_HANDLE_VALUE;
	HIDP_CAPS capabilities;
	char path[MAX_PATH];
	char *pp = (char *)path;
	short ret, i;
	short device_count = _hid_count_devices();

	for (i = device_number; i < device_count; i++)
	{
		/* get path for specified device number */
		ret = _hid_get_device_path(i, &pp, MAX_PATH);
		if (ret == -1)
		{
			return -1;
		}
		else
		{
			/* open file on the device (read & write, no overlapp) */
			fd = CreateFile(path,
								 GENERIC_READ|GENERIC_WRITE,
								 FILE_SHARE_READ|FILE_SHARE_WRITE,
								 (LPSECURITY_ATTRIBUTES)NULL,
								 OPEN_EXISTING,
								 0,
								 NULL);
			if (fd == INVALID_HANDLE_VALUE)
			{
				return -1;
			}

			/* get the capabilities */
			_hid_get_capabilities(fd, &capabilities);

			/* check whether they match with what we want */
			if (capabilities.UsagePage == usage_page && capabilities.Usage == usage)
			{
				CloseHandle(fd);
				return i;
			}
			CloseHandle(fd);
		}
	}
	return -1;
}


void hidio_build_element_list(t_hidio *x) 
{
	char type_name[256];
	char usage_name[256];

	t_hid_element *current_element;
	t_hid_element *new_element = NULL;
	HIDP_CAPS capabilities;
	short i;

	element_count[x->x_device_number] = 0;
	if (x->x_fd != INVALID_HANDLE_VALUE)
	{
		/* now get device capabilities */
		_hid_get_capabilities(x->x_fd, &capabilities);

		/* for every possible element check what we got */
        for (i = 0; i < capabilities.NumberInputDataIndices; i++) 
        {
#if 0
                for (j = 0; j < capabilities.NumberInputButtonCaps; j++) 
                {
					new_element = getbytes(sizeof(t_hid_element));
                    if ((i == EV_ABS) && (test_bit(j, abs_bitmask)))
                    {
                        /* this means that the bit is set in the axes list */
                        if(ioctl(x->x_fd, EVIOCGABS(j), &abs_features)) 
                            perror("evdev EVIOCGABS ioctl");
                        new_element->min = abs_features.minimum;
                        new_element->max = abs_features.maximum;
                    }
                    else
                    {
                        new_element->min = 0;
                        new_element->max = 0;
                    }
                    if(test_bit(j, element_bitmask[i])) 
                    {
						new_element->linux_type = i; /* the int from linux/input.h */
						new_element->type = gensym(ev[i] ? ev[i] : "?"); /* the symbol */
                        new_element->linux_code = j;
                        if((i == EV_KEY) && (j >= BTN_MISC) && (j < KEY_OK) )
                        {
                            new_element->name = hidio_convert_linux_buttons_to_numbers(j);
                        }
                        else
                        {
                            new_element->name = gensym(event_names[i][j] ? event_names[i][j] : "?");
                        }
                        if( i == EV_REL )
                            new_element->relative = 1;
                        else
                            new_element->relative = 0;

                        /* fill in the t_hid_element struct here */
                        post("x->x_device_number: %d   element_count[]: %d",
                             x->x_device_number, element_count[x->x_device_number]);
                        post("usage_page/usage_id: %d/%d  type/name: %s/%s    max: %d   min: %d ", 
                             new_element->usage_page, new_element->usage_id, 
                             new_element->type->s_name, new_element->name->s_name,
                             new_element->max, new_element->min);
                        post("\tpolled: %d   relative: %d",
                             new_element->polled, new_element->relative);
                        element[x->x_device_number][element_count[x->x_device_number]] = new_element;
                        ++element_count[x->x_device_number];
                    }
                }
            }   
#endif
        }
	}
}

t_int hidio_print_element_list(t_hidio *x)
{
	debug_post(LOG_DEBUG,"hidio_print_element_list");

	return EXIT_SUCCESS;	
}

t_int hidio_print_device_list(t_hidio *x) 
{
	struct _GUID GUID;
	SP_INTERFACE_DEVICE_DATA DeviceInterfaceData;
	struct
	{
		DWORD cbSize;
		char DevicePath[MAX_PATH];
	} FunctionClassDeviceData;
	HIDD_ATTRIBUTES HIDAttributes;
	SECURITY_ATTRIBUTES SecurityAttributes;
	int i;
	HANDLE PnPHandle, HIDHandle;
	ULONG BytesReturned;
	int Success, ManufacturerName, ProductName;
	PWCHAR widestring[MAXPDSTRING];
	char ManufacturerBuffer[MAXPDSTRING];
	char ProductBuffer[MAXPDSTRING];
	const char NotSupplied[] = "NULL";
	DWORD lastError = 0;

	/* Initialize the GUID array and setup the security attributes for Win2000 */
	HidD_GetHidGuid(&GUID);
	SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
	SecurityAttributes.lpSecurityDescriptor = NULL;
	SecurityAttributes.bInheritHandle = FALSE;

	/* Get a handle for the Plug and Play node and request currently active devices */
	PnPHandle = SetupDiGetClassDevs(&GUID, NULL, NULL, 
											  DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);

	if ((int)PnPHandle == -1) 
	{ 
		error("[hidio] ERROR: Could not attach to PnP node");
		return (t_int) GetLastError();
	}

	post("");

	/* Lets look for a maximum of 32 Devices */
	for (i = 0; i < MAX_DEVICES; i++)
	{
		/* Initialize our data */
		DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);
		/* Is there a device at this table entry */
		Success = SetupDiEnumDeviceInterfaces(PnPHandle, NULL, &GUID, i, 
														  &DeviceInterfaceData);
		if (Success)
		{
			/* There is a device here, get it's name */
			FunctionClassDeviceData.cbSize = 5;
			Success = SetupDiGetDeviceInterfaceDetail(PnPHandle, 
					&DeviceInterfaceData,
					(PSP_INTERFACE_DEVICE_DETAIL_DATA)&FunctionClassDeviceData, 
					256, &BytesReturned, NULL);
			if (!Success) 
			{ 
				error("[hidio] ERROR: Could not find the system name for device %d",i); 
				return GetLastError();
			}
			/* Can now open this device */
			HIDHandle = CreateFile(FunctionClassDeviceData.DevicePath, 
										  0, 
										  FILE_SHARE_READ|FILE_SHARE_WRITE, 
										  &SecurityAttributes, OPEN_EXISTING, 0, NULL);
			lastError =  GetLastError();
			if (HIDHandle == INVALID_HANDLE_VALUE) 
			{
				error("[hidio] ERROR: Could not open HID #%d, Errorcode = %d", i, (int)lastError);
				return lastError;
			}
			
			/* Get the information about this HID */
			Success = HidD_GetAttributes(HIDHandle, &HIDAttributes);
			if (!Success) 
			{ 
				error("[hidio] ERROR: Could not get HID attributes"); 
				return GetLastError(); 
			}
			ManufacturerName = HidD_GetManufacturerString(HIDHandle, widestring, MAXPDSTRING);
			wcstombs(ManufacturerBuffer, (const unsigned short *)widestring, MAXPDSTRING);
			ProductName = HidD_GetProductString(HIDHandle, widestring, MAXPDSTRING);
			wcstombs(ProductBuffer, (const unsigned short *)widestring, MAXPDSTRING);

			/* And display it! */
			post("__________________________________________________");
			post("Device %d: '%s' '%s' version %d", i, 
				 ManufacturerName ? ManufacturerBuffer : NotSupplied, ProductName ? ProductBuffer : NotSupplied, 
				 HIDAttributes.VersionNumber);
			post("    vendorID: 0x%04x    productID: 0x%04x",
				 HIDAttributes.VendorID, HIDAttributes.ProductID);

			CloseHandle(HIDHandle);
		} // if (SetupDiEnumDeviceInterfaces . .
	} // for (i = 0; i < 32; i++)
	SetupDiDestroyDeviceInfoList(PnPHandle);

	post("");

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
#ifdef PD
	outlet_symbol( x->x_status_outlet, gensym( device_name ) );
#else
	outlet_anything( x->x_status_outlet, gensym( device_name ),0,NULL );
#endif
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

void hidio_platform_specific_info(t_hidio *x)
{
	//debug_post(LOG_DEBUG,"hidio_platform_specific_info");
}

void hidio_get_events(t_hidio *x)
{
	long bytesRead;

	debug_post(LOG_DEBUG,"hidio_get_events");
#if 0
	while (1)
	{
		if (!ReadFile(current_device->device,
                 current_device->inputReportBuffer,
                 current_device->caps.InputReportByteLength,
                 &bytesRead,
                 NULL))
		{
			debug_error(x,LOG_ERR,"[hidio]: failed to read from device %d", x->x_device_number);
			return;
		}

		if (bytesRead)
		{
			debug_error(x,LOG_INFO,"[hidio]: got event from device %d", x->x_device_number);
		}
		else	/* no more data to read */
		{
			break;
		}
	}
#endif
}


t_int hidio_open_device(t_hidio *x, short device_number)
{
	short ret;
	char path[MAX_PATH];
	char *pp = (char *)path;
	short device_count = -1;

	debug_post(LOG_DEBUG,"hidio_open_device");

	device_count = _hid_count_devices();

	if (device_number > device_count)
	{
		debug_error(x,LOG_ERR,"[hidio]: device %d is not a valid device (%d)\n", device_number, device_count);
		return EXIT_FAILURE;
	}

	/* get path for specified device number */
	ret = _hid_get_device_path(device_number, &pp, MAX_PATH);
	if (ret == -1)
	{
		debug_error(x,LOG_ERR,"[hidio]: could not obtain path for device %d\n", device_number);
		return EXIT_FAILURE;
	}
	else
	{
		/* open file on the device (read & write, no overlapp) */
		x->x_fd = CreateFile(path,
							 GENERIC_READ|GENERIC_WRITE,
							 FILE_SHARE_READ|FILE_SHARE_WRITE,
							 (LPSECURITY_ATTRIBUTES)NULL,
							 OPEN_EXISTING,
							 FILE_FLAG_OVERLAPPED,
							 NULL);
		if (x->x_fd == INVALID_HANDLE_VALUE)
		{
			debug_error(x,LOG_ERR,"[hidio]: failed to open device %d at %s\n", device_number, path);
			return EXIT_FAILURE;
		}
		/* LATER get the real device name here instead of displaying the path */
		post ("[hidio] opened device %d: %s", device_number, path);

		post("pre hidio_build_element_list");
		hidio_build_element_list(x);
	}
	return EXIT_SUCCESS;
}


t_int hidio_close_device(t_hidio *x)
{
	t_int result = 0;

	debug_post(LOG_DEBUG, "hidio_close_device");
	
	if (x->x_device_number > -1)
	{
		if (x->x_fd != INVALID_HANDLE_VALUE)
		{
			CloseHandle(x->x_fd);
			x->x_fd = INVALID_HANDLE_VALUE;
		}
	}

	return (result);
}


void hidio_build_device_list(void)
{
	debug_post(LOG_DEBUG,"hidio_build_device_list");
}


void hidio_print(t_hidio *x)
{
	hidio_print_device_list(x);
	
	if (x->x_device_open) 
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






#endif  /* _WINDOWS */
