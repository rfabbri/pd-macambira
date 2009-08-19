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

t_class* song_proxy_class;

static t_atom response_pattern_length[2];
static t_atom response_cell[4];
static t_atom* response_row; // TODO: memory leaks check
static size_t response_row_sz;

void song_proxy_setup(void) {
    debugprint("registering 'song' class...");
    song_proxy_class = class_new(
        gensym("song"),
        (t_newmethod)song_proxy_new,
        (t_method)song_proxy_free,
        sizeof(t_song_proxy),
        CLASS_DEFAULT, //0,
        A_SYMBOL,
        0
    );
    class_addfloat(song_proxy_class, song_proxy_float);
#if PD_MINOR_VERSION >= 37
    class_setpropertiesfn(song_proxy_class, song_proxy_properties);
    class_setsavefn(song_proxy_class, song_proxy_save);
#endif
    class_sethelpsymbol(song_proxy_class, gensym("song.pd"));
}

static t_song_proxy* song_proxy_new(t_symbol* song_name) {
    t_song_proxy *x = (t_song_proxy*)pd_new(song_proxy_class);
    x->outlet = outlet_new(&x->x_obj, &s_list);
    x->x_song = song_new(song_name);
    x->b_editor_open = 0;
    char rcv_buf[80];
    sprintf(rcv_buf, "track_proxy-%s-%s", x->x_song->x_name->s_name, x->x_song->x_mastertrack->x_name->s_name);
    x->rcv = gensym(rcv_buf);
    pd_bind(&x->x_obj.ob_pd, x->rcv);

    debugprint("created an instance of t_song_proxy: " PTR, x);
    return x;

    song_proxy_properties_close((t_gobj*) x, NULL);

    pd_bind(&x->x_obj.ob_pd, gensym(SONG_SELECTOR));

    sys_vgui("pd::composer::init %s %s %s %d %d\n", x->rcv->s_name, x->x_song->x_name->s_name, x->x_song->x_mastertrack->x_name->s_name, x->x_song->x_mastertrack->x_ncolumns, DEBUG_BOOL);

    return x;
}

static void song_proxy_free(t_song_proxy* x) {
    song_proxy_properties_close((t_gobj*) x, NULL);

    pd_unbind(&x->x_obj.ob_pd, gensym(SONG_SELECTOR));
    /* LATER find a way to get SONG_SELECTOR unbound earlier (at end of load?) */
    t_pd* x2;
    while (x2 = pd_findbyclass(gensym(SONG_SELECTOR), song_proxy_class))
        pd_unbind(x2, gensym(SONG_SELECTOR));

    pd_unbind(&x->x_obj.ob_pd, x->rcv);
}

static void song_proxy_float(t_song_proxy* x, t_floatarg f) {
}

static void song_proxy_properties(t_gobj* z, t_glist* owner) {
    t_song_proxy* x = (t_song_proxy*)z;
    sys_vgui("pd::composer::openWindow %s\n", x->rcv->s_name);
    x->b_editor_open = 1;
}

static void song_proxy_properties_close(t_gobj* z, t_glist* owner) {
    t_song_proxy* x = (t_song_proxy*)z;
    debugprint("song_proxy_properties_close(" PTR ", " PTR ") [editor is %s]", z, owner, x->b_editor_open ? "open" : "closed");
    if(x->b_editor_open) {
        debugprint("closing...");
        sys_vgui("pd::composer::closeWindow %s\n", x->rcv->s_name);
        x->b_editor_open = 0;
    }
}

