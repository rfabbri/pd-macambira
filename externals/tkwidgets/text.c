/* text widget for Pd

  Copyright 2003 Guenter Geiger
  Copyright 2004 Ben Bogart <ben@ekran.org>
  Copyright 2007 Hans-Christoph Steiner <hans@at.or.at>

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
*/

#include <stdio.h>
#include <string.h>
#include "shared/tkwidgets.h"

/* TODO: get Ctrl-A working to select all */
/* TODO: set message doesnt work with a loadbang */
/* TODO: add size to query and save */
/* TODO: add scrollbars to query and save */
/* TODO: remove glist from _erase() args */
/* TODO: window name "handle1376fc00" already exists in parent */
/* TODO: figure out window vs. text width/height */


#define DEFAULT_COLOR           "grey70"

#define TEXT_DEFAULT_WIDTH     130
#define TEXT_DEFAULT_HEIGHT    60
#define TEXT_MIN_WIDTH         40
#define TEXT_MIN_HEIGHT        20

#define TOTAL_INLETS            1
#define TOTAL_OUTLETS           2

#define DEBUG(x) x

static t_class *textwidget_class;
static t_widgetbehavior textwidget_widgetbehavior;

typedef struct _textwidget
{
    t_object    x_obj;
    t_canvas*   x_canvas;      /* canvas this widget is currently drawn in */
    t_glist*    x_glist;       /* glist that owns this widget */
    t_binbuf*   options_binbuf;/* binbuf to save options state in */

    int         width;
    int         height;
    int         have_scrollbars;

    int         x_resizing;
    int         x_selected;
    
    /* IDs for Tk widgets */
	t_symbol*   tcl_namespace;       
    t_symbol*   receive_name;  /* name to bind to to receive callbacks */
	t_symbol*   canvas_id;  
    t_symbol*   frame_id;       
	t_symbol*   widget_id;        
    t_symbol*   scrollbar_id;   
	t_symbol*   handle_id;      
	t_symbol*   window_tag;      
	t_symbol*   iolets_tag;
	t_symbol*   all_tag;
    
    t_outlet*   x_data_outlet;
    t_outlet*   x_status_outlet;
} t_textwidget;

static char *textwidget_tk_options[] = {
    "autoseparators",
    "background",
    "borderwidth",
    "cursor",
    "exportselection",
    "font",
    "foreground",
    "height",
    "highlightbackground",
    "highlightcolor",
    "highlightthickness",
    "insertbackground",
    "insertborderwidth",
    "insertofftime",
    "insertontime",
    "insertwidth",
    "maxundo",
    "padx",
    "pady",
    "relief",
    "selectbackground",
    "selectborderwidth",
    "selectforeground",
    "setgrid",
    "spacing1",
    "spacing2",
    "spacing3",
    "state",
    "tabs",
    "takefocus",
    "undo",
    "width",
    "wrap",
    "xscrollcommand",
    "yscrollcommand"
};


/* common symbols to preload */
static t_symbol *scrollbars_symbol;
static t_symbol *size_symbol;
static t_symbol *backspace_symbol;
static t_symbol *return_symbol;
static t_symbol *space_symbol;
static t_symbol *tab_symbol;
static t_symbol *escape_symbol;
static t_symbol *left_symbol;
static t_symbol *right_symbol;
static t_symbol *up_symbol;
static t_symbol *down_symbol;

/* -------------------- function prototypes --------------------------------- */

static void textwidget_query_callback(t_textwidget *x, t_symbol *s, int argc, t_atom *argv);


/* -------------------- widget helper functions ----------------------------- */

static void set_tkwidgets_ids(t_textwidget *x, t_canvas *canvas)
{
    x->x_canvas = canvas;
    x->canvas_id = tkwidgets_gen_canvas_id(x->x_canvas);
    x->frame_id = tkwidgets_gen_frame_id((t_object*)x, x->canvas_id);
    x->widget_id = tkwidgets_gen_widget_id((t_object*)x, x->frame_id);
    x->scrollbar_id = tkwidgets_gen_scrollbar_id((t_object*)x, x->frame_id);
    x->window_tag = tkwidgets_gen_window_tag((t_object*)x, x->frame_id);
    x->handle_id = tkwidgets_gen_handle_id((t_object *)x, x->canvas_id);
}

