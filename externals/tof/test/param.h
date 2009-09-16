
#include <stdio.h>

struct param {
   t_symbol* 	selector; //Type of data stored
   int 			ac; //Number of values stored
   int			 alloc; //Memory allocated
   t_atom* 		av; //Values stored
   t_symbol* 	path; //Path(name) of the param
   t_symbol*    path_g;
   //t_symbol* 	basepath;
   struct param* next; //Next param
   struct param* previous; //Previous param
   int 			users; //Number of param objects using this param
   t_symbol* 	id; //An id set only if it is saveable
   int 			ac_g;
   t_atom*		av_g;
};


struct param* paramlist;


typedef struct param_build_info {
	int ac;
	t_atom* 	av;
	t_symbol* 	path;
	t_symbol* 	id;
	t_symbol*	path_g;
	int 		ac_g;
    t_atom*		av_g;
	t_symbol* 	basepath;
	t_symbol* 	basename;
}t_param_build_info;

static void set_param_anything( struct param* p, t_symbol* s, int ac, t_atom *av) {
	
	if ( s == &s_bang ) {
		p->ac = 0;
		p->selector = s;
	} else {
		if(ac > p->alloc) {	
			p->av = resizebytes(p->av, p->alloc*sizeof(*(p->av)), 
				(10 + ac)*sizeof(*(p->av)));
			p->alloc = 10 + ac;
		}
		p->ac = ac;
		p->selector = s;
		tof_copy_atoms(av, p->av, ac);
    }
}


static void set_param( struct param* p, int ac, t_atom *av) {
	t_symbol* s;
	tof_set_selector(&s,&ac, &av );
	set_param_anything(p,s,ac,av);
}






static struct param* get_param_list(void) {
	
	if (paramlist == NULL) {
		//post("No params found");
		return NULL;
	 }
	
    return paramlist;
	
}

static void print_all_params(void) {
	
	struct param* p = paramlist;
	post("--paramlist--");
		while(p) {
			if ( p->path) post("Path: %s",p->path->s_name);
			if (p->id) post("Id: %s",p->id->s_name);
			p = p->next;
	}
	
}


//static struct param* register_param( t_symbol* path, int ac, t_atom* av, t_symbol* id
static struct param* register_param( t_param_build_info* build) {
	
	//post("registering %s", path->s_name);
	t_symbol* path = build->path;
	t_symbol* path_g = build->path_g;
	int ac = build->ac;
	t_atom* av = build->av;
	t_symbol* id = build->id;
	int ac_g = build->ac_g;
	t_atom* av_g = build->av_g;
	
	
	//if ( path) post("path:%s",path->s_name);
	//if ( id) post("id:%s",id->s_name);
	
        struct param* last = paramlist;
		// Search for param with same path
		while( last ) {
			if ( last->path == path) {
				//post("Found param with same name");
				last->users = last->users + 1;
				return last;
			}
			if ( last->next == NULL ) break;
			last = last->next; 
		}
		
		// Create and add param to the end
		struct param* p = getbytes(sizeof(*p));
		p->alloc = 0;
		p->path = path;
		p->path_g = path_g;
		p->next = NULL;
		p->users = 1;
		p->id = id;
		set_param( p, ac, av);
		p->ac_g = ac_g;
		p->av_g = getbytes(ac_g*sizeof(*(p->av_g)));
		tof_copy_atoms(av_g,p->av_g,ac_g);
		if (last) {
			//post("Appending param");
			p->previous = last;
			last->next = p;
		} else {
			//post("Creating first param");
			p->previous = NULL;
			paramlist = p;
		}
		
		
		//print_all_params();
		
	return p;
	
}

