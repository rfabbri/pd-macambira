
#define UPDATE 0
#define CREATE 1
#define DESTROY 2
/*
static int pmenu_w_is_visible(t_pmenu* x) {
	
	return ( x->x_glist != NULL);

}

static void pmenu_w_text(t_pmenu* x, t_symbol* s) {
	
	sys_vgui(".x%x.c itemconfigure %xLABEL -text {%s} \n",
                 glist_getcanvas(x->x_glist), x, s->s_name);
                 

}
*/

static void pmenu_w_activate(t_pmenu* x){
	//sys_vgui("$%xw activate %i\n", x,x->current_selection);
	sys_vgui("set %xradio %i\n",x,x->current_selection); 
}


static void pmenu_w_clear(t_pmenu* x){
	sys_vgui("$%xw delete 0 end \n", x);
	sys_vgui("set %xradio %i\n",x,-1); 
}

static void pmenu_w_additem(t_pmenu* x, t_symbol *label, int i) {
	
	sys_vgui("$%xw add radiobutton -label \"%s\" -command {select%x \"%d\"} -variable %xradio -value %d \n",
                   x, label->s_name, x, i,x,i);
}

static void pmenu_w_apply_colors(t_pmenu* x) {
	
	sys_vgui("$%xw configure -background \"%s\" -foreground \"%s\" -activeforeground \"%s\" -activebackground \"%s\" -selectcolor \"%s\"\n", x,
	x->bg_color->s_name,x->fg_color->s_name,x->fg_color->s_name,x->hi_color->s_name,x->fg_color->s_name);
	
	/*
	sys_vgui(".x%x.c.s%x configure -background \"%s\" -foreground \"%s\" -activeforeground \"%s\" -activebackground \"%s\"\n", 
	x->x_glist, x, x->bg_color->s_name,x->fg_color->s_name,x->fg_color->s_name,x->hi_color->s_name);
	
	sys_vgui(".x%x.c.s%x.menu configure -background \"%s\" -foreground \"%s\" -activeforeground \"%s\" -activebackground \"%s\"\n", 
	x->x_glist, x, x->bg_color->s_name,x->fg_color->s_name,x->fg_color->s_name,x->hi_color->s_name);

    sys_vgui(".x%x.c itemconfigure  %xR -outline \"%s\"\n", x->x_glist, x,x->co_color->s_name);
    * */
}



/*
static void pmenu_w_draw_inlets(t_pmenu *x, t_glist *glist, int draw, int nin, int nout)
{
	
 // outlets
     int n = nin;
     int nplus, i;
     nplus = (n == 1 ? 1 : n-1);
     DEBUG(post("draw inlet");)
     for (i = 0; i < n; i++)
     {
	  int onset = text_xpix(&x->x_obj, glist) + (x->x_width - IOWIDTH) * i / nplus;
	  if (draw==CREATE) {
	       sys_vgui(".x%x.c create rectangle %d %d %d %d -outline blue -tags {%xo%d %xo}\n",
			glist_getcanvas(glist),
			onset, text_ypix(&x->x_obj, glist) + x->x_height + 1 ,
			onset + IOWIDTH, text_ypix(&x->x_obj, glist) + x->x_height+2,
			x, i, x);
	  } else if (draw==UPDATE) {
	       sys_vgui(".x%x.c coords %xo%d %d %d %d %d\n",
			glist_getcanvas(glist), x, i,
			onset, text_ypix(&x->x_obj, glist) + x->x_height +1,
			onset + IOWIDTH, text_ypix(&x->x_obj, glist) + x->x_height+2);
		} else {
			
			sys_vgui(".x%x.c delete %xo\n",glist_getcanvas(glist),x); // Added tag for all outlets of one instance 
		}
     }
 // inlets 
     n = nout; 
     nplus = (n == 1 ? 1 : n-1);
     for (i = 0; i < n; i++)
     {
	  int onset = text_xpix(&x->x_obj, glist) + (x->x_width - IOWIDTH) * i / nplus;
	  if (draw==CREATE) {
	       sys_vgui(".x%x.c create rectangle %d %d %d %d -outline blue -tags {%xi%d %xi}\n",
			glist_getcanvas(glist),
			onset, text_ypix(&x->x_obj, glist)-2,
			     onset + IOWIDTH, text_ypix(&x->x_obj, glist)-1,
			x, i, x);
	  } else if (draw==UPDATE) {
	       sys_vgui(".x%x.c coords %xi%d %d %d %d %d\n",
			glist_getcanvas(glist), x, i,
			onset, text_ypix(&x->x_obj, glist)-2,
			onset + IOWIDTH, text_ypix(&x->x_obj, glist)-1);
	  } else {
		  sys_vgui(".x%x.c delete %xi\n",glist_getcanvas(glist),x); // Added tag for all inlets of one instance
	  }
     }
     DEBUG(post("draw inlet end");)
     
}
*/


