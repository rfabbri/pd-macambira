/******************************************************
 *
 * dmxout - implementation file
 *
 * copyleft (c) IOhannes m zmölnig
 *
 *   0603:forum::für::umläute:2008
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/


#include "dmx4pd.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>


#include <dmx/dmx.h>

static t_class *dmxout_class;

typedef struct _dmxout
{
  t_object x_obj;

  t_inlet *x_portinlet;

  int      x_device;
  t_float  x_port;

} t_dmxout;


static void dmxout_doout(t_dmxout*x, short port, unsigned char value)
{
  dmx_t buffer[1] = {value};
  if(x->x_device<=0) {
    pd_error(x, "no DMX universe found");
    return;
  }

  lseek (x->x_device, sizeof(buffer)*port, SEEK_SET);  /* set to the current channel */
  write (x->x_device, buffer, sizeof(buffer)); /* write the channel */
}


static void dmxout_close(t_dmxout*x)
{
  if(x->x_device>=0) {
    close(x->x_device);
  }
  x->x_device=-1;
}


static void dmxout_open(t_dmxout*x, t_symbol*s_devname)
{
  int argc=2;
  char *args[2] = {"--dmx", s_devname->s_name};
  const char**argv=args;
  char*devname="";
  int fd;

  if(s_devname && s_devname->s_name)
    devname=s_devname->s_name;

  //  strncpy(args[0], "--dmx", MAXPDSTRING);
  //  strncpy(args[1], devname, MAXPDSTRING);

  fd = open (DMXdev(&argc, argv), O_WRONLY);

  if(fd!=-1) {
    dmxout_close(x);
    x->x_device=fd;
  }
}


static void dmxout_float(t_dmxout*x, t_float f)
{
  unsigned char val=(unsigned char)f;
  short port = (short)x->x_port;
  if(f<0. || f>255.) {
    pd_error(x, "value %f out of bounds [0..255]", f);
    return;
  }
  if(x->x_port<0. || x->x_port>512.) {
    pd_error(x, "port %f out of bounds [0..512]", x->x_port);
    return;
  }

  dmxout_doout(x, port, val);
}

static void *dmxout_new(t_symbol*s, int argc, t_atom*argv)
{
  t_dmxout *x = (t_dmxout *)pd_new(dmxout_class);

  t_symbol*devname=gensym("");

  x->x_portinlet=floatinlet_new(&x->x_obj, &x->x_port);

  x->x_device=-1;
  x->x_port  =0;

  switch(argc) {
  case 0: break;
  case 1:
    if(A_FLOAT==argv->a_type) {
      x->x_port=atom_getfloat(argv);
    } else {
      devname=atom_getsymbol(argv);
    }
    break;
  default:
    if((A_FLOAT==(argv+0)->a_type) && (A_SYMBOL==(argv+1)->a_type)) {
      x->x_port=atom_getfloat(argv+0);
      devname=atom_getsymbol(argv+1);
    } else if(A_FLOAT==(argv+1)->a_type && A_SYMBOL==(argv+0)->a_type) {
      x->x_port=atom_getfloat(argv+1);
      devname=atom_getsymbol (argv+0);
    }
    break;
  }


  dmxout_open(x, devname);
  return (x);
}

static void *dmxout_free(t_dmxout*x)
{

}


void dmxout_setup(void)
{
  dmxout_class = class_new(gensym("dmxout"), (t_newmethod)dmxout_new, (t_method)dmxout_free,
                           sizeof(t_dmxout), 
                           0,
                           A_GIMME, A_NULL);

  class_addfloat(dmxout_class, dmxout_float);


  post("DMX4PD (ver.%s): (c) 2008 IOhannes m zmölnig - iem @ kug", DMX4PD_VERSION);
}
