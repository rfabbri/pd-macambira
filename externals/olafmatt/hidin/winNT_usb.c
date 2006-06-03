/*
  hidin USB HID support stuff for Windows 2000 / XP

  Written by Olaf Matthes <olaf.matthes@gmx.de>

  This file contains the implementation for connecting to USB HID
  (Human Interface Device) devices. It provides several functions
  to open devices and to query information and capabilities. 

*/

#ifdef PD
#include "m_pd.h"	/* we need this because we want to print() to the PD console */
#include <stdio.h>
#else
#include "ext.h"	/* we need this because we want to print() to the Max window */
#endif
#include "hidin.h"


/*
 *  get information for a given device (HANDLE)
 *
 */

void getDeviceInfo(HANDLE deviceHandle)
{
    HIDD_ATTRIBUTES deviceAttributes;
	PWCHAR deviceName;
	ULONG length = 256;

	if(deviceHandle == INVALID_HANDLE_VALUE)
	{
		post("hidin: -- couldn't get device info due to an invalid handle");
		return;
	}

	if (!HidD_GetAttributes (deviceHandle, &deviceAttributes))
	{
		post("hidin: -- failed to get attributes");
		return;
	}
	else
	{
		// post("hidin: ** VendorID: 0x%x", deviceAttributes.VendorID);
		// post("hidin: ** ProductID: 0x%x", deviceAttributes.ProductID);
		// post("hidin: ** VersionNumber: 0x%x", deviceAttributes.VersionNumber);
	}

	deviceName = (PWCHAR)getbytes((short)(length));
 	if(!HidD_GetProductString (deviceHandle, deviceName, length))
	{
		freebytes(deviceName, (short)(length));
		return;
	}
	else
	{
		char name[256];
		int i = 0;
		wcstombs(name, deviceName, length);
		post("hidin: >> opening device: \"%s\"", name);
		freebytes(deviceName, (short)(length));
	}
    return;
}

/*
 *  find name of attached HID devices
 */
t_symbol *findDeviceName(DWORD deviceID)
{
	HANDLE deviceHandle;
	PWCHAR deviceName;
	ULONG length = 256;
	char name[256];	// this will be the return value
	int i = 0;

	deviceHandle = connectDeviceNumber(deviceID);

	if(deviceHandle != INVALID_HANDLE_VALUE)
	{
		deviceName = (PWCHAR)getbytes((short)(length));
 		if(!HidD_GetProductString (deviceHandle, deviceName, length))
		{
			freebytes(deviceName, (short)(length));
			sprintf(name, "Unknown (Device #%d)", deviceID + 1);
			return gensym(name);
		}
		else
		{
			wcstombs(name, deviceName, length);
			freebytes(deviceName, (short)(length));
		}

		CloseHandle(deviceHandle);

	    return gensym(name);
	}
	return gensym("Unsupported Device");
}

/*
 *  find number of attached HID devices
 */
int findHidDevices()
{
    HDEVINFO                            hardwareDeviceInfo;
    SP_INTERFACE_DEVICE_DATA            deviceInfoData;
    ULONG                               i;
    BOOLEAN                             done;
    GUID                                hidGuid;
    PSP_INTERFACE_DEVICE_DETAIL_DATA    functionClassDeviceData = NULL;
    ULONG                               predictedLength = 0;
    ULONG                               requiredLength = 0;
    ULONG								NumberDevices = 0;

    HidD_GetHidGuid (&hidGuid);

    //
    // Open a handle to the plug and play dev node.
    //
    hardwareDeviceInfo = SetupDiGetClassDevs ( &hidGuid,
                                               NULL, // Define no enumerator (global)
                                               NULL, // Define no
                                               (DIGCF_PRESENT | // Only Devices present
                                                DIGCF_DEVICEINTERFACE)); // Function class devices.

    //
    // Take a wild guess to start
    //
    
    NumberDevices = 4;
    done = FALSE;
    deviceInfoData.cbSize = sizeof (SP_INTERFACE_DEVICE_DATA);

    i=0;
    while (!done) 
    {
        NumberDevices *= 2;

        for (; i < NumberDevices; i++) 
        {
            if (SetupDiEnumDeviceInterfaces (hardwareDeviceInfo,
                                             0, // No care about specific PDOs
                                             &hidGuid,
                                             i,
                                             &deviceInfoData))
            {
                //
                // allocate a function class device data structure to receive the
                // goods about this particular device.
                //

                SetupDiGetDeviceInterfaceDetail (
                        hardwareDeviceInfo,
                        &deviceInfoData,
                        NULL, // probing so no output buffer yet
                        0, // probing so output buffer length of zero
                        &requiredLength,
                        NULL); // not interested in the specific dev-node


                predictedLength = requiredLength;

                functionClassDeviceData = malloc (predictedLength);
                if (functionClassDeviceData)
                {
                    functionClassDeviceData->cbSize = sizeof (SP_INTERFACE_DEVICE_DETAIL_DATA);
                }
                else
                {
                    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
                    return 0;
                }

                //
                // Retrieve the information from Plug and Play.
                //

                if (! SetupDiGetDeviceInterfaceDetail (
                           hardwareDeviceInfo,
                           &deviceInfoData,
                           functionClassDeviceData,
                           predictedLength,
                           &requiredLength,
                           NULL)) 
                {
                    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);
                    return 0;
                }
				//
				// get name of device
				//
				// HidDevices[i] = getbytes(1024 * sizeof(char));
            } 
            else
            {
                if (ERROR_NO_MORE_ITEMS == GetLastError()) 
                {
                    done = TRUE;
                    break;
                }
            }
        }
    }

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfo);

    return (i);		// return number of devices
}


