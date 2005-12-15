/*
 *   Pure Data Packet module: Guile Interpreter for pd/pdp
 *   Copyright (c) by Tom Schouten <pdp@zzz.kotnet.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#include <pthread.h>
#include <libguile.h>
#include <stdio.h>
#include "pdp.h"
#include "pdp_forth.h"

#define D if(0)



/* communication stuff */

static t_class *guile_class;

pthread_mutex_t guile_mux;

t_pdp_list *guile_inlist;
t_pdp_list *guile_outlist;
t_clock *guile_clock;
t_float guile_clock_deltime;


/* GUILE GLUE CODE */

static scm_t_bits pdp_tag;

typedef struct _pdp_scm_wrapper
{
    int packet;
    SCM update_func;
    //char dummy[1000000];
} t_pdp_scm_wrapper;


/* list (comm queue) utility stuff */

static void guile_list_send(t_pdp_list *list, t_pdp_symbol *tag, t_pdp_word_type type, t_pdp_word word)
{
	pthread_mutex_lock(&guile_mux);
	pdp_list_add_back(list, a_symbol, (t_pdp_word)tag);
	pdp_list_add_back(list, type, word);
	pthread_mutex_unlock(&guile_mux);
}

static int guile_list_receive(t_pdp_list *list, t_pdp_symbol **tag, t_pdp_word_type *type, t_pdp_word *word)
{
    int success = 0;
    if (pdp_list_size(list)){
	pthread_mutex_lock(&guile_mux);
	*tag = pdp_list_pop(list).w_symbol;
	*type = list->first->t;
	*word = pdp_list_pop(list);
	pthread_mutex_unlock(&guile_mux);
	success = 1;
    }
    return success;

}

/* pdp list <-> scm list conversion */

/* any packets contained in the pdp list are copied ro */

static SCM pdp_to_scm (t_pdp_list *l) 
{
  SCM pair = SCM_EOL; //temporary pair
  SCM thing; //thing to add to the list
  t_pdp_list *cl = pdp_list_copy_reverse(l); //make a reverse copy of the list
  t_pdp_atom *a;
  t_pdp_scm_wrapper *pg;

  for(a=cl->first;a;a=a->next){

    // convert the atom to a lisp scalar
    switch (a->t){
    case a_list:
      thing = pdp_to_scm(a->w.w_list); break;
    case a_int:
      thing = SCM_MAKINUM(a->w.w_int); break;
    case a_symbol:
      thing = scm_str2symbol(a->w.w_symbol->s_name); break;
    case a_float:
      thing = scm_make_real (a->w.w_float); break;
    case a_packet:
      //post("ATTENTION: pdp_to_scm copies packets");
	pg = (t_pdp_scm_wrapper *) scm_must_malloc (sizeof (t_pdp_scm_wrapper), "pdp");
	pg->packet = pdp_packet_copy_ro(a->w.w_packet);
	pg->update_func = SCM_BOOL_F;
	SCM_NEWSMOB (thing, pdp_tag, pg);
	break;
    default:
      thing = scm_str2symbol("undef"); break;
    }

    // add the atom to the list
    pair = scm_cons(thing, pair); // add thing
    
  }

  // 

  // return the list
  return pair;
}

/* any packets contained in the list are copied ro */
static t_pdp_list *scm_to_pdp (SCM pair) 
{
  t_pdp_list *l = pdp_list_new(0);
  SCM rest = pair;
  SCM head;
  while (1){
    if (!SCM_CONSP(rest)){
      head = rest;
      rest = SCM_EOL;
    }
    else {
      head = SCM_CAR(rest);
      rest = SCM_CDR(rest);
    }
    if (SCM_NULLP(head)) break;
   
    else if (SCM_CONSP(head)){
      pdp_list_add_back(l, a_list, (t_pdp_word)scm_to_pdp(head));
    }
    else if (SCM_SMOB_PREDICATE(pdp_tag, head)){
	t_pdp_scm_wrapper *pg = (t_pdp_scm_wrapper *) SCM_SMOB_DATA (head);
	//post("ATTENTION: scm_to_pdp copies packets");
	pdp_list_add_back(l, a_packet, 
			  (t_pdp_word)(pdp_packet_copy_ro(pg->packet)));
    }
    else if (SCM_INUMP(head)){
	int i = SCM_INUM(head);
	pdp_list_add_back(l, a_int, (t_pdp_word)i);
    }
    else if (SCM_REALP(head)){
	float f = (float)SCM_REAL_VALUE(head);
	pdp_list_add_back(l, a_float, (t_pdp_word)f);
    }
    else if (SCM_SYMBOLP(head)){
      pdp_list_add_back(l, a_symbol, 
			(t_pdp_word)pdp_gensym(SCM_SYMBOL_CHARS(head)));
    }
    else {
      pdp_list_add_back(l, a_undef, (t_pdp_word)0);
    }
  }
  return l;
}




