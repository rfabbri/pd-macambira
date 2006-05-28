#ifndef _HID_H
#define _HID_H

#include <stdio.h>
#include <sys/syslog.h>

#include <m_pd.h>

/* 
 * this is automatically generated from linux/input.h by
 * make-arrays-from-input.h.pl to be the cross-platform event types and codes 
 */
#include "input_arrays.h"

#define HID_MAJOR_VERSION 0
#define HID_MINOR_VERSION 7

/* static char *version = "$Revision: 1.23 $"; */

/*------------------------------------------------------------------------------
 * GLOBAL DEFINES
 */

#define DEFAULT_DELAY 5

/* this is set to simplify data structures (arrays instead of linked lists) */
#define MAX_DEVICES 128

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
typedef struct _hid 
{
	t_object            x_obj;
	t_int               x_fd;
	t_int               x_device_number;
//	unsigned short      vendor_id;    // USB idVendor for current device
//	unsigned short      product_id;   // USB idProduct for current device
	t_int               x_has_ff;
	void                *x_ff_device;
	t_clock             *x_clock;
	t_int               x_delay;
	t_int               x_started;
	t_int               x_device_open;
	t_outlet            *x_data_outlet;
	t_outlet            *x_status_outlet;
} t_hid;




/*------------------------------------------------------------------------------
 *  GLOBAL VARIABLES
 */

/*
 * count the number of instances of this object so that certain free()
 * functions can be called only after the final instance is detroyed.
 */
t_int hid_instance_count;

extern unsigned short global_debug_level;

/* next I need to make a data structure to hold the data to be output for this
 * poll.  This should probably be an array for efficiency */

/*------------------------------------------------------------------------------
 *  FUNCTION PROTOTYPES FOR DIFFERENT PLATFORMS
 */

/* support functions */
void debug_print(t_int debug_level, const char *fmt, ...);
void debug_error(t_hid *x, t_int debug_level, const char *fmt, ...);
void hid_output_event(t_hid *x, t_symbol *type, t_symbol *code, t_float value);

/* generic, cross-platform functions implemented in a separate file for each
 * platform 
 */
t_int hid_open_device(t_hid *x, t_int device_number);
t_int hid_close_device(t_hid *x);
void hid_build_device_list(void);
t_int hid_get_events(t_hid *x);
void hid_print(t_hid* x); /* print info to the console */
void hid_platform_specific_info(t_hid* x); /* device info on the status outlet */
void hid_platform_specific_free(t_hid *x);
t_int get_device_number_by_id(unsigned short vendor_id, unsigned short product_id);
t_int get_device_number_from_usage_list(t_int device_number, 
										unsigned short usage_page, 
										unsigned short usage);


/* cross-platform force feedback functions */
t_int hid_ff_autocenter(t_hid *x, t_float value);
t_int hid_ff_gain(t_hid *x, t_float value);
t_int hid_ff_motors(t_hid *x, t_float value);
t_int hid_ff_continue(t_hid *x);
t_int hid_ff_pause(t_hid *x);
t_int hid_ff_reset(t_hid *x);
t_int hid_ff_stopall(t_hid *x);

// these are just for testing...
t_int hid_ff_fftest (t_hid *x, t_float value);
void hid_ff_print(t_hid *x);





#endif  /* #ifndef _HID_H */