static void song_proxy_save(t_gobj* z, t_binbuf* b) {
    t_track_proxy* x = (t_track_proxy*)z;
    t_track* t = x->x_track;

    binbuf_addv(b, "ssiisssi;", gensym("#X"), gensym("obj"),
        (t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,
        gensym("track"), t->x_song->x_name, t->x_name, t->x_ncolumns);

    track_binbuf_save(t, gensym(TRACK_SELECTOR), b);
}

static t_atom* song_proxy_gettracks(t_song_proxy* x) {
    if(response_row) {
        freebytes(response_row, response_row_sz);
        response_row = NULL;
        response_row_sz = 0;
    }
    response_row_sz = sizeof(t_atom) * x->x_song->x_tracks_count;
    response_row = (t_atom*)getbytes(response_row_sz);
    int i;
    for(i = 0; i < x->x_song->x_tracks_count; i++) {
        SETSYMBOL(&response_row[i], x->x_song->x_tracks[i]->x_name);
    }
    return &response_row[0];
}

static void song_proxy_gettracks_o(t_song_proxy* x) {
    t_atom* rsp = song_proxy_gettracks(x);
    if(rsp)
        outlet_list(x->outlet, &s_list, response_row_sz, rsp);
}

static t_int song_proxy_gettracks_count(t_song_proxy* x) {
    return x->x_song->x_tracks_count;
}

static void song_proxy_gettracks_count_o(t_song_proxy* x) {
    outlet_float(x->outlet, (t_float)song_proxy_gettracks_count(x));
}

static void song_proxy_anything(t_song_proxy* x, t_symbol* s, int argc, t_atom* argv) {
    debugprint("song_proxy_anything(" PTR ", %s, %d, " PTR ")", x, s->s_name, argc, argv);

    if(s == gensym("DATA")) {
        song_proxy_loaddata(x, s, argc, argv);
        return;
    } else if(s == gensym("EDIT")) {
        song_proxy_editcmd(x, s, argc, argv);
        return;
    } else {
        error("unrecognized command for anything method: %s ", s->s_name);
        return;
    }
}

static void song_proxy_loaddata(t_song_proxy* x, t_symbol* s, int argc, t_atom* argv) {
    track_loaddata(x->x_song->x_mastertrack, argc, argv);
}

static t_atom* song_proxy_getpatternlength(t_song_proxy* x, t_symbol* pat_name) {
    ArrayListGetByName(x->x_song->x_mastertrack->x_patterns, pat_name, t_pattern*, pat);
    if(!pat) {
        error("song: getpatternlength: no such pattern: '%s'", pat_name->s_name);
        return NULL;
    }
    SETSYMBOL(&response_pattern_length[0], pat->x_name);
    SETFLOAT(&response_pattern_length[1], pat->x_rows_count);
    return &response_pattern_length[0];
}

static void song_proxy_editcmd(t_song_proxy* x, t_symbol* s_, int argc, t_atom* argv_) {
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
        song_proxy_properties((t_gobj*) x, NULL);
    } else if(s == gensym("editor-close")) {
        song_proxy_properties_close((t_gobj*) x, NULL);
    } else if(s == gensym("gettracks")) {
        t_atom* rsp = song_proxy_get_track_names(x);
        song_proxy_sendgui(x, gensym("tracks"), response_row_sz, rsp);
    } else if(s == gensym("gettrackscount")) {
        t_atom a;
        SETFLOAT(&a, (t_float)song_proxy_gettracks_count(x));
        song_proxy_sendgui(x, gensym("trackscount"), 1, &a);
    } else if(s == gensym("getpatternlength")) {
        song_proxy_sendgui(x, gensym("patternlength"), 2, song_proxy_getpatternlength(x, s1));
    } else if(s == gensym("getrow")) {
        song_proxy_sendgui(x, gensym("row"), 2 + x->x_song->x_mastertrack->x_ncolumns, song_proxy_getrow_with_header(x, s1, f2));
    } else if(s == gensym("setrow")) {
        song_proxy_setrow(x, s, argc, argv);
    } else if(s == gensym("getcell")) {
        song_proxy_sendgui(x, gensym("cell"), 4, song_proxy_getcell_with_header(x, s1, f2, f3));
    } else if(s == gensym("setcell")) {
        song_proxy_setcell(x, s, argc, argv);
    } else if(s == gensym("resizepattern")) {
        p = song_proxy_resizepattern(x, s1, f2);
        if(p) {
            song_proxy_sendgui(x, gensym("patternlength"), 2, song_proxy_getpatternlength(x, p->x_name));
        }
    } else {
        error("track: editcmd: unknown command: %s", s->s_name);
    }
}

static void song_proxy_sendgui(t_song_proxy* x, t_symbol* s, int argc, t_atom* argv) {
    static const unsigned int bufsz = 8*MAXPDSTRING;
    char buf[bufsz];
    list_snconvf(buf, bufsz, s, argc, argv);
    debugprint("pd::composer::dispatch %s %s", x->rcv->s_name, buf);
    sys_vgui("pd::composer::dispatch %s %s\n", x->rcv->s_name, buf);
}

static void song_proxy_setrow(t_song_proxy* x, t_symbol* sel, int argc, t_atom* argv) {
    if(argc < 2 || argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT) {
        error("song: setrow: usage: setrow <pattern_name> <row#> <atom0> <atom1> ...");
        return;
    }
    if((argc - 2) != x->x_song->x_mastertrack->x_ncolumns) {
        post("argc=%d, ncolumns=%d", argc, x->x_song->x_mastertrack->x_ncolumns);
        error("track: setrow: input error: must provide exactly %d elements for the row", x->x_song->x_mastertrack->x_ncolumns);
        return;
    }
    t_symbol* pat_name = argv[0].a_w.w_symbol;
    t_int rownum = (t_int)argv[1].a_w.w_float;
    ArrayListGetByName(x->x_song->x_mastertrack->x_patterns, pat_name, t_pattern*, pat);
    if(!pat) {
        error("track: setrow: no such pattern: '%s'", pat_name->s_name);
        return;
    }
    pattern_setrow(pat, rownum, &argv[2]);
}

