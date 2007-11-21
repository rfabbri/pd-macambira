/* [checkbutton] object for dislaying a check box

   Copyright (C) 2002-2004 Guenter Geiger
   Copyright (C) 2007 Hans-Christoph Steiner <hans@at.or.at>

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

   This is part of the tkwidgets library for Pd.

*/

#include "shared/tkwidgets.h"

/* ------------------------ class variables --------------------------------- */

static t_class *checkbutton_class;
static t_widgetbehavior checkbutton_widgetbehavior;

typedef struct _checkbutton
{
    t_object     x_obj;
    t_glist*     x_glist;
    int          x_width;
    int          x_height;
    
    /* IDs for Tk widgets */
	t_symbol*   tcl_namespace;       
    t_symbol*   receive_name;  /* name to bind to to receive callbacks */
	t_symbol*   canvas_id;  
	t_symbol*   widget_id;        
	t_symbol*   handle_id;      
	t_symbol*   all_tag;
    
    t_outlet*   x_data_outlet;
    t_outlet*   x_status_outlet;
} t_checkbutton;

static char *checkbutton_tk_options[] = {
    "activebackground",
    "activeforeground",
    "anchor",
    "background",
    "bitmap",
    "borderwidth",
    "command",
    "compound",
    "cursor",
    "disabledforeground",
    "font",
    "foreground",
    "height",
    "highlightbackground",
    "highlightcolor",
    "highlightthickness",
    "image",
    "indicatoron",
    "justify",
    "offrelief",
    "offvalue",
    "onvalue",
    "overrelief",
    "padx",
    "pady",
    "relief",
    "selectcolor",
    "selectimage",
    "state",
    "takefocus",
    "text",
    "textvariable",
    "underline",
    "variable",
    "width",
    "wraplength"
};

/* -------------------- widget helper functions------------------------------ */

static void checkbutton_drawme(t_checkbutton *x, t_glist *glist)
{
    sys_vgui("checkbutton %s\n", x->widget_id);
}


static void checkbutton_erase(t_checkbutton* x,t_glist* glist)
{
    sys_vgui("%s delete %s\n", x->canvas_id, x->widget_id);
}



/* --------------------- checkbutton widgetbehaviour ------------------------ */
static void checkbutton_getrect(t_gobj *z, t_glist *glist,
                          int *xp1, int *yp1, int *xp2, int *yp2)
{
    t_checkbutton* x = (t_checkbutton*)z;

    *xp1 = text_xpix(&x->x_obj, glist);
    *yp1 = text_ypix(&x->x_obj, glist);
    *xp2 = text_xpix(&x->x_obj, glist) + x->x_width;
    *yp2 = text_ypix(&x->x_obj, glist) + x->x_height;
}

static void checkbutton_delete(t_gobj *z, t_glist *glist)
{
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist_getcanvas(glist), x);
}
       
static void checkbutton_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_checkbutton* s = (t_checkbutton*)z;
    if (vis)
        checkbutton_drawme(s, glist);
    else
        checkbutton_erase(s, glist);
}

/* --------------------------- standard class functions --------------------- */

static void checkbutton_free(t_checkbutton *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->receive_name);
}

static void *checkbutton_new(t_symbol* s, int argc, t_atom *argv)
{
    t_checkbutton *x = (t_checkbutton *)pd_new(checkbutton_class);

    x->x_glist = (t_glist*) canvas_getcurrent();

    x->x_width = 15;
    x->x_height = 15;

    outlet_new(&x->x_obj, &s_float);
    return (x);
}

void checkbutton_setup(void)
{
    checkbutton_class = class_new(gensym("checkbutton"), (t_newmethod)checkbutton_new, 
                            (t_method)checkbutton_free, 
                            sizeof(t_checkbutton), 0, A_GIMME,0);

    checkbutton_widgetbehavior.w_getrectfn  = checkbutton_getrect;
    checkbutton_widgetbehavior.w_displacefn = NULL;
    checkbutton_widgetbehavior.w_selectfn   = NULL;
    checkbutton_widgetbehavior.w_activatefn = NULL;
    checkbutton_widgetbehavior.w_deletefn   = checkbutton_delete;
    checkbutton_widgetbehavior.w_visfn      = checkbutton_vis;
    checkbutton_widgetbehavior.w_clickfn    = NULL;
    class_setwidget(checkbutton_class, &checkbutton_widgetbehavior);
}


