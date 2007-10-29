
/* this object should probably register a e_motionfn callback with the root
 * canvas so that it is guaranteed to get the mouse motion data regardless of
 * whether the current canvas is visible.  Check g_canvas.h for info.*/

// list of Tk cursors: http://www.die.net/doc/linux/man/mann/cursors.n.html

#include "m_pd.h"
#include "g_canvas.h"

typedef struct _cursor {
	t_object            x_obj;
	t_outlet            *x_data_outlet;
	t_outlet            *x_status_outlet;
} t_cursor;

t_widgetbehavior cursor_widgetbehavior;
static t_class *cursor_class;

static void cursor_motion(t_cursor *x, t_floatarg dx, t_floatarg dy)
{

}

static void cursor_bang(t_cursor *x)
{

}

static void cursor_float(t_cursor *x, t_floatarg f)
{

}

static void *cursor_new(t_symbol *s, int argc, t_atom *argv)
{
    t_cursor *x = (t_cursor *)pd_new(cursor_class);


    return (x);
}

static void cursor_free(t_cursor *x)
{

}

void cursor_setup(void)
{
    cursor_class = class_new(gensym("cursor"), (t_newmethod)cursor_new,
                              (t_method)cursor_free, sizeof(t_cursor), 0, A_GIMME, 0);
    class_addbang(cursor_class, cursor_bang);
    class_addfloat(cursor_class, cursor_float);

    cursor_widgetbehavior.w_getrectfn =    NULL;
    cursor_widgetbehavior.w_displacefn =   NULL;
    cursor_widgetbehavior.w_selectfn =     NULL;
    cursor_widgetbehavior.w_activatefn =   NULL;
    cursor_widgetbehavior.w_deletefn =     NULL;
    cursor_widgetbehavior.w_visfn =        NULL;
    cursor_widgetbehavior.w_clickfn =      NULL;
    class_setwidget(cursor_class, &cursor_widgetbehavior);
}
