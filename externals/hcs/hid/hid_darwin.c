#ifdef __APPLE__
/*
 *  Apple Darwin HID Manager support for [hid]
 *
 *  based on SC_HID.cpp from SuperCollider3 by Jan Truetzschler v. Falkenstein
 *
 *  Copyright (c) 2004 Hans-Christoph All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */


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

/*
#include "SCBase.h"
#include "VMGlobals.h"
#include "PyrSymbolTable.h"
#include "PyrInterpreter.h"
#include "PyrKernel.h"

#include "PyrObjectProto.h"
#include "PyrPrimitiveProto.h"
#include "PyrKernelProto.h"
#include "SC_InlineUnaryOp.h"
#include "SC_InlineBinaryOp.h"
#include "PyrSched.h"
#include "GC.h"
*/


#define DEBUG(x)
//#define DEBUG(x) x 

/*==============================================================================
 *  GLOBAL VARS
 *======================================================================== */

int gNumberOfHIDDevices = 0;
EventLoopTimerRef gTimer = NULL; // timer for element data updates


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

/*
	PyrSlot *a = g->sp - 1; //class
	PyrSlot *b = g->sp; //locID device
*/
	int locID = NULL;
	int cookieNum = NULL;
	UInt32 i;
	pRecElement	devElement;
	pRecDevice pCurrentHIDDevice;
	UInt32 numElements;
	char cstrElementName [256];

//	int err = slotIntVal(b, &locID);
//	if (err) return err;

	// look for the right device: 
	pCurrentHIDDevice = HIDGetFirstDevice ();
	while (pCurrentHIDDevice && (pCurrentHIDDevice->locID !=locID))
        pCurrentHIDDevice = HIDGetNextDevice (pCurrentHIDDevice);
	if(!pCurrentHIDDevice) return (1);
	
	devElement =  HIDGetFirstDeviceElement (pCurrentHIDDevice, kHIDElementTypeInput);
	numElements = HIDCountDeviceElements (pCurrentHIDDevice, kHIDElementTypeInput);

	//PyrObject* devAllElementsArray = newPyrArray(g->gc, numElements * sizeof(PyrObject), 0 , true);
		
		for(i=0; i<numElements; i++)
		{
			//PyrObject* devElementArray = newPyrArray(g->gc, 5 * sizeof(PyrObject), 0 , true);
			HIDGetTypeName((IOHIDElementType) devElement->type, cstrElementName);
			//PyrString *devstring = newPyrString(g->gc, cstrElementName, 0, true);
			//SetObject(devElementArray->slots+devElementArray->size++, devstring);
			//g->gc->GCWrite(devElementArray, (PyrObject*) devstring);
			//usage
			HIDGetUsageName (devElement->usagePage, devElement->usage, cstrElementName);
			//devstring = newPyrString(g->gc, cstrElementName, 0, true);
			//SetObject(devElementArray->slots+devElementArray->size++, devstring);
			//g->gc->GCWrite(devElementArray, (PyrObject*) devstring);
			//cookie
			//SetInt(devElementArray->slots+devElementArray->size++, (long) devElement->cookie);
			//SetInt(devElementArray->slots+devElementArray->size++, (long) devElement->min);
			//SetInt(devElementArray->slots+devElementArray->size++, (long) devElement->max);
			
			//SetObject(devAllElementsArray->slots+devAllElementsArray->size++, devElementArray);
			//g->gc->GCWrite(devAllElementsArray, (PyrObject*) devElementArray);
			
			devElement =  HIDGetNextDeviceElement (devElement, kHIDElementTypeInput);
		}
		//SetObject(a, devAllElementsArray);

		return (0);	
}

int prHIDBuildDeviceList(void)
{
	DEBUG(post("prHIDBuildDeviceList"););

	int i,err;
	UInt32 usagePage, usage;
/*
	//build a device list
	PyrSlot *a = g->sp - 2;
	PyrSlot *b = g->sp - 1; //usagePage
	PyrSlot *c = g->sp;		//usage

	if(IsNil(b)) 
		usagePage = NULL;
	else
	{	
		err = slotIntVal(b, &usagePage);
		if (err) return err;
	}
	if(IsNil(c)) 
		usage = NULL;
	else
	{
		err = slotIntVal(c, &usage);
		if (err) return err;
	}

	//pass in usage & usagepage
	//kHIDUsage_GD_Joystick kHIDUsage_GD_GamePad
	*/
	usagePage = kHIDPage_GenericDesktop;
	usage = NULL;

	Boolean result = HIDBuildDeviceList (usagePage, usage); 
	// returns false if no device found (ignored in this case) - returns always false ?

	if(result) post("no HID devices found\n");
	
	int numdevs = HIDCountDevices();
	gNumberOfHIDDevices = numdevs;
	// exit if no devices found
	if(!numdevs) return (0);

	post("number of devices: %d", numdevs);
	char cstrDeviceName [256];
	
	pRecDevice  pCurrentHIDDevice = HIDGetFirstDevice ();
	pRecElement devElement;
	//PyrObject* allDevsArray = newPyrArray(g->gc, numdevs * sizeof(PyrObject), 0 , true);
	for(i=0; i<numdevs; i++)
	{
		/*
		//device:
		/PyrObject* devNameArray = newPyrArray(g->gc, 6 * sizeof(PyrObject), 0 , true);
		//manufacturer:
		PyrString *devstring = newPyrString(g->gc, pCurrentHIDDevice->manufacturer, 0, true);
		SetObject(devNameArray->slots+devNameArray->size++, devstring);
		g->gc->GCWrite(devNameArray, (PyrObject*) devstring);
		//product name:
		devstring = newPyrString(g->gc, pCurrentHIDDevice->product, 0, true);
		SetObject(devNameArray->slots+devNameArray->size++, devstring);
		g->gc->GCWrite(devNameArray, (PyrObject*) devstring);
		*/
		//usage
		HIDGetUsageName (pCurrentHIDDevice->usagePage, pCurrentHIDDevice->usage, cstrDeviceName);
		/*
		devstring = newPyrString(g->gc, cstrDeviceName, 0, true);
		SetObject(devNameArray->slots+devNameArray->size++, devstring);
		g->gc->GCWrite(devNameArray, (PyrObject*) devstring);
		//vendor id
		SetInt(devNameArray->slots+devNameArray->size++, pCurrentHIDDevice->vendorID);
		//product id
		SetInt(devNameArray->slots+devNameArray->size++, pCurrentHIDDevice->productID);
		//locID
		SetInt(devNameArray->slots+devNameArray->size++, pCurrentHIDDevice->locID);
		
		SetObject(allDevsArray->slots+allDevsArray->size++, devNameArray);
		g->gc->GCWrite(allDevsArray, (PyrObject*) devNameArray);
		*/
		pCurrentHIDDevice = HIDGetNextDevice (pCurrentHIDDevice);

	}

	//UInt32 outnum = HIDCountDeviceElements (pCurrentHIDDevice, kHIDElementTypeOutput);
	//post("number of outputs: %d \n", outnum);
//	SetObject(a, allDevsArray);

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

	static EventLoopTimerUPP	sTimerUPP = NULL;
	
	if (sTimerUPP == NULL)
		sTimerUPP = NewEventLoopTimerUPP (IdleTimer);
	
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

	InstallEventLoopTimer (GetCurrentEventLoop(), 0, 0.001, GetTimerUPP (), 0, &gTimer);

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


#endif  /* #ifdef __APPLE__ */
