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

static t_pattern* pattern_new(t_track* track, t_symbol* name, t_int rows) {
    ArrayListGetByName(track->x_patterns, name, t_pattern*, obj);
    debugprint("pattern_new - object lookup %s => " PTR, name->s_name, obj);
    if(obj) return obj;

    t_pattern* x = (t_pattern*)getbytes(sizeof(t_pattern));
    x->x_name = name;
    x->x_track = track;
    ArrayListInit(x->x_rows, t_atom*, rows);

    int i;
    for(i = 0; i < rows; i++) {
        //debugprint("x->x_rows[%d] = " PTR, i, x->x_rows[i]);
        pattern_new_empty_row(x);
        //debugprint("x->x_rows[%d] <- " PTR, i, x->x_rows[i]);
    }

    debugprint("created new pattern " PTR " with %d rows (x_rows = " PTR ")", x, x->x_rows_count, x->x_rows);

    // add pattern to track's pattern list
    ArrayListAdd(x->x_track->x_patterns, t_pattern*, x);
    return x;
}

static t_pattern* pattern_clone(t_pattern* src, t_symbol* newname) {
    t_pattern* x = (t_pattern*)getbytes(sizeof(t_pattern));
    x->x_name = newname;
    x->x_track = src->x_track;
    ArrayListInit(x->x_rows, t_atom*, src->x_rows_count);

    int i;
    for(i = 0; i < src->x_rows_count; i++) {
        ArrayListAdd(x->x_rows, t_atom*, pattern_clone_row(x, src->x_rows[i]));
    }

    ArrayListAdd(x->x_track->x_patterns, t_pattern*, x);
    return x;
}

static void pattern_free(t_pattern* x) {
    // free rows memory
    ArrayListFree(x->x_rows, t_atom*);
    // remove pattern from track's pattern list
    ArrayListRemove(x->x_track->x_patterns, x);
}

static void pattern_rename(t_pattern* x, t_symbol* newname) {
    x->x_name = newname;
}

static void pattern_resize(t_pattern *x, t_int newsize) {
    debugprint("pattern_resize(" PTR ", %d)", x, newsize);
    debugprint("initial size: %d", x->x_rows_count);
    while(x->x_rows_count < newsize)
        pattern_new_empty_row(x);
    while(x->x_rows_count > newsize)
        ArrayListRemoveByIndex(x->x_rows, x->x_rows_count - 1);
    debugprint("final size: %d", x->x_rows_count);
}

/* WARNING: do not call this for track with more than 1 pattern!
 *          Works only for the mastertrack (song_proxy)
 */
static void pattern_resize_cols(t_pattern* x, t_int newcols) {
    int j;
    for(j = 0; j < x->x_rows_count; j++) {
        if(&x->x_rows[j])
            x->x_rows[j] = (t_atom*)resizebytes(x->x_rows[j], x->x_track->x_ncolumns, newcols);
        else
            x->x_rows[j] = (t_atom*)getbytes(sizeof(t_atom) * newcols);
    }
    x->x_track->x_ncolumns = newcols;
}

static void pattern_new_empty_row(t_pattern* x) {
    t_atom* rowdata = (t_atom*)getbytes(sizeof(t_atom) * x->x_track->x_ncolumns);
    int j;
    for(j = 0; j < x->x_track->x_ncolumns; j++)
        SETSYMBOL(&(rowdata[j]), gensym("empty"));
    ArrayListAdd(x->x_rows, t_atom*, rowdata);
}

static t_atom* pattern_getrow(t_pattern* x, t_int row) {
    debugprint("pattern_getrow(" PTR ", %d)", x, row);
    row = WRAP(row, x->x_rows_count);
    t_atom* rowdata = x->x_rows[row];
    return rowdata;
}

static t_atom* pattern_clone_row(t_pattern* x, t_atom* rowdata) {
    debugprint("pattern_clone_row(" PTR ", " PTR ")", x, rowdata);
    t_atom* clone = (t_atom*)copybytes(rowdata, sizeof(t_atom) * x->x_track->x_ncolumns);
    return clone;
}

static t_atom* pattern_getcell(t_pattern* x, t_int row, t_int col) {
    row = WRAP(row, x->x_rows_count);
    col = WRAP(col, x->x_track->x_ncolumns);
    return &(x->x_rows[row][col]);
}

static void pattern_setrow(t_pattern* x, t_int row, t_atom* rowdata) {
    debugprint("pattern_setrow(" PTR ", %d, " PTR ")", x, row, rowdata);
    row = WRAP(row, x->x_rows_count);
    //debugprint("x->x_rows[%d] = " PTR, row, x->x_rows[row]);
    t_atom *myrowdata = x->x_rows[row];
    memcpy(myrowdata, rowdata, sizeof(t_atom) * x->x_track->x_ncolumns);
    //debugprint("x->x_rows[%d] <- " PTR, row, x->x_rows[row]);
}

static void pattern_setcell(t_pattern* x, t_int row, t_int col, t_atom* a) {
    row = WRAP(row, x->x_rows_count);
    col = WRAP(col, x->x_track->x_ncolumns);
    //debugprint("about to write an atom (size=%d) at address " PTR, sizeof(t_atom), &(x->x_rows[row][col]));
    memcpy(&(x->x_rows[row][col]), a, sizeof(t_atom));
}

static t_pattern* pattern_get(t_symbol* song_name, t_symbol* track_name, t_symbol* pattern_name) {
    t_track* track = track_get(song_name, track_name);
    if(!track || !track->x_patterns) return (t_pattern*) 0L;
    ArrayListGetByName(track->x_patterns, pattern_name, t_pattern*, result);
    return result;
}

static int pattern_exists(t_symbol* song_name, t_symbol* track_name, t_symbol* pattern_name) {
    return pattern_get(song_name, track_name, pattern_name) != 0L;
}
