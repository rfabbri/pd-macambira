#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>

#include "m_pd.h"

#define DEBUG(x)
//#define DEBUG(x) x

#define DEFAULT_DELAY    10
#define SIXAXIS_DEVICE   "/dev/hidraw0"

static char *version = "$Revision: 1.1 $";

/*------------------------------------------------------------------------------
 *  GLOBAL DECLARATIONS
 */

/* hidraw data format */
struct sixaxis_state {
    double time;
    int ax, ay, az;       // Raw accelerometer data
    double ddx, ddy, ddz; // Acceleration
    double dx, dy, dz;    // Speed
    double x, y, z;       // Position
};

/* pre-generated symbols */
static t_symbol *ps_open, *ps_device, *ps_poll, *ps_total, *ps_range;
static t_symbol *ps_accelerometer, *ps_acceleration, *ps_speed, *ps_position;

/* mostly for status querying */
static unsigned short device_count;

/* previous state for calculating position, speed, acceleration */
static struct sixaxis_state prev;

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */
static t_class *sixaxis_class;

typedef struct _sixaxis {
    t_object            x_obj;
    t_int               x_fd;
    t_symbol            *x_devname;
    t_clock             *x_clock;
	short               x_device_number;
	short               x_instance;
	t_int               x_device_open;
    int                 x_read_ok;
    int                 x_started;
    int                 x_delay;
    unsigned char       buf[128];
    struct sixaxis_state x_sixaxis_state;
    t_atom              x_output_atoms[3];
    t_outlet            *x_data_outlet;
    t_outlet            *x_status_outlet;
} t_sixaxis;



/*------------------------------------------------------------------------------
 * SUPPORT FUNCTIONS
 */

static void output_status(t_sixaxis *x, t_symbol *selector, t_float output_value)
{
    t_atom *output_atom = (t_atom *)getbytes(sizeof(t_atom));
    SETFLOAT(output_atom, output_value);
    outlet_anything( x->x_status_outlet, selector, 1, output_atom);
    freebytes(output_atom,sizeof(t_atom));
}

static void output_open_status(t_sixaxis *x)
{
    output_status(x, ps_open, x->x_device_open);
}

static void output_device_number(t_sixaxis *x)
{
    output_status(x, ps_device, x->x_device_number);
}

static void output_poll_time(t_sixaxis *x)
{
    output_status(x, ps_poll, x->x_delay);
}

static void output_device_count(t_sixaxis *x)
{
    output_status(x, ps_total, device_count);
}

/*------------------------------------------------------------------------------
 * CLASS METHODS
 */

void sixaxis_stop(t_sixaxis* x)
{
    DEBUG(post("sixaxis_stop"););
  
    if (x->x_fd >= 0 && x->x_started) { 
        clock_unset(x->x_clock);
        post("sixaxis: polling stopped");
        x->x_started = 0;
    }
}

static void sixaxis_close(t_sixaxis *x)
{
	DEBUG(post("sixaxis_close"););

/* just to be safe, stop it first */
	sixaxis_stop(x);

    if(x->x_fd < 0) 
        return;
    close(x->x_fd);
	post("[sixaxis] closed %s",x->x_devname->s_name);
    x->x_device_open = 0;
    output_open_status(x);
}

static int sixaxis_open(t_sixaxis *x, t_symbol *s)
{
    DEBUG(post("sixaxis_open"););

    sixaxis_close(x);

    /* set obj device name to parameter 
     * otherwise set to default
     */  
    if (s != &s_)
        x->x_devname = s;
  
    /* open device */
    if (x->x_devname) {
        /* open the device read-only, non-exclusive */
        x->x_fd = open(x->x_devname->s_name, O_RDONLY | O_NONBLOCK);
        /* test if device open */
        if (x->x_fd > -1 ) {
            x->x_device_open = 1;
            output_open_status(x);
            return 1;
        } else {
            pd_error(x, "[sixaxis] open %s failed", x->x_devname->s_name);
            x->x_fd = -1;
            x->x_device_open = 0;
            output_open_status(x);
        }
    }
    return 0;
}

