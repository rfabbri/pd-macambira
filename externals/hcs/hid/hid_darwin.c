#ifdef __APPLE__
/*
 *  Apple Darwin HID Manager support for Pd [hid] object
 *
 *  based on SC_HID.cpp from SuperCollider3 by Jan Truetzschler v. Falkenstein
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

/*
#include <IOKit/IOKitLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <IOKit/hid/IOHIDLib.h>
#include <IOKit/hid/IOHIDKeys.h>
*/
#include <IOKit/hid/IOHIDUsageTables.h>

#include <mach/mach.h>
#include <mach/mach_error.h>

#include "hid.h"

//#define DEBUG(x)
#define DEBUG(x) x 

/*==============================================================================
 *  GLOBAL VARS
 *======================================================================== */

/* count of total number of devices found */
int gNumberOfHIDDevices = 0;

/* 
 *	an array of discovered devices, which is used for selecting the
 * current device, by #, type, or name
 */
pRecDevice discoveredDevices[256];

/* timer for element data updates */
EventLoopTimerRef gTimer = NULL; 

/*==============================================================================
 *  FUNCTIONS
 *==============================================================================
 */

void releaseHIDDevices (void)
{
	DEBUG(post("releaseHIDDevices"););

	if (gTimer)
    {    
		RemoveEventLoopTimer(gTimer);
		gTimer = NULL;
	}
	HIDReleaseAllDeviceQueues();
	HIDReleaseDeviceList();
	gNumberOfHIDDevices = 0;
}

int prHIDBuildElementList(void)
{
	DEBUG(post("prHIDBuildElementList"););

	int locID = NULL;
	int cookieNum = NULL;
	UInt32 i;
	pRecElement	devElement;
	pRecDevice pCurrentHIDDevice;
	UInt32 numElements;
	char cstrElementName [256];

// Apple Trackpad locID for testing
	locID = 50397184;

	// look for the right device using locID 
	pCurrentHIDDevice = HIDGetFirstDevice ();
	while (pCurrentHIDDevice && (pCurrentHIDDevice->locID !=locID))
		pCurrentHIDDevice = HIDGetNextDevice (pCurrentHIDDevice);
	if(!pCurrentHIDDevice) return (1);
	
	devElement = HIDGetFirstDeviceElement(pCurrentHIDDevice, kHIDElementTypeInput);
	numElements = HIDCountDeviceElements(pCurrentHIDDevice, kHIDElementTypeInput);
	
	post("[hid] found %d elements",numElements);
	
	for(i=0; i<numElements; i++)
	{
		//type
		HIDGetTypeName((IOHIDElementType) devElement->type, cstrElementName);
		post("Type: %s  %d  0x%x",cstrElementName,devElement->type,devElement->type);
		//usage
		HIDGetUsageName (devElement->usagePage, devElement->usage, cstrElementName);
		post("Usage: %s  %d  0x%x",cstrElementName,devElement->usage,devElement->usage);
		
			//devstring = newPyrString(g->gc, cstrElementName, 0, true);
			//SetObject(devElementArray->slots+devElementArray->size++, devstring);
			//g->gc->GCWrite(devElementArray, (PyrObject*) devstring);
		//cookie
		post("Cookie: %d %d %d",devElement->cookie,devElement->min,devElement->max);
		
		devElement =  HIDGetNextDeviceElement (devElement, kHIDElementTypeInput);
	}
	return (0);	
}

int prHIDBuildDeviceList(void)
{
	DEBUG(post("prHIDBuildDeviceList"););

	int i,err;
	UInt32 usagePage, usage;
	pRecElement devElement;
	pRecDevice pCurrentHIDDevice;

	//pass in usage & usagepage
	//kHIDUsage_GD_Joystick kHIDUsage_GD_GamePad
	usagePage = kHIDPage_GenericDesktop;
	usage = NULL;

	Boolean result = HIDBuildDeviceList (usagePage, usage); 
	// returns false if no device found

	if(result) error("[hid]: no HID devices found\n");
	
	int numdevs = HIDCountDevices();
	gNumberOfHIDDevices = numdevs;
	// exit if no devices found
	if(!numdevs) return (0);

	post("number of devices: %d", numdevs);
	char cstrDeviceName [256];
	
	pCurrentHIDDevice = HIDGetFirstDevice();
	for(i=0; i<numdevs; i++)
	{
		post("'%s' '%s' version %d",
			  pCurrentHIDDevice->manufacturer,pCurrentHIDDevice->product,pCurrentHIDDevice->version);
		//usage
		HIDGetUsageName (pCurrentHIDDevice->usagePage, pCurrentHIDDevice->usage, cstrDeviceName);
		post("vendorID: %d   productID: %d   locID: %d",
			  pCurrentHIDDevice->vendorID,pCurrentHIDDevice->productID,pCurrentHIDDevice->locID);
		pCurrentHIDDevice = HIDGetNextDevice (pCurrentHIDDevice);
	}

	UInt32 outnum = HIDCountDeviceElements (pCurrentHIDDevice, kHIDElementTypeOutput);
	post("number of outputs: %d \n", outnum);

	return (0);	
}