static void unregister_param( struct param* p) {
	
	//post("unregistering %s", p->path->s_name);
	
	if ( paramlist) {

		p->users = p->users - 1;
		if ( p->users == 0 ) {
			// Remove param
			//post("Removing last param of this name");
			if (p->previous) {
				p->previous->next = p->next;
				if (p->next) p->next->previous = p->previous;
				/*
				if (p->next == NULL) {
					p->previous->next = NULL;
				} else {
					p->previous->next = p->next;
				}
				*/
			} else {
				paramlist = p->next;
				if ( p->next != NULL) p->next->previous = NULL;
			}
			freebytes(p->av, p->alloc * sizeof *(p->av) );
			freebytes(p->av_g, p->ac_g * sizeof *(p->av_g) );
			freebytes(p, sizeof *p);
		}
		
	} else {
		post("Euh... no params found!");
	}
	
	//print_all_params();
	
}

////////////////////



static void param_find_value(t_symbol *name, int ac, t_atom *av, int *ac_r,t_atom** av_r) {
	
	int i;
	int j = 0;
	for (i=0;i<ac;i++) {
		//if ( IS_A_SYMBOL(av,i)) post("analyzing %s",atom_getsymbol(av+i)->s_name);
		if ( IS_A_SYMBOL(av,i) && name == atom_getsymbol(av+i) && (i+1)<ac ) {
			//post("matches");
			i=i+1;
			for (j=i;j<ac;j++) {
				if (  IS_A_SYMBOL(av,j) && (atom_getsymbol(av+j))->s_name[0] == '/' ) {
					//j = j-1;
					break;
				}
			}
			break;
		}
	}
	j = j-i;
	//post("i:%d j:%d",i,j);
	
	
	if ( j > 0) {
		*ac_r = j;
		*av_r = av+i;
		//x->x_param = register_param( x->x_path , j, av+i,saveable);
	} else {
		*ac_r = 0;
	}
 }


