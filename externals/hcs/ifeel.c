/* 
 * ifeel mouse object for Miller Puckette's Pure Data
 * copyright 2003 Hans-Christoph Steiner <hans@eds.org>

 * This program is free software; you can redistribute it and/or               
 * modify it under the terms of the GNU General Public License                 
 * as published by the Free Software Foundation; either version 2              
 * of the License, or (at your option) any later version.                      
 * This program is distributed in the hope that it will be useful,             
 * but WITHOUT ANY WARRANTY; without even the implied warranty of              
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               
 * GNU General Public License for more details.                                
 * You should have received a copy of the GNU General Public License           
 * along with this program; if not, write to the Free Software                 
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
 *
 *
 * based on ifeel_send.c from the linux iFeel driver:
 * http://sourceforge.net/projects/tactile
 *
 */

#include "m_pd.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "ifeel.h"


#define IFEEL_DEVICE    "/dev/input/ifeel0"

static t_class *ifeel_class;

typedef struct _ifeel {
	t_object x_obj;
	int x_fd;
	struct ifeel_command x_ifeel_command;
} t_ifeel;



/******************************************************************************
general ff methods
******************************************************************************/

void ifeel_start(t_ifeel *x)
{
  post("ifeel_start");
  
  if (ioctl(x->x_fd, USB_IFEEL_BUZZ_IOCTL, &x->x_ifeel_command) < 0) {
    printf("ERROR %s\n", strerror(errno));
  }  
}

void ifeel_stop(t_ifeel *x)
{
  post("ifeel_stop");
  
  x->x_ifeel_command.strength = 0;
  x->x_ifeel_command.delay = 0;
  x->x_ifeel_command.count = 0;
  if (ioctl(x->x_fd, USB_IFEEL_BUZZ_IOCTL, &x->x_ifeel_command) < 0) {
    printf("ERROR %s\n", strerror(errno));
  }  
}

void ifeel_level(t_ifeel *x, t_floatarg level)
{
  post("ifeel_level");
  
  /* make sure its in the proper range */
  level = level * 255;
  level = (level > 255 ? 255 : level);
  level = (level < 0 ? 0 : level);

  x->x_ifeel_command.strength  = (unsigned int)level;

  if (ioctl(x->x_fd, USB_IFEEL_BUZZ_IOCTL, &x->x_ifeel_command) < 0) {
    printf("ERROR %s\n", strerror(errno));
  }  
  
}

void ifeel_interval(t_ifeel *x, t_floatarg interval)
{
  post("ifeel_interval");
  
  interval = (interval < 0 ? 0 : interval);

  x->x_ifeel_command.delay  = (unsigned int)interval;

  if (ioctl(x->x_fd, USB_IFEEL_BUZZ_IOCTL, &x->x_ifeel_command) < 0) {
    printf("ERROR %s\n", strerror(errno));
  }  
}

void ifeel_count(t_ifeel *x, t_floatarg count )
{
  post("ifeel_count");
  
  count = (count < 0 ? 0 : count);

  x->x_ifeel_command.count  = (unsigned int)count;

  if (ioctl(x->x_fd, USB_IFEEL_BUZZ_IOCTL, &x->x_ifeel_command) < 0) {
    printf("ERROR %s\n", strerror(errno));
  }  
}

void ifeel_free(t_ifeel *x)
{
  post("ifeel_free");
  
  /* stop effect */
  ifeel_stop(x);
  
  /* close device */
  close(x->x_fd);
}

void *ifeel_new(t_symbol *device, t_floatarg level, t_floatarg interval, t_floatarg count)
{
  post("ifeel_new");
  
  t_ifeel *x = (t_ifeel *)pd_new(ifeel_class);
  
  /* 
   * init to zero so I can use the ifeel_* methods to set the 
   * struct with the argument values
   */
  x->x_ifeel_command.strength = 0;
  x->x_ifeel_command.delay = 0;
  x->x_ifeel_command.count = 0;
  
  inlet_new(&x->x_obj,
	    &x->x_obj.ob_pd,
	    gensym("float"),
	    gensym("direction"));
  inlet_new(&x->x_obj,
	    &x->x_obj.ob_pd,
	    gensym("float"),
	    gensym("count"));
  inlet_new(&x->x_obj,
	    &x->x_obj.ob_pd,
	    gensym("float"),
	    gensym("level"));
  
  if (device != &s_) {
    post("Using %s",device->s_name);
    
    /* x->x_fd = open(IFEEL_DEVICE, O_RDWR); */
    /*   x->x_fd = open((char *) device->s_name, O_RDWR); */
    if ((x->x_fd = open((char *) device->s_name, O_RDWR | O_NONBLOCK, 0)) <= 0) {
      printf("ERROR %s\n", strerror(errno)); 
    }
    
    ifeel_level(x,level);
    ifeel_interval(x,interval);
    ifeel_count(x,count);
  }
  else {
    post("ifeel: You need to set an ifeel device (i.e /dev/input/ifeel0)");
  }
  
  return (void*)x;
}

/******************************************************************************
   initialisation functions
******************************************************************************/

void ifeel_setup(void)
{
  post("ifeel_setup");
  
  ifeel_class = class_new(gensym("ifeel"),
			  (t_newmethod)ifeel_new,
			  (t_method)ifeel_free,
			  sizeof(t_ifeel),
			  CLASS_DEFAULT,
			  A_DEFSYMBOL,
			  A_DEFFLOAT,
			  A_DEFFLOAT,
			  A_DEFFLOAT,
			  0);
  
  class_addbang(ifeel_class,ifeel_start);

  class_addmethod(ifeel_class, (t_method)ifeel_stop,gensym("start"),0);
  class_addmethod(ifeel_class, (t_method)ifeel_stop,gensym("stop"),0);
  
  class_addmethod(ifeel_class, (t_method)ifeel_level,  gensym("level"), A_DEFFLOAT,0);  
  class_addmethod(ifeel_class, (t_method)ifeel_interval,gensym("interval"),A_DEFFLOAT,0);
  class_addmethod(ifeel_class, (t_method)ifeel_count,gensym("count"),A_DEFFLOAT,0);

 }