int prHIDGetValue(void)
{
	DEBUG(post("prHIDGetValue"););

	int locID = NULL; 
	int cookieNum = NULL;
	SInt32 value;
	/*
	PyrSlot *a = g->sp - 2; //class
	PyrSlot *b = g->sp - 1; //locID device
	PyrSlot *c = g->sp; //element cookie
	int err = slotIntVal(b, &locID);
	if (err) return err;
	err = slotIntVal(c, &cookieNum);
	if (err) return err;
	*/
	IOHIDElementCookie cookie = (IOHIDElementCookie) cookieNum;
	// look for the right device: 
	pRecDevice  pCurrentHIDDevice = HIDGetFirstDevice ();
	while (pCurrentHIDDevice && (pCurrentHIDDevice->locID !=locID))
		pCurrentHIDDevice = HIDGetNextDevice (pCurrentHIDDevice);
	if(!pCurrentHIDDevice) return (1);
	// look for the right element:
	pRecElement pCurrentHIDElement =  HIDGetFirstDeviceElement (pCurrentHIDDevice, kHIDElementTypeIO);
	// use gElementCookie to find current element
	while (pCurrentHIDElement && (pCurrentHIDElement->cookie != cookie))
		pCurrentHIDElement = HIDGetNextDeviceElement (pCurrentHIDElement, kHIDElementTypeIO);
	
	if (pCurrentHIDElement)
	{
		value = HIDGetElementValue (pCurrentHIDDevice, pCurrentHIDElement);
		// if it's not a button and it's not a hatswitch then calibrate
		if(( pCurrentHIDElement->type != kIOHIDElementTypeInput_Button ) &&
			( pCurrentHIDElement->usagePage == 0x01 && pCurrentHIDElement->usage != kHIDUsage_GD_Hatswitch)) 
			value = HIDCalibrateValue ( value, pCurrentHIDElement );
		//SetInt(a, value);
	}
	//else SetNil(a);
	return (0);	
	
}


void PushQueueEvents_RawValue(void)
{
	DEBUG(post("PushQueueEvents_RawValue"););

	int i;
	
	IOHIDEventStruct event;
	pRecDevice  pCurrentHIDDevice = HIDGetFirstDevice ();
	int numdevs = gNumberOfHIDDevices;
	unsigned char result;
	for(i=0; i< numdevs; i++)
	{
		result = HIDGetEvent(pCurrentHIDDevice, (void*) &event);
		if(result) 
		{
			SInt32 value = event.value;
			int vendorID = pCurrentHIDDevice->vendorID;
			int productID = pCurrentHIDDevice->productID;			
			int locID = pCurrentHIDDevice->locID;
			IOHIDElementCookie cookie = (IOHIDElementCookie) event.elementCookie;
			//set arguments: 
//			++g->sp;SetInt(g->sp, vendorID); 
//			++g->sp;SetInt(g->sp, productID); 			
//			++g->sp;SetInt(g->sp, locID); 
//			++g->sp;SetInt(g->sp, (int) cookie); 
//			++g->sp;SetInt(g->sp, value); 
		}
		pCurrentHIDDevice = HIDGetNextDevice(pCurrentHIDDevice);
	}
}


