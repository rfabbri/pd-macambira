/* 
    Copyright (C) 2007 Free Software Foundation
    written by Hans-Christoph Steiner <hans@at.or.at>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    This is the shared library for the tkwidgets library for Pd.

*/

#ifndef __TKWIDGETS_H
#define __TKWIDGETS_H

#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* size and offset for the resizing handle */
#define TKW_HANDLE_HEIGHT       15
#define TKW_HANDLE_WIDTH        15
#define TKW_HANDLE_INSET        -2

/* sketch for a common struct */
typedef struct _tkwidgets
{
    t_symbol *canvas_id; /* the canvas that is showing this widget */
    t_symbol *receive_name; /* name to bind to, to receive callbacks */
    t_symbol *window_id; /* the window that contains the widget */
    t_symbol *widget_id; /* the core widget */
    t_symbol *all_tag;   /* the tag for moving/deleting everything */
    int      resizing;   /* flag to tell when being resized */
    int      selected;   /* flag for when widget is selected */
} t_tkwidgets;   



/* query a tk widget for the state of all its options */
void query_options(t_symbol *receive_name, char *widget_id, int argc, char** argv);


/* this should be part of the Pd API */
t_symbol *canvas_getname(t_canvas *canvas);
void tkwidgets_setcallbackname(void *x, char *widget_name);

// TODO perhaps I should try to use glist_drawiofor() from g_text.c
void draw_inlets(t_object *x, t_glist *glist, int firsttime, 
                 int total_inlets, int total_outlets);
void draw_handle(); // TODO draw resize handle when selected in editmode
void draw_resize_window(); // TODO draw the resize window while resizing







#endif /* NOT g_TK_WIDGETS_H */