/* smob callbacks */

static SCM mark_pdp (SCM pdp_smob)
{
    /* Mark the image's name and update function.  */
    t_pdp_scm_wrapper *pg = (t_pdp_scm_wrapper *) SCM_SMOB_DATA (pdp_smob);
    scm_gc_mark (pg->update_func);
    D post("mark_pdp called");
    return SCM_BOOL_F;
}

static size_t free_pdp (SCM pdp_smob)
{
    t_pdp_scm_wrapper *pg = (t_pdp_scm_wrapper *) SCM_SMOB_DATA (pdp_smob);
    pdp_packet_mark_unused(pg->packet);
    scm_must_free(pg);
    D post("free_pdp called");
    return sizeof(t_pdp_scm_wrapper);
}


static int
print_pdp (SCM pdp_smob, SCM port, scm_print_state *pstate)
{
    t_pdp_scm_wrapper *pg = (t_pdp_scm_wrapper *) SCM_SMOB_DATA (pdp_smob);
    char tmp[1000];
    t_pdp *h = pdp_packet_header(pg->packet);
    tmp[999] = 0;
    snprintf(tmp, 999, "#<pdp %d %s>", pg->packet, 
	    h && h->desc ? h->desc->s_name : "unknown");
    scm_puts(tmp, port);
    D post ("print_pdp called");
    return 1;
}


/* extension functions */


static SCM
forth_pdp (SCM stack, SCM program)
{
    t_pdp_word_error e;
    t_pdp_list *s = scm_to_pdp (stack);
    t_pdp_list *p = scm_to_pdp (program);
    e = pdp_forth_execute_def(s, p);
    stack = pdp_to_scm(s);

    if (e){
	post("ERROR: program evaluation aborted with error %d.", e);
    }
    pdp_tree_strip_packets(s);
    pdp_tree_strip_packets(p);
    
    return stack;
    
}

static SCM
define_forth_word_pdp (SCM word, SCM in, SCM out, SCM program)
{
    t_pdp_word_error e;
    t_pdp_list *p;
    t_pdp_symbol *s;
    SCM_ASSERT (SCM_SYMBOLP(word), word, SCM_ARG1, "define-forth-word");
    SCM_ASSERT (SCM_INUMP(in), word, SCM_ARG2, "define-forth-word");
    SCM_ASSERT (SCM_INUMP(out), word, SCM_ARG3, "define-forth-word");
    p = scm_to_pdp (program);
    s = pdp_gensym(SCM_SYMBOL_CHARS(word));
    pdp_forthdict_add_word(s, p, SCM_INUM(in), SCM_INUM(out), -1, 0); 
    
    return SCM_UNSPECIFIED;
    
}


static SCM
unmark_pdp (SCM pdp_smob)
{
    t_pdp_scm_wrapper *pg;
    SCM_ASSERT (SCM_SMOB_PREDICATE (pdp_tag, pdp_smob),
		pdp_smob, SCM_ARG1, "mark-unused");
    pg = (t_pdp_scm_wrapper *) SCM_SMOB_DATA (pdp_smob);
    pdp_packet_mark_unused(pg->packet);
    pg->packet = -1;
    return SCM_UNSPECIFIED;
}




static SCM
is_writable_pdp (SCM pdp_smob)
{
    t_pdp_scm_wrapper *pg;
    SCM_ASSERT (SCM_SMOB_PREDICATE (pdp_tag, pdp_smob),
		pdp_smob, SCM_ARG1, "writable?");
    pg = (t_pdp_scm_wrapper *) SCM_SMOB_DATA (pdp_smob);
    if (pdp_packet_writable(pg->packet)) return SCM_BOOL_T;
    else return SCM_BOOL_F;
    return SCM_UNSPECIFIED;
}

