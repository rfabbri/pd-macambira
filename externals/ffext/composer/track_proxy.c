/* ------------------------------------------------------------------------ */
/* Copyright (c) 2009 Federico Ferri.                                       */
/* For information on usage and redistribution, and for a DISCLAIMER OF ALL */
/* WARRANTIES, see the file, "LICENSE.txt," in this distribution.           */
/*                                                                          */
/* composer: a music composition framework for pure-data                    */
/*                                                                          */
/* This program is free software; you can redistribute it and/or            */
/* modify it under the terms of the GNU General Public License              */
/* as published by the Free Software Foundation; either version 2           */
/* of the License, or (at your option) any later version.                   */
/*                                                                          */
/* See file LICENSE for further informations on licensing terms.            */
/*                                                                          */
/* This program is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of           */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            */
/* GNU General Public License for more details.                             */
/*                                                                          */
/* You should have received a copy of the GNU General Public License        */
/* along with this program; if not, write to the Free Software Foundation,  */
/* Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.          */
/*                                                                          */
/* Based on PureData by Miller Puckette and others.                         */
/* ------------------------------------------------------------------------ */

#include "common.h"

t_class* track_proxy_class;

void track_proxy_setup(void) {
    debugprint("registering 'track' class...");
    ArrayListInit(songs, t_song*, 10);
    track_proxy_class = class_new(
        gensym("track"),
        (t_newmethod)track_proxy_new,
        (t_method)track_proxy_free,
        sizeof(t_track_proxy),
        CLASS_DEFAULT, //0,
        A_SYMBOL, A_SYMBOL, A_FLOAT,
        0
    );
    class_addfloat(track_proxy_class, track_proxy_float);
    class_addanything(track_proxy_class, track_proxy_anything);
    class_addmethod(track_proxy_class, (t_method)track_proxy_properties, \
            gensym("editor-open"), 0);
    class_addmethod(track_proxy_class, (t_method)track_proxy_properties_close, \
            gensym("editor-close"), 0);
    class_addmethod(track_proxy_class, (t_method)track_proxy_reload, \
            gensym("reload"), 0);
    class_addmethod(track_proxy_class, (t_method)track_proxy_setrow, \
            gensym("setrow"), A_GIMME, 0);
    class_addmethod(track_proxy_class, (t_method)track_proxy_getrow_o, \
            gensym("getrow"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(track_proxy_class, (t_method)track_proxy_setcell, \
            gensym("setcell"), A_GIMME, 0);
    class_addmethod(track_proxy_class, (t_method)track_proxy_getcell_o, \
            gensym("getcell"), A_SYMBOL, A_FLOAT, A_FLOAT, 0);
    class_addmethod(track_proxy_class, (t_method)track_proxy_addpattern, \
            gensym("addpattern"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(track_proxy_class, (t_method)track_proxy_removepattern, \
            gensym("removepattern"), A_SYMBOL, 0);
    class_addmethod(track_proxy_class, (t_method)track_proxy_resizepattern, \
            gensym("resizepattern"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(track_proxy_class, (t_method)track_proxy_renamepattern, \
            gensym("renamepattern"), A_SYMBOL, A_SYMBOL, 0);
    class_addmethod(track_proxy_class, (t_method)track_proxy_copypattern, \
            gensym("copypattern"), A_SYMBOL, A_SYMBOL, 0);
#if PD_MINOR_VERSION >= 37
    class_setpropertiesfn(track_proxy_class, track_proxy_properties);
    class_setsavefn(track_proxy_class, track_proxy_save);
#endif
    class_sethelpsymbol(track_proxy_class, gensym("track.pd"));
}

static t_track_proxy* track_proxy_new(t_symbol* song_name, t_symbol* track_name, t_floatarg cols) {
    t_track_proxy *x = (t_track_proxy*)pd_new(track_proxy_class);
    x->outlet = outlet_new(&x->x_obj, &s_list);
    x->x_track = track_new(song_name, track_name, (t_int)cols);
    x->b_editor_open = 0;
    char rcv_buf[80];
    sprintf(rcv_buf, "track_proxy-%s-%s", x->x_track->x_song->x_name->s_name, x->x_track->x_name->s_name);
    x->rcv = gensym(rcv_buf);
    pd_bind(&x->x_obj.ob_pd, x->rcv);
    debugprint("created an instance of t_track_proxy " PTR ", to_track = " PTR, x, x->x_track);

    track_proxy_properties_close((t_gobj*) x, NULL);

    pd_bind(&x->x_obj.ob_pd, gensym(TRACK_SELECTOR));

    sys_vgui("pd::composer::init %s %s %s %d %d\n", x->rcv->s_name, x->x_track->x_song->x_name->s_name, x->x_track->x_name->s_name, x->x_track->x_ncolumns, DEBUG_BOOL);

    return x;
}

static void track_proxy_free(t_track_proxy* x) {
    track_proxy_properties_close((t_gobj*) x, NULL);

    pd_unbind(&x->x_obj.ob_pd, gensym(TRACK_SELECTOR));
    /* LATER find a way to get #TRACK unbound earlier (at end of load?) */
    t_pd* x2;
    while (x2 = pd_findbyclass(gensym(TRACK_SELECTOR), track_proxy_class))
        pd_unbind(x2, gensym(TRACK_SELECTOR));

    pd_unbind(&x->x_obj.ob_pd, x->rcv);
}

static void track_proxy_sendgui_pattern_names(t_track_proxy* x) {
    debugprint("track_proxy_sendgui_pattern_names(" PTR ")", x);
    t_int n = track_get_pattern_count(x->x_track);
    t_atom* a = (t_atom*)getbytes(sizeof(t_atom)*n);
    track_get_pattern_names(x->x_track, a);
    track_proxy_sendgui(x, gensym("patterns"), n, a);
    freebytes(a, sizeof(t_atom)*n);
}

static void track_proxy_reload(t_track_proxy* x) {
    sys_vgui("source {window.tk}\n");
}

static void track_proxy_properties(t_gobj* z, t_glist* owner) {
    t_track_proxy* x = (t_track_proxy*)z;
    sys_vgui("pd::composer::openWindow %s\n", x->rcv->s_name);
    x->b_editor_open = 1;
}

static void track_proxy_properties_close(t_gobj* z, t_glist* owner) {
    t_track_proxy* x = (t_track_proxy*)z;
    debugprint("track_proxy_properties_close(" PTR ", " PTR ") [editor is %s]", z, owner, x->b_editor_open ? "open" : "closed");
    if(x->b_editor_open) {
        debugprint("closing...");
        sys_vgui("pd::composer::closeWindow %s\n", x->rcv->s_name);
        x->b_editor_open = 0;
    }
}

static void track_proxy_save(t_gobj* z, t_binbuf* b) {
    t_track_proxy* x = (t_track_proxy*)z;
    t_track* t = x->x_track;

    binbuf_addv(b, "ssiisssi;", gensym("#X"), gensym("obj"),
        (t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,
        gensym("track"), t->x_song->x_name, t->x_name, t->x_ncolumns);

    // data format:
    // #TRACK DATA <npatterns> [<pat_name> <pat rows> RxC_atoms]*n

    binbuf_addv(b, "ssi", gensym(TRACK_SELECTOR), gensym("DATA"), t->x_patterns_count);

    int i,j,k;
    for(i = 0; i < t->x_patterns_count; i++) {
        t_pattern* pat = t->x_patterns[i];
        binbuf_addv(b, "si", pat->x_name, pat->x_rows_count);
        for(j = 0; j < pat->x_rows_count; j++) {
            for(k = 0; k < t->x_ncolumns; k++) {
                switch(pat->x_rows[j][k].a_type) {
                case A_FLOAT: binbuf_addv(b, "i", pat->x_rows[j][k].a_w.w_float); break;
                case A_SYMBOL: binbuf_addv(b, "s", pat->x_rows[j][k].a_w.w_symbol); break;
                default: binbuf_addv(b, "s", gensym("?")); break;
                }
            }
        }
    }

    binbuf_addv(b, ";");
}

static void track_proxy_sendrow(t_track_proxy* x, t_pattern* pat, t_int row) {
    t_track* t = x->x_track;
    if(row < 0) row = 0;
    row = row % pat->x_rows_count;
    outlet_list(x->outlet, &s_list, t->x_ncolumns, pat->x_rows[row]);
}

static void track_proxy_anything(t_track_proxy* x, t_symbol* s, int argc, t_atom* argv) {
    debugprint("track_proxy_anything(" PTR ", %s, %d, " PTR ")", s, s->s_name, argc, argv);

    if(s == gensym("DATA")) {
        track_proxy_loaddata(x, s, argc, argv);
        return;
    } else if(s == gensym("EDIT")) {
        track_proxy_editcmd(x, s, argc, argv);
        return;
    } else {
        error("unrecognized command for anything method: %s ", s->s_name);
        return;
    }
}

static void track_proxy_loaddata(t_track_proxy* x, t_symbol* s, int argc, t_atom* argv) {
    int i,base;
    base = 0;

    if(argc < (base+1) || argv[base].a_type != A_FLOAT) {
        error("track: data format error 2");
        return;
    }

    t_int npatterns = (t_int)argv[base].a_w.w_float;
    debugprint("track: %d patterns to read", npatterns);
    base += 1;

    t_symbol* patname;
    t_int patrows;
    t_pattern* pat;

    debugprint("track_proxy_loaddata(" PTR ", %s, %d, " PTR ")", x, s->s_name, argc, argv);
    for(i = 0; i < npatterns; i++) {
        debugprint("reading pattern %d...", i);
        if(argc < (base + 2)) {
            error("track: data format error 3 (i=%d)", i);
            return;
        }
        if(argv[base+0].a_type != A_SYMBOL || argv[base+1].a_type != A_FLOAT) {
            error("track: data format error 4 (i=%d)", i);
            return;
        }
        patname = argv[base+0].a_w.w_symbol;
        patrows = (t_int)argv[base+1].a_w.w_float;
        debugprint("pattern %d: name='%s', length=%d, RxC=%d", i, patname->s_name, patrows,
                patrows * x->x_track->x_ncolumns);
        base += 2;
        if(argc >= (base + patrows * x->x_track->x_ncolumns) && patrows > 0) {
            pat = pattern_new(x->x_track, patname, patrows);
            debugprint("created new pattern " PTR " ('%s', %d rows) for track " PTR, pat, patname->s_name, patrows, x->x_track);
            int j,h,k;
            for(h = 0, j = base; j < (base + patrows * x->x_track->x_ncolumns); j += x->x_track->x_ncolumns, h++) {
                debugprint("  working on row %d", h);
                for(k = 0; k < x->x_track->x_ncolumns; k++) {
                    pattern_setcell(pat, h, k, &argv[j+k]);
                }
            }
            base += patrows * x->x_track->x_ncolumns;
        } else {
            error("track: data format error 8 (i=%d)", i);
            return;
        }
    }
}

static t_atom* track_proxy_getpatternlength(t_track_proxy* x, t_symbol* pat_name) {
    /*if(argc < 1 || argv[0].a_type != A_SYMBOL) {
        error("track: getpatternlength: usage: getpatternlength <pattern_name>");
        return NULL;
    }
    t_symbol* pat_name = argv[0].a_w.w_symbol;*/
    ArrayListGetByName(x->x_track->x_patterns, pat_name, t_pattern*, pat);
    if(!pat) {
        error("track: getpatternlength: no such pattern: '%s'", pat_name->s_name);
        return NULL;
    }
    t_atom* pl = (t_atom*)getbytes(sizeof(t_atom) * 2);
    SETSYMBOL(&pl[0], pat->x_name);
    SETFLOAT(&pl[1], pat->x_rows_count);
    return pl;
}

static void track_proxy_editcmd(t_track_proxy* x, t_symbol* s_, int argc, t_atom* argv_) {
    if(argc < 1 || argv_[0].a_type != A_SYMBOL) {
        error("track: editcmd: missing method selector");
        return;
    }
    /*if(argc < 2) {
        error("track: editcmd: missing data after selector");
        return;
    }*/

    // route -> selector
    t_symbol* s = argv_[0].a_w.w_symbol;
    t_atom* argv = &argv_[1];
    argc--;

    t_symbol *s1 = (argc >= 1 && argv[0].a_type == A_SYMBOL) ? argv[0].a_w.w_symbol : NULL;
    t_symbol *s2 = (argc >= 2 && argv[1].a_type == A_SYMBOL) ? argv[1].a_w.w_symbol : NULL;
    t_float f2 = (argc >= 2 && argv[1].a_type == A_FLOAT) ? argv[1].a_w.w_float : 0;
    t_float f3 = (argc >= 3 && argv[2].a_type == A_FLOAT) ? argv[2].a_w.w_float : 0;

    t_pattern* p = NULL;
    int i,j;

    if(s == gensym("editor-open")) {
        track_proxy_properties((t_gobj*) x, NULL);
    } else if(s == gensym("editor-close")) {
        track_proxy_properties_close((t_gobj*) x, NULL);
    } else if(s == gensym("getpatterns")) {
        track_proxy_sendgui_pattern_names(x);
    } else if(s == gensym("getpatternlength")) {
        track_proxy_sendgui(x, gensym("patternlength"), 2, track_proxy_getpatternlength(x, s1));
    } else if(s == gensym("getrow")) {
        track_proxy_sendgui(x, gensym("row"), x->x_track->x_ncolumns + 2, track_proxy_getrow_with_header(x, s1, f2));
    } else if(s == gensym("setrow")) {
        track_proxy_setrow(x, s, argc, argv);
    } else if(s == gensym("getcell")) {
        track_proxy_sendgui(x, gensym("cell"), 4, track_proxy_getcell_with_header(x, s1, f2, f3));
    } else if(s == gensym("setcell")) {
        track_proxy_setcell(x, s, argc, argv);
    } else if(s == gensym("addpattern")) {
        p = track_proxy_addpattern(x, s1, f2);
        if(p) {
            debugprint("BAMBOLOOOOOO");
            //for(i = 0; i < p->x_rows_count; i++)
            //    track_proxy_sendgui(x, gensym("row"), x->x_track->x_ncolumns + 2, track_proxy_getrow_with_header(x, p->x_name, i));
            track_proxy_sendgui_pattern_names(x);
        }
    } else if(s == gensym("removepattern")) {
        j = track_proxy_removepattern(x, s1);
        if(j) {
            track_proxy_sendgui_pattern_names(x);
        }
    } else if(s == gensym("resizepattern")) {
        p = track_proxy_resizepattern(x, s1, f2);
        if(p) {
            track_proxy_sendgui(x, gensym("patternlength"), 2, track_proxy_getpatternlength(x, p->x_name));
        }
    } else if(s == gensym("renamepattern")) {
        p = track_proxy_renamepattern(x, s1, s2);
        if(p) {
            track_proxy_sendgui_pattern_names(x);
        }
    } else if(s == gensym("copypattern")) {
        p = track_proxy_copypattern(x, s1, s2);
        if(p) {
            //for(i = 0; i < p->x_rows_count; i++)
            //    track_proxy_sendgui(x, gensym("row"), x->x_track->x_ncolumns + 2, track_proxy_getrow_with_header(x, p->x_name, i));
            track_proxy_sendgui_pattern_names(x);
        }
    } else {
        error("track: editcmd: unknown command: %s", s->s_name);
    }
}

static void track_proxy_sendgui(t_track_proxy* x, t_symbol* s, int argc, t_atom* argv) {
    static const unsigned int tmpsz = 80;
    static const unsigned int bufsz = 8*MAXPDSTRING;
    char* tmp;
    char buf[bufsz];
    int i,j;
    buf[0] = '\0';
    strncat(buf, s->s_name, bufsz);
    strncat(buf, " ", bufsz);
    for(i = 0; i < argc; i++) {
        if(i > 0) strncat(buf, " ", bufsz);
        if(argv[i].a_type == A_FLOAT) {
            tmp = (char*)getbytes(tmpsz*sizeof(char));
            if(argv[i].a_w.w_float == (t_float)(t_int)argv[i].a_w.w_float)
                sprintf(tmp, "%ld", (t_int)argv[i].a_w.w_float);
            else
                sprintf(tmp, "%f", argv[i].a_w.w_float);
            strncat(buf, tmp, bufsz);
        } else if(argv[i].a_type == A_SYMBOL) {
            strncat(buf, argv[i].a_w.w_symbol->s_name, bufsz);
        } else {
            strncat(buf, "null", bufsz);
        }
    }
    if(strlen(buf) >= bufsz) {
        bug("track: sendgui: message too long");
        return;
    }
    sys_vgui("pd::composer::dispatch %s %s\n", x->rcv->s_name, buf);
}

static void track_proxy_float(t_track_proxy* x, t_floatarg f) {
    t_track* t = x->x_track;
    if(t->x_patterns_count < 1) return;

    t_int curpat_i = (t_int) t->x_currentpat;

    if(curpat_i < 0) curpat_i = 0;
    curpat_i = curpat_i % t->x_patterns_count;

    t_pattern* curpat = t->x_patterns[curpat_i];
    track_proxy_sendrow(x, curpat, (t_int) f);
}

static void track_proxy_setrow(t_track_proxy* x, t_symbol* sel, int argc, t_atom* argv) {
    if(argc < 2 || argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT) {
        error("track: setrow: usage: setrow <pattern_name> <row#> <atom0> <atom1> ...");
        return;
    }
    if((argc - 2) != x->x_track->x_ncolumns) {
        post("argc=%d, ncolumns=%d", argc, x->x_track->x_ncolumns);
        error("track: setrow: input error: must provide exactly %d elements for the row", x->x_track->x_ncolumns);
        return;
    }
    t_symbol* pat_name = argv[0].a_w.w_symbol;
    t_int rownum = (t_int)argv[1].a_w.w_float;
    ArrayListGetByName(x->x_track->x_patterns, pat_name, t_pattern*, pat);
    if(!pat) {
        error("track: setrow: no such pattern: '%s'", pat_name->s_name);
        return;
    }
    pattern_setrow(pat, rownum, &argv[2]);
}

static t_atom* track_proxy_getrow(t_track_proxy* x, t_symbol* pat_name, t_floatarg rownum) {
    ArrayListGetByName(x->x_track->x_patterns, pat_name, t_pattern*, pat);
    if(!pat) {
        error("track: getrow: no such pattern: '%s'", pat_name->s_name);
        return NULL;
    }
    t_atom* row = pattern_getrow(pat, (t_int)rownum);
    debugprint("track_proxy_getrow returning " PTR, row);
    return row;
}

static t_atom* track_proxy_getrow_with_header(t_track_proxy* x, t_symbol* pat_name, t_floatarg rownum) {
    t_atom* row = track_proxy_getrow(x, pat_name, rownum);
    if(!row) {
        error("track: getrow: nu such patern: '%s'", pat_name->s_name);
        return NULL;
    }
    t_atom* row_with_hdr = (t_atom*)getbytes(sizeof(t_atom) * (x->x_track->x_ncolumns + 2));
    SETSYMBOL(&row_with_hdr[0], pat_name);
    SETFLOAT(&row_with_hdr[1], rownum);
    memcpy(&row_with_hdr[2], row, sizeof(t_atom) * x->x_track->x_ncolumns);
    return row_with_hdr;
}

static void track_proxy_getrow_o(t_track_proxy* x, t_symbol* pat_name, t_floatarg rownum) {
    t_atom* row = track_proxy_getrow(x, pat_name, rownum);
    if(row)
        outlet_list(x->outlet, &s_list, x->x_track->x_ncolumns, row);
}

static void track_proxy_setcell(t_track_proxy* x, t_symbol* sel, int argc, t_atom* argv) {
    debugprint("track_proxy_setcell, s=%s", sel->s_name);
    if(argc < 4 || argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT || argv[2].a_type != A_FLOAT) {
        error("track: setcell: usage: setcell <pattern_name> <row#> <col#> <atom>");
        return;
    }
    t_symbol* pat_name = argv[0].a_w.w_symbol;
    t_int rownum = (t_int)argv[1].a_w.w_float;
    t_int colnum = (t_int)argv[2].a_w.w_float;
    ArrayListGetByName(x->x_track->x_patterns, pat_name, t_pattern*, pat);
    if(!pat) {
        error("track: setcell: no such pattern: '%s'", pat_name->s_name);
        return;
    }
    pattern_setcell(pat, (t_int)rownum, (t_int)colnum, &argv[3]);
}

static t_atom* track_proxy_getcell(t_track_proxy* x, t_symbol* pat_name, t_floatarg rownum, t_floatarg colnum) {
    ArrayListGetByName(x->x_track->x_patterns, pat_name, t_pattern*, pat);
    if(!pat) {
        error("track: getcell: no such pattern: '%s'", pat_name->s_name);
        return NULL;
    }
    return pattern_getcell(pat, (t_int)rownum, (t_int)colnum);
}

static t_atom* track_proxy_getcell_with_header(t_track_proxy* x, t_symbol* pat_name, t_floatarg rownum, t_floatarg colnum) {
    t_atom* cell = track_proxy_getcell(x, pat_name, rownum, colnum);
    if(!cell) {
        error("track: getcell: nu such patern: '%s'", pat_name->s_name);
        return NULL;
    }
    t_atom* row_with_hdr = (t_atom*)getbytes(sizeof(t_atom) * 4);
    SETSYMBOL(&row_with_hdr[0], pat_name);
    SETFLOAT(&row_with_hdr[1], rownum);
    SETFLOAT(&row_with_hdr[2], colnum);
    memcpy(&row_with_hdr[3], cell, sizeof(t_atom));
    return row_with_hdr;
}

static void track_proxy_getcell_o(t_track_proxy* x, t_symbol* pat_name, t_floatarg rownum, t_floatarg colnum) {
    t_atom* cell = track_proxy_getcell(x, pat_name, rownum, colnum);
    if(cell)
        outlet_list(x->outlet, &s_list, 1, cell);
}

static t_pattern* track_proxy_addpattern(t_track_proxy* x, t_symbol* name, t_floatarg rows) {
    return pattern_new(x->x_track, name, (t_int)rows);
}

static int track_proxy_removepattern(t_track_proxy* x, t_symbol* name) {
    ArrayListGetByName(x->x_track->x_patterns, name, t_pattern*, pat);
    if(!pat) {
        error("track: removepattern: no such pattern: '%s'", name->s_name);
        return 0; //FAIL
    }
    pattern_free(pat);
    return 1; //OK
}

static t_pattern* track_proxy_resizepattern(t_track_proxy* x, t_symbol* name, t_floatarg rows) {
    ArrayListGetByName(x->x_track->x_patterns, name, t_pattern*, pat);
    if(!pat) {
        error("track: resizepattern: no such pattern: '%s'", name->s_name);
        return NULL;
    }
    pattern_resize(pat, (t_int)rows);
    return pat;
}

static t_pattern* track_proxy_renamepattern(t_track_proxy* x, t_symbol* name, t_symbol* newname) {
    ArrayListGetByName(x->x_track->x_patterns, name, t_pattern*, pat);
    if(!pat) {
        error("track: renamepattern: no such pattern: '%s'", name->s_name);
        return NULL;
    }
    pattern_rename(pat, newname);
    return pat;
}

static t_pattern* track_proxy_copypattern(t_track_proxy* x, t_symbol* src, t_symbol* dst) {
    ArrayListGetByName(x->x_track->x_patterns, src, t_pattern*, pat);
    if(!pat) {
        error("track: copypattern: no such pattern: '%s'", src->s_name);
        return NULL;
    }
    ArrayListGetByName(x->x_track->x_patterns, dst, t_pattern*, pat2);
    if(pat2) {
        error("track: copypattern: destination '%s' exists", pat2->x_name->s_name);
        return NULL;
    }
    return pattern_clone(pat, dst);
}