static void draw_scrollbar(t_textwidget *x)
{
    sys_vgui("pack %s -side right -fill y -before %s \n",
             x->scrollbar_id->s_name, x->widget_id->s_name);
    x->have_scrollbars = 1;
}

static void erase_scrollbar(t_textwidget *x)
{
    sys_vgui("pack forget %s \n", x->scrollbar_id->s_name);
    x->have_scrollbars = 0;
}

static void bind_standard_keys(t_textwidget *x)
{
#ifdef __APPLE__
    sys_vgui("bind %s <Mod1-Key> {pdtk_canvas_ctrlkey %s %%K 0}\n",
             x->widget_id->s_name, x->canvas_id->s_name);
    sys_vgui("bind %s <Mod1-Shift-Key> {pdtk_canvas_ctrlkey %s %%K 1}\n",
             x->widget_id->s_name, x->canvas_id->s_name);
#else
    sys_vgui("bind %s <Control-Key> {pdtk_canvas_ctrlkey %s %%K 0}\n",
             x->widget_id->s_name, x->canvas_id->s_name);
    sys_vgui("bind %s <Control-Shift-Key> {pdtk_canvas_ctrlkey %s %%K 1}\n",
             x->widget_id->s_name, x->canvas_id->s_name);
#endif
}

static void bind_button_events(t_textwidget *x)
{
    /* mouse buttons */
    sys_vgui("bind %s <Button> {pdtk_canvas_sendclick %s \
[expr %%X - [winfo rootx %s]] [expr %%Y - [winfo rooty %s]] %%b 0}\n",
             x->widget_id->s_name, x->canvas_id->s_name, 
             x->canvas_id->s_name, x->canvas_id->s_name);
    sys_vgui("bind %s <ButtonRelease> {pdtk_canvas_mouseup %s \
[expr %%X - [winfo rootx %s]] [expr %%Y - [winfo rooty %s]] %%b}\n",
             x->widget_id->s_name, x->canvas_id->s_name, 
             x->canvas_id->s_name, x->canvas_id->s_name);
    sys_vgui("bind %s <Shift-Button> {pdtk_canvas_click %s \
[expr %%X - [winfo rootx %s]] [expr %%Y - [winfo rooty %s]] %%b 1}\n",
             x->widget_id->s_name, x->canvas_id->s_name, 
             x->canvas_id->s_name, x->canvas_id->s_name);
    sys_vgui("bind %s <Button-2> {pdtk_canvas_rightclick %s \
[expr %%X - [winfo rootx %s]] [expr %%Y - [winfo rooty %s]] %%b}\n",
             x->widget_id->s_name, x->canvas_id->s_name, 
             x->canvas_id->s_name, x->canvas_id->s_name);
    sys_vgui("bind %s <Button-3> {pdtk_canvas_rightclick %s \
[expr %%X - [winfo rootx %s]] [expr %%Y - [winfo rooty %s]] %%b}\n",
             x->widget_id->s_name, x->canvas_id->s_name, 
             x->canvas_id->s_name, x->canvas_id->s_name);
    sys_vgui("bind %s <Control-Button> {pdtk_canvas_rightclick %s \
[expr %%X - [winfo rootx %s]] [expr %%Y - [winfo rooty %s]] %%b}\n",
             x->widget_id->s_name, x->canvas_id->s_name, 
             x->canvas_id->s_name, x->canvas_id->s_name);
    /* mouse motion */
    sys_vgui("bind %s <Motion> {pdtk_canvas_motion %s \
[expr %%X - [winfo rootx %s]] [expr %%Y - [winfo rooty %s]] 0}\n",
             x->widget_id->s_name, x->canvas_id->s_name, 
             x->canvas_id->s_name, x->canvas_id->s_name);
}

