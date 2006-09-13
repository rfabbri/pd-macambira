/*

  hidin.h - headers and declarationd for the 'hidin' external

*/

#ifndef hidin_H
#define hidin_H

#include <windows.h>
#include <basetyps.h>
#include <stdlib.h>
#include <wtypes.h>
#include <setupapi.h>
#ifdef _MSC_VER
#include "hidusage.h"
#include "hidsdi.h"
#else
#include <ddk/hidusage.h>
#include <ddk/hidsdi.h>
#endif

//
// A structure to hold the steady state data received from the hid device.
// Each time a read packet is received we fill in this structure.
// Each time we wish to write to a hid device we fill in this structure.
// This structure is here only for convenience.  Most real applications will
// have a more efficient way of moving the hid data to the read, write, and
// feature routines.
//
typedef struct _hid_data
{
   BOOLEAN     IsButtonData;
   UCHAR       Reserved;
   USAGE       UsagePage;   // The usage page for which we are looking.
   ULONG       Status;      // The last status returned from the accessor function
                            // when updating this field.
   ULONG       ReportID;    // ReportID for this given data structure
   BOOLEAN     IsDataSet;   // Variable to track whether a given data structure
                            //  has already been added to a report structure

   union {
      struct {
         ULONG       UsageMin;       // Variables to track the usage minimum and max
         ULONG       UsageMax;       // If equal, then only a single usage
         ULONG       MaxUsageLength; // Usages buffer length.
         PUSAGE      Usages;         // list of usages (buttons ``down'' on the device.

      } ButtonData;
      struct {
         USAGE       Usage; // The usage describing this value;
         USHORT      Reserved;

         ULONG       Value;
         LONG        ScaledValue;
      } ValueData;
   };
} t_hid_data;

typedef struct _hid_device
{   
    PCHAR                devicePath;
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
    ULONG                inputDataLength; // Num elements in this array.
    PHIDP_BUTTON_CAPS    inputButtonCaps;
    PHIDP_VALUE_CAPS     inputValueCaps;

    char                 *outputReportBuffer;
    t_hid_data           *outputData;
    ULONG                outputDataLength;
    PHIDP_BUTTON_CAPS    outputButtonCaps;
    PHIDP_VALUE_CAPS     outputValueCaps;

    char                 *featureReportBuffer;
    t_hid_data           *featureData;
    ULONG                featureDataLength;
    PHIDP_BUTTON_CAPS    featureButtonCaps;
    PHIDP_VALUE_CAPS     featureValueCaps;
} t_hid_device;


/*
 *  displays the vendor and product ID and the version 
 *  number for the given device handle
 */
void getDeviceInfo(HANDLE deviceHandle);

/*
 *  find number of attached HID devices
 */
int findHidDevices();

/*
 *  find name of attached HID devices
 */
t_symbol *findDeviceName(DWORD deviceID);

/*
 *  connects to the hid device specified through a number
 *  returns a handle to the device (x->x_hid.device)
 */
HANDLE connectDeviceNumber(DWORD i);

/* Connects to the USB HID described by the combination of vendor id, product id
   If the attribute is null, it will connect to first device satisfying the remaining
   attributes. */
HANDLE connectDeviceName(DWORD *vendorID, DWORD *productID, DWORD *versionNumber);

/*
 *  get hid device capabilities (and display them)
 *  also instantiates the x->x_hid.caps field and
 *  allocates the memory we'll need to use this device
 */
void getDeviceCapabilities(t_hid_device *hid);



/*
 *  read input data from hid device and display them
 */
int readHid(t_hid_device *hidDevice);

int readHidOverlapped(t_hid_device *hidDevice);

BOOLEAN getFeature(t_hid_device *HidDevice);

BOOLEAN unpackReport(PCHAR ReportBuffer, USHORT ReportBufferLength, HIDP_REPORT_TYPE ReportType,
                     t_hid_data *Data, ULONG DataLength, PHIDP_PREPARSED_DATA Ppd);

BOOLEAN packReport(PCHAR ReportBuffer, USHORT ReportBufferLength, HIDP_REPORT_TYPE ReportType,
                   t_hid_data *Data, ULONG DataLength, PHIDP_PREPARSED_DATA Ppd);

#endif // hidin_H
