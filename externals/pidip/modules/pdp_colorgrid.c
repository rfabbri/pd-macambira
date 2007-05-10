/* Copyright (c) 1997-1999 Miller Puckette.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution. */

/* g_pdp_colorgrid.c written by Yves Degoyon 2002                                       */
/* pdp_colorgrid control object : two dimensionnal pdp_colorgrid                                 */
/* thanks to Thomas Musil, Miller Puckette, Guenther Geiger and Krzystof Czaja */


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "m_pd.h"
#include "m_imp.h"
#include "g_canvas.h"
#include "t_tk.h"
#include "g_colorgrid.h"

#ifdef NT
#include <io.h>
#else
#include <unistd.h>
#endif

#define COLORGRID_IMG PWD"/patches/images/colorgrid.pnm"
#define DEFAULT_COLORGRID_WIDTH 256
#define DEFAULT_COLORGRID_HEIGHT 50
#define DEFAULT_COLORGRID_NBLINES 10

t_widgetbehavior pdp_colorgrid_widgetbehavior;
static t_class *pdp_colorgrid_class;
static int pdp_colorgridcount=0;

static int guidebug=0;
static int pointsize = 5;

static char   *pdp_colorgrid_version = "pdp_colorgrid: version 0.4, written by Yves Degoyon (ydegoyon@free.fr) & Lluis Gomez i Bigorda (lluis@artefacte.org)";

#define COLORGRID_SYS_VGUI2(a,b) if (guidebug) \
                         post(a,b);\
                         sys_vgui(a,b)

#define COLORGRID_SYS_VGUI3(a,b,c) if (guidebug) \
                         post(a,b,c);\
                         sys_vgui(a,b,c)

#define COLORGRID_SYS_VGUI4(a,b,c,d) if (guidebug) \
                         post(a,b,c,d);\
                         sys_vgui(a,b,c,d)

#define COLORGRID_SYS_VGUI5(a,b,c,d,e) if (guidebug) \
                         post(a,b,c,d,e);\
                         sys_vgui(a,b,c,d,e)

#define COLORGRID_SYS_VGUI6(a,b,c,d,e,f) if (guidebug) \
                         post(a,b,c,d,e,f);\
                         sys_vgui(a,b,c,d,e,f)

#define COLORGRID_SYS_VGUI7(a,b,c,d,e,f,g) if (guidebug) \
                         post(a,b,c,d,e,f,g );\
                         sys_vgui(a,b,c,d,e,f,g)

#define COLORGRID_SYS_VGUI8(a,b,c,d,e,f,g,h) if (guidebug) \
                         post(a,b,c,d,e,f,g,h );\
                         sys_vgui(a,b,c,d,e,f,g,h)

#define COLORGRID_SYS_VGUI9(a,b,c,d,e,f,g,h,i) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i );\
                         sys_vgui(a,b,c,d,e,f,g,h,i)

/* drawing functions */
static void pdp_colorgrid_draw_update(t_pdp_colorgrid *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int xpoint=x->x_current, ypoint=x->y_current;

    // later : try to figure out what's this test for ??  
    // if (glist_isvisible(glist))
    // {
       // delete previous point if existing
       if (x->x_point)  
       {
          COLORGRID_SYS_VGUI3(".x%x.c delete %xPOINT\n", canvas, x);
       }
        
       if ( x->x_current < x->x_obj.te_xpix ) xpoint = x->x_obj.te_xpix;
       if ( x->x_current > x->x_obj.te_xpix + x->x_width - pointsize ) 
			xpoint = x->x_obj.te_xpix + x->x_width - pointsize;
       if ( x->y_current < x->x_obj.te_ypix ) ypoint = x->x_obj.te_ypix;
       if ( x->y_current > x->x_obj.te_ypix + x->x_height - pointsize ) 
			ypoint = x->x_obj.te_ypix + x->x_height - pointsize;
       // draw the selected point
       COLORGRID_SYS_VGUI7(".x%x.c create rectangle %d %d %d %d -outline {} -fill #FF0000 -tags %xPOINT\n",
	     canvas, xpoint, ypoint, xpoint+5, ypoint+5, x);
       x->x_point = 1;
    // }  
    // else 
    // {
    //    post( "pdp_colorgrid : position updated in an invisible pdp_colorgrid" );
    // }
}