static void create_widget(t_textwidget *x)
{
    DEBUG(post("create_widget"););

    sys_vgui("namespace eval text%lx {} \n", x);
    
    /* Seems we have to delete the widget in case it already exists (Provided by Guenter)*/
    sys_vgui("destroy %s\n", x->frame_id->s_name);
    sys_vgui("frame %s \n", x->frame_id->s_name);
    sys_vgui("text %s -border 1 \
    -highlightthickness 1 -relief sunken -bg \"%s\" -yscrollcommand {%s set} \n",
             x->widget_id->s_name, DEFAULT_COLOR, x->scrollbar_id->s_name);
    sys_vgui("scrollbar %s -command {%s yview}\n",
             x->scrollbar_id->s_name, x->widget_id->s_name);
    sys_vgui("pack %s -side left -fill both -expand 1 \n", x->widget_id->s_name);
    sys_vgui("pack %s -side bottom -fill both -expand 1 \n", x->frame_id->s_name);

    bind_standard_keys(x);
    bind_button_events(x);
    sys_vgui("bind %s <KeyRelease> {+pd %s keyup %%N \\;} \n", 
             x->widget_id->s_name, x->receive_name->s_name);
}

static void textwidget_drawme(t_textwidget *x, t_glist *glist)
{
    DEBUG(post("textwidget_drawme: firsttime %d canvas %lx glist %lx", x->x_canvas, glist););
    set_tkwidgets_ids(x,glist_getcanvas(glist));	
    create_widget(x);	
    tkwidgets_draw_inlets((t_object*)x, glist, 
                          x->canvas_id, x->iolets_tag, x->all_tag,
                          x->width, x->height, TOTAL_INLETS, TOTAL_OUTLETS);
    if(x->have_scrollbars) draw_scrollbar(x);
    sys_vgui("%s create window %d %d -anchor nw -window %s    \
                  -tags {%s %s} -width %d -height %d \n", x->canvas_id->s_name,
             text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
             x->frame_id->s_name, x->window_tag->s_name, x->all_tag->s_name, 
             x->width, x->height);
}     

static void textwidget_erase(t_textwidget* x,t_glist* glist)
{
    DEBUG(post("textwidget_erase: canvas %lx glist %lx", x->x_canvas, glist););

    set_tkwidgets_ids(x,glist_getcanvas(glist));
    tkwidgets_erase_inlets(x->canvas_id, x->iolets_tag);
    sys_vgui("destroy %s\n", x->frame_id->s_name);
    sys_vgui("%s delete %s\n", x->canvas_id->s_name, x->all_tag->s_name);
}
	


/* ------------------------ text widgetbehaviour----------------------------- */

static void textwidget_getrect(t_gobj *z, t_glist *owner, 
                          int *xp1, int *yp1, int *xp2, int *yp2)
{
//    DEBUG(post("textwidget_getrect");); /* this one is very chatty :D */
    t_textwidget *x = (t_textwidget*)z;
    *xp1 = text_xpix(&x->x_obj, owner);
    *yp1 = text_ypix(&x->x_obj, owner);
    *xp2 = *xp1 + x->width;
    *yp2 = *yp1 + x->height + 2; // add 2 to give space for outlets
}

static void textwidget_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_textwidget *x = (t_textwidget *)z;
    DEBUG(post("textwidget_displace: canvas %lx glist %lx", x->x_canvas, glist););
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if (glist_isvisible(glist))
    {
        set_tkwidgets_ids(x,glist_getcanvas(glist));
        sys_vgui("%s move %s %d %d\n", x->canvas_id->s_name, x->all_tag->s_name, dx, dy);
        sys_vgui("%s move RESIZE %d %d\n", x->canvas_id->s_name, dx, dy);
        canvas_fixlinesfor(glist_getcanvas(glist), (t_text*) x);
    }
    DEBUG(post("displace end"););
}

