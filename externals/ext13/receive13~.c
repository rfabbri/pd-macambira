/* Copyright (c) 1997-1999 Miller Puckette source modified by dieb13.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  catch13~ ...  = original objects with set-message for changing - bad hack - but works somehow*/

#include "m_pd.h"
#include "d_global13.h"

#define DEFSENDVS 64	/* LATER get send to get this from canvas */

/* ----------------------------- receive13~ ----------------------------- */

 void *sigreceive13_new(t_symbol *s)
{
    t_sigreceive13 *x = (t_sigreceive13 *)pd_new(sigreceive13_class);
    if (!*s->s_name) s = gensym("receive13~");
    x->x_n = DEFSENDVS;   	    /* LATER find our vector size correctly */
    x->x_sym = s;
    x->x_wherefrom = 0;
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

 t_int *sigreceive13_perform(t_int *w)
{
    t_sigreceive13 *x = (t_sigreceive13 *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    t_float *in = x->x_wherefrom;
    if (in)
    {
    	while (n--)
	    *out++ = *in++; 
    }
    else
    {
    	while (n--)
	    *out++ = 0; 
    }
    return (w+4);
}

 void sigreceive13_set(t_sigreceive13 *x, t_symbol *s)
{
    t_sigsend13 *sender = (t_sigsend13 *)pd_findbyclass((x->x_sym = s),
    	sigsend13_class);
    if (sender)
    {
    	if (sender->x_n == x->x_n)
    	    x->x_wherefrom = sender->x_vec;
	else
	{
	    pd_error(x, "receive13~ %s: vector size mismatch", x->x_sym->s_name);
	    x->x_wherefrom = 0;
	}
    }
    else
    {
    	pd_error(x, "receive13~ %s: no matching send", x->x_sym->s_name);
    	x->x_wherefrom = 0;
    }
}

 void sigreceive13_dsp(t_sigreceive13 *x, t_signal **sp)
{
    if (sp[0]->s_n != x->x_n)
    {
    	pd_error(x, "receive13~ %s: vector size mismatch", x->x_sym->s_name);
    }
    else
    {
    	sigreceive13_set(x, x->x_sym);
    	dsp_add(sigreceive13_perform, 3,
    	    x, sp[0]->s_vec, sp[0]->s_n);
    }
}

void sigreceive13_setup(void)
{
    sigreceive13_class = class_new(gensym("receive13~"),
    	(t_newmethod)sigreceive13_new, 0,
    	sizeof(t_sigreceive13),0, A_DEFSYM, 0);
    class_addcreator((t_newmethod)sigreceive13_new, gensym("r13~"), A_DEFSYM, 0);
    class_sethelpsymbol(sigreceive13_class, gensym("ext13"));
    class_addmethod(sigreceive13_class, (t_method)sigreceive13_set, gensym("set"),
    	A_SYMBOL, 0);
    class_addmethod(sigreceive13_class, (t_method)sigreceive13_dsp, gensym("dsp"),
    	0);
}


