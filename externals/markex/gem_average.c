/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
 */

#include "m_pd.h"

/* -------------------------- alternate ------------------------------ */

/* instance structure */
static t_class *gem_average_class;

typedef struct _gem_average
{
	t_object    x_obj;	        /* obligatory object header */
	int	        a_total;	    /* number of numbers to gem_average */
	int	        a_whichNum;	    /* which number are pointing at */
	float	    a_numbers[100]; /* numbers to gem_average, 100 maximum */
	t_outlet    *t_out1;	    /* the outlet */
} t_gem_average;

void gem_average_bang(t_gem_average *x)
{
    float gem_average = 0.0f;
    int n;
    
    for (n = 0; n < x->a_total; n++) gem_average = gem_average + x->a_numbers[n];
    gem_average = gem_average / (float)x->a_total;
     
    outlet_float(x->t_out1, gem_average);
}

void gem_average_float(t_gem_average *x, t_floatarg n)
{
    if (x->a_whichNum >= x->a_total) x->a_whichNum = 0;
    x->a_numbers[x->a_whichNum] = n;
    x->a_whichNum++;
    gem_average_bang(x);
}

void gem_average_total(t_gem_average *x, t_floatarg n)
{
    x->a_total = (int)n;
}

void gem_average_reset(t_gem_average *x, t_floatarg newVal)
{
    int n;  
    for (n=0; n < 100; n ++) x->a_numbers[n] = newVal;
}

void gem_average_clear(t_gem_average *x)
{
    int n;    
    for ( n = 0; n < 100; n ++) x->a_numbers[n] = 0.0f;
}

void *gem_average_new(t_floatarg f) /* init vals in struc */
{
    t_gem_average *x = (t_gem_average *)pd_new(gem_average_class);
    x->t_out1 = outlet_new(&x->x_obj, 0);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl1"));
    gem_average_clear(x);
    if (f) x->a_total = (int)f;
    else x->a_total = 10;
    x->a_whichNum = 0;
    return (x);
}

void gem_average_setup(void)
{
    gem_average_class = class_new(gensym("gem_average"), (t_newmethod)gem_average_new, 0,
    	    	    	sizeof(t_gem_average), 0, A_DEFFLOAT, 0);
    class_addbang(gem_average_class, (t_method)gem_average_bang);
    class_addfloat(gem_average_class, (t_method)gem_average_float);
    class_addmethod(gem_average_class, (t_method)gem_average_total, gensym("fl1"), A_FLOAT, 0);
    class_addmethod(gem_average_class, (t_method)gem_average_clear, gensym("clear"), A_NULL);
    class_addmethod(gem_average_class, (t_method)gem_average_reset, gensym("reset"), A_FLOAT,  0);

	#if PD_MINOR_VERSION < 37 
	class_sethelpsymbol(gem_average_class, gensym("gem_average-help.pd"));
#endif
}