static void pmenu_w_menu(t_pmenu *x, int draw)
{
  DEBUG(post("menu start");)
  
  if ( draw == CREATE ) {
	  //x->created = 1;
	   // Create menu 
	   //sys_vgui("set %xw .x%x.c.s%x ; menubutton $%xw -justify left 
	   // Create a variable to store a pointer to the menu, create the menu, create a variable to store the selected item
      sys_vgui("set %xw .%x ; menu $%xw -relief solid -tearoff 0; set %xradio 0 \n",x,x,x);
      int i;
	  for(i=0 ; i<x->x_num_options ; i++)
        {
			// Add menu itmes
          pmenu_w_additem(x,x->x_options[i],i);
        }
  } else if ( draw == DESTROY) {
	  //x->created = 0;
	  sys_vgui("destroy $%xw \n",x);
  }

  //DEBUG(post("id: .x%x.c.s%x", glist, x);)
  DEBUG(post("menu end");)
}




/*
static void pmenu_w_draw(t_pmenu *x, t_glist *glist, int draw)
{
	
	 int xpos = text_xpix(&x->x_obj,glist);
     int ypos = text_ypix(&x->x_obj,glist);
	 t_canvas* canvas = glist_getcanvas(glist);
  //t_canvas *canvas=glist_getcanvas(glist);
  DEBUG(post("drawme start");)

  DEBUG(post("drawme %d",draw);)
     if (draw==CREATE) {
		// Base
		
		sys_vgui(".x%x.c create rectangle %d %d %d %d -tags %xR -fill \"%s\" -outline \"%s\" \n",
	     canvas, xpos, ypos ,
	     xpos + x->x_width, ypos + x->x_height,
	     x,x->bg_color->s_name,x->co_color->s_name); 
	     
	    // Text
	    sys_vgui(".x%x.c create text %d %d -text {%s} -anchor w -fill \"%s\" -tags %xLABEL -width %i \n" ,
             canvas, xpos+2, ypos+x->x_height/2,
             "",x->fg_color->s_name, x, x->x_width-2,x->x_height -2); 
             //-font {{%s} -%d %s}
             //x->x_gui.x_font, x->x_gui.x_fontsize, sys_fontweight
     } else if (draw==UPDATE) {
		 // Base
		sys_vgui(".x%x.c coords %xR %d %d %d %d\n",
	     canvas, x, 
	     xpos, ypos ,
	     xpos + x->x_width, ypos + x->x_height );
		// Text
		sys_vgui(".x%x.c coords %xLABEL %d %d\n",
             canvas, x, xpos+2,ypos+x->x_height/2);
     } else {
		 // Base
		sys_vgui(".x%x.c delete  %xR\n",canvas,x);
		// Text
		sys_vgui(".x%x.c delete %xLABEL\n", canvas, x);
		
		DEBUG(post("erase done");)
       }
		
  DEBUG(post("drawme end");)
}
*/
/*
static void  pmenu_w_resize(t_pmenu* x) {
	
	//sys_vgui(".x%x.c itemconfigure %xS -width %i -height %i \n", x->x_glist, x,x->x_width-1,x->x_height-1);
	
	
	pmenu_w_draw(x,x->x_glist,UPDATE);
	canvas_fixlinesfor(x->x_glist,(t_text*) x);
}
*/
/*
static void pmenu_w_getrect(t_gobj *z, t_glist *owner,
    int *xp1, int *yp1, int *xp2, int *yp2)
{
 // DEBUG(post("getrect start");)

    int width, height;
    t_pmenu* s = (t_pmenu*)z;

    width = s->x_width;
    height = s->x_height;
    *xp1 = text_xpix(&s->x_obj, owner) ;
    *yp1 = text_ypix(&s->x_obj, owner) ;
    *xp2 = text_xpix(&s->x_obj, owner) + width  ;
    *yp2 = text_ypix(&s->x_obj, owner) + height ;
  
   // DEBUG(post("getrect end");)
}


static void pmenu_w_displace(t_gobj *z, t_glist *glist,
    int dx, int dy)
{
    t_pmenu *x = (t_pmenu *)z;
    DEBUG(post("displace start");)
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
   
      //pmenu_w_draw_handle(x,glist,UPDATE);
      //pmenu_w_draw_inlets(x, glist, UPDATE, 1,1);
      pmenu_w_draw(x, glist, UPDATE);
      canvas_fixlinesfor(glist_getcanvas(glist),(t_text*) x);
    
    DEBUG(post("displace end");)
}

static void pmenu_w_select(t_gobj *z, t_glist *glist, int state)
{
     DEBUG(post("select start");)

     t_pmenu *x = (t_pmenu *)z;
     
		 
		  //sys_vgui(".x%x.c itemconfigure %xhandle -fill %s\n", glist, 
	     //x, (state? "blue" : x->fg_color->s_name));
		 
       if (state) {
		    //pmenu_w_draw_handle(x,glist,CREATE);
		    pmenu_w_draw_inlets(x, glist, CREATE, 1,1);
		    //pmenu_w_disable(x,1);
       }
       else {
		   //pmenu_w_disable(x,0);
		 //pmenu_w_draw_handle(x,glist,DESTROY);
         pmenu_w_draw_inlets(x,glist,DESTROY,1,1);
         
       }
     

     DEBUG(post("select end");)
}


// Standard delete function
static void pmenu_w_delete(t_gobj *z, t_glist *glist)
{

   canvas_deletelinesfor(glist, (t_text *)z);

}

// Standard create function      
static void pmenu_w_vis(t_gobj *z, t_glist *glist, int vis)
{
    t_pmenu* x = (t_pmenu*)z;
    t_rtext *y;
    DEBUG(post("vis start");)
    DEBUG(post("vis: %d",vis);)
    if (vis) {
		x->x_glist = glist;
		
	    pmenu_w_draw(x, glist, CREATE);
    } else {
		x->x_glist = NULL;
		//if ( x->created ) pmenu_w_menu(x, glist,DESTROY);
		pmenu_w_draw(x, glist, DESTROY);
    }

    DEBUG(post("vis end");)
}
*/