static void pdp_colorgrid_draw_new(t_pdp_colorgrid *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    char *tagRoot;
    char fname[MAXPDSTRING]=COLORGRID_IMG;
    rtext_new(glist, (t_text *)x);
    tagRoot = rtext_gettag(glist_findrtext(glist,(t_text *)x));

    COLORGRID_SYS_VGUI3("image create photo img%x -file %s\n",x,fname);
    
    COLORGRID_SYS_VGUI6(".x%x.c create image %d %d -image img%x -tags %sS\n", canvas,text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),x,tagRoot);
    COLORGRID_SYS_VGUI5(".x%x.c coords %sS %d %d \n",
	     canvas, tagRoot,
	     x->x_obj.te_xpix + 128, x->x_obj.te_ypix + 25);
				  
    COLORGRID_SYS_VGUI7(".x%x.c create rectangle %d %d %d %d -tags %so0\n",
	     canvas, x->x_obj.te_xpix, x->x_obj.te_ypix + x->x_height+1,
	     x->x_obj.te_xpix+7, x->x_obj.te_ypix + x->x_height+2,
	     tagRoot);
    COLORGRID_SYS_VGUI7(".x%x.c create rectangle %d %d %d %d -tags %so1\n",
	     canvas, x->x_obj.te_xpix+x->x_width-7, x->x_obj.te_ypix + x->x_height+1,
	     x->x_obj.te_xpix+x->x_width, x->x_obj.te_ypix + x->x_height+2,
	     tagRoot);
    COLORGRID_SYS_VGUI7(".x%x.c create rectangle %d %d %d %d -tags %so2\n",
	     canvas, x->x_obj.te_xpix+x->x_width-131, x->x_obj.te_ypix + x->x_height+1,
	     x->x_obj.te_xpix+x->x_width-126, x->x_obj.te_ypix + x->x_height+2,
	     tagRoot);

    if ( x->x_pdp_colorgrid ) 
    {
       int xlpos = x->x_obj.te_xpix+x->x_width/x->x_xlines;
       int ylpos = x->x_obj.te_ypix+x->x_height/x->x_ylines;
       int xcount = 1;
       int ycount = 1;
       while ( xlpos < x->x_obj.te_xpix+x->x_width )
       {
         COLORGRID_SYS_VGUI9(".x%x.c create line %d %d %d %d -fill #FFFFFF -tags %xLINE%d%d\n",
	     canvas, xlpos, x->x_obj.te_ypix,
	     xlpos, x->x_obj.te_ypix+x->x_height,
	     x, xcount, 0 );
         xlpos+=x->x_width/x->x_xlines;
         xcount++;
       }
       while ( ylpos < x->x_obj.te_ypix+x->x_height )
       {
         COLORGRID_SYS_VGUI9(".x%x.c create line %d %d %d %d -fill #FFFFFF -tags %xLINE%d%d\n",
	     canvas, x->x_obj.te_xpix, ylpos,
	     x->x_obj.te_xpix+x->x_width, ylpos,
	     x, 0, ycount);
         ylpos+=x->x_height/x->x_ylines;
         ycount++;
       }
    }
    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void pdp_colorgrid_draw_move(t_pdp_colorgrid *x, t_glist *glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    char *tagRoot;

    tagRoot = rtext_gettag(glist_findrtext(glist,(t_text *)x)); 
    COLORGRID_SYS_VGUI7(".x%x.c coords %xCOLORGRID %d %d %d %d\n",
	     canvas, x,
	     x->x_obj.te_xpix, x->x_obj.te_ypix,
	     x->x_obj.te_xpix+x->x_width, x->x_obj.te_ypix+x->x_height);
    COLORGRID_SYS_VGUI5(".x%x.c coords %sS %d %d \n",
	     canvas, tagRoot,
	     x->x_obj.te_xpix + 128, x->x_obj.te_ypix + 25);
    COLORGRID_SYS_VGUI7(".x%x.c coords %so0 %d %d %d %d\n",
	     canvas, tagRoot,
	     x->x_obj.te_xpix, x->x_obj.te_ypix + x->x_height+1,
	     x->x_obj.te_xpix+7, x->x_obj.te_ypix + x->x_height+2 );
    COLORGRID_SYS_VGUI7(".x%x.c coords %so1 %d %d %d %d\n",
	     canvas, tagRoot,
	     x->x_obj.te_xpix+x->x_width-7, x->x_obj.te_ypix + x->x_height+1,
	     x->x_obj.te_xpix+x->x_width, x->x_obj.te_ypix + x->x_height+2 );
    COLORGRID_SYS_VGUI7(".x%x.c coords %so2 %d %d %d %d\n",
	     canvas, tagRoot,
	     x->x_obj.te_xpix+x->x_width-131, x->x_obj.te_ypix + x->x_height+1,
	     x->x_obj.te_xpix+x->x_width-126, x->x_obj.te_ypix + x->x_height+2 );
    if ( x->x_point ) 
    {
       pdp_colorgrid_draw_update(x, glist);
    }
    if ( x->x_pdp_colorgrid ) 
    {
       int xlpos = x->x_obj.te_xpix+x->x_width/x->x_xlines;
       int ylpos = x->x_obj.te_ypix+x->x_height/x->x_ylines;
       int xcount = 1;
       int ycount = 1;
       while ( xlpos < x->x_obj.te_xpix+x->x_width )
       {
         COLORGRID_SYS_VGUI9(".x%x.c coords %xLINE%d%d %d %d %d %d\n",
	     canvas, x, xcount, 0, xlpos, x->x_obj.te_ypix,
	     xlpos, x->x_obj.te_ypix + x->x_height);
         xlpos+=x->x_width/x->x_xlines;
         xcount++;
       }
       while ( ylpos < x->x_obj.te_ypix+x->x_height )
       {
         COLORGRID_SYS_VGUI9(".x%x.c coords %xLINE%d%d %d %d %d %d\n",
	     canvas, x, 0, ycount, x->x_obj.te_xpix, ylpos,
	     x->x_obj.te_xpix + x->x_width, ylpos);
         ylpos+=x->x_height/x->x_ylines;
         ycount++;
       }
    }
    canvas_fixlinesfor( canvas, (t_text*)x );
}

