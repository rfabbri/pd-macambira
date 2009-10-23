
static t_class *paramGui_class;
 

typedef struct _paramGui
{
  t_object					x_obj;
  //t_outlet*					outlet;
  //t_symbol*					canvasname;
  t_canvas*					childcanvas;
  //t_symbol*                 fullpath;
  t_symbol*                 path;
  int                       path_l;
  int                       build;
  t_symbol*                 s_vis;
  t_symbol*					s_empty;
  t_symbol*                 s_clear;
  t_symbol*                 s_set;
  t_symbol*                 root;
  t_symbol*                 s_obj;
  t_symbol*                 s_nbx;
  t_symbol*                 s_bng;
  t_symbol*                 s_slider;
  t_symbol*                 s_hsl;
  t_symbol*                 s_knob;
  t_symbol*                 s_tgl;
  t_symbol*                 s_symbolatom;
  t_symbol*                 s_sym;
  t_symbol*                 s_text;
  
  //t_symbol*					target;
  //t_class*					empty_s;
  //t_symbol*					root;

} t_paramGui;



// Dump out
static void paramGui_bang(t_paramGui *x) {
    
    if (x->childcanvas) {
        
        if (x->build) {
            // Clear the canvas
            pd_typedmess((t_pd*)x->childcanvas,x->s_clear,0,NULL);
            
            int pos_x = 0;
            int pos_y = 0;
            t_atom atoms[22]; // This should be the maximum number of atoms
            
            t_param* p = get_param_list(x->root);
            int ac;
            t_atom* av;
            t_symbol* type;
            t_symbol* send;
            t_symbol* receive;
            
            int gui_built = 1;
            
            // ac & av for updating the values of the gui (p->get())
            int ac_got = 0;
            t_atom* av_got;
            t_symbol* s_got;
             
            while (p) {
                gui_built = 1;
                    if (p->GUI && (strncmp(p->path->s_name,x->path->s_name,x->path_l)==0)) {
                        p->GUI(p->x,&ac,&av,&send,&receive);
                        if ( send == NULL ) send = x->s_empty;
                        if ( receive == NULL ) receive = x->s_empty;
                        if ( IS_A_SYMBOL(av,0)) {
                           type = atom_getsymbol(av);
                           if ( type == x->s_nbx ) {
                                SETSYMBOL(&atoms[0],x->s_obj);
                                SETFLOAT(&atoms[1],pos_x);
                                SETFLOAT(&atoms[2],pos_y);
                                SETSYMBOL(&atoms[3],x->s_nbx);
                                SETFLOAT(&atoms[4],5);
                                SETFLOAT(&atoms[5],14);
                                SETFLOAT(&atoms[6],-1.0e+37);
                                SETFLOAT(&atoms[7],1.0e+37);
                                SETFLOAT(&atoms[8],0);
                                SETFLOAT(&atoms[9],0);
                                SETSYMBOL(&atoms[10],send);
                                SETSYMBOL(&atoms[11],receive);
                                SETSYMBOL(&atoms[12],p->path);
                                SETFLOAT(&atoms[13],50);
                                SETFLOAT(&atoms[14],8);
                                SETFLOAT(&atoms[15],0);
                                SETFLOAT(&atoms[16],8);
                                SETFLOAT(&atoms[17],-262144);
                                SETFLOAT(&atoms[18],-1);
                                SETFLOAT(&atoms[19],-1);
                                SETFLOAT(&atoms[20],0);
                                SETFLOAT(&atoms[21],256);
                                pd_forwardmess((t_pd*)x->childcanvas, 22, atoms);
                                pos_y = pos_y + 18;
                                
                            } else if (type == x->s_bng) {
                                SETSYMBOL(&atoms[0],x->s_obj);
                                SETFLOAT(&atoms[1],pos_x);
                                SETFLOAT(&atoms[2],pos_y);
                                SETSYMBOL(&atoms[3],x->s_bng);
                                SETFLOAT(&atoms[4],15);
                                SETFLOAT(&atoms[5],250);
                                SETFLOAT(&atoms[6],50);
                                SETFLOAT(&atoms[7],0);
                                SETSYMBOL(&atoms[8],send);
                                SETSYMBOL(&atoms[9],receive);
                                SETSYMBOL(&atoms[10],p->path);
                                SETFLOAT(&atoms[11],17);
                                SETFLOAT(&atoms[12],7);
                                SETFLOAT(&atoms[13],0);
                                SETFLOAT(&atoms[14],8);
                                SETFLOAT(&atoms[15],-262144);
                                SETFLOAT(&atoms[16],-1);
                                SETFLOAT(&atoms[17],-1);
                                pd_forwardmess((t_pd*)x->childcanvas, 18, atoms);
                                pos_y = pos_y + 18;
                            } else if ( (type == x->s_slider) || (type == x->s_knob) || (type == x->s_hsl) ) {
                                SETSYMBOL(&atoms[0],x->s_obj);
                                SETFLOAT(&atoms[1],pos_x);
                                SETFLOAT(&atoms[2],pos_y);
                                SETSYMBOL(&atoms[3],x->s_hsl);
                                SETFLOAT(&atoms[4],100);
                                SETFLOAT(&atoms[5],15);
                                if (ac > 1 && IS_A_FLOAT(av,1) ) {
                                     SETFLOAT(&atoms[6],atom_getfloat(av+1));
                                 } else {
                                     SETFLOAT(&atoms[6],0);
                                 }
                                 if (ac > 2 && IS_A_FLOAT(av,2) ) {
                                     SETFLOAT(&atoms[7],atom_getfloat(av+2));
                                 } else {
                                     SETFLOAT(&atoms[7],1);
                                 }
                                SETFLOAT(&atoms[8],0);
                                SETFLOAT(&atoms[9],0);
                                SETSYMBOL(&atoms[10],send);
                                SETSYMBOL(&atoms[11],receive);
                                SETSYMBOL(&atoms[12],p->path);
                                SETFLOAT(&atoms[13],105);
                                SETFLOAT(&atoms[14],7);
                                SETFLOAT(&atoms[15],0);
                                SETFLOAT(&atoms[16],8);
                                SETFLOAT(&atoms[17],-262144);
                                SETFLOAT(&atoms[18],-1);
                                SETFLOAT(&atoms[19],-1);
                                SETFLOAT(&atoms[20],0);
                                SETFLOAT(&atoms[21],1);
                                pd_forwardmess((t_pd*)x->childcanvas, 22, atoms);
                                pos_y = pos_y + 18;
                                
                            } else if (type == x->s_tgl) {
                                SETSYMBOL(&atoms[0],x->s_obj);
                                SETFLOAT(&atoms[1],pos_x);
                                SETFLOAT(&atoms[2],pos_y);
                                SETSYMBOL(&atoms[3],x->s_tgl);
                                SETFLOAT(&atoms[4],15);
                                SETFLOAT(&atoms[5],0);
                                SETSYMBOL(&atoms[6],send);
                                SETSYMBOL(&atoms[7],receive);
                                SETSYMBOL(&atoms[8],p->path);
                                SETFLOAT(&atoms[9],17);
                                SETFLOAT(&atoms[10],7);
                                SETFLOAT(&atoms[11],0);
                                SETFLOAT(&atoms[12],8);
                                SETFLOAT(&atoms[13],-262144);
                                SETFLOAT(&atoms[14],1);
                                SETFLOAT(&atoms[15],-1);
                                SETFLOAT(&atoms[16],0);
                                SETFLOAT(&atoms[17],1);
                                pd_forwardmess((t_pd*)x->childcanvas, 18, atoms);
                                pos_y = pos_y + 18;
                                
                                
                            } else if ( type == x->s_symbolatom || type == x->s_sym) {
                                SETSYMBOL(&atoms[0],x->s_symbolatom);
                                SETFLOAT(&atoms[1],pos_x);
                                SETFLOAT(&atoms[2],pos_y);
                                SETFLOAT(&atoms[3],17);
                                SETFLOAT(&atoms[4],0);
                                SETFLOAT(&atoms[5],0);
                                SETFLOAT(&atoms[6],1);
                                SETSYMBOL(&atoms[7],p->path);
                                SETSYMBOL(&atoms[8],receive);
                                SETSYMBOL(&atoms[9],send);
                                pd_forwardmess((t_pd*)x->childcanvas, 10,atoms);
                                pos_y = pos_y + 18;
                            } else {
                                SETSYMBOL(&atoms[0],x->s_text);
                                SETFLOAT(&atoms[1],pos_x);
                                SETFLOAT(&atoms[2],pos_y);
                                SETSYMBOL(&atoms[3],p->path);
                                pd_forwardmess((t_pd*)x->childcanvas, 4,atoms);
                                pos_y = pos_y + 18;
                                gui_built = 0;
                            }
                            
                            if ((gui_built) && (receive != x->s_empty) && (p->get)) {
                                p->get(p->x,&s_got,&ac_got,&av_got);
                                tof_send_anything_prepend(receive,s_got,ac_got,av_got,x->s_set);
                            }
                        }
                    }
                p = p->next;
               
            }
        }
        
        x->build = 0;
        
        // Show canvas
        t_atom a;
        SETFLOAT(&a,1);
        pd_typedmess((t_pd*)x->childcanvas,x->s_vis,1,&a);
        
    }  else {
        
       pd_error(x,"No canvas to write to!");
       
   }
}