static void sixaxis_read(t_sixaxis *x)
{
	if(x->x_fd < 0) 
        return;
    if(read(x->x_fd, &(x->buf), sizeof(x->buf)) > -1) {
//        if ( nr < 0 ) { perror("read(stdin)"); exit(1); }
//        if ( nr != 48 ) { fprintf(stderr, "Unsupported report\n"); exit(1); }

        struct timeval tv;
        if ( gettimeofday(&tv, NULL) ) {
            perror("gettimeofday");
            return;
        }
        x->x_sixaxis_state.time = tv.tv_sec + tv.tv_usec*1e-6;
        x->x_sixaxis_state.ax = x->buf[40]<<8 | x->buf[41];
        x->x_sixaxis_state.ay = x->buf[42]<<8 | x->buf[43];
        x->x_sixaxis_state.az = x->buf[44]<<8 | x->buf[45];
        if ( ! prev.time ) {
            prev.time = x->x_sixaxis_state.time;
            prev.ax = x->x_sixaxis_state.ax;
            prev.ay = x->x_sixaxis_state.ay;
            prev.az = x->x_sixaxis_state.az;
        }
        double dt = x->x_sixaxis_state.time - prev.time;
        double rc_dd = 2.0;  // Time constant for highpass filter on acceleration
        double alpha_dd = rc_dd / (rc_dd+dt);
        x->x_sixaxis_state.ddx = alpha_dd*(prev.ddx + (x->x_sixaxis_state.ax-prev.ax)*0.01);
        x->x_sixaxis_state.ddy = alpha_dd*(prev.ddy + (x->x_sixaxis_state.ay-prev.ay)*0.01);
        x->x_sixaxis_state.ddz = alpha_dd*(prev.ddz - (x->x_sixaxis_state.az-prev.az)*0.01);
        double rc_d = 2.0;  // Time constant for highpass filter on speed
        double alpha_d = rc_d / (rc_d+dt);
        x->x_sixaxis_state.dx = alpha_d*(prev.dx + x->x_sixaxis_state.ddx*dt);
        x->x_sixaxis_state.dy = alpha_d*(prev.dy + x->x_sixaxis_state.ddy*dt);
        x->x_sixaxis_state.dz = alpha_d*(prev.dz + x->x_sixaxis_state.ddz*dt);
        double rc = 1.0;  // Time constant for highpass filter on position
        double alpha = rc / (rc+dt);
        x->x_sixaxis_state.x = alpha*(prev.x + x->x_sixaxis_state.dx*dt);
        x->x_sixaxis_state.y = alpha*(prev.y + x->x_sixaxis_state.dy*dt);
        x->x_sixaxis_state.z = alpha*(prev.z + x->x_sixaxis_state.dz*dt);
        /* raw accelerometer data */
		SETFLOAT(x->x_output_atoms, x->x_sixaxis_state.ax);
		SETFLOAT(x->x_output_atoms + 1, x->x_sixaxis_state.ay);
		SETFLOAT(x->x_output_atoms + 2, x->x_sixaxis_state.az);
		outlet_anything(x->x_data_outlet, ps_accelerometer, 3, x->x_output_atoms);
        /* acceleration data */
		SETFLOAT(x->x_output_atoms, x->x_sixaxis_state.ddx);
		SETFLOAT(x->x_output_atoms + 1, x->x_sixaxis_state.ddy);
		SETFLOAT(x->x_output_atoms + 2, x->x_sixaxis_state.ddz);
		outlet_anything(x->x_data_outlet, ps_acceleration, 3, x->x_output_atoms);
        /* speed data */
		SETFLOAT(x->x_output_atoms, x->x_sixaxis_state.dx);
		SETFLOAT(x->x_output_atoms + 1, x->x_sixaxis_state.dy);
		SETFLOAT(x->x_output_atoms + 2, x->x_sixaxis_state.dz);
		outlet_anything(x->x_data_outlet, ps_speed, 3, x->x_output_atoms);
        /* position data */
		SETFLOAT(x->x_output_atoms, x->x_sixaxis_state.x);
		SETFLOAT(x->x_output_atoms + 1, x->x_sixaxis_state.y);
		SETFLOAT(x->x_output_atoms + 2, x->x_sixaxis_state.z);
		outlet_anything(x->x_data_outlet, ps_position, 3, x->x_output_atoms);
	}
	if(x->x_started) {
		clock_delay(x->x_clock, x->x_delay);
	}
}
//    double ddx, ddy, ddz; // Acceleration
//    double dx, dy, dz;    // Speed
//    double x, y, z;       // Position

