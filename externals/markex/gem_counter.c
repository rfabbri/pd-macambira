/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
 */

#include "m_pd.h"

/* -------------------------- gem_counter ------------------------------ */

/* instance structure */
static t_class *gem_counter_class;

typedef struct _gem_counter
{
	t_object    x_obj;	    /* obligatory object header */
	int	        c_current;	/* current number */
	int	        c_high;	    /* highest number */
	int	        c_low;	    /* lowest number */
	int	        c_updown;	/* 0 = going up, 1 = going down */
	int	        c_dir;	    /* gem_counter dir. 1=up, 2=down, 3=up/down */
	t_outlet    *t_out1;	/* the outlet */
	t_outlet    *t_out2;	/* the outlet */
} t_gem_counter;

void gem_counter_bang(t_gem_counter *x)
{
	int sendBang = 0;
    switch(x->c_dir)
    {
		// count up
	    case 1:
            x->c_current++;
	        if (x->c_current > x->c_high)
				x->c_current = x->c_low;
	        else if (x->c_current < x->c_low)
				x->c_current = x->c_low;
			else if (x->c_current == x->c_high)
				sendBang = 1;
	        break;
	    // count down
		case 2:
	        x->c_current--;
	        if (x->c_current < x->c_low)
				x->c_current = x->c_high;
	        else if (x->c_current > x->c_high)
				x->c_current = x->c_high;
			else if (x->c_current == x->c_low)
				sendBang = 1;
	        break;
	    // count up and down
		case 3:
	        // going up
			if (x->c_updown == 0)
	        {
                x->c_current++;
		        if (x->c_current > x->c_high)
		        {
		            x->c_current = x->c_high - 1;
		            x->c_updown = 1;
		        }
		        else if (x->c_current < x->c_low)
					x->c_current = x->c_low;
				else if (x->c_current == x->c_high)
					sendBang = 1;
	        }
	        // going down
			else if (x->c_updown == 1)
	        {
                x->c_current--;
		        if (x->c_current < x->c_low)
		        {
		            x->c_current = x->c_low + 1;
		            x->c_updown = 0;
		        }
		        else if (x->c_current > x->c_high)
					x->c_current = x->c_high;
				else if (x->c_current == x->c_low)
					sendBang = 1;
	        }
	        else
	        {
		        error("up/down wrong");
		        return;
	        }
	        break;
	    default:
	        error("dir wrong");
	        return;
    }
    outlet_float(x->t_out1, (float)x->c_current);
	if (sendBang)
		outlet_bang(x->t_out2);
}

void gem_counter_dir(t_gem_counter *x, t_floatarg n)
{
    if (n == 1 || n == 2 || n == 3) x->c_dir = (int)n;
    else error("bad dir");
}

void gem_counter_high(t_gem_counter *x, t_floatarg n)
{
    x->c_high = (int)n;
}

void gem_counter_low(t_gem_counter *x, t_floatarg n)
{
    x->c_low = (int)n;
}

void gem_counter_reset(t_gem_counter *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!argc)
    {
	    switch(x->c_dir)
	    {
	        case 1:
		        x->c_current = x->c_low;
		        break;
	        case 2:
		        x->c_current = x->c_high;
		        break;
	        case 3:
		        if (x->c_updown == 0) x->c_current = x->c_low;
		        else if (x->c_updown == 1) x->c_current = x->c_high;
		        break;
	        default:
		        return;
	    }
    }
    else
    {
	    switch(argv[0].a_type)
	    {
	        case A_FLOAT :
		        x->c_current = (int)argv[0].a_w.w_float;
		        break;
	        default :
		        error ("gem_counter: reset not float");
		        break;
	    }
    }
    outlet_float(x->t_out1, (float)x->c_current);
}

void gem_counter_clear(t_gem_counter *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!argc)
    {
	    switch(x->c_dir)
	    {
	        case 1:
		        x->c_current = x->c_low - 1;
		        break;
	        case 2:
		        x->c_current = x->c_high + 1;
		        break;
	        case 3:
		        if (x->c_updown == 0) x->c_current = x->c_low - 1;
		        else if (x->c_updown == 1) x->c_current = x->c_high + 1;
		        break;
	        default:
		        break;
	    }
    }
    else
    {
	    switch(argv[0].a_type)
	    {
	        case A_FLOAT :
		        x->c_current = (int)argv[0].a_w.w_float - 1;
		        break;
	        default :
		        error ("gem_counter: reset not float");
		        break;
	    }
    }
}

void *gem_counter_new(t_floatarg f, t_floatarg g, t_floatarg h) /* init vals in struc */
{
    t_gem_counter *x = (t_gem_counter *)pd_new(gem_counter_class);
    x->t_out1 = outlet_new(&x->x_obj, 0);
    x->t_out2 = outlet_new(&x->x_obj, 0);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl1"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl2"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl3"));
    x->c_current = 0;
    x->c_updown = 0;
    if (h)
    {
	    gem_counter_low(x, f);
	    gem_counter_high(x, g);
	    gem_counter_dir(x, h);
    }
    else if (g)
    {
	    x->c_dir = 1;
	    gem_counter_low(x, f);
	    gem_counter_high(x, g);
    }
    else if (f)
    {
	    x->c_dir = x->c_low = 1;
	    gem_counter_high(x, f);
    }
    else
    {
	    x->c_dir = x->c_low = 1;
	    x->c_high = 10;
    }
    return (x);
}

void gem_counter_setup(void)
{
    gem_counter_class = class_new(gensym("gem_counter"), (t_newmethod)gem_counter_new, 0,
    	    sizeof(t_gem_counter), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addbang(gem_counter_class, (t_method)gem_counter_bang);
    class_addmethod(gem_counter_class, (t_method)gem_counter_dir, gensym("fl1"), A_FLOAT, 0);
    class_addmethod(gem_counter_class, (t_method)gem_counter_low, gensym("fl2"), A_FLOAT, 0);
    class_addmethod(gem_counter_class, (t_method)gem_counter_high, gensym("fl3"), A_FLOAT, 0);
    class_addmethod(gem_counter_class, (t_method)gem_counter_reset, gensym("reset"), A_GIMME, 0);
    class_addmethod(gem_counter_class, (t_method)gem_counter_clear, gensym("clear"), A_GIMME, 0);

	class_sethelpsymbol(gem_counter_class, gensym("help-gem_counter"));
}
