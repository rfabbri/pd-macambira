/*------------------------ playlist~ ------------------------------------------ */
/*                                                                              */
/* playlist~ : lets you choose a file with 1 click                              */
/*             or by sending a 'seek #' message                                 */
/* constructor : playlist <extension> <width> <height>                          */
/*                                                                              */
/* Copyleft Yves Degoyon ( ydegoyon@free.fr )                                   */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* See file LICENSE for further informations on licensing terms.                */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.                             */
/*                                                                              */
/* "If a man's made of blood and iron"                                          */
/* "Doctor, doctor, what's in my chest ????"                                    */
/* Gang Of Four -- Guns Before Butter                                           */
/* ---------------------------------------------------------------------------- */  

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <regex.h>
#include "m_imp.h"
#include "g_canvas.h"
#include "t_tk.h"

#ifdef NT
#include <io.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#endif

t_widgetbehavior playlist_widgetbehavior;
static t_class *playlist_class;

static int guidebug=0;

static char   *playlist_version = "playlist: 1 click file chooser : version 0.6, written by Yves Degoyon (ydegoyon@free.fr)";

#define CHAR_WIDTH 6 // average character width with font Helvetica 8
#define MAX_DIR_LENGTH 2048 // maximum length for a directory name

#define MIN(a,b) (a>b?b:a)

#define SYS_VGUI2(a,b) if (guidebug) \
                         post(a,b);\
                         sys_vgui(a,b)

#define SYS_VGUI3(a,b,c) if (guidebug) \
                         post(a,b,c);\
                         sys_vgui(a,b,c)

#define SYS_VGUI4(a,b,c,d) if (guidebug) \
                         post(a,b,c,d);\
                         sys_vgui(a,b,c,d)

#define SYS_VGUI5(a,b,c,d,e) if (guidebug) \
                         post(a,b,c,d,e);\
                         sys_vgui(a,b,c,d,e)

#define SYS_VGUI6(a,b,c,d,e,f) if (guidebug) \
                         post(a,b,c,d,e,f);\
                         sys_vgui(a,b,c,d,e,f)

#define SYS_VGUI7(a,b,c,d,e,f,g) if (guidebug) \
                         post(a,b,c,d,e,f,g);\
                         sys_vgui(a,b,c,d,e,f,g)

#define SYS_VGUI8(a,b,c,d,e,f,g,h) if (guidebug) \
                         post(a,b,c,d,e,f,g,h);\
                         sys_vgui(a,b,c,d,e,f,g,h)

#define SYS_VGUI9(a,b,c,d,e,f,g,h,i) if (guidebug) \
                         post(a,b,c,d,e,f,g,h,i );\
                         sys_vgui(a,b,c,d,e,f,g,h,i)


typedef struct _playlist
{
    t_object x_obj;
    t_glist *x_glist;
    t_outlet *x_fullpath;
    t_outlet *x_file;
    t_outlet *x_dir;
    char *x_extension;          /* extension to selected files               */
    t_int x_height;             /* height of the playlist                    */
    t_int x_width;              /* width of the playlist                     */
    t_int x_itemselected;       /* index of the selected item                */
    t_int x_selected;           /* stores selected state                     */
    char **x_dentries;          /* directory entries                         */
    t_int x_nentries;           /* number of entries in the current dir      */
    t_int x_pnentries;          /* previous size of entries list             */
    t_int x_firstseen;          /* first displayed entry                     */
    t_int x_lastseen;           /* last displayed entry                      */
    t_int x_cdy;                /* cumulated y drag                          */
    char   *x_curdir;           /* current directory informations            */
} t_playlist;     