static void pdp_colorgrid_draw_erase(t_pdp_colorgrid* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int i;
    char *tagRoot;

    tagRoot = rtext_gettag(glist_findrtext(glist,(t_text *)x));
    COLORGRID_SYS_VGUI3(".x%x.c delete %xCOLORGRID\n", canvas, x);
    COLORGRID_SYS_VGUI3(".x%x.c delete %sS\n", canvas, tagRoot);
    COLORGRID_SYS_VGUI3(".x%x.c delete %so0\n", canvas, tagRoot);
    COLORGRID_SYS_VGUI3(".x%x.c delete %so1\n", canvas, tagRoot);
    COLORGRID_SYS_VGUI3(".x%x.c delete %so2\n", canvas, tagRoot);
    if (x->x_pdp_colorgrid)  
    {
       for (i=1; i<x->x_xlines; i++ )
       {
           COLORGRID_SYS_VGUI4(".x%x.c delete %xLINE%d0\n", canvas, x, i);
       }
       for (i=1; i<x->x_ylines; i++ )
       {
           COLORGRID_SYS_VGUI4(".x%x.c delete %xLINE0%d\n", canvas, x, i);
       }
    }
    if (x->x_point)  
    {
          COLORGRID_SYS_VGUI3(".x%x.c delete %xPOINT\n", canvas, x);
          x->x_point = 0;
    }
    rtext_free(glist_findrtext(glist, (t_text *)x));
}

static void pdp_colorgrid_draw_select(t_pdp_colorgrid* x,t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);

    if(x->x_selected)
    {
	pd_bind(&x->x_obj.ob_pd, x->x_name);
        /* sets the item in blue */
	COLORGRID_SYS_VGUI3(".x%x.c itemconfigure %xCOLORGRID -outline #0000FF\n", canvas, x);
    }
    else
    {
	pd_unbind(&x->x_obj.ob_pd, x->x_name);
	COLORGRID_SYS_VGUI3(".x%x.c itemconfigure %xCOLORGRID -outline #000000\n", canvas, x);
    }
}

