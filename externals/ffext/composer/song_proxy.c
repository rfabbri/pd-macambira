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
    debugprint("created an instance of t_song_proxy: " PTR, x);
    return x;
}

static void song_proxy_free(t_song_proxy* x) {
}

static void song_proxy_float(t_song_proxy* x, t_floatarg f) {
}

static void song_proxy_properties(t_gobj* z, t_glist* owner) {
}

static void song_proxy_save(t_gobj* z, t_binbuf* b) {
}