/* Actions */

static void sixaxis_info(t_sixaxis *x)
{
    output_open_status(x);
    output_device_number(x);
    output_device_count(x);
    output_poll_time(x);
// TODO output ranges for sixaxis
//    output_element_ranges(x);
}

void sixaxis_start(t_sixaxis* x)
{
    DEBUG(post("sixaxis_start"););
    
    if (x->x_fd > -1 && !x->x_started) {
        clock_delay(x->x_clock, DEFAULT_DELAY);
        post("sixaxis: polling started");
        x->x_started = 1;
    } else {
        post("You need to set a input device (i.e /dev/input/event0)");
    }

	if(x->x_device_number > -1) 
	{
		if(!x->x_device_open)
		{
			sixaxis_open(x,x->x_devname);
		}
		if(!x->x_started) 
		{
			clock_delay(x->x_clock, x->x_delay);
			x->x_started = 1;
		} 
	}

}

static void sixaxis_float(t_sixaxis* x, t_floatarg f)
{
    DEBUG(post("sixaxis_float"););
    
    if (f > 0)
        sixaxis_start(x);
    else
        sixaxis_stop(x);
}

/* setup functions */
static void sixaxis_free(t_sixaxis* x)
{
    DEBUG(post("sixaxis_free"););
    
    if (x->x_fd < 0) return;

    sixaxis_stop(x);
    clock_free(x->x_clock);
    close(x->x_fd);
}

static void *sixaxis_new(t_symbol *s)
{
    t_sixaxis *x = (t_sixaxis *)pd_new(sixaxis_class);

    DEBUG(post("sixaxis_new"););

    post("[sixaxis] %s, written by Hans-Christoph Steiner <hans@eds.org>",version);  

    /* init vars */
    x->x_fd = -1;
    x->x_read_ok = 1;
    x->x_started = 0;
    x->x_delay = DEFAULT_DELAY;
    x->x_devname = gensym(SIXAXIS_DEVICE);

    x->x_clock = clock_new(x, (t_method)sixaxis_read);
  
    /* create standard hidio-style outlets */
    x->x_data_outlet = outlet_new(&x->x_obj, 0);
    x->x_status_outlet = outlet_new(&x->x_obj, 0);

    /* set to the value from the object argument, if that exists */
    if (s != &s_)
        x->x_devname = s;
  
    return (x);
}

void sixaxis_setup(void) 
{
    DEBUG(post("sixaxis_setup"););
    sixaxis_class = class_new(gensym("sixaxis"), 
                              (t_newmethod)sixaxis_new, 
                              (t_method)sixaxis_free,
                              sizeof(t_sixaxis), 0, A_DEFSYM, 0);
  
    /* add inlet datatype methods */
    class_addfloat(sixaxis_class,(t_method) sixaxis_float);
    class_addbang(sixaxis_class,(t_method) sixaxis_read);
  
    /* add inlet message methods */
    class_addmethod(sixaxis_class,(t_method) sixaxis_open,gensym("open"),A_DEFFLOAT,0);
    class_addmethod(sixaxis_class,(t_method) sixaxis_close,gensym("close"),0);
    class_addmethod(sixaxis_class,(t_method) sixaxis_info,gensym("info"),0);
    
    /* pre-generate often used symbols */
    ps_open = gensym("open");
    ps_device = gensym("device");
    ps_poll = gensym("poll");
    ps_total = gensym("total");
    ps_range = gensym("range");
    ps_accelerometer = gensym("accelerometer");
    ps_acceleration = gensym("acceleration");
    ps_speed = gensym("speed");
    ps_position = gensym("position");
}

