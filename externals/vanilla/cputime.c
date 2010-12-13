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

#if defined (__APPLE__) || defined (__FreeBSD__)
#define CLOCKHZ CLK_TCK
#endif
#if defined (__linux__)
#define CLOCKHZ sysconf(_SC_CLK_TCK)
#endif

static t_class *cputime_class;

typedef struct _cputime
{
    t_object x_obj;
#ifdef UNISTD
    struct tms x_setcputime;
#endif
#ifdef MSW
    LARGE_INTEGER x_kerneltime;
    LARGE_INTEGER x_usertime;
    int x_warned;
#endif
} t_cputime;

static void cputime_bang(t_cputime *x)
{
#ifdef UNISTD
    times(&x->x_setcputime);
#endif
#ifdef MSW
    FILETIME ignorethis, ignorethat;
    BOOL retval;
    retval = GetProcessTimes(GetCurrentProcess(), &ignorethis, &ignorethat,
        (FILETIME *)&x->x_kerneltime, (FILETIME *)&x->x_usertime);
    if (!retval)
    {
        if (!x->x_warned)
            post("cputime is apparently not supported on your platform");
        x->x_warned = 1;
        x->x_kerneltime.QuadPart = 0;
        x->x_usertime.QuadPart = 0;
    }
#endif
}

static void cputime_bang2(t_cputime *x)
{
#ifdef UNISTD
    t_float elapsedcpu;
    struct tms newcputime;
    times(&newcputime);
    elapsedcpu = 1000 * (
        newcputime.tms_utime + newcputime.tms_stime -
            x->x_setcputime.tms_utime - x->x_setcputime.tms_stime) / CLOCKHZ;
    outlet_float(x->x_obj.ob_outlet, elapsedcpu);
#endif
#ifdef MSW
    t_float elapsedcpu;
    FILETIME ignorethis, ignorethat;
    LARGE_INTEGER usertime, kerneltime;
    BOOL retval;
    
    retval = GetProcessTimes(GetCurrentProcess(), &ignorethis, &ignorethat,
        (FILETIME *)&kerneltime, (FILETIME *)&usertime);
    if (retval)
        elapsedcpu = 0.0001 *
            ((kerneltime.QuadPart - x->x_kerneltime.QuadPart) +
                (usertime.QuadPart - x->x_usertime.QuadPart));
    else elapsedcpu = 0;
    outlet_float(x->x_obj.ob_outlet, elapsedcpu);
#endif
}

static void *cputime_new(void)
{
    t_cputime *x = (t_cputime *)pd_new(cputime_class);
    outlet_new(&x->x_obj, gensym("float"));

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("bang"), gensym("bang2"));
#ifdef MSW
    x->x_warned = 0;
#endif
    cputime_bang(x);
    return (x);
}

static void cputime_setup(void)
{
    cputime_class = class_new(gensym("cputime"), (t_newmethod)cputime_new, 0,
        sizeof(t_cputime), 0, 0);
    class_addbang(cputime_class, cputime_bang);
    class_addmethod(cputime_class, (t_method)cputime_bang2, gensym("bang2"), 0);
}
