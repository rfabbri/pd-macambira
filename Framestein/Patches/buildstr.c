#include <m_pd.h>

#define MAXLEN 500

static t_class *buildstr_class;

typedef struct _buildstr {
	t_object  x_obj;
	t_symbol s;
	char buf[MAXLEN];
	int pos;
} t_buildstr;

void buildstr_bang(t_buildstr *x)
{
	x->s.s_name = t_getbytes(strlen(x->buf)+1);
	strcpy(x->s.s_name, x->buf);
	outlet_symbol(x->x_obj.ob_outlet, &x->s);
	t_freebytes(x->s.s_name, strlen(x->buf)+1);
} 

void buildstr_float(t_buildstr *x, t_float f)
{
	x->buf[x->pos] = (char)f;
	x->pos++;

	if(f==0)
	{
		x->pos=0;
		buildstr_bang(x);
	}
}

void *buildstr_new(t_symbol *s)
{
	t_buildstr *x = (t_buildstr *)pd_new(buildstr_class);

	x->pos = 0;

	outlet_new(&x->x_obj, gensym("symbol"));

	return (void *)x;
}

void buildstr_setup(void)
{
  buildstr_class = class_new(gensym("buildstr"), (t_newmethod)buildstr_new, 0, sizeof(t_buildstr), CLASS_DEFAULT, A_DEFSYMBOL, 0);
  
	class_addbang(buildstr_class, buildstr_bang);
	class_addfloat(buildstr_class, buildstr_float);
}
