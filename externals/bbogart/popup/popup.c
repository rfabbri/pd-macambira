/* Popup menu widget for PD                                              *
 * Based on button from GGEE by Guenter Geiger                           *
 * Copyright Ben Bogart 2004 ben@ekran.org                               * 

 * This program is distributed under the terms of the GNU General Public *
 * License                                                               *

 * popup is free software; you can redistribute it and/or modify         *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *

 * popup is distributed in the hope that it will be useful,              *
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

#define MAX_OPTIONS 100

typedef struct _popup
{
     t_object x_obj;

     t_glist * x_glist;
     t_outlet* out2;
     t_inlet* in2;
     int x_rect_width;
     int x_rect_height;
     t_symbol*  x_sym;
	
     int x_height;
     int x_width;
	 
     int current_selection;
     int x_num_options;	 
     t_symbol* x_colour;
     t_symbol* x_name;
	
     t_symbol* x_options[MAX_OPTIONS];

} t_popup;

/* widget helper functions */

/* Append " x " to the following line to show debugging messages */
#define DEBUG(x) 


static void draw_inlets(t_popup *x, t_glist *glist, int firsttime, int nin, int nout)
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
			onset, text_ypix(&x->x_obj, glist)-2,
			     onset + IOWIDTH, text_ypix(&x->x_obj, glist)-1,
			x, i, x);
	  else
	       sys_vgui(".x%x.c coords %xi%d %d %d %d %d\n",
			glist_getcanvas(glist), x, i,
			onset, text_ypix(&x->x_obj, glist),
			onset + IOWIDTH, text_ypix(&x->x_obj, glist)-1);
	  
     }
     DEBUG(post("draw inlet end");)
}


static void draw_handle(t_popup *x, t_glist *glist, int firsttime) {

  DEBUG(post("draw_handle start");)
  int onset = text_xpix(&x->x_obj, glist) + (x->x_rect_width - IOWIDTH);

  if (firsttime) {
    sys_vgui(".x%x.c create rectangle %d %d %d %d -tags %xhandle\n",
	     glist_getcanvas(glist),
	     onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 12,
	     onset + IOWIDTH-2, text_ypix(&x->x_obj, glist) + x->x_rect_height-4,
	     x);
  } 
  else {
    sys_vgui(".x%x.c coords %xhandle %d %d %d %d\n",
	     glist_getcanvas(glist), x, 
	     onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 12,
	     onset + IOWIDTH-2, text_ypix(&x->x_obj, glist) + x->x_rect_height-4);
  }
  DEBUG(post("draw_handle end");)
}

static void create_widget(t_popup *x, t_glist *glist)
{
  DEBUG(post("create_widget start");)

  char text[MAXPDSTRING];
  int len,i;
  t_symbol* temp_name;
  t_canvas *canvas=glist_getcanvas(glist);
  x->x_rect_width = x->x_width;
  x->x_rect_height =  x->x_height+2;
  
  /* Create menubutton and empty menu widget -- maybe the menu should be created elseware?*/

  /* draw using the last name if it was selected otherwise use default name. */
  if(x->current_selection < 0)
  {
    temp_name = x->x_name;
  } else {
    temp_name = x->x_options[x->current_selection];
  }

  /* Seems we have to delete the widget in case it already exists (Provided by Guenter)*/
  sys_vgui("destroy .x%x.c.s%x\n",glist_getcanvas(glist),x);

  sys_vgui("set %xw .x%x.c.s%x ; menubutton $%xw -relief raised -background \"%s\" -text \"%s\" -direction flush -menu $%xw.menu ; menu $%xw.menu -tearoff 0\n",
		x,canvas,x,x,x->x_colour->s_name,temp_name->s_name,x,x);

  for(i=0 ; i<x->x_num_options ; i++)
  {
	sys_vgui(".x%x.c.s%x.menu add command -label \"%s\" -command {.x%x.c.s%x configure -text \"%s\" ; popup_sel%x \"%d\"} \n", 
		canvas, x, x->x_options[i]->s_name, canvas, x, x->x_options[i]->s_name, x, i);
  }

  DEBUG(post("id: .x%x.c.s%x", canvas, x);)
  DEBUG(post("create_widget end");)
}

