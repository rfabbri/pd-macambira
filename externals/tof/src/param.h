//#define PARAMDEBUG
#include <stdio.h>

char param_buf_temp_a[MAXPDSTRING];
char param_buf_temp_b[MAXPDSTRING];
char* separator = "/";

//char PARAMECHO = 0;


struct param {
   
   t_symbol*		root;
   t_symbol* 		path; //Path(name) of the param
   t_symbol*    	send;
   t_symbol*		receive; 
   t_symbol* 		selector; //Type of data stored
   int			 	alloc; //Memory allocated
   int 				ac; //Number of values stored
   t_atom* 			av; //Values stored
   struct param* 	next; //Next param
   struct param* 	previous; //Previous param
   int 				users; //Number of param objects using this param
   t_symbol* 		id; //The base id
   int 				ac_g; //Gui argument count
   t_atom*			av_g; //Gui argument values
};

struct paramroot {
	t_symbol*			root;
	struct param* 		params; //param list
	struct paramroot* 	next; //Next paramroot
    struct paramroot* 	previous; //Previous paramroot
};

struct paramroot* paramroots;

//struct param* paramlist;


static void set_param_anything( struct param* p, t_symbol* s, int ac, t_atom *av) {
	
	if ( s == &s_bang || ac == 0 ) {
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





/*
static void print_all_params(void) {
	
	struct param* p = paramlist;
	post("--paramlist--");
		while(p) {
			if ( p->path) post("Path: %s",p->path->s_name);
			if (p->id) post("Id: %s",p->id->s_name);
			p = p->next;
	}
	
}
*/


static struct paramroot* param_get_root(t_symbol* root) {
	
	if (paramroots == NULL) {
		#ifdef PARAMDEBUG
		post("Could not get...not even one root created");
		#endif
		return NULL;
	 }
	
	
	// Pointer to the start of paramroots
	struct paramroot* branch = paramroots;
		
		while( branch ) {
			if ( branch->root == root) {
				#ifdef PARAMDEBUG
				  post("Found root:%s",root->s_name);
				#endif
				
				return branch;
			}
			branch = branch->next; 
		}
	#ifdef PARAMDEBUG
		post("Could not find root");
	#endif
	return branch;
	
}


static struct paramroot* param_root_attach(t_symbol* root){
		
		// Pointer to the start of paramroots
		struct paramroot* branch = paramroots;
		
		while( branch ) {
			if ( branch->root == root) {
				#ifdef PARAMDEBUG
				  post("Found root:%s",root->s_name);
				#endif
				
				return branch;
			}
			if ( branch->next == NULL ) break;
			branch = branch->next; 
		}
		
		// we did not find a paramroot linked to this root canvas
		// so we create it
		#ifdef PARAMDEBUG
			 post("Creating root:%s",root->s_name);
		#endif
		
		// Create and add paramroot to the end
		struct paramroot* newbranch = getbytes(sizeof(*newbranch));
		newbranch->root = root;
		newbranch->next = NULL;
		newbranch->params = NULL;
		
		if (branch) {
			#ifdef PARAMDEBUG
			  post("Appending it to previous roots");
			#endif
			newbranch->previous = branch;
			branch->next = newbranch;
		} else {
			#ifdef PARAMDEBUG
				post("Creating first root");
			#endif
			newbranch->previous = NULL;
			paramroots = newbranch;
		}
		
		
		return newbranch;
	
}



static struct param* get_param_list(t_symbol* root) {
	
	
	struct paramroot* branch = param_get_root(root);
	if (branch) {
		
	#ifdef PARAMDEBUG
		post("Getting params from %s",branch->root->s_name);
		if (!branch->params) post("Root contains no params");
	#endif
		return branch->params;
	} 
	
	return NULL;
    
	
}

static t_symbol* param_get_name ( int ac, t_atom* av  ) {
	
	if (ac  && IS_A_SYMBOL(av, 0)) {
		char *firstChar =  (atom_getsymbol(av))->s_name;
		if (*firstChar == *separator) {
			return atom_getsymbol(av);
		 }
	} 
	post("param requires a name that starts with a \"/\"");
	return NULL;
}

// Name can be NULL

static t_symbol* param_get_path( t_canvas* i_canvas,  t_symbol* name) {
	
	char* sbuf_name = param_buf_temp_a;
	char* sbuf_temp = param_buf_temp_b;
	sbuf_name[0] = '\0';
	sbuf_temp[0] = '\0';
	//char* separator = "/";
	
	
	t_symbol* id_s = gensym("/id"); // symbol that points to "/id" symbol
	   
    // arguments of the current canvas being analyzed
    int i_ac;
    t_atom * i_av;
	
	// temp pointer to the current id being added to the path
	t_symbol* id_temp;
	
	/* FIND ID AND BASEPATH  */
   while( i_canvas->gl_owner) {
	   // Ignore all supatches
	   if ( tof_canvas_is_not_subpatch(i_canvas) ) {
		tof_get_canvas_arguments(i_canvas,&i_ac, &i_av);
		id_temp=tof_get_canvas_name(i_canvas);
		//id_temp= canvas_realizedollar(i_canvas, gensym("$0"));
		int ac_a = 0;
		t_atom* av_a = NULL;
		int iter = 0;
		//found_id_flag = 0;
		
		while( tof_next_tagged_argument(*separator,i_ac,i_av,&ac_a,&av_a,&iter) ) {
			
			if ( IS_A_SYMBOL(av_a,0)
			   && (id_s == av_a->a_w.w_symbol) 
			   && (ac_a > 1) ) {  
	           	id_temp = atom_getsymbol(av_a+1);
	           	//id_canvas = i_canvas;
				//found_id_flag = 1;
	           	break;
			}
		}	        
        // if ever an /id is missing, this param is not saveable
        //if (found_id_flag == 0)  saveable = 0;
        
	   // Prepend newly found ID
		   strcpy(sbuf_temp,sbuf_name);
		   strcpy(sbuf_name, separator);
		   strcat(sbuf_name, id_temp->s_name);
		   strcat(sbuf_name,sbuf_temp);  
		}
        i_canvas = i_canvas->gl_owner;
    } 
  //strcat(sbuf_name,separator);
  if ( name != NULL) {
	  strcat(sbuf_name,name->s_name);
  } else {
	  strcat(sbuf_name, separator);
  }
  
  return gensym(sbuf_name);
	
}



// root, path, ac, av, ac_g, av_g
// From there, deduct id, path_, etc...

//static struct param* register_param( t_canvas* canvas, int o_ac, t_atom* o_av) {

static struct param* param_register(t_symbol* root, t_symbol* path, int ac, t_atom* av,int ac_g, t_atom* av_g) {
	
			
     //char *separator = "/";
		
		
		
		/* GET POINTER TO PARAMLIST FOR THAT ROOT  */
		struct paramroot* branch = param_root_attach(root);
        struct param* last = branch->params;
		
		// Search for param with same path
		while( last ) {
			if ( last->path == path) {
				#ifdef PARAMDEBUG
				  post("Found param with same name");
				#endif
				last->users = last->users + 1;
				return last;
			}
			if ( last->next == NULL ) break;
			last = last->next; 
		}
		
		// Create and add param to the end
		
		
		
		
		struct param* p = getbytes(sizeof(*p));
		p->root = root;
		p->alloc = 0;
		p->path = path;
		
		// Create receive and send symbols: $0/path
		strcpy(param_buf_temp_a,p->root->s_name);  
		//strcat(param_buf_temp_a,separator);
		strcat(param_buf_temp_a,p->path->s_name);
		p->receive = gensym(param_buf_temp_a);
		strcat(param_buf_temp_a,"_");  
		p->send = gensym(param_buf_temp_a);
		
		p->next = NULL;
		p->users = 1;
		//p->id = id;
		set_param( p, ac, av);
		p->ac_g = ac_g;
		p->av_g = getbytes(ac_g*sizeof(*(p->av_g)));
		tof_copy_atoms(av_g,p->av_g,ac_g);
		if (last) {
			#ifdef PARAMDEBUG
			  post("Appending param");
			#endif
			p->previous = last;
			last->next = p;
		} else {
			#ifdef PARAMDEBUG
				post("Creating first param");
			#endif
			p->previous = NULL;
			branch->params = p;
		}
		
		
		return p;
   
}

static void param_unregister(struct param* p) {
	
	//post("unregistering %s", p->path->s_name);
	struct paramroot* branch = param_get_root(p->root);
	struct param* paramlist = branch->params;
	
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
		
		// Update the params for that root
		if (paramlist == NULL) {
			if (branch->previous) {
				branch->previous->next = branch->next;
					if (branch->next) branch->next->previous = branch->previous;
			} else {
				paramroots = branch->next;
				if ( branch->next != NULL) branch->next->previous = NULL;
			}
			#ifdef PARAMDEBUG
			  post("Removing root:%s",branch->root->s_name);
			#endif
			freebytes(branch, sizeof *branch);
		} else {
			branch->params = paramlist;
		}
		
	} else {
		post("Euh... no params found!");
	}
	
	
}






/*
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
   
   
   while( i_canvas->gl_owner) {
	   // Ignore all supatches
	   if ( tof_canvas_is_not_subpatch(i_canvas) ) {
		tof_get_canvas_arguments(i_canvas,&i_ac, &i_av);
		id_s= canvas_realizedollar(i_canvas, gensym("$0"));
	
		
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
        
		
	   // Prepend newly found ID
		   strcpy(sbuf_temp,sbuf_name);
		   strcpy(sbuf_name, "/");
		   strcat(sbuf_name, id_s->s_name);
		   strcat(sbuf_name,sbuf_temp);  
        
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
*/

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
		if(!(p->selector == &s_bang) ) {
			
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
	} else if (p->selector == &s_bang) {
		outlet_anything(outlet,s,0,NULL);
	}
	
}


