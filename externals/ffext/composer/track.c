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
    t_song* song = song_new(song_name);

    ArrayListGetByName(song->x_tracks, track_name, t_track*, obj);
    debugprint("track_new - object lookup {{%s} {%s} {%d}} => " PTR, song_name->s_name, track_name->s_name, columns, obj);
    if(obj) return obj;

    t_track* x = (t_track*)getbytes(sizeof(t_track));
    x->x_name = track_name;
    x->x_song = song;
    x->x_ncolumns = columns;
    ArrayListInit(x->x_patterns, struct _pattern*, 4);
    x->x_currentpat = 0;

    debugprint("created a track object (" PTR "), "
        "creation args: {{%s} {%s} {%d}}",
        x, x->x_song->x_name->s_name, x->x_name->s_name, x->x_ncolumns);

    // att track to song's track list
    ArrayListAdd(song->x_tracks, t_track*, x);
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
