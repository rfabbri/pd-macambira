#include "m_pd.h"
#include "math.h"
#include "sharemem.h"
#include "vframe.h"
#include "plugin.h"
#include "displaydepth.h"

/* -------------------------- vrgb~ ------------------------------ */
static t_class *vrgb_class;

typedef struct _vrgb
{
	t_object x_obj;
	float x_f;
	float mode; /* 0=rgb 1=r ... */
	int depth;
} t_vrgb;

static t_int *vrgb_perform(t_int *w)
{
	t_float *in = (t_float *)(w[1]);
	t_float *r = (t_float *)(w[2]);
	t_float *g = (t_float *)(w[3]);
	t_float *b = (t_float *)(w[4]);
	int n = (int)(w[5]);
	t_vrgb *x = (t_vrgb *)(w[6]);
	byte cr, cg, cb;
	short c16;
	long c32;

	// support "x->mode" later....

	switch(x->depth)
	{
	case 16:
		while (n--)
		{
			c16 = sampletocolor16(*in++);
			cr = r16(c16);
			cg = g16(c16);
			cb = b16(c16);
			*r++ = cr / 255.0;
			*g++ = cg / 255.0;
			*b++ = cb / 255.0;

/*			*r++ = colortosample16(rgbtocolor16(cr, 0, 0));
			*g++ = colortosample16(rgbtocolor16(0, cg, 0));
			*b++ = colortosample16(rgbtocolor16(0, 0, cb));
*/		}
		break;
	case 32:
		while (n--)
		{
			c32 = sampletocolor32(*in++);
			cr = r32(c32);
			cg = g32(c32);
			cb = b32(c32);
			*r++ = cr / 255.0;
			*g++ = cg / 255.0;
			*b++ = cb / 255.0;

/*			*b++ = colortosample32(rgbtocolor32(cr, 0, 0));
			*g++ = colortosample32(rgbtocolor32(0, cg, 0));
			*r++ = colortosample32(rgbtocolor32(0, 0, cb));
*/		}
		break;
	}
	return (w+7);
}

static void vrgb_dsp(t_vrgb *x, t_signal **sp)
{
	dsp_add(vrgb_perform, 6,
	 sp[0]->s_vec,
	 sp[1]->s_vec,
	 sp[2]->s_vec,
	 sp[3]->s_vec,
	 sp[0]->s_n,
	 x);
}

static void vrgb_float(t_vrgb *x, t_float f)
{
	x->mode = f;
post("mode %f", f);
}

static void *vrgb_new(t_floatarg f)
{
	t_vrgb *x = (t_vrgb *)pd_new(vrgb_class);
	x->x_f = f;
	x->mode=0;
	x->depth = getdisplaydepth();
	if(!x->depth)
	{
		post("vrgb~: getdisplaydepth() failed, defaulting to 16.");
		x->depth = 16;
	}
	outlet_new(&x->x_obj, gensym("signal"));
	outlet_new(&x->x_obj, gensym("signal"));
	outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static void vrgb_destroy(t_vrgb *x)
{
}

void vrgb_tilde_setup(void)
{
    vrgb_class = class_new(gensym("vrgb~"),
	(t_newmethod)vrgb_new, (t_method)vrgb_destroy,
    	sizeof(t_vrgb), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(vrgb_class, t_vrgb, x_f);
    class_addfloat(vrgb_class, (t_method)vrgb_float);
    class_addmethod(vrgb_class, (t_method)vrgb_dsp, gensym("dsp"), 0);
}
