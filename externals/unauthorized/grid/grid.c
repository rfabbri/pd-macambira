/* Copyright (c) 1997-1999 Miller Puckette.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution. */

/* g_grid.c written by Yves Degoyon 2002                                       */
/* grid control object : two dimensionnal grid                                 */
/* thanks to Thomas Musil, Miller Puckette, Guenther Geiger and Krzystof Czaja */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <m_pd.h>
#include "m_imp.h"
#include "g_canvas.h"
#include "t_tk.h"
#include "g_grid.h"

#ifdef NT
#include <io.h>
#else
#include <unistd.h>
#endif

#define DEFAULT_GRID_WIDTH 200
#define DEFAULT_GRID_HEIGHT 200
#define DEFAULT_GRID_NBLINES 10

t_widgetbehavior grid_widgetbehavior;
static t_class *grid_class;
static int gridcount=0;

static int guidebug=0;
static int pointsize = 5;

static char   *grid_version = "grid: version 0.4, written by Yves Degoyon (ydegoyon@free.fr)";

#define GRID_SYS_VGUI2(a,b) if (guidebug) \
                         post(a,b);\
                         sys_vgui(a,b)

#define GRID_SYS_VGUI3(a,b,c) if (guidebug) \
                         post(a,b,c);\
                         sys_vgui(a,b,c)

#define GRID_SYS_VGUI4(a,b,c,d) if (guidebug) \
                         post(a,b,c,d);\
                         sys_vgui(a,b,c,d)

#define GRID_SYS_VGUI7(a,b,c,d,e,f,g) if (guidebug) \
                         post(a,b,c,d,e,f,g );\
                         sys_vgui(a,b,c,d,e,f,g)

#define GRID_SYS_VGUI9(a,b,c,d,e,f,g,h,i) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i );\
                         sys_vgui(a,b,c,d,e,f,g,h,i)

/* drawing functions */
static void grid_draw_update(t_grid *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    t_int xpoint=x->x_current, ypoint=x->y_current;

    // later : try to figure out what's this test for ??  
    // if (glist_isvisible(glist))
    // {
       // delete previous point if existing
       if (x->x_point)  
       {
          GRID_SYS_VGUI3(".x%x.c delete %xPOINT\n", canvas, x);
       }
        
       if ( x->x_current < x->x_obj.te_xpix ) xpoint = x->x_obj.te_xpix;
       if ( x->x_current > x->x_obj.te_xpix + x->x_width - pointsize ) 
			xpoint = x->x_obj.te_xpix + x->x_width - pointsize;
       if ( x->y_current < x->x_obj.te_ypix ) ypoint = x->x_obj.te_ypix;
       if ( x->y_current > x->x_obj.te_ypix + x->x_height - pointsize ) 
			ypoint = x->x_obj.te_ypix + x->x_height - pointsize;
       // draw the selected point
       GRID_SYS_VGUI7(".x%x.c create rectangle %d %d %d %d -fill #FF0000 -tags %xPOINT\n",
	     canvas, xpoint, ypoint, xpoint+pointsize, ypoint+pointsize, x);
       x->x_point = 1;
    // }  
    // else 
    // {
    //    post( "grid : position updated in an invisible grid" );
    // }
}

