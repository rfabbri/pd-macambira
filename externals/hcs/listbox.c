/* text entry widget for PD                                              *
 * Based on button from GGEE by Guenter Geiger                           *
 * Copyright 2004 Ben Bogart 2004 ben@ekran.org                          * 
 * Copyright 2007 Free Software Foundation

 * This program is distributed under the terms of the GNU General Public *
 * License                                                               *

 * entry is free software; you can redistribute it and/or modify         *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *

 * listbox is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          */

#include <m_pd.h>
#include <m_imp.h>
#include <g_canvas.h>
#include <stdio.h>
#include <string.h>

/* TODO: make "display only" option, to force box to never accept focus */
/* TODO: make focus option only accept regular and shifted chars, not Cmd, Alt, Ctrl */
/* TODO: make listbox_save include whole classname, including namespace prefix */
/* TODO: make [size( message redraw object */
/* TODO: set message doesnt work with a loadbang */
/* TODO: make message to add a single character to the existing text  */
/* TODO: complete inlet draw/erase logic */

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#ifndef IOWIDTH 
#define IOWIDTH 4
#endif

#define BACKGROUNDCOLOR "grey70"

#define DEBUG(x) x

typedef struct _listbox
{
    t_object x_obj;
    
    t_glist * x_glist;
    int x_rect_width;
    int x_rect_height;
    t_symbol*  x_receive_name;

/* TODO: these all should be settable by messages */
    int x_height;
    int x_width;

    t_symbol* x_bgcolour;
    t_symbol* x_fgcolour;
    
    t_symbol *x_font_face;
    t_int x_font_size;
    t_symbol *x_font_weight;

    t_float x_border;
    t_float x_highlightthickness;
    t_symbol *x_relief;
    t_int x_have_scrollbar;
    
    t_outlet* x_data_outlet;
    t_outlet* x_status_outlet;
} t_listbox;


static t_class *listbox_class;


static t_symbol *backspace_symbol;
static t_symbol *return_symbol;
static t_symbol *space_symbol;
static t_symbol *tab_symbol;
static t_symbol *escape_symbol;
static t_symbol *left_symbol;
static t_symbol *right_symbol;
static t_symbol *up_symbol;
static t_symbol *down_symbol;

/* function prototypes */

static void listbox_getrect(t_gobj *z, t_glist *owner, int *xp1, int *yp1, int *xp2, int *yp2);
static void listbox_displace(t_gobj *z, t_glist *glist, int dx, int dy);
static void listbox_select(t_gobj *z, t_glist *glist, int state);
static void listbox_activate(t_gobj *z, t_glist *glist, int state);
static void listbox_delete(t_gobj *z, t_glist *glist);
static void listbox_vis(t_gobj *z, t_glist *glist, int vis);
static void listbox_save(t_gobj *z, t_binbuf *b);


t_widgetbehavior   listbox_widgetbehavior = {
w_getrectfn:  listbox_getrect,
w_displacefn: listbox_displace,
w_selectfn:   listbox_select,
w_activatefn: listbox_activate,
w_deletefn:   listbox_delete,
w_visfn:      listbox_vis,
w_clickfn:    NULL,
}; 

/* widget helper functions */

static int calculate_onset(t_listbox *x, t_glist *glist, int i, int nplus)
{
    return(text_xpix(&x->x_obj, glist) + (x->x_rect_width - IOWIDTH) * i / nplus);
}

