#include <stdio.h>
#include <string.h>
#include <m_pd.h>
#include <g_canvas.h>

#define DEBUG(x)

static t_class *canvas_name_class;

typedef struct _canvas_name
{
    t_object x_obj;
    t_symbol *x_canvas_name;
} t_canvas_name;

static void canvas_name_bang(t_canvas_name *x)
{
    outlet_symbol(x->x_obj.ob_outlet,x->x_canvas_name);
}

static void *canvas_name_new(void)
{
    t_canvas_name *x = (t_canvas_name *)pd_new(canvas_name_class);
    char buf[MAXPDSTRING];
    
    snprintf(buf,MAXPDSTRING,".x%lx.c",(long unsigned int)canvas_getcurrent());
    x->x_canvas_name = gensym(buf);
    
	outlet_new(&x->x_obj, &s_symbol);

    return(x);
}

void canvas_name_setup(void)
{
    canvas_name_class = class_new(gensym("canvas_name"),
        (t_newmethod)canvas_name_new, NULL,
        sizeof(t_canvas_name), 0, 0);

    class_addbang(canvas_name_class, (t_method)canvas_name_bang);
}
