/* this code only works for Linux kernels */
#ifdef __linux__


#include <linux/input.h>
#include <sys/ioctl.h>

#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <stdint.h>

#include "hidio.h"

/* 
 * this is automatically generated from linux/input.h by
 * make-arrays-from-input.h.pl to be the cross-platform event types and codes 
 */
#include "input_arrays.h"

#define DEBUG(x)
//#define DEBUG(x) x 

#define LINUX_BLOCK_DEVICE   "/dev/input/event"


/*------------------------------------------------------------------------------
 * from evtest.c from the ff-utils package
 */

/* from asm/types.h and linux/input.h __kernel__ sections */
#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) (((x)/BITS_PER_LONG)+1)
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> (bit%BITS_PER_LONG)) & 1)


/*
 * from an email from Vojtech:
 *
 * The application reading the device is supposed to queue all events up to 
 * the SYN_REPORT event, and then process them, so that a mouse pointer
 * will move diagonally instead of following the sides of a rectangle, 
 * which would be very annoying. 
 */


/* ------------------------------------------------------------------------------ */
/* LINUX-SPECIFIC SUPPORT FUNCTIONS */
/* ------------------------------------------------------------------------------ */

t_symbol* hidio_convert_linux_buttons_to_numbers(__u16 linux_code)
{
    char hidio_code[MAXPDSTRING];
    if(linux_code >= 0x100) 
    {
        if(linux_code < BTN_MOUSE)         /* numbered buttons */
            snprintf(hidio_code, MAXPDSTRING,"btn_%d",linux_code - BTN_MISC);  
        else if(linux_code < BTN_JOYSTICK) /* mouse buttons */
            snprintf(hidio_code, MAXPDSTRING,"btn_%d",linux_code - BTN_MOUSE);
        else if(linux_code < BTN_GAMEPAD)  /* joystick buttons */
            snprintf(hidio_code, MAXPDSTRING,"btn_%d",linux_code - BTN_JOYSTICK);
        else if(linux_code < BTN_DIGI)     /* gamepad buttons */
            snprintf(hidio_code, MAXPDSTRING,"btn_%d",linux_code - BTN_GAMEPAD);
        else if(linux_code < BTN_WHEEL)    /* tablet buttons */
            snprintf(hidio_code, MAXPDSTRING,"btn_%d",linux_code - BTN_DIGI);
        else if(linux_code < KEY_OK)       /* wheel buttons */
            snprintf(hidio_code, MAXPDSTRING,"btn_%d",linux_code - BTN_WHEEL);
	else return 0;
    }
    return gensym(hidio_code ? hidio_code : "?");
}

/* Georg Holzmann: implementation of the keys */
/* JMZ: use t_symbol instead of char[] (s.a.) AND 
 * appended "key_" in the array so we don't have to append it each time AND
 * made the table static
 */