/*
 *  connect to Ith USB device (count starting with 0)
 */

HANDLE connectDeviceNumber(DWORD deviceIndex)
{
    GUID hidGUID;
    HDEVINFO hardwareDeviceInfoSet;
    SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
    PSP_INTERFACE_DEVICE_DETAIL_DATA deviceDetail;
    ULONG requiredSize;
    HANDLE deviceHandle = INVALID_HANDLE_VALUE;
    DWORD result;

    //Get the HID GUID value - used as mask to get list of devices
    HidD_GetHidGuid (&hidGUID);

    //Get a list of devices matching the criteria (hid interface, present)
    hardwareDeviceInfoSet = SetupDiGetClassDevs (&hidGUID,
                                                 NULL, // Define no enumerator (global)
                                                 NULL, // Define no
                                                 (DIGCF_PRESENT | // Only Devices present
                                                 DIGCF_DEVICEINTERFACE)); // Function class devices.

    deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    //Go through the list and get the interface data
    result = SetupDiEnumDeviceInterfaces (hardwareDeviceInfoSet,
                                          NULL, //infoData,
                                          &hidGUID, //interfaceClassGuid,
                                          deviceIndex, 
                                          &deviceInterfaceData);

    /* Failed to get a device - possibly the index is larger than the number of devices */
    if (result == FALSE)
    {
        SetupDiDestroyDeviceInfoList (hardwareDeviceInfoSet);
		post("hidin: -- failed to get specified device number");
        return INVALID_HANDLE_VALUE;
    }

    //Get the details with null values to get the required size of the buffer
    SetupDiGetDeviceInterfaceDetail (hardwareDeviceInfoSet,
                                     &deviceInterfaceData,
                                     NULL, //interfaceDetail,
                                     0, //interfaceDetailSize,
                                     &requiredSize,
                                     0); //infoData))

    //Allocate the buffer
    deviceDetail = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(requiredSize);
    deviceDetail->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

    //Fill the buffer with the device details
    if (!SetupDiGetDeviceInterfaceDetail (hardwareDeviceInfoSet,
                                          &deviceInterfaceData,
                                          deviceDetail,
                                          requiredSize,
                                          &requiredSize,
                                          NULL)) 
    {
        SetupDiDestroyDeviceInfoList (hardwareDeviceInfoSet);
        free (deviceDetail);
		post("hidin: -- failed to get device info");
        return INVALID_HANDLE_VALUE;
    }

#if 1
    //Open file on the device (read only)
	deviceHandle = CreateFile 
					(deviceDetail->DevicePath,
					GENERIC_READ,
					FILE_SHARE_READ|FILE_SHARE_WRITE,
					(LPSECURITY_ATTRIBUTES)NULL,
					OPEN_EXISTING,
					FILE_FLAG_OVERLAPPED,
					NULL);
#else
    //Open file on the device (read & write)
	deviceHandle = CreateFile 
					(deviceDetail->DevicePath, 
					GENERIC_READ|GENERIC_WRITE, 
					FILE_SHARE_READ|FILE_SHARE_WRITE, 
					(LPSECURITY_ATTRIBUTES)NULL,
					OPEN_EXISTING, 
					FILE_FLAG_OVERLAPPED, // was 0
					NULL);
#endif

	if(deviceHandle == INVALID_HANDLE_VALUE)
	{
		int err = GetLastError();
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
		post("hidin: -- could not get device #%i: %s", deviceIndex + 1, (LPCTSTR)lpMsgBuf);
		if(err == ERROR_ACCESS_DENIED)
			post("hidin: -- can not read from mouse and keyboard");
		LocalFree(lpMsgBuf);
 	}

    SetupDiDestroyDeviceInfoList (hardwareDeviceInfoSet);
    free (deviceDetail);
    return deviceHandle;
}