static void draw_inlets(t_listbox *x, t_glist *glist, int firsttime, int nin, int nout)
{
    DEBUG(post("draw_inlets in: %d  out: %d", nin, nout););

    int nplus, i, onset;
    t_canvas *canvas = glist_getcanvas(glist);
    
    nplus = (nin == 1 ? 1 : nin-1);
    /* inlets */
    for (i = 0; i < nin; i++)
    {
        onset = calculate_onset(x,glist,i,nplus);
        if (firsttime)
        {
            DEBUG(post(".x%x.c create rectangle %d %d %d %d -tags {%xi%d %xi}\n",
                       canvas, onset, text_ypix(&x->x_obj, glist) - 2,
                       onset + IOWIDTH, text_ypix(&x->x_obj, glist) - 1,
                       x, i, x););
            sys_vgui(".x%x.c create rectangle %d %d %d %d -tags {%xi%d %xi}\n",
                     canvas, onset, text_ypix(&x->x_obj, glist) - 2,
                     onset + IOWIDTH, text_ypix(&x->x_obj, glist) - 1,
                     x, i, x);
        }
        else
        {
            DEBUG(post(".x%x.c coords %xi%d %d %d %d %d\n",
                       canvas, x, i, onset, text_ypix(&x->x_obj, glist) - 2,
                       onset + IOWIDTH, text_ypix(&x->x_obj, glist) - 1););
            sys_vgui(".x%x.c coords %xi%d %d %d %d %d\n",
                     canvas, x, i, onset, text_ypix(&x->x_obj, glist) - 2,
                     onset + IOWIDTH, text_ypix(&x->x_obj, glist)- 1);
        }
    }
    nplus = (nout == 1 ? 1 : nout-1);
    for (i = 0; i < nout; i++) /* outlets */
    {
        onset = calculate_onset(x,glist,i,nplus);
        if (firsttime)
        {
            DEBUG(post(".x%x.c create rectangle %d %d %d %d -tags {%xo%d %xo}\n",
                       canvas, onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 2,
                       onset + IOWIDTH, text_ypix(&x->x_obj, glist) + x->x_rect_height-1,
                       x, i, x););
            sys_vgui(".x%x.c create rectangle %d %d %d %d -tags {%xo%d %xo}\n",
                     canvas, onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 2,
                     onset + IOWIDTH, text_ypix(&x->x_obj, glist) + x->x_rect_height-1,
                     x, i, x);
        }
        else
        {
            DEBUG(post(".x%x.c coords %xo%d %d %d %d %d\n",
                       canvas, x, i, 
                       onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 2,
                       onset + IOWIDTH, text_ypix(&x->x_obj, glist) + x->x_rect_height-1););
            sys_vgui(".x%x.c coords %xo%d %d %d %d %d\n",
                     canvas, x, i,
                     onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 2,
                     onset + IOWIDTH, text_ypix(&x->x_obj, glist) + x->x_rect_height-1);
        }
    }
    DEBUG(post("draw inlet end"););
}

static void erase_inlets(t_listbox *x, t_canvas *canvas)
{
    DEBUG(post("erase_inlets"););
/* Added tag for all inlets of one instance */
    DEBUG(post(".x%x.c delete %xi\n", canvas,x););
    sys_vgui(".x%x.c delete %xi\n", canvas,x); 
    DEBUG(post(".x%x.c delete %xo\n", canvas,x););
    sys_vgui(".x%x.c delete %xo\n", canvas,x); 
/* Added tag for all outlets of one instance */
    DEBUG(post(".x%x.c delete  %xhandle\n", canvas,x,0););
    sys_vgui(".x%x.c delete  %xhandle\n", canvas,x,0);
}

/* currently unused
   static void draw_handle(t_listbox *x, t_glist *glist, int firsttime) {
   int onset = text_xpix(&x->x_obj, glist) + (x->x_rect_width - IOWIDTH+2);

   if (firsttime)
   sys_vgui(".x%x.c create rectangle %d %d %d %d -tags %xhandle\n",
   glist_getcanvas(glist),
   onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 12,
   onset + IOWIDTH-2, text_ypix(&x->x_obj, glist) + x->x_rect_height-4,
   x);
   else
   sys_vgui(".x%x.c coords %xhandle %d %d %d %d\n",
   glist_getcanvas(glist), x, 
   onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 12,
   onset + IOWIDTH-2, text_ypix(&x->x_obj, glist) + x->x_rect_height-4);
   }
*/
static void draw_scrollbar(t_listbox *x)
{
    DEBUG(post("pack .x%x.c.s%x.scrollbar -side right -fill y -before .x%x.c.s%x.text \n",
               x->x_glist, x, x->x_glist, x););
    sys_vgui("pack .x%x.c.s%x.scrollbar -side right -fill y -before .x%x.c.s%x.text \n",
             x->x_glist, x, x->x_glist, x);
    x->x_have_scrollbar = 1;
}

