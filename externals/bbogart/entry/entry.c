/* text entry widget for PD                                              *
 * Based on button from GGEE by Guenter Geiger                           *
 * Copyright Ben Bogart 2004 ben@ekran.org                               * 

 * This program is distributed under the terms of the GNU General Public *
 * License                                                               *

 * entry is free software; you can redistribute it and/or modify         *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *

 * entry is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          */

#include <m_pd.h>
#include <g_canvas.h>
#include <stdio.h>
#include <string.h>



#ifdef NT
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#if PD_MINOR_VERSION < 37
#define t_rtext t_text
#endif

#ifndef IOWIDTH 
#define IOWIDTH 4
#endif

typedef struct _entry
{
     t_object x_obj;

     t_glist * x_glist;
     int x_rect_width;
     int x_rect_height;
     t_symbol*  x_sym;
	
     int x_height;
     int x_width;
	 
     t_symbol* x_bgcolour;
     t_symbol* x_fgcolour;
     t_symbol* x_contents;

} t_entry;

/* widget helper functions */

#define DEBUG(x)


static void draw_inlets(t_entry *x, t_glist *glist, int firsttime, int nin, int nout)
{
 /* outlets */
     int n = nin;
     int nplus, i;
     nplus = (n == 1 ? 1 : n-1);
     DEBUG(post("draw inlet");)
     for (i = 0; i < n; i++)
     {
	  int onset = text_xpix(&x->x_obj, glist) + (x->x_rect_width - IOWIDTH) * i / nplus;
	  if (firsttime)
	       sys_vgui(".x%x.c create rectangle %d %d %d %d -tags {%xo%d %xo}\n",
			glist_getcanvas(glist),
			onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 2,
			onset + IOWIDTH, text_ypix(&x->x_obj, glist) + x->x_rect_height-1,
			x, i, x);
	  else
	       sys_vgui(".x%x.c coords %xo%d %d %d %d %d\n",
			glist_getcanvas(glist), x, i,
			onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 2,
			onset + IOWIDTH, text_ypix(&x->x_obj, glist) + x->x_rect_height-1);
     }
 /* inlets */
     n = nout; 
     nplus = (n == 1 ? 1 : n-1);
     for (i = 0; i < n; i++)
     {
	  int onset = text_xpix(&x->x_obj, glist) + (x->x_rect_width - IOWIDTH) * i / nplus;
	  if (firsttime)
	       sys_vgui(".x%x.c create rectangle %d %d %d %d -tags {%xi%d %xi}\n",
			glist_getcanvas(glist),
			onset, text_ypix(&x->x_obj, glist),
			     onset + IOWIDTH, text_ypix(&x->x_obj, glist)+1,
			x, i, x);
	  else
	       sys_vgui(".x%x.c coords %xi%d %d %d %d %d\n",
			glist_getcanvas(glist), x, i,
			onset, text_ypix(&x->x_obj, glist),
			onset + IOWIDTH, text_ypix(&x->x_obj, glist)+1);
	  
     }
     DEBUG(post("draw inlet end");)
}


static void draw_handle(t_entry *x, t_glist *glist, int firsttime) {
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

static void create_widget(t_entry *x, t_glist *glist)
{
  char text[MAXPDSTRING];
  int len,i;
  t_canvas *canvas=glist_getcanvas(glist);
  /* I guess this is for fine-tuning of the rect size based on width and height? */
  x->x_rect_width = x->x_width;
  x->x_rect_height =  x->x_height+2;
  
  /* Create text widget */
  sys_vgui("set %xw .x%x.c.s%x ; text $%xw -font {helvetica 10} -border 1 -highlightthickness 1 -relief sunken -bg \"%s\" -fg \"%s\" \n",
		x,canvas,x,x,x->x_bgcolour->s_name,x->x_fgcolour->s_name);
  sys_vgui("bind .x%x.c.s%x <Leave> {focus [winfo parent .x%x.c.s%x]} \n", canvas, x, canvas, x);
  sys_vgui("namespace eval entry%x {} \n", x);
  
}

static void entry_drawme(t_entry *x, t_glist *glist, int firsttime)
{
  t_canvas *canvas=glist_getcanvas(glist);
  DEBUG(post("drawme %d",firsttime);)
     if (firsttime) {
       DEBUG(post("glist %x canvas %x",x->x_glist,canvas);)
       create_widget(x,glist);	       
       x->x_glist = canvas;
       sys_vgui(".x%x.c create window %d %d -anchor nw -window .x%x.c.s%x -tags %xS -width %d -height %d \n", 
		canvas,text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),x->x_glist,x,x, x->x_width, x->x_height);
              
     }     
     else {
       sys_vgui(".x%x.c coords %xS \
                %d %d\n",
		canvas, x,
		text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist));
     }
     draw_inlets(x, glist, firsttime, 1,1);
     //     draw_handle(x, glist, firsttime);

}