/*
 *  connect to USB device specified through VendorID, ProductID and VersionNumber
 *
 */

HANDLE connectDeviceName(DWORD *vendorID, DWORD *productID, DWORD *versionNumber)
{
    HANDLE deviceHandle = INVALID_HANDLE_VALUE;
    DWORD index = 0;
    HIDD_ATTRIBUTES deviceAttributes;
    BOOL matched = FALSE;

    while (!matched && (deviceHandle = connectDeviceNumber(index)) != INVALID_HANDLE_VALUE)
    {
        if (!HidD_GetAttributes (deviceHandle, &deviceAttributes))
            return INVALID_HANDLE_VALUE;

        if ((vendorID == 0 || deviceAttributes.VendorID == *vendorID) &&
            (productID == 0 || deviceAttributes.ProductID == *productID) &&
            (versionNumber == 0 || deviceAttributes.VersionNumber == *versionNumber))
            return deviceHandle; /* matched */
        
        CloseHandle (deviceHandle); /* not a match - close and try again */

        index++;
    }

    return INVALID_HANDLE_VALUE;
}


/*
 *  get device capabilities
 *
 */
void getDeviceCapabilities(t_hid_device *hid)
{
	// Get the Capabilities structure for the device.
	PHIDP_PREPARSED_DATA	preparsedData;
	HIDP_CAPS		        capabilities;

	if(hid->device == INVALID_HANDLE_VALUE)
	{
		post("hidin: -- couldn't get device capabilities due to an invalid handle");
		return;
	}

	/*
		API function: HidD_GetPreparsedData
		Returns: a pointer to a buffer containing the information about the device's capabilities.
		Requires: A handle returned by CreateFile.
		There's no need to access the buffer directly,
		but HidP_GetCaps and other API functions require a pointer to the buffer.
	*/

	HidD_GetPreparsedData(hid->device, &preparsedData);

	/* get the device attributes */
    // HidD_GetAttributes (hid->device, &hid->attributes);

	/*
		API function: HidP_GetCaps
		Learn the device's capabilities.
		For standard devices such as joysticks, you can find out the specific
		capabilities of the device.
		For a custom device, the software will probably know what the device is capable of,
		and the call only verifies the information.
		Requires: the pointer to the buffer returned by HidD_GetPreparsedData.
		Returns: a Capabilities structure containing the information.
	*/
	
	HidP_GetCaps(preparsedData, &capabilities);

	// Display the capabilities

	hid->caps = capabilities;
	hid->ppd = preparsedData;
	// No need for PreparsedData any more, so free the memory it's using.
	// HidD_FreePreparsedData(preparsedData);
}

/*
 *  Given a struct _hid_device, obtain a read report and unpack the values
 *  into the InputData array.
 */
int readHid(t_hid_device *hidDevice)
{
	long value = 0, ret;
	long bytesRead;

    if(!ReadFile(hidDevice->device,
                 hidDevice->inputReportBuffer,
                 hidDevice->caps.InputReportByteLength,
                 &bytesRead,
                 NULL)) 
    {
		int err = GetLastError();
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL);
		post("hidin: -- could not read from device: %s", (LPCTSTR)lpMsgBuf);
		LocalFree(lpMsgBuf);
        return 0;
    }
	// we need: bytesRead == hidDevice->caps.InputReportByteLength

    return bytesRead;
}

/*
 *  Given a struct _hid_device, obtain a read report and unpack the values
 *  into the InputData array.
 */
int readHidOverlapped(t_hid_device *hidDevice)
{
	long value = 0, ret;
	long bytesRead;

    if(!ReadFile(hidDevice->device,
                 hidDevice->inputReportBuffer,
                 hidDevice->caps.InputReportByteLength,
                 &bytesRead,
                 (LPOVERLAPPED) &hidDevice->overlapped)) 
    {
        return 0;
    }
    return bytesRead;
}