static void erase_scrollbar(t_listbox *x)
{
    DEBUG(post("pack forget .x%x.c.s%x.scrollbar \n", x->x_glist, x););
    sys_vgui("pack forget .x%x.c.s%x.scrollbar \n", x->x_glist, x);
    x->x_have_scrollbar = 0;
}

static void create_widget(t_listbox *x, t_glist *glist)
{
    DEBUG(post("create_widget"););
    t_canvas *canvas=glist_getcanvas(glist);
    /* I guess this is for fine-tuning of the rect size based on width and height? */
    x->x_rect_width = x->x_width;
    x->x_rect_height =  x->x_height+2;
	
    DEBUG(post("namespace eval listbox%lx {} \n", x););
    sys_vgui("namespace eval listbox%lx {} \n", x);

    /* Seems we have to delete the widget in case it already exists (Provided by Guenter)*/
    DEBUG(post("destroy .x%x.c.s%x\n", canvas, x););
    sys_vgui("destroy .x%x.c.s%x\n", canvas, x);


    DEBUG(post("frame .x%x.c.s%x \n",canvas, x););
    sys_vgui("frame .x%x.c.s%x \n",canvas, x);
    DEBUG(post("text .x%x.c.s%x.text -font {%s %d %s} -border 1 \
              -highlightthickness 1 -relief sunken -bg \"%s\" -fg \"%s\" \
              -yscrollcommand {.x%x.c.s%x.scrollbar set} \n",
               canvas, x, x->x_font_face->s_name, x->x_font_size, 
               x->x_font_weight->s_name,
               x->x_bgcolour->s_name,x->x_fgcolour->s_name,
               canvas, x););
    sys_vgui("text .x%x.c.s%x.text -font {%s %d %s} -border 1 \
              -highlightthickness 1 -relief sunken -bg \"%s\" -fg \"%s\" \
              -yscrollcommand {.x%x.c.s%x.scrollbar set} \n",
             canvas, x, x->x_font_face->s_name, x->x_font_size, 
             x->x_font_weight->s_name,
             x->x_bgcolour->s_name, x->x_fgcolour->s_name,
             canvas, x);
    DEBUG(post("scrollbar .x%x.c.s%x.scrollbar -command {.x%x.c.s%x.text yview} \n",canvas, x, canvas, x););
    sys_vgui("scrollbar .x%x.c.s%x.scrollbar -command {.x%x.c.s%x.text yview} \n",canvas, x ,canvas, x);
    DEBUG(post("pack .x%x.c.s%x.scrollbar -side right -fill y \n",canvas, x););
    sys_vgui("pack .x%x.c.s%x.scrollbar -side right -fill y \n",canvas, x);
    DEBUG(post("pack .x%x.c.s%x.text -side left -fill both -expand 1 \n",canvas, x););
    sys_vgui("pack .x%x.c.s%x.text -side left -fill both -expand 1 \n",canvas, x);
    DEBUG(post("pack .x%x.c.s%x -side bottom -fill both -expand 1 \n",canvas, x););
    sys_vgui("pack .x%x.c.s%x -side bottom -fill both -expand 1 \n",canvas, x);

    DEBUG(post("bind .x%x.c.s%x.text <KeyRelease> {+pd %s keyup %%N \\;} \n", 
               canvas, x, x->x_receive_name->s_name););
    sys_vgui("bind .x%x.c.s%x.text <KeyRelease> {+pd %s keyup %%N \\;} \n", 
             canvas, x, x->x_receive_name->s_name);
    DEBUG(post("bind .x%x.c.s%x.text <Leave> {focus [winfo parent .x%x.c.s%x]} \n", 
               canvas, x, canvas, x);); 
    sys_vgui("bind .x%x.c.s%x.text <Leave> {focus [winfo parent .x%x.c.s%x]} \n", 
             canvas, x, canvas, x); 
}

