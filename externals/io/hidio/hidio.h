#ifndef _HIDIO_H
#define _HIDIO_H

#include <stdio.h>
#ifdef _WINDOWS
#include "pthread.h"	/* needs pthread library */
#define LOG_DEBUG 7
#define LOG_INFO 6
#define LOG_WARNING 4
#define vsnprintf _vsnprintf
#pragma warning (disable: 4305 4244 4761)
#else
#include <sys/syslog.h>
#include <pthread.h>
#endif

#ifdef __linux__
#include <linux/types.h>
#endif /* __linux__ */

#ifdef PD
#include <m_pd.h>
#else
#include "ext.h"
#include "ext_obex.h"
#include "commonsyms.h"
/* some Pd specific types need definition in Max */
typedef long t_int;
typedef double t_float;
typedef double t_floatarg;
typedef void t_outlet;
typedef void t_clock;
#define MAXPDSTRING 512
#define pd_error(x, b) error(b)
#define SETSYMBOL SETSYM
#endif

/* 
 * this is automatically generated from linux/input.h by
 * make-arrays-from-input.h.pl to be the cross-platform event types and codes 
 */
#include "input_arrays.h"

#define HIDIO_MAJOR_VERSION 0
#define HIDIO_MINOR_VERSION 0

/* static char *version = "$Revision: 1.5 $"; */

/*------------------------------------------------------------------------------
 * GLOBAL DEFINES
 */

#define DEFAULT_DELAY 5

/* this is set to simplify data structures (arrays instead of linked lists) */
#define MAX_DEVICES 128

/* I think 64 is the limit per device as defined in the OS <hans@at.or.at> */
#define MAX_ELEMENTS 64

/* this is limited so that the object doesn't cause a click getting too many
 * events from the OS's event queue */
#define MAX_EVENTS_PER_POLL 64

/*------------------------------------------------------------------------------
 *  THREADING RELATED DEFINES
 */
 
#define REQUEST_NOTHING 0
#define REQUEST_OPEN 1
#define REQUEST_READ 2
#define REQUEST_SEND 3
#define REQUEST_INFO 4
#define REQUEST_CLOSE 5
#define REQUEST_QUIT 6


/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
typedef struct _hidio

{
	t_object            x_obj;
#ifndef PD
	void				*x_obex;
#endif
	t_int               x_fd;
	void                *x_ff_device;
	short               x_device_number;
	short               x_instance;
	t_int               x_has_ff;
	t_int               x_started;
	t_int               x_device_open;
	t_int               x_delay;
	t_clock             *x_clock;
	t_outlet            *x_data_outlet;
	t_outlet            *x_status_outlet;
    t_int				x_requestcode;
    pthread_mutex_t		x_mutex;
	pthread_cond_t		x_requestcondition;
    pthread_cond_t		x_answercondition;
    pthread_t			x_thread;
} t_hidio;



/*------------------------------------------------------------------------------
 *  GLOBAL VARIABLES
 */

/* count the number of instances of this object so that certain free()
 * functions can be called only after the final instance is detroyed.
 */
extern t_int hidio_instance_count;

/* this is used to test for the first instance to execute */
extern double last_execute_time[MAX_DEVICES];

extern unsigned short global_debug_level;

/* built up when the elements of an open device are enumerated */
typedef struct _hid_element
{
#ifdef __linux__
    /* GNU/Linux store type and code to compare against */
    __u16 linux_type;
    __u16 linux_code;
#endif /* __linux__ */
#ifdef _WIN32
	/* this should be pointers to the UsagePage and Usage */
#endif /* _WIN32 */
#ifdef __APPLE__
    void *pHIDElement;  // pRecElement on Mac OS X; ... on Windows
#endif /* __APPLE__ */
    t_symbol *type; // Linux "type"; HID "usagePage"
    t_symbol *name; // Linux "code"; HID "usage"
    unsigned char polled; // is it polled or queued? (maybe only on Mac OS X?)
    unsigned char relative; // relative data gets output everytime
    t_int min; // from device report
    t_int max; // from device report
    t_float instance; // usage page/usage instance # ([absolute throttle 2 163( 
    t_int value; // output the sum of events in a poll for relative axes
    t_int previous_value; //only output on change on abs and buttons
} t_hid_element;

/* mostly for status querying */
extern unsigned short device_count;

/* store element structs to eliminate symbol table lookups, etc. */
extern t_hid_element *element[MAX_DEVICES][MAX_ELEMENTS];
/* number of active elements per device */
extern unsigned short element_count[MAX_DEVICES]; 

/*------------------------------------------------------------------------------
 *  FUNCTION PROTOTYPES FOR DIFFERENT PLATFORMS
 */

/* support functions */
void debug_print(t_int debug_level, const char *fmt, ...);
void debug_error(t_hidio *x, t_int debug_level, const char *fmt, ...);
void hidio_output_event(t_hidio *x, t_hid_element *output_data);


/* generic, cross-platform functions implemented in a separate file for each
 * platform 
 */
t_int hidio_open_device(t_hidio *x, short device_number);
t_int hidio_close_device(t_hidio *x);
void hidio_build_device_list(void);
void hidio_get_events(t_hidio *x);
void hidio_print(t_hidio* x); /* print info to the console */
void hidio_platform_specific_info(t_hidio* x); /* device info on the status outlet */
void hidio_platform_specific_free(t_hidio *x);
short get_device_number_by_id(unsigned short vendor_id, unsigned short product_id);
/* TODO: this function should probably accept the single unsigned for the combined usage_page and usage, instead of two separate variables */
short get_device_number_from_usage(short device_number, 
										unsigned short usage_page, 
										unsigned short usage);


/* cross-platform force feedback functions */
t_int hidio_ff_autocenter(t_hidio *x, t_float value);
t_int hidio_ff_gain(t_hidio *x, t_float value);
t_int hidio_ff_motors(t_hidio *x, t_float value);
t_int hidio_ff_continue(t_hidio *x);
t_int hidio_ff_pause(t_hidio *x);
t_int hidio_ff_reset(t_hidio *x);
t_int hidio_ff_stopall(t_hidio *x);

// these are just for testing...
t_int hidio_ff_fftest (t_hidio *x, t_float value);
void hidio_ff_print(t_hidio *x);





#endif  /* #ifndef _HIDIO_H */