static void pmenu_w_pop(t_pmenu *x) {
	
	if (x->x_num_options > 0) {
			 //if ( x->created == 0 )  pmenu_w_menu(x, glist, CREATE);
			 if ( x->current_selection != -1 ) {
				 sys_vgui("tk_popup $%xw [winfo pointerx .] [winfo pointery .] %i\n",x,x->current_selection);
			 } else {
				sys_vgui("tk_popup $%xw [winfo pointerx .] [winfo pointery .] 1\n",x);
		   	}
		 }
	
}

/*   
static int pmenu_w_click(t_pmenu *x, struct _glist *glist,
    int xpos, int ypos, int shift, int alt, int dbl, int doit) {
		
		DEBUG(post("x: %i,y: %i,shift: %i, alt: %i, dbl: %i, doit: %i",xpos,ypos,shift,alt,dbl,doit);)
		
		//if (doit) sys_vgui("tk_popup $%xw %i %i \n",x,100,100);
		
		if (doit && x->x_num_options > 0) {
			 //if ( x->created == 0 )  pmenu_w_menu(x, glist, CREATE);
			 if ( x->current_selection != -1 ) {
				 sys_vgui("tk_popup $%xw [winfo pointerx .] [winfo pointery .] %i\n",x,x->current_selection);
			 } else {
				sys_vgui("tk_popup $%xw [winfo pointerx .] [winfo pointery .] \n",x);
		   	}
		 }
		return (1);
}
*/

/*
t_widgetbehavior   pmenu_widgetbehavior = {
  w_getrectfn:  pmenu_w_getrect,
  w_displacefn: pmenu_w_displace,
  w_selectfn:   pmenu_w_select,
  w_activatefn: NULL,
  w_deletefn:   pmenu_w_delete,
  w_visfn:      pmenu_w_vis,
  //w_clickfn:    NULL,
  w_clickfn:    (t_clickfn)pmenu_w_click,
}; 
*/
