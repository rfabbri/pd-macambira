/* code for foo1 pd class */

#include "m_pd.h"

typedef struct foo1
{
  t_object t_ob;
} t_foo1;

void foo1_float(t_foo1 *x, t_floatarg f)
{
    post("foo1: %f", f);
}

void foo1_rats(t_foo1 *x)
{
    post("foo1: rats");
}

t_class *foo1_class;

void *foo1_new(void)
{
    t_foo1 *x = (t_foo1 *)pd_new(foo1_class);
    post("foo1_new");
    return (void *)x;
}

void foo1_setup(void)
{
    post("foo1_setup");
    foo1_class = class_new(gensym("foo1"), (t_newmethod)foo1_new, 0,
    	sizeof(t_foo1), 0, 0);
    class_addmethod(foo1_class, (t_method)foo1_rats, gensym("rats"), 0);
    class_addfloat(foo1_class, foo1_float);
}