static void playlist_update_dir(t_playlist *x, t_glist *glist)
{
  t_canvas *canvas=glist_getcanvas(glist);    
  t_int i;
  char wrappedname[ MAX_DIR_LENGTH ];

    // set title
    SYS_VGUI3(".x%x.c delete %xTITLE\n", glist_getcanvas(glist), x); 
    SYS_VGUI7(".x%x.c create text %d %d -width %d -text \"%s\"  \
               -anchor w -font {Helvetica 8 bold} -tags %xTITLE\n",
               canvas, 
               x->x_obj.te_xpix+5, 
               x->x_obj.te_ypix-10, 
               x->x_width,
               x->x_curdir,
               x );

    // delete previous entries
    for ( i=x->x_firstseen; i<=x->x_lastseen; i++ )
    {
        SYS_VGUI4(".x%x.c delete %xENTRY%d\n", glist_getcanvas(glist), x, i); 
    }

    // display the content of current directory
    {
       t_int nentries, i;
       struct dirent** dentries;      /* all directory entries                         */

       // post( "playlist : scandir : %s", x->x_curdir );
       if ( ( nentries = scandir(x->x_curdir, &dentries, NULL, alphasort ) ) == -1 )
       {
          post( "playlist : could not scan current directory ( where the hell are you ??? )" );
          perror( "scandir" );
          return; 
       }

       x->x_firstseen = 0;
       if ( x->x_dentries )
       {
          for ( i=0; i<x->x_nentries; i++ )
          {
              // post( "playlist : freeing entry %d size=%d : %s", i, strlen( x->x_dentries[i] ) + 1, x->x_dentries[i] );
              freebytes( x->x_dentries[i], strlen( x->x_dentries[i] ) + 1 );
          }
       }
       if ( x->x_pnentries != -1 )
       {
              freebytes( x->x_dentries, x->x_pnentries*sizeof(char**) );
       }

       x->x_nentries = 0;
       // post( "playlist : allocating dentries %d", nentries );
       x->x_dentries = (char **) getbytes( nentries*sizeof(char**) ) ;
       x->x_pnentries = nentries;
       for ( i=0; i<nentries; i++ )
       {
        size_t nmatches = 0;
        regmatch_t matchinfos[1];
        DIR* tmpdir;

          // ckeck if that entry should be displayed
          if ( ( ( ( tmpdir = opendir( dentries[i]->d_name ) ) != NULL ) ) ||
               ( strstr( dentries[i]->d_name, x->x_extension ) ) ||
               ( !strcmp( x->x_extension, "all" ) ) 
             )
          {
            // close temporarily opened dir
            if ( tmpdir )
            {
               if ( closedir( tmpdir ) < 0 ) 
               {
                  post( "playlist : could not close directory %s", dentries[i]->d_name );
               }
            }

            // post( "playlist : allocating entry %d %d : %s", x->x_nentries, strlen( dentries[i]->d_name ) + 1, dentries[i]->d_name );
            x->x_dentries[x->x_nentries] = ( char * ) getbytes( strlen( dentries[i]->d_name ) + 1 );
            strcpy( x->x_dentries[x->x_nentries],  dentries[i]->d_name );
            
            // display the entry if displayable
            if ( x->x_nentries*10+5 < x->x_height )
            {
             x->x_lastseen = x->x_nentries;
             strncpy( wrappedname, x->x_dentries[x->x_nentries],  MIN(x->x_width/CHAR_WIDTH, MAX_DIR_LENGTH) ); 
             wrappedname[ x->x_width/CHAR_WIDTH ] = '\0';
             SYS_VGUI8(".x%x.c create text %d %d -fill #000000 -activefill #FF0000 -width %d -text \"%s\"  \
                        -anchor w -font {Helvetica 8 bold} -tags %xENTRY%d\n",
                    canvas, 
                    x->x_obj.te_xpix+5, 
                    x->x_obj.te_ypix+5+(x->x_nentries-x->x_firstseen)*10, 
                    x->x_width,
                    wrappedname,
                    x, x->x_nentries );
            }
            x->x_nentries++;
          }

       }
       
    }
}

