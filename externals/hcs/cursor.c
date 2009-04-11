/* TODO add reset method for cursor icons, this should probably be done in
pd.tk, or cursor reset method could be done in help patch */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <m_pd.h>
#include <g_canvas.h>

static t_symbol *button_symbol;
static t_symbol *wheel_symbol;
static t_symbol *motion_symbol;
static t_symbol *x_symbol;
static t_symbol *y_symbol;
static t_symbol *cursor_receive_symbol;

t_int cursor_instance_count;
t_int cursor_instances_polling;

static t_class *cursor_class;

typedef struct _cursor
{
    t_object x_obj;
    t_symbol *receive_symbol;
    t_canvas *parent_canvas;
    t_outlet *data_outlet;
//    t_outlet *status_outlet; // not used (yet?)
    t_int optionc;
    char *optionv[];
} t_cursor;

/* idea from #tcl for a Tcl unbind */

static void create_unbind (void)
{
    sys_gui("if { [info commands ::hcs_cursor_class::unbind] ne {::hcs_cursor_class::unbind}} {");
    sys_gui("  puts stderr {creating ::hcs_cursor_class::unbind}\n");
    sys_gui("  proc ::hcs_cursor_class::unbind {tag event script} {\n");
    sys_gui("    set bind {}\n");
    sys_gui("    foreach x [split [bind $tag $event] \"\n\"] {\n");
    sys_gui("      if {$x != $script} {\n");
    sys_gui("        lappend bind $x\n");
    sys_gui("        puts stderr {rebind $x $script}\n");
    sys_gui("      }\n");
    sys_gui("    }\n");
    sys_gui("    bind $tag $event {}\n");
    sys_gui("    foreach x $bind {bind $tag $event $x}\n");
    sys_gui("  }\n");
    sys_gui("}\n");
}

static void create_namespace(void)
{
    sys_gui("if { [namespace exists ::hcs_cursor_class]} {\n");
    sys_gui("  puts stderr {WARNING: ::hcs_cursor_class namespace exists!}\n");
    sys_gui("} else {\n");
    sys_gui("  namespace eval ::hcs_cursor_class {\n");
    sys_gui("    variable send_to_pd 0\n");
    sys_gui("    variable last_x 0\n");
    sys_gui("    variable last_y 0\n");
    sys_gui("  }\n");
    sys_gui("}\n");
}

static void create_button_proc(void)
{
}

static void create_mousewheel_proc(void)
{
}

static void create_motion_proc(void)
{
    /* create proc and bind it, if it doesn't exist */
    sys_gui("if {[info commands ::hcs_cursor_class::motion] ne {::hcs_cursor_class::motion}} {");
    sys_gui ("  puts stderr {creating ::hcs_cursor_class::motion}\n");
    sys_gui ("  proc ::hcs_cursor_class::motion {x y} {\n");
//    sys_gui ("    if {{$::hcs_cursor_class::send_to_pd > 0} {\n");
//    sys_gui ("      if { $x != $::hcs_cursor_class::last_x \\\n");
//    sys_gui ("        || $y != $::hcs_cursor_class::last_y} {\n");
    sys_vgui("        pd [concat %s motion $x $y \\;]\n",
             cursor_receive_symbol->s_name);
//    sys_gui ("      }\n");
//    sys_gui ("    }\n");
    sys_gui ("  }\n");
    sys_gui ("}\n");
    sys_gui ("\n");
}

static void cursor_setmethod(t_cursor *x, t_symbol *s, int argc, t_atom *argv)
{
    sys_vgui("set cursor_%s \"%s\"\n", s->s_name, atom_getsymbol(argv)->s_name);
    canvas_setcursor(x->parent_canvas, 0); /* hack to refresh the cursor */
}

static void cursor_bang(t_cursor *x)
{
    sys_vgui("pd [concat %s motion [winfo pointerx .] [winfo pointery .] \\;]\n",
             x->receive_symbol->s_name);
}

static void cursor_float(t_cursor *x, t_float f)
{
    if(f > 0)
    {
        // TODO make sure the procs are setup
        // TODO bind to the global cursor receive symbol
        pd_bind(&x->x_obj.ob_pd, cursor_receive_symbol);
        cursor_instances_polling++;
        /* if this is the first instance to start polling, set up the bind */
        post("cursor_instances_polling %d", cursor_instances_polling);
        if (cursor_instances_polling == 1) {
            post("bind all <Motion>");
            sys_gui ("bind all <Motion> {+::hcs_cursor_class::motion %x %y}\n");
        }
    }
    else
    {
        // TODO unbind from the global cursor receive symbol
        pd_unbind(&x->x_obj.ob_pd, cursor_receive_symbol);
        cursor_instances_polling--;
         /* if no more objects are listening, stop sending the events */
        post("cursor_instances_polling %d", cursor_instances_polling);
        if (cursor_instances_polling == 0) {
            post("unbind all <Motion>");
            sys_gui ("::hcs_cursor_class::unbind all <Motion> {::hcs_cursor_class::motion %x %y}\n");
        }
    }
}