static void textwidget_select(t_gobj *z, t_glist *glist, int state)
{
    t_textwidget *x = (t_textwidget *)z;
    DEBUG(post("textwidget_select: canvas %lx glist %lx state %d", x->x_canvas, glist, state););
    
    if( (state) && (!x->x_selected))
    {
        sys_vgui("%s configure -bg #bdbddd -state disabled -cursor $cursor_editmode_nothing\n",
                 x->widget_id->s_name);
        x->x_selected = 1;
    }
    else if (!state)
    {
        sys_vgui("%s configure -bg grey -state normal -cursor xterm\n",
                 x->widget_id->s_name);
        /* activatefn never gets called with 0, so destroy here */
        sys_vgui("destroy %s\n", x->handle_id->s_name);
        x->x_selected = 0;
    }
}

static void textwidget_activate(t_gobj *z, t_glist *glist, int state)
{
    DEBUG(post("textwidget_activate"););    
    t_textwidget *x = (t_textwidget *)z;
 	int x1, y1, x2, y2;

    if(state)
    {
        textwidget_getrect(z, glist, &x1, &y1, &x2, &y2);
        sys_vgui("canvas %s -width %d -height %d -bg #ddd -bd 0 \
-highlightthickness 3 -highlightcolor {#f00} -cursor bottom_right_corner\n",
                 x->handle_id->s_name, TKW_HANDLE_WIDTH, TKW_HANDLE_HEIGHT);
        int handle_x1 = x2 - TKW_HANDLE_WIDTH;
        int handle_y1 = y2 - (TKW_HANDLE_HEIGHT - TKW_HANDLE_INSET);
//        int handle_x2 = x2;
//        int handle_y2 = y2 - TKW_HANDLE_INSET;
/* no worky, this should draw MAC OS X style lines on the resize handle */
/*         sys_vgui("%s create line %d %d %d %d -fill black -tags RESIZE_LINES\n",  */
/*                  x->handle_id->s_name, handle_x2, handle_y1, handle_x1, handle_y2); */
        sys_vgui("%s create window %d %d -anchor nw -width %d -height %d -window %s -tags RESIZE\n",
                 x->canvas_id->s_name, handle_x1, handle_y1,
                 TKW_HANDLE_WIDTH, TKW_HANDLE_HEIGHT,
                 x->handle_id->s_name, x->all_tag->s_name);
        sys_vgui("raise %s\n", x->handle_id->s_name);
        sys_vgui("bind %s <Button> {pd [concat %s resize_click 1 \\;]}\n",
                 x->handle_id->s_name, x->receive_name->s_name);
        sys_vgui("bind %s <ButtonRelease> {pd [concat %s resize_click 0 \\;]}\n",
                 x->handle_id->s_name, x->receive_name->s_name);
        sys_vgui("bind %s <Motion> {pd [concat %s resize_motion %%x %%y \\;]}\n",
                 x->handle_id->s_name, x->receive_name->s_name);
    }
}

static void textwidget_delete(t_gobj *z, t_glist *glist)
{
    DEBUG(post("textwidget_delete: glist %lx", glist););    
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist_getcanvas(glist), x);
}

       
static void textwidget_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_textwidget *x = (t_textwidget*)z;
    DEBUG(post("textwidget_vis: vis %d canvas %lx glist %lx", vis, x->x_canvas, glist););
    t_rtext *y;
    if (vis) {
        y = (t_rtext *) rtext_new(glist, (t_text *)z);
        textwidget_drawme(x, glist);
    }
    else {
        y = glist_findrtext(glist, (t_text *)z);
        textwidget_erase(x, glist);
        rtext_free(y);
    }
}

/*  the clickfn is only called in run mode
static int textwidget_click(t_gobj *z, t_glist *glist, int xpix, int ypix, 
                       int shift, int alt, int dbl, int doit)
{
    t_textwidget *x = (t_textwidget *)z;
    DEBUG(post("textwidget_click x:%d y:%d edit: %d", xpix, ypix, x->x_canvas->gl_edit););    
    return 0;
}
*/

