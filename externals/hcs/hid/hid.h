#ifndef _HID_H
#define _HID_H

#include <stdio.h>

#include <m_pd.h>

/* 
 * this is automatically generated from linux/input.h by
 * make-arrays-from-input.h.pl to be the cross-platform event types and codes 
 */
#include "input_arrays.h"

#define HID_MAJOR_VERSION 0
#define HID_MINOR_VERSION 1

static char *version = "$Revision: 1.9 $";

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
		t_int               x_delay;
		t_int               x_started;
		t_int               x_device_open;
		t_int               x_instance_count;
} t_hid;


/*------------------------------------------------------------------------------
 *  GLOBALS
 */

/* TODO: what are these for again? */
char *deviceList[64];
char *typeList[256];
char *codeList[256];

/*------------------------------------------------------------------------------
 *  FUNCTION PROTOTYPES FOR DIFFERENT PLATFORMS
 */

/* support functions */
void hid_output_event(t_hid *x,
							  char *type, char *code, t_float value, t_float timestamp);

/* generic, cross-platform functions */
t_int hid_open_device(t_hid *x, t_int device_number);
t_int hid_close_device(t_hid *x);
t_int hid_build_device_list(t_hid* x);
t_int hid_get_events(t_hid *x) ;
void hid_platform_specific_free(t_hid *x);

#endif  /* #ifndef _HID_H */
