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

#include <unistd.h>
#include <string.h>

static t_class *dmxout_class;
static t_class *dmxout_class2;

typedef struct _dmxout
{
  t_object x_obj;

  t_inlet *x_portinlet;

  int      x_device;
  t_float  x_port;
  int  x_portrange;

} t_dmxout;

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
  const char *args[2] = {"--dmx", s_devname->s_name};
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

static void dmxout_doout(t_dmxout*x, short baseport, short portrange, dmx_t*values)
{
  if(x->x_device<=0) {
    pd_error(x, "no DMX universe found");
    return;
  }

  lseek (x->x_device, sizeof(dmx_t)*baseport, SEEK_SET);  /* set to the current channel */
  write (x->x_device, values, portrange*sizeof(dmx_t)); /* write the channel */
}

static void dmxout_doout1(t_dmxout*x, short port, unsigned char value)
{
  dmx_t buffer[1] = {value};
  dmxout_doout(x, port, 1, buffer);
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

  dmxout_doout1(x, port, val);
}

static void dmxout_list(t_dmxout*x, t_symbol*s, int argc, t_atom*argv)
{
  int count=(argc<x->x_portrange)?argc:x->x_portrange;
  dmx_t*buffer=(dmx_t*)getbytes(count*sizeof(dmx_t));
  int i=0;

  int errors=0;

  for(i=0; i<count; i++) {
    t_float f=atom_getfloat(argv+i);
    if(f<0. || f>255.) {
      errors++;
      if(f<0.)f=0.;
      if(f>255)f=255;
    }
    buffer[i]=(unsigned char)f;
  }
  if(errors) {
    pd_error(x, "%d valu%s out of bound [0..255]", errors, (1==errors)?"e":"es");
  }

  dmxout_doout(x, x->x_port, count, buffer);
}

static void dmxout_port(t_dmxout*x, t_float f_baseport, t_floatarg f_portrange)
{
  short baseport =(short)f_baseport;
  short portrange=(short)f_portrange;


  if(baseport<0 || baseport>=512) {
    pd_error(x, "port %f out of bounds [0..512]", f_baseport);
    baseport =0;
  }
  x->x_port = baseport;

  if(portrange<0) {
    pd_error(x, "portrange %f<0! setting to 1", portrange);
    portrange=1;
  } else if (portrange==0) {
    portrange=x->x_portrange;
  }

  if (baseport+portrange>512) {
    pd_error(x, "upper port exceeds 512! clamping");
    portrange=512-baseport;
  }
  x->x_portrange=portrange;
}

static void *dmxout_new(t_symbol*s, int argc, t_atom*argv)
{
  t_floatarg baseport=0.f, portrange=0.f;
  t_dmxout *x = 0;

  switch(argc) {
  case 2:
    x=(t_dmxout *)pd_new(dmxout_class2);
    x->x_portinlet=inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("port"));
    baseport=atom_getfloat(argv);
    portrange=atom_getfloat(argv+1);
    dmxout_port(x, baseport, portrange);
    break;
  case 1:
    baseport=atom_getfloat(argv);
  case 0:
    x=(t_dmxout *)pd_new(dmxout_class);
    x->x_portinlet=floatinlet_new(&x->x_obj, &x->x_port);
    x->x_port  = baseport;
    x->x_portrange = -1;
    break;
  default:
    return 0;
  }
  x->x_device=-1;

  dmxout_open(x, gensym(""));
  return (x);
}

static void *dmxout_free(t_dmxout*x)
{
  dmxout_close(x);
}


void dmxout_setup(void)
{
  dmxout_class = class_new(gensym("dmxout"), (t_newmethod)dmxout_new, (t_method)dmxout_free,
                           sizeof(t_dmxout), 
                           0,
                           A_GIMME, A_NULL);

  class_addfloat(dmxout_class, dmxout_float);

  dmxout_class2 = class_new(gensym("dmxout"), (t_newmethod)dmxout_new, (t_method)dmxout_free,
			    sizeof(t_dmxout), 
			    0,
			    A_GIMME, A_NULL);

  class_addlist(dmxout_class2, dmxout_list);


  class_addmethod(dmxout_class2, (t_method)dmxout_port, gensym("port"), 
		  A_FLOAT, A_DEFFLOAT, A_NULL);

#ifdef DMX4PD_POSTBANNER
  DMX4PD_POSTBANNER
#endif
}