void playlist_output_current(t_playlist* x)
{
    // output the selected dir+file
    if ( x->x_dentries && x->x_itemselected < x->x_nentries && x->x_itemselected >= 0 )
    {
       char* tmpstring = (char*) getbytes( strlen( x->x_curdir ) + strlen( x->x_dentries[x->x_itemselected]) + 2 );

       sprintf( tmpstring, "%s/%s", x->x_curdir, x->x_dentries[x->x_itemselected] );
       outlet_symbol( x->x_dir, gensym( x->x_curdir ) );
       outlet_symbol( x->x_file, gensym( x->x_dentries[x->x_itemselected] ) );
       outlet_symbol( x->x_fullpath, gensym( tmpstring ) );
    }
}             

void playlist_draw_new(t_playlist *x, t_glist *glist)
{   
  t_canvas *canvas=glist_getcanvas(glist);    

    SYS_VGUI7(".x%x.c create rectangle %d %d %d %d -fill #457782 -tags %xPLAYLIST\n",
             canvas, x->x_obj.te_xpix, x->x_obj.te_ypix,
             x->x_obj.te_xpix + x->x_width, x->x_obj.te_ypix + x->x_height,
             x); 
    SYS_VGUI7(".x%x.c create rectangle %d %d %d %d -fill yellow -tags %xSCROLLLIST\n",
             canvas, x->x_obj.te_xpix+4*x->x_width/5, x->x_obj.te_ypix,
             x->x_obj.te_xpix + x->x_width, x->x_obj.te_ypix + x->x_height,
             x); 

    playlist_update_dir( x, glist );

}

void playlist_draw_move(t_playlist *x, t_glist *glist)
{  
    t_canvas *canvas=glist_getcanvas(glist);
    t_int i;
 
    SYS_VGUI7(".x%x.c coords %xPLAYLIST %d %d %d %d\n",
             canvas, x,
             x->x_obj.te_xpix, x->x_obj.te_ypix,
             x->x_obj.te_xpix+x->x_width, 
             x->x_obj.te_ypix+x->x_height);
    SYS_VGUI7(".x%x.c coords %xSCROLLLIST %d %d %d %d\n",
             canvas, x,
             x->x_obj.te_xpix+4*x->x_width/5, x->x_obj.te_ypix,
             x->x_obj.te_xpix+x->x_width, 
             x->x_obj.te_ypix+x->x_height);
    SYS_VGUI5(".x%x.c coords %xTITLE %d %d\n",
             canvas, x,
             x->x_obj.te_xpix+5, x->x_obj.te_ypix-10 );
    for ( i=x->x_firstseen; i<=x->x_lastseen; i++ )
    {
       SYS_VGUI6(".x%x.c coords %xENTRY%d %d %d\n",
             canvas, x, i,
             x->x_obj.te_xpix+5,
             x->x_obj.te_ypix+5+(i-x->x_firstseen)*10);
    }

    canvas_fixlinesfor( canvas, (t_text*)x );
}

void playlist_draw_erase(t_playlist* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
    int i;
 
    SYS_VGUI3(".x%x.c delete %xPLAYLIST\n", canvas, x);
    SYS_VGUI3(".x%x.c delete %xSCROLLLIST\n", canvas, x);
    SYS_VGUI3(".x%x.c delete %xTITLE\n", canvas, x);
    for ( i=x->x_firstseen; i<=x->x_lastseen; i++ )
    {
        SYS_VGUI4(".x%x.c delete %xENTRY%d\n", glist_getcanvas(glist), x, i); 
    }
} 

void playlist_draw_select(t_playlist* x, t_glist* glist)
{
    t_canvas *canvas=glist_getcanvas(glist);
 
    // post( "playlist : select" );
    if(x->x_selected)
    {
        /* sets the item in blue */
        SYS_VGUI3(".x%x.c itemconfigure %xPLAYLIST -outline #0000FF\n", canvas, x);
    }
    else
    {
        SYS_VGUI3(".x%x.c itemconfigure %xPLAYLIST -outline #000000\n", canvas, x);
    }
}       

/* ------------------------ playlist widgetbehaviour----------------------------- */