static void popup_drawme(t_popup *x, t_glist *glist, int firsttime)
{
  t_canvas *canvas=glist_getcanvas(glist);
  DEBUG(post("drawme start");)
  DEBUG(post("drawme %d",firsttime);)
     if (firsttime) {
       DEBUG(post("glist %x canvas %x",x->x_glist,canvas);)
       create_widget(x,glist);	       
       x->x_glist = canvas;
       sys_vgui(".x%x.c create window %d %d -width %d -height %d -anchor nw -window .x%x.c.s%x -tags %xS\n", 
		canvas,text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist), x->x_width, x->x_height, x->x_glist,x,x);
              
     }     
     else {
       sys_vgui(".x%x.c coords %xS %d %d\n",
		canvas, x,
		text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist));
     }
     draw_inlets(x, glist, firsttime, 2,2);
     //     draw_handle(x, glist, firsttime);

  // Output a bang to first outlet when we're ready to receive float messages the first time!. 
  // Too bad this is NOT always the first time... window shading makes the bang go out again. :(
  if(firsttime) {outlet_bang(x->x_obj.ob_outlet);}

  DEBUG(post("drawme end");)
}


static void popup_erase(t_popup* x,t_glist* glist)
{
     int n;

     DEBUG(post("erase start");)
     sys_vgui("destroy .x%x.c.s%x\n",glist_getcanvas(glist),x);

     sys_vgui(".x%x.c delete %xS\n",glist_getcanvas(glist), x);

     /* inlets and outlets */
     
     sys_vgui(".x%x.c delete %xi\n",glist_getcanvas(glist),x); /* Added tag for all inlets of one instance */
     sys_vgui(".x%x.c delete %xo\n",glist_getcanvas(glist),x); /* Added tag for all outlets of one instance */
     sys_vgui(".x%x.c delete  %xhandle\n",glist_getcanvas(glist),x,0);

    DEBUG(post("erase end");)
}
	


/* ------------------------ popup widgetbehaviour----------------------------- */


static void popup_getrect(t_gobj *z, t_glist *owner,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    DEBUG(post("getrect start");)

    int width, height;
    t_popup* s = (t_popup*)z;

    width = s->x_rect_width;
    height = s->x_rect_height;
    *xp1 = text_xpix(&s->x_obj, owner);
    *yp1 = text_ypix(&s->x_obj, owner) - 1;
    *xp2 = text_xpix(&s->x_obj, owner) + width;
    *yp2 = text_ypix(&s->x_obj, owner) + height;
  
    DEBUG(post("getrect end");)
}

static void popup_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_popup *x = (t_popup *)z;
    DEBUG(post("displace start");)
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if (glist_isvisible(glist))
    {
      sys_vgui(".x%x.c coords %xSEL %d %d %d %d\n",
	       glist_getcanvas(glist), x,
	       text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)-1,
	       text_xpix(&x->x_obj, glist) + x->x_rect_width, text_ypix(&x->x_obj, glist) + x->x_rect_height-2);
      
      popup_drawme(x, glist, 0);
      canvas_fixlinesfor(glist_getcanvas(glist),(t_text*) x);
    }
    DEBUG(post("displace end");)
}

static void popup_select(t_gobj *z, t_glist *glist, int state)
{
     DEBUG(post("select start");)

     t_popup *x = (t_popup *)z;
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

     DEBUG(post("select end");)
}

static void popup_activate(t_gobj *z, t_glist *glist, int state)
{
    DEBUG(post("activate commented out!");)
  
/* What does this do, why commented out? 
    t_text *x = (t_text *)z;
    t_rtext *y = glist_findrtext(glist, x);
    if (z->g_pd != gatom_class) rtext_activate(y, state);*/
}

static void popup_delete(t_gobj *z, t_glist *glist)
{
    DEBUG(post("delete start");)

    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist_getcanvas(glist), x);

    DEBUG(post("delete end");)
}

       
static void popup_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_popup* s = (t_popup*)z;
    t_rtext *y;
    DEBUG(post("vis start");)
    DEBUG(post("vis: %d",vis);)
    if (vis) {
#ifdef PD_MINOR_VERSION
      	y = (t_rtext *) rtext_new(glist, (t_text *)z);
#else
        y = (t_rtext *) rtext_new(glist, (t_text *)z,0,0);
#endif
	 popup_drawme(s, glist, 1);
    }
    else {
	y = glist_findrtext(glist, (t_text *)z);
	 popup_erase(s,glist);
	rtext_free(y);
    }

    DEBUG(post("vis end");)
}

