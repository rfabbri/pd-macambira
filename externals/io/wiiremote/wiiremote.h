// wiiremote.h
// Copyright by Masayuki Akamatsu
// Based on "DarwiinRemote" by Hiroaki Kimura

#include <CoreFoundation/CoreFoundation.h>
#include <IOBluetooth/Bluetooth.h>
#include <IOBluetooth/IOBluetoothUserLib.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	int x, y, s;
} IRData;

typedef struct _WiiRemoteRec
{
	IOBluetoothDeviceInquiryRef	inquiry;
	IOBluetoothDeviceRef		device;
	IOBluetoothL2CAPChannelRef	ichan;
	IOBluetoothL2CAPChannelRef	cchan;

	unsigned char	accX;
	unsigned char	accY;
	unsigned char	accZ;
	unsigned short	buttonData;
	
	float			lowZ;
	float			lowX;
	int				orientation;
	int				leftPoint; // is point 0 or 1 on the left. -1 when not tracking.
	float			posX;
	float			posY;
	float			angle;
	Boolean			tracking;

	IRData			irData[4];
	double			batteryLevel;
	
	Boolean			isIRSensorEnabled;
	Boolean			isMotionSensorEnabled;
	Boolean			isVibrationEnabled;
	
	Boolean			isExpansionPortUsed;
	Boolean			isLED1Illuminated;
	Boolean			isLED2Illuminated;
	Boolean			isLED3Illuminated;
	Boolean			isLED4Illuminated;
	
	IOBluetoothUserNotificationRef	disconnectNotification;
}	WiiRemoteRec, *WiiRemoteRef;

void			wiiremote_init(WiiRemoteRef wiiremote);
Boolean			wiiremote_isconnected(WiiRemoteRef wiiremote);
Boolean			wiiremote_search(WiiRemoteRef wiiremote);
Boolean			wiiremote_stopsearch(WiiRemoteRef wiiremote);
Boolean			wiiremote_connect(WiiRemoteRef wiiremote);
Boolean			wiiremote_disconnect(WiiRemoteRef wiiremote);
Boolean			wiiremote_motionsensor(WiiRemoteRef wiiremote, Boolean enabled);
Boolean			wiiremote_irsensor(WiiRemoteRef wiiremote, Boolean enabled);
Boolean			wiiremote_vibration(WiiRemoteRef wiiremote, Boolean enabled);
Boolean			wiiremote_led(WiiRemoteRef wiiremote, Boolean enabled1, Boolean enabled2, Boolean enabled3, Boolean enabled4);
Boolean			wiiremote_getstatus(WiiRemoteRef wiiremote);
