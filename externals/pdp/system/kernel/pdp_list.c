
/*
 *   Pure Data Packet header file. List class
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

/* who can live without a list, hmm? */

/* rethink the list of list thing:
   should free and copy really copy the lists, or should
   references to lists be allowed too?

   maybe references to lists as generic pointers? */

#include "pdp_list.h"
#include "pdp_types.h"
#include "pdp.h"
#include <pthread.h>
#include <stdlib.h>

#define D if (0)

#define PDP_LIST_BLOCKSIZE 4096

#define PDP_ATOM_ALLOC _pdp_atom_reuse
#define PDP_ATOM_DEALLOC _pdp_atom_save
//#define PDP_ATOM_ALLOC _pdp_atom_alloc
//#define PDP_ATOM_DEALLOC _pdp_atom_dealloc

static t_pdp_atom *freelist;
static t_pdp_list *blocklist;
static pthread_mutex_t mut;

/* memory allocation (for ease of wrapping) */
static inline t_pdp_list* _pdp_list_alloc(void){return (t_pdp_list *)pdp_alloc(sizeof(t_pdp_list));}
static inline void _pdp_list_dealloc(t_pdp_list *list){pdp_dealloc(list);}
static inline t_pdp_atom* _pdp_atom_alloc(void){return (t_pdp_atom *)pdp_alloc(sizeof(t_pdp_atom));}
static inline void _pdp_atom_dealloc(t_pdp_atom *a){pdp_dealloc(a);}



/* some private helper methods */

/* list pool setup */
void pdp_list_setup(void)
{
    freelist = 0;
    blocklist = _pdp_list_alloc();
    blocklist->elements = 0;
    blocklist->first = 0;
    pthread_mutex_init(&mut, NULL);

#if 0
    {
	char *c = "(test 1 2 (23 4)) (een (zes (ze)ven ()))) (())";
	while (*c){
	    t_pdp_list *l = pdp_list_from_char(c, &c);
	    if (l) pdp_list_print(l); 
	    else{
		post("parse error: remaining input: %s", c); break;
	    }
	}
    }
#endif

}

/* allocate a block */
static t_pdp_atom* _pdp_atom_block_alloc(int size)
{
    int i = size;
    t_pdp_atom *atom_block, *atom;
    atom_block = (t_pdp_atom *)pdp_alloc(sizeof(t_pdp_atom) * size);
    atom = atom_block;
    while(i--){
        atom->next = atom + 1;
        atom->t = a_undef;
	atom->w.w_int = 0;
        atom++;
    }
    atom_block[size-1].next = 0;
    return atom_block;
}

static void _pdp_atomlist_fprint(FILE* f, t_pdp_atom *a);

static void _pdp_atom_refill_freelist(void)
{
    t_pdp_atom *atom;

    if (freelist != 0) post("ERROR:_pdp_atom_do_reuse: freelist != 0");

    /* get a new block */
    freelist = _pdp_atom_block_alloc(PDP_LIST_BLOCKSIZE);
    //post("new block");
    //_pdp_atomlist_fprint(stderr, freelist);

    /* take the first element from the block to serve as a blocklist element */
    atom = freelist;
    atom->t = a_pointer;
    atom->w.w_pointer = freelist;
    freelist = freelist->next;
    atom->next = blocklist->first;
    blocklist->first = atom;
    blocklist->elements++;
    
}


/* reuse an old element from the freelist, or allocate a new block */
static inline t_pdp_atom* _pdp_atom_reuse(void)
{
    t_pdp_atom *atom;

    pthread_mutex_lock(&mut);

    while (!(atom = freelist)){
	_pdp_atom_refill_freelist();
    }
    /* delete the element from the freelist */
    freelist = freelist->next;
    atom->next = 0;

    pthread_mutex_unlock(&mut);

    return atom;
}

static inline void _pdp_atom_save(t_pdp_atom *atom)
{
    pthread_mutex_lock(&mut);

    atom->next = freelist;
    freelist = atom;

    pthread_mutex_unlock(&mut);
}






/* create a list */
t_pdp_list* pdp_list_new(int elements)
{
    t_pdp_atom *a = 0;
    t_pdp_list *l = _pdp_list_alloc();
    l->elements = 0;
    if (elements){
	a = PDP_ATOM_ALLOC();
	l->elements++;
	a->t = a_undef;
	a->w.w_int = 0;
	a->next = 0;
	elements--;
    }
    l->first = a;
    l->last = a;

    while (elements--){
	a = PDP_ATOM_ALLOC();
	l->elements++;
	a->t = a_undef;
	a->w.w_int = 0;
	a->next = l->first;
	l->first = a;
    }
    return l;
}