static void get_param_build_info(t_canvas* canvas, int o_ac, t_atom* o_av, struct param_build_info* pbi, int flag) {
	
	 pbi->path = NULL;
	 pbi->path_g = NULL;
	 pbi->av = NULL;
	 int saveable = 1;
	 pbi->ac = 0;
	 pbi->id = NULL;
	 pbi->ac_g = 0;
	 pbi->av_g = NULL;
	 pbi->basepath = NULL;
	 pbi->basename = NULL;
	 
	 int ac;
	 t_atom* av;
	 tof_get_canvas_arguments(canvas,&ac, &av);
	
	char *separator = "/";
	char sbuf_name[MAXPDSTRING];
	char sbuf_temp[MAXPDSTRING];
	sbuf_name[0] = '\0';
	sbuf_temp[0] = '\0';
	
   t_symbol* id = gensym("/id");
   t_canvas* id_canvas = canvas;
   t_canvas* i_canvas = canvas;
   
   int i;
   
   int i_ac;
   t_atom * i_av;
   
   // A simple flag to indicate if an ID was found
   int found_id_flag;
   
   
   t_symbol* id_s = NULL;
   
   // A HACK to find out if we are in a subpatch or an abstraction 
   // A subpatch always has the same $0 as it's parent
   //t_symbol* p_id_s = canvas_realizedollar(i_canvas, gensym("$0"));
  //t_symbol* p_id_s = gensym("");
   
   while( i_canvas->gl_owner) {
	   // Ignore all supatches
	   if ( tof_canvas_is_not_subpatch(i_canvas) ) {
		tof_get_canvas_arguments(i_canvas,&i_ac, &i_av);
		id_s= canvas_realizedollar(i_canvas, gensym("$0"));
	
		//if (id_s != p_id_s) {
		//	p_id_s = id_s;
		
		int start = 0;
		int count = 0;
		found_id_flag = 0;
		
		while( tof_get_tagged_argument('/',i_ac,i_av,&start,&count) ) {
			
			if ( IS_A_SYMBOL(i_av,start)
			   && (id == (i_av+start)->a_w.w_symbol) 
			   && (count > 1) ) {  
	           	id_s = atom_getsymbol(i_av+start+1);
	           	id_canvas = i_canvas;
				found_id_flag = 1;
	           	break;
			}
			start= start + count;
		}
		        
				
        // if ever an /id is missing, this param is not saveable
        if (found_id_flag == 0)  saveable = 0;
        
		
		//if (id_s != p_id_s) {
	   // Prepend newly found ID
		   strcpy(sbuf_temp,sbuf_name);
		   strcpy(sbuf_name, "/");
		   strcat(sbuf_name, id_s->s_name);
		   strcat(sbuf_name,sbuf_temp);  
        //}
	     //  p_id_s = id_s;
		   
     //} 
		}
        i_canvas = i_canvas->gl_owner;
    } 
    
	if ( saveable ) {
		
		if ( id_s) {
			
			int id_buf_length = strlen(id_s->s_name)+2;
			char * id_buf = getbytes(id_buf_length * sizeof (*id_buf));
			strcpy(id_buf, "/");
			strcat(id_buf,id_s->s_name);
			pbi->id = gensym(id_buf);
			freebytes(id_buf,id_buf_length * sizeof (*id_buf));
		} else {
			pbi->id = gensym("/");
		}
		
	}
	
	pbi->basepath = gensym(sbuf_name);
	
	if (flag) {
		
			// FIND NAME
		  t_symbol* midpath = NULL;
		  //t_symbol* basename = NULL;
		  
		  if (o_ac  && IS_A_SYMBOL(o_av, 0)) {
			char *firstChar =  (atom_getsymbol(o_av))->s_name;
			if (*firstChar == (char)'/') {
				 strcat(sbuf_name, atom_getsymbol(o_av)->s_name);
				  pbi->path = gensym(sbuf_name);
				  strcat(sbuf_temp, atom_getsymbol(o_av)->s_name);
				  midpath = gensym(sbuf_temp);
				  strcat(sbuf_name, "_");
				  pbi->path_g = gensym(sbuf_name);
				  pbi->basename = atom_getsymbol(o_av);
			 }
		  } 
		  
		  // if ( x->x_param )
		  
		  
		  if ( pbi->path) {
		  
		  
		  // FIND VALUE
		  // A. Find in SUB canvas arguments
		  // B. In canvas' arguments
		  // C. In object's arguments
		  // D. Defaults to a bang
		  
		  int p_ac =0;
		  t_atom* p_av;
		  
		  
		   // A. If name, try to find value in ID canvas' arguments
		  if ( midpath) {
			// GET ID CANVAS ARGUMENTS (may be the same as the local canvas)
			//int i_ac;
			//t_atom * i_av;
			tof_get_canvas_arguments(id_canvas,&i_ac , &i_av);
			param_find_value(midpath, i_ac, i_av,&(pbi->ac),&(pbi->av));
		  } 
		  
		  // B. If basename, try to find value in LOCAL canvas' arguments
		  if ( pbi->basename && pbi->ac == 0 ) {
		
			param_find_value(pbi->basename, ac, av,&(pbi->ac),&(pbi->av));
		  } 
		  
		  
			// C. If no value found in canvas' arguments, check the object's arguments
		  if ( pbi->ac == 0  && o_ac > 1) {
			int start = 1;
			int count = 0;
			tof_get_tagged_argument('/',o_ac,o_av,&start,&count);
			if (count > 0) {
				pbi->ac = count;
				pbi->av = o_av + start;
			}
		  }
		 
		 //FIND THE GUI TAGS
		 param_find_value(gensym("/gui"), o_ac, o_av,&(pbi->ac_g),&(pbi->av_g));
		 //post("GUI COUNT:%d",pbi->ac_g);
		  
	}	
	
}
}


static void param_send_prepend(struct param *p, t_symbol* s,t_symbol* prepend) {
	
	if (p) {
	   if((p->selector == &s_bang)) {
			// if (s->s_thing) 
			  //   pd_bang(s->s_thing);
	  } else {
		if (s->s_thing) {
			if ( p->selector == &s_list || p->selector == &s_float || p->selector == &s_symbol ) {
				typedmess(s->s_thing, prepend, p->ac, p->av);
			} else {
			int ac = p->ac + 1;
			t_atom *av = getbytes(ac*sizeof(*av));	
			tof_copy_atoms(p->av,av+1,p->ac);
			SETSYMBOL(av, p->selector);
			typedmess(s->s_thing, prepend, ac, av);
			freebytes(av, ac*sizeof(*av));
		}
		}
	  }
	}
}

