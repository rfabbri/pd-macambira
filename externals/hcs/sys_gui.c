#include <stdio.h>
#include <string.h>
#include <m_pd.h>
#include <g_canvas.h>

static t_class *sys_gui_class;

typedef struct _sys_gui
{
    t_object x_obj;
} t_sys_gui;


static void *sys_gui_anything(t_sys_gui *x, t_symbol *s, int argc, t_atom *argv)
{
    int i = 0;
    int firsttime = 1;
    t_symbol *tmp_symbol = s;
    char tmp_string[MAXPDSTRING];
    char send_buffer[MAXPDSTRING] = "\0";
    
    do {
        tmp_symbol = atom_getsymbolarg(i, argc, argv);
        if(tmp_symbol == &s_)
        {
            snprintf(tmp_string, MAXPDSTRING, "%g", atom_getfloatarg(i, argc , argv));
            strncat(send_buffer, tmp_string, MAXPDSTRING);
        }
        else 
        {
            strncat(send_buffer, tmp_symbol->s_name, MAXPDSTRING);
        }
        i++;
        if(firsttime) firsttime = 0;
    } while(i<argc);
    strncat(send_buffer, " ;\n", MAXPDSTRING);
    post(send_buffer);
    sys_gui(send_buffer);
}


static void *sys_gui_new(t_symbol *s, int argc, t_atom *argv)
{
    t_sys_gui *x = (t_sys_gui *)pd_new(sys_gui_class);

	outlet_new(&x->x_obj, &s_anything);

    return(x);
}

static void sys_gui_free(t_sys_gui *x)
{
}

void sys_gui_setup(void)
{
    sys_gui_class = class_new(gensym("sys_gui"),
        (t_newmethod)sys_gui_new, (t_method)sys_gui_free,
        sizeof(t_sys_gui), 0, 0);

    class_addanything(sys_gui_class, (t_method)sys_gui_anything);
}