/* clear a list */
void pdp_list_clear(t_pdp_list *l)
{
    t_pdp_atom *a = l->first; 
    t_pdp_atom *next_a;

    while(a){
	next_a = a->next;
	PDP_ATOM_DEALLOC(a);
	a = next_a;
    }
    l->first = 0;
    l->last = 0;
    l->elements = 0;
}

/* destroy a list */
void pdp_list_free(t_pdp_list *l)
{
    if (l){
	pdp_list_clear(l);
	_pdp_list_dealloc(l);
    }
}


/* destroy a (sub)tree */
void pdp_tree_free(t_pdp_list *l)
{
  if (l) {
    pdp_tree_clear(l);
    _pdp_list_dealloc(l);
  }
}

/* clear a tree */
void pdp_tree_clear(t_pdp_list *l)
{
    t_pdp_atom *a = l->first; 
    t_pdp_atom *next_a;

    while(a){
      if (a->t == a_list) pdp_tree_free(a->w.w_list);
      next_a = a->next;
      PDP_ATOM_DEALLOC(a);
      a = next_a;
    }
    l->first = 0;
    l->last = 0;
    l->elements = 0;
}


static inline int _is_whitespace(char c){return (c == ' ' || c == '\n' || c == '\t');}
static inline void _skip_whitespace(char **c){while (_is_whitespace(**c)) (*c)++;}
static inline int _is_left_separator(char c) {return (c == '(');}
static inline int _is_right_separator(char c) {return (c == ')');}
static inline int _is_separator(char c) {return (_is_left_separator(c) || _is_right_separator(c) || _is_whitespace(c));}


static inline void _parse_atom(t_pdp_word_type *t, t_pdp_word *w, char *c, int n)
{
    char tmp[n+1];
    //post("_parse_atom(%d, %x, %s, %d)", *t, w, c, n); 
    strncpy(tmp, c, n);
    tmp[n] = 0;


    /* check if it's a number */
    if (tmp[0] >= '0' && tmp[0] <= '9'){
	int is_float = 0;
	char *t2;
	for(t2 = tmp; *t2; t2++){
	    if (*t2 == '.') {
		is_float = 1;
		break;
	    }
	}
	/* it's a float */
	if (is_float){
	    float f = strtod(tmp, 0);
	    D post("adding float %f", f);
	    *t = a_float;
	    *w = (t_pdp_word)f;
	}

	/* it's an int */
	else {
	    int i = strtol(tmp, 0, 0);
	    D post("adding int %d", i);
	    *t = a_int;
	    *w = (t_pdp_word)i;
	}
	
    }
    /* it's a symbol */
    else {
	D post("adding symbol %s", tmp);
	*t = a_symbol;
	*w = (t_pdp_word)pdp_gensym(tmp);
    }

    D post("done parsing atom");

}


/* create a list from a character string */
t_pdp_list *pdp_list_from_cstring(char *chardef, char **nextchar)
{
    t_pdp_list *l = pdp_list_new(0);
    char *c = chardef;
    char *lastname = c;
    int n = 0;

    D post ("creating list from char: %s", chardef);

    /* find opening parenthesis and skip it*/
    _skip_whitespace(&c);
    if (!_is_left_separator(*c)) goto error; else c++;

    /* start counting at the first non-whitespace
       character after opening parenthesis */
    _skip_whitespace(&c);
    lastname = c;
    n = 0;

    while(*c){
	if (!_is_separator(*c)){
	    /* item not terminated: keep counting */
	    c++;
	    n++;
	}
	else {
	    /* separator encountered whitespace or parenthesis */

	    if (n){
		/* if there was something between this and the previous
		   separator, we've found and atom and parse it */
		pdp_list_add_back(l, a_undef, (t_pdp_word)0);
		_parse_atom(&l->last->t, &l->last->w, lastname, n);

		/* skip the whitespace after the parsed word, if any */
		_skip_whitespace(&c);
	    }

	    /* if we're at a right separator, we're done */
	    if (_is_right_separator(*c)) {c++; goto done;}

	    /* if it's a left separater, we have a sublist */
	    if (_is_left_separator(*c)){
		t_pdp_list *sublist = pdp_list_from_cstring(c, &c);
		if (!sublist) goto error; //sublist had parse error
		pdp_list_add_back(l, a_list, (t_pdp_word)sublist);
	    }

	    /* prepare for next atom */
	    lastname = c;
	    n = 0;
	}

    }

 error:
    /* end of string encountered: parse error */
    D post("parse error: %s", c);
    if (nextchar) *nextchar = c;
    pdp_tree_free(l); //this will free all sublists too
    return 0;

	
 done:
    _skip_whitespace(&c);
    if (nextchar) *nextchar = c;
    return l;
    
    

}