static void listbox_drawme(t_listbox *x, t_glist *glist, int firsttime)
{
    DEBUG(post("listbox_drawme"););
    t_canvas *canvas=glist_getcanvas(glist);
    DEBUG(post("drawme %d",firsttime););
    if (firsttime) 
    {
        x->x_glist = canvas;
        create_widget(x,glist);	       
        DEBUG(post(".x%x.c create window %d %d -anchor nw -window .x%x.c.s%x \
                    -tags %xS -width %d -height %d \n", canvas,
                   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
                   canvas, x, x, x->x_width, x->x_height););
        sys_vgui(".x%x.c create window %d %d -anchor nw -window .x%x.c.s%x \
                  -tags %xS -width %d -height %d \n", canvas, 
                 text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
                 canvas, x,x, x->x_width, x->x_height);
    }     
    else 
    {
        DEBUG(post(".x%x.c coords %xS %d %d\n", canvas, x,
                   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)););
        sys_vgui(".x%x.c coords %xS %d %d\n", canvas, x,
                 text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist));
    }
    post("canvas: %d  glist: %d", canvas->gl_edit, glist->gl_edit); 
    if( (x->x_glist->gl_edit) && (canvas == x->x_glist) )
        draw_inlets(x, glist, firsttime, 1,2);
    else
        erase_inlets(x, canvas);
    //     draw_handle(x, glist, firsttime);
}


static void listbox_erase(t_listbox* x,t_glist* glist)
{
    DEBUG(post("listbox_erase"););
    t_canvas *canvas = glist_getcanvas(glist);
    DEBUG(post("destroy .x%x.c.s%x\n", canvas, x););
    sys_vgui("destroy .x%x.c.s%x\n", canvas, x);

    DEBUG(post(".x%x.c delete %xS\n", canvas, x););
    sys_vgui(".x%x.c delete %xS\n", canvas, x);

    erase_inlets(x, canvas);
}
	


/* ------------------------ text widgetbehaviour----------------------------- */


static void listbox_getrect(t_gobj *z, t_glist *owner, 
                          int *xp1, int *yp1, int *xp2, int *yp2)
{
/*     DEBUG(post("listbox_getrect");); */
    int width, height;
    t_listbox* s = (t_listbox*)z;

    width = s->x_rect_width;
    height = s->x_rect_height;
    *xp1 = text_xpix(&s->x_obj, owner);
    *yp1 = text_ypix(&s->x_obj, owner) - 1;
    *xp2 = text_xpix(&s->x_obj, owner) + width;
    *yp2 = text_ypix(&s->x_obj, owner) + height;
}

static void listbox_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    DEBUG(post("listbox_displace"););
    t_listbox *x = (t_listbox *)z;
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if (glist_isvisible(glist))
    {
        t_canvas *canvas = glist_getcanvas(glist);
        DEBUG(post(".x%x.c coords %xSEL %d %d %d %d\n", canvas, x,
                   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)-1,
                   text_xpix(&x->x_obj, glist) + x->x_rect_width, 
                   text_ypix(&x->x_obj, glist) + x->x_rect_height-2););
        sys_vgui(".x%x.c coords %xSEL %d %d %d %d\n", canvas, x,
                 text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)-1,
                 text_xpix(&x->x_obj, glist) + x->x_rect_width, 
                 text_ypix(&x->x_obj, glist) + x->x_rect_height-2);
      
        listbox_drawme(x, glist, 0);
        canvas_fixlinesfor(canvas, (t_text*) x);
    }
    DEBUG(post("displace end"););
}

static void listbox_select(t_gobj *z, t_glist *glist, int state)
{
    DEBUG(post("listbox_select"););
    t_listbox *x = (t_listbox *)z;
    t_canvas *canvas = glist_getcanvas(glist);
    if (state) {
        DEBUG(post(".x%x.c create rectangle %d %d %d %d -tags %xSEL -outline blue\n",
                   canvas,
                   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)-1,
                   text_xpix(&x->x_obj, glist) + x->x_rect_width, 
                   text_ypix(&x->x_obj, glist) + x->x_rect_height-2, x););
        sys_vgui(".x%x.c create rectangle %d %d %d %d -tags %xSEL -outline blue\n",
                 canvas,
                 text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)-1,
                 text_xpix(&x->x_obj, glist) + x->x_rect_width, 
                 text_ypix(&x->x_obj, glist) + x->x_rect_height-2, x);
    }
    else {
        DEBUG(post(".x%x.c delete %xSEL\n", canvas, x););
        sys_vgui(".x%x.c delete %xSEL\n", canvas, x);
    }
}

