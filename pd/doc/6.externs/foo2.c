/* code for foo2 pd class */

#include "m_pd.h"
 
typedef struct foo2
{
  t_object t_ob;
} t_foo2;

void foo2_float(t_foo2 *x, t_floatarg f)
{
    post("foo2: %f", f);
}

void foo2_rats(t_foo2 *x)
{
    post("foo2: rats");
}

void foo2_ft1(t_foo2 *x, t_floatarg g)
{
    post("ft1: %f", g);
}

void foo2_free(void)
{
    post("foo2_free");
}

t_class *foo2_class;

void *foo2_new(void)
{
    t_foo2 *x = (t_foo2 *)pd_new(foo2_class);
    inlet_new(&x->t_ob, &x->t_ob.ob_pd, gensym("float"), gensym("ft1"));
    post("foo2_new");
    return (void *)x;
}

void foo2_setup(void)
{
    post("foo2_setup");
    foo2_class = class_new(gensym("foo2"), (t_newmethod)foo2_new,
    	(t_method)foo2_free, sizeof(t_foo2), 0, 0);
    class_addmethod(foo2_class, (t_method)foo2_rats, gensym("rats"), 0);
    class_addmethod(foo2_class, (t_method)foo2_ft1, gensym("ft1"), A_FLOAT, 0);
    class_addfloat(foo2_class, foo2_float);
}

