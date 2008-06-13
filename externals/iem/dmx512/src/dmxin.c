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
} t_dmxin;

static void *dmxin_new(void)
{
  t_dmxin *x = (t_dmxin *)pd_new(dmxin_class);
  return (x);
}
static void *dmxin_free(t_dmxin*x)
{

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
