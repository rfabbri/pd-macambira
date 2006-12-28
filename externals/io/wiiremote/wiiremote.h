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

WiiRemoteRef	wiiremote_init(void);
Boolean			wiiremote_isconnected(void);
Boolean			wiiremote_search(void);
Boolean			wiiremote_stopsearch(void);
Boolean			wiiremote_connect(void);
Boolean			wiiremote_disconnect(void);
Boolean			wiiremote_motionsensor(Boolean enabled);
Boolean			wiiremote_irsensor(Boolean enabled);
Boolean			wiiremote_vibration(Boolean enabled);
Boolean			wiiremote_led(Boolean enabled1, Boolean enabled2, Boolean enabled3, Boolean enabled4);
void			wiiremote_getstatus(void);