static void textwidget_append(t_textwidget* x,  t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("textwidget_append"););
    int i;
    t_symbol *tmp_symbol = s; /* <-- this gets rid of the unused variable warning */
    t_float tmp_float;

    for(i=0; i<argc ; i++)
    {
        tmp_symbol = atom_getsymbolarg(i, argc, argv);
        if(tmp_symbol == &s_)
        {
            tmp_float = atom_getfloatarg(i, argc , argv);
            sys_vgui("lappend ::%s::list %g \n", x->tcl_namespace->s_name, tmp_float );
        }
        else 
        {
            sys_vgui("lappend ::%s::list %s \n", x->tcl_namespace->s_name, tmp_symbol->s_name );
        }
    }
    sys_vgui("append ::%s::list \" \"\n", x->tcl_namespace->s_name);
    sys_vgui("%s insert end $::%s::list ; unset ::%s::list \n", 
               x->widget_id->s_name, x->tcl_namespace->s_name, x->tcl_namespace->s_name );
    sys_vgui("%s yview end-2char \n", x->widget_id->s_name );
}

static void textwidget_key(t_textwidget* x,  t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("textwidget_key"););
    t_symbol *tmp_symbol = s; /* <-- this gets rid of the unused variable warning */
    t_int tmp_int;

    tmp_symbol = atom_getsymbolarg(0, argc, argv);
    if(tmp_symbol == &s_)
    {
        tmp_int = (t_int) atom_getfloatarg(0, argc , argv);
        if(tmp_int < 10)
        {
            sys_vgui("%s insert end %d\n", x->widget_id->s_name, tmp_int);
        }
        else if(tmp_int == 10)
        {
            sys_vgui("%s insert end {\n}\n", x->widget_id->s_name);
        }
        else
        {
            sys_vgui("%s insert end [format \"%c\" %d]\n", x->widget_id->s_name, tmp_int);
        }
    }
    else 
    {
        sys_vgui("%s insert end %s\n", x->widget_id->s_name, tmp_symbol->s_name );
    }
    sys_vgui("%s yview end-2char \n", x->widget_id->s_name );
}

/* Clear the contents of the text widget */
static void textwidget_clear(t_textwidget* x)
{
    sys_vgui("%s delete 0.0 end \n", x->widget_id->s_name);
}

/* Function to reset the contents of the textwidget box */
static void textwidget_set(t_textwidget* x,  t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("textwidget_set"););

    textwidget_clear(x);
    textwidget_append(x, s, argc, argv);
}

/* Pass the contents of the text widget onto the textwidget_output_callback fuction above */
static void textwidget_bang_output(t_textwidget* x)
{
    /* With "," and ";" escaping thanks to JMZ */
    sys_vgui("pd [concat %s output [string map {\",\" \"\\\\,\" \";\" \"\\\\;\"} \
              [%s get 0.0 end]] \\;]\n", 
             x->receive_name->s_name, x->widget_id->s_name);
}

static void textwidget_output_callback(t_textwidget* x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list(x->x_data_outlet, s, argc, argv );
}

static void textwidget_keyup_callback(t_textwidget *x, t_float f)
{
/*     DEBUG(post("textwidget_keyup_callback");); */
    int keycode = (int) f;
    char buf[10];
    t_symbol *output_symbol;

    if( (keycode > 32 ) && (keycode < 65288) )
    {
        snprintf(buf, 2, "%c", keycode);
        output_symbol = gensym(buf);
    } else
        switch(keycode)
        {
        case 32: /* space */
            output_symbol = space_symbol;
            break;
        case 65293: /* return */
            output_symbol = return_symbol;
            break;
        case 65288: /* backspace */
            output_symbol = backspace_symbol;
            break;
        case 65289: /* tab */
            output_symbol = tab_symbol;
            break;
        case 65307: /* escape */
            output_symbol = escape_symbol;
            break;
        case 65361: /* left */
            output_symbol = left_symbol;
            break;
        case 65363: /* right */
            output_symbol = right_symbol;
            break;
        case 65362: /* up */
            output_symbol = up_symbol;
            break;
        case 65364: /* down */
            output_symbol = down_symbol;
            break;
        default:
            snprintf(buf, 10, "key_%d", keycode);
            DEBUG(post("keyup: %d", keycode););
            output_symbol = gensym(buf);
        }
    outlet_symbol(x->x_status_outlet, output_symbol);
}