BOOLEAN getFeature (t_hid_device *HidDevice)
/*++
RoutineDescription:
   Given a struct _HID_DEVICE, fill in the feature data structures with
   all features on the device.  May issue multiple HidD_GetFeature() calls to
   deal with multiple report IDs.
--*/
{
    ULONG     Index;
    t_hid_data *pData;
    BOOLEAN   FeatureStatus;
    BOOLEAN   Status;

    /*
    // As with writing data, the IsDataSet value in all the structures should be
    //    set to FALSE to indicate that the value has yet to have been set
    */

    pData = HidDevice->featureData;

    for (Index = 0; Index < HidDevice->featureDataLength; Index++, pData++) 
    {
        pData->IsDataSet = FALSE;
    }

    /*
    // Next, each structure in the HID_DATA buffer is filled in with a value
    //   that is retrieved from one or more calls to HidD_GetFeature.  The 
    //   number of calls is equal to the number of reportIDs on the device
    */

    Status = TRUE; 
    pData = HidDevice->featureData;

    for (Index = 0; Index < HidDevice->featureDataLength; Index++, pData++) 
    {
        /*
        // If a value has yet to have been set for this structure, build a report
        //    buffer with its report ID as the first byte of the buffer and pass
        //    it in the HidD_GetFeature call.  Specifying the report ID in the
        //    first specifies which report is actually retrieved from the device.
        //    The rest of the buffer should be zeroed before the call
        */

        if (!pData->IsDataSet) 
        {
            memset(HidDevice->featureReportBuffer, 0x00, HidDevice->caps.FeatureReportByteLength);

            HidDevice->featureReportBuffer[0] = (UCHAR) pData -> ReportID;

            FeatureStatus = HidD_GetFeature (HidDevice->device,
                                              HidDevice->featureReportBuffer,
                                              HidDevice->caps.FeatureReportByteLength);

            /*
            // If the return value is TRUE, scan through the rest of the HID_DATA
            //    structures and fill whatever values we can from this report
            */


            if (FeatureStatus) 
            {
                FeatureStatus = unpackReport ( HidDevice->featureReportBuffer,
                                           HidDevice->caps.FeatureReportByteLength,
                                           HidP_Feature,
                                           HidDevice->featureData,
                                           HidDevice->featureDataLength,
                                           HidDevice->ppd);
            }

            Status = Status && FeatureStatus;
        }
   }

   return (Status);
}


BOOLEAN
unpackReport (
   PCHAR                ReportBuffer,
   USHORT               ReportBufferLength,
   HIDP_REPORT_TYPE     ReportType,
   t_hid_data           *Data,
   ULONG                DataLength,
   PHIDP_PREPARSED_DATA Ppd
)
/*++
Routine Description:
   Given ReportBuffer representing a report from a HID device where the first
   byte of the buffer is the report ID for the report, extract all the HID_DATA
   in the Data list from the given report.
--*/
{
    ULONG       numUsages; // Number of usages returned from GetUsages.
    ULONG       i;
    UCHAR       reportID;
    ULONG       Index;
    ULONG       nextUsage;

    reportID = ReportBuffer[0];

    for (i = 0; i < DataLength; i++, Data++) 
    {
        if (reportID == Data->ReportID) 
        {
            if (Data->IsButtonData) 
            {
                numUsages = Data->ButtonData.MaxUsageLength;

                Data->Status = HidP_GetUsages (ReportType,
                                               Data->UsagePage,
                                               0, // All collections
                                               Data->ButtonData.Usages,
                                               &numUsages,
                                               Ppd,
                                               ReportBuffer,
                                               ReportBufferLength);


                //
                // Get usages writes the list of usages into the buffer
                // Data->ButtonData.Usages newUsage is set to the number of usages
                // written into this array.
                // A usage cannot not be defined as zero, so we'll mark a zero
                // following the list of usages to indicate the end of the list of
                // usages
                //
                // NOTE: One anomaly of the GetUsages function is the lack of ability
                //        to distinguish the data for one ButtonCaps from another
                //        if two different caps structures have the same UsagePage
                //        For instance:
                //          Caps1 has UsagePage 07 and UsageRange of 0x00 - 0x167
                //          Caps2 has UsagePage 07 and UsageRange of 0xe0 - 0xe7
                //
                //        However, calling GetUsages for each of the data structs
                //          will return the same list of usages.  It is the 
                //          responsibility of the caller to set in the HID_DEVICE
                //          structure which usages actually are valid for the
                //          that structure. 
                //      

                /*
                // Search through the usage list and remove those that 
                //    correspond to usages outside the define ranged for this
                //    data structure.
                */
                
                for (Index = 0, nextUsage = 0; Index < numUsages; Index++) 
                {
                    if (Data->ButtonData.UsageMin <= Data->ButtonData.Usages[Index] &&
                            Data -> ButtonData.Usages[Index] <= Data->ButtonData.UsageMax) 
                    {
                        Data->ButtonData.Usages[nextUsage++] = Data->ButtonData.Usages[Index];
                        
                    }
                }

                if (nextUsage < Data -> ButtonData.MaxUsageLength) 
                {
                    Data->ButtonData.Usages[nextUsage] = 0;
                }
            }
            else 
            {
                Data->Status = HidP_GetUsageValue (
                                                ReportType,
                                                Data->UsagePage,
                                                0,               // All Collections.
                                                Data->ValueData.Usage,
                                                &Data->ValueData.Value,
                                                Ppd,
                                                ReportBuffer,
                                                ReportBufferLength);

                Data->Status = HidP_GetScaledUsageValue (
                                                       ReportType,
                                                       Data->UsagePage,
                                                       0, // All Collections.
                                                       Data->ValueData.Usage,
                                                       &Data->ValueData.ScaledValue,
                                                       Ppd,
                                                       ReportBuffer,
                                                       ReportBufferLength);
            } 
            Data->IsDataSet = TRUE;
        }
    }
    return (TRUE);
}