void playlist_getrect(t_gobj *z, t_glist *owner,
			    int *xp1, int *yp1, int *xp2, int *yp2)
{
   t_playlist* x = (t_playlist*)z;

   *xp1 = x->x_obj.te_xpix;
   *yp1 = x->x_obj.te_ypix;
   *xp2 = x->x_obj.te_xpix+x->x_width;
   *yp2 = x->x_obj.te_ypix+x->x_height;
}

void playlist_save(t_gobj *z, t_binbuf *b)
{
   t_playlist *x = (t_playlist *)z;

   // post( "saving playlist : %s", x->x_extension );
   binbuf_addv(b, "ssiissii", gensym("#X"),gensym("obj"),
		(t_int)x->x_obj.te_xpix, (t_int)x->x_obj.te_ypix,
		gensym("playlist"), gensym(x->x_extension), x->x_width, x->x_height );
   binbuf_addv(b, ";");
}

void playlist_select(t_gobj *z, t_glist *glist, int selected)
{
   t_playlist *x = (t_playlist *)z;

   x->x_selected = selected;

   playlist_draw_select( x, glist );
}

void playlist_vis(t_gobj *z, t_glist *glist, int vis)
{
   t_playlist *x = (t_playlist *)z;
   t_rtext *y;

   if (vis)
   {
      playlist_draw_new( x, glist );
   }
   else
   {
      playlist_draw_erase( x, glist );
   }
}

void playlist_delete(t_gobj *z, t_glist *glist)
{
    canvas_deletelinesfor( glist_getcanvas(glist), (t_text *)z);
}

void playlist_displace(t_gobj *z, t_glist *glist, int dx, int dy)
{
    t_playlist *x = (t_playlist *)z;
    int xold = x->x_obj.te_xpix;
    int yold = x->x_obj.te_ypix;

    // post( "playlist_displace dx=%d dy=%d", dx, dy );

    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if(xold != x->x_obj.te_xpix || yold != x->x_obj.te_ypix)
    {
	playlist_draw_move(x, glist);
    }
}

void playlist_motion(t_playlist *x, t_floatarg dx, t_floatarg dy)
{
 int i;
  
  x->x_cdy+=dy;
 
  // check if we need to scroll
  if (  ( x->x_lastseen < x->x_nentries ) )
  {
    // eventually, move down
    if ( ( x->x_cdy >= 3 ) && 
         ( x->x_firstseen < x->x_nentries - ( x->x_height/10 ) ) )
    {
       x->x_cdy = 0;
       if ( x->x_firstseen + 1 < x->x_nentries )
       {
          for ( i=x->x_firstseen; i<=x->x_lastseen; i++ )
          {
              SYS_VGUI4(".x%x.c delete %xENTRY%d\n", x->x_glist, x, i); 
          }
          x->x_firstseen++;
          for ( i=x->x_firstseen; i< x->x_nentries; i++ )
          {
             char *wrappedname = (char *) getbytes( x->x_width );

             if ( (i-x->x_firstseen)*10+5 < x->x_height )
             {
               x->x_lastseen = i;
               strncpy( wrappedname, x->x_dentries[i],  x->x_width/CHAR_WIDTH );
               wrappedname[ x->x_width/CHAR_WIDTH ] = '\0';
               SYS_VGUI8(".x%x.c create text %d %d -fill #000000 -activefill #FF0000 -width %d -text \"%s\"  \
                        -anchor w -font {Helvetica 8 bold} -tags %xENTRY%d\n",
                    glist_getcanvas(x->x_glist), 
                    x->x_obj.te_xpix+5, 
                    x->x_obj.te_ypix+5+(i-x->x_firstseen)*10, 
                    x->x_width,
                    wrappedname,
                    x, i );
             }
             else break;
          }
          SYS_VGUI4(".x%x.c itemconfigure %xENTRY%d -fill #FF0000\n", x->x_glist, x, x->x_itemselected); 
          // post( "playlist : moved down first=%d last=%d", x->x_firstseen, x->x_lastseen );
       }
    }
    // eventually, move up
    if ( ( x->x_cdy <= -3 ) && 
         ( x->x_lastseen >= ( x->x_height/10 ) ) )
    {
       x->x_cdy = 0;
       if ( x->x_firstseen - 1 >= 0 )
       {
          for ( i=x->x_firstseen; i<=x->x_lastseen; i++ )
          {
              SYS_VGUI4(".x%x.c delete %xENTRY%d\n", x->x_glist, x, i); 
          }
          x->x_firstseen--;
          for ( i=x->x_firstseen; i< x->x_nentries; i++ )
          {
             char *wrappedname = (char *) getbytes( x->x_width );

             if ( (i-x->x_firstseen)*10+5 < x->x_height )
             {
               x->x_lastseen = i;
               strncpy( wrappedname, x->x_dentries[i],  x->x_width/CHAR_WIDTH );
               wrappedname[ x->x_width/CHAR_WIDTH ] = '\0';
               SYS_VGUI8(".x%x.c create text %d %d -fill #000000 -activefill #FF0000 -width %d -text \"%s\"  \
                        -anchor w -font {Helvetica 8 bold} -tags %xENTRY%d\n",
                    glist_getcanvas(x->x_glist), 
                    x->x_obj.te_xpix+5, 
                    x->x_obj.te_ypix+5+(i-x->x_firstseen)*10, 
                    x->x_width,
                    wrappedname,
                    x, i );
             }
             else break;
          }
          SYS_VGUI4(".x%x.c itemconfigure %xENTRY%d -fill #FF0000\n", x->x_glist, x, x->x_itemselected); 
          // post( "playlist : moved up first=%d last=%d", x->x_firstseen, x->x_lastseen );
       }
    }
  } // scroll test
} 
  

