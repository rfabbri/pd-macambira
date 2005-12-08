/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
 */

#include "m_pd.h"

/* -------------------------- change ------------------------------ */

/* instance structure */

static t_class *change_class;

typedef struct _change
{
	t_object    x_obj;			/* obligatory object header */
	float		x_cur;
	t_outlet    *t_out1;	    /* the outlet */
} t_change;

static void change_float(t_change *x, t_floatarg n)
{
    if (n != x->x_cur)
	{
		outlet_float(x->t_out1, n);
		x->x_cur = n;
    }
}

static void *change_new(void) /* init vals in struc */
{
    t_change *x = (t_change *)pd_new(change_class);
    x->x_cur = -1.f;
	x->t_out1 = outlet_new(&x->x_obj, 0);
    return(x);
}

void change_setup(void)
{
    change_class = class_new(gensym("change"), (t_newmethod)change_new, 0,
    	    	    	sizeof(t_change), 0, A_NULL);
    class_addfloat(change_class, change_float);

	 #if PD_MINOR_VERSION < 37 

#endif
}
