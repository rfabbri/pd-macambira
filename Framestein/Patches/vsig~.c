#include "m_pd.h"
#include "math.h"
#include "sharemem.h"
#include "vframe.h"

/* -------------------------- vsig~ ------------------------------ */
static t_class *vsig_class;

typedef struct _vsig
{
	t_object x_obj;
	float x_f;
	t_outlet *x_bang;
	int id;
	HANDLE hvf, hbits;
	LPVOID memvf, membits;
	unsigned long pos, size;
} t_vsig;

static t_int *vsig_perform(t_int *w)
{
	t_vsig *x = (t_vsig *)(w[1]);
	t_float *out = (t_float *)(w[2]);
	t_float *sync = (t_float *)(w[3]);
	int n = (int)(w[4]);
	short *p16;
	long *p32;
	struct vframeimage *vfp=x->memvf;
	int bitspp;

	if(!vfp || !x->membits) return(w+5);

	bitspp = vfp->f.pixelformat / 8;

	switch(vfp->f.pixelformat)
	{
	case 16:
		p16 = (short *)x->membits;
		while (n--)
		{
			*out++ = colortosample16(p16[x->pos++]);
			if(x->pos*bitspp >= x->size)
			{
				x->pos=0;
				*sync = 1; // moveto topleft
				outlet_bang(x->x_bang);
			} else *sync = -1; // continue from whereever you are
			sync++;
		}
		break;
	case 32:
		p32 = (long *)x->membits;
		while (n--)
		{
			*out++ = colortosample32(p32[x->pos++]);
			if(x->pos*bitspp >= x->size)
			{
				x->pos=0;
				*sync = 1;
				outlet_bang(x->x_bang);
			} else *sync = -1;
			sync++;
		}
		break;
	}
	return (w+5);
}

static void vsig_dsp(t_vsig *x, t_signal **sp)
{
	dsp_add(vsig_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void vsig_float(t_vsig *x, t_float f)
{
	struct vframeimage *vfp;

	if(x->id==f && x->membits!=NULL) return;

	x->id = x->x_f = f;

	openframedatabyid(x->id,
	 &x->hvf, &x->hbits, &x->memvf, &x->membits, &vfp);
	if(!x->membits)
	{
		post("vsig~: no memory at %d", x->id);
		return;
	} else
	{
		x->size = vfp->f.height*vfp->f.lpitch;
		x->pos = 0;
	}
}

static void *vsig_new(t_floatarg f)
{
	t_vsig *x = (t_vsig *)pd_new(vsig_class);
	x->id = -1;
	x->x_f = f;
	x->hvf=NULL;
	x->memvf=NULL;
	x->hbits=NULL;
	x->membits=NULL;
	outlet_new(&x->x_obj, gensym("signal"));
	outlet_new(&x->x_obj, gensym("signal"));
	x->x_bang = outlet_new(&x->x_obj, gensym("bang"));
	return (x);
}

static void vsig_destroy(t_vsig *x)
{
	if(x->membits!=NULL) smfree(&x->hbits, x->membits);
	if(x->memvf!=NULL) smfree(&x->hvf, x->memvf);
}

void vsig_tilde_setup(void)
{
    vsig_class = class_new(gensym("vsig~"),
	(t_newmethod)vsig_new, (t_method)vsig_destroy,
    	sizeof(t_vsig), 0, A_DEFFLOAT, 0);
    class_addfloat(vsig_class, (t_method)vsig_float);
    class_addmethod(vsig_class, (t_method)vsig_dsp, gensym("dsp"), 0);
}
