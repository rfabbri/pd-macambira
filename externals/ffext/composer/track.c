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

static t_track* track_new(t_symbol* song_name, t_symbol* track_name, t_int columns) {
    debugprint("track_new(%s, %s, %d)", song_name->s_name, track_name->s_name, columns);
    t_song* song = song_new(song_name);
    t_track* t = song_create_track(song, track_name, columns);

    // add track to song's track list
    ArrayListAdd(song->x_tracks, t_track*, t);
    song_mastertrack_fix_cols(song);

    return t;
}

static t_track* mastertrack_new(t_song* song, t_symbol* track_name, t_int columns) {
    debugprint("mastertrack_new(%s, %s, %d)", song->x_name->s_name, track_name->s_name, columns);
    t_track* t = song_create_track(song, track_name, columns);
    debugprint("mastertrack_new add pattern");
    ArrayListAdd(t->x_patterns, t_pattern*, pattern_new(t, gensym("Arrangement"), 16));
    return t;
}

// both a song method and the track constructor:
static t_track* song_create_track(t_song* song, t_symbol* track_name, t_int columns) {
    ArrayListGetByName(song->x_tracks, track_name, t_track*, obj);
    debugprint("song_create_track - object lookup %s, %s, %d => " PTR,
            song->x_name->s_name, track_name->s_name, columns, obj);
    if(obj) return obj;

    t_track* x = (t_track*)getbytes(sizeof(t_track));
    x->x_name = track_name;
    x->x_song = song;
    x->x_ncolumns = columns;
    ArrayListInit(x->x_patterns, struct _pattern*, 4);
    x->x_currentpat = 0;

    debugprint("created a track object (" PTR "), "
        "creation args: %s, %s, %d",
        x, x->x_song->x_name->s_name, x->x_name->s_name, x->x_ncolumns);

    debugprint("song_create_track returns " PTR, x);
    return x;
}

static void track_free(t_track* x) {
    // free patterns memory
    ArrayListFree(x->x_patterns, t_pattern*);
    // remove track from song's track list
    ArrayListRemove(x->x_song->x_tracks, x);
}

static t_int track_get_pattern_count(t_track* x) {
    return x->x_patterns_count;
}

static void track_get_pattern_names(t_track* x, t_atom* /* OUT */ out) {
    int i;
    for(i = 0; i < x->x_patterns_count; i++) {
        SETSYMBOL(&out[i], x->x_patterns[i]->x_name);
    }
}

static t_track* track_get(t_symbol* song_name, t_symbol* track_name) {
    t_song* song = song_get(song_name);
    if(!song || !song->x_tracks) return (t_track*) 0L;
    ArrayListGetByName(song->x_tracks, track_name, t_track*, result);
    return result;
}

static int track_exists(t_symbol* song_name, t_symbol* track_name) {
    return track_get(song_name, track_name) != 0L;
}

static void track_loaddata(t_track* x, int argc, t_atom* argv) {
    debugprint("track_loaddata(" PTR ", %d, " PTR ")", x, argc, argv);
    int i,base;
    base = 0;

    if(argc < (base+2) || argv[base].a_type != A_SYMBOL || argv[base+1].a_type != A_SYMBOL) {
        error("track: data format error 1");
        return;
    }
    t_symbol* song_name = argv[base+0].a_w.w_symbol;
    t_symbol* track_name = argv[base+1].a_w.w_symbol;
    base += 2;

    if(x->x_song->x_name != song_name) {
        debugprint("WARNING: discarding data from another song: %s", song_name->s_name);
        return;
    }

    if(x->x_name != track_name) {
        debugprint("WARNING: discarding data from another track: %s", track_name->s_name);
        return;
    }

    t_song* song = song_get(song_name);
    if(!song) {
        error("track: song '%s' does not exist", song_name->s_name);
        return;
    }

    debugprint("track_loaddata: song='%s', track='%s'", song_name->s_name, track_name->s_name);

    if(argc < (base+1) || argv[base].a_type != A_FLOAT) {
        error("track: data format error 2");
        return;
    }
    t_int npatterns = (t_int)argv[base].a_w.w_float;
    base += 1;

    debugprint("track_loaddata: %d patterns to read", npatterns);

    t_symbol* patname;
    t_int patrows;
    t_pattern* pat;

    debugprint("track_loaddata(" PTR ", %d, " PTR ")", x, argc, argv);
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
        debugprint("pattern %d: %s-%s-%s, length=%d, RxC=%d", i,
                song_name->s_name, track_name->s_name, patname->s_name,
                patrows, patrows * x->x_ncolumns);
        base += 2;
        if(argc >= (base + patrows * x->x_ncolumns) && patrows > 0) {
            pat = pattern_new(x, patname, patrows);
            debugprint("created new pattern " PTR " ('%s', %d rows) for track " PTR, pat, patname->s_name, patrows, x);
            int j,h,k;
            for(h = 0, j = base; j < (base + patrows * x->x_ncolumns); j += x->x_ncolumns, h++) {
                debugprint("  working on row %d", h);
                for(k = 0; k < x->x_ncolumns; k++) {
                    pattern_setcell(pat, h, k, &argv[j+k]);
                }
            }
            base += patrows * x->x_ncolumns;
        } else {
            error("track: data format error 8 (i=%d)", i);
            return;
        }
    }
}

static void track_binbuf_save(t_track* t, t_symbol* selector, t_binbuf* b) {
    // data format:
    // SELECTOR DATA <song_name> <track_name> <npatterns> [<pat_name> <pat rows> RxC_atoms]*n

    binbuf_addv(b, "ssssi", selector, gensym("DATA"),
            t->x_song->x_name, t->x_name, t->x_patterns_count);

    int i,j,k;
    for(i = 0; i < t->x_patterns_count; i++) {
        t_pattern* pat = t->x_patterns[i];
        binbuf_addv(b, "si", pat->x_name, pat->x_rows_count);
        for(j = 0; j < pat->x_rows_count; j++) {
            for(k = 0; k < t->x_ncolumns; k++) {
                switch(pat->x_rows[j][k].a_type) {
                case A_FLOAT: binbuf_addv(b, "f", pat->x_rows[j][k].a_w.w_float); break;
                case A_SYMBOL: binbuf_addv(b, "s", pat->x_rows[j][k].a_w.w_symbol); break;
                case A_NULL: binbuf_addv(b, "s", gensym("empty")); break;
                default: binbuf_addv(b, "s", gensym("unknown")); break;
                }
            }
            //binbuf_add(b, t->x_ncolumns, &pat->x_rows[j]);
        }
    }

    binbuf_addv(b, ";");
}