t_symbol* hidio_convert_linux_keys(__u16 linux_code)
{  
    if(linux_code > 226)
        return 0;

    static char key_names[227][32] =
        { 
            "key_reserved", "key_esc", "key_1", "key_2", "key_3", "key_4", 
            "key_5", "key_6", "key_7", "key_8", "key_9", "key_0", "key_minus", 
            "key_equal", "key_backspace", "key_tab", "key_q", "key_w", 
            "key_e", "key_r", "key_t", "key_y", "key_u", "key_i", "key_o", 
            "key_p","key_leftbrace", "key_rightbrace", "key_enter", 
            "key_leftctrl", "key_a","key_s", "key_d", "key_f", "key_g", 
            "key_h", "key_j", "key_k", "key_l", "key_semicolon",
            "key_apostrophe", "key_grave", "key_leftshift", "key_backslash", 
            "key_z","key_x", "key_c", "key_v", "key_b", "key_n", "key_m", 
            "key_comma", "key_dot", "key_slash","key_rightshift", 
            "key_kpasterisk", "key_leftalt", "key_space", "key_capslock",
            "key_f1", "key_f2", "key_f3", "key_f4", "key_f5", "key_f6", 
            "key_f7", "key_f8", "key_f9", "key_f10","key_numlock", 
            "key_scrolllock", "key_kp7", "key_kp8", "key_kp9", "key_kpminus",
            "key_kp4", "key_kp5", "key_kp6", "key_kpplus", "key_kp1", "key_kp2",
            "key_kp3", "key_kp3",  "key_kpdot","key_103rd", "key_f13", 
            "key_102nd", "key_f11", "key_f12", "key_f14", "key_f15", "key_f16",
            "key_f17", "key_f18", "key_f19", "key_f20", "key_kpenter", 
            "key_rightctrl", "key_kpslash","key_sysrq", "key_rightalt", 
            "key_linefeed", "key_home", "key_up", "key_pageup", "key_left",
            "key_right", "key_end", "key_down", "key_pagedown", "key_insert", 
            "key_delete", "key_macro","key_mute", "key_volumedown", 
            "key_volumeup", "key_power", "key_kpequal", "key_kpplusminus",
            "key_pause", "key_f21", "key_f22", "key_f23", "key_f24", 
            "key_kpcomma", "key_leftmeta","key_rightmeta", "key_compose",
            "key_stop", "key_again", "key_props", "key_undo", "key_front", 
            "key_copy", "key_open","key_paste", "key_find","key_cut","key_help", 
            "key_menu", "key_calc", "key_setup", "key_sleep", "key_wakeup",
            "key_file", "key_sendfile", "key_deletefile","key_xfer","key_prog1",
            "key_prog2", "key_www","key_msdos", "key_coffee", "key_direction", 
            "key_cyclewindows", "key_mail", "key_bookmarks","key_computer", 
            "key_back", "key_forward", "key_colsecd", "key_ejectcd", 
            "key_ejectclosecd","key_nextsong","key_playpause","key_previoussong",
            "key_stopcd", "key_record","key_rewind", "key_phone", "key_iso", 
            "key_config", "key_homepage", "key_refresh", "key_exit","key_move", 
            "key_edit", "key_scrollup", "key_scrolldown", "key_kpleftparen", 
            "key_kprightparen","key_intl1", "key_intl2", "key_intl3","key_intl4", 
            "key_intl5", "key_intl6", "key_intl7","key_intl8", "key_intl9", 
            "key_lang1", "key_lang2", "key_lang3", "key_lang4", "key_lang5",
            "key_lang6", "key_lang7", "key_lang8", "key_lang9", "key_playcd", 
            "key_pausecd", "key_prog3","key_prog4", "key_suspend", "key_close", 
            "key_play", "key_fastforward", "key_bassboost","key_print", "key_hp",
            "key_camera", "key_sound", "key_question", "key_email", "key_chat",
            "key_search", "key_connect", "key_finance", "key_sport", "key_shop", 
            "key_alterase","key_cancel", "key_brightnessdown", "key_brightnessup", 
            "key_media"
        };
    return gensym(key_names[linux_code]);   // TODO: this should just return the char *
}




