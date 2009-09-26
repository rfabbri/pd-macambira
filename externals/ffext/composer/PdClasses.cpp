#include "PdClasses.hpp"
#include "Song.hpp"
#include "Track.hpp"
#include "Pattern.hpp"
#include "Editor.hpp"

#include "callwrappers_pd.cpp"
#include "callwrappers_ed.cpp"

#include <iostream>

#include "Common.hpp"

using std::cout;
using std::cerr;
using std::endl;

t_atom result_argv[MAX_RESULT_SIZE];
int result_argc;

t_class *track_proxy_class;
t_class *song_proxy_class;

void track_proxy_setup(void)
{
    track_proxy_class = class_new(
        gensym("track"),
        (t_newmethod)track_proxy_new,
        (t_method)track_proxy_free,
        sizeof(t_track_proxy),
        CLASS_DEFAULT,
        A_SYMBOL, A_SYMBOL, A_NULL
    );
#include "classsetup.cpp" 
    class_addmethod(track_proxy_class, (t_method)track_proxy_data, \
            gensym("data"), A_GIMME, A_NULL);
#if PD_MINOR_VERSION >= 37
    class_setpropertiesfn(track_proxy_class, track_proxy_properties);
    class_setsavefn(track_proxy_class, track_proxy_save);
#endif
    class_sethelpsymbol(track_proxy_class, gensym("track.pd"));
}

t_track_proxy *track_proxy_new(t_symbol *song_name, t_symbol *track_name)
{
    t_track_proxy *x = (t_track_proxy*)pd_new(track_proxy_class);
    x->outlet = outlet_new(&x->x_obj, &s_list);
    x->editor_open = 0;

    // get or create Track object:
    x->track = Track::byName(song_name->s_name, track_name->s_name);

    // set send/recv for communication with editor    
    Song *song = x->track->getSong();
    string base_name = "track_proxy-" + song->getName() + "-" + x->track->getName();
    string recv_name = base_name + "-r";
    string send_name = base_name + "-s";
    x->editor_recv = gensym(recv_name.c_str());
    x->editor_send = gensym(send_name.c_str());
    pd_bind(&x->x_obj.ob_pd, x->editor_recv);

    // bind to TRACK_SELECTOR for loading in-patch data
    pd_bind(&x->x_obj.ob_pd, gensym(TRACK_SELECTOR));

    Editor::init(x);

    return x;
}

void track_proxy_free(t_track_proxy *x)
{
    pd_unbind(&x->x_obj.ob_pd, gensym(TRACK_SELECTOR));
    /* LATER find a way to get TRACK_SELECTOR unbound earlier (at end of load?) */
    /*t_pd* x2;
    while((x2 = pd_findbyclass(gensym(TRACK_SELECTOR), track_proxy_class)))
        pd_unbind(x2, gensym(TRACK_SELECTOR));*/

    pd_unbind(&x->x_obj.ob_pd, x->editor_recv);
}