static void pdp_colorgrid_hsv2rgb(t_float hue, t_float saturation, t_float value, t_float *red, t_float *green, t_float *blue)
{
   t_float i=0, f=0, p=0, q=0, t=0;
	
   if (saturation == 0) {
       *red = value;
       *green = value;
       *blue = value;
   } else {
   if (hue == 6) hue = 0;
   i = (int)hue ;  /* the integer part of hue */
   f = hue - i;
   p = value * (1 - saturation);
   q = value * (1 - (saturation * f));
   t = value * (1 - (saturation * (1 - f)));
   switch ((int)i) {
           case 0:
            *red = value;
            *green = t;
            *blue = p;
            break;
           case 1:
            *red = q;
            *green = value;
            *blue = p;
            break;
           case 2:
            *red = p;
            *green = value;
            *blue = t;
            break;
           case 3:
            *red = p;
            *green = q;
            *blue = value;
            break;
           case 4:
            *red = t;
            *green = p;
            *blue = value;
	    break;
	   case 5:
	    *red = value;
            *green = p;
            *blue = q;
            break;
   } 
   }
}

static void pdp_colorgrid_output_current(t_pdp_colorgrid* x)
{
  t_float ox=0, oy=0, hue, saturation, value, red, green, blue ;

/* These values need to be the same as those that produced the spectrum image:*/

  t_float box_x = 256;
  t_float box_y = 25;

  t_float min_value = 0.3;
  t_float max_value = 1.0;
  t_float value_inc = (max_value - min_value) / box_y;

  t_float min_hue = 0;
  t_float max_hue = 6;
  t_float hue_inc = (max_hue - min_hue) / box_x;

  t_float max_saturation = 0.9;
  t_float min_saturation = 0.0;
  t_float saturation_inc = (max_saturation - min_saturation) / box_y;

  t_float xvalue, yvalue, rvalue, gvalue, bvalue;
  t_float xmodstep, ymodstep;

  xvalue = x->x_min + (x->x_current - x->x_obj.te_xpix) * (x->x_max-x->x_min) / x->x_width ;
  if (xvalue < x->x_min ) xvalue = x->x_min;
  if (xvalue > x->x_max ) xvalue = x->x_max;
  xmodstep = ((float)((int)(xvalue*10000) % (int)(x->x_xstep*10000))/10000.);
  xvalue = xvalue - xmodstep;
  yvalue = x->y_max - (x->y_current - x->x_obj.te_ypix ) * (x->y_max-x->y_min) / x->x_height ;
  if (yvalue < x->y_min ) yvalue = x->y_min;
  if (yvalue > x->y_max ) yvalue = x->y_max;
  ymodstep = ((float)((int)(yvalue*10000) % (int)(x->x_ystep*10000))/10000.);
  yvalue = yvalue - ymodstep;
  yvalue = 50 - yvalue;

  /* Use the coordinates only if they are non-zero: */

  if ((xvalue >= 0) && (yvalue >= 0)) {
    ox = xvalue;
    oy = yvalue;
  } else {
    xvalue = ox;
    yvalue = oy;
  } 

  if ((yvalue != 0)&&(yvalue!=50))
  {
   /* Calculate HSV based on given coordinates and convert to RGB: */
   hue = hue_inc * xvalue;
   if (yvalue <= box_y) {
    saturation = max_saturation;
    value = min_value + (value_inc * yvalue);
   } else {
    value = max_value - value_inc;
    saturation = max_saturation - (saturation_inc * (yvalue - box_y));
   } 

   pdp_colorgrid_hsv2rgb(hue, saturation, value, &red, &green, &blue);
 } else {
  if (yvalue == 0) {
    red     = 0;
    green   = 0;
    blue    = 0;
  } else {
    red     = 1;
    green   = 1;
    blue    = 1;
  }
 }
  
 /* The RGB values are returned in the interval [0..1] so we
    need to multiply by 256 to get "normal" color values.*/

  red     = red * 256;
  green   = green * 256;
  blue    = blue * 256;

    outlet_float( x->x_xoutlet, red );
    outlet_float( x->x_youtlet, green );
    outlet_float( x->x_zoutlet, blue );
}

/* ------------------------ pdp_colorgrid widgetbehaviour----------------------------- */


