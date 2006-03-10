/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_tab written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2005 */

#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif


#include "m_pd.h"
#include "iemlib.h"
#include "iem_tab.h"
#include <math.h>
#include <stdio.h>
#include <string.h>


/* -------------------------- tab_mean ------------------------------ */

typedef struct _tab_mean
{
	t_object	x_obj;
	int				x_size_src1;
	int				x_offset_src1;
	float			*x_beg_mem_src1;
	t_symbol	*x_sym_scr1;
	void			*x_bang_out;
	void			*x_mean_out;
} t_tab_mean;

static t_class *tab_mean_class;

static void tab_mean_src(t_tab_mean *x, t_symbol *s)
{
	x->x_sym_scr1 = s;
}

static void tab_mean_bang(t_tab_mean *x)
{
	int i, n;
	int ok_src;
	t_float *vec_src;
	t_float mean=0.0f;

	ok_src = iem_tab_check_arrays(gensym("tab_mean"), x->x_sym_scr1, &x->x_beg_mem_src1, &x->x_size_src1, 0);

	if(ok_src)
	{
		n = x->x_size_src1;
		vec_src = x->x_beg_mem_src1;
		if(n)
		{
			for(i=0; i<n; i++)
			{
				mean += vec_src[i];
			}
			outlet_float(x->x_mean_out, mean);
			outlet_bang(x->x_bang_out);
		}
	}
}

static void tab_mean_list(t_tab_mean *x, t_symbol *s, int argc, t_atom *argv)
{
	int beg_src;
	int i, n;
	int ok_src;
	t_float *vec_src;
	t_float mean=0.0f;

	if((argc >= 2) &&
		IS_A_FLOAT(argv,0) &&
		IS_A_FLOAT(argv,1))
	{
		beg_src = (int)atom_getintarg(0, argc, argv);
		n = (int)atom_getintarg(1, argc, argv);
		if(beg_src < 0)
			beg_src = 0;
		if(n < 0)
			n = 0;

		ok_src = iem_tab_check_arrays(gensym("tab_mean"), x->x_sym_scr1, &x->x_beg_mem_src1, &x->x_size_src1, beg_src+n);

		if(ok_src)
		{
			vec_src = x->x_beg_mem_src1 + beg_src;
			if(n)
			{
				for(i=0; i<n; i++)
				{
					mean += vec_src[i];
				}
				outlet_float(x->x_mean_out, mean);
				outlet_bang(x->x_bang_out);
			}
		}
	}
	else
	{
		post("tab_mean-ERROR: list need 2 float arguments:");
		post("  source_offset + number_of_samples_to_calc_mean-value");
	}
}

static void tab_mean_free(t_tab_mean *x)
{
}

static void *tab_mean_new(t_symbol *s, int argc, t_atom *argv)
{
	t_tab_mean *x = (t_tab_mean *)pd_new(tab_mean_class);
	t_symbol	*src;

	if((argc >= 1) &&
		IS_A_SYMBOL(argv,0))
	{
		src = (t_symbol *)atom_getsymbolarg(0, argc, argv);
	}
	else
	{
		post("tab_mean-ERROR: need 1 symbol argument:");
		post("  source_array_name");
		return(0);
	}

	x->x_sym_scr1 = src;
	x->x_bang_out = outlet_new(&x->x_obj, &s_bang);
	x->x_mean_out = outlet_new(&x->x_obj, &s_float);
	return(x);
}

void tab_mean_setup(void)
{
	tab_mean_class = class_new(gensym("tab_mean"), (t_newmethod)tab_mean_new, (t_method)tab_mean_free,
					 sizeof(t_tab_mean), 0, A_GIMME, 0);
	class_addbang(tab_mean_class, (t_method)tab_mean_bang);
	class_addlist(tab_mean_class, (t_method)tab_mean_list);
	class_addmethod(tab_mean_class, (t_method)tab_mean_src, gensym("src"), A_DEFSYMBOL, 0);
	class_addmethod(tab_mean_class, (t_method)tab_mean_src, gensym("src1"), A_DEFSYMBOL, 0);
	class_sethelpsymbol(tab_mean_class, gensym("iemhelp2/tab_mean-help"));
}
