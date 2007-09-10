/* (C) 2005 Guenter Geiger */

#include "m_pd.h"
#include "g_canvas.h"


/* HACK

struct _glist
{
    t_object gl_obj;            // header in case we're a glist
    t_gobj *gl_list;            // the actual data
    struct _gstub *gl_stub;     // safe pointer handler
    int gl_valid;               // incremented when pointers might be stale
    struct _glist *gl_owner;    // parent glist, supercanvas, or 0 if none
};

END HACK

*/

typedef struct getdollarzero
{
    t_object x_ob;
    t_canvas * x_canvas;
    t_outlet* x_outlet;
    int x_level;
} t_getdollarzero;




static void getdollarzero_bang(t_getdollarzero *x)
{
    int i = x->x_level;
    t_canvas* last = x->x_canvas;

    while (i>0) {
        i--;
        if (last->gl_owner) last = last->gl_owner;
    }
// x->s_parent_unique = canvas_realizedollar((t_canvas *)this_canvas->gl_owner, gensym("$0"));
    //outlet_symbol(x->x_outlet,canvas_getdir(last));
    outlet_symbol(x->x_outlet,canvas_realizedollar(last, gensym("$0")));
}

t_class *getdollarzero_class;

static void *getdollarzero_new(t_floatarg level)
{
    t_getdollarzero *x = (t_getdollarzero *)pd_new(getdollarzero_class);
    x->x_canvas =  canvas_getcurrent();
    x->x_outlet =  outlet_new(&x->x_ob, &s_);
    x->x_level  =  level;
    return (void *)x;
}

void getdollarzero_setup(void)
{
    getdollarzero_class = class_new(gensym("getdollarzero"), (t_newmethod)getdollarzero_new, 0,
    	sizeof(t_getdollarzero), 0, A_DEFFLOAT,0);
    class_addbang(getdollarzero_class, getdollarzero_bang);
}

