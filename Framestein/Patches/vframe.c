#include <stdlib.h>
#include <stdio.h>
#include "m_pd.h"
#include "sharemem.h"
#include "vframe.h"
#include "dllcall.h"
#include "displaydepth.h"

/* vframe */

static t_class *vframe_class;

typedef struct _vframe
{
	t_object x_obj;

	int id;
	HANDLE hlvframe, hlbits;
	LPVOID memvframe, membits;

	HMODULE effectlib, copylib;
	cdecl FARPROC effectproc, copyproc;
	char effectplugin[80], effectloaded[80],
	 copyplugin[80], copyloaded[80];
	char effectparams[256];
} t_vframe;

static void vframe_effect(t_vframe *x)
{
	struct vframeimage *vfp;
	byte *bp;
	int i1, i2, i3, i4;
	char *s1=0, s2[256];

	if(!x->membits)
	{
		post("vframe: init has failed.");
		return;
	}
	if(!x->effectplugin) return;
	if(strcmp(x->effectloaded, x->effectplugin))
	{
		loadeffect(&x->effectlib, &x->effectproc, x->effectplugin);
		if(!x->effectlib || !x->effectproc)
		{
			post("vframe: failed to load effect from %s", x->effectplugin);
			strcpy(x->effectloaded, "-");
			return;
		}
		strcpy(x->effectloaded, x->effectplugin);
	}
	vfp = x->memvframe;
	bp = x->membits;
	i1 = vfp->f.lpitch;
	i2 = vfp->f.width;
	i3 = vfp->f.height;
	i4 = vfp->f.pixelformat;

	(*x->effectproc)(bp, i1, i2, i3, i4, &x->effectparams, &s2);
	outlet_float(x->x_obj.ob_outlet, x->id);
}

static void vframe_copy(t_vframe *x, float source, char *cmd)
{
	char *t, *args=0;
	HANDLE h1, h2;
	LPVOID p1=NULL, p2=NULL;
	struct vframeimage *vfp1=NULL, *vfp2=NULL;
	byte *b1, *b2;
	int i1, i2, i3, i4, i5, i6, i7, i8;
	char returnbuf[256];

	if(!x->membits)
	{
		post("vframe: init has failed.");
		return;
	}
	strcpy(x->copyplugin, cmd);
	t = strstr(x->copyplugin, " ");
	if(t)
	{
		t[0]=0;
		args=t+1;
	}

	if(!x->copyplugin) return;
	if(strcmp(x->copyloaded, x->copyplugin))
	{
		loadcopy(&x->copylib, &x->copyproc, x->copyplugin);
		if(!x->copylib || !x->copyproc)
		{
			post("vframe: failed to load copy from %s", x->copyplugin);
			strcpy(x->copyloaded, "-");
			return;
		}
		strcpy(x->copyloaded, x->copyplugin);
	}

	p2 = openframedatabyid((int)source, &h1, &h2, &p1, &p2, &vfp1);
	if(!p2)
	{
		if(source) post("vframe: no memory at %f", source);
		return;
	}

	vfp2 = (struct vframeimage *)x->memvframe;

	b1 = p2;
	i1 = vfp1->f.lpitch;
	i2 = vfp1->f.width;
	i3 = vfp1->f.height;
	i4 = vfp1->f.pixelformat;

	b2 = x->membits;
	i5 = vfp2->f.lpitch;
	i6 = vfp2->f.width;
	i7 = vfp2->f.height;
	i8 = vfp2->f.pixelformat;

	(*x->copyproc)(
	 b1, i1, i2, i3, i4,
	 b2, i5, i6, i7, i8,
	 args, &returnbuf
	);

	outlet_float(x->x_obj.ob_outlet, x->id);
}

static void vframe_float(t_vframe *x, t_float f)
{
	post("float %f", f);
}