int playlist_click(t_gobj *z, struct _glist *glist,
			    int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
    t_playlist* x = (t_playlist *)z;

    if (doit) 
    {
      // leave a margin for scrolling without selection 
      if ( (xpix-x->x_obj.te_xpix) < 4*x->x_width/5 )
      {
        // deselect previously selected item
        SYS_VGUI4(".x%x.c itemconfigure %xENTRY%d -fill #000000\n", x->x_glist, x, x->x_itemselected); 
        x->x_itemselected = x->x_firstseen + (ypix-x->x_obj.te_ypix)/10;
        SYS_VGUI4(".x%x.c itemconfigure %xENTRY%d -fill #FF0000\n", x->x_glist, x, x->x_itemselected); 
        // post( "playlist : selected item : %d", x->x_itemselected );
        if ( x->x_dentries && ( x->x_itemselected < x->x_nentries ) )
        {
          char *tmpstring = (char *) getbytes( strlen( x->x_curdir ) + strlen( x->x_dentries[x->x_itemselected] ) + 2 );  
          sprintf( tmpstring, "%s/%s", x->x_curdir, x->x_dentries[x->x_itemselected] );
          // post( "playlist : chdir : %s", tmpstring );
          if ( chdir( tmpstring ) < 0 )
          {
             playlist_output_current(x);
          }
          else
          {
             if ( !strcmp(  x->x_dentries[ x->x_itemselected ], ".." ) )
             {
                char *iamthelastslash;
                   
                 iamthelastslash = strrchr( x->x_curdir, '/' );
                 *iamthelastslash = '\0';

                 if ( !strcmp( x->x_curdir, "" ) )
                 {
                    strcpy( x->x_curdir, "/" );
                 }
             }
             else 
             if ( !strcmp(  x->x_dentries[ x->x_itemselected ], "." ) )
             {
                  // nothing
             }
             else 
             {
                 if ( strlen( x->x_curdir ) + strlen( x->x_dentries[x->x_itemselected] ) + 2 > MAX_DIR_LENGTH ) 
                 {
                    post( "playlist : maximum dir length reached : cannot change directory" );
                    return -1;
                 }
                 if ( strcmp( x->x_curdir, "/" ) )
                 {
                    sprintf( x->x_curdir, "%s/%s", x->x_curdir, x->x_dentries[x->x_itemselected] );
                 }
                 else
                 {
                    sprintf( x->x_curdir, "/%s", x->x_dentries[x->x_itemselected] );
                 }
             }

             playlist_update_dir( x, glist );
          }
        }
      }
      glist_grab(glist, &x->x_obj.te_g, (t_glistmotionfn)playlist_motion,
               0, xpix, ypix); 
    }
    return (1);
}