/* traversal */
void pdp_list_apply(t_pdp_list *l, t_pdp_atom_method m)
{
  t_pdp_atom *a;
  if (!l) return;
  for (a=l->first; a; a=a->next) m(a);
}

void pdp_tree_apply(t_pdp_list *l, t_pdp_atom_method m) 
{
  t_pdp_atom *a;
  if (!l) return;
  for (a=l->first; a; a=a->next){
    if (a->t == a_list) pdp_tree_apply(a->w.w_list, m);
    else m(a);
  }
}

void pdp_list_apply_word_method(t_pdp_list *l, 
				t_pdp_word_type type, t_pdp_word_method wm)
{
  t_pdp_atom *a;
  if (!l) return;
  for (a=l->first; a; a=a->next){
    if (a->t == type) wm(a->w);
  }
}
void pdp_list_apply_pword_method(t_pdp_list *l, 
				t_pdp_word_type type, t_pdp_pword_method pwm)
{
  t_pdp_atom *a;
  if (!l) return;
  for (a=l->first; a; a=a->next){
    if (a->t == type) pwm(&a->w);
  }
}

void pdp_tree_apply_word_method(t_pdp_list *l, 
				t_pdp_word_type type, t_pdp_word_method wm) 
{
  t_pdp_atom *a;
  if (!l) return;
  for (a=l->first; a; a=a->next){
    if (a->t == a_list) pdp_tree_apply_word_method(a->w.w_list, type, wm);
    else if (a->t == type) wm(a->w);
  }
}
void pdp_tree_apply_pword_method(t_pdp_list *l, 
				t_pdp_word_type type, t_pdp_pword_method pwm) 
{
  t_pdp_atom *a;
  if (!l) return;
  for (a=l->first; a; a=a->next){
    if (a->t == a_list) pdp_tree_apply_pword_method(a->w.w_list, type ,pwm);
    else if (a->t == type) pwm(&a->w);
  }
}

static void _atom_packet_mark_unused(t_pdp_atom *a)
{
  if (a->t == a_packet){
    pdp_packet_mark_unused(a->w.w_packet);
    a->t = a_undef;
    a->w.w_int = 0;
  }
}

void pdp_tree_strip_packets  (t_pdp_list *l)
{
   pdp_tree_apply(l, _atom_packet_mark_unused);
}


/* debug */
static void _pdp_atomlist_fprint(FILE* f, t_pdp_atom *a)
{
    fprintf(f, "(");
    while (a){
	switch(a->t){
	case a_symbol:   fprintf(f, "%s",a->w.w_symbol->s_name); break;
	case a_float:    fprintf(f, "%f",a->w.w_float); break;
	case a_int:      fprintf(f, "%d",a->w.w_int); break;
	case a_packet:   fprintf(f, "#<pdp %d %s>",a->w.w_packet,
				 pdp_packet_get_description(a->w.w_packet)->s_name); break;
	case a_pointer:  fprintf(f, "#<0x%08x>",(unsigned int)a->w.w_pointer); break;
	case a_list:     _pdp_atomlist_fprint(f, a->w.w_list->first); break;
	case a_undef:    fprintf(f, "undef"); break;

	default:         fprintf(f, "unknown"); break;
	}
       	a = a->next;
	if (a) fprintf(f, " ");
    }
    fprintf(f, ")");
}

void _pdp_list_fprint(FILE* f, t_pdp_list *l)
{
    _pdp_atomlist_fprint(f, l->first);
    fprintf(f, "\n");
}

void pdp_list_print(t_pdp_list *l)
{
    _pdp_list_fprint(stderr, l);
}


/* public list operations */


/* add a word to the start of the list */
void pdp_list_add(t_pdp_list *l, t_pdp_word_type t, t_pdp_word w)
{
    t_pdp_atom *new_a = PDP_ATOM_ALLOC();
    l->elements++;
    new_a->next = l->first;
    new_a->w = w;
    new_a->t = t;
    l->first = new_a;
    if (!l->last) l->last = new_a;
}


