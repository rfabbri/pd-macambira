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

/*
I think I'll probably have to follow Krzsztof and make structs to make this work
tkwidgets_setcallbackname(void *x, char *widget_name)
{
    char buf[MAXPDSTRING];

    sprintf(buf,"%s%lx", widget_name, (long unsigned int)x);
    x->tcl_namespace = getbytes(strlen(buf));
    strcpy(x->tcl_namespace, buf);    

    sprintf(buf,"#%s", x->tcl_namespace);
    x->receive_name = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->receive_name);
}
*/  

void draw_inlets(t_object *x, t_glist *glist, int firsttime, 
                 int total_inlets, int total_outlets)
{
    // TODO perhaps I should try to use glist_drawiofor() from g_text.c
}



void draw_handle()
{
    // TODO draw resize handle when selected in editmode
}


void draw_resize_window()
{
    // TODO draw the resize window while resizing
}