void hidio_print_element_list(t_hidio *x)
{
    debug_post(LOG_DEBUG,"hidio_print_element_list");
    unsigned long element_bitmask[EV_MAX][NBITS(KEY_MAX)];
//    char event_type_string[256];
//    char event_code_string[256];
    char *event_type_name = "";
    t_int i, j;
    /* counts for various event types */
    t_int syn_count,key_count,rel_count,abs_count,msc_count,led_count,
        snd_count,rep_count,ff_count,pwr_count,ff_status_count;

    /* get bitmask representing supported element (axes, keys, etc.) */
    memset(element_bitmask, 0, sizeof(element_bitmask));
    ioctl(x->x_fd, EVIOCGBIT(0, EV_MAX), element_bitmask[0]);
    post("\nSupported events:");
    
/* init all count vars */
    syn_count = key_count = rel_count = abs_count = msc_count = led_count = 0;
    snd_count = rep_count = ff_count = pwr_count = ff_status_count = 0;
    
    /* cycle through all possible event types 
     * i = i   j = j
     */
    for(i = 1; i < EV_MAX; i++) 
    {
        if(test_bit(i, element_bitmask[0])) 
        {
            /* make pretty names for event types */
            switch(i) 
            {
//            case EV_SYN: event_type_name = "Synchronization"; break;
            case EV_KEY: event_type_name = "Keys/Buttons"; break;
            case EV_REL: event_type_name = "Relative Axis"; break;
            case EV_ABS: event_type_name = "Absolute Axis"; break;
            case EV_MSC: event_type_name = "Miscellaneous"; break;
            case EV_LED: event_type_name = "LEDs"; break;
            case EV_SND: event_type_name = "System Sounds"; break;
            case EV_REP: event_type_name = "Autorepeat Values"; break;
            case EV_FF:  event_type_name = "Force Feedback"; break;
            case EV_PWR: event_type_name = "Power"; break;
            case EV_FF_STATUS: event_type_name = "Force Feedback Status"; break;
            default: event_type_name = "UNSUPPORTED"; 
            }
		 
            /* get bitmask representing supported button types */
            ioctl(x->x_fd, EVIOCGBIT(i, KEY_MAX), element_bitmask[i]);
		 
            post("");
            post("  TYPE\tCODE\tEVENT NAME");
            post("-----------------------------------------------------------");

            /* cycle through all possible event codes (axes, keys, etc.) 
             * testing to see which are supported.
             * i = i   j = j
             */
            for(j = 0; j < KEY_MAX; j++) 
            {
                if(test_bit(j, element_bitmask[i])) 
                {
                    if((i == EV_KEY) && (j >= BTN_MISC) && (j < KEY_OK) )
                    {
			t_symbol * hidio_codesym = hidio_convert_linux_buttons_to_numbers(j);
			if(hidio_codesym)
                        {
                            post("  %s\t%s\t%s (%s)",
                                 ev[i] ? ev[i] : "?", 
                                 hidio_codesym->s_name,
                                 event_type_name,
                                 event_names[i] ? (event_names[i][j] ? event_names[i][j] : "?") : "?");
			}
		    }
                    else if(i != EV_SYN)
                    {
                        post("  %s\t%s\t%s",
                             ev[i] ? ev[i] : "?", 
                             event_names[i][j] ? event_names[i][j] : "?", 
                             event_type_name);
                        
/* 	  post("    Event code %d (%s)", j, names[i] ? (names[i][j] ? names[i][j] : "?") : "?"); */
                    }
				  
                    switch(i) {
/* 
 * the API changed at some point...  EV_SYN seems to be the new name
 * from "Reset" events to "Syncronization" events
 */
/* #ifdef EV_RST */
/*                     case EV_RST: syn_count++; break; */
/* #else  */
/*                     case EV_SYN: syn_count++; break; */
/* #endif */
                    case EV_KEY: key_count++; break;
                    case EV_REL: rel_count++; break;
                    case EV_ABS: abs_count++; break;
                    case EV_MSC: msc_count++; break;
                    case EV_LED: led_count++; break;
                    case EV_SND: snd_count++; break;
                    case EV_REP: rep_count++; break;
                    case EV_FF:  ff_count++;  break;
                    case EV_PWR: pwr_count++; break;
                    case EV_FF_STATUS: ff_status_count++; break;
                    }
                }
            }
        }        
    }
    
    post("\nDetected:");
//    if(syn_count > 0) post ("  %d Synchronization types",syn_count);
    if(key_count > 0) post ("  %d Key/Button types",key_count);
    if(rel_count > 0) post ("  %d Relative Axis types",rel_count);
    if(abs_count > 0) post ("  %d Absolute Axis types",abs_count);
    if(msc_count > 0) post ("  %d Misc types",msc_count);
    if(led_count > 0) post ("  %d LED types",led_count);
    if(snd_count > 0) post ("  %d System Sound types",snd_count);
    if(rep_count > 0) post ("  %d Key Repeat types",rep_count);
    if(ff_count > 0) post ("  %d Force Feedback types",ff_count);
    if(pwr_count > 0) post ("  %d Power types",pwr_count);
    if(ff_status_count > 0) post ("  %d Force Feedback types",ff_status_count);
}