BOOLEAN
packReport (
   PCHAR                ReportBuffer,
   USHORT               ReportBufferLength,
   HIDP_REPORT_TYPE     ReportType,
   t_hid_data           *Data,
   ULONG                DataLength,
   PHIDP_PREPARSED_DATA Ppd
   )
/*++
Routine Description:
   This routine takes in a list of HID_DATA structures (DATA) and builds 
      in ReportBuffer the given report for all data values in the list that 
      correspond to the report ID of the first item in the list.  

   For every data structure in the list that has the same report ID as the first
      item in the list will be set in the report.  Every data item that is 
      set will also have it's IsDataSet field marked with TRUE.

   A return value of FALSE indicates an unexpected error occurred when setting
      a given data value.  The caller should expect that assume that no values
      within the given data structure were set.

   A return value of TRUE indicates that all data values for the given report
      ID were set without error.
--*/
{
    ULONG       numUsages; // Number of usages to set for a given report.
    ULONG       i;
    ULONG       CurrReportID;

    /*
    // All report buffers that are initially sent need to be zero'd out
    */

    memset (ReportBuffer, (UCHAR) 0, ReportBufferLength);

    /*
    // Go through the data structures and set all the values that correspond to
    //   the CurrReportID which is obtained from the first data structure 
    //   in the list
    */

    CurrReportID = Data -> ReportID;

    for (i = 0; i < DataLength; i++, Data++) 
    {
        /*
        // There are two different ways to determine if we set the current data
        //    structure: 
        //    1) Store the report ID were using and only attempt to set those
        //        data structures that correspond to the given report ID.  This
        //        example shows this implementation.
        //
        //    2) Attempt to set all of the data structures and look for the 
        //        returned status value of HIDP_STATUS_INVALID_REPORT_ID.  This 
        //        error code indicates that the given usage exists but has a 
        //        different report ID than the report ID in the current report 
        //        buffer
        */

        if (Data -> ReportID == CurrReportID) 
        {
            if (Data->IsButtonData) 
            {
                numUsages = Data->ButtonData.MaxUsageLength;
                Data->Status = HidP_SetUsages (ReportType,
                                               Data->UsagePage,
                                               0, // All collections
                                               Data->ButtonData.Usages,
                                               &numUsages,
                                               Ppd,
                                               ReportBuffer,
                                               ReportBufferLength);
            }
            else
            {
                Data->Status = HidP_SetUsageValue (ReportType,
                                                   Data->UsagePage,
                                                   0, // All Collections.
                                                   Data->ValueData.Usage,
                                                   Data->ValueData.Value,
                                                   Ppd,
                                                   ReportBuffer,
                                                   ReportBufferLength);
            }

            if (HIDP_STATUS_SUCCESS != Data->Status)
            {
                return FALSE;
            }
        }
    }   

    /*
    // At this point, all data structures that have the same ReportID as the
    //    first one will have been set in the given report.  Time to loop 
    //    through the structure again and mark all of those data structures as
    //    having been set.
    */

    for (i = 0; i < DataLength; i++, Data++) 
    {
        if (CurrReportID == Data -> ReportID)
        {
            Data -> IsDataSet = TRUE;
        }
    }
    return (TRUE);
}