static void listbox_activate(t_gobj *z, t_glist *glist, int state)
{
    DEBUG(post("listbox_activate"););    
/* this is currently unused
   t_text *x = (t_text *)z;
   t_rtext *y = glist_findrtext(glist, x);
   if (z->g_pd != gatom_class) rtext_activate(y, state);
*/
}

static void listbox_delete(t_gobj *z, t_glist *glist)
{
    DEBUG(post("listbox_delete"););    
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist_getcanvas(glist), x);
}

       
static void listbox_vis(t_gobj *z, t_glist *glist, int vis)
{
    DEBUG(post("listbox_vis"););
    t_listbox* s = (t_listbox*)z;
    t_rtext *y;
    DEBUG(post("vis: %d",vis););
    if (vis) {
        y = (t_rtext *) rtext_new(glist, (t_text *)z);
        listbox_drawme(s, glist, 1);
    }
    else {
        y = glist_findrtext(glist, (t_text *)z);
        listbox_erase(s,glist);
        rtext_free(y);
    }
}

static void listbox_add(t_listbox* x,  t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("listbox_add"););
    int i;
    t_symbol *tmp_symbol = s; /* <-- this gets rid of the unused variable warning */
    t_float tmp_float;

    for(i=0; i<argc ; i++)
    {
        tmp_symbol = atom_getsymbolarg(i, argc, argv);
        if(tmp_symbol == &s_)
        {
            tmp_float = atom_getfloatarg(i, argc , argv);
            DEBUG(post("lappend ::listbox%lx::list %g \n", x, tmp_float ););
            sys_vgui("lappend ::listbox%lx::list %g \n", x, tmp_float );
        }
        else 
        {
            DEBUG(post("lappend ::listbox%lx::list %s \n", x, tmp_symbol->s_name ););
            sys_vgui("lappend ::listbox%lx::list %s \n", x, tmp_symbol->s_name );
        }
    }
    DEBUG(post("append ::listbox%lx::list \" \"\n", x););
    sys_vgui("append ::listbox%lx::list \" \"\n", x);
    DEBUG(post(".x%x.c.s%x.text insert end $::listbox%lx::list ; unset ::listbox%lx::list \n", 
               x->x_glist, x, x, x ););
    sys_vgui(".x%x.c.s%x.text insert end $::listbox%lx::list ; unset ::listbox%lx::list \n", 
             x->x_glist, x, x, x );
    DEBUG(post(".x%x.c.s%x.text yview end-2char \n", x->x_glist, x ););
    sys_vgui(".x%x.c.s%x.text yview end-2char \n", x->x_glist, x );
}

/* Clear the contents of the text widget */
static void listbox_clear(t_listbox* x)
{
    DEBUG(post(".x%x.c.s%x.text delete 0.0 end \n", x->x_glist, x););
    sys_vgui(".x%x.c.s%x.text delete 0.0 end \n", x->x_glist, x);
}

/* Function to reset the contents of the listbox box */
static void listbox_set(t_listbox* x,  t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("listbox_set"););
    int i;

    listbox_clear(x);
    listbox_add(x, s, argc, argv);
}

/* Output the symbol */
/* , t_symbol *s, int argc, t_atom *argv) */
static void listbox_output(t_listbox* x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list(x->x_data_outlet, s, argc, argv );
}

/* Pass the contents of the text widget onto the listbox_output fuction above */
static void listbox_bang_output(t_listbox* x)
{
    /* With "," and ";" escaping thanks to JMZ */
    DEBUG(post("pd [concat %s output [string map {\",\" \"\\\\,\" \";\" \"\\\\;\"} \
                [.x%x.c.s%x.text get 0.0 end]] \\;]\n", 
               x->x_receive_name->s_name, x->x_glist, x););
    sys_vgui("pd [concat %s output [string map {\",\" \"\\\\,\" \";\" \"\\\\;\"} \
              [.x%x.c.s%x.text get 0.0 end]] \\;]\n", 
             x->x_receive_name->s_name, x->x_glist, x);

    DEBUG(post("bind .x%x.c.s%x.text <Leave> {focus [winfo parent .x%x.c.s%x]} \n", 
               x->x_glist, x, x->x_glist, x););
    sys_vgui("bind .x%x.c.s%x.text <Leave> {focus [winfo parent .x%x.c.s%x]} \n", 
             x->x_glist, x, x->x_glist, x);
}