static void param_output(struct param *p, t_outlet* outlet) {
	// SHOULD I COPY THIS DATA BEFORE SENDING IT OUT?
	// OR IS THE NORM TO ONLY COPY ON INPUT?
	if (p) {
		if((p->selector == &s_bang) ) {
			outlet_bang(outlet);
		} else {
			outlet_anything(outlet, p->selector, p->ac, p->av);
		}
	}	
}


static void param_output_prepend(struct param* p, t_outlet* outlet, t_symbol* s) {
	
	if (p->selector == &s_list || p->selector == &s_float  || p->selector == &s_symbol) {
			//t_atom *av = (t_atom *)getbytes(p->ac*sizeof(t_atom));	
			//tof_copy_atoms(p->av,av,p->ac);
			outlet_anything(outlet,s,p->ac,p->av);
			//freebytes(av, p->ac*sizeof(t_atom));
	} else if (p->selector != &s_bang) {
			int ac = p->ac + 1;
			t_atom *av = (t_atom *)getbytes(ac*sizeof(t_atom));	
			tof_copy_atoms(p->av,av+1,p->ac);
			SETSYMBOL(av, p->selector);
			outlet_anything(outlet,s,ac,av);
			freebytes(av, ac*sizeof(t_atom));
	}
	
}



static int param_write(t_canvas* canvas, t_symbol* filename, t_symbol* id) {
	
	int w_error;
	
	//t_symbol* filename = param_makefilename(basename, n);
		
	t_binbuf *bbuf = binbuf_new();
	
	struct param *p = get_param_list();
	while(p) {
	    if ( p->id && ( id == NULL || p->id == id) && (p->selector != &s_bang)) {
			int ac = p->ac + 2;
			t_atom *av = getbytes(ac*sizeof(*av));	
			tof_copy_atoms(p->av,av+2,p->ac);
			SETSYMBOL(av, p->path);
			SETSYMBOL(av+1, p->selector);
			binbuf_add(bbuf, ac, av);
			binbuf_addsemi(bbuf);
			
			freebytes(av, ac*sizeof(*av));
		}
		p = p->next;
	}
	
	
    char buf[MAXPDSTRING];
    canvas_makefilename(canvas, filename->s_name,
        buf, MAXPDSTRING);
		
	
    w_error = (binbuf_write(bbuf, buf, "", 0));
            //pd_error("%s: write failed", filename->s_name);
			
	binbuf_free(bbuf);
	
	return w_error;

}


static int param_read(t_canvas* canvas, t_symbol* filename)
{
	
	int r_error;
	
	//t_symbol* filename = param_makefilename(basename, n);
	
	t_binbuf *bbuf = binbuf_new();
	
    r_error= (binbuf_read_via_canvas(bbuf, filename->s_name, canvas, 0));
            //pd_error(x, "%s: read failed", filename->s_name);
			
  
	
	int bb_ac = binbuf_getnatom(bbuf);
	int ac = 0;
    t_atom *bb_av = binbuf_getvec(bbuf);
    t_atom *av = bb_av;
	
	  while (bb_ac--) {
		if (bb_av->a_type == A_SEMI) {
			if ( IS_A_SYMBOL(av,0) && ac > 1) {
			   t_symbol* s = atom_getsymbol(av);
			   if ( s->s_thing) pd_forwardmess(s->s_thing, ac-1, av+1);
				  
			   
		    }
		  
		  ac = 0;
		  av = bb_av + 1;
		} else {
		  
		  ac = ac + 1;
		}
		bb_av++;
	  }
	
	
	binbuf_free(bbuf);
	
	return r_error;
}




