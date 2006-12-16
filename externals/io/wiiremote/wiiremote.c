// wiiremote.c
// Copyright by Masayuki Akamatsu
// Based on "DarwiinRemote" by Hiroaki Kimura

#include "wiiremote.h"

#include <unistd.h>

// this type is used a lot (data array):
typedef unsigned char darr[];

#define	kTrial	10

static	WiiRemoteRec	gWiiRemote;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

WiiRemoteRef wiiremote_init(void)
{
	gWiiRemote.inquiry = nil;
	gWiiRemote.device = nil;
	gWiiRemote.ichan = nil;
	gWiiRemote.cchan = nil;
	
	gWiiRemote.accX = 0x10;
	gWiiRemote.accY = 0x10;
	gWiiRemote.accZ = 0x10;
	gWiiRemote.buttonData = 0;
	gWiiRemote.leftPoint = -1;
	gWiiRemote.tracking = false;
	
	gWiiRemote.batteryLevel = 0;
	
	gWiiRemote.isIRSensorEnabled = false;
	gWiiRemote.isMotionSensorEnabled = false;
	gWiiRemote.isVibrationEnabled = false;
	
	gWiiRemote.isExpansionPortUsed = false;
	gWiiRemote.isLED1Illuminated = false;
	gWiiRemote.isLED2Illuminated = false;
	gWiiRemote.isLED3Illuminated = false;
	gWiiRemote.isLED4Illuminated = false;
	
	return &gWiiRemote;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

void checkDevice(IOBluetoothDeviceRef device)
{
	CFStringRef	myString;

	myString = IOBluetoothDeviceGetName(device);
	if (CFStringCompare(myString, CFSTR("Nintendo RVL-CNT-01"), 0) == kCFCompareEqualTo)
	{
		gWiiRemote.device = IOBluetoothObjectRetain(device);
	}
}

IOBluetoothDeviceInquiryDeviceFoundCallback myFoundFunc(void *refCon, IOBluetoothDeviceInquiryRef inquiry, IOBluetoothDeviceRef device)
{
	checkDevice(device);
}

IOBluetoothDeviceInquiryDeviceNameUpdatedCallback	myUpdatedFunc(void *refCon, IOBluetoothDeviceInquiryRef inquiry, IOBluetoothDeviceRef device, uint32_t devicesRemaining)
{
	checkDevice(device);
}

IOBluetoothDeviceInquiryCompleteCallback myCompleteFunc(void *refCon, IOBluetoothDeviceInquiryRef inquiry, IOReturn error, Boolean aborted)
{
	IOReturn				result;
	
	if (aborted) return; // called by stop ;)
	
	if (error != kIOReturnSuccess)
	{
		wiiremote_stopsearch();
		return;
	}
}

//--------------------------------------------------------------------------------------------

Boolean wiiremote_search(void)
{
	IOReturn	ret;
	
	if (gWiiRemote.inquiry != nil)
		return true;
	
	gWiiRemote.inquiry = IOBluetoothDeviceInquiryCreateWithCallbackRefCon(nil);
	IOBluetoothDeviceInquirySetDeviceFoundCallback(gWiiRemote.inquiry, myFoundFunc);
	IOBluetoothDeviceInquirySetDeviceNameUpdatedCallback(gWiiRemote.inquiry, myUpdatedFunc);
	IOBluetoothDeviceInquirySetCompleteCallback(gWiiRemote.inquiry, myCompleteFunc);

	ret = IOBluetoothDeviceInquiryStart(gWiiRemote.inquiry);
	if (ret != kIOReturnSuccess)
	{
		IOBluetoothDeviceInquiryDelete(gWiiRemote.inquiry);
		gWiiRemote.inquiry = nil;
		return false;
	}
	return true;
}

Boolean wiiremote_stopsearch(void)
{
	IOReturn	ret;

	if (gWiiRemote.inquiry == nil)
	{
		return true;	// already stopped
	}
	
	ret = IOBluetoothDeviceInquiryStop(gWiiRemote.inquiry);
	
	if (ret != kIOReturnSuccess && ret != kIOReturnNotPermitted)
	{
		// kIOReturnNotPermitted is if it's already stopped
	}
	
	IOBluetoothDeviceInquiryDelete(gWiiRemote.inquiry);
	gWiiRemote.inquiry = nil;
	
	return (ret==kIOReturnSuccess);
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

 IOBluetoothL2CAPChannelIncomingDataListener myDataListener(IOBluetoothL2CAPChannelRef channel, void *data, UInt16 length, void *refCon)
{
	unsigned char *dp = (unsigned char*)data;

	 if (dp[1] == 0x20 && length >= 8)
	 {
		 gWiiRemote.batteryLevel = (double)dp[7];
		 gWiiRemote.batteryLevel /= (double)0xC0;
		 
		 gWiiRemote.isExpansionPortUsed =  (dp[4] & 0x02) != 0;
		 gWiiRemote.isLED1Illuminated = (dp[4] & 0x10) != 0;
		 gWiiRemote.isLED2Illuminated = (dp[4] & 0x20) != 0;
		 gWiiRemote.isLED3Illuminated = (dp[4] & 0x40) != 0;
		 gWiiRemote.isLED4Illuminated = (dp[4] & 0x80) != 0;
		 
		 //have to reset settings (vibration, motion, IR and so on...)
		 wiiremote_irsensor(gWiiRemote.isIRSensorEnabled);
	 }

	if ((dp[1]&0xF0) == 0x30)
	{
		gWiiRemote.buttonData = ((short)dp[2] << 8) + dp[3];
	 
		if (dp[1] & 0x01)
		{
			gWiiRemote.accX = dp[4];
			gWiiRemote.accY = dp[5];
			gWiiRemote.accZ = dp[6];
		 
			gWiiRemote.lowZ = gWiiRemote.lowZ * .9 + gWiiRemote.accZ * .1;
			gWiiRemote.lowX = gWiiRemote.lowX * .9 + gWiiRemote.accX * .1;
		 
			float absx = abs(gWiiRemote.lowX - 128);
			float absz = abs(gWiiRemote.lowZ - 128);
		 
			if (gWiiRemote.orientation == 0 || gWiiRemote.orientation == 2) absx -= 5;
			if (gWiiRemote.orientation == 1 || gWiiRemote.orientation == 3) absz -= 5;
		 
			if (absz >= absx)
			{
				if (absz > 5)
					gWiiRemote.orientation = (gWiiRemote.lowZ > 128) ? 0 : 2;
			}
			else
			{
				if (absx > 5)
					gWiiRemote.orientation = (gWiiRemote.lowX > 128) ? 3 : 1;
			}
			//printf("orientation: %d\n", orientation);
		}
	 
		if (dp[1] & 0x02)
		{
			int i;
			for(i=0 ; i<4 ; i++)
			{
				gWiiRemote.irData[i].x = dp[7 + 3*i];
				gWiiRemote.irData[i].y = dp[8 + 3*i];
				gWiiRemote.irData[i].s = dp[9 + 3*i];
				gWiiRemote.irData[i].x += (gWiiRemote.irData[i].s & 0x30) << 4;
				gWiiRemote.irData[i].y += (gWiiRemote.irData[i].s & 0xC0) << 2;
				gWiiRemote.irData[i].s &= 0x0F;
			} 
		}
	}

	float ox, oy;

	if (gWiiRemote.irData[0].s < 0x0F && gWiiRemote.irData[1].s < 0x0F)
	{
		int l = gWiiRemote.leftPoint, r;
		if (gWiiRemote.leftPoint == -1)
		{
			//	printf("Tracking.\n");
			switch (gWiiRemote.orientation)
			{
				case 0: l = (gWiiRemote.irData[0].x < gWiiRemote.irData[1].x) ? 0 : 1; break;
				case 1: l = (gWiiRemote.irData[0].y > gWiiRemote.irData[1].y) ? 0 : 1; break;
				case 2: l = (gWiiRemote.irData[0].x > gWiiRemote.irData[1].x) ? 0 : 1; break;
				case 3: l = (gWiiRemote.irData[0].y < gWiiRemote.irData[1].y) ? 0 : 1; break;
			}
			gWiiRemote.leftPoint = l;
		}
		
		r = 1-l;
	 
		float dx = gWiiRemote.irData[r].x - gWiiRemote.irData[l].x;
		float dy = gWiiRemote.irData[r].y - gWiiRemote.irData[l].y;
	 
		float d = sqrt(dx*dx+dy*dy);
	 
		dx /= d;
		dy /= d;
	 
		float cx = (gWiiRemote.irData[l].x+gWiiRemote.irData[r].x)/1024.0 - 1;
		float cy = (gWiiRemote.irData[l].y+gWiiRemote.irData[r].y)/1024.0 - .75;
	 
		gWiiRemote.angle = atan2(dy, dx);
	 
		ox = -dy*cy-dx*cx;
		oy = -dx*cy+dy*cx;
		//printf("x:%5.2f;  y: %5.2f;  angle: %5.1f\n", ox, oy, angle*180/M_PI);
		
		gWiiRemote.tracking = true;
	}
	else
	{
		//	printf("Not tracking.\n");
		ox = oy = -100;
		gWiiRemote.leftPoint = -1;
		gWiiRemote.tracking = false;
	}
	
	gWiiRemote.posX = ox;	
	gWiiRemote.posY = oy;
}

IOBluetoothL2CAPChannelIncomingEventListener myEventListener(IOBluetoothL2CAPChannelRef channel, void *refCon, IOBluetoothL2CAPChannelEvent *event)
{
	switch (event->eventType)
	{
		case kIOBluetoothL2CAPChannelEventTypeData:
			// In thise case:
			// event->u.newData.dataPtr  is a pointer to the block of data received.
			// event->u.newData.dataSize is the size of the block of data.
			myDataListener(channel, event->u.data.dataPtr, event->u.data.dataSize, refCon);
			break;
			
		case kIOBluetoothL2CAPChannelEventTypeClosed:
			// In this case:
			// event->u.terminatedChannel is the channel that was terminated. It can be converted in an IOBluetoothL2CAPChannel
			// object with [IOBluetoothL2CAPChannel withL2CAPChannelRef:]. (see below).
			break;
	}
}

IOBluetoothUserNotificationCallback myDisconnectedFunc(void * refCon, IOBluetoothUserNotificationRef inRef, IOBluetoothObjectRef objectRef)
{
	wiiremote_disconnect();
}

//--------------------------------------------------------------------------------------------

Boolean wiiremote_connect(void)
{
	IOReturn	result;
	short		i;
	
	if (gWiiRemote.device == nil)
		return false;
	
	// connect the device
	for (i=0; i<kTrial; i++)
	{
		if (IOBluetoothDeviceOpenConnection(gWiiRemote.device, nil, nil) == kIOReturnSuccess)
			break;
		usleep(10000); //  wait 10ms
	}
	if (i==kTrial)	return false;
	
	gWiiRemote.disconnectNotification = IOBluetoothDeviceRegisterForDisconnectNotification(gWiiRemote.device, myDisconnectedFunc, 0);
	
	// performs an SDP query
	for (i=0; i<kTrial; i++)
	{
		if (IOBluetoothDevicePerformSDPQuery(gWiiRemote.device, nil, nil) == kIOReturnSuccess)
			break;
		usleep(10000); //  wait 10ms
	}
	if (i==kTrial)	return false;
	
	// open L2CAPChannel : BluetoothL2CAPPSM = 17
	for (i=0; i<kTrial; i++)
	{
		if (IOBluetoothDeviceOpenL2CAPChannelSync(gWiiRemote.device, &(gWiiRemote.cchan), 17, myEventListener, nil) == kIOReturnSuccess)
			break;
		usleep(10000); //  wait 10ms
	}
	if (i==kTrial)
	{
		gWiiRemote.cchan = nil;
		IOBluetoothDeviceCloseConnection(gWiiRemote.device);
		return false;
	}
	
	// open L2CAPChannel : BluetoothL2CAPPSM = 19
	for (i=0; i<kTrial; i++)
	{
		if (IOBluetoothDeviceOpenL2CAPChannelSync(gWiiRemote.device, &(gWiiRemote.ichan), 19, myEventListener, nil) == kIOReturnSuccess)
			break;
		usleep(10000); //  wait 10ms
	}
	if (i==kTrial)
	{
		gWiiRemote.ichan = nil;
		IOBluetoothL2CAPChannelCloseChannel(gWiiRemote.cchan);
		IOBluetoothDeviceCloseConnection(gWiiRemote.device);
		return false;
	}

	wiiremote_motionsensor(true);
	wiiremote_irsensor(false);
	wiiremote_vibration(false);
	wiiremote_led(false, false, false, false);
	
	return true;
}


Boolean wiiremote_disconnect(void)
{
	short	i;
	
	if (gWiiRemote.disconnectNotification != nil)
	{
		IOBluetoothUserNotificationUnregister(gWiiRemote.disconnectNotification);
		gWiiRemote.disconnectNotification = nil;
	}
	
	if (gWiiRemote.cchan && IOBluetoothDeviceIsConnected(gWiiRemote.device))
	{
		for (i=0; i<kTrial; i++)
		{
			if (IOBluetoothL2CAPChannelCloseChannel(gWiiRemote.cchan) == kIOReturnSuccess)
			{
				gWiiRemote.cchan = nil;
				break;
			}
		}
		if (i==kTrial)	return false;
	}
	
	if (gWiiRemote.ichan && IOBluetoothDeviceIsConnected(gWiiRemote.device))
	{
		for (i=0; i<kTrial; i++)
		{
			if (IOBluetoothL2CAPChannelCloseChannel(gWiiRemote.ichan) == kIOReturnSuccess)
			{
				gWiiRemote.ichan = nil;
				break;
			}
		}
		if (i==kTrial)	return false;
	}
	
	if (gWiiRemote.device && IOBluetoothDeviceIsConnected(gWiiRemote.device))
	{
		for (i=0; i<kTrial; i++)
		{
			if (IOBluetoothDeviceCloseConnection(gWiiRemote.device) == kIOReturnSuccess)
			{
				gWiiRemote.device = nil;
				break;
			}
		}
		if (i==kTrial)	return false;
	}
		
	return true;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

Boolean sendCommand(unsigned char *data, size_t length)
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
		ret = IOBluetoothL2CAPChannelWriteSync(gWiiRemote.cchan, buf, length);
		if (ret == kIOReturnSuccess)
			break;
		usleep(10000);
	}	
	
	return (ret==kIOReturnSuccess);
}

Boolean	writeData(const unsigned char *data, unsigned long address, size_t length)
{
	unsigned char cmd[22];
	int i;

	for(i=0 ; i<length ; i++) cmd[i+6] = data[i];
	
	for(;i<16 ; i++) cmd[i+6]= 0;
	
	cmd[0] = 0x16;
	cmd[1] = (address>>24) & 0xFF;
	cmd[2] = (address>>16) & 0xFF;
	cmd[3] = (address>> 8) & 0xFF;
	cmd[4] = (address>> 0) & 0xFF;
	cmd[5] = length;

	// and of course the vibration flag, as usual
	if (gWiiRemote.isVibrationEnabled)	cmd[1] |= 0x01;
	
	data = cmd;
	
	return sendCommand(cmd, 22);
}

//--------------------------------------------------------------------------------------------

Boolean wiiremote_motionsensor(Boolean enabled)
{
	gWiiRemote.isMotionSensorEnabled = enabled;

	unsigned char cmd[] = {0x12, 0x00, 0x30};
	if (gWiiRemote.isVibrationEnabled)		cmd[1] |= 0x01;
	if (gWiiRemote.isMotionSensorEnabled)	cmd[2] |= 0x01;
	if (gWiiRemote.isIRSensorEnabled)		cmd[2] |= 0x02;
	
	return sendCommand(cmd, 3);
}

Boolean wiiremote_irsensor(Boolean enabled)
{
	IOReturn ret;
	
	gWiiRemote.isIRSensorEnabled = enabled;
	
	// set register 0x12 (report type)
	if (ret = wiiremote_motionsensor(gWiiRemote.isMotionSensorEnabled)) return ret;
	
	// set register 0x13 (ir enable/vibe)
	if (ret = wiiremote_vibration(gWiiRemote.isVibrationEnabled)) return ret;
	
	// set register 0x1a (ir enable 2)
	unsigned char cmd[] = {0x1a, 0x00};
	if (enabled)	cmd[1] |= 0x04;
	if (ret = sendCommand(cmd, 2)) return ret;
	
	if(enabled){
		// based on marcan's method, found on wiili wiki:
		// tweaked to include some aspects of cliff's setup procedure in the hopes
		// of it actually turning on 100% of the time (was seeing 30-40% failure rate before)
		// the sleeps help it it seems
		usleep(10000);
		if (ret = writeData((darr){0x01}, 0x04B00030, 1)) return ret;
		usleep(10000);
		if (ret = writeData((darr){0x08}, 0x04B00030, 1)) return ret;
		usleep(10000);
		if (ret = writeData((darr){0x90}, 0x04B00006, 1)) return ret;
		usleep(10000);
		if (ret = writeData((darr){0xC0}, 0x04B00008, 1)) return ret;
		usleep(10000);
		if (ret = writeData((darr){0x40}, 0x04B0001A, 1)) return ret;
		usleep(10000);
		if (ret = writeData((darr){0x33}, 0x04B00033, 1)) return ret;
		usleep(10000);
		if (ret = writeData((darr){0x08}, 0x04B00030, 1)) return ret;
		
	}else{
		// probably should do some writes to power down the camera, save battery
		// but don't know how yet.
		
		//bug fix #1614587 
		wiiremote_motionsensor(gWiiRemote.isMotionSensorEnabled);
		wiiremote_vibration(gWiiRemote.isVibrationEnabled);
	}
	
	return true;
}

Boolean wiiremote_vibration(Boolean enabled)
{
	
	gWiiRemote.isVibrationEnabled = enabled;
	
	unsigned char cmd[] = {0x13, 0x00};
	if (gWiiRemote.isVibrationEnabled)	cmd[1] |= 0x01;
	if (gWiiRemote.isIRSensorEnabled)	cmd[1] |= 0x04;
	
	return sendCommand(cmd, 2);;
}

Boolean wiiremote_led(Boolean enabled1, Boolean enabled2, Boolean enabled3, Boolean enabled4)
{
	unsigned char cmd[] = {0x11, 0x00};
	if (gWiiRemote.isVibrationEnabled)	cmd[1] |= 0x01;
	if (enabled1)	cmd[1] |= 0x10;
	if (enabled2)	cmd[1] |= 0x20;
	if (enabled3)	cmd[1] |= 0x40;
	if (enabled4)	cmd[1] |= 0x80;
	
	gWiiRemote.isLED1Illuminated = enabled1;
	gWiiRemote.isLED2Illuminated = enabled2;
	gWiiRemote.isLED3Illuminated = enabled3;
	gWiiRemote.isLED4Illuminated = enabled4;
	
	return sendCommand(cmd, 2);
}

void wiiremote_getstatus(void)
{
	unsigned char cmd[] = {0x15, 0x00};
	sendCommand(cmd, 2);
}