void track_proxy_save(t_gobj *z, t_binbuf *b)
{
    t_track_proxy *x = (t_track_proxy*)z;
    Track *t = x->track;
    Song *s = t->getSong();

    binbuf_addv(b, "ssiisss;", gensym("#X"), gensym("obj"),
        (t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,
        gensym("track"), gensym(s->getName().c_str()),
	gensym(t->getName().c_str()));

    for(map<string,Pattern *>::iterator i = t->patternsBegin(); i != t->patternsEnd(); i++)
    {
        Pattern *pattern = i->second;
        binbuf_addv(b, "ss", gensym(TRACK_SELECTOR), gensym("data"));
        t_int r = pattern->getRows();
        t_int c = pattern->getColumns();
        binbuf_addv(b, "sii", gensym(pattern->getName().c_str()), r, c);
        t_atom tmp;
        for(unsigned int j = 0; j < r; j++)
        {
            for(unsigned int k = 0; k < c; k++)
            {
                tmp = pattern->getCell(j, k);
                switch(tmp.a_type)
                {
                case A_SYMBOL:
                    binbuf_addv(b, "s", tmp.a_w.w_symbol);
                    break;
                case A_FLOAT:
                    binbuf_addv(b, "f", tmp.a_w.w_float);
                    break;
                default:
                    binbuf_addv(b, "s", gensym("?"));
                    break;
                }
            }
        }
        binbuf_addv(b, ";");
    }

    binbuf_addv(b, "sss;", gensym(TRACK_SELECTOR), gensym("data"), gensym("end"));
}

void track_proxy_data(t_track_proxy *x, t_symbol *s, int argc, t_atom *argv)
{
    if(argc == 1 && IS_A_SYMBOL(argv,0) && argv[0].a_w.w_symbol == gensym("end"))
    {
        pd_unbind(&x->x_obj.ob_pd, gensym(TRACK_SELECTOR));
        return;
    }
    if(argc < 3 || !IS_A_SYMBOL(argv,0) || !IS_A_FLOAT(argv,1) || !IS_A_FLOAT(argv,2))
    {
        pd_error(x, "unrecognized format for in-patch data");
        return;
    }
    t_symbol *pat = argv[0].a_w.w_symbol;
    t_int rows = (t_int) argv[1].a_w.w_float;
    t_int cols = (t_int) argv[2].a_w.w_float;
    if(argc != (3 + rows * cols))
    {
        pd_error(x, "unrecognized format for in-patch data (malformed data?)");
        return;
    }
    Track *t = x->track;
    t->addPattern(rows, cols, pat->s_name);
    Pattern *p = t->getPattern(pat->s_name);
    if(!p)
    {
        pd_error(x, "fatal error: cannot create patern");
        return;
    }
    int a = 3;
    for(int r = 0; r < rows; r++)
    {
        for(int c = 0; c < cols; c++)
        {
            p->setCell(r, c, argv[a]);
            a++;
        }
    }
}

void track_proxy_properties(t_gobj *z, t_glist *owner)
{
    t_track_proxy *x = (t_track_proxy *) z;
    if(!x->editor_open) Editor::openWindow(x);
    else Editor::closeWindow(x);
}

void track_proxy_send_result(t_track_proxy *x, int outlet, int editor)
{
    if(result_argc <= 0) return;
    if(outlet)
    {
        outlet_list(x->outlet, &s_list, result_argc, &result_argv[0]);
    }
    if(editor)
    {
        Editor::dispatch(x, result_argc, &result_argv[0]);
    }
}

int track_proxy_editor(t_track_proxy *x, t_floatarg arg)
{
    t_int a = (t_int) arg;
    if(a < 0)
    {
        if(!x->editor_open) Editor::openWindow(x);
        else Editor::closeWindow(x);
    }
    else
    {
        if(a > 0) Editor::openWindow(x);
        else Editor::closeWindow(x);
    }
    result_argc = 0;
    return 0;
}

int track_proxy_getpatterns(t_track_proxy *x)
{
    SETSYMBOL(&result_argv[0], gensym("patternnames"));
    result_argc = 1;
    Track *t = x->track;
    for(map<string,Pattern *>::iterator i = t->patternsBegin(); i != t->patternsEnd(); i++)
    {
        if(result_argc >= MAX_RESULT_SIZE)
        {
            pd_error(x, "getpatternnames: result too long");
            return -2;
        }
        Pattern *pattern = i->second;
        SETSYMBOL(&result_argv[result_argc], gensym(pattern->getName().c_str()));
        result_argc++;
    }
    return 0;
}

int track_proxy_getpatternsize(t_track_proxy *x, t_symbol *pat)
{
    Pattern *pattern = x->track->getPattern(pat->s_name);
    if(!pattern)
    {
        pd_error(x, "getpatternsize: no such pattern: %s", pat->s_name);
        return -1;
    }
    SETSYMBOL(&result_argv[0], gensym("patternsize"));
    SETSYMBOL(&result_argv[1], pat);
    SETFLOAT(&result_argv[2], pattern->getRows());
    SETFLOAT(&result_argv[3], pattern->getColumns());
    result_argc = 4;
    return 0;
}

int track_proxy_setrow(t_track_proxy *x, t_symbol *sel, int argc, t_atom *argv)
{
    result_argc = 0;
    if(argc < 2 || !IS_A_SYMBOL(argv,0) || !IS_A_FLOAT(argv,1))
    {
        pd_error(x, "setrow: usage: setrow <pattern> <row#> <atom0> <atom1> ...");
        return -1;
    }
    t_symbol *pat = argv[0].a_w.w_symbol;
    Pattern *pattern = x->track->getPattern(pat->s_name);
    t_int r = (t_int) argv[1].a_w.w_float;
    if(!pattern)
    {
        pd_error(x, "setrow: no such pattern: %s", pat->s_name);
        return -2;
    }
    unsigned int argc2 = argc - 2;
    if(argc2 != pattern->getColumns())
    {
        pd_error(x, "setrow: input error: must provide exactly %d elements for a row", pattern->getColumns());
        return -3;
    }
    for(unsigned int i = 0; i < argc2; i++)
    {
        pattern->setCell(r, i, argv[i + 2]);
    }
    return 0;
}

int track_proxy_getrow(t_track_proxy *x, t_symbol *pat, t_floatarg rownum)
{
    t_int r = (t_int) rownum;
    Pattern *pattern = x->track->getPattern(pat->s_name);
    if(!pattern)
    {
        pd_error(x, "getrow: no such pattern: %s", pat->s_name);
        return -2;
    }
    SETSYMBOL(&result_argv[0], gensym("patternrow"));
    SETSYMBOL(&result_argv[1], pat);
    SETFLOAT(&result_argv[2], (t_float) r);
    result_argc = 3;
    for(unsigned int i = 0; i < pattern->getColumns(); i++)
    {
        if(result_argc >= MAX_RESULT_SIZE)
        {
            pd_error(x, "getrow: result too long");
            return -2;
        }
        result_argv[result_argc] = pattern->getCell(r, i);
        result_argc++;
    }
    return 0;
}

int track_proxy_setcell(t_track_proxy *x, t_symbol *sel, int argc, t_atom *argv)
{
    result_argc = 0;
    if(argc != 4 || !IS_A_SYMBOL(argv,0) || !IS_A_FLOAT(argv,1) || !IS_A_FLOAT(argv,2))
    {
        pd_error(x, "setrow: usage: setcell <pattern> <row#> <col#> <atom>");
        return -1;
    }
    t_symbol *pat = argv[0].a_w.w_symbol;
    Pattern *pattern = x->track->getPattern(pat->s_name);
    t_int r = (t_int) argv[1].a_w.w_float;
    t_int c = (t_int) argv[2].a_w.w_float;
    if(!pattern)
    {
        pd_error(x, "setcell: no such pattern: %s", pat->s_name);
        return -2;
    }
    pattern->setCell(r, c, argv[3]);
    return 0;
}

int track_proxy_getcell(t_track_proxy *x, t_symbol *pat, t_floatarg rownum, t_floatarg colnum)
{
    t_int r = (t_int) rownum;
    t_int c = (t_int) colnum;
    Pattern *pattern = x->track->getPattern(pat->s_name);
    if(!pattern)
    {
        pd_error(x, "getcell: no such pattern: %s", pat->s_name);
        return -2;
    }
    SETSYMBOL(&result_argv[0], gensym("patterncell"));
    SETSYMBOL(&result_argv[1], pat);
    SETFLOAT(&result_argv[2], (t_float) r);
    SETFLOAT(&result_argv[3], (t_float) c);
    result_argv[4] = pattern->getCell(r, c);
    result_argc = 5;
    return 0;
}


int track_proxy_addpattern(t_track_proxy *x, t_symbol *name, t_floatarg rows, t_floatarg cols)
{
    result_argc = 0;
    t_int r = (t_int) rows;
    t_int c = (t_int) cols;
    x->track->addPattern(r, c, string(name->s_name));
    return 0;
}

int track_proxy_removepattern(t_track_proxy *x, t_symbol *pat)
{
    result_argc = 0;
    Pattern *pattern = x->track->getPattern(pat->s_name);
    if(!pattern)
    {
        pd_error(x, "removepattern: no such pattern: %s", pat->s_name);
        return -2;
    }
    pd_error(x, "removepattern: not implemented yet");
    return -9;
}

int track_proxy_resizepattern(t_track_proxy *x, t_symbol *pat, t_floatarg rows, t_floatarg cols)
{
    result_argc = 0;
    t_int r = (t_int) rows;
    t_int c = (t_int) cols;
    Pattern *pattern = x->track->getPattern(pat->s_name);
    if(!pattern)
    {
        pd_error(x, "resizepattern: no such pattern: %s", pat->s_name);
        return -2;
    }
    pattern->resize(r, c);
    return 0;
}

int track_proxy_renamepattern(t_track_proxy *x, t_symbol *oldName, t_symbol *newName)
{
    result_argc = 0;
    Pattern *pattern = x->track->getPattern(oldName->s_name);
    if(!pattern)
    {
        pd_error(x, "resizepattern: no such pattern: %s", oldName->s_name);
        return -2;
    }
    x->track->renamePattern(oldName->s_name, newName->s_name);
    return 0;
}

int track_proxy_copypattern(t_track_proxy *x, t_symbol *src, t_symbol *dst)
{
    result_argc = 0;
    pd_error(x, "copypattern: not implemented yet");
    return -9;
}

void composer_setup()
{
    track_proxy_setup();
}
