#ifndef _HID_H
#define _HID_H

#include <m_pd.h>

#include "hid.h"

static char *version = "$Revision: 1.3 $";

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
 *  GLOBALS
 */

char *deviceList[64];
char *typeList[256];
char *codeList[256];

/*------------------------------------------------------------------------------
 *  FUNCTION PROTOTYPES FOR DIFFERENT PLATFORMS
 */

#ifdef __linux__
void releaseDevices(void);
void buildDeviceList(void);
void buildTypeList(void);

#endif


#ifdef __APPLE__
void releaseHIDDevices(void);
int prHIDBuildElementList(void);
int prHIDBuildDeviceList(void);
int prHIDGetValue(void);
void PushQueueEvents_RawValue(void);
void PushQueueEvents_CalibratedValue(void);
int prHIDReleaseDeviceList(void);
int prHIDRunEventLoop(void);
int prHIDQueueDevice(void);
int prHIDQueueElement(void);
int prHIDDequeueElement(void);
int prHIDDequeueDevice(void);
int prHIDStopEventLoop(void);
#endif  /* #ifdef __APPLE__ */




#endif  /* #ifndef _HID_H */