void hidio_print_device_list(void)
{
    debug_post(LOG_DEBUG,"hidio_print_device_list");
    int i,fd;
    char device_output_string[MAXPDSTRING] = "Unknown";
    char dev_handle_name[MAXPDSTRING] = "/dev/input/event0";

    post("");
    for(i=0;i<128;++i) 
    {
        snprintf(dev_handle_name, MAXPDSTRING, "/dev/input/event%d", i);
        if(dev_handle_name) 
        {
            /* open the device read-only, non-exclusive */
            fd = open (dev_handle_name, O_RDONLY | O_NONBLOCK);
            /* test if device open */
            if(fd < 0 ) 
            { 
                fd = -1;
            } 
            else 
            {
                /* get name of device */
                ioctl(fd, EVIOCGNAME(sizeof(device_output_string)), device_output_string);
                post("Device %d: '%s' on '%s'", i, device_output_string, dev_handle_name);
			  
                close (fd);
            }
        } 
    }
    post("");	
}



static void hidio_build_element_list(t_hidio *x) 
{
    debug_post(LOG_DEBUG,"hidio_build_element_list");
    unsigned long element_bitmask[EV_MAX][NBITS(KEY_MAX)];
    uint8_t abs_bitmask[ABS_MAX/8 + 1];
    struct input_absinfo abs_features;
    t_hid_element *new_element = NULL;
    t_int i, j;
  
    element_count[x->x_device_number] = 0;
    if( x->x_fd ) 
    {
        /* get bitmask representing supported elements (axes, keys, etc.) */
        memset(element_bitmask, 0, sizeof(element_bitmask));
        if( ioctl(x->x_fd, EVIOCGBIT(0, EV_MAX), element_bitmask[0]) < 0 )
            perror("[hidio] error: evdev ioctl: element_bitmask");
        memset(abs_bitmask, 0, sizeof(abs_bitmask));
        if( ioctl(x->x_fd, EVIOCGBIT(EV_ABS, sizeof(abs_bitmask)), abs_bitmask) < 0 ) 
            perror("[hidio] error: evdev ioctl: abs_bitmask");
        for( i = 1; i < EV_MAX; i++ ) 
        {
            if(test_bit(i, element_bitmask[0])) 
            {
                /* get bitmask representing supported button types */
                ioctl(x->x_fd, EVIOCGBIT(i, KEY_MAX), element_bitmask[i]);
                /* cycle through all possible event codes (axes, keys, etc.) 
                 * testing to see which are supported.
                 * i = i   j = j
                 */
                for(j = 0; j < KEY_MAX; j++) 
                {
					new_element = getbytes(sizeof(t_hid_element));
                    if( (i == EV_ABS) && (test_bit(j, abs_bitmask)) )
                    {
                        /* this means that the bit is set in the axes list */
                        if(ioctl(x->x_fd, EVIOCGABS(j), &abs_features)) 
                            perror("evdev EVIOCGABS ioctl");
                        new_element->min = abs_features.minimum;
                        new_element->max = abs_features.maximum;
                    }
                    else
                    {
                        new_element->min = 0;
                        new_element->max = 0;
                    }
                    if(test_bit(j, element_bitmask[i])) 
                    {
						new_element->linux_type = i; /* the int from linux/input.h */
						new_element->type = gensym(ev[i] ? ev[i] : "?"); /* the symbol */
                        new_element->linux_code = j;
                        if((i == EV_KEY) && (j >= BTN_MISC) && (j < KEY_OK) )
                        {
                            new_element->name = hidio_convert_linux_buttons_to_numbers(j);
                        }
                        else
                        {
                            new_element->name = gensym(event_names[i][j] ? event_names[i][j] : "?");
                        }
                        if( i == EV_REL )
                            new_element->relative = 1;
                        else
                            new_element->relative = 0;
						SETSYMBOL(new_element->output_message, new_element->name); 
						SETFLOAT(new_element->output_message + 1, new_element->instance);
                        // fill in the t_hid_element struct here
                        post("x->x_device_number: %d   element_count[]: %d",
                             x->x_device_number, element_count[x->x_device_number]);
                        post("linux_type/linux_code: %d/%d  type/name: %s/%s    max: %d   min: %d ", 
                             new_element->linux_type, new_element->linux_code, 
                             new_element->type->s_name, new_element->name->s_name,
                             new_element->max, new_element->min);
                        post("\tpolled: %d   relative: %d",
                             new_element->polled, new_element->relative);
                        element[x->x_device_number][element_count[x->x_device_number]] = new_element;
                        ++element_count[x->x_device_number];
                    }
                }
            }        
        }
    }
}