static void vframe_symbol(t_vframe *x, t_symbol *s)
{
	char *t;

	strcpy(x->effectplugin, s->s_name);
	x->effectparams[0]=0;

	t = strstr(x->effectplugin, " ");
	if(t)
	{
		strcpy(x->effectparams, t+1);
		t[0]=0;
	}
	vframe_effect(x);
}

static void vframe_list(t_vframe *x, t_symbol *s, int ac, t_atom *av)
{
	float f;
	char *s2;
	if (!ac || ac!=2) return;

	f = atom_getfloatarg(0, ac, av);
	s2 = atom_getsymbolarg(1, ac, av)->s_name;
	vframe_copy(x, f, s2);
}

static void vframe_anything(t_vframe *x, t_symbol *s, int argc, t_atom *argv)
{
	char *t;

	strcpy(x->effectplugin, s->s_name);
	x->effectparams[0]=0;

	t = strstr(x->effectplugin, " ");
	if(t)
	{
		strcpy(x->effectparams, t+1);
		t[0]=0;
	}
	vframe_effect(x);
}

static void vframe_bang(t_vframe *x, t_floatarg fa)
{
	if(x->membits!=NULL)
		outlet_float(x->x_obj.ob_outlet, x->id);
}

static void *vframe_new(t_symbol *sym, int argc, t_atom *argv)
{
	t_vframe *x = (t_vframe *)pd_new(vframe_class);

	struct vframeimage vf;
	int width=176, height=144, pixelformat=16;
	char s[20];

	if(argc>=2)
	{
		width = atom_getfloat(argv);
		height = atom_getfloat(argv+1);
	}
	pixelformat = getdisplaydepth();
	if(pixelformat!=16 && pixelformat!=24 && pixelformat!=32)
	{
		post("vframe: bad display depth, should be 16, 24 or 32. defaulting to 16.");
		pixelformat = 16;
	}

	// allocate shared memory for bits and struct frameimage
	x->id = rand();
	itoa(x->id, s, 10);

	x->membits = smalloc(&x->hlbits, s, width*height*(pixelformat/8));
	if(x->membits==NULL)
	{
		post("vframe: membits smalloc() failed.");
	} else
	{
		strcpy(vf.bitsname, s);
		vf.f.width = width;
		vf.f.height = height;
		vf.f.pixelformat = pixelformat;
		vf.f.lpitch = vf.f.width*(pixelformat/8); /* + some more for directx-crap??? */
		vf.f.bits=NULL;

		x->id = rand();
		itoa(x->id, s, 10);

		x->memvframe = smalloc(&x->hlvframe, s,
		 sizeof(struct vframeimage));

		if(x->memvframe==NULL)
		{
			post("vframeimage smalloc() failed.");
		} else
		{
			memcpy(x->memvframe, &vf, sizeof(struct vframeimage));
		}
	}

	x->effectlib=NULL;
	strcpy(x->effectloaded, "n o t h i n g");
	x->copylib=NULL;
	strcpy(x->copyloaded, "n o t h i n g");

	outlet_new(&x->x_obj, gensym("float"));
	return((void *)x);
}

static void vframe_destroy(t_vframe *x)
{
	if(x->membits!=NULL) smfree(&x->hlbits, x->membits);
	if(x->memvframe!=NULL) smfree(&x->hlvframe, x->memvframe);
	if(x->effectlib) FreeLibrary(x->effectlib);
	if(x->copylib) FreeLibrary(x->copylib);
}

void vframe_setup(void)
{
	vframe_class = class_new(gensym("vframe"),
	(t_newmethod)vframe_new, (t_method)vframe_destroy,
	sizeof(t_vframe), CLASS_DEFAULT, A_GIMME, 0);
	class_addfloat(vframe_class, vframe_float);
	class_addbang(vframe_class, vframe_bang);
	class_addsymbol(vframe_class, vframe_symbol);
	class_addanything(vframe_class, vframe_anything);
	class_addlist(vframe_class, vframe_list);
}