static void playlist_properties(t_gobj *z, t_glist *owner)
{
   char buf[800];
   t_playlist *x=(t_playlist *)z;

   sprintf(buf, "pdtk_playlist_dialog %%s %s %d %d\n",
            x->x_extension, x->x_width, x->x_height);
   // post("playlist_properties : %s", buf );
   gfxstub_new(&x->x_obj.ob_pd, x, buf);
}

static void playlist_dialog(t_playlist *x, t_symbol *s, int argc, t_atom *argv)
{
   if ( !x ) {
     post( "playlist : error :tried to set properties on an unexisting object" );
   }
   if ( argc != 3 )
   {
      post( "playlist : error in the number of arguments ( %d instead of 3 )", argc );
      return;
   }
      if ( argv[0].a_type != A_SYMBOL || argv[1].a_type != A_FLOAT ||
        argv[2].a_type != A_FLOAT ) {
      post( "playlist : wrong arguments" );
      return;
   }
   x->x_extension = argv[0].a_w.w_symbol->s_name;
   x->x_width = (int)argv[1].a_w.w_float;
   x->x_height = (int)argv[2].a_w.w_float;

   playlist_draw_erase(x, x->x_glist);
   playlist_draw_new(x, x->x_glist);
}


t_playlist *playlist_new(t_symbol *extension, t_floatarg fwidth, t_floatarg fheight )
{
    int i;
    t_playlist *x;
    char *tmpcurdir;

    if ( !strcmp( extension->s_name, "" ) )
    {
       error( "playlist : no extension specified" );
       error( "playlist : usage : playlist <extension> <width> <height>" );
       return NULL;
    }

    if ( fwidth <= 0 ) 
    {
       error( "playlist : wrong width (%d)", fwidth );
       error( "playlist : usage : playlist <extension> <width> <height>" );
       return NULL;
    }

    if ( fheight <= 0 ) 
    {
       error( "playlist : wrong height (%d)", fheight );
       error( "playlist : usage : playlist <extension> <width> <height>" );
       return NULL;
    }

    x = (t_playlist *)pd_new(playlist_class);

    x->x_width = fwidth;
    x->x_height = fheight;
    x->x_extension = ( char * ) getbytes( strlen( extension->s_name ) + 2 );
    sprintf( x->x_extension, "%s", extension->s_name );

    x->x_fullpath = outlet_new(&x->x_obj, &s_symbol );
    x->x_file = outlet_new(&x->x_obj, &s_symbol );
    x->x_dir = outlet_new(&x->x_obj, &s_symbol );

    x->x_glist = (t_glist *) canvas_getcurrent(); 
    x->x_nentries = 0;
    x->x_pnentries = 0;
    x->x_dentries = NULL;

    // get current directory full path
    x->x_curdir = ( char * ) getbytes( MAX_DIR_LENGTH );
    if ( ( tmpcurdir = getenv( "PWD" ) ) == NULL )
    {
        post( "playlist : could not get current directory ( where the hell are you ??? )" ); 
        return NULL; 
    }
    strncpy( x->x_curdir, tmpcurdir, strlen( tmpcurdir ) );
    x->x_curdir[ strlen( tmpcurdir ) ] = '\0';

    x->x_selected = 0;
    x->x_itemselected = -1;

    // post( "playlist : built extension=%s width=%d height=%d", x->x_extension, x->x_width, x->x_height );

    return (x);
}

void playlist_free(t_playlist *x)
{
    // post( "playlist : playlist_free" );
    if ( x->x_extension )
    {
       freebytes( x->x_extension, strlen( x->x_extension ) );
    }
    if ( x->x_curdir )
    {
       freebytes( x->x_curdir, MAX_DIR_LENGTH );
    }
}