/* ------------------------------------------------------------------------------ */
/* Pd [hidio] FUNCTIONS */
/* ------------------------------------------------------------------------------ */

void hidio_get_events(t_hidio *x)
{
    debug_post(9,"hidio_get_events");

/* for debugging, counts how many events are processed each time hidio_read() is called */
    DEBUG(t_int event_counter = 0;);
    unsigned short i;
    t_hid_element *output_element = NULL;

/* this will go into the generic read function declared in hidio.h and
 * implemented in hidio_linux.c 
 */
    struct input_event hidio_input_event;

    if(x->x_fd < 0) return;

    while( read (x->x_fd, &(hidio_input_event), sizeof(struct input_event)) > -1 )
    {
        if( hidio_input_event.type != EV_SYN )
        {
            for( i=0; i < element_count[x->x_device_number]; ++i )
            {
				output_element = element[x->x_device_number][i];
                if( (hidio_input_event.type == output_element->linux_type) && \
                    (hidio_input_event.code == output_element->linux_code) )
                {
                    output_element->value = hidio_input_event.value;
					debug_post(9,"i: %d  linux_type: %d  linux_code: %d", i, 
								output_element->linux_type, output_element->linux_code);
                    debug_post(9,"value to output: %d",output_element->value);
                    break;
                }
            }
            if( output_element != NULL )
                hidio_output_event(x, output_element);
        }
        DEBUG(++event_counter;);
    }
    DEBUG(
        if(event_counter > 0)
        debug_post(8,"output %d events",event_counter);
	);
	
    return;
}


void hidio_print(t_hidio* x)
{
    hidio_print_device_list();
    hidio_print_element_list(x);
}


t_int hidio_open_device(t_hidio *x, short device_number)
{
    debug_post(LOG_DEBUG,"hidio_open_device");

    char device_name[MAXPDSTRING] = "Unknown";
    char block_device[MAXPDSTRING] = "/dev/input/event0";
    struct input_event hidio_input_event;

    x->x_fd = -1;
  
    x->x_device_number = device_number;
    snprintf(block_device,MAXPDSTRING,"/dev/input/event%d",x->x_device_number);

    if(block_device) 
    {
        /* open the device read-only, non-exclusive */
        x->x_fd = open(block_device, O_RDONLY | O_NONBLOCK);
        /* test if device open */
        if(x->x_fd < 0 ) 
        { 
            error("[hidio] open %s failed",block_device);
            x->x_fd = -1;
            return EXIT_FAILURE;
        }
    } 
  
    /* read input_events from the HID_DEVICE stream 
     * It seems that is just there to flush the input event queue
     */
    while (read (x->x_fd, &(hidio_input_event), sizeof(struct input_event)) > -1);

    /* get name of device */
    ioctl(x->x_fd, EVIOCGNAME(sizeof(device_name)), device_name);
    post ("[hidio] opened device %d (%s): %s",
          x->x_device_number,block_device,device_name);

    post("pre hidio_build_element_list");
    hidio_build_element_list(x);

    return EXIT_SUCCESS;
}

/* Under GNU/Linux, the device is a filehandle */
// TODO: return the same as POSIX open()/close() - 0=success, -1=fail
t_int hidio_close_device(t_hidio *x)
{
    debug_post(LOG_DEBUG,"hidio_close_device");
    if(x->x_fd > -1) 
        return (close(x->x_fd));
    else
        return EXIT_SUCCESS;
}


