#ifndef _HID_H
#define _HID_H

#include <m_pd.h>

#include "hid.h"

static char *version = "$Revision: 1.2 $";

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *hid_class;

typedef struct _hid 
{
		t_object            x_obj;
		t_int               x_fd;
		t_symbol            *x_devname;
		t_clock             *x_clock;
		t_int               x_read_ok;
		t_int               x_started;
		t_int               x_delay;
		t_int               x_vendorID;
		t_int               x_productID;
		t_int               x_locID;
} t_hid;


/*------------------------------------------------------------------------------
 *  FUNCTION PROTOTYPES FOR APPLE HID MANAGER
 */
#ifdef __APPLE__
void releaseHIDDevices (void);
int prHIDBuildElementList(void);
int prHIDBuildDeviceList(void);
int prHIDGetValue(void);
void PushQueueEvents_RawValue(void);
void PushQueueEvents_CalibratedValue(void);
//static pascal void IdleTimer(EventLoopTimerRef inTimer, void* userData);
int prHIDReleaseDeviceList(void);
//static EventLoopTimerUPP GetTimerUPP(void);
//void callback(void * target, IOReturn result, void * refcon, void * sender);
int prHIDRunEventLoop(void);
int prHIDQueueDevice(void);
int prHIDQueueElement(void);
int prHIDDequeueElement(void);
int prHIDDequeueDevice(void);
int prHIDStopEventLoop(void);
#endif  /* #ifdef __APPLE__ */




#endif  /* #ifndef _HID_H */