static void textwidget_save(t_gobj *z, t_binbuf *b)
{
    t_textwidget *x = (t_textwidget *)z;
    
    binbuf_addv(b, "ssiisiii", &s__X, gensym("obj"),
                x->x_obj.te_xpix, x->x_obj.te_ypix, 
                atom_getsymbol(binbuf_getvec(x->x_obj.te_binbuf)),
                x->width, x->height, x->have_scrollbars);
    binbuf_addbinbuf(b, x->options_binbuf);
    binbuf_addv(b, ";");
}

static void textwidget_option(t_textwidget *x, t_symbol *s, int argc, t_atom *argv)
{
    if(s != &s_list)
    {
        t_binbuf *argument_binbuf = binbuf_new();
        char *argument_buffer;
        int buffer_length;
        
        binbuf_add(argument_binbuf, argc, argv);
        binbuf_gettext(argument_binbuf, &argument_buffer, &buffer_length);
        binbuf_free(argument_binbuf);
        argument_buffer[buffer_length] = 0;
        post("argument_buffer: %s", argument_buffer);
        sys_vgui("%s configure -%s {%s} \n", 
                 x->widget_id->s_name, s->s_name, argument_buffer);
        tkwidgets_store_options(x->receive_name, x->tcl_namespace, x->widget_id, 
                                sizeof(textwidget_tk_options)/sizeof(char *), 
                                &textwidget_tk_options);
    }
}

static void query_scrollbars(t_textwidget *x)
{
    t_atom state[2];
    SETSYMBOL(state, scrollbars_symbol);
    SETFLOAT(state + 1, (t_float)x->have_scrollbars);
    textwidget_query_callback(x, gensym("query_callback"), 2, state);
}

static void query_size(t_textwidget *x)
{
    t_atom coords[3];
    SETSYMBOL(coords, size_symbol);
    SETFLOAT(coords + 1, (t_float)x->width);
    SETFLOAT(coords + 2, (t_float)x->height);
    textwidget_query_callback(x, gensym("query_callback"), 3, coords);
}

static void textwidget_query(t_textwidget *x, t_symbol *s)
{
    post("textwidget_query %s", s->s_name);
    if(s == &s_)
    {
        tkwidgets_query_options(x->receive_name, x->widget_id, 
                                sizeof(textwidget_tk_options)/sizeof(char *), 
                                textwidget_tk_options);
        query_scrollbars(x);
        query_size(x);
    }
    else if(s == scrollbars_symbol)
        query_scrollbars(x);
    else if(s == size_symbol)
        query_size(x);
    else
        tkwidgets_query_options(x->receive_name, x->widget_id, 1, &(s->s_name));
}

static void textwidget_scrollbars(t_textwidget *x, t_float f)
{
    int value = (int) f;
    if(value > 0)
        draw_scrollbar(x);
    else
        erase_scrollbar(x);
}

static void textwidget_size(t_textwidget *x, t_float width, t_float height)
{
    DEBUG(post("textwidget_size"););
    x->height = height;
    x->width = width;
    if(glist_isvisible(x->x_glist))
    {
        sys_vgui("%s itemconfigure %s -width %d -height %d\n",
                 x->canvas_id->s_name, x->window_tag->s_name, x->width, x->height);
        tkwidgets_erase_inlets(x->canvas_id, x->iolets_tag);
        tkwidgets_draw_inlets((t_object*)x, x->x_glist, 
                              x->canvas_id, x->iolets_tag, x->all_tag,
                              x->width, x->height, TOTAL_INLETS, TOTAL_OUTLETS);
        canvas_fixlinesfor(x->x_glist, (t_text *)x);  // 2nd inlet
    }
}