static void listbox_keyup(t_listbox *x, t_float f)
{
/*     DEBUG(post("listbox_keyup");); */
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

static void listbox_save(t_gobj *z, t_binbuf *b)
{
    t_listbox *x = (t_listbox *)z;

    binbuf_addv(b, "ssiisiiss", gensym("#X"),gensym("obj"),
                x->x_obj.te_xpix, x->x_obj.te_ypix, 
                gensym("listbox"), x->x_width, x->x_height, x->x_bgcolour, x->x_fgcolour);
    binbuf_addv(b, ";");
}


static void listbox_option_float(t_listbox* x, t_symbol *option, t_float value)
{
	DEBUG(post(".x%x.c.s%x.text configure -%s %f \n", 
               x->x_glist, x, option->s_name, value););
	sys_vgui(".x%x.c.s%x.text configure -%s %f \n", 
               x->x_glist, x, option->s_name, value);
}

static void listbox_option_symbol(t_listbox* x, t_symbol *option, t_symbol *value)
{
	DEBUG(post(".x%x.c.s%x.text configure -%s {%s} \n", 
               x->x_glist, x, option->s_name, value->s_name););
	sys_vgui(".x%x.c.s%x.text configure -%s {%s} \n", 
               x->x_glist, x, option->s_name, value->s_name);
}

static void listbox_option(t_listbox *x, t_symbol *s, int argc, t_atom *argv)
{
    t_symbol *tmp_symbol = s; /* <-- this gets rid of the unused variable warning */

    tmp_symbol = atom_getsymbolarg(1, argc, argv);
    if(tmp_symbol == &s_)
    {
        listbox_option_float(x,atom_getsymbolarg(0, argc, argv),
                           atom_getfloatarg(1, argc, argv));
    }
    else
    {
        listbox_option_symbol(x,atom_getsymbolarg(0, argc, argv),tmp_symbol);
    }
}

static void listbox_scrollbar(t_listbox *x, t_float f)
{
    if(f > 0)
        draw_scrollbar(x);
    else
        erase_scrollbar(x);
}


/* function to change colour of text background */
void listbox_bgcolour(t_listbox* x, t_symbol* bgcol)
{
	x->x_bgcolour = bgcol;
	DEBUG(post(".x%x.c.s%x.text configure -background \"%s\" \n", 
               x->x_glist, x, x->x_bgcolour->s_name););
	sys_vgui(".x%x.c.s%x.text configure -background \"%s\" \n", 
             x->x_glist, x, x->x_bgcolour->s_name);
}

/* function to change colour of text foreground */
void listbox_fgcolour(t_listbox* x, t_symbol* fgcol)
{
	x->x_fgcolour = fgcol;
	DEBUG(post(".x%x.c.s%x.text configure -foreground \"%s\" \n", 
               x->x_glist, x, x->x_fgcolour->s_name););
	sys_vgui(".x%x.c.s%x.text configure -foreground \"%s\" \n", 
             x->x_glist, x, x->x_fgcolour->s_name);
}

static void listbox_fontsize(t_listbox *x, t_float font_size)
{
    DEBUG(post("listbox_fontsize"););
    post("font size: %f",font_size);
    if(font_size > 8) 
    {
        x->x_font_size = (t_int)font_size;
        DEBUG(post(".x%x.c.s%x.text configure -font {%s %d %s} \n", 
                   x->x_glist, x,
                   x->x_font_face->s_name, x->x_font_size, x->x_font_weight->s_name););
        sys_vgui(".x%x.c.s%x.text configure -font {%s %d %s} \n", 
                 x->x_glist, x,
                 x->x_font_face->s_name, x->x_font_size, 
                 x->x_font_weight->s_name);
    }
    else
        pd_error(x,"listbox: invalid font size: %f",font_size);
}

static void listbox_size(t_listbox *x, t_float width, t_float height)
{
    DEBUG(post("listbox_size"););
    x->x_height = height;
    x->x_width = width;
}

static void listbox_free(t_listbox *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_receive_name);
}

