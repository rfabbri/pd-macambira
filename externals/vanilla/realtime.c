/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* misc. */

#include "m_pd.h"
#include "s_stuff.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#ifdef UNISTD
#include <sys/types.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/param.h>
#include <unistd.h>
#endif
#ifdef MSW
#include <wtypes.h>
#include <time.h>
#endif

static t_class *realtime_class;

typedef struct _realtime
{
    t_object x_obj;
    double x_setrealtime;
} t_realtime;

static void realtime_bang(t_realtime *x)
{
    x->x_setrealtime = sys_getrealtime();
}

static void realtime_bang2(t_realtime *x)
{
    outlet_float(x->x_obj.ob_outlet,
        (sys_getrealtime() - x->x_setrealtime) * 1000.);
}

static void *realtime_new(void)
{
    t_realtime *x = (t_realtime *)pd_new(realtime_class);
    outlet_new(&x->x_obj, gensym("float"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("bang"), gensym("bang2"));
    realtime_bang(x);
    return (x);
}

void realtime_setup(void)
{
    realtime_class = class_new(gensym("realtime"), (t_newmethod)realtime_new, 0,
        sizeof(t_realtime), 0, 0);
    class_addbang(realtime_class, realtime_bang);
    class_addmethod(realtime_class, (t_method)realtime_bang2, gensym("bang2"),
        0);
}
