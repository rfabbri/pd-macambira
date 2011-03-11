
#include "m_pd.h"
#include <string.h>

/* -------------------------- logpost ------------------------------ */
static t_class *logpost_class;

typedef struct _logpost
{
    t_object x_obj;
    t_float level;
    t_symbol* tag;
} t_logpost;

static void logpost_bang(t_logpost *x)
{
    logpost(x, (const int)x->level, "%s%sbang", 
            x->tag->s_name, (*x->tag->s_name ? ": " : ""));
}

static void logpost_pointer(t_logpost *x, t_gpointer *gp)
{
    logpost(x, (const int)x->level, "%s%s(pointer %lx)", 
            x->tag->s_name, (*x->tag->s_name ? ": " : ""), gp);
}

static void logpost_float(t_logpost *x, t_float f)
{
    logpost(x, (const int)x->level, "%s%s%g",
            x->tag->s_name, (*x->tag->s_name ? ": " : ""), f);
}

static void logpost_list(t_logpost *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    if (argc && argv->a_type != A_SYMBOL) startpost("%s:", x->tag->s_name);
    else startpost("%s%s%s", x->tag->s_name,
        (*x->tag->s_name ? ": " : ""),
        (argc > 1 ? s_list.s_name : (argc == 1 ? s_symbol.s_name :
            s_bang.s_name)));
    postatom(argc, argv);
    endpost();
}

static void logpost_anything(t_logpost *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    startpost("%s%s%s", x->tag->s_name, (*x->tag->s_name ? ": " : ""),
        s->s_name);
    postatom(argc, argv);
    endpost();
}

static void *logpost_new(t_symbol *sel, int argc, t_atom *argv)
{
    t_logpost *x = (t_logpost *)pd_new(logpost_class);

    if (argc == 0)
        x->tag = &s_;
    else if (argc == 1)
    {
        t_symbol *s = atom_getsymbolarg(0, argc, argv);
        if (s != &s_) // we have a symbol
            x->tag = s;
        else // we have a float
        {
            x->level = atom_getfloatarg(0, argc, argv);
        }
    }
    else
    {
        int bufsize;
        char *buf;
        t_binbuf *bb = binbuf_new();
        binbuf_add(bb, argc, argv);
        binbuf_gettext(bb, &buf, &bufsize);
        buf = resizebytes(buf, bufsize, bufsize+1);
        buf[bufsize] = 0;
        x->tag = gensym(buf);
        freebytes(buf, bufsize+1);
        binbuf_free(bb);
    }
    floatinlet_new(&x->x_obj, &x->level);
    return (x);
}

void logpost_setup(void)
{
    logpost_class = class_new(gensym("logpost"),
                              (t_newmethod)logpost_new,
                              0,
                              sizeof(t_logpost),
                              CLASS_DEFAULT,
                              A_GIMME, 0);
    class_addbang(logpost_class, logpost_bang);
    class_addfloat(logpost_class, logpost_float);
    class_addpointer(logpost_class, logpost_pointer);
    class_addlist(logpost_class, logpost_list);
    class_addanything(logpost_class, logpost_anything);
}
