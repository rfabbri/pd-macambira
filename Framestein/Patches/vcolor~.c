#include "m_pd.h"
#include "math.h"
#include "sharemem.h"
#include "vframe.h"
#include "plugin.h"
#include "displaydepth.h"

/* -------------------------- vcolor~ ------------------------------ */
static t_class *vcolor_class;

typedef struct _vcolor
{
	t_object x_obj;
	float x_r;
	float x_g;
	float x_b;
	int depth;
} t_vcolor;

static t_int *vcolor_perform(t_int *w)
{
	t_float *in_r = (t_float *)(w[1]);
	t_float *in_g = (t_float *)(w[2]);
	t_float *in_b = (t_float *)(w[3]);
	t_float *out = (t_float *)(w[4]);
	int n = (int)(w[5]);
	t_vcolor *x = (t_vcolor *)(w[6]);

	switch(x->depth)
	{
	case 16:
		while (n--)
		{
			*out++ = colortosample16(rgbtocolor16(
			 (byte)(*in_r++*255.0),
			 (byte)(*in_g++*255.0),
			 (byte)(*in_b++*255.0)
			));
		}
		break;
	case 32:
		while (n--)
		{
			*out++ = colortosample32(rgbtocolor32(
			 (byte)(*in_r++*255.0),
			 (byte)(*in_g++*255.0),
			 (byte)(*in_b++*255.0)));
		}
		break;
	}
	return (w+7);
}

static void vcolor_dsp(t_vcolor *x, t_signal **sp)
{
	dsp_add(vcolor_perform, 6,
	 sp[0]->s_vec,
	 sp[1]->s_vec,
	 sp[2]->s_vec,
	 sp[3]->s_vec,
	 sp[0]->s_n,
	 x);
}

static void vcolor_float(t_vcolor *x, t_float f)
{
}

static void *vcolor_new(t_floatarg f)
{
	t_vcolor *x = (t_vcolor *)pd_new(vcolor_class);
	x->depth = getdisplaydepth();
	if(!x->depth)
	{
		post("vcolor~: getdisplaydepth() failed, defaulting to 16.");
		x->depth = 16;
	}
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));
	outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static void vcolor_destroy(t_vcolor *x)
{
}

void vcolor_tilde_setup(void)
{
    vcolor_class = class_new(gensym("vcolor~"),
	(t_newmethod)vcolor_new, (t_method)vcolor_destroy,
    	sizeof(t_vcolor), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(vcolor_class, t_vcolor, x_r);
    class_addfloat(vcolor_class, (t_method)vcolor_float);
    class_addmethod(vcolor_class, (t_method)vcolor_dsp, gensym("dsp"), 0);
}