static SCM
make_pdp (SCM type)
{
    int p;
    t_pdp_scm_wrapper *pg;
    char *t;

    /* construct with type */
    if (SCM_SYMBOLP(type)){
      t = SCM_SYMBOL_CHARS(type);
      p =  pdp_factory_newpacket(pdp_gensym(t));
    }
    /* copy construct from packet id */
    else if (SCM_INUMP(type)){
      int i = SCM_INUM(type);
      p = pdp_packet_copy_ro(i);
    }
    else {
      SCM_ASSERT (0,  type,  SCM_ARG1, "make-pdp");
    }

    /* check if valid packet */
    SCM_ASSERT (p != -1,  type,  SCM_ARG1, "make-pdp");

    /* build smob */
    pg = (t_pdp_scm_wrapper *) scm_must_malloc (sizeof (t_pdp_scm_wrapper), "pdp");
    pg->packet = p;
    pg->update_func = SCM_BOOL_F;
    SCM_RETURN_NEWSMOB (pdp_tag, pg);
}

static SCM
_in (void)
{
    
    t_pdp_scm_wrapper *pg;
    t_pdp_word w;
    t_pdp_symbol *stag;
    t_pdp_word_type type = a_undef;
    int skip = 1;

    SCM tag;
    SCM thing;
    SCM pair;

    D post("receiving pdp packet");

    if (!guile_list_receive(guile_inlist, &stag, &type, &w)) goto exit;
    tag = scm_str2symbol(stag->s_name);

    switch(type){
    case a_packet:
	pg = (t_pdp_scm_wrapper *) scm_must_malloc (sizeof (t_pdp_scm_wrapper), "pdp");
	pg->packet = w.w_packet;
	pg->update_func = SCM_BOOL_F;
	SCM_NEWSMOB (thing, pdp_tag, pg);
	break;
    case a_symbol:
	thing = scm_str2symbol(w.w_symbol->s_name);
	break;
    case a_float:
	thing = scm_make_real (w.w_float);
	break;

    default:
	goto exit;
	
    }
    pair = scm_cons2(tag, thing, SCM_EOL);
    return pair;

	
 exit:
    return SCM_BOOL_F;
}

static SCM
_out (SCM tag, SCM thing)
{
    t_pdp_symbol *stag;
    SCM_ASSERT (SCM_SYMBOLP (tag),  tag,  SCM_ARG1, "out");
    stag = pdp_gensym(SCM_SYMBOL_CHARS(tag));

    if (SCM_SMOB_PREDICATE(pdp_tag, thing)){
	t_pdp_scm_wrapper *pg = (t_pdp_scm_wrapper *) SCM_SMOB_DATA (thing);
	guile_list_send(guile_outlist, stag, a_packet, 
			(t_pdp_word)(pdp_packet_copy_ro(pg->packet)));
    }
    else if (SCM_INUMP(thing)){
	int i = SCM_INUM(thing);
	guile_list_send(guile_outlist, stag, a_int, (t_pdp_word)i);
    }
    else if (SCM_REALP(thing)){
	float f = (float)SCM_REAL_VALUE(thing);
	guile_list_send(guile_outlist, stag, a_float, (t_pdp_word)f);
    }
    else if (SCM_SYMBOLP(thing)){
	char *s = SCM_SYMBOL_CHARS(thing);
	guile_list_send(guile_outlist, stag, a_symbol, (t_pdp_word)(pdp_gensym(s)));
    }
    else if (SCM_CONSP(thing)){
      t_pdp_list *l = scm_to_pdp(thing);
      guile_list_send(guile_outlist, stag, a_list, (t_pdp_word)l);
    }
    else{
	SCM_ASSERT (0,  thing,  SCM_ARG2, "out");
    }

    return SCM_BOOL_T;

}


static SCM
test_pdp (SCM thing)
{
    if (SCM_SMOB_PREDICATE(pdp_tag, thing)) return SCM_BOOL_T;
    else return SCM_BOOL_F;
}
static SCM
printdebugforthword_pdp (SCM thing)
{
    t_pdp_symbol *s;
    t_pdp_atom *a;
    SCM_ASSERT (SCM_SYMBOLP(thing), thing, SCM_ARG1, "pdp-debug-forth-word");
    s = pdp_gensym(SCM_SYMBOL_CHARS(thing));

    pdp_forth_word_print_debug(s);

    return SCM_UNSPECIFIED;
    
}

static SCM
printdebug_pdp (SCM thing)
{
  if (SCM_INUMP(thing)){
    int p = SCM_INUM(thing);
    pdp_packet_print_debug(p);
    return SCM_UNSPECIFIED;
  }

    SCM_ASSERT (SCM_SMOB_PREDICATE(pdp_tag, thing), thing, SCM_ARG1, "pdp-debug-packet");
    pdp_packet_print_debug(((t_pdp_scm_wrapper *) SCM_SMOB_DATA (thing))->packet);
    return SCM_UNSPECIFIED;
    
}
/* the inner main function. when this is called, guile
   is initialized. when this function returns, exit(0)
   is called. */