void PushQueueEvents_CalibratedValue(void)
{
	DEBUG(post("PushQueueEvents_CalibratedValue"););

	int i;
	
	IOHIDEventStruct event;
	pRecDevice  pCurrentHIDDevice = HIDGetFirstDevice ();
	
	int numdevs = gNumberOfHIDDevices;
	unsigned char result;
	for(i=0; i< numdevs; i++)
	{

		result = HIDGetEvent(pCurrentHIDDevice, (void*) &event);
		if(result) 
		{
			SInt32 value = event.value;
			int vendorID = pCurrentHIDDevice->vendorID;
			int productID = pCurrentHIDDevice->productID;			
			int locID = pCurrentHIDDevice->locID;
			IOHIDElementCookie cookie = (IOHIDElementCookie) event.elementCookie;
			pRecElement pCurrentHIDElement =  HIDGetFirstDeviceElement (pCurrentHIDDevice, kHIDElementTypeIO);
	// use gElementCookie to find current element
			while (pCurrentHIDElement && ( (pCurrentHIDElement->cookie) != cookie))
				pCurrentHIDElement = HIDGetNextDeviceElement (pCurrentHIDElement, kHIDElementTypeIO);
		
			if (pCurrentHIDElement)
			{
			value = HIDCalibrateValue(value, pCurrentHIDElement);
			//find element to calibrate
			//set arguments: 
//			++g->sp;SetInt(g->sp, vendorID); 
//			++g->sp;SetInt(g->sp, productID); 			
//			++g->sp;SetInt(g->sp, locID); 
//			++g->sp;SetInt(g->sp, (int) cookie); 
//			++g->sp;SetInt(g->sp, value); 
			}
		}
		pCurrentHIDDevice = HIDGetNextDevice(pCurrentHIDDevice);
	}
}


static pascal void IdleTimer(EventLoopTimerRef inTimer, void* userData)
{
	DEBUG(post("IdleTimer"););

	#pragma unused (inTimer, userData)
	PushQueueEvents_CalibratedValue ();
}


int prHIDReleaseDeviceList(void)
{
	DEBUG(post("prHIDReleaseDeviceList"););

	releaseHIDDevices();
	return (0);	
}


static EventLoopTimerUPP GetTimerUPP(void)
{
	DEBUG(post("GetTimerUPP"););

	static EventLoopTimerUPP sTimerUPP = NULL;
	
	if (sTimerUPP == NULL)
		sTimerUPP = NewEventLoopTimerUPP(IdleTimer);
	
	return sTimerUPP;
}
/*
typedef void (*IOHIDCallbackFunction)
              (void * target, IOReturn result, void * refcon, void * sender);

*/
/*
void callback  (void * target, IOReturn result, void * refcon, void * sender);
void callback  (void * target, IOReturn result, void * refcon, void * sender)
{
}
*/

int prHIDRunEventLoop(void)
{
	DEBUG(post("prHIDRunEventLoop"););

	//PyrSlot *a = g->sp - 1; //class

	InstallEventLoopTimer(GetCurrentEventLoop(), 0, 0.001, GetTimerUPP (), 0, &gTimer);

	//HIDSetQueueCallback(pCurrentHIDDevice, callback);
	return (0);
}


int prHIDQueueDevice(void)
{
	DEBUG(post("prHIDQueueDevice"););

	int locID = NULL;
	int cookieNum = NULL;
	
	//PyrSlot *a = g->sp - 1; //class
	//PyrSlot *b = g->sp; //locID device
	//int err = slotIntVal(b, &locID);
	//if (err) return err;
	//look for the right device: 
	pRecDevice  pCurrentHIDDevice = HIDGetFirstDevice ();
	while (pCurrentHIDDevice && (pCurrentHIDDevice->locID !=locID))
		pCurrentHIDDevice = HIDGetNextDevice (pCurrentHIDDevice);
	if(!pCurrentHIDDevice) return (1);
	HIDQueueDevice(pCurrentHIDDevice);
	return (0);	
}


int prHIDQueueElement(void)
{
	DEBUG(post("prHIDQueueElement"););

	int locID = NULL;
	int cookieNum = NULL;
	
	//PyrSlot *a = g->sp - 2; //class
	//PyrSlot *b = g->sp - 1; //locID device
	//PyrSlot *c = g->sp; //element cookie
	//int err = slotIntVal(b, &locID);
	//if (err) return err;
	//err = slotIntVal(c, &cookieNum);
	//if (err) return err;
	IOHIDElementCookie cookie = (IOHIDElementCookie) cookieNum;
	//look for the right device: 
	pRecDevice  pCurrentHIDDevice = HIDGetFirstDevice ();
	while (pCurrentHIDDevice && (pCurrentHIDDevice->locID !=locID))
		pCurrentHIDDevice = HIDGetNextDevice (pCurrentHIDDevice);
	if(!pCurrentHIDDevice) return (1);
	//look for the right element:
	pRecElement pCurrentHIDElement =  HIDGetFirstDeviceElement (pCurrentHIDDevice, kHIDElementTypeIO);
	// use gElementCookie to find current element
	while (pCurrentHIDElement && (pCurrentHIDElement->cookie != cookie))
		pCurrentHIDElement = HIDGetNextDeviceElement (pCurrentHIDElement, kHIDElementTypeIO);
	if(!pCurrentHIDElement) return (1);
	HIDQueueElement(pCurrentHIDDevice, pCurrentHIDElement);
	return (0);	
}


