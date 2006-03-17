/* (C) Guenter Geiger <geiger@xdv.org> */


#include <m_pd.h>
#include "g_canvas.h"

/* ------------------------ cursor ----------------------------- */


#define BACKGROUNDCOLOR "grey"

#define DEFAULTSIZE 80

static t_class *cursor_class;

typedef struct _cursor
{
     t_object x_obj;
     t_glist * x_glist;
     t_outlet* out2;
     int x_width;
     int x_height;
     int x;
     int y;
} t_cursor;

/* widget helper functions */

void cursor_drawme(t_cursor *x, t_glist *glist, int firsttime)
{
     if (firsttime) {
	  sys_vgui(".x%x.c create rectangle %d %d %d %d -tags %xS -fill %s\n",
		   glist_getcanvas(glist),
		   x->x_obj.te_xpix, x->x_obj.te_ypix,
		   x->x_obj.te_xpix + x->x_width, x->x_obj.te_ypix + x->x_height,
		   x,BACKGROUNDCOLOR);
     }     
     else {
	  sys_vgui(".x%x.c coords %xS \
%d %d %d %d\n",
		   glist_getcanvas(glist), x,
		   x->x_obj.te_xpix, x->x_obj.te_ypix,
		   x->x_obj.te_xpix + x->x_width, x->x_obj.te_ypix + x->x_height);
     }

     {
       /* outlets */
	  int n = 2;
	  int nplus, i;
	  nplus = (n == 1 ? 1 : n-1);
	  for (i = 0; i < n; i++)
	  {
	       int onset = x->x_obj.te_xpix + (x->x_width - IOWIDTH) * i / nplus;
	       if (firsttime)
		    sys_vgui(".x%x.c create rectangle %d %d %d %d -tags %xo%d\n",
			     glist_getcanvas(glist),
			     onset, x->x_obj.te_ypix + x->x_height - 1,
			     onset + IOWIDTH, x->x_obj.te_ypix + x->x_height,
			     x, i);
	       else
		    sys_vgui(".x%x.c coords %xo%d %d %d %d %d\n",
			     glist_getcanvas(glist), x, i,
			     onset, x->x_obj.te_ypix + x->x_height - 1,
			     onset + IOWIDTH, x->x_obj.te_ypix + x->x_height);
	  }
	  /* inlets */
	  n = 0; 
	  nplus = (n == 1 ? 1 : n-1);
	  for (i = 0; i < n; i++)
	  {
	       int onset = x->x_obj.te_xpix + (x->x_width - IOWIDTH) * i / nplus;
	       if (firsttime)
		    sys_vgui(".x%x.c create rectangle %d %d %d %d -tags %xi%d\n",
			     glist_getcanvas(glist),
			     onset, x->x_obj.te_ypix,
			     onset + IOWIDTH, x->x_obj.te_ypix + 1,
			     x, i);
	       else
		    sys_vgui(".x%x.c coords %xi%d %d %d %d %d\n",
			     glist_getcanvas(glist), x, i,
			     onset, x->x_obj.te_ypix,
			     onset + IOWIDTH, x->x_obj.te_ypix + 1);
	       
	  }
     }

}




void cursor_erase(t_cursor* x,t_glist* glist)
{
     int n;
     sys_vgui(".x%x.c delete %xS\n",
	      glist_getcanvas(glist), x);
     n = 2;
     while (n--) {
	  sys_vgui(".x%x.c delete %xo%d\n",glist_getcanvas(glist),x,n);
     }
}
	


/* ------------------------ cursor widgetbehaviour----------------------------- */


static void cursor_getrect(t_gobj *z, t_glist *owner,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
    int width, height;
    t_cursor* s = (t_cursor*)z;


    width = s->x_width;
    height = s->x_height;
    *xp1 = s->x_obj.te_xpix;
    *yp1 = s->x_obj.te_ypix;
    *xp2 = s->x_obj.te_xpix + width;
    *yp2 = s->x_obj.te_ypix + height;
}

static void cursor_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_cursor *x = (t_cursor *)z;
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    cursor_drawme(x, glist, 0);
    canvas_fixlinesfor(glist_getcanvas(glist),(t_text*) x);
}

static void cursor_select(t_gobj *z, t_glist *glist, int state)
{
     t_cursor *x = (t_cursor *)z;
     sys_vgui(".x%x.c itemconfigure %xS -fill %s\n", glist, 
	     x, (state? "blue" : BACKGROUNDCOLOR));
}

static void cursor_motion(t_cursor *x, t_floatarg dx, t_floatarg dy)
{
  x->x += dx;
  x->y += dy;
  outlet_float(x->out2,x->y);
  outlet_float(x->x_obj.ob_outlet,x->x);
}