static void *listbox_new(t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("listbox_new"););
    t_listbox *x = (t_listbox *)pd_new(listbox_class);
    char buf[MAXPDSTRING];

    x->x_height = 1;
    x->x_font_face = gensym("helvetica");
    x->x_font_size = 10;
    x->x_font_weight = gensym("normal");
    x->x_have_scrollbar = 1;
	
	if (argc < 4)
	{
		post("listbox: You must enter at least 4 arguments. Default values used.");
		x->x_width = 124;
		x->x_height = 100;
		x->x_bgcolour = gensym("grey70");
		x->x_fgcolour = gensym("black");
		
	} else {
		/* Copy args into structure */
		x->x_width = atom_getint(argv);
		x->x_height = atom_getint(argv+1);
		x->x_bgcolour = atom_getsymbol(argv+2);
		x->x_fgcolour = atom_getsymbol(argv+3);
	}	

    x->x_data_outlet = outlet_new(&x->x_obj, &s_float);
    x->x_status_outlet = outlet_new(&x->x_obj, &s_symbol);

    snprintf(buf,MAXPDSTRING,"#listbox%lx",(long unsigned int)x);
    x->x_receive_name = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->x_receive_name);

    return (x);
}

void listbox_setup(void) {
    listbox_class = class_new(gensym("listbox"), (t_newmethod)listbox_new, 
                            (t_method)listbox_free, sizeof(t_listbox),0,A_GIMME,0);
				
	class_addbang(listbox_class, (t_method)listbox_bang_output);

    class_addmethod(listbox_class, (t_method)listbox_keyup,
                    gensym("keyup"),
                    A_DEFFLOAT,
                    0);

    class_addmethod(listbox_class, (t_method)listbox_scrollbar,
                    gensym("scrollbar"),
                    A_DEFFLOAT,
                    0);

    class_addmethod(listbox_class, (t_method)listbox_option,
                    gensym("option"),
                    A_GIMME,
                    0);

    class_addmethod(listbox_class, (t_method)listbox_size,
                    gensym("size"),
                    A_DEFFLOAT,
                    A_DEFFLOAT,
                    0);

    class_addmethod(listbox_class, (t_method)listbox_fontsize,
                    gensym("fontsize"),
                    A_DEFFLOAT,
                    0);
	
	class_addmethod(listbox_class, (t_method)listbox_output,
                    gensym("output"),
                    A_GIMME,
                    0);
								  
	class_addmethod(listbox_class, (t_method)listbox_set,
                    gensym("set"),
                    A_GIMME,
                    0);
								  
	class_addmethod(listbox_class, (t_method)listbox_add,
                    gensym("add"),
                    A_GIMME,
                    0);
								  
	class_addmethod(listbox_class, (t_method)listbox_clear,
                    gensym("clear"),
                    0);
								  
	class_addmethod(listbox_class, (t_method)listbox_bgcolour,
                    gensym("bgcolour"),
                    A_DEFSYMBOL,
                    0);
								  
	class_addmethod(listbox_class, (t_method)listbox_fgcolour,
                    gensym("fgcolour"),
                    A_DEFSYMBOL,
                    0);
								  
    class_setwidget(listbox_class,&listbox_widgetbehavior);
    class_setsavefn(listbox_class,&listbox_save);

    backspace_symbol = gensym("backspace");
    return_symbol = gensym("return");
	space_symbol = gensym("space");
	tab_symbol = gensym("tab");
	escape_symbol = gensym("escape");
	left_symbol = gensym("left");
	right_symbol = gensym("right");
	up_symbol = gensym("up");
	down_symbol = gensym("down");
    
	post("Text v0.1 Ben Bogart.\nCVS: $Revision: 1.1 $ $Date: 2007-10-28 21:19:50 $");
}


