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

#include "tkwidgets.h"
#include <stdio.h>
#include <string.h>

/* this should be part of the Pd API */
t_symbol *canvas_getname(t_canvas *canvas)
{
    char buf[MAXPDSTRING];
    snprintf(buf, MAXPDSTRING, ".x%lx", (unsigned long)glist_getcanvas(canvas));
    return gensym(buf);
}


void tkwidgets_query_options(t_symbol *receive_name, t_symbol *widget_id, 
                             int argc, char** argv)
{
    int i;
    for(i = 0; i < argc; i++)
        sys_vgui("pd [concat %s query_callback %s [%s cget -%s] \\;]\n",
                 receive_name->s_name, argv[i], widget_id->s_name, argv[i]);
}

void tkwidgets_restore_options(t_symbol *receive_name, t_symbol *widget_id, 
                             int argc, char** argv)
{
    int i;
    for(i = 0; i < argc; i++)
    {
        // TODO parse out -flags and values, and set them here:
        sys_vgui("%s configure %s\n",
                 widget_id->s_name, argv[i]);
    }
}

void tkwidgets_set_ids(t_object *x, t_tkwidgets *tkw, t_canvas *canvas)
{

    tkw->canvas = canvas;
}


t_symbol* tkwidgets_gen_tcl_namespace(t_object* x, t_symbol* widget_name)
{
    char buf[MAXPDSTRING];
    sprintf(buf,"%s%lx", widget_name->s_name, (long unsigned int)x);
    return gensym(buf);
}

t_symbol* tkwidgets_gen_callback_name(t_symbol* tcl_namespace)
{
    char buf[MAXPDSTRING];
    sprintf(buf,"#%s", tcl_namespace->s_name);
    return gensym(buf);
}

t_symbol* tkwidgets_gen_canvas_id(t_canvas* canvas)
{
    char buf[MAXPDSTRING];
    sprintf(buf,".x%lx.c", (long unsigned int) canvas);
    return gensym(buf);
}

t_symbol* tkwidgets_gen_frame_id(t_object* x, t_symbol* canvas_id)
{
    char buf[MAXPDSTRING];
    sprintf(buf,"%s.frame%lx", canvas_id->s_name, (long unsigned int)x);
    return gensym(buf);    
}

t_symbol* tkwidgets_gen_widget_id(t_object* x, t_symbol* parent_id)
{
    char buf[MAXPDSTRING];
    sprintf(buf,"%s.widget%lx", parent_id->s_name, (long unsigned int)x);
    return gensym(buf);
}

t_symbol* tkwidgets_gen_window_id(t_object* x, t_symbol* canvas_id)
{
    char buf[MAXPDSTRING];
    sprintf(buf,"%s.window%lx", canvas_id->s_name, (long unsigned int)x);
    return gensym(buf);
}

t_symbol* tkwidgets_gen_handle_id(t_object *x, t_symbol* canvas_id)
{
    char buf[MAXPDSTRING];
    sprintf(buf,"%s.handle%lx", canvas_id->s_name, (long unsigned int)x);
    return gensym(buf);
}

t_symbol* tkwidgets_gen_scrollbar_id(t_object *x, t_symbol* frame_id)
{
    char buf[MAXPDSTRING];
    sprintf(buf,"%s.scrollbar%lx", frame_id->s_name, (long unsigned int)x);
    return gensym(buf);
}

t_symbol* tkwidgets_gen_all_tag(t_object *x)
{
    char buf[MAXPDSTRING];
    sprintf(buf,"all%lx", (long unsigned int)x);
    return gensym(buf);
}

void tkwidgets_draw_inlets(t_object *x, t_glist *glist, 
                 int total_inlets, int total_outlets)
{
    // TODO perhaps I should try to use glist_drawiofor() from g_text.c
}

void tkwidgets_draw_handle()
{
    // TODO draw resize handle when selected in editmode
}

void tkwidgets_draw_resize_window()
{
    // TODO draw the resize window while resizing
}


/*
void query_options()
{
    
}



*/