static void guile_inner_main(void *closure, int argv, char **argc)
{
    /* initialize data types and functions */
    pdp_tag = scm_make_smob_type ("pdp", sizeof(t_pdp_scm_wrapper)); 
    scm_set_smob_free (pdp_tag, free_pdp);
    scm_set_smob_print (pdp_tag, print_pdp);
    scm_set_smob_mark (pdp_tag, mark_pdp);

    /* pdp stuff */
    scm_c_define_gsubr ("make-pdp", 1, 0, 0, make_pdp);
    scm_c_define_gsubr ("writable?", 1, 0, 0, is_writable_pdp);
    scm_c_define_gsubr ("pdp?", 1, 0, 0, test_pdp);

    /* communication */
    scm_c_define_gsubr ("in", 0, 0, 0, _in);
    scm_c_define_gsubr ("out", 2, 0, 0, _out);

    /* pdp forth link */
    scm_c_define_gsubr ("forth", 2, 0, 0, forth_pdp);
    scm_c_define_gsubr ("define-forth-word", 4, 0, 0, define_forth_word_pdp);

    /* debug methods */
    scm_c_define_gsubr ("mark-unused", 1, 0, 0, unmark_pdp);
    scm_c_define_gsubr ("pdp-debug-packet", 1, 0, 0, printdebug_pdp);
    scm_c_define_gsubr ("pdp-debug-forth-word", 1, 0, 0, printdebugforthword_pdp);

    /* start the shell */
    scm_c_eval_string("(set! scm-repl-prompt \"pdp> \")");

    //scm_c_eval_string("(load \"/home/tom/.guile\")");
    //while(1) {
    //post("read evaluate print loop");
    //scm_c_eval_string("(display scm-repl-prompt)(display (eval (read) (current-module)))(newline)");
    //}
    
    scm_shell(0,0);
}


/* the outer main function for booting the guile system
   this is a separate thread */
static void *guile_main(void *x)
{
    scm_boot_guile(0,0,guile_inner_main,0);
    return 0;
}


/* start the thread that boots the guile system */
static void guile_boot_in_thread(void)
{
    pthread_t thread;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setschedpolicy(&attr, SCHED_OTHER); 
    pthread_create(&thread, &attr, guile_main, 0);

}


/* some restrictions on the pdp -> pd list conversion:

   -  due to the nature of the pdp protocol in pd (3 phase)
      a packet can not appear in a pd list. it will be sent out
      as a pdp packet to the receiver corresponding to the tag.
      the legal way to send a packet from guile is (out 'tag packet)

   -  pd lists cannot be recursive, so pdp trees will need to be 
      flattened first.
*/
     
/* this sends a (flat pdp list) to a pd receiver 
   the list cannot contain pdp packets */
static void pd_pdplist(void *thing, t_symbol *tag, t_pdp_list *l, int elements){
  t_atom pd_atom[elements];
  int i;
  t_pdp_atom *a = l->first;
  for (i=0; i<elements; i++,a=a->next){
    switch(a->t){
    case a_float:
      SETFLOAT(pd_atom+i, a->w.w_float); break;
    case a_int:
      SETFLOAT(pd_atom+i, (float)a->w.w_int); break;
    case a_pointer:
      SETPOINTER(pd_atom+i, a->w.w_pointer); break;
    case a_symbol:
      SETSYMBOL(pd_atom+i, gensym(a->w.w_symbol->s_name)); break;
    case a_packet:
      post("ERROR: can't put packet %d inside a pd list (not a pd atom)", 
	   a->w.w_packet);
      SETSYMBOL(pd_atom+i, gensym("deadpacket")); break;
    case a_list:
      post("ERROR: can't put a list inside a pd list (not a pd atom):");
      pdp_list_print(a->w.w_list);
      SETSYMBOL(pd_atom+i, gensym("deadlist")); break;
    default:
      SETSYMBOL(pd_atom+i, gensym("undef")); break;
      
    }
  }
  typedmess(thing, tag, elements, pd_atom);


  pdp_tree_strip_packets(l);
  pdp_tree_free(l);
}



