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

#include "hid.h"

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 * from evtest.c from the ff-utils package
 */

#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)	((array[LONG(bit)] >> OFF(bit)) & 1)


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

/* JMZ: i changed the convert functions (and the get-event function too!) to return
 * t_symbol* instead of writing into a fixed-sized buffer (which was way too
 * small and those made this object crash)
 * in order to change as little lines as possible the callback functions to the
 * hid-object still use (char*): so we convert a char[] into a symbol and then 
 * extract the (char*) out of it to make it a symbol again
 * LATER: use t_symbol's all over, since it is very flexible (with respect to length)
 * and sooner or later the strings are converted to t_symbol anyhow...
 *
 * Why? bug-fixing
 */

t_symbol* hid_convert_linux_buttons_to_numbers(__u16 linux_code)
{
  char hid_code[10];
    if(linux_code >= 0x100) 
    {
        if(linux_code < BTN_MOUSE)   
            sprintf(hid_code,"btn_%d",linux_code - BTN_MISC);  /* numbered buttons */
        else if(linux_code < BTN_JOYSTICK)
            sprintf(hid_code,"btn_%d",linux_code - BTN_MOUSE);  /* mouse buttons */
        else if(linux_code < BTN_GAMEPAD)
            sprintf(hid_code,"btn_%d",linux_code - BTN_JOYSTICK);  /* joystick buttons */
        else if(linux_code < BTN_DIGI)
            sprintf(hid_code,"btn_%d",linux_code - BTN_GAMEPAD);  /* gamepad buttons */
        else if(linux_code < BTN_WHEEL)
            sprintf(hid_code,"btn_%d",linux_code - BTN_DIGI);  /* tablet buttons */
        else if(linux_code < KEY_OK)
            sprintf(hid_code,"btn_%d",linux_code - BTN_WHEEL);  /* wheel buttons */
	else return 0;
    }
    return gensym(hid_code);
}

/* Georg Holzmann: implementation of the keys */
/* JMZ: use t_symbol instead of char[] (s.a.) AND 
 * appended "key_" in the array so we don't have to append it each time AND
 * made the table static
 */
t_symbol* hid_convert_linux_keys(__u16 linux_code)
{  
    if(linux_code > 226)
        return 0;
    /*         quick hack to get the keys         */
    /* (in future this should be auto-generated)  */

    static char key_names[227][20] =
        { 
            "key_reserved", "key_esc", "key_1", "key_2", "key_3", "key_4", "key_5", "key_6", "key_7",
            "key_8", "key_9", "key_0", "key_minus", "key_equal", "key_backspace", "key_tab",
            "key_q", "key_w", "key_e", "key_r", "key_t", "key_y", "key_u", "key_i", "key_o", "key_p",
            "key_leftbrace", "key_rightbrace", "key_enter", "key_leftctrl", "key_a",
            "key_s", "key_d", "key_f", "key_g", "key_h", "key_j", "key_k", "key_l", "key_semicolon",
            "key_apostrophe", "key_grave", "key_leftshift", "key_backslash", "key_z",
            "key_x", "key_c", "key_v", "key_b", "key_n", "key_m", "key_comma", "key_dot", "key_slash",
            "key_rightshift", "key_kpasterisk", "key_leftalt", "key_space", "key_capslock",
            "key_f1", "key_f2", "key_f3", "key_f4", "key_f5", "key_f6", "key_f7", "key_f8", "key_f9", "key_f10",
            "key_numlock", "key_scrolllock", "key_kp7", "key_kp8", "key_kp9", "key_kpminus",
            "key_kp4", "key_kp5", "key_kp6", "key_kpplus", "key_kp1", "key_kp2", "key_kp3", "key_kp3",  "key_kpdot",
            "key_103rd", "key_f13", "key_102nd", "key_f11", "key_f12", "key_f14", "key_f15", "key_f16",
            "key_f17", "key_f18", "key_f19", "key_f20", "key_kpenter", "key_rightctrl", "key_kpslash",
            "key_sysrq", "key_rightalt", "key_linefeed", "key_home", "key_up", "key_pageup", "key_left",
            "key_right", "key_end", "key_down", "key_pagedown", "key_insert", "key_delete", "key_macro",
            "key_mute", "key_volumedown", "key_volumeup", "key_power", "key_kpequal", "key_kpplusminus",
            "key_pause", "key_f21", "key_f22", "key_f23", "key_f24", "key_kpcomma", "key_leftmeta",
            "key_rightmeta", "key_compose",
    
            "key_stop", "key_again", "key_props", "key_undo", "key_front", "key_copy", "key_open",
            "key_paste", "key_find", "key_cut", "key_help", "key_menu", "key_calc", "key_setup", "key_sleep", "key_wakeup",
            "key_file", "key_sendfile", "key_deletefile", "key_xfer", "key_prog1", "key_prog2", "key_www",
            "key_msdos", "key_coffee", "key_direction", "key_cyclewindows", "key_mail", "key_bookmarks",
            "key_computer", "key_back", "key_forward", "key_colsecd", "key_ejectcd", "key_ejectclosecd",
            "key_nextsong", "key_playpause", "key_previoussong", "key_stopcd", "key_record",
            "key_rewind", "key_phone", "key_iso", "key_config", "key_homepage", "key_refresh", "key_exit",
            "key_move", "key_edit", "key_scrollup", "key_scrolldown", "key_kpleftparen", "key_kprightparen",
    
            "key_intl1", "key_intl2", "key_intl3", "key_intl4", "key_intl5", "key_intl6", "key_intl7",
            "key_intl8", "key_intl9", "key_lang1", "key_lang2", "key_lang3", "key_lang4", "key_lang5",
            "key_lang6", "key_lang7", "key_lang8", "key_lang9", "key_playcd", "key_pausecd", "key_prog3",
            "key_prog4", "key_suspend", "key_close", "key_play", "key_fastforward", "key_bassboost",
            "key_print", "key_hp", "key_camera", "key_sound", "key_question", "key_email", "key_chat",
            "key_search", "key_connect", "key_finance", "key_sport", "key_shop", "key_alterase",
            "key_cancel", "key_brightnessdown", "key_brightnessup", "key_media"
        };
    return gensym(key_names[linux_code]);
}

