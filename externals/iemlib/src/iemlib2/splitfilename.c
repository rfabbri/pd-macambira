/* Copyright (c) 1997-2003 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil (c) IEM KUG Graz Austria 2000 - 2003 */

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#include "m_pd.h"
#include "iemlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* -------------------------- splitfilename ------------------------------ */

static t_class *splitfilename_class;

typedef struct _splitfilename
{
    t_object x_obj;
    char     x_sep[2];
    void    *x_outpath;
    void    *x_outfile;
} t_splitfilename;

static void splitfilename_separator(t_splitfilename *x, t_symbol *s, int ac, t_atom *av)
{
    if(ac > 0)
    {
	if(IS_A_SYMBOL(av, 0))
	{
	    if(strlen(av->a_w.w_symbol->s_name) == 1)
		x->x_sep[0] = av->a_w.w_symbol->s_name[0];
	    else if(!strcmp(av->a_w.w_symbol->s_name, "backslash"))
		x->x_sep[0] = '\\';
	    else if(!strcmp(av->a_w.w_symbol->s_name, "slash"))
		x->x_sep[0] = '/';
	    else if(!strcmp(av->a_w.w_symbol->s_name, "blank"))
		x->x_sep[0] = ' ';
	    else if(!strcmp(av->a_w.w_symbol->s_name, "space"))
		x->x_sep[0] = ' ';
	    else if(!strcmp(av->a_w.w_symbol->s_name, "dollar"))
		x->x_sep[0] = '$';
	    else if(!strcmp(av->a_w.w_symbol->s_name, "comma"))
		x->x_sep[0] = ',';
	    else if(!strcmp(av->a_w.w_symbol->s_name, "semi"))
		x->x_sep[0] = ';';
	    else if(!strcmp(av->a_w.w_symbol->s_name, "leftbrace"))
		x->x_sep[0] = '{';
	    else if(!strcmp(av->a_w.w_symbol->s_name, "rightbrace"))
		x->x_sep[0] = '}';
	    else
		x->x_sep[0] = '/';
	}
	else if(IS_A_FLOAT(av, 0))
	{
	    int i;
	    float f=fabs(av->a_w.w_float);

	    while(f >= 10.0)
		f *= 0.1;
	    i = (int)f;
	    x->x_sep[0] = (char)i + '0';
	}
    }
    else
	x->x_sep[0] = 0;
}

static void splitfilename_symbol(t_splitfilename *x, t_symbol *s)
{
    int len = strlen(s->s_name);

    if(len)
    {
	if(x->x_sep[0] != 0)
	{
	    char *str_path = (char *)getbytes(len*sizeof(char));
	    char *str_file = (char *)getbytes(len*sizeof(char));
	    char *cpp, *cpf;

	    strcpy(str_path, s->s_name);
	    strcpy(str_file, s->s_name);
	    cpp = strrchr(str_path, x->x_sep[0]);
	    cpf = strrchr(str_file, x->x_sep[0]);
	    if(((int)cpp - (int)str_path) < 0)
	    {
		outlet_symbol(x->x_outfile, gensym(str_file));
		outlet_symbol(x->x_outpath, &s_);
	    }
	    else if(((int)cpp - (int)str_path) >= len)
	    {
		outlet_symbol(x->x_outfile, &s_);
		outlet_symbol(x->x_outpath, gensym(str_path));
	    }
	    else
	    {
		*cpp = 0;
		cpf++;
		outlet_symbol(x->x_outfile, gensym(cpf));
		outlet_symbol(x->x_outpath, gensym(str_path));
	    }
	    freebytes(str_file, len*sizeof(char));
	    freebytes(str_path, len*sizeof(char));
	}
	else
	{
	    outlet_symbol(x->x_outfile, &s_);
	    outlet_symbol(x->x_outpath, s);
	}
    }
}

static void *splitfilename_new(t_symbol *s, int ac, t_atom *av)
{
    t_splitfilename *x = (t_splitfilename *)pd_new(splitfilename_class);

    x->x_sep[0] = 0;
    x->x_sep[1] = 0;
    if(ac == 0)
        x->x_sep[0] = '/';
    else
        splitfilename_separator(x, s, ac, av);
    x->x_outpath = outlet_new(&x->x_obj, &s_symbol);
    x->x_outfile = outlet_new(&x->x_obj, &s_symbol);
    return (x);
}

void splitfilename_setup(void)
{
    splitfilename_class = class_new(gensym("splitfilename"), (t_newmethod)splitfilename_new,
		 0, sizeof(t_splitfilename), 0, A_GIMME, 0);
    class_addsymbol(splitfilename_class, splitfilename_symbol);
    class_addmethod(splitfilename_class, (t_method)splitfilename_separator, gensym("separator"), A_GIMME, 0);
    class_addmethod(splitfilename_class, (t_method)splitfilename_separator, gensym("sep"), A_GIMME, 0);
    class_sethelpsymbol(splitfilename_class, gensym("iemhelp/help-splitfilename"));
}