static void cursor_button_callback(t_cursor *x, t_float button, t_float state)
{
    t_atom output_atoms[2];
    
    SETFLOAT(output_atoms, button);
    SETFLOAT(output_atoms + 1, state);
    outlet_anything(x->data_outlet, button_symbol, 2, output_atoms);
}

static void cursor_motion_callback(t_cursor *x, t_float x_position, t_float y_position)
{
    t_atom output_atoms[2];
    
    SETSYMBOL(output_atoms, x_symbol);
    SETFLOAT(output_atoms + 1, x_position);
    outlet_anything(x->data_outlet, motion_symbol, 2, output_atoms);
    SETSYMBOL(output_atoms, y_symbol);
    SETFLOAT(output_atoms + 1, y_position);
    outlet_anything(x->data_outlet, motion_symbol, 2, output_atoms);
}

static void cursor_wheel_callback(t_cursor *x, t_float f)
{
    t_atom output_atom;
    
    SETFLOAT(&output_atom, f);
    outlet_anything(x->data_outlet, wheel_symbol, 1, &output_atom);
}

static void cursor_free(t_cursor *x)
{
    cursor_float(x, 0);
    /* TODO free the "bind all" somehow so that the tcl procs aren't
     * continuing to work even tho no cursor objects are receiving the data */
    cursor_instance_count--;
}

static void *cursor_new(void)
{
    char buf[MAXPDSTRING];
    t_cursor *x = (t_cursor *)pd_new(cursor_class);

    x->parent_canvas = canvas_getcurrent();

    sprintf(buf, "#%lx", (t_int)x);
    x->receive_symbol = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->receive_symbol);
	x->data_outlet = outlet_new(&x->x_obj, 0);
//	x->status_outlet = outlet_new(&x->x_obj, 0);

/* not working yet
    sys_vgui("bind . <ButtonPress> {+pd [concat %s button %%b 1 \\;]}\n",
             x->receive_symbol->s_name);
    sys_vgui("bind . <ButtonRelease> {+pd [concat %s button %%b 0 \\;]}\n",
             x->receive_symbol->s_name);
    sys_vgui("bind . <MouseWheel> {+pd [concat %s wheel %%D \\;]}\n",
             x->receive_symbol->s_name);
*/
    cursor_instance_count++;

    return(x);
}

void cursor_setup(void)
{
    cursor_class = class_new(gensym("cursor"),
        (t_newmethod)cursor_new, (t_method)cursor_free,
        sizeof(t_cursor), 0, 0);

    class_addbang(cursor_class, (t_method)cursor_bang);
    class_addfloat(cursor_class, (t_method)cursor_float);

    button_symbol = gensym("button");
    wheel_symbol = gensym("wheel");
    motion_symbol = gensym("motion");
    x_symbol = gensym("x");
    y_symbol = gensym("y");
    cursor_receive_symbol = gensym("#hcs_cursor_class_receive");

    class_addmethod(cursor_class, (t_method)cursor_button_callback, 
                    button_symbol, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(cursor_class, (t_method)cursor_motion_callback, 
                    motion_symbol, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(cursor_class, (t_method)cursor_wheel_callback, 
                    wheel_symbol, A_DEFFLOAT, 0);

    /* methods for setting the cursor icon */
    class_addmethod(cursor_class, (t_method)cursor_setmethod, 
                    gensym("runmode_nothing"), A_GIMME, 0);
    class_addmethod(cursor_class, (t_method)cursor_setmethod, 
                    gensym("runmode_clickme"), A_GIMME, 0);
    class_addmethod(cursor_class, (t_method)cursor_setmethod, 
                    gensym("runmode_thicken"), A_GIMME, 0);
    class_addmethod(cursor_class, (t_method)cursor_setmethod, 
                    gensym("runmode_addpoint"), A_GIMME, 0);
    class_addmethod(cursor_class, (t_method)cursor_setmethod, 
                    gensym("editmode_nothing"), A_GIMME, 0);
    class_addmethod(cursor_class, (t_method)cursor_setmethod, 
                    gensym("editmode_connect"), A_GIMME, 0);
    class_addmethod(cursor_class, (t_method)cursor_setmethod, 
                    gensym("editmode_disconnect"), A_GIMME, 0);

    create_namespace();
    create_unbind();
    create_motion_proc();
}