/* -------------------- callback functions ---------------------------------- */

static void textwidget_store_callback(t_textwidget *x, t_symbol *s, int argc, t_atom *argv)
{
    if(s != &s_)
        binbuf_restore(x->options_binbuf, argc, argv);
}

static void textwidget_query_callback(t_textwidget *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *tmp_symbol = atom_getsymbolarg(0, argc, argv);
    if(tmp_symbol != &s_)
    {
        post("tmp_symbol %s argc %d", tmp_symbol->s_name, argc);
        outlet_anything(x->x_status_outlet, tmp_symbol, argc - 1, argv + 1);
    }
    else
    {
        post("ERROR: textwidget_query_callback %s %d", s->s_name, argc);
    }
}

static void textwidget_click_callback(t_textwidget *x, t_floatarg f)
{
    if( (x->x_glist->gl_edit) && (x->x_glist == x->x_canvas) )
    {	
        textwidget_select((t_gobj *)x, x->x_glist, f);
    }
}

static void textwidget_resize_click_callback(t_textwidget *x, t_floatarg f)
{
    t_canvas *canvas = (glist_isvisible(x->x_glist) ? x->x_canvas : 0);
    int newstate = (int)f;
    if (x->x_resizing && newstate == 0)
    {
        if (canvas)
        {
            tkwidgets_draw_inlets((t_object*)x, canvas,
                                  x->canvas_id, x->iolets_tag, x->all_tag,
                                  x->width, x->height, TOTAL_INLETS, TOTAL_OUTLETS);
            canvas_fixlinesfor(x->x_glist, (t_text *)x);  // 2nd inlet
        }
    }
    else if (!x->x_resizing && newstate)
    {
        tkwidgets_erase_inlets(x->canvas_id, x->iolets_tag);
    }
    x->x_resizing = newstate;
}

static void textwidget_resize_motion_callback(t_textwidget *x, t_floatarg f1, t_floatarg f2)
{
    DEBUG(post("textwidget_resize_motion_callback"););
    if (x->x_resizing)
    {
        int dx = (int)f1, dy = (int)f2;
        if (glist_isvisible(x->x_glist))
        {
            x->width += dx;
            x->height += dy;
            sys_vgui("%s itemconfigure %s -width %d -height %d\n",
                     x->canvas_id->s_name, x->window_tag->s_name, 
                     x->width, x->height);
            sys_vgui("%s move RESIZE %d %d\n",
                     x->canvas_id->s_name, dx, dy);
            canvas_fixlinesfor(x->x_glist, (t_text *)x);
        }
    }
}

/* --------------------------- standard class functions --------------------- */

static void textwidget_free(t_textwidget *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->receive_name);
}

static void *textwidget_new(t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("textwidget_new"););
    t_textwidget *x = (t_textwidget *)pd_new(textwidget_class);
    
    x->options_binbuf = binbuf_new();

    x->x_selected = 0;
    x->x_resizing = 0;
	
	if (argc < 3)
	{
		post("[text]: less than 3 arguments entered, default values used.");
		x->width = TEXT_DEFAULT_WIDTH;
		x->height = TEXT_DEFAULT_HEIGHT;
        x->have_scrollbars = 0;
	} 
    else 
    {
		x->width = atom_getint(argv);
		x->height = atom_getint(argv + 1);
        x->have_scrollbars = atom_getint(argv + 2);
        if(argc > 3) 
        {
            binbuf_add(x->options_binbuf, argc - 3, argv + 3);
//TODO            tkwidgets_restore_options(x->receive_name, x->widget_id);
        }
	}	

    x->tcl_namespace = tkwidgets_gen_tcl_namespace((t_object*)x, s);
    x->receive_name = tkwidgets_gen_callback_name(x->tcl_namespace);
    pd_bind(&x->x_obj.ob_pd, x->receive_name);

    x->x_glist = canvas_getcurrent();
    set_tkwidgets_ids(x, x->x_glist);
    x->iolets_tag = tkwidgets_gen_iolets_tag((t_object*)x);
    x->all_tag = tkwidgets_gen_all_tag((t_object*)x);

    x->x_data_outlet = outlet_new(&x->x_obj, &s_float);
    x->x_status_outlet = outlet_new(&x->x_obj, &s_symbol);

    return (x);
}