static void cursor_start(t_cursor *x)
{
	glist_grab(x->x_glist, &x->x_obj.te_g, (t_glistmotionfn) cursor_motion,
			   (t_glistkeyfn) NULL, NULL, NULL);
	
/*     x->x = xpos - x->x_obj.te_xpix; */
/*     x->y = ypos - x->x_obj.te_ypix; */
/*     outlet_float(x->out2,x->y); */
/*     outlet_float(x->x_obj.ob_outlet,x->x); */
}

static void cursor_activate(t_gobj *z, t_glist *glist, int state)
{
/*    t_text *x = (t_text *)z;
    t_rtext *y = glist_findrtext(glist, x);
    if (z->g_pd != gatom_class) rtext_activate(y, state);*/
	t_cursor *x = (t_cursor *) z;
	cursor_start(x);
}

static void cursor_delete(t_gobj *z, t_glist *glist)
{
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist_getcanvas(glist), x);
}

       
static void cursor_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_cursor* s = (t_cursor*)z;
    if (vis)
	 cursor_drawme(s, glist, 1);
    else
	 cursor_erase(s,glist);
}

/* can we use the normal text save function ?? */

static void cursor_save(t_gobj *z, t_binbuf *b)
{
    t_cursor *x = (t_cursor *)z;
    binbuf_addv(b, "ssiisii", gensym("#X"),gensym("obj"),
		(t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,  
		gensym("cursor"),x->x_width,x->x_height);
    binbuf_addv(b, ";");
}


t_widgetbehavior   cursor_widgetbehavior;

void cursor_key(t_cursor *x, t_floatarg f)
{
  post("key");
}



static void cursor_click(t_cursor *x,
    t_floatarg xpos, t_floatarg ypos, t_floatarg shift, t_floatarg ctrl,
    t_floatarg alt)
{
    glist_grab(x->x_glist, &x->x_obj.te_g, (t_glistmotionfn) cursor_motion,
		(t_glistkeyfn) NULL, xpos, ypos);

    x->x = xpos - x->x_obj.te_xpix;
    x->y = ypos - x->x_obj.te_ypix;
    outlet_float(x->out2,x->y);
    outlet_float(x->x_obj.ob_outlet,x->x);
}

static int cursor_newclick(t_gobj *z, struct _glist *glist,
    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    	if (doit)
	    cursor_click((t_cursor *)z, (t_floatarg)xpix, (t_floatarg)ypix,
	    	(t_floatarg)shift, 0, (t_floatarg)alt);
	return (1);
}

void cursor_size(t_cursor* x,t_floatarg w,t_floatarg h) {
     x->x_width = w;
     x->x_height = h;
     cursor_drawme(x, x->x_glist, 0);
}

static void cursor_setwidget(void)
{
    cursor_widgetbehavior.w_getrectfn =     cursor_getrect;
    cursor_widgetbehavior.w_displacefn =    cursor_displace;
    cursor_widgetbehavior.w_selectfn =   cursor_select;
    cursor_widgetbehavior.w_activatefn =   cursor_activate;
    cursor_widgetbehavior.w_deletefn =   cursor_delete;
    cursor_widgetbehavior.w_visfn =   cursor_vis;
    cursor_widgetbehavior.w_clickfn = cursor_newclick;
}


static void *cursor_new(t_floatarg h,t_floatarg o)
{
    t_cursor *x = (t_cursor *)pd_new(cursor_class);

    x->x_glist = (t_glist*) canvas_getcurrent();
    if (h) x->x_width = h;
    else
	 x->x_width = DEFAULTSIZE;

    if (o) x->x_height = o;
    else
	 x->x_height = DEFAULTSIZE;

    outlet_new(&x->x_obj, &s_float);
    x->out2 = outlet_new(&x->x_obj, &s_float);
	
    return (x);
}

void cursor_setup(void)
{
    cursor_class = class_new(gensym("cursor"), (t_newmethod)cursor_new, 0,
				sizeof(t_cursor),0, A_DEFFLOAT,A_DEFFLOAT,0);

    class_addcreator((t_newmethod)cursor_new,gensym("bng"),A_DEFSYM,A_DEFFLOAT,A_DEFFLOAT,A_GIMME,0);

    class_addmethod(cursor_class, (t_method)cursor_click, gensym("click"),
    	A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(cursor_class, (t_method)cursor_size, gensym("size"),
    	A_FLOAT, A_FLOAT, 0);

	class_addmethod(cursor_class,(t_method) cursor_start,gensym("start"),0);
/* 	class_addfloat(cursor_class,(t_method) cursor_float); */

    cursor_setwidget();
    class_setwidget(cursor_class,&cursor_widgetbehavior);
#if PD_MINOR_VERSION >= 37
    class_setsavefn(cursor_class,&cursor_save);
#endif
}


