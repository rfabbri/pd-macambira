#include "m_pd.h"
#include "math.h"
#include "sharemem.h"
#include "vframe.h"

/* -------------------------- vframeread~ ------------------------------ */
static t_class *vframeread_class;

typedef struct _vframeread
{
	t_object x_obj;
	float x_f;
	float x_sigf;
	int id;
	HANDLE hvf, hbits;
	LPVOID memvf, membits;
	unsigned long pos, size;
} t_vframeread;

static t_int *vframeread_perform(t_int *w)
{
	t_vframeread *x = (t_vframeread *)(w[1]);
	t_float *in_sync = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
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
		p16 = (short *)x->membits;
		while (n--)
		{
			if(*in_sync>=0 && *in_sync<=1)
				x->pos = *in_sync * (float)(x->size / bytespp);
			in_sync++;
			*out++ = colortosample16(p16[x->pos++]);
			if(x->pos*bytespp >= x->size)
				x->pos=0;
		}
		break;
	case 32:
		p32 = (long *)x->membits;
		while (n--)
		{
			if(*in_sync>=0 && *in_sync<1)
				x->pos = *in_sync * (float)(x->size / bytespp);
			in_sync++;
			*out++ = colortosample32(p32[x->pos++]);
			if(x->pos*bytespp >= x->size)
				x->pos=0;
		}
		break;
	}
	return (w+5);
}

static void vframeread_dsp(t_vframeread *x, t_signal **sp)
{
	dsp_add(vframeread_perform, 4,
	 x,
	 sp[0]->s_vec,
	 sp[1]->s_vec,
	 sp[0]->s_n);
}

static void vframeread_float(t_vframeread *x, t_float f)
{
	struct vframeimage *vfp;

	if(x->id==f && x->membits) return;

	x->id = x->x_f = f;

	openframedatabyid(x->id,
	 &x->hvf, &x->hbits, &x->memvf, &x->membits, &vfp);
	if(!x->membits)
	{
		post("vframeread~: no memory at %d", x->id);
		return;
	} else
	{
		x->size = vfp->f.height*vfp->f.lpitch;
		x->pos = 0;
	}
}

static void *vframeread_new(t_floatarg f)
{
	t_vframeread *x = (t_vframeread *)pd_new(vframeread_class);
	x->id = -1;
	x->x_f = f;
	x->x_sigf = 0;
	x->hvf=NULL;
	x->memvf=NULL;
	x->hbits=NULL;
	x->membits=NULL;
	outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static void vframeread_destroy(t_vframeread *x)
{
	if(x->membits!=NULL) smfree(&x->hbits, x->membits);
	if(x->memvf!=NULL) smfree(&x->hvf, x->memvf);
}

void vframeread_tilde_setup(void)
{
    vframeread_class = class_new(gensym("vframeread~"),
	(t_newmethod)vframeread_new, (t_method)vframeread_destroy,
    	sizeof(t_vframeread), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(vframeread_class, t_vframeread, x_sigf);
    class_addfloat(vframeread_class, (t_method)vframeread_float);
    class_addmethod(vframeread_class, (t_method)vframeread_dsp, gensym("dsp"), 0);
}