/* add a word to the end of the list */
void pdp_list_add_back(t_pdp_list *l, t_pdp_word_type t, t_pdp_word w)
{
    t_pdp_atom *new_a = PDP_ATOM_ALLOC();
    l->elements++;
    new_a->next = 0;
    new_a->w = w;
    new_a->t = t;
    if (l->last){
	l->last->next = new_a;
    }
    else{
	l->first = new_a;
    }
    l->last = new_a;
}

/* get list size */
int pdp_list_size(t_pdp_list *l)
{
    return l->elements;
}




/* pop: return first item and remove */
t_pdp_word pdp_list_pop(t_pdp_list *l)
{
    t_pdp_atom *a = l->first;
    t_pdp_word w;

    w = a->w;
    l->first = a->next;
    PDP_ATOM_DEALLOC(a);
    l->elements--;
    if (!l->first) l->last = 0;

    return w;
}


/* return element at index */
t_pdp_word pdp_list_index(t_pdp_list *l, int index)
{
    t_pdp_atom *a;
    for (a = l->first; index--; a = a->next);
    return a->w;
}





/* remove an element from a list */
void pdp_list_remove(t_pdp_list *l, t_pdp_word_type t, t_pdp_word w)
{
    t_pdp_atom head;
    t_pdp_atom *a;
    t_pdp_atom *kill_a;
    head.next = l->first;

    for(a = &head; a->next; a = a->next){
	if (a->next->w.w_int == w.w_int && a->next->t == t){
	    kill_a = a->next;        // element to be killed
	    a->next = a->next->next; // remove link
	    PDP_ATOM_DEALLOC(kill_a);
	    l->elements--;
	    l->first = head.next;    // restore the start pointer
	    if (l->last == kill_a) { // restore the end pointer
		l->last = (a != &head) ? a : 0;
	    }

	    break;
	}
    }
    
}





/* copy a list */
t_pdp_list* pdp_tree_copy_reverse(t_pdp_list *list)
{
    t_pdp_list *newlist = pdp_list_new(0);
    t_pdp_atom *a;
    for (a = list->first; a; a = a->next)
	if (a->t == a_list){
	    pdp_list_add(newlist, a->t, 
			 (t_pdp_word)pdp_tree_copy_reverse(a->w.w_list));
	}
	else{
	    pdp_list_add(newlist, a->t, a->w);
	}
    return newlist;
}
t_pdp_list* pdp_list_copy_reverse(t_pdp_list *list)
{
    t_pdp_list *newlist = pdp_list_new(0);
    t_pdp_atom *a;
    for (a = list->first; a; a = a->next)
      pdp_list_add(newlist, a->t, a->w);
    return newlist;
}

t_pdp_list* pdp_tree_copy(t_pdp_list *list)
{
    t_pdp_list *newlist = pdp_list_new(list->elements);
    t_pdp_atom *a_src = list->first;
    t_pdp_atom *a_dst = newlist->first;

    while(a_src){
	a_dst->t = a_src->t;
	if (a_dst->t == a_list){ //recursively copy sublists (tree copy)
	    a_dst->w.w_list = pdp_tree_copy(a_src->w.w_list);
	}
	else{
	    a_dst->w = a_src->w;
	}
	a_src = a_src->next;
	a_dst = a_dst->next;
    }

    return newlist;
}
t_pdp_list* pdp_list_copy(t_pdp_list *list)
{
    t_pdp_list *newlist = pdp_list_new(list->elements);
    t_pdp_atom *a_src = list->first;
    t_pdp_atom *a_dst = newlist->first;

    while(a_src){
	a_dst->t = a_src->t;
	a_dst->w = a_src->w;
	a_src = a_src->next;
	a_dst = a_dst->next;
    }
    return newlist;
}

void pdp_list_cat (t_pdp_list *l, t_pdp_list *tail)
{
    t_pdp_list *tmp = pdp_list_copy(tail);
    l->elements += tmp->elements;
    l->last->next = tmp->first;
    l->last = tmp->last;
    _pdp_list_dealloc(tmp); //delete the list stub
    
}


/* check if a list contains an element */
int pdp_list_contains(t_pdp_list *list, t_pdp_word_type t, t_pdp_word w)
{
    t_pdp_atom *a;
    for(a = list->first; a; a=a->next){
	if (a->w.w_int == w.w_int && a->t == t) return 1;
    }
    return 0;
}

/* add a thing to the start of the list if it's not in there already */
void pdp_list_add_to_set(t_pdp_list *list, t_pdp_word_type t, t_pdp_word w)
{
    if (!pdp_list_contains(list, t, w))
	pdp_list_add(list, t, w);
}