static void pdp_colorgrid_getrect(t_gobj *z, t_glist *owner,
			    int *xp1, int *yp1, int *xp2, int *yp2)
{
   t_pdp_colorgrid* x = (t_pdp_colorgrid*)z;

   *xp1 = x->x_obj.te_xpix;
   *yp1 = x->x_obj.te_ypix;
   *xp2 = x->x_obj.te_xpix+x->x_width;
   *yp2 = x->x_obj.te_ypix+x->x_height;
}

static void pdp_colorgrid_save(t_gobj *z, t_binbuf *b)
{
   t_pdp_colorgrid *x = (t_pdp_colorgrid *)z;

   // post( "saving pdp_colorgrid : %s", x->x_name->s_name );
   binbuf_addv(b, "ssiissiffiffiffiiff", gensym("#X"),gensym("obj"),
		(int)x->x_obj.te_xpix, (int)x->x_obj.te_ypix,
		gensym("pdp_colorgrid"), x->x_name, x->x_width, x->x_min,
		x->x_max, x->x_height,
                x->y_min, x->y_max,
                x->x_pdp_colorgrid, x->x_xstep, 
                x->x_ystep, x->x_xlines, x->x_ylines, 
                x->x_current, x->y_current );
   binbuf_addv(b, ";");
}

static void pdp_colorgrid_properties(t_gobj *z, t_glist *owner)
{
   char buf[800];
   t_pdp_colorgrid *x=(t_pdp_colorgrid *)z;

   sprintf(buf, "pdtk_pdp_colorgrid_dialog %%s %d %d %d\n",
                 x->x_xlines, x->x_ylines, x->x_pdp_colorgrid );
   // post("pdp_colorgrid_properties : %s", buf );
   gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

static void pdp_colorgrid_select(t_gobj *z, t_glist *glist, int selected)
{
   t_pdp_colorgrid *x = (t_pdp_colorgrid *)z;

   x->x_selected = selected;
   pdp_colorgrid_draw_select( x, glist );
}

static void pdp_colorgrid_vis(t_gobj *z, t_glist *glist, int vis)
{
   t_pdp_colorgrid *x = (t_pdp_colorgrid *)z;

   if (vis)
   {
      pdp_colorgrid_draw_new( x, glist );
      pdp_colorgrid_draw_update( x, glist );
      pdp_colorgrid_output_current(x);
   }
   else
   {
      pdp_colorgrid_draw_erase( x, glist );
   }
}

static void pdp_colorgrid_dialog(t_pdp_colorgrid *x, t_symbol *s, int argc, t_atom *argv)
{
   if ( !x ) {
     post( "pdp_colorgrid : error :tried to set properties on an unexisting object" );
   }
   if ( argv[0].a_type != A_FLOAT || argv[1].a_type != A_FLOAT ||
           argv[2].a_type != A_FLOAT ) 
   { 
      post( "pdp_colorgrid : wrong arguments" );
      return;
   }
   x->x_xlines = argv[0].a_w.w_float;
   x->x_ylines = argv[1].a_w.w_float;
   x->x_pdp_colorgrid = argv[2].a_w.w_float;
   pdp_colorgrid_draw_erase(x, x->x_glist);
   pdp_colorgrid_draw_new(x, x->x_glist);
}

static void pdp_colorgrid_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor( glist_getcanvas(glist), (t_text *)z);
}

static void pdp_colorgrid_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_pdp_colorgrid *x = (t_pdp_colorgrid *)z;
    int xold = x->x_obj.te_xpix;
    int yold = x->x_obj.te_ypix;

    // post( "pdp_colorgrid_displace dx=%d dy=%d", dx, dy );

    x->x_obj.te_xpix += dx;
    x->x_current += dx;
    x->x_obj.te_ypix += dy;
    x->y_current += dy;
    if(xold != x->x_obj.te_xpix || yold != x->x_obj.te_ypix)
    {
	pdp_colorgrid_draw_move(x, x->x_glist);
    }
}

static void pdp_colorgrid_motion(t_pdp_colorgrid *x, t_floatarg dx, t_floatarg dy)
{
    int xold = x->x_current;
    int yold = x->y_current;

    // post( "pdp_colorgrid_motion dx=%f dy=%f", dx, dy );

    x->x_current += dx;
    x->y_current += dy;
    if(xold != x->x_current || yold != x->y_current)
    {
        pdp_colorgrid_output_current(x);
	pdp_colorgrid_draw_update(x, x->x_glist);
    }
}