void playlist_seek(t_playlist *x, t_floatarg fseeked)
{
   int iout=0;

   if ( fseeked < 0 )
   {
      post( "playlist : wrong searched file : %f", fseeked );
      return;
   }

   if ( x->x_nentries > 2 )
   {
      // do not select . or ..
      iout = (int)fseeked % (x->x_nentries-2) + 2;
   }
   else
   {
      return;
   }
   SYS_VGUI4(".x%x.c itemconfigure %xENTRY%d -fill #000000\n", x->x_glist, x, x->x_itemselected); 
   x->x_itemselected = iout;
   SYS_VGUI4(".x%x.c itemconfigure %xENTRY%d -fill #FF0000\n", x->x_glist, x, x->x_itemselected); 
   playlist_output_current(x);
}

void playlist_location(t_playlist *x, t_symbol *flocation)
{
   int iout=0;
   char olddir[ MAX_DIR_LENGTH ];           /* remember old location  */

   strcpy( olddir, x->x_curdir );

   if ( !strcmp(  flocation->s_name, ".." ) )
   {
      char *iamthelastslash;
         
       iamthelastslash = strrchr( x->x_curdir, '/' );
       *iamthelastslash = '\0';

       if ( !strcmp( x->x_curdir, "" ) )
       {
          strcpy( x->x_curdir, "/" );
       }
   }
   else 
   if ( !strncmp( flocation->s_name, "/", 1 ) )
   {
        // absolute path required
        if ( strlen( flocation->s_name ) >= MAX_DIR_LENGTH )
        {
          error( "playlist : maximum dir length reached : cannot change directory" );
          return;
        }
        strncpy( x->x_curdir, flocation->s_name, MAX_DIR_LENGTH );
   }
   else 
   {
       // relative path
       if ( strlen( x->x_curdir ) + strlen( flocation->s_name ) + 2 > MAX_DIR_LENGTH ) 
       {
          post( "playlist : maximum dir length reached : cannot change directory" );
          return;
       }
       if ( strcmp( x->x_curdir, "/" ) )
       {
          sprintf( x->x_curdir, "%s/%s", x->x_curdir, flocation->s_name );
       }
       else
       {
          sprintf( x->x_curdir, "/%s", flocation->s_name );
       }
   }
   
   if ( chdir( x->x_curdir ) < 0 )
   {
     error( "playlist : requested location >%s< is not a directory", x->x_curdir );
     strcpy( x->x_curdir, olddir );
     return;
   }

   playlist_update_dir( x, x->x_glist );
}

void playlist_setup(void)
{
    post( playlist_version );
#include "playlist.tk2c"
    playlist_class = class_new(gensym("playlist"), (t_newmethod)playlist_new,
			      (t_method)playlist_free, sizeof(t_playlist), 
                              CLASS_DEFAULT, A_SYMBOL, A_FLOAT, A_FLOAT, 0);
    class_addmethod(playlist_class, (t_method)playlist_seek, gensym("seek"), A_FLOAT, 0 );
    class_addmethod(playlist_class, (t_method)playlist_location, gensym("location"), A_SYMBOL, 0 );
    class_addmethod(playlist_class, (t_method)playlist_dialog, gensym("dialog"), A_GIMME, 0 );

    playlist_widgetbehavior.w_getrectfn =    playlist_getrect;
    playlist_widgetbehavior.w_displacefn =   playlist_displace;
    playlist_widgetbehavior.w_selectfn =     playlist_select;
    playlist_widgetbehavior.w_activatefn =   NULL;
    playlist_widgetbehavior.w_deletefn =     playlist_delete;
    playlist_widgetbehavior.w_visfn =        playlist_vis;
    playlist_widgetbehavior.w_clickfn =      playlist_click;
    playlist_widgetbehavior.w_propertiesfn = playlist_properties;
    playlist_widgetbehavior.w_savefn =       playlist_save;
    class_setwidget(playlist_class, &playlist_widgetbehavior);
    class_sethelpsymbol(playlist_class, gensym("help-playlist.pd"));
}
