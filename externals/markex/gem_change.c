/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
 */

#include "m_pd.h"

/* -------------------------- gem_change ------------------------------ */

/* instance structure */

static t_class *gem_change_class;

typedef struct _gem_change
{
	t_object    x_obj;			/* obligatory object header */
	float		x_cur;
	t_outlet    *t_out1;	    /* the outlet */
} t_gem_change;

static void gem_change_float(t_gem_change *x, t_floatarg n)
{
    if (n != x->x_cur)
	{
		outlet_float(x->t_out1, n);
		x->x_cur = n;
    }
}

static void *gem_change_new(void) /* init vals in struc */
{
    t_gem_change *x = (t_gem_change *)pd_new(gem_change_class);
    x->x_cur = -1.f;
	x->t_out1 = outlet_new(&x->x_obj, 0);
    return(x);
}

void gem_change_setup(void)
{
    gem_change_class = class_new(gensym("gem_change"), (t_newmethod)gem_change_new, 0,
    	    	    	sizeof(t_gem_change), 0, A_NULL);
    class_addfloat(gem_change_class, gem_change_float);

	 class_sethelpsymbol(gem_change_class, gensym("help-gem_change"));
}