static void popup_save(t_gobj *z, t_binbuf *b);

t_widgetbehavior   popup_widgetbehavior = {
  w_getrectfn:  popup_getrect,
  w_displacefn: popup_displace,
  w_selectfn:   popup_select,
  w_activatefn: popup_activate,
  w_deletefn:   popup_delete,
  w_visfn:      popup_vis,
#if PD_MINOR_VERSION < 37
  w_savefn:     popup_save,
#endif
  w_clickfn:    NULL,
#if PD_MINOR_VERSION < 37
  w_propertiesfn: NULL,
#endif

}; 

static void popup_output(t_popup* x, t_floatarg popup_index)
{
  DEBUG(post("output start");)

  x->current_selection = popup_index;
  outlet_symbol(x->out2, x->x_options[(int)popup_index]);
  outlet_float(x->x_obj.ob_outlet, popup_index); 
}


static void popup_save(t_gobj *z, t_binbuf *b)
{
        DEBUG(post("save start");)
    
	int i;
    t_popup *x = (t_popup *)z;

    binbuf_addv(b, "ssiisiiss", gensym("#X"),gensym("obj"),
		x->x_obj.te_xpix, x->x_obj.te_ypix ,  
		gensym("popup"), x->x_width, x->x_height, x->x_colour, x->x_name);
	/* Loop for menu items */
	for(i=0 ; i<x->x_num_options ; i++)
	{
		binbuf_addv(b, "s", x->x_options[i]);
	}
    binbuf_addv(b, ";");

    DEBUG(post("vis end");)
}

/* function to change the popup's menu */
void popup_options(t_popup* x, t_symbol *s, int argc, t_atom *argv)
{
	DEBUG(post("options start");)
	
	int i;

	x->x_num_options = argc;
	
	/* delete old menu items */
	sys_vgui(".x%x.c.s%x.menu delete 0 end \n", x->x_glist, x);

	for(i=0 ; i<argc ; i++)
	{
		x->x_options[i] = atom_getsymbol(argv+i);
		sys_vgui(".x%x.c.s%x.menu add command -label \"%s\" -command {.x%x.c.s%x configure -text \"%s\" ; popup_sel%x \"%d\"} \n", 
			x->x_glist, x, x->x_options[i]->s_name, x->x_glist, x, x->x_options[i]->s_name, x, i);
	}

	DEBUG(post("options end");)
}

/* function to change colour of popup background */
static void popup_bgcolour(t_popup* x, t_symbol* col)
{
	DEBUG(post("bgcolour start");)

	x->x_colour = col;
	sys_vgui(".x%x.c.s%x configure -background \"%s\"\n", x->x_glist, x, col->s_name);
}

/* Function to change name of popup */
static void popup_name(t_popup* x, t_symbol *name)
{
	DEBUG(post("name start");)

	x->x_name = name;
	sys_vgui(".x%x.c.s%x configure -text \"%s\"\n", x->x_glist, x, name->s_name);
}

/* Function to select a menu option by inlet */
static void popup_iselect(t_popup* x, t_floatarg item)
{
	DEBUG(post("iselect start");)

	int i=(int)item;
	if( i<x->x_num_options && i>=0)
	{
		sys_vgui(".x%x.c.s%x configure -text \"%s\" ; popup_sel%x \"%d\" \n",
			glist_getcanvas(x->x_glist), x, x->x_options[i]->s_name,x, i);

	} else {
		post("popup: Valid menu selections are from %d to %d\npopup: You entered %d.", 0, x->x_num_options, i);
	}

	DEBUG(post("iselect end");)
}

/* Function to choose value via symbol name */
static void popup_symselect(t_popup* x, t_symbol *s)
{
	int i,match=0;

	/* Compare inlet symbol to each option */
	for(i=0; i<x->x_num_options; i++)
	{
	  if(x->x_options[i]->s_name == s->s_name)
	  {
	    sys_vgui(".x%x.c.s%x configure -text \"%s\" ; popup_sel%x \"%d\" \n",
                        glist_getcanvas(x->x_glist), x, x->x_options[i]->s_name,x, i);
	    match = 1;
	    break;
          }
	}

	if(match != 1)
	  post("popup: '%s' is not an available option.", s->s_name);
	
}

