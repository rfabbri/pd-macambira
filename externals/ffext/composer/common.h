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

#ifndef COMPOSER_COMMON_H_INCLUDED
#define COMPOSER_COMMON_H_INCLUDED

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"
#include "s_stuff.h"
#include "t_tk.h"
#include <unistd.h>
#include <stdio.h>
#include "arraylist.h"

#define PTR "0x%x"
#ifdef DEBUG
#define debugprint(args...)  post( args )
#define DEBUG_BOOL 1
#else
#define debugprint(args...)
#define DEBUG_BOOL 0
#endif

#define STRINGIFY(x) #x

#define WRAP(v, w) (((v) < 0 ? (1+(int)((-(v))/(w)))*(w) : (v)) % w)

#define TRACK_SELECTOR "#TRACK"
#define SONG_SELECTOR "#SONG"

extern t_symbol s_list;

struct _track;
struct _pattern;

typedef struct _song
{
    t_symbol* x_name;
    ArrayListDeclare(x_tracks, struct _track*, t_int);
} t_song;

typedef struct _song_proxy
{
    t_object x_obj;
    t_outlet* outlet;
    t_song* x_song;
    t_int b_editor_open;
} t_song_proxy;

typedef struct _track
{
    t_symbol* x_name;
    t_song* x_song;
    t_int x_ncolumns;
    t_outlet* outlet;
    ArrayListDeclare(x_patterns, struct _pattern*, t_int);
    t_float x_currentpat;
} t_track;

typedef struct _track_proxy
{
    t_object x_obj;
    t_outlet* outlet;
    t_track* x_track;
    t_int b_editor_open;
    t_symbol* rcv;
} t_track_proxy;

typedef struct _pattern
{
    t_symbol* x_name;
    t_track* x_track;
    ArrayListDeclare(x_rows, t_atom*, t_int);
} t_pattern;

static t_song* song_new(t_symbol* song_name);
static void song_free(t_song* x);
static t_song* song_get(t_symbol* song_name);
static int song_exists(t_symbol* song_name);

static t_track* track_new(t_symbol* song_name, t_symbol* track_name, t_int columns);
static void track_free(t_track* x);
static t_track* track_get(t_symbol* song_name, t_symbol* track_name);
static int track_exists(t_symbol* song_name, t_symbol* track_name);

static t_pattern* pattern_new(t_track* track, t_symbol* name, t_int rows);
static t_pattern* pattern_clone(t_pattern* src, t_symbol* newname);
static void pattern_free(t_pattern* x);
static void pattern_rename(t_pattern* x, t_symbol* newname);
static void pattern_resize(t_pattern *x, t_int newsize);
static void pattern_new_empty_row(t_pattern* x);
static t_atom* pattern_getrow(t_pattern* x, t_int row);
static t_atom* pattern_clone_row(t_pattern* x, t_atom* row);
static t_atom* pattern_getcell(t_pattern* x, t_int row, t_int col);
static void pattern_setrow(t_pattern* x, t_int row, t_atom* rowdata);
static void pattern_setcell(t_pattern* x, t_int row, t_int col, t_atom* a);
static t_pattern* pattern_get(t_symbol* song_name, t_symbol* track_name, t_symbol* pattern_name);
static int pattern_exists(t_symbol* song_name, t_symbol* track_name, t_symbol* pattern_name);

void song_proxy_setup(void);
static t_song_proxy* song_proxy_new(t_symbol* song_name);
static void song_proxy_free(t_song_proxy* x);
static void song_proxy_float(t_song_proxy* x, t_floatarg f);
static void song_proxy_properties(t_gobj* z, t_glist* owner);
static void song_proxy_save(t_gobj* z, t_binbuf* b);

void track_proxy_setup(void);
static t_track_proxy* track_proxy_new(t_symbol* song_name, t_symbol* track_name, t_floatarg cols);
static void track_proxy_free(t_track_proxy* x);
static void track_proxy_reload(t_track_proxy* x);
static void track_proxy_properties(t_gobj* z, t_glist* owner);
static void track_proxy_properties_close(t_gobj* z, t_glist* owner);
static void track_proxy_save(t_gobj* z, t_binbuf* b);
static void track_proxy_sendrow(t_track_proxy* x, t_pattern* pat, t_int row);
static void track_proxy_anything(t_track_proxy* x, t_symbol* s, int argc, t_atom* argv);
static void track_proxy_loaddata(t_track_proxy* x, t_symbol* s, int argc, t_atom* argv);
static t_atom* track_proxy_getpatternlength(t_track_proxy* x, t_symbol* pat_name);
static void track_proxy_editcmd(t_track_proxy* x, t_symbol* s, int argc, t_atom* argv);
static void track_proxy_sendgui(t_track_proxy* x, t_symbol* s, int argc, t_atom* argv);
static void track_proxy_float(t_track_proxy* x, t_floatarg f);
static void track_proxy_setrow(t_track_proxy* x, t_symbol* sel, int argc, t_atom* argv);
static t_atom* track_proxy_getrow(t_track_proxy* x, t_symbol* pat_name, t_floatarg rownum);
static t_atom* track_proxy_getrow_with_header(t_track_proxy* x, t_symbol* pat_name, t_floatarg rownum);
static void track_proxy_getrow_o(t_track_proxy* x, t_symbol* pat_name, t_floatarg rownum);
static void track_proxy_setcell(t_track_proxy* x, t_symbol* sel, int argc, t_atom* argv);
static t_atom* track_proxy_getcell(t_track_proxy* x, t_symbol* pat_name, t_floatarg rownum, t_floatarg colnum);
static t_atom* track_proxy_getcell_with_header(t_track_proxy* x, t_symbol* pat_name, t_floatarg rownum, t_floatarg colnum);
static void track_proxy_getcell_o(t_track_proxy* x, t_symbol* pat_name, t_floatarg rownum, t_floatarg colnum);
static t_pattern* track_proxy_addpattern(t_track_proxy* x, t_symbol* name, t_floatarg rows);
static int track_proxy_removepattern(t_track_proxy* x, t_symbol* name);
static t_pattern* track_proxy_resizepattern(t_track_proxy* x, t_symbol* name, t_floatarg rows);
static t_pattern* track_proxy_renamepattern(t_track_proxy* x, t_symbol* name, t_symbol* newname);
static t_pattern* track_proxy_copypattern(t_track_proxy* x, t_symbol* src, t_symbol* dst);

ArrayListDeclareWithPrefix(extern, songs, t_song*, int);

void composer_setup(void);

#endif // COMPOSER_COMMON_H_INCLUDED