static void entry_erase(t_entry* x,t_glist* glist)
{
     int n;

     DEBUG(post("erase");)
     sys_vgui("destroy .x%x.c.s%x\n",glist_getcanvas(glist),x);

     sys_vgui(".x%x.c delete %xS\n",glist_getcanvas(glist), x);

     /* inlets and outlets */
     
     sys_vgui(".x%x.c delete %xi\n",glist_getcanvas(glist),x); /* Added tag for all inlets of one instance */
     sys_vgui(".x%x.c delete %xo\n",glist_getcanvas(glist),x); /* Added tag for all outlets of one instance */
     sys_vgui(".x%x.c delete  %xhandle\n",glist_getcanvas(glist),x,0);
}
	


/* ------------------------ text widgetbehaviour----------------------------- */


static void entry_getrect(t_gobj *z, t_glist *owner,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    int width, height;
    t_entry* s = (t_entry*)z;

    width = s->x_rect_width;
    height = s->x_rect_height;
    *xp1 = text_xpix(&s->x_obj, owner);
    *yp1 = text_ypix(&s->x_obj, owner) - 1;
    *xp2 = text_xpix(&s->x_obj, owner) + width;
    *yp2 = text_ypix(&s->x_obj, owner) + height;
}

static void entry_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_entry *x = (t_entry *)z;
    DEBUG(post("displace");)
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if (glist_isvisible(glist))
    {
      sys_vgui(".x%x.c coords %xSEL %d %d %d %d\n",
	       glist_getcanvas(glist), x,
	       text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)-1,
	       text_xpix(&x->x_obj, glist) + x->x_rect_width, text_ypix(&x->x_obj, glist) + x->x_rect_height-2);
      
      entry_drawme(x, glist, 0);
      canvas_fixlinesfor(glist_getcanvas(glist),(t_text*) x);
    }
    DEBUG(post("displace end");)
}

static void entry_select(t_gobj *z, t_glist *glist, int state)
{
     t_entry *x = (t_entry *)z;
     if (state) {
	  sys_vgui(".x%x.c create rectangle \
%d %d %d %d -tags %xSEL -outline blue\n",
		   glist_getcanvas(glist),
		   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)-1,
		   text_xpix(&x->x_obj, glist) + x->x_rect_width, text_ypix(&x->x_obj, glist) + x->x_rect_height-2,
		   x);
     }
     else {
	  sys_vgui(".x%x.c delete %xSEL\n",
		   glist_getcanvas(glist), x);
     }
}

static void entry_activate(t_gobj *z, t_glist *glist, int state)
{
/* What does this do, why commented out? 
    t_text *x = (t_text *)z;
    t_rtext *y = glist_findrtext(glist, x);
    if (z->g_pd != gatom_class) rtext_activate(y, state);*/
}

static void entry_delete(t_gobj *z, t_glist *glist)
{
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist_getcanvas(glist), x);
}

       
static void entry_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_entry* s = (t_entry*)z;
    t_rtext *y;
    DEBUG(post("vis: %d",vis);)
    if (vis) {
#ifdef PD_MINOR_VERSION
      	y = (t_rtext *) rtext_new(glist, (t_text *)z);
#else
        y = (t_rtext *) rtext_new(glist, (t_text *)z,0,0);
#endif
	 entry_drawme(s, glist, 1);
    }
    else {
	y = glist_findrtext(glist, (t_text *)z);
	 entry_erase(s,glist);
	rtext_free(y);
    }
}

static void entry_save(t_gobj *z, t_binbuf *b);

t_widgetbehavior   entry_widgetbehavior = {
  w_getrectfn:  entry_getrect,
  w_displacefn: entry_displace,
  w_selectfn:   entry_select,
  w_activatefn: entry_activate,
  w_deletefn:   entry_delete,
  w_visfn:      entry_vis,
#if PD_MINOR_VERSION < 37
  w_savefn:     entry_save,
#endif
  w_clickfn:    NULL,
#if PD_MINOR_VERSION < 37
  w_propertiesfn: NULL,
#endif
}; 