void hidio_build_device_list(void)
{
/*
 *	since in GNU/Linux the device list is the input event devices 
 *	(/dev/input/event?), nothing needs to be done as of yet to refresh 
 * the device list.  Once the device name can be other things in addition
 * the current t_float, then this will probably need to be changed.
 */
    int fd;
    unsigned int i;
    unsigned int last_active_device = 0;
    char device_name[MAXPDSTRING] = "Unknown";
    char block_device[MAXPDSTRING] = "/dev/input/event0";
    struct input_event  x_input_event; 
    
    debug_post(LOG_DEBUG,"hidio_build_device_list");
    
    debug_post(LOG_WARNING,"[hidio] Building device list...");
    
    for(i=0; i<MAX_DEVICES; ++i)
    {
        snprintf(block_device, MAXPDSTRING, "%s%d", LINUX_BLOCK_DEVICE, i);
        /* open the device read-only, non-exclusive */
        fd = open (block_device, O_RDONLY | O_NONBLOCK);
        /* test if device open */
        if(fd < 0 ) { 
            /* post("Nothing on %s.", &block_device); */
            fd = -1;
        } else {
            /* read input_events from the LINUX_BLOCK_DEVICE stream 
             * It seems that is just there to flush the event input buffer? */
            while( read(fd, &(x_input_event),sizeof(struct input_event)) > -1 );
			  
            /* get name of device */
            ioctl(fd, EVIOCGNAME(sizeof(device_name)), device_name);
            post("Found '%s' on '%s'",device_name, &block_device);

            close (fd);
        }
        last_active_device = i;

    }
    device_count = last_active_device ; // set the global variable
    debug_post(LOG_WARNING,"[hidio] completed device list.");
}



void hidio_platform_specific_free(t_hidio *x)
{
    /* nothing to be done here on GNU/Linux */
}




/* device info on the status outlet */
void hidio_platform_specific_info(t_hidio* x)
{
    struct input_id my_id;
    char device_name[MAXPDSTRING] = "Unknown";
    char vendor_id_string[7];
    char product_id_string[7];
    t_atom *output_atom = getbytes(sizeof(t_atom));

    ioctl(x->x_fd, EVIOCGID);
    snprintf(vendor_id_string,7,"0x%04x", my_id.vendor);
    SETSYMBOL(output_atom, gensym(vendor_id_string));
    outlet_anything( x->x_status_outlet, gensym("vendorID"), 
                     1, output_atom);
    snprintf(product_id_string,7,"0x%04x", my_id.product);
    SETSYMBOL(output_atom, gensym(product_id_string));
    outlet_anything( x->x_status_outlet, gensym("productID"), 
                     1, output_atom);
    ioctl(x->x_fd, EVIOCGNAME(sizeof(device_name)), device_name);
    SETSYMBOL(output_atom, gensym(device_name));
    outlet_anything( x->x_status_outlet, gensym("name"), 
                     1, output_atom);
    freebytes(output_atom,sizeof(t_atom));
}

        
short get_device_number_by_id(unsigned short vendor_id, unsigned short product_id)
{
    
    return -1;
}

short get_device_number_from_usage(short device_number, 
                                        unsigned short usage_page, 
                                        unsigned short usage)
{

    return -1;
}



/* ------------------------------------------------------------------------------ */
/*  FORCE FEEDBACK FUNCTIONS */
/* ------------------------------------------------------------------------------ */

/* cross-platform force feedback functions */
void hidio_ff_autocenter( t_hidio *x, t_float value )
{

}


void hidio_ff_gain( t_hidio *x, t_float value )
{

}


void hidio_ff_motors( t_hidio *x, t_float value )
{

}


void hidio_ff_continue( t_hidio *x )
{

}


void hidio_ff_pause( t_hidio *x )
{

}


void hidio_ff_reset( t_hidio *x )
{

}


void hidio_ff_stopall( t_hidio *x )
{

}



// these are just for testing...
void hidio_ff_fftest ( t_hidio *x, t_float value)
{

}


void hidio_ff_print( t_hidio *x )
{
}


#endif  /* #ifdef __linux__ */

