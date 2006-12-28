// wiiremote.c
// Copyright by Masayuki Akamatsu
// Based on "DarwiinRemote" by Hiroaki Kimura

#include "wiiremote.h"
#include <unistd.h>

// this type is used a lot (data array):
typedef unsigned char darr[];

#define	kTrial	10

//static	WiiRemoteRec	gWiiRemote;		// remove in 1.0B4

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

void wiiremote_init(WiiRemoteRef wiiremote)
{
	wiiremote->inquiry = nil;
	wiiremote->device = nil;
	wiiremote->ichan = nil;
	wiiremote->cchan = nil;
	
	wiiremote->accX = 0x10;
	wiiremote->accY = 0x10;
	wiiremote->accZ = 0x10;
	wiiremote->buttonData = 0;
	wiiremote->leftPoint = -1;
	wiiremote->tracking = false;
	
	wiiremote->batteryLevel = 0;
	
	wiiremote->isIRSensorEnabled = false;
	wiiremote->isMotionSensorEnabled = false;
	wiiremote->isVibrationEnabled = false;
	
	wiiremote->isExpansionPortUsed = false;
	wiiremote->isLED1Illuminated = false;
	wiiremote->isLED2Illuminated = false;
	wiiremote->isLED3Illuminated = false;
	wiiremote->isLED4Illuminated = false;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

void checkDevice(WiiRemoteRef wiiremote, IOBluetoothDeviceRef device)
{
	CFStringRef	myString;

	myString = IOBluetoothDeviceGetName(device);
	if (myString != nil)
	{
		if (CFStringCompare(myString, CFSTR("Nintendo RVL-CNT-01"), 0) == kCFCompareEqualTo)
		{
			wiiremote->device = IOBluetoothObjectRetain(device);
			if ( !wiiremote_connect(wiiremote))	// add in B3
				wiiremote_disconnect(wiiremote); // add in B3
		}
	}
}

void myFoundFunc(void *refCon, IOBluetoothDeviceInquiryRef inquiry, IOBluetoothDeviceRef device)
{
	checkDevice((WiiRemoteRef)refCon, device);
}

void myUpdatedFunc(void *refCon, IOBluetoothDeviceInquiryRef inquiry, IOBluetoothDeviceRef device, uint32_t devicesRemaining)
{
	checkDevice((WiiRemoteRef)refCon, device);
}

void myCompleteFunc(void *refCon, IOBluetoothDeviceInquiryRef inquiry, IOReturn error, Boolean aborted)
{
	if (aborted) return; // called by stop ;)
	
	if (error != kIOReturnSuccess)
	{
		wiiremote_stopsearch((WiiRemoteRef)refCon);
	}
}

//--------------------------------------------------------------------------------------------

Boolean wiiremote_isconnected(WiiRemoteRef wiiremote)
{
	Boolean	result;
	
	result = wiiremote->device != nil && IOBluetoothDeviceIsConnected(wiiremote->device);
	return result;
}
	
Boolean wiiremote_search(WiiRemoteRef wiiremote)
{
	IOReturn	ret;
	
	if (wiiremote->inquiry != nil)
		return true;
	
	wiiremote->inquiry = IOBluetoothDeviceInquiryCreateWithCallbackRefCon((void *)wiiremote);
	IOBluetoothDeviceInquirySetDeviceFoundCallback(wiiremote->inquiry, myFoundFunc);
	IOBluetoothDeviceInquirySetDeviceNameUpdatedCallback(wiiremote->inquiry, myUpdatedFunc);
	IOBluetoothDeviceInquirySetCompleteCallback(wiiremote->inquiry, myCompleteFunc);

	ret = IOBluetoothDeviceInquiryStart(wiiremote->inquiry);
	if (ret != kIOReturnSuccess)
	{
		IOBluetoothDeviceInquiryDelete(wiiremote->inquiry);
		wiiremote->inquiry = nil;
		return false;
	}
	return true;
}

Boolean wiiremote_stopsearch(WiiRemoteRef wiiremote)
{
	IOReturn	ret;

	if (wiiremote->inquiry == nil)
	{
		return true;	// already stopped
	}
	
	ret = IOBluetoothDeviceInquiryStop(wiiremote->inquiry);
	
	if (ret != kIOReturnSuccess && ret != kIOReturnNotPermitted)
	{
		// kIOReturnNotPermitted is if it's already stopped
	}
	
	IOBluetoothDeviceInquiryDelete(wiiremote->inquiry);
	wiiremote->inquiry = nil;
	
	return (ret==kIOReturnSuccess);
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

 void myDataListener(IOBluetoothL2CAPChannelRef channel, void *data, UInt16 length, void *refCon)
{
	WiiRemoteRef	wiiremote = (WiiRemoteRef)refCon;
	unsigned char	*dp = (unsigned char*)data;

	if (dp[1] == 0x20 && length >= 8)
	{
		wiiremote->batteryLevel = (double)dp[7];
		wiiremote->batteryLevel /= (double)0xC0;
		 
		wiiremote->isExpansionPortUsed =  (dp[4] & 0x02) != 0;
		wiiremote->isLED1Illuminated = (dp[4] & 0x10) != 0;
		wiiremote->isLED2Illuminated = (dp[4] & 0x20) != 0;
		wiiremote->isLED3Illuminated = (dp[4] & 0x40) != 0;
		wiiremote->isLED4Illuminated = (dp[4] & 0x80) != 0;
		 
		//have to reset settings (vibration, motion, IR and so on...)
		wiiremote_irsensor(wiiremote, wiiremote->isIRSensorEnabled);
	}

	if ((dp[1]&0xF0) == 0x30)
	{
		wiiremote->buttonData = ((short)dp[2] << 8) + dp[3];
	 
		if (dp[1] & 0x01)
		{
			wiiremote->accX = dp[4];
			wiiremote->accY = dp[5];
			wiiremote->accZ = dp[6];
		 
			wiiremote->lowZ = wiiremote->lowZ * .9 + wiiremote->accZ * .1;
			wiiremote->lowX = wiiremote->lowX * .9 + wiiremote->accX * .1;
		 
			float absx = abs(wiiremote->lowX - 128);
			float absz = abs(wiiremote->lowZ - 128);
		 
			if (wiiremote->orientation == 0 || wiiremote->orientation == 2) absx -= 5;
			if (wiiremote->orientation == 1 || wiiremote->orientation == 3) absz -= 5;
		 
			if (absz >= absx)
			{
				if (absz > 5)
					wiiremote->orientation = (wiiremote->lowZ > 128) ? 0 : 2;
			}
			else
			{
				if (absx > 5)
					wiiremote->orientation = (wiiremote->lowX > 128) ? 3 : 1;
			}
			//printf("orientation: %d\n", orientation);
		}
	 
		if (dp[1] & 0x02)
		{
			int i;
			for(i=0 ; i<4 ; i++)
			{
				wiiremote->irData[i].x = dp[7 + 3*i];
				wiiremote->irData[i].y = dp[8 + 3*i];
				wiiremote->irData[i].s = dp[9 + 3*i];
				wiiremote->irData[i].x += (wiiremote->irData[i].s & 0x30) << 4;
				wiiremote->irData[i].y += (wiiremote->irData[i].s & 0xC0) << 2;
				wiiremote->irData[i].s &= 0x0F;
			} 
		}
	}

	float ox, oy;

	if (wiiremote->irData[0].s < 0x0F && wiiremote->irData[1].s < 0x0F)
	{
		int l = wiiremote->leftPoint, r;
		if (wiiremote->leftPoint == -1)
		{
			//	printf("Tracking.\n");
			switch (wiiremote->orientation)
			{
				case 0: l = (wiiremote->irData[0].x < wiiremote->irData[1].x) ? 0 : 1; break;
				case 1: l = (wiiremote->irData[0].y > wiiremote->irData[1].y) ? 0 : 1; break;
				case 2: l = (wiiremote->irData[0].x > wiiremote->irData[1].x) ? 0 : 1; break;
				case 3: l = (wiiremote->irData[0].y < wiiremote->irData[1].y) ? 0 : 1; break;
			}
			wiiremote->leftPoint = l;
		}
		
		r = 1-l;
	 
		float dx = wiiremote->irData[r].x - wiiremote->irData[l].x;
		float dy = wiiremote->irData[r].y - wiiremote->irData[l].y;
	 
		float d = sqrt(dx*dx+dy*dy);
	 
		dx /= d;
		dy /= d;
	 
		float cx = (wiiremote->irData[l].x+wiiremote->irData[r].x)/1024.0 - 1;
		float cy = (wiiremote->irData[l].y+wiiremote->irData[r].y)/1024.0 - .75;
	 
		wiiremote->angle = atan2(dy, dx);
	 
		ox = -dy*cy-dx*cx;
		oy = -dx*cy+dy*cx;
		//printf("x:%5.2f;  y: %5.2f;  angle: %5.1f\n", ox, oy, angle*180/M_PI);
		
		wiiremote->tracking = true;
	}
	else
	{
		//	printf("Not tracking.\n");
		ox = oy = -100;
		wiiremote->angle = -100;
		wiiremote->leftPoint = -1;
		wiiremote->tracking = false;
	}
	
	wiiremote->posX = ox;	
	wiiremote->posY = oy;
}

void myEventListener(IOBluetoothL2CAPChannelRef channel, void *refCon, IOBluetoothL2CAPChannelEvent *event)
{
	if (event->eventType == kIOBluetoothL2CAPChannelEventTypeData)
	{
		// In thise case:
		// event->u.newData.dataPtr  is a pointer to the block of data received.
		// event->u.newData.dataSize is the size of the block of data.
		myDataListener(channel, event->u.data.dataPtr, event->u.data.dataSize, refCon);
	}
	else
	if (event->eventType == kIOBluetoothL2CAPChannelEventTypeClosed)
	{
		// In this case:
		// event->u.terminatedChannel is the channel that was terminated. It can be converted in an IOBluetoothL2CAPChannel
		// object with [IOBluetoothL2CAPChannel withL2CAPChannelRef:]. (see below).
	}
}

void myDisconnectedFunc(void * refCon, IOBluetoothUserNotificationRef inRef, IOBluetoothObjectRef objectRef)
{
	//wiiremote_disconnect();
}

//--------------------------------------------------------------------------------------------

Boolean wiiremote_connect(WiiRemoteRef wiiremote)
{
	IOReturn	ret;
	short		i;
	
	if (wiiremote->device == nil)
		return false;
	
	// connect the device
	for (i=0; i<kTrial; i++)
	{
		ret = IOBluetoothDeviceOpenConnection(wiiremote->device, nil, nil);
		if ( ret == kIOReturnSuccess)
			break;
		usleep(10000); //  wait 10ms
	}
	if (i==kTrial)
		return false;
	
	wiiremote->disconnectNotification = IOBluetoothDeviceRegisterForDisconnectNotification(wiiremote->device, myDisconnectedFunc, 0);
	
	// performs an SDP query
	for (i=0; i<kTrial; i++)
	{
		ret = IOBluetoothDevicePerformSDPQuery(wiiremote->device, nil, nil);
		if ( ret == kIOReturnSuccess)
			break;
		usleep(10000); //  wait 10ms
	}
	if (i==kTrial)
		return false;
	
	// open L2CAPChannel : BluetoothL2CAPPSM = 17
	for (i=0; i<kTrial; i++)
	{
		ret = IOBluetoothDeviceOpenL2CAPChannelSync(wiiremote->device, &(wiiremote->cchan), 17, myEventListener, (void *)wiiremote);
		if ( ret == kIOReturnSuccess)
			break;
		usleep(10000); //  wait 10ms
	}
	if (i==kTrial)
	{
		wiiremote->cchan = nil;
		IOBluetoothDeviceCloseConnection(wiiremote->device);
		wiiremote->device = nil;
		return false;
	}
	
	// open L2CAPChannel : BluetoothL2CAPPSM = 19
	for (i=0; i<kTrial; i++)
	{
		ret = IOBluetoothDeviceOpenL2CAPChannelSync(wiiremote->device, &(wiiremote->ichan), 19, myEventListener, (void *)wiiremote);
		if ( ret == kIOReturnSuccess)
			break;
		usleep(10000); //  wait 10ms
	}
	if (i==kTrial)
	{
		wiiremote->ichan = nil;
		IOBluetoothL2CAPChannelCloseChannel(wiiremote->cchan);
		IOBluetoothDeviceCloseConnection(wiiremote->device);
		wiiremote->device = nil;
		return false;
	}

	wiiremote_motionsensor(wiiremote, true);
	wiiremote_irsensor(wiiremote, false);
	wiiremote_vibration(wiiremote, false);
	wiiremote_led(wiiremote, false, false, false, false);
	
	return true;
}


Boolean wiiremote_disconnect(WiiRemoteRef wiiremote)
{
	short	i;
	
	if (wiiremote->disconnectNotification != nil)
	{
		IOBluetoothUserNotificationUnregister(wiiremote->disconnectNotification);
		wiiremote->disconnectNotification = nil;
	}
	
	if (wiiremote->cchan && IOBluetoothDeviceIsConnected(wiiremote->device))
	{
		for (i=0; i<kTrial; i++)
		{
			if (IOBluetoothL2CAPChannelCloseChannel(wiiremote->cchan) == kIOReturnSuccess)
			{
				wiiremote->cchan = nil;
				break;
			}
		}
		if (i==kTrial)	return false;
	}
	
	if (wiiremote->ichan && IOBluetoothDeviceIsConnected(wiiremote->device))
	{
		for (i=0; i<kTrial; i++)
		{
			if (IOBluetoothL2CAPChannelCloseChannel(wiiremote->ichan) == kIOReturnSuccess)
			{
				wiiremote->ichan = nil;
				break;
			}
		}
		if (i==kTrial)	return false;
	}
	
	if (wiiremote->device && IOBluetoothDeviceIsConnected(wiiremote->device))
	{
		for (i=0; i<kTrial; i++)
		{
			if (IOBluetoothDeviceCloseConnection(wiiremote->device) == kIOReturnSuccess)
			{
				break;
			}
		}
		if (i==kTrial)	return false;
	}
	
	if (wiiremote->device != nil)
	{
		IOBluetoothObjectRelease(wiiremote->device);
		wiiremote->device = nil;
	}
	
	return true;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

Boolean sendCommand(WiiRemoteRef wiiremote, unsigned char *data, size_t length)
{
	unsigned char buf[40];
	IOReturn	ret;
	int			i;
	
	memset(buf,0,40);
	buf[0] = 0x52;
	memcpy(buf+1, data, length);
	if (buf[1] == 0x16)
		length=23;
	else
		length++;
	
	for (i = 0; i<kTrial; i++)
	{
		ret = IOBluetoothL2CAPChannelWriteSync(wiiremote->cchan, buf, length);
		if (ret == kIOReturnSuccess)
			break;
		usleep(10000);
	}	
	
	return (ret==kIOReturnSuccess);
}

Boolean	writeData(WiiRemoteRef wiiremote, const unsigned char *data, unsigned long address, size_t length)
{
	unsigned char cmd[22];
	unsigned int i;

	for(i=0 ; i<length ; i++) cmd[i+6] = data[i];
	
	for(;i<16 ; i++) cmd[i+6]= 0;
	
	cmd[0] = 0x16;
	cmd[1] = (address>>24) & 0xFF;
	cmd[2] = (address>>16) & 0xFF;
	cmd[3] = (address>> 8) & 0xFF;
	cmd[4] = (address>> 0) & 0xFF;
	cmd[5] = length;

	// and of course the vibration flag, as usual
	if (wiiremote->isVibrationEnabled)	cmd[1] |= 0x01;
	
	data = cmd;
	
	return sendCommand(wiiremote, cmd, 22);
}

//--------------------------------------------------------------------------------------------

Boolean wiiremote_motionsensor(WiiRemoteRef wiiremote, Boolean enabled)
{
	wiiremote->isMotionSensorEnabled = enabled;

	unsigned char cmd[] = {0x12, 0x00, 0x30};
	if (wiiremote->isVibrationEnabled)		cmd[1] |= 0x01;
	if (wiiremote->isMotionSensorEnabled)	cmd[2] |= 0x01;
	if (wiiremote->isIRSensorEnabled)		cmd[2] |= 0x02;
	
	return sendCommand(wiiremote, cmd, 3);
}

Boolean wiiremote_irsensor(WiiRemoteRef wiiremote, Boolean enabled)
{
	IOReturn ret;
	
	wiiremote->isIRSensorEnabled = enabled;
	
	// set register 0x12 (report type)
	if (ret = wiiremote_motionsensor(wiiremote, wiiremote->isMotionSensorEnabled) == false)	return ret;
	
	// set register 0x13 (ir enable/vibe)
	if (ret = wiiremote_vibration(wiiremote, wiiremote->isVibrationEnabled) == false)	return ret;
	
	// set register 0x1a (ir enable 2)
	unsigned char cmd[] = {0x1a, 0x00};
	if (enabled)	cmd[1] |= 0x04;
	if (ret = sendCommand(wiiremote, cmd, 2) == false) return ret;
	
	if(enabled){
		// based on marcan's method, found on wiili wiki:
		// tweaked to include some aspects of cliff's setup procedure in the hopes
		// of it actually turning on 100% of the time (was seeing 30-40% failure rate before)
		// the sleeps help it it seems
		usleep(10000);
		if (ret = writeData(wiiremote, (darr){0x01}, 0x04B00030, 1) == false) return ret;
		usleep(10000);
		if (ret = writeData(wiiremote, (darr){0x08}, 0x04B00030, 1) == false) return ret;
		usleep(10000);
		if (ret = writeData(wiiremote, (darr){0x90}, 0x04B00006, 1) == false) return ret;
		usleep(10000);
		if (ret = writeData(wiiremote, (darr){0xC0}, 0x04B00008, 1) == false) return ret;
		usleep(10000);
		if (ret = writeData(wiiremote, (darr){0x40}, 0x04B0001A, 1) == false) return ret;
		usleep(10000);
		if (ret = writeData(wiiremote, (darr){0x33}, 0x04B00033, 1) == false) return ret;
		usleep(10000);
		if (ret = writeData(wiiremote, (darr){0x08}, 0x04B00030, 1) == false) return ret;
		
	}else{
		// probably should do some writes to power down the camera, save battery
		// but don't know how yet.
		
		//bug fix #1614587 
		wiiremote_motionsensor(wiiremote, wiiremote->isMotionSensorEnabled);
		wiiremote_vibration(wiiremote, wiiremote->isVibrationEnabled);
	}
	
	return true;
}

Boolean wiiremote_vibration(WiiRemoteRef wiiremote, Boolean enabled)
{
	
	wiiremote->isVibrationEnabled = enabled;
	
	unsigned char cmd[] = {0x13, 0x00};
	if (wiiremote->isVibrationEnabled)	cmd[1] |= 0x01;
	if (wiiremote->isIRSensorEnabled)	cmd[1] |= 0x04;
	
	return sendCommand(wiiremote, cmd, 2);;
}

Boolean wiiremote_led(WiiRemoteRef wiiremote, Boolean enabled1, Boolean enabled2, Boolean enabled3, Boolean enabled4)
{
	unsigned char cmd[] = {0x11, 0x00};
	if (wiiremote->isVibrationEnabled)	cmd[1] |= 0x01;
	if (enabled1)	cmd[1] |= 0x10;
	if (enabled2)	cmd[1] |= 0x20;
	if (enabled3)	cmd[1] |= 0x40;
	if (enabled4)	cmd[1] |= 0x80;
	
	wiiremote->isLED1Illuminated = enabled1;
	wiiremote->isLED2Illuminated = enabled2;
	wiiremote->isLED3Illuminated = enabled3;
	wiiremote->isLED4Illuminated = enabled4;
	
	return sendCommand(wiiremote, cmd, 2);
}

Boolean wiiremote_getstatus(WiiRemoteRef wiiremote)
{
	unsigned char cmd[] = {0x15, 0x00};
	return sendCommand(wiiremote, cmd, 2);
}