void text_setup(void) {
    textwidget_class = class_new(gensym("text"), (t_newmethod)textwidget_new, 
                                 (t_method)textwidget_free,sizeof(t_textwidget),
                                 0, A_GIMME, 0);
				
	class_addbang(textwidget_class, (t_method)textwidget_bang_output);
	class_addanything(textwidget_class, (t_method)textwidget_option);

	class_addmethod(textwidget_class, (t_method)textwidget_append,
                    gensym("append"), A_GIMME, 0);
	class_addmethod(textwidget_class, (t_method)textwidget_clear,
                    gensym("clear"), 0);
	class_addmethod(textwidget_class, (t_method)textwidget_key,
                    gensym("key"), A_GIMME, 0);
    class_addmethod(textwidget_class, (t_method)textwidget_query,
                    gensym("query"), A_DEFSYMBOL, 0);
    class_addmethod(textwidget_class, (t_method)textwidget_scrollbars,
                    gensym("scrollbars"), A_DEFFLOAT, 0);
	class_addmethod(textwidget_class, (t_method)textwidget_set,
                    gensym("set"), A_GIMME, 0);
    class_addmethod(textwidget_class, (t_method)textwidget_size,
                    gensym("size"), A_DEFFLOAT, A_DEFFLOAT, 0);
/* callbacks */
    class_addmethod(textwidget_class, (t_method)textwidget_store_callback,
                    gensym("store_callback"), A_GIMME, 0);
    class_addmethod(textwidget_class, (t_method)textwidget_query_callback,
                    gensym("query_callback"), A_GIMME, 0);
	class_addmethod(textwidget_class, (t_method)textwidget_output_callback,
                    gensym("output"), A_GIMME, 0);
    class_addmethod(textwidget_class, (t_method)textwidget_keyup_callback,
                    gensym("keyup"), A_DEFFLOAT, 0);
    class_addmethod(textwidget_class, (t_method)textwidget_click_callback,
                    gensym("click"), A_FLOAT, 0);
    class_addmethod(textwidget_class, (t_method)textwidget_resize_click_callback,
                    gensym("resize_click"), A_FLOAT, 0);
    class_addmethod(textwidget_class, (t_method)textwidget_resize_motion_callback,
                    gensym("resize_motion"), A_FLOAT, A_FLOAT, 0);

    textwidget_widgetbehavior.w_getrectfn  = textwidget_getrect;
    textwidget_widgetbehavior.w_displacefn = textwidget_displace;
    textwidget_widgetbehavior.w_selectfn   = textwidget_select;
    textwidget_widgetbehavior.w_activatefn = textwidget_activate;
    textwidget_widgetbehavior.w_deletefn   = textwidget_delete;
    textwidget_widgetbehavior.w_visfn      = textwidget_vis;
    textwidget_widgetbehavior.w_clickfn    = NULL;
    class_setwidget(textwidget_class,&textwidget_widgetbehavior);
    class_setsavefn(textwidget_class,&textwidget_save);

/* commonly used symbols */
    size_symbol = gensym("size");
    scrollbars_symbol = gensym("scrollbars");
    backspace_symbol = gensym("backspace");
    return_symbol = gensym("return");
	space_symbol = gensym("space");
	tab_symbol = gensym("tab");
	escape_symbol = gensym("escape");
	left_symbol = gensym("left");
	right_symbol = gensym("right");
	up_symbol = gensym("up");
	down_symbol = gensym("down");
}