static void grid_draw_new(t_grid *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    char *tagRoot;

/* rtext_new(); in g_canvas.h changed in 0.37 */
#if PD_MINOR_VERSION >= 37
	 rtext_new(glist, (t_text *)x);
#else
	 rtext_new(glist, (t_text *)x, glist->gl_editor->e_rtext, 0);
#endif
    tagRoot = rtext_gettag(glist_findrtext(glist,(t_text *)x));
    GRID_SYS_VGUI7(".x%x.c create rectangle %d %d %d %d -fill #124392 -tags %xGRID\n",
	     canvas, x->x_obj.te_xpix, x->x_obj.te_ypix,
	     x->x_obj.te_xpix + x->x_width, x->x_obj.te_ypix + x->x_height,
	     x);
    GRID_SYS_VGUI7(".x%x.c create rectangle %d %d %d %d -tags %so0\n",
	     canvas, x->x_obj.te_xpix, x->x_obj.te_ypix + x->x_height+1,
	     x->x_obj.te_xpix+7, x->x_obj.te_ypix + x->x_height+2,
	     tagRoot);
    GRID_SYS_VGUI7(".x%x.c create rectangle %d %d %d %d -tags %so1\n",
	     canvas, x->x_obj.te_xpix+x->x_width-7, x->x_obj.te_ypix + x->x_height+1,
	     x->x_obj.te_xpix+x->x_width, x->x_obj.te_ypix + x->x_height+2,
	     tagRoot);

    if ( x->x_grid ) 
    {
       int xlpos = x->x_obj.te_xpix+x->x_width/x->x_xlines;
       int ylpos = x->x_obj.te_ypix+x->x_height/x->x_ylines;
       int xcount = 1;
       int ycount = 1;
       while ( xlpos < x->x_obj.te_xpix+x->x_width )
       {
         GRID_SYS_VGUI9(".x%x.c create line %d %d %d %d -fill #FFFFFF -tags %xLINE%d%d\n",
	     canvas, xlpos, x->x_obj.te_ypix,
	     xlpos, x->x_obj.te_ypix+x->x_height,
	     x, xcount, 0 );
         xlpos+=x->x_width/x->x_xlines;
         xcount++;
       }
       while ( ylpos < x->x_obj.te_ypix+x->x_height )
       {
         GRID_SYS_VGUI9(".x%x.c create line %d %d %d %d -fill #FFFFFF -tags %xLINE%d%d\n",
	     canvas, x->x_obj.te_xpix, ylpos,
	     x->x_obj.te_xpix+x->x_width, ylpos,
	     x, 0, ycount);
         ylpos+=x->x_height/x->x_ylines;
         ycount++;
       }
    }
    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void grid_draw_move(t_grid *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    char *tagRoot;

    tagRoot = rtext_gettag(glist_findrtext(glist,(t_text *)x)); 
    GRID_SYS_VGUI7(".x%x.c coords %xGRID %d %d %d %d\n",
	     canvas, x,
	     x->x_obj.te_xpix, x->x_obj.te_ypix,
	     x->x_obj.te_xpix+x->x_width, x->x_obj.te_ypix+x->x_height);
    GRID_SYS_VGUI7(".x%x.c coords %so0 %d %d %d %d\n",
	     canvas, tagRoot,
	     x->x_obj.te_xpix, x->x_obj.te_ypix + x->x_height+1,
	     x->x_obj.te_xpix+7, x->x_obj.te_ypix + x->x_height+2 );
    GRID_SYS_VGUI7(".x%x.c coords %so1 %d %d %d %d\n",
	     canvas, tagRoot,
	     x->x_obj.te_xpix+x->x_width-7, x->x_obj.te_ypix + x->x_height+1,
	     x->x_obj.te_xpix+x->x_width, x->x_obj.te_ypix + x->x_height+2 );
    if ( x->x_point ) 
    {
       grid_draw_update(x, glist);
    }
    if ( x->x_grid ) 
    {
       int xlpos = x->x_obj.te_xpix+x->x_width/x->x_xlines;
       int ylpos = x->x_obj.te_ypix+x->x_height/x->x_ylines;
       int xcount = 1;
       int ycount = 1;
       while ( xlpos < x->x_obj.te_xpix+x->x_width )
       {
         GRID_SYS_VGUI9(".x%x.c coords %xLINE%d%d %d %d %d %d\n",
	     canvas, x, xcount, 0, xlpos, x->x_obj.te_ypix,
	     xlpos, x->x_obj.te_ypix + x->x_height);
         xlpos+=x->x_width/x->x_xlines;
         xcount++;
       }
       while ( ylpos < x->x_obj.te_ypix+x->x_height )
       {
         GRID_SYS_VGUI9(".x%x.c coords %xLINE%d%d %d %d %d %d\n",
	     canvas, x, 0, ycount, x->x_obj.te_xpix, ylpos,
	     x->x_obj.te_xpix + x->x_width, ylpos);
         ylpos+=x->x_height/x->x_ylines;
         ycount++;
       }
    }
    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void grid_draw_erase(t_grid* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int i;
    char *tagRoot;

    tagRoot = rtext_gettag(glist_findrtext(glist,(t_text *)x));
    GRID_SYS_VGUI3(".x%x.c delete %xGRID\n", canvas, x);
    GRID_SYS_VGUI3(".x%x.c delete %so0\n", canvas, tagRoot);
    GRID_SYS_VGUI3(".x%x.c delete %so1\n", canvas, tagRoot);
    if (x->x_grid)  
    {
       for (i=1; i<x->x_xlines; i++ )
       {
           GRID_SYS_VGUI4(".x%x.c delete %xLINE%d0\n", canvas, x, i);
       }
       for (i=1; i<x->x_ylines; i++ )
       {
           GRID_SYS_VGUI4(".x%x.c delete %xLINE0%d\n", canvas, x, i);
       }
    }
    if (x->x_point)  
    {
          GRID_SYS_VGUI3(".x%x.c delete %xPOINT\n", canvas, x);
          x->x_point = 0;
    }
    rtext_free(glist_findrtext(glist, (t_text *)x));
}

static void grid_draw_select(t_grid* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if(x->x_selected)
    {
	pd_bind(&x->x_obj.ob_pd, x->x_name);
        /* sets the item in blue */
	GRID_SYS_VGUI3(".x%x.c itemconfigure %xGRID -outline #0000FF\n", canvas, x);
    }
    else
    {
	pd_unbind(&x->x_obj.ob_pd, x->x_name);
	GRID_SYS_VGUI3(".x%x.c itemconfigure %xGRID -outline #000000\n", canvas, x);
    }
}

static void grid_output_current(t_grid* x)
{
  t_float xvalue, yvalue;
  t_float xmodstep, ymodstep;

    xvalue = x->x_min + (x->x_current - x->x_obj.te_xpix) * (x->x_max-x->x_min) / x->x_width ;
    if (xvalue < x->x_min ) xvalue = x->x_min;
    if (xvalue > x->x_max ) xvalue = x->x_max;
    xmodstep = ((float)((int)(xvalue*10000) % (int)(x->x_xstep*10000))/10000.);
    xvalue = xvalue - xmodstep;
    outlet_float( x->x_xoutlet, xvalue );

    yvalue = x->y_max - (x->y_current - x->x_obj.te_ypix ) * (x->y_max-x->y_min) / x->x_height ;
    if (yvalue < x->y_min ) yvalue = x->y_min;
    if (yvalue > x->y_max ) yvalue = x->y_max;
    ymodstep = ((float)((int)(yvalue*10000) % (int)(x->x_ystep*10000))/10000.);
    yvalue = yvalue - ymodstep;
    outlet_float( x->x_youtlet, yvalue );
}

/* ------------------------ grid widgetbehaviour----------------------------- */


static void grid_getrect(t_gobj *z, t_glist *owner,
			    int *xp1, int *yp1, int *xp2, int *yp2)
{
   t_grid* x = (t_grid*)z;

   *xp1 = x->x_obj.te_xpix;
   *yp1 = x->x_obj.te_ypix;
   *xp2 = x->x_obj.te_xpix+x->x_width;
   *yp2 = x->x_obj.te_ypix+x->x_height;
}

static void grid_save(t_gobj *z, t_binbuf *b)
{
   t_grid *x = (t_grid *)z;

   // post( "saving grid : %s", x->x_name->s_name );
   binbuf_addv(b, "ssiissiffiffiffiiff", gensym("#X"),gensym("obj"),
		(t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,
		gensym("grid"), x->x_name, x->x_width, x->x_min,
		x->x_max, x->x_height,
                x->y_min, x->y_max,
                x->x_grid, x->x_xstep, 
                x->x_ystep, x->x_xlines, x->x_ylines, 
                x->x_current, x->y_current );
   binbuf_addv(b, ";");
}

static void grid_properties(t_gobj *z, t_glist *owner)
{
   char buf[800];
   t_grid *x=(t_grid *)z;

   sprintf(buf, "pdtk_grid_dialog %%s %s %d %.2f %.2f %d %.2f %.2f %d %.2f %.2f %d %d\n",
            x->x_name->s_name, x->x_width, x->x_min, x->x_max, x->x_height, 
            x->y_min, x->y_max, x->x_grid, x->x_xstep, x->x_ystep,
            x->x_xlines, x->x_ylines );
   // post("grid_properties : %s", buf );
   gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

static void grid_select(t_gobj *z, t_glist *glist, int selected)
{
   t_grid *x = (t_grid *)z;

   x->x_selected = selected;
   grid_draw_select( x, glist );
}

static void grid_vis(t_gobj *z, t_glist *glist, int vis)
{
   t_grid *x = (t_grid *)z;

   if (vis)
   {
      grid_draw_new( x, glist );
      grid_draw_update( x, glist );
      grid_output_current(x);
   }
   else
   {
      grid_draw_erase( x, glist );
   }
}

static void grid_dialog(t_grid *x, t_symbol *s, int argc, t_atom *argv)
{
   if ( !x ) {
     post( "grid : error :tried to set properties on an unexisting object" );
   }
   if ( argc != 12 )
   {
      post( "grid : error in the number of arguments ( %d instead of 12 )", argc );
      return;
   }
      if ( argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT ||
        argv[2].a_type != A_FLOAT || argv[3].a_type != A_FLOAT ||
        argv[4].a_type != A_FLOAT || argv[5].a_type != A_FLOAT ||
        argv[6].a_type != A_FLOAT || argv[7].a_type != A_FLOAT || 
        argv[8].a_type != A_FLOAT || argv[9].a_type != A_FLOAT || 
        argv[10].a_type != A_FLOAT || argv[11].a_type != A_FLOAT ) {
      post( "grid : wrong arguments" );
      return;
   }
   x->x_name = argv[0].a_w.w_symbol;
   x->x_width = (int)argv[1].a_w.w_float;
   x->x_min = argv[2].a_w.w_float;
   x->x_max = argv[3].a_w.w_float;
   x->x_height = (int)argv[4].a_w.w_float;
   x->y_min = argv[5].a_w.w_float;
   x->y_max = argv[6].a_w.w_float;
   x->x_grid = argv[7].a_w.w_float;
   x->x_xstep = argv[8].a_w.w_float;
   x->x_ystep = argv[9].a_w.w_float;
   x->x_xlines = argv[10].a_w.w_float;
   x->x_ylines = argv[11].a_w.w_float;
   grid_draw_erase(x, x->x_glist);
   grid_draw_new(x, x->x_glist);
}

static void grid_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor( glist_getcanvas(glist), (t_text *)z);
}

static void grid_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_grid *x = (t_grid *)z;
    int xold = x->x_obj.te_xpix;
    int yold = x->x_obj.te_ypix;

    // post( "grid_displace dx=%d dy=%d", dx, dy );

    x->x_obj.te_xpix += dx;
    x->x_current += dx;
    x->x_obj.te_ypix += dy;
    x->y_current += dy;
    if(xold != x->x_obj.te_xpix || yold != x->x_obj.te_ypix)
    {
	grid_draw_move(x, x->x_glist);
    }
}

static void grid_motion(t_grid *x, t_floatarg dx, t_floatarg dy)
{
    int xold = x->x_current;
    int yold = x->y_current;

    // post( "grid_motion dx=%f dy=%f", dx, dy );

    x->x_current += dx;
    x->y_current += dy;
    if(xold != x->x_current || yold != x->y_current)
    {
        grid_output_current(x);
	grid_draw_update(x, x->x_glist);
    }
}

static int grid_click(t_gobj *z, struct _glist *glist,
			    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_grid* x = (t_grid *)z;

    // post( "grid_click doit=%d x=%d y=%d", doit, xpix, ypix );
    if ( doit) 
    {
      x->x_current = xpix;
      x->y_current = ypix;
      grid_output_current(x);
      grid_draw_update(x, glist);
      glist_grab(glist, &x->x_obj.te_g, (t_glistmotionfn)grid_motion,
	       0, xpix, ypix);
    }
    return (1);
}

static void grid_goto(t_grid *x, t_floatarg newx, t_floatarg newy)
{
    int xold = x->x_current;
    int yold = x->y_current;

    if ( newx > x->x_width-1 ) newx = x->x_width-1;
    if ( newx < 0 ) newx = 0;
    if ( newy > x->x_height-1 ) newy = x->x_height-1;
    if ( newy < 0 ) newy = 0;

    // post( "grid_set x=%f y=%f", newx, newy );

    x->x_current = newx + x->x_obj.te_xpix;
    x->y_current = newy + x->x_obj.te_ypix;
    if(xold != x->x_current || yold != x->y_current)
    {
        grid_output_current(x);
        grid_draw_update(x, x->x_glist);
    }
}

static void grid_values(t_grid* x, t_floatarg xvalue, t_floatarg yvalue)
{
  int xold = x->x_current;
  int yold = x->y_current;

  if (xvalue < x->x_min ) xvalue = x->x_min;
  if (xvalue > x->x_max ) xvalue = x->x_max;

  x->x_current = x->x_obj.te_xpix + ((xvalue - x->x_min) / x->x_max) * x->x_width;

  if (yvalue < x->y_min ) yvalue = x->y_min;
  if (yvalue > x->y_max ) yvalue = x->y_max;

  x->y_current =  x->x_obj.te_ypix + (1 - ((yvalue - x->y_min) / x->y_max)) * x->x_height;

  if(xold != x->x_current || yold != x->y_current) {
        grid_output_current(x);
        grid_draw_update(x, x->x_glist);
  }
}

static void grid_xvalues(t_grid* x, t_floatarg xvalue, t_floatarg yvalue)
{
  int xold = x->x_current;
  int yold = x->y_current;

  if (xvalue < x->x_min ) xvalue = x->x_min;
  if (xvalue > x->x_max ) xvalue = x->x_max;

  x->x_current = x->x_obj.te_xpix + ((xvalue - x->x_min) / x->x_max) * x->x_width;

  if (yvalue < x->y_min ) yvalue = x->y_min;
  if (yvalue > x->y_max ) yvalue = x->y_max;

  x->y_current =  x->x_obj.te_ypix + (1 - ((yvalue - x->y_min) / x->y_max)) * x->x_height;

  if(xold != x->x_current || yold != x->y_current) {
        grid_draw_update(x, x->x_glist);
  }
}

static void grid_valuemotion(t_grid* x, t_floatarg dx, t_floatarg dy)
{
  int xold = x->x_current;
  int yold = x->y_current;
  t_float xvalue, yvalue;

  xvalue = x->x_min + (x->x_current - x->x_obj.te_xpix) * (x->x_max-x->x_min) / x->x_width ;
  if (xvalue < x->x_min ) xvalue = x->x_min;
  if (xvalue > x->x_max ) xvalue = x->x_max;

  yvalue = x->y_max - (x->y_current - x->x_obj.te_ypix ) * (x->y_max-x->y_min) / x->x_height ;
  if (yvalue < x->y_min ) yvalue = x->y_min;
  if (yvalue > x->y_max ) yvalue = x->y_max;

  xvalue += dx;
  yvalue += dy;

  if (xvalue < x->x_min ) xvalue = x->x_min;
  if (xvalue > x->x_max ) xvalue = x->x_max;

  x->x_current = x->x_obj.te_xpix + ((xvalue - x->x_min) / x->x_max) * x->x_width;

  if (yvalue < x->y_min ) yvalue = x->y_min;
  if (yvalue > x->y_max ) yvalue = x->y_max;

  x->y_current =  x->x_obj.te_ypix + (1 - ((yvalue - x->y_min) / x->y_max)) * x->x_height;

  if(xold != x->x_current || yold != x->y_current) {
        grid_output_current(x);
        grid_draw_update(x, x->x_glist);
  }
}

static void grid_xvaluemotion(t_grid* x, t_floatarg dx, t_floatarg dy)
{
  int xold = x->x_current;
  int yold = x->y_current;
  t_float xvalue, yvalue;

  xvalue = x->x_min + (x->x_current - x->x_obj.te_xpix) * (x->x_max-x->x_min) / x->x_width ;
  if (xvalue < x->x_min ) xvalue = x->x_min;
  if (xvalue > x->x_max ) xvalue = x->x_max;

  yvalue = x->y_max - (x->y_current - x->x_obj.te_ypix ) * (x->y_max-x->y_min) / x->x_height ;
  if (yvalue < x->y_min ) yvalue = x->y_min;
  if (yvalue > x->y_max ) yvalue = x->y_max;

  xvalue += dx;
  yvalue += dy;

  if (xvalue < x->x_min ) xvalue = x->x_min;
  if (xvalue > x->x_max ) xvalue = x->x_max;

  x->x_current = x->x_obj.te_xpix + ((xvalue - x->x_min) / x->x_max) * x->x_width;

  if (yvalue < x->y_min ) yvalue = x->y_min;
  if (yvalue > x->y_max ) yvalue = x->y_max;

  x->y_current =  x->x_obj.te_ypix + (1 - ((yvalue - x->y_min) / x->y_max)) * x->x_height;

  if(xold != x->x_current || yold != x->y_current) {
        grid_draw_update(x, x->x_glist);
  }
}

static void grid_bang(t_grid *x) {
  grid_output_current(x);
}

static t_grid *grid_new(t_symbol *s, int argc, t_atom *argv)
{
    int i, zz;
    t_grid *x;
    t_pd *x2;
    char *str;
 
    // post( "grid_new : create : %s argc =%d", s->s_name, argc );

    x = (t_grid *)pd_new(grid_class);
    // new grid created from the gui 
    if ( argc != 0 )
    {
      if ( argc != 14 )
      {
        post( "grid : error in the number of arguments ( %d instead of 14 )", argc );
        return NULL;
      }
      if ( argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT ||
        argv[2].a_type != A_FLOAT || argv[3].a_type != A_FLOAT ||
        argv[4].a_type != A_FLOAT || argv[5].a_type != A_FLOAT ||
        argv[6].a_type != A_FLOAT || argv[7].a_type != A_FLOAT || 
        argv[8].a_type != A_FLOAT || argv[9].a_type != A_FLOAT || 
        argv[10].a_type != A_FLOAT || argv[11].a_type != A_FLOAT || 
        argv[12].a_type != A_FLOAT || argv[13].a_type != A_FLOAT ) {
        post( "grid : wrong arguments" );
        return NULL;
      }

      // update grid count
      if (!strncmp((str = argv[0].a_w.w_symbol->s_name), "grid", 5)
    	 && (zz = atoi(str + 5)) > gridcount) 
      {
        gridcount = zz;
      }
      x->x_name = argv[0].a_w.w_symbol;
      pd_bind(&x->x_obj.ob_pd, x->x_name);
      x->x_width = argv[1].a_w.w_float;
      x->x_min = argv[2].a_w.w_float;
      x->x_max = argv[3].a_w.w_float;
      x->x_height = argv[4].a_w.w_float;
      x->y_min = argv[5].a_w.w_float;
      x->y_max = argv[6].a_w.w_float;
      x->x_grid = argv[7].a_w.w_float;
      x->x_xstep = argv[8].a_w.w_float;
      x->x_ystep = argv[9].a_w.w_float;
      x->x_xlines = argv[10].a_w.w_float;
      x->x_ylines = argv[11].a_w.w_float;
      x->x_current = argv[12].a_w.w_float;
      x->y_current = argv[13].a_w.w_float;
      x->x_point = 1;
    }
    else
    {
      char buf[40];

      sprintf(buf, "grid%d", ++gridcount);
      s = gensym(buf);    	

      x->x_name = s;
      pd_bind(&x->x_obj.ob_pd, x->x_name);

      x->x_width = DEFAULT_GRID_WIDTH;
      x->x_min = 0;
      x->x_max = DEFAULT_GRID_WIDTH - 1;
      x->x_height = DEFAULT_GRID_HEIGHT;
      x->y_min = 0;
      x->y_max = DEFAULT_GRID_HEIGHT - 1;
      x->x_grid = 1;	
      x->x_xstep = 1.0;	
      x->x_ystep = 1.0;	
      x->x_xlines = DEFAULT_GRID_NBLINES;	
      x->x_ylines = DEFAULT_GRID_NBLINES;	
      x->x_current = 0;
      x->y_current = 0;	

    }

    // common fields for new and restored grids
    x->x_point = 0;	
    x->x_selected = 0;	
    x->x_glist = (t_glist *) canvas_getcurrent();
    x->x_xoutlet = outlet_new(&x->x_obj, &s_float ); 
    x->x_youtlet = outlet_new(&x->x_obj, &s_float ); 

    // post( "grid_new name : %s width: %d height : %d", x->x_name->s_name, x->x_width, x->x_height );

    return (x);
}

static void grid_free(t_grid *x)
{
    post( "grid~: freeing ressources [NULL]" );
}

void grid_setup(void)
{
#include "grid.tk2c"
    post ( grid_version );
    grid_class = class_new(gensym("grid"), (t_newmethod)grid_new,
			      (t_method)grid_free, sizeof(t_grid), 0, A_GIMME, 0);
    class_addmethod(grid_class, (t_method)grid_click, gensym("click"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(grid_class, (t_method)grid_motion, gensym("motion"),
		    A_FLOAT, A_FLOAT, 0);
    class_addmethod(grid_class, (t_method)grid_bang, gensym("bang"), 0);
    class_addmethod(grid_class, (t_method)grid_values, gensym("values"),
                    A_FLOAT, A_FLOAT, 0);
    class_addmethod(grid_class, (t_method)grid_valuemotion, gensym("valuemotion"),
                    A_FLOAT, A_FLOAT, 0);
    class_addmethod(grid_class, (t_method)grid_xvalues, gensym("xvalues"),
                    A_FLOAT, A_FLOAT, 0);
    class_addmethod(grid_class, (t_method)grid_xvaluemotion, gensym("xvaluemotion"),
                    A_FLOAT, A_FLOAT, 0);
    class_addmethod(grid_class, (t_method)grid_goto, gensym("goto"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(grid_class, (t_method)grid_dialog, gensym("dialog"), A_GIMME, 0);
    grid_widgetbehavior.w_getrectfn =    grid_getrect;
    grid_widgetbehavior.w_displacefn =   grid_displace;
    grid_widgetbehavior.w_selectfn =     grid_select;
    grid_widgetbehavior.w_activatefn =   NULL;
    grid_widgetbehavior.w_deletefn =     grid_delete;
    grid_widgetbehavior.w_visfn =        grid_vis;
    grid_widgetbehavior.w_clickfn =      grid_click;
	 /* 
	  * <hans@eds.org>: As of 0.37, pd does not have these last 
	  * two elements in t_widgetbehavoir anymore.
	  * see pd/src/notes.txt:
	  *           savefunction and dialog into class structure
	  */
#if PD_MINOR_VERSION < 37  || !defined(PD_MINOR_VERSION)
    grid_widgetbehavior.w_propertiesfn = grid_properties;
    grid_widgetbehavior.w_savefn =       grid_save;
#else
	 class_setsavefn(grid_class, &grid_save);
	 class_setpropertiesfn(grid_class, &grid_properties);
#endif
    class_setwidget(grid_class, &grid_widgetbehavior);
    class_sethelpsymbol(grid_class, gensym("help-grid.pd"));
}