static void paramGui_reset(t_paramGui *x) {
    x->build = 1;
    paramGui_bang(x);
}


static void paramGui_free(t_paramGui *x)
{
	if (x->childcanvas) {
		//post("Deleting it");
		pd_free((t_pd *)x->childcanvas);
	}
	x->childcanvas = NULL;
}

/*
static void paramGui_properties(t_gobj*z, t_glist*owner) {
  t_iemguts_objlist*objs=objectsInCanvas((t_pd*)z);
   while(objs) {
    t_propertybang*x=(t_propertybang*)objs->obj;
    propertybang_bang(x);
    objs=objs->next;
  } 
}
*/

static void *paramGui_new(t_symbol *s, int ac, t_atom *av) {
  t_paramGui *x = (t_paramGui *)pd_new(paramGui_class);
  
  
  x->build = 1;
  
  x->s_vis = gensym("vis");
  x->s_empty = gensym("empty");
  x->s_clear = gensym("clear");
  x->s_set = gensym("set");
  x->s_obj = gensym("obj");
  x->s_nbx = gensym("nbx");
  x->s_bng = gensym("bng");
  x->s_slider = gensym("slider");
  x->s_hsl=gensym("hsl");
  x->s_knob = gensym("knob");
  x->s_tgl = gensym("tgl");
  x->s_symbolatom = gensym("symbolatom");
  x->s_sym = gensym("sym");
  x->s_text = gensym("text");
  
  
  
  t_canvas* currentcanvas = tof_get_canvas();
  
  x->root = tof_get_dollarzero(tof_get_root_canvas(currentcanvas));
  
  
  x->path = param_get_path(currentcanvas, NULL);
  x->path_l = strlen(x->path->s_name);
  // Prepend $0 to path 
  t_symbol* dollarzero = tof_get_dollarzero(currentcanvas);
  int zeropath_len = strlen(dollarzero->s_name)+strlen(x->path->s_name)+1;
  char* zeropath = getbytes(zeropath_len * sizeof(* zeropath));
  strcpy(zeropath,dollarzero->s_name);
  strcat(zeropath,x->path->s_name);
  t_symbol* fullpath = gensym(zeropath);
  freebytes(zeropath,zeropath_len * sizeof(* zeropath));
  
  //post("path: %s",x->path->s_name);
  
  // create a new canvas
    
    
  t_atom a;
  SETSYMBOL(&a, fullpath);
  pd_typedmess(&pd_objectmaker,gensym("pd"),1,&a);
  
    // From this point on, we are hoping the "pd" object has been created
    x->childcanvas = (t_canvas*) pd_newest();
   
	
    // Hide the window (stupid way of doing this)
    if (x->childcanvas) {
		SETFLOAT(&a,0);
		pd_typedmess((t_pd*)x->childcanvas,gensym("vis"),1,&a);
    }
	
   
   inlet_new(&x->x_obj, &x->x_obj.ob_pd,gensym("bang"), gensym("reset"));

  

  // SET THE PROPERTIES FUNCTION
  //t_class *class = ((t_gobj*)currentcanvas)->g_pd;
  //class_setpropertiesfn(class, propertybang_properties);

    
  return (x);
}

void paramGui_setup(void) {
  paramGui_class = class_new(gensym("param gui"),
    (t_newmethod)paramGui_new, (t_method)paramGui_free,
    sizeof(t_paramGui), 0, A_GIMME, 0);

 class_addbang(paramGui_class, paramGui_bang);
 //class_addsymbol(paramGui_class, paramGui_symbol);

 class_addmethod(paramGui_class, (t_method) paramGui_reset, gensym("reset"), 0);
 
 
 class_sethelpsymbol(paramGui_class,gensym("param"));
 //class_addmethod(paramGui_class, (t_method) paramGui_guis, gensym("guis"), A_DEFSYMBOL,0);
 //class_addmethod(paramGui_class, (t_method) paramGui_updateguis, gensym("updateguis"), A_DEFSYMBOL,0);

 
 //class_addmethod(paramGui_class, (t_method) paramGui_update_guis, gensym("update"), A_DEFSYMBOL,0);

}
