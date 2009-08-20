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

static t_song* song_new(t_symbol* song_name) {
    ArrayListGetByName(songs, song_name, t_song*, obj);
    debugprint("song_new - object lookup %s => " PTR, song_name->s_name, obj);
    if(obj) return obj;

    t_song* x = (t_song*)getbytes(sizeof(t_song));
    x->x_name = song_name;
    ArrayListInit(x->x_tracks, struct _track*, 16);
    // mastertrack is named like the song
    x->x_mastertrack = mastertrack_new(x, x->x_name, 0);

    debugprint("created a song object (" PTR "), "
        "creation args: %s",
        x, x->x_name->s_name);

    ArrayListAdd(songs, t_song*, x);
    debugprint("registered song object to global song collection");
    return x;
}

static void song_mastertrack_fix_cols(t_song* x) {
    debugprint("song_mastertrack_fix_cols(" PTR "), new track count: %d", x, x->x_tracks_count);
    debugprint("song='%s' mastertrack=" PTR, x->x_name->s_name, x->x_mastertrack);
    debugprint("we have %d patterns, sir", x->x_mastertrack->x_patterns_count);
    if(x->x_mastertrack->x_patterns_count < x->x_mastertrack->x_ncolumns) {
        debugprint("song_mastertrack_fix_cols: still loading (apparently?), skipping.");
        return;
    }
    song_internal_resize_cols(x, x->x_tracks_count);
}

static void song_free(t_song* x) {
    // free tracks memory
    ArrayListFree(x->x_tracks, t_track*);
    // remove song from global song collection
    ArrayListRemove(songs, x);
}

static void song_internal_resize_cols(t_song* x, t_int sz) {
    int j;
    for(j = 0; j < x->x_mastertrack->x_patterns_count; j++) {
        if(j > 0) post("WARNING: mastertrack with more than one pattern!");
        pattern_resize_cols(x->x_mastertrack->x_patterns[j], sz);
    }
}

static t_song* song_get(t_symbol* song_name) {
    ArrayListGetByName(songs, song_name, t_song*, result);
    return result;
}

static int song_exists(t_symbol* song_name) {
    return song_get(song_name) != 0L;
}

static void song_loaddata(t_song* x, int argc, t_atom* argv) {
    if(argc < 2) {
        error("song_loaddata: format error 1");
        return;
    }
    if(argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT) {
        error("song_loaddata: format error 2");
    }
    t_symbol* name = argv[0].a_w.w_symbol;
    t_int ntracks = (t_int)argv[1].a_w.w_float;
    debugprint("song_loaddata: song='%s', tracks=%d", name->s_name, ntracks);
    song_internal_resize_cols(x, ntracks);
}

static void song_binbuf_save(t_song* t, t_symbol* selector, t_binbuf* b) {
    // data format:
    // SELECTOR DATA <song_name> <ntracks>

    binbuf_addv(b, "sssi", selector, gensym("SONGINFO"), t->x_name, t->x_tracks_count);
    binbuf_addv(b, ";");
}
