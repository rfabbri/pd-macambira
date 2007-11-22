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

/* -------------------- options handling ------------------------------------ */

void tkwidgets_query_options(t_symbol *receive_name, t_symbol *widget_id, 
                             int argc, char** argv)
{
    int i;
    for(i = 0; i < argc; i++)
        sys_vgui("pd [concat %s query_callback %s [%s cget -%s] \\;]\n",
                 receive_name->s_name, argv[i], widget_id->s_name, argv[i]);
}

/* this queries the widget for each option listed in the tk_options struct,
 * builts a list in Tcl-space, then send that list to the store_callback */
void tkwidgets_store_options(t_symbol *receive_name, t_symbol *tcl_namespace,
                             t_symbol *widget_id, int argc, char **argv)
{
    int i;
    for(i = 0; i < argc; i++)
    {
        sys_vgui("set ::%s::ret [%s cget -%s]\n",
                 tcl_namespace->s_name, widget_id->s_name, argv[i]);
        sys_vgui("if {[string length $::%s::ret] > 0} {\n",
                 tcl_namespace->s_name);
        sys_vgui("lappend ::%s::list -%s; lappend ::%s::list $::%s::ret}\n", 
                 tcl_namespace->s_name, argv[i], 
                 tcl_namespace->s_name, tcl_namespace->s_name);
    }
    sys_vgui("pd [concat %s store_callback $::%s::list \\;]\n",
             receive_name->s_name, tcl_namespace->s_name);
    sys_vgui("unset ::%s::list \n", tcl_namespace->s_name);  
}

void tkwidgets_restore_options(t_symbol *receive_name, t_symbol *tcl_namespace,
                               t_symbol *widget_id, t_binbuf *options_binbuf)
{
    int length;
    char *options;
    binbuf_gettext(options_binbuf, &options, &length);
    options[length] = 0; //binbuf_gettext() doesn't put a null, so we do
    sys_vgui("%s configure %s\n", widget_id->s_name, options);
}

/* -------------------- generate names for various elements ----------------- */

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

t_symbol* tkwidgets_gen_window_tag(t_object* x, t_symbol* canvas_id)
{
    char buf[MAXPDSTRING];
    sprintf(buf,"%s.window%lx", canvas_id->s_name, (long unsigned int)x);
    return gensym(buf);
}

t_symbol* tkwidgets_gen_iolets_tag(t_object* x)
{
    char buf[MAXPDSTRING];
    sprintf(buf,"iolets%lx", (long unsigned int)x);
    return gensym(buf);
}

t_symbol* tkwidgets_gen_all_tag(t_object *x)
{
    char buf[MAXPDSTRING];
    sprintf(buf,"all%lx", (long unsigned int)x);
    return gensym(buf);
}

/* -------------------- inlets/outlets -------------------------------------- */
 
static int calculate_onset(int x_location, int width,
                           int current_iolet, int total_iolets)
{
    post("calculate_onset");
    return(x_location + (width - IOWIDTH)                               \
           * current_iolet / (total_iolets == 1 ? 1 : total_iolets - 1));
}

void tkwidgets_draw_iolets(t_object *x, t_glist *glist, t_symbol *canvas_id,
                           t_symbol *iolets_tag, t_symbol *all_tag,
                           int width, int height,
                           int total_inlets, int total_outlets)
{
    int i, onset;
    int x_location = text_xpix(x, glist);
    int y_location = text_ypix(x, glist);

    for (i = 0; i < total_inlets; i++)  /* inlets */
    {
        onset = calculate_onset(x_location, width, i, total_inlets);
        sys_vgui("%s create rectangle %d %d %d %d -tags {%s %s}\n",
                 canvas_id->s_name, onset, y_location - 2,
                 onset + IOWIDTH, y_location,
                 iolets_tag->s_name, all_tag->s_name);
        sys_vgui("%s raise %s\n", canvas_id->s_name, iolets_tag->s_name);
    }
    for (i = 0; i < total_outlets; i++) /* outlets */
    {
        onset = calculate_onset(x_location, width, i, total_outlets);
        sys_vgui("%s create rectangle %d %d %d %d -tags {%s %s}\n",
                 canvas_id->s_name, onset, y_location + height,
                 onset + IOWIDTH, y_location + height + 2,
                 iolets_tag->s_name, all_tag->s_name);
        sys_vgui("%s raise %s\n", canvas_id->s_name, iolets_tag->s_name);
    }
}

void tkwidgets_erase_iolets(t_symbol* canvas_id, t_symbol* iolets_tag)
{
    sys_vgui("%s delete %s\n", canvas_id->s_name, iolets_tag); 
}

/* -------------------- gui elements for resizing --------------------------- */

void tkwidgets_draw_handle()
{
    // TODO draw resize handle when selected in editmode
}

void tkwidgets_draw_resize_window()
{
    // TODO draw the resize window while resizing
}