/* Function to append symbols to popup list */
void popup_append(t_popup* x, t_symbol *s, int argc, t_atom *argv)
{
        DEBUG(post("append start");)

        int i, new_limit;

        new_limit = x->x_num_options + argc;

        for(i=x->x_num_options ; i<new_limit ; i++)
        {
                x->x_options[i] = atom_getsymbol(argv+i-x->x_num_options);
                sys_vgui(".x%x.c.s%x.menu add command -label \"%s\" -command {.x%x.c.s%x configure -text \"%s\" ; popup_sel%x \"%d\"} \n",
                        x->x_glist, x, x->x_options[i]->s_name, x->x_glist, x, x->x_options[i]->s_name, x, i);
        }

	x->x_num_options = new_limit;

        DEBUG(post("append end");)
}



static t_class *popup_class;


static void *popup_new(t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("popup new start");)

    t_popup *x = (t_popup *)pd_new(popup_class);
    int i;
	char buf[256];

    x->x_glist = (t_glist*)NULL;

    x->x_height = 25;
    x->current_selection = -1;
	
	if (argc < 5)
	{
		post("popup: You must enter at least 5 arguments. Default values used.\n\nArguments:\npopup [width] [height] [colour] [name] [option-1] [option-2] ...");
		x->x_width = 124;
		x->x_height = 25;
		x->x_num_options = 1; 
		x->x_colour = gensym("#ffffff");
		x->x_name = gensym("popup");
		
		x->x_options[0] = gensym("option");
		
	} else {
		/* Copy args into structure */
		x->x_width = atom_getint(argv);
		x->x_height = atom_getint(argv+1);
		x->x_colour = atom_getsymbol(argv+2);
		x->x_name = atom_getsymbol(argv+3);
		
		x->x_num_options = argc-4;
		
		for(i=0 ; i<x->x_num_options ; i++)
		{
			x->x_options[i] = atom_getsymbol( argv+(i+4) );
		}
	}	

	/* Bind the recieve "popup%p" to the widget outlet*/
    sprintf(buf,"popup%p",x);
    x->x_sym = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->x_sym);

	/* define proc in tcl/tk where "popup%p" is the receive, "output" is the method, and "$index" is an argument. */
    sys_vgui("proc popup_sel%x {index} {\n pd [concat popup%p output $index \\;]\n }\n",x,x); 

    /* Add symbol inlet (hard to say how this actually works?? */
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("symbol"), gensym(""));
    outlet_new(&x->x_obj, &s_float);
    x->out2 = outlet_new(&x->x_obj, &s_symbol);

DEBUG(post("popup new end");)

    return (x);
}

void popup_setup(void) {

    DEBUG(post("setup start");)

    popup_class = class_new(gensym("popup"), (t_newmethod)popup_new, 0,
				sizeof(t_popup),0,A_GIMME,0);
				
	class_addmethod(popup_class, (t_method)popup_output,
								  gensym("output"),
								  A_DEFFLOAT,
								  0);

	class_addmethod(popup_class, (t_method)popup_name,
								  gensym("name"),
								  A_DEFSYMBOL,
								  0);
								  
	class_addmethod(popup_class, (t_method)popup_options,
								  gensym("options"),
								  A_GIMME,
								  0);
								  
	class_addmethod(popup_class, (t_method)popup_bgcolour,
								  gensym("bgcolour"),
								  A_DEFSYMBOL,
								  0);

        class_addmethod(popup_class, (t_method)popup_append,
                                                                  gensym("append"),
                                                                  A_GIMME,
                                                                  0);

	class_addmethod(popup_class, (t_method)popup_symselect,
                                                                  gensym(""),
                                                                  A_DEFSYMBOL,
                                                                  0);

	class_doaddfloat(popup_class, (t_method)popup_iselect);

//	class_addsymbol(popup_class, (t_method)popup_symselect);

    class_setwidget(popup_class,&popup_widgetbehavior);
#if PD_MINOR_VERSION >= 37
    class_setsavefn(popup_class,&popup_save);
#endif

	post("Popup v0.1 Ben Bogart.\nCVS: $Revision: 1.11 $ $Date: 2005-01-17 21:06:59 $");
}