static t_atom* song_proxy_getrow(t_song_proxy* x, t_symbol* pat_name, t_floatarg rownum) {
    ArrayListGetByName(x->x_song->x_mastertrack->x_patterns, pat_name, t_pattern*, pat);
    if(!pat) {
        error("song: getrow: no such pattern: '%s'", pat_name->s_name);
        return NULL;
    }
    t_atom* row = pattern_getrow(pat, (t_int)rownum);
    debugprint("song_proxy_getrow returning " PTR, row);
    return row;
}

static t_atom* song_proxy_getrow_with_header(t_song_proxy* x, t_symbol* pat_name, t_floatarg rownum) {
    if(response_row) {
        freebytes(response_row, response_row_sz);
        response_row = NULL;
        response_row_sz = 0;
    }

    t_atom* row = song_proxy_getrow(x, pat_name, rownum);
    if(!row) {
        error("song: getrow: nu such patern: '%s'", pat_name->s_name);
        return NULL;
    }
    response_row_sz = sizeof(t_atom) * (x->x_song->x_mastertrack->x_ncolumns + 2);
    t_atom* response_row = (t_atom*)getbytes(response_row_sz);
    SETSYMBOL(&response_row[0], pat_name);
    SETFLOAT(&response_row[1], rownum);
    memcpy(&response_row[2], row, sizeof(t_atom) * x->x_song->x_mastertrack->x_ncolumns);
    return &response_row[0];
}

static void song_proxy_getrow_o(t_song_proxy* x, t_symbol* pat_name, t_floatarg rownum) {
    t_atom* row = song_proxy_getrow(x, pat_name, rownum);
    if(row)
        outlet_list(x->outlet, &s_list, x->x_song->x_mastertrack->x_ncolumns, row);
}

static void song_proxy_setcell(t_song_proxy* x, t_symbol* sel, int argc, t_atom* argv) {
    debugprint("song_proxy_setcell, s=%s", sel->s_name);
    if(argc < 4 || argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT || argv[2].a_type != A_FLOAT) {
        error("song: setcell: usage: setcell <pattern_name> <row#> <col#> <atom>");
        return;
    }
    t_symbol* pat_name = argv[0].a_w.w_symbol;
    t_int rownum = (t_int)argv[1].a_w.w_float;
    t_int colnum = (t_int)argv[2].a_w.w_float;
    ArrayListGetByName(x->x_song->x_mastertrack->x_patterns, pat_name, t_pattern*, pat);
    if(!pat) {
        error("song: setcell: no such pattern: '%s'", pat_name->s_name);
        return;
    }
    pattern_setcell(pat, (t_int)rownum, (t_int)colnum, &argv[3]);
}

static t_atom* song_proxy_getcell(t_song_proxy* x, t_symbol* pat_name, t_floatarg rownum, t_floatarg colnum) {
    ArrayListGetByName(x->x_song->x_mastertrack->x_patterns, pat_name, t_pattern*, pat);
    if(!pat) {
        error("song: getcell: no such pattern: '%s'", pat_name->s_name);
        return NULL;
    }
    return pattern_getcell(pat, (t_int)rownum, (t_int)colnum);
}

static t_atom* song_proxy_getcell_with_header(t_song_proxy* x, t_symbol* pat_name, t_floatarg rownum, t_floatarg colnum) {
    t_atom* cell = song_proxy_getcell(x, pat_name, rownum, colnum);
    if(!cell) {
        error("song: getcell: nu such patern: '%s'", pat_name->s_name);
        return NULL;
    }
    SETSYMBOL(&response_cell[0], pat_name);
    SETFLOAT(&response_cell[1], rownum);
    SETFLOAT(&response_cell[2], colnum);
    memcpy(&response_cell[3], cell, sizeof(t_atom));
    return &response_cell[0];
}

static void song_proxy_getcell_o(t_song_proxy* x, t_symbol* pat_name, t_floatarg rownum, t_floatarg colnum) {
    t_atom* cell = song_proxy_getcell(x, pat_name, rownum, colnum);
    if(cell)
        outlet_list(x->outlet, &s_list, 1, cell);
}

static t_pattern* song_proxy_resizepattern(t_song_proxy* x, t_symbol* name, t_floatarg rows) {
    ArrayListGetByName(x->x_song->x_mastertrack->x_patterns, name, t_pattern*, pat);
    if(!pat) {
        error("song: resizepattern: no such pattern: '%s'", name->s_name);
        return NULL;
    }
    pattern_resize(pat, (t_int)rows);
    return pat;
}

static t_atom* song_proxy_get_track_names(t_song_proxy* x) {
    if(response_row) {
        freebytes(response_row, response_row_sz);
        response_row = NULL;
        response_row_sz = 0;
    }
    response_row_sz = sizeof(t_atom) * x->x_song->x_tracks_count;
    response_row = (t_atom*)getbytes(response_row_sz);
    int i;
    for(i = 0; i < x->x_song->x_tracks_count; i++) {
        SETSYMBOL(&response_row[i], x->x_song->x_tracks[i]->x_name);
    }
    return &response_row[0];
}
