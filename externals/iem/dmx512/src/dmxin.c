/******************************************************
 *
 * dmxin - implementation file
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


static t_class *dmxin_class;

typedef struct _dmxin
{
  t_object x_obj;
  int      x_device;

  t_outlet*x_outlet1, *x_outlet2;
} t_dmxin;




static void dmxin_close(t_dmxin*x)
{
  if(x->x_device>=0) {
    close(x->x_device);
  }
  x->x_device=-1;
}


static void dmxin_open(t_dmxin*x, t_symbol*s_devname)
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

  fd = open (DMXINdev(&argc, argv), O_RDONLY);

  if(fd!=-1) {
    dmxin_close(x);
    x->x_device=fd;
  }
}


static void *dmxin_new(void)
{
  if(0) {
    t_dmxin *x = (t_dmxin *)pd_new(dmxin_class);
    return (x);
  } else {
    error("[dmxin] not yet implemented");
    return 0;
  }
}
static void *dmxin_free(t_dmxin*x)
{
  dmxin_close(x);
}

void dmxin_setup(void)
{
  dmxin_class = class_new(gensym("dmxin"), (t_newmethod)dmxin_new, (t_method)dmxin_free,
                          sizeof(t_dmxin),  
                          CLASS_NOINLET,
                          A_NULL);

#ifdef DMX4PD_POSTBANNER
  DMX4PD_POSTBANNER
#endif
}