static int pdp_colorgrid_click(t_gobj *z, struct _glist *glist,
			    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_pdp_colorgrid* x = (t_pdp_colorgrid *)z;

    // post( "pdp_colorgrid_click doit=%d x=%d y=%d", doit, xpix, ypix );
    if ( doit) 
    {
      x->x_current = xpix;
      x->y_current = ypix;
      pdp_colorgrid_output_current(x);
      pdp_colorgrid_draw_update(x, glist);
      glist_grab(glist, &x->x_obj.te_g, (t_glistmotionfn)pdp_colorgrid_motion,
	       0, xpix, ypix);
    }
    return (1);
}

static void pdp_colorgrid_goto(t_pdp_colorgrid *x, t_floatarg newx, t_floatarg newy)
{
    int xold = x->x_current;
    int yold = x->y_current;

    if ( newx > x->x_width-1 ) newx = x->x_width-1;
    if ( newx < 0 ) newx = 0;
    if ( newy > x->x_height-1 ) newy = x->x_height-1;
    if ( newy < 0 ) newy = 0;

    // post( "pdp_colorgrid_set x=%f y=%f", newx, newy );

    x->x_current = newx + x->x_obj.te_xpix;
    x->y_current = newy + x->x_obj.te_ypix;
    if(xold != x->x_current || yold != x->y_current)
    {
        pdp_colorgrid_output_current(x);
        pdp_colorgrid_draw_update(x, x->x_glist);
    }
}

static void pdp_colorgrid_xgoto(t_pdp_colorgrid *x, t_floatarg newx, t_floatarg newy)
{
    int xold = x->x_current;
    int yold = x->y_current;

    if ( newx > x->x_width-1 ) newx = x->x_width-1;
    if ( newx < 0 ) newx = 0;
    if ( newy > x->x_height-1 ) newy = x->x_height-1;
    if ( newy < 0 ) newy = 0;

    // post( "pdp_colorgrid_set x=%f y=%f", newx, newy );

    x->x_current = newx + x->x_obj.te_xpix;
    x->y_current = newy + x->x_obj.te_ypix;
    if(xold != x->x_current || yold != x->y_current)
    {
        pdp_colorgrid_draw_update(x, x->x_glist);
    }
}

static void pdp_colorgrid_bang(t_pdp_colorgrid *x) {
  pdp_colorgrid_output_current(x);
}