/* Function to reset the contents of the entry box */
static void entry_set(t_entry* x,  t_symbol *s, int argc, t_atom *argv)
{
  int i;
  t_symbol *tmp;
  
  sys_vgui(".x%x.c.s%x delete 0.0 end \n", x->x_glist, x);
  for(i=0; i<argc ; i++)
  {
	tmp = atom_getsymbol(argv+i);
	sys_vgui("lappend ::entry%x::list %s \n", x, tmp->s_name );
  }
  sys_vgui(".x%x.c.s%x insert end $::entry%x::list ; unset ::entry%x::list \n", x->x_glist, x, x, x );
}

/* Clear the contents of the text widget */
static void entry_clear(t_entry* x)
{
  sys_vgui(".x%x.c.s%x delete 0.0 end \n", x->x_glist, x);
}

/* Output the symbol */
/* , t_symbol *s, int argc, t_atom *argv) */
static void entry_output(t_entry* x, t_symbol *s, int argc, t_atom *argv)
{
  outlet_list(x->x_obj.ob_outlet, s, argc, argv );
}

/* Pass the contents of the text widget onto the entry_output fuction above */
static void entry_bang_output(t_entry* x)
{
  sys_vgui("pd [concat entry%p output [.x%x.c.s%x get 0.0 end] \\;]\n", x, x->x_glist, x);
  sys_vgui("bind .x%x.c.s%x <Leave> {focus [winfo parent .x%x.c.s%x]} \n", x->x_glist, x, x->x_glist, x);
}


static void entry_save(t_gobj *z, t_binbuf *b)
{
	int i;
    t_entry *x = (t_entry *)z;

    binbuf_addv(b, "ssiisiiss", gensym("#X"),gensym("obj"),
		x->x_obj.te_xpix, x->x_obj.te_ypix ,  
		gensym("entry"), x->x_width, x->x_height, x->x_bgcolour, x->x_fgcolour);
    binbuf_addv(b, ";");
}

/* function to change colour of text background */
void entry_bgcolour(t_entry* x, t_symbol* bgcol)
{
	x->x_bgcolour->s_name = bgcol->s_name;
	sys_vgui(".x%x.c.s%x configure -background \"%s\" \n", x->x_glist, x, bgcol->s_name);
}

/* function to change colour of text foreground */
void entry_fgcolour(t_entry* x, t_symbol* fgcol)
{
	x->x_fgcolour->s_name = fgcol->s_name;
	sys_vgui(".x%x.c.s%x configure -foreground \"%s\" \n", x->x_glist, x, fgcol->s_name);
}

static t_class *entry_class;


static void *entry_new(t_symbol *s, int argc, t_atom *argv)
{
    t_entry *x = (t_entry *)pd_new(entry_class);
    int i;
    char buf[256];

    /*x->x_glist = (t_glist*)NULL;*/
    /*x->x_glist = canvas_getcurrent();*/

    
    x->x_height = 1;
	
	if (argc < 4)
	{
		post("entry: You must enter at least 4 arguments. Default values used.");
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


     /*x->x_width = x->x_char_width*5;  
     x->x_height = (int)x->x_char_height*0.7;
    x->x_width = x->x_char_width*7;     tuned for Linux 
     x->x_height = x->x_char_height*5; */

    /* Bind the recieve "entry%p" to the widget instance*/
    sprintf(buf,"entry%p",x);
    x->x_sym = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->x_sym);
	
    outlet_new(&x->x_obj, &s_float);
    return (x);
}

void entry_setup(void) {
    entry_class = class_new(gensym("entry"), (t_newmethod)entry_new, 0,
				sizeof(t_entry),0,A_GIMME,0);
				
	class_addbang(entry_class, (t_method)entry_bang_output);
	
	class_addmethod(entry_class, (t_method)entry_output,
        gensym("output"),
        A_GIMME,
        0);
								  
	class_addmethod(entry_class, (t_method)entry_set,
        gensym("set"),
        A_GIMME,
        0);
								  
	class_addmethod(entry_class, (t_method)entry_clear,
        gensym("clear"),
        0);
								  
	class_addmethod(entry_class, (t_method)entry_bgcolour,
        gensym("bgcolour"),
        A_DEFSYMBOL,
        0);
								  
	class_addmethod(entry_class, (t_method)entry_fgcolour,
        gensym("fgcolour"),
        A_DEFSYMBOL,
        0);
								  
    class_setwidget(entry_class,&entry_widgetbehavior);
#if PD_MINOR_VERSION >= 37
    class_setsavefn(entry_class,&entry_save);
#endif

	post("Text v0.1 Ben Bogart.");
}


