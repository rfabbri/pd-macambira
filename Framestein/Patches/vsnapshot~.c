#include "m_pd.h"
#include "math.h"
#include "sharemem.h"
#include "vframe.h"

/* -------------------------- vsnapshot~ ------------------------------ */
static t_class *vsnapshot_class;

typedef struct _vsnapshot
{
	t_object x_obj;
	float x_f;
	int id;
	HANDLE hvf, hbits;
	LPVOID memvf, membits;
	unsigned long pos, size;
} t_vsnapshot;

static t_int *vsnapshot_perform(t_int *w)
{
	t_vsnapshot *x = (t_vsnapshot *)(w[1]);
	t_float *in = (t_sample *)(w[2]);
	t_float *sync = (t_sample *)(w[3]);
	int n = (int)(w[4]);
	short *p16;
	long *p32;
	struct vframeimage *vfp=x->memvf;
	unsigned long bytespp;

	if(!vfp || !x->membits) return(w+5);

	bytespp = vfp->f.pixelformat / 8;

	switch(vfp->f.pixelformat)
	{
	case 16:
		p16 = x->membits;
		while (n--)
		{
			if(*sync>0 && *sync<1)
				x->pos=*sync * (float)(x->size / bytespp);
			else
				if(*sync==1) x->pos=(x->size / bytespp)-1;

			p16[x->pos++] = sampletocolor16(*in);
			in++;
			sync++;
			if(x->pos*bytespp >= x->size)
			{
				x->pos=0;
				outlet_float(x->x_obj.ob_outlet, x->id);
			}
		}
		break;
	case 32:
		p32 = x->membits;
		while (n--)
		{
			if(*sync>0 && *sync<1)
				x->pos=*sync * (float)(x->size / bytespp);
			else
				if(*sync==1) x->pos=(x->size / bytespp)-1;

			p32[x->pos++] = sampletocolor32(*in);
			in++;
			sync++;
			if(x->pos*bytespp >= x->size)
			{
				x->pos=0;
				outlet_float(x->x_obj.ob_outlet, x->id);
			}
		}
		break;
	}
	return(w+5);
}

static void vsnapshot_dsp(t_vsnapshot *x, t_signal **sp)
{
	dsp_add(vsnapshot_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void vsnapshot_float(t_vsnapshot *x, t_float f)
{
	struct vframeimage *vfp;

	if(x->id==f && x->membits!=NULL)
	{
		outlet_float(x->x_obj.ob_outlet, x->id);
		return;
	}

	x->id = x->x_f = f;

	openframedatabyid(x->id,
	 &x->hvf, &x->hbits, &x->memvf, &x->membits, &vfp);
	if(!x->membits)
	{
		post("vsnapshot~: no memory at %d", x->id);
		return;
	} else
	{
		x->size = vfp->f.height*vfp->f.lpitch;
		x->pos = 0;
		outlet_float(x->x_obj.ob_outlet, x->id);
	}
}

static void *vsnapshot_new(t_floatarg f)
{
	t_vsnapshot *x = (t_vsnapshot *)pd_new(vsnapshot_class);
	x->x_f = f;
	x->pos=0;
	x->hvf=NULL;
	x->memvf=NULL;
	x->hbits=NULL;
	x->membits=NULL;
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft1"));
	outlet_new(&x->x_obj, gensym("float"));
	return (x);
}

static void vsnapshot_destroy(t_vsnapshot *x)
{
	if(x->membits!=NULL) smfree(&x->hbits, x->membits);
	if(x->memvf!=NULL) smfree(&x->hvf, x->memvf);
}

void vsnapshot_tilde_setup(void)
{
	vsnapshot_class = class_new(gensym("vsnapshot~"),
	 (t_newmethod)vsnapshot_new, (t_method)vsnapshot_destroy,
	 sizeof(t_vsnapshot), 0, A_DEFFLOAT, 0);
	CLASS_MAINSIGNALIN(vsnapshot_class, t_vsnapshot, x_f);
	class_addmethod(vsnapshot_class, (t_method)vsnapshot_float,
	 gensym("ft1"), A_FLOAT, 0);
	class_addmethod(vsnapshot_class, (t_method)vsnapshot_dsp, gensym("dsp"), 0);
}
