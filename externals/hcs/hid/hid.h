#ifndef _HID_H
#define _HID_H

#include <stdio.h>

#include <m_pd.h>

/* 
 * this is automatically generated from linux/input.h by
 * make-arrays-from-input.h.pl to be the cross-platform event types and codes 
 */
#include "input_arrays.h"

static char *version = "$Revision: 1.4 $";

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *hid_class;

typedef struct _hid 
{
		t_object            x_obj;
		t_int               x_fd;
		t_symbol            *x_devname;
		t_int               x_device_number;
		t_clock             *x_clock;
		t_int               x_read_ok;
		t_int               x_started;
		t_int               x_delay;
		t_int               x_vendorID;
		t_int               x_productID;
		t_int               x_locID;
} t_hid;


/*------------------------------------------------------------------------------
 *  GLOBALS
 */

/* what are these for again? */
char *deviceList[64];
char *typeList[256];
char *codeList[256];

/*------------------------------------------------------------------------------
 *  FUNCTION PROTOTYPES FOR DIFFERENT PLATFORMS
 */

/* generic, cross-platform functions */
t_int hid_open_device(t_hid *x, t_int device_number);
t_int hid_close_device(t_hid *x);
t_int hid_devicelist_refresh(t_hid* x);
t_int hid_output_events(t_hid *x) ;





#ifdef __APPLE__
void releaseHIDDevices(void);
int prHIDBuildElementList(void);
int prHIDBuildDeviceList(void);
int prHIDGetValue(void);
void PushQueueEvents_RawValue(void);
void PushQueueEvents_CalibratedValue(void);
int prHIDReleaseDeviceList(void);
//int prHIDRunEventLoop(void);
int prHIDQueueDevice(void);
int prHIDQueueElement(void);
int prHIDDequeueElement(void);
int prHIDDequeueDevice(void);
//int prHIDStopEventLoop(void);
#endif  /* #ifdef __APPLE__ */




#endif  /* #ifndef _HID_H */