static t_pdp_colorgrid *pdp_colorgrid_new(t_symbol *s, int argc, t_atom *argv)
{
    int i, zz;
    t_pdp_colorgrid *x;
    t_pd *x2;
    char *str;
 
    // post( "pdp_colorgrid_new : create : %s argc =%d", s->s_name, argc );

    x = (t_pdp_colorgrid *)pd_new(pdp_colorgrid_class);
    // new pdp_colorgrid created from the gui 
    if ( argc != 0 )
    {
      if ( argc != 14 )
      {
        post( "pdp_colorgrid : error in the number of arguments ( %d instead of 14 )", argc );
        return NULL;
      }
      if ( argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT ||
        argv[2].a_type != A_FLOAT || argv[3].a_type != A_FLOAT ||
        argv[4].a_type != A_FLOAT || argv[5].a_type != A_FLOAT ||
        argv[6].a_type != A_FLOAT || argv[7].a_type != A_FLOAT || 
        argv[8].a_type != A_FLOAT || argv[9].a_type != A_FLOAT || 
        argv[10].a_type != A_FLOAT || argv[11].a_type != A_FLOAT || 
        argv[12].a_type != A_FLOAT || argv[13].a_type != A_FLOAT ) {
        post( "pdp_colorgrid : wrong arguments" );
        return NULL;
      }

      // update pdp_colorgrid count
      if (!strncmp((str = argv[0].a_w.w_symbol->s_name), "pdp_colorgrid", 5)
    	 && (zz = atoi(str + 5)) > pdp_colorgridcount) 
      {
        pdp_colorgridcount = zz;
      }
      x->x_name = argv[0].a_w.w_symbol;
      pd_bind(&x->x_obj.ob_pd, x->x_name);
      x->x_width = argv[1].a_w.w_float;
      x->x_min = argv[2].a_w.w_float;
      x->x_max = argv[3].a_w.w_float;
      x->x_height = argv[4].a_w.w_float;
      x->y_min = argv[5].a_w.w_float;
      x->y_max = argv[6].a_w.w_float;
      x->x_pdp_colorgrid = argv[7].a_w.w_float;
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

      sprintf(buf, "pdp_colorgrid%d", ++pdp_colorgridcount);
      s = gensym(buf);    	

      x->x_name = s;
      pd_bind(&x->x_obj.ob_pd, x->x_name);

      x->x_width = DEFAULT_COLORGRID_WIDTH;
      x->x_min = 0;
      x->x_max = DEFAULT_COLORGRID_WIDTH;
      x->x_height = DEFAULT_COLORGRID_HEIGHT;
      x->y_min = 0;
      x->y_max = DEFAULT_COLORGRID_HEIGHT;
      x->x_pdp_colorgrid = 0;	
      x->x_xstep = 1.0;	
      x->x_ystep = 1.0;	
      x->x_xlines = DEFAULT_COLORGRID_NBLINES;	
      x->x_ylines = DEFAULT_COLORGRID_NBLINES;	
      x->x_current = 0;
      x->y_current = 0;	

    }

    // common fields for new and restored pdp_colorgrids
    x->x_point = 0;	
    x->x_selected = 0;	
    x->x_glist = (t_glist *) canvas_getcurrent();
    x->x_xoutlet = outlet_new(&x->x_obj, &s_float ); 
    x->x_youtlet = outlet_new(&x->x_obj, &s_float ); 
    x->x_zoutlet = outlet_new(&x->x_obj, &s_float ); 

    // post( "pdp_colorgrid_new name : %s width: %d height : %d", x->x_name->s_name, x->x_width, x->x_height );

    return (x);
}

static void pdp_colorgrid_free(t_pdp_colorgrid *x)
{
    post( "pdp_colorgrid~: freeing ressources [NULL]" );
}

void pdp_colorgrid_setup(void)
{
#include "pdp_colorgrid.tk2c"
    post ( pdp_colorgrid_version );
    pdp_colorgrid_class = class_new(gensym("pdp_colorgrid"), (t_newmethod)pdp_colorgrid_new,
			      (t_method)pdp_colorgrid_free, sizeof(t_pdp_colorgrid), 0, A_GIMME, 0);
    class_addmethod(pdp_colorgrid_class, (t_method)pdp_colorgrid_click, gensym("click"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(pdp_colorgrid_class, (t_method)pdp_colorgrid_motion, gensym("motion"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(pdp_colorgrid_class, (t_method)pdp_colorgrid_bang, gensym("bang"), 0);
    class_addmethod(pdp_colorgrid_class, (t_method)pdp_colorgrid_goto, gensym("goto"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(pdp_colorgrid_class, (t_method)pdp_colorgrid_xgoto, gensym("xgoto"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(pdp_colorgrid_class, (t_method)pdp_colorgrid_dialog, gensym("dialog"), A_GIMME, 0);
    pdp_colorgrid_widgetbehavior.w_getrectfn =    pdp_colorgrid_getrect;
    pdp_colorgrid_widgetbehavior.w_displacefn =   pdp_colorgrid_displace;
    pdp_colorgrid_widgetbehavior.w_selectfn =     pdp_colorgrid_select;
    pdp_colorgrid_widgetbehavior.w_activatefn =   NULL;
    pdp_colorgrid_widgetbehavior.w_deletefn =     pdp_colorgrid_delete;
    pdp_colorgrid_widgetbehavior.w_visfn =        pdp_colorgrid_vis;
    pdp_colorgrid_widgetbehavior.w_clickfn =      pdp_colorgrid_click;
    class_setwidget(pdp_colorgrid_class, &pdp_colorgrid_widgetbehavior);
    class_setpropertiesfn(pdp_colorgrid_class, pdp_colorgrid_properties);
    class_setsavefn(pdp_colorgrid_class, pdp_colorgrid_save);

}