static void guile_callback(void *dummy)
{
    int skip = 1;
    t_pdp_symbol *s = 0;
    t_pdp_word_type type = a_undef;
    t_pdp_word w;
    t_atom atom[3];
    void *thing;

    t_symbol *tag_sym;

    if (!guile_list_receive(guile_outlist, &s, &type, &w)) goto exit;

    tag_sym = gensym(s->s_name);
    thing = tag_sym->s_thing;

    //post("got something from guile");

    if (!thing) goto exit;



    /* send to outlet */
    switch(type){
    case a_packet:
	SETFLOAT(atom+1, (float)w.w_packet);
	SETSYMBOL(atom+0, pdp_sym_rro());
	typedmess(thing, pdp_sym_pdp(), 2, atom);
	SETSYMBOL(atom+0, pdp_sym_rrw());
	typedmess(thing, pdp_sym_pdp(), 2, atom);
	SETSYMBOL(atom+0, pdp_sym_prc());
	typedmess(thing, pdp_sym_pdp(), 2, atom);
	pdp_packet_mark_unused(w.w_packet);
	break;
   
    case a_float:
	pd_float(thing, w.w_float);
	break;
	
    case a_int:
	pd_float(thing, (float)w.w_int);
	break;

    case a_symbol:
	pd_symbol(thing, gensym(w.w_symbol->s_name));
	break;

    case a_list:{
      int elements = w.w_list->elements;
      t_symbol *tag;
      
      if (elements){
	/* if first element is a symbol, we use it to tag the list */
	if (w.w_list->first->t == a_symbol){
	  tag = gensym(w.w_list->first->w.w_symbol->s_name);
	  elements--;
	  pdp_list_pop(w.w_list);
	  pd_pdplist(thing, tag, w.w_list, elements);
	}
	/* else tag it as "list" */
	else {
	  pd_pdplist(thing, gensym("list"), w.w_list, elements);

	}
      }
      else{ //an empty list is a bang
	pd_bang(thing);
      }
      break;
    }

    default:
	break;
    }

 exit:
    clock_delay(guile_clock, guile_clock_deltime);
}



/* PD INTERFACE OBJECT */

t_class *scheme_send_class;

typedef struct _scheme_send
{
    t_object x_obj;
    t_outlet *x_outlet;
    t_pdp_symbol *x_tag;
} t_scheme_send;


static void *scheme_send_new(t_symbol *s)
{
    t_scheme_send *x = 0;
    x = (t_scheme_send *)pd_new(scheme_send_class);

    x->x_tag = (s == gensym("")) ? 
	x->x_tag = pdp_gensym("dummy") : pdp_gensym(s->s_name);

    return (void*)x;
}

static void scheme_send_anything(t_scheme_send *x, t_symbol *s, int argc, t_atom *argv)
{
    void *thing = 0;

    if (s == &s_float && argc == 1 && argv[0].a_type == A_FLOAT){
	guile_list_send(guile_inlist, x->x_tag, a_float, 
			(t_pdp_word)argv[1].a_w.w_float);
    }

    else if (s == &s_symbol && argc == 1 && argv[0].a_type == A_SYMBOL){
	guile_list_send(guile_inlist, x->x_tag, a_symbol, 
			(t_pdp_word)pdp_gensym(argv[0].a_w.w_symbol->s_name));
    }

    else if (argc == 0){
	guile_list_send(guile_inlist, x->x_tag, a_symbol, 
			(t_pdp_word)pdp_gensym(s->s_name));
    }

    else if (s == gensym("pdp") 
	&& argc == 2 
	&& argv[0].a_type == A_SYMBOL 
	&& argv[0].a_w.w_symbol == gensym("register_ro")
	&& argv[1].a_type == A_FLOAT){
	  
	guile_list_send(guile_inlist, x->x_tag, a_packet, 
			(t_pdp_word)pdp_packet_copy_ro((int)argv[1].a_w.w_float));
    }

}




static void scheme_send_free(t_scheme_send *x)
{
}

void pdp_guile_setup(void)
{

    post("PDP: scheme extension");

    /* setup guile interpreter */
    guile_boot_in_thread();

    /* setup communication stuff */
    guile_inlist = pdp_list_new(0);
    guile_outlist = pdp_list_new(0);

    pthread_mutex_init(&guile_mux, NULL);
    guile_clock_deltime = 1.0f;
    guile_clock = clock_new(0, (t_method)guile_callback);
    clock_delay(guile_clock, guile_clock_deltime);



    /* create interface class */
    scheme_send_class = class_new(gensym("ss"), (t_newmethod)scheme_send_new,
   	(t_method)scheme_send_free, sizeof(t_scheme_send), 0, A_DEFSYMBOL, A_NULL);

    class_addanything(scheme_send_class, scheme_send_anything);
}

