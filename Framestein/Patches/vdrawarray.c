#include "m_pd.h"
#include "math.h"
#include "sharemem.h"
#include "vframe.h"

/* -------------------------- vdrawarray ------------------------------ */
static t_class *vdrawarray_class;

typedef struct _vdrawarray
{
	t_object x_obj;
	t_symbol *array_name;
	t_float startpos, endpos;
	int id;
	HANDLE hvf, hbits;
	LPVOID memvf, membits;
	struct vframeimage *vfp;
} t_vdrawarray;

void vdrawarray_set(t_vdrawarray *x, t_symbol *s)
{
	x->array_name = s;
}

static void vdrawarray_bang(t_vdrawarray *x)
{
	int i, o, h, y, bytesize, gsize, pos, len;
	t_float *gdata, tmpf;
	byte *p;
	t_garray *g;

	if(!x->id)
	{
		post("vdrawarray: connect vframe first");
		return;
	}

	if(!(g = (t_garray *)pd_findbyclass(x->array_name, garray_class)))
	{
		pd_error(x, "vdrawarray: %s: no such table", x->array_name->s_name);
		return;
	}

	garray_getfloatarray(g, &gsize, &gdata);

	h = x->vfp->f.height;
	p = (byte*)x->membits;
	bytesize = x->vfp->f.pixelformat/8;

	if(x->startpos==0 && x->endpos==0)
		x->endpos=gsize;

	if(x->startpos<0) x->startpos=0;
	if(x->startpos>gsize) x->startpos=gsize;
	if(x->endpos<0) x->endpos=0;
	if(x->endpos>gsize) x->endpos=gsize;

	if(x->endpos<x->startpos)
	{
		tmpf = x->endpos;
		x->endpos = x->startpos;
		x->startpos = tmpf;
	}

	for(i=0; i<x->vfp->f.width; i++)
	{
		pos = (int)((i / (float)x->vfp->f.width) * (x->endpos-x->startpos) + x->startpos );
		len = (int)((fabs(gdata[pos]) * (float)x->vfp->f.height));		

		o = (x->vfp->f.height - len) / 2;
		for(y=o; y<o+len && y<x->vfp->f.height; y++)
			memset(&p[y*x->vfp->f.lpitch+i*bytesize], 253, bytesize);
	}
}

static void vdrawarray_float(t_vdrawarray *x, t_float f)
{
	if(x->id==f && x->membits!=NULL)
	{
		outlet_float(x->x_obj.ob_outlet, x->id);
		vdrawarray_bang(x);
		return;
	}

	x->id = f;

	openframedatabyid(x->id,
	 &x->hvf, &x->hbits, &x->memvf, &x->membits, &x->vfp);
	if(!x->membits)
	{
		post("vdrawarray: no memory at %d", x->id);
		return;
	} else
	{
		outlet_float(x->x_obj.ob_outlet, x->id);
		vdrawarray_bang(x);
	}
}

static void *vdrawarray_new(t_symbol *s)
{
	t_vdrawarray *x = (t_vdrawarray *)pd_new(vdrawarray_class);

	x->array_name = s;
	x->id=0;
	x->hvf=NULL;
	x->memvf=NULL;
	x->hbits=NULL;
	x->membits=NULL;
	x->startpos=x->endpos=0;
	outlet_new(&x->x_obj, gensym("float"));
	floatinlet_new(&x->x_obj, &x->startpos);
	floatinlet_new(&x->x_obj, &x->endpos);
	return (x);
}

static void vdrawarray_destroy(t_vdrawarray *x)
{
	if(x->membits!=NULL) smfree(&x->hbits, x->membits);
	if(x->memvf!=NULL) smfree(&x->hvf, x->memvf);
}

void vdrawarray_setup(void)
{
	vdrawarray_class = class_new(gensym("vdrawarray"),
	 (t_newmethod)vdrawarray_new, (t_method)vdrawarray_destroy,
	 sizeof(t_vdrawarray), 0, A_DEFSYM, 0);
	class_addfloat(vdrawarray_class, (t_method)vdrawarray_float);
	class_addbang(vdrawarray_class, (t_method)vdrawarray_bang);
	class_addmethod(vdrawarray_class, (t_method)vdrawarray_set, gensym("set"), A_DEFSYM, 0);
}