// Write will only save the params that share the same root
static int param_write(t_canvas* canvas, t_symbol* filename) {
	
	
	int w_error;
	
	//t_symbol* filename = param_makefilename(basename, n);
		
	t_binbuf *bbuf = binbuf_new();
	
	struct param *p = get_param_list(tof_get_dollarzero(canvas));
	while(p) {
	    if ((p->selector != &s_bang)) {
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
			
    t_symbol* root = tof_get_dollarzero(canvas);
	
	int bb_ac = binbuf_getnatom(bbuf);
	int ac = 0;
    t_atom *bb_av = binbuf_getvec(bbuf);
    t_atom *av = bb_av;
	
	  while (bb_ac--) {
		if (bb_av->a_type == A_SEMI) {
			if ( IS_A_SYMBOL(av,0) && ac > 1) {
				t_symbol* path = atom_getsymbol(av);
				strcpy(param_buf_temp_a,root->s_name);
				strcat(param_buf_temp_a,path->s_name);
			   t_symbol* s = gensym(param_buf_temp_a);
			   #ifdef PARAMDEBUG
				post("Restoring:%s",s->s_name);
			   #endif
			   
			   // STUPID SYMBOL WITH SPACES MANAGEMENT
			   if ( s->s_thing && ac > 3 && IS_A_SYMBOL(av,1) &&  atom_getsymbol(av+1) == &s_symbol) {
				   // This whole block is simply to convert symbols saved with spaces to complete symbols
				   
				   t_binbuf *bbuf_stupid = binbuf_new();
				   binbuf_add(bbuf_stupid, ac-2, av+2);
				   
				   char *char_buf;
				   int char_length;
				   binbuf_gettext(bbuf_stupid, &char_buf, &char_length);
				   char_buf = resizebytes(char_buf, char_length, char_length+1);
				   char_buf[char_length] = 0;
				   t_symbol* stupid_symbol = gensym(char_buf);
				   //post("STUPID: %s",stupid_symbol->s_name);
				   freebytes(char_buf, char_length+1);
				   binbuf_free(bbuf_stupid);
				   t_atom* stupid_atom = getbytes(sizeof(*stupid_atom));
				   SETSYMBOL(stupid_atom, stupid_symbol);
				   pd_typedmess(s->s_thing, &s_symbol, 1, stupid_atom);
				   freebytes(stupid_atom, sizeof(*stupid_atom));
				   
			   } else {
					if ( s->s_thing) pd_forwardmess(s->s_thing, ac-1, av+1);
				}
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




