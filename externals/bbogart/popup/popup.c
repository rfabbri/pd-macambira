/* Popup menu widget for PD */
/* Based on button from GGEE by Guenter Geiger */
/* Copyright Ben Bogart 2004 ben@ekran.org */ 


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
     int x_rect_width;
     int x_rect_height;
     t_symbol*  x_sym;
	
     int x_height;
     int x_width;
	 
	 int x_num_options;	 
	 t_symbol* x_colour;
     t_symbol* x_name;
	
	 t_symbol* x_options[MAX_OPTIONS];

} t_popup;

/* widget helper functions */

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


static void draw_handle(t_popup *x, t_glist *glist, int firsttime) {
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

static void create_widget(t_popup *x, t_glist *glist)
{
  char text[MAXPDSTRING];
  int len,i;
  t_canvas *canvas=glist_getcanvas(glist);
  x->x_rect_width = x->x_width;
  x->x_rect_height =  x->x_height+2;
  
  /* Create menubutton and empty menu widget -- maybe the menu should be created elseware?*/
  sys_vgui("set %xw .x%x.c.s%x ; menubutton $%xw -relief raised -background \"%s\" -text \"%s\" -direction flush -menu $%xw.menu ; menu $%xw.menu -tearoff 0\n",
		x,canvas,x,x,x->x_colour->s_name,x->x_name->s_name,x,x);

  for(i=0 ; i<x->x_num_options ; i++)
  {
	sys_vgui(".x%x.c.s%x.menu add command -label \"%s\" -command {.x%x.c.s%x configure -text \"%s\" ; popup_sel%x \"%d\"} \n", 
		canvas, x, x->x_options[i]->s_name, canvas, x, x->x_options[i]->s_name, x, i);
  }
  
  DEBUG(post("id: .x%x.c.s%x", canvas, x);)
}

static void popup_drawme(t_popup *x, t_glist *glist, int firsttime)
{
  t_canvas *canvas=glist_getcanvas(glist);
  DEBUG(post("drawme %d",firsttime);)
     if (firsttime) {
       DEBUG(post("glist %x canvas %x",x->x_glist,canvas);)
       create_widget(x,glist);	       
       x->x_glist = canvas;
       sys_vgui(".x%x.c create window %d %d -width %d -height 25 -anchor nw -window .x%x.c.s%x -tags %xS\n", 
		canvas,text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist), x->x_width, x->x_glist,x,x);
              
     }     
     else {
       sys_vgui(".x%x.c coords %xS \
%d %d\n",
		canvas, x,
		text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist));
     }
     draw_inlets(x, glist, firsttime, 2,1);
     //     draw_handle(x, glist, firsttime);

}


static void popup_erase(t_popup* x,t_glist* glist)
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
	


/* ------------------------ popup widgetbehaviour----------------------------- */


static void popup_getrect(t_gobj *z, t_glist *owner,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    int width, height;
    t_popup* s = (t_popup*)z;

    width = s->x_rect_width;
    height = s->x_rect_height;
    *xp1 = text_xpix(&s->x_obj, owner);
    *yp1 = text_ypix(&s->x_obj, owner) - 1;
    *xp2 = text_xpix(&s->x_obj, owner) + width;
    *yp2 = text_ypix(&s->x_obj, owner) + height;
}

static void popup_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_popup *x = (t_popup *)z;
    DEBUG(post("displace");)
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
}

static void popup_activate(t_gobj *z, t_glist *glist, int state)
{
/* What does this do, why commented out? 
    t_text *x = (t_text *)z;
    t_rtext *y = glist_findrtext(glist, x);
    if (z->g_pd != gatom_class) rtext_activate(y, state);*/
}

static void popup_delete(t_gobj *z, t_glist *glist)
{
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist_getcanvas(glist), x);
}

       
static void popup_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_popup* s = (t_popup*)z;
    t_rtext *y;
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
  outlet_symbol(x->out2, x->x_options[(int)popup_index]);
  outlet_float(x->x_obj.ob_outlet, popup_index); 
}


static void popup_save(t_gobj *z, t_binbuf *b)
{
	int i;
    t_popup *x = (t_popup *)z;

    binbuf_addv(b, "ssiisiss", gensym("#X"),gensym("obj"),
		x->x_obj.te_xpix, x->x_obj.te_ypix ,  
		gensym("popup"), x->x_width, x->x_colour, x->x_name);
	/* Loop for menu items */
	for(i=0 ; i<x->x_num_options ; i++)
	{
		binbuf_addv(b, "s", x->x_options[i]);
	}
    binbuf_addv(b, ";");
}

/* function to change the popup's menu */
void popup_options(t_popup* x, t_symbol *s, int argc, t_atom *argv)
{
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
}

/* function to change colour of popup background */
void popup_bgcolour(t_popup* x, t_symbol* col)
{
	x->x_colour->s_name = col->s_name;
	sys_vgui(".x%x.c.s%x configure -background \"%s\"\n", x->x_glist, x, col->s_name);
}

/* Function to change name of popup */
static void popup_name(t_popup* x, t_symbol *name)
{
	x->x_name->s_name = name->s_name;
	sys_vgui(".x%x.c.s%x configure -text \"%s\"\n", x->x_glist, x, name->s_name);
}

/* Function to select a menu option by inlet */
static void popup_iselect(t_popup* x, t_floatarg item)
{
	int i=(int)item;
	if( i<x->x_num_options && i>=0)
	{
		sys_vgui(".x%x.c.s%x configure -text \"%s\" ; popup_sel%x \"%d\" \n",
			glist_getcanvas(x->x_glist), x, x->x_options[i]->s_name,x, i);
	} else {
		post("popup: Valid menu selections are from %d to %d\npopup: You entered %d.", 0, x->x_num_options, i);
	}
}

static t_class *popup_class;


static void *popup_new(t_symbol *s, int argc, t_atom *argv)
{
    t_popup *x = (t_popup *)pd_new(popup_class);
    int i;
	char buf[256];

    x->x_glist = (t_glist*)NULL;

    
    x->x_height = 25;
	
	if (argc < 4)
	{
		post("popup: You must enter at least 4 arguments. Default values used.");
		x->x_width = 124;
		x->x_num_options = 1; 
		x->x_colour = gensym("#ffffff");
		x->x_name = gensym("popup");
		
		x->x_options[0] = gensym("option");
		
	} else {
		/* Copy args into structure */
		x->x_width = atom_getint(argv);
		x->x_colour = atom_getsymbol(argv+1);
		x->x_name = atom_getsymbol(argv+2);
		
		x->x_num_options = argc-3;
		
		for(i=0 ; i<x->x_num_options ; i++)
		{
			x->x_options[i] = atom_getsymbol( argv+(i+3) );
		}
	}	

	/* Bind the recieve "popup%p" to the widget outlet*/
    sprintf(buf,"popup%p",x);
    x->x_sym = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->x_sym);

	/* define proc in tcl/tk where "popup%p" is the receive, "output" is the method, and "$index" is an argument. */
    sys_vgui("proc popup_sel%x {index} {\n pd [concat popup%p output $index \\;]\n }\n",x,x); 

    outlet_new(&x->x_obj, &s_float);
	x->out2 = outlet_new(&x->x_obj, &s_symbol);
    return (x);
}

void popup_setup(void) {
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
	class_doaddfloat(popup_class, (t_method)popup_iselect);

    class_setwidget(popup_class,&popup_widgetbehavior);
#if PD_MINOR_VERSION >= 37
    class_setsavefn(popup_class,&popup_save);
#endif

	post("Popup v0 Ben Bogart.");
}