int prHIDDequeueElement(void)
{
	DEBUG(post("prHIDDequeueElement"););

	int locID = NULL;
	int cookieNum = NULL;
	
	//PyrSlot *a = g->sp - 2; //class
	//PyrSlot *b = g->sp - 1; //locID device
	//PyrSlot *c = g->sp; //element cookie
	//int err = slotIntVal(b, &locID);
	//if (err) return err;
	//err = slotIntVal(c, &cookieNum);
	//if (err) return err;
	IOHIDElementCookie cookie = (IOHIDElementCookie) cookieNum;
	//look for the right device: 
    pRecDevice  pCurrentHIDDevice = HIDGetFirstDevice ();
	while (pCurrentHIDDevice && (pCurrentHIDDevice->locID !=locID))
        pCurrentHIDDevice = HIDGetNextDevice (pCurrentHIDDevice);
	if(!pCurrentHIDDevice) return (1);
	//look for the right element:
	pRecElement pCurrentHIDElement =  HIDGetFirstDeviceElement (pCurrentHIDDevice, kHIDElementTypeIO);
    while (pCurrentHIDElement && (pCurrentHIDElement->cookie != cookie))
        pCurrentHIDElement = HIDGetNextDeviceElement (pCurrentHIDElement, kHIDElementTypeIO);
	if(!pCurrentHIDElement) return (1);
	HIDDequeueElement(pCurrentHIDDevice, pCurrentHIDElement);
	return (0);	
}


int prHIDDequeueDevice(void)
{
	DEBUG(post("prHIDDequeueDevice"););

	int locID = NULL;
	int cookieNum = NULL;
	
	/*
	PyrSlot *a = g->sp - 1; //class
	PyrSlot *b = g->sp; //locID device
	int err = slotIntVal(b, &locID);
	if (err) return err;
	*/
	//look for the right device: 
	pRecDevice  pCurrentHIDDevice = HIDGetFirstDevice ();
	while (pCurrentHIDDevice && (pCurrentHIDDevice->locID !=locID))
		pCurrentHIDDevice = HIDGetNextDevice (pCurrentHIDDevice);
	if(!pCurrentHIDDevice) return (1);
	HIDDequeueDevice(pCurrentHIDDevice);
	return (0);	
}


int prHIDStopEventLoop(void)
{
	DEBUG(post("prHIDStopEventLoop"););

	if (gTimer)
        RemoveEventLoopTimer(gTimer);
	gTimer = NULL;
	return (0);
}


t_int hid_open_device(t_hid *x, t_int device_number)
{
	post("open_device %d",device_number);

	return (1);
}

t_int hid_open_device(t_hid *x)
{
	return (0);
}


t_int hid_devicelist_refresh(t_hid *x)
{
	/* the device list should be refreshed here */
	if ( (prHIDBuildDeviceList()) && (prHIDBuildElementList()) )
		return (0);
	else
		return (1);
}


/* this is just a rough sketch */

/* getEvents(t_hid *x) { */
/* 	pRecDevice pCurrentHIDDevice = GetSetCurrentDevice (gWindow); */
/* 	pRecElement pCurrentHIDElement = GetSetCurrenstElement (gWindow); */

/* 	// if we have a good device and element which is not a collecion */
/* 	if (pCurrentHIDDevice && pCurrentHIDElement && (pCurrentHIDElement->type != kIOHIDElementTypeCollection)) */
/* 	{ */
/* 		SInt32 value = HIDGetElementValue (pCurrentHIDDevice, pCurrentHIDElement); */
/* 		SInt32 valueCal = HIDCalibrateValue (value, pCurrentHIDElement); */
/* 		SInt32 valueScale = HIDScaleValue (valueCal, pCurrentHIDElement); */
/* 	 } */
/* } */




#endif  /* #ifdef __APPLE__ */
