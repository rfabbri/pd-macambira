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
    debugprint("song_new - object lookup {{%s}} => " PTR, song_name->s_name, obj);
    if(obj) return obj;

    t_song* x = (t_song*)getbytes(sizeof(t_song));
    x->x_name = song_name;
    ArrayListInit(x->x_tracks, struct _track*, 16); 

    debugprint("created a song object (" PTR "), "
        "creation args: {{%s}}",
        x, x->x_name->s_name);

    ArrayListAdd(songs, t_song*, x);
    debugprint("registered song object to global song collection");
    return x;
}

static void song_free(t_song* x) {
    // free tracks memory
    ArrayListFree(x->x_tracks, t_track*);
    // remove song from global song collection
    ArrayListRemove(songs, x);
}

static t_song* song_get(t_symbol* song_name) {
    ArrayListGetByName(songs, song_name, t_song*, result);
    return result;
}

static int song_exists(t_symbol* song_name) {
    return song_get(song_name) != 0L;
}