void hid_print_element_list(t_hid *x)
{
    DEBUG(post("hid_print_element_list"););
    unsigned long bitmask[EV_MAX][NBITS(KEY_MAX)];
//    char event_type_string[256];
//    char event_code_string[256];
    char *event_type_name = "";
    t_int i, j;
    /* counts for various event types */
    t_int syn_count,key_count,rel_count,abs_count,msc_count,led_count,snd_count,rep_count,ff_count,pwr_count,ff_status_count;

    /* get bitmask representing supported events (axes, keys, etc.) */
    memset(bitmask, 0, sizeof(bitmask));
    ioctl(x->x_fd, EVIOCGBIT(0, EV_MAX), bitmask[0]);
    post("\nSupported events:");
    
/* init all count vars */
    syn_count = key_count = rel_count = abs_count = msc_count = led_count = 0;
    snd_count = rep_count = ff_count = pwr_count = ff_status_count = 0;
    
    /* cycle through all possible event types 
     * i = i   j = j
     */
    for (i = 1; i < EV_MAX; i++) 
    {
        if (test_bit(i, bitmask[0])) 
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
            ioctl(x->x_fd, EVIOCGBIT(i, KEY_MAX), bitmask[i]);
		 
            post("");
            post("  TYPE\tCODE\tEVENT NAME");
            post("-----------------------------------------------------------");

            /* cycle through all possible event codes (axes, keys, etc.) 
             * testing to see which are supported.
             * i = i   j = j
             */
            for (j = 0; j < KEY_MAX; j++) 
            {
                if (test_bit(j, bitmask[i])) 
                {
                    if ((i == EV_KEY) && (j >= BTN_MISC) && (j < KEY_OK) )
                    {
			t_symbol*hid_codesym=hid_convert_linux_buttons_to_numbers(j);
			if(hid_codesym){
			  post("  %s\t%s\t%s",
                             ev[i] ? ev[i] : "?", 
                             hid_codesym->s_name,
                             event_names[i] ? (event_names[i][j] ? event_names[i][j] : "?") : "?");
			}
		    }
                    else if (i != EV_SYN)
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
//    if (syn_count > 0) post ("  %d Synchronization types",syn_count);
    if (key_count > 0) post ("  %d Key/Button types",key_count);
    if (rel_count > 0) post ("  %d Relative Axis types",rel_count);
    if (abs_count > 0) post ("  %d Absolute Axis types",abs_count);
    if (msc_count > 0) post ("  %d Misc types",msc_count);
    if (led_count > 0) post ("  %d LED types",led_count);
    if (snd_count > 0) post ("  %d System Sound types",snd_count);
    if (rep_count > 0) post ("  %d Key Repeat types",rep_count);
    if (ff_count > 0) post ("  %d Force Feedback types",ff_count);
    if (pwr_count > 0) post ("  %d Power types",pwr_count);
    if (ff_status_count > 0) post ("  %d Force Feedback types",ff_status_count);
}


void hid_print_device_list(void)
{
    DEBUG(post("hid_print_device_list"););
    int i,fd;
    char device_output_string[256] = "Unknown";
    char dev_handle_name[20] = "/dev/input/event0";

    post("");
    for (i=0;i<128;++i) 
    {
        sprintf(dev_handle_name,"/dev/input/event%d",i);
        if (dev_handle_name) 
        {
            /* open the device read-only, non-exclusive */
            fd = open (dev_handle_name, O_RDONLY | O_NONBLOCK);
            /* test if device open */
            if (fd < 0 ) 
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

/* ------------------------------------------------------------------------------ */
/*  FORCE FEEDBACK FUNCTIONS */
/* ------------------------------------------------------------------------------ */

/* cross-platform force feedback functions */
t_int hid_ff_autocenter( t_hid *x, t_float value )
{
    return ( 0 );
}


t_int hid_ff_gain( t_hid *x, t_float value )
{
    return ( 0 );
}


t_int hid_ff_motors( t_hid *x, t_float value )
{
    return ( 0 );
}


t_int hid_ff_continue( t_hid *x )
{
    return ( 0 );
}


t_int hid_ff_pause( t_hid *x )
{
    return ( 0 );
}


t_int hid_ff_reset( t_hid *x )
{
    return ( 0 );
}


t_int hid_ff_stopall( t_hid *x )
{
    return ( 0 );
}



// these are just for testing...
t_int hid_ff_fftest ( t_hid *x, t_float value)
{
    return ( 0 );
}


void hid_ff_print( t_hid *x )
{
}



/* ------------------------------------------------------------------------------ */
/* Pd [hid] FUNCTIONS */
/* ------------------------------------------------------------------------------ */

t_int hid_get_events(t_hid *x)
{
    DEBUG(post("hid_get_events"););

/* for debugging, counts how many events are processed each time hid_read() is called */
    DEBUG(t_int event_counter = 0;);

    t_symbol*hid_code=0;

/* this will go into the generic read function declared in hid.h and
 * implemented in hid_linux.c 
 */
    struct input_event hid_input_event;

    if (x->x_fd < 0) return 0;

    while( read (x->x_fd, &(hid_input_event), sizeof(struct input_event)) > -1 )
    {
        hid_code=0;
        if( hid_input_event.type == EV_KEY )
        {
	  /* JMZ: originally both functions were called, the latter evtl. overwriting
	   * the former; now i only call the latter if the former does not return 
	   * a valid result
	   */
	  if(!(hid_code=hid_convert_linux_buttons_to_numbers(hid_input_event.code)))
            hid_code=hid_convert_linux_keys(hid_input_event.code);
        }
        else if( hid_input_event.type == EV_SYN )
        {
            // filter out EV_SYN events, they are currently unused
        }
        else if( event_names[hid_input_event.type][hid_input_event.code] != NULL )
        {
	  hid_code=gensym(event_names[hid_input_event.type][hid_input_event.code]);
        }
        else 
        {
	  hid_code=gensym("unknown");
        }
        if( hid_code && hid_input_event.type != EV_SYN )
            hid_output_event(x, ev[hid_input_event.type], hid_code->s_name, 
                             (t_float)hid_input_event.value);
        DEBUG(++event_counter;);
    }
    DEBUG(
        //if (event_counter > 0)
        //post("output %d events",event_counter);
	);
	
    return (0);
}


void hid_print(t_hid* x)
{
    hid_print_device_list();
    hid_print_element_list(x);
}


t_int hid_open_device(t_hid *x, t_int device_number)
{
    DEBUG(post("hid_open_device"););

    char device_name[256] = "Unknown";
    char dev_handle_name[20] = "/dev/input/event0";
    struct input_event hid_input_event;

    x->x_fd = -1;
  
    x->x_device_number = device_number;
    sprintf(dev_handle_name,"/dev/input/event%d",(int)x->x_device_number);

    if (dev_handle_name) 
    {
        /* open the device read-only, non-exclusive */
        x->x_fd = open(dev_handle_name, O_RDONLY | O_NONBLOCK);
        /* test if device open */
        if (x->x_fd < 0 ) 
        { 
            error("[hid] open %s failed",dev_handle_name);
            x->x_fd = -1;
            return 1;
        }
    } 
  
    /* read input_events from the HID_DEVICE stream 
     * It seems that is just there to flush the input event queue
     */
    while (read (x->x_fd, &(hid_input_event), sizeof(struct input_event)) > -1);

    /* get name of device */
    ioctl(x->x_fd, EVIOCGNAME(sizeof(device_name)), device_name);
    post ("[hid] opened device %d (%s): %s",
          x->x_device_number,dev_handle_name,device_name);

    return (0);
}

/*
 * Under GNU/Linux, the device is a filehandle
 */
t_int hid_close_device(t_hid *x)
{
    DEBUG(post("hid_close_device"););
    if (x->x_fd <0) 
        return 0;
    else
        return (close(x->x_fd));
}

t_int hid_build_device_list(t_hid *x)
{
    DEBUG(post("hid_build_device_list"););
    /* the device list should be refreshed here */
/*
 *	since in GNU/Linux the device list is the input event devices 
 *	(/dev/input/event?), nothing needs to be done as of yet to refresh 
 * the device list.  Once the device name can be other things in addition
 * the current t_float, then this will probably need to be changed.
 */

    return (0);
}

void hid_platform_specific_free(t_hid *x)
{
    /* nothing to be done here on GNU/Linux */
}



#endif  /* #ifdef __linux__ */

