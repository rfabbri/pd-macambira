
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

/* the pdp list is composed of atoms. 
   the default atom is a pointer.
   lists can be recursed into trees.

   note: all functions that return t_pdp_word and don't take a type argument
   obviously don't perform any type checking. if you have heterogenous lists,
   you should use atom iterators or direct access.

*/



#ifndef PDP_LIST_H
#define PDP_LIST_H

struct _pdp_list;
struct _pdp_atom;
typedef struct _pdp_list t_pdp_list;
typedef struct _pdp_atom t_pdp_atom;



/* THE LIST OBJECT */

typedef enum {
    a_undef = 0,
    a_pointer,
    a_float,
    a_int,
    a_symbol,
    a_packet,
    a_list
} t_pdp_word_type;

typedef union _pdp_word
{
    void*                w_pointer;
    float                w_float;
    int                  w_int;
    struct _pdp_symbol*  w_symbol;
    int                  w_packet;
    struct _pdp_list*    w_list;

} t_pdp_word;

/* a list element */
struct _pdp_atom
{
    struct _pdp_atom *next;
    t_pdp_word w;
    t_pdp_word_type t;
};

/* a list container */
struct _pdp_list
{
    int elements;
    t_pdp_atom *first;
    t_pdp_atom *last;
    
};


/* CONVENTION: trees stacks and lists.

   * all operations with "list" in them operate on flat lists. all the
     items contained in the list are either pure atoms (floats, ints, or symbols) 
     or references (packets, pointers, lists)

   * all operations with "tree" in them, operate on recursive lists (trees)
     all sublists of the list (tree) are owned by the parent list, so you can't
     build trees from references to other lists.
 
   * stacks are by definition flat lists, so they can not contains sublists

*/
     
typedef void (*t_pdp_atom_method)(t_pdp_atom *);
typedef void (*t_pdp_word_method)(t_pdp_word);
typedef void (*t_pdp_pword_method)(t_pdp_word *);
typedef void (*t_pdp_free_method)(void *);

/* creation / destruction */
t_pdp_list*        pdp_list_new           (int elements);
void               pdp_list_free          (t_pdp_list *l);
void               pdp_list_clear         (t_pdp_list *l);
void               pdp_tree_free          (t_pdp_list *l);
void               pdp_tree_clear         (t_pdp_list *l);

void               pdp_tree_strip_pointers (t_pdp_list *l, t_pdp_free_method f);
void               pdp_tree_strip_packets  (t_pdp_list *l);

t_pdp_list*        pdp_list_from_cstring   (char *chardef, char **nextchar);



/* traversal routines */
void pdp_list_apply       (t_pdp_list *l, t_pdp_atom_method am);
void pdp_tree_apply       (t_pdp_list *l, t_pdp_atom_method am);
void pdp_list_apply_word_method  (t_pdp_list *l, t_pdp_word_type t, t_pdp_word_method wm);
void pdp_tree_apply_word_method  (t_pdp_list *l, t_pdp_word_type t, t_pdp_word_method wm);
void pdp_list_apply_pword_method (t_pdp_list *l, t_pdp_word_type t, t_pdp_pword_method pwm);
void pdp_tree_apply_pword_method (t_pdp_list *l, t_pdp_word_type t, t_pdp_pword_method pwm);


/* copy: (reverse) copies a list. */ 

t_pdp_list*        pdp_list_copy          (t_pdp_list *l);
t_pdp_list*        pdp_list_copy_reverse  (t_pdp_list *l);
t_pdp_list*        pdp_tree_copy          (t_pdp_list *l);
t_pdp_list*        pdp_tree_copy_reverse  (t_pdp_list *l);


/* cat: this makes a copy of the second list and adds it at the end of the first one */
void               pdp_list_cat           (t_pdp_list *l, t_pdp_list *tail);

/* information */
int     pdp_list_contains                 (t_pdp_list *l, t_pdp_word_type t, t_pdp_word w);
int     pdp_list_size                     (t_pdp_list *l);
void    pdp_list_print                    (t_pdp_list *l);

/* access */
void          pdp_list_add           (t_pdp_list *l, t_pdp_word_type t, t_pdp_word w);
void          pdp_list_add_back      (t_pdp_list *l, t_pdp_word_type t, t_pdp_word w);
void          pdp_list_add_to_set    (t_pdp_list *l, t_pdp_word_type t, t_pdp_word w);
void          pdp_list_remove        (t_pdp_list *l, t_pdp_word_type t, t_pdp_word w);

/* these don't do error checking. out of bound == error */
t_pdp_word    pdp_list_pop           (t_pdp_list *l);
t_pdp_word    pdp_list_index         (t_pdp_list *l, int index);

/* some aliases */
#define pdp_list_add_front pdp_list_add
#define pdp_list_push      pdp_list_add
#define pdp_list_queue     pdp_list_add_end
#define pdp_list_unqueue   pdp_list_pop

/* generic atom iterator */
#define PDP_ATOM_IN(list,atom)              for (atom = list->first ; atom ; atom = atom->next)

/* fast single type iterators */

/* generic */
#define PDP_WORD_IN(list, atom, word, type) for (atom=list->first ;atom && ((word = atom -> w . type) || 1); atom=atom->next)

/* type specific */
#define PDP_POINTER_IN(list, atom, x) PDP_WORD_IN(list, atom, x, w_pointer)
#define PDP_INT_IN(list, atom, x)     PDP_WORD_IN(list, atom, x, w_int)
#define PDP_FLOAT_IN(list, atom, x)   PDP_WORD_IN(list, atom, x, w_float)
#define PDP_SYMBOL_IN(list, atom, x)  PDP_WORD_IN(list, atom, x, w_symbol)
#define PDP_PACKET_IN(list, atom, x)  PDP_WORD_IN(list, atom, x, w_packet)
#define PDP_LIST_IN(list, atom, x)    PDP_WORD_IN(list, atom, x, w_list)


/* some macros for the pointer type */

#define pdp_list_add_pointer(l,p)          pdp_list_add(l, a_pointer, ((t_pdp_word)((void *)(p))))
#define pdp_list_add_back_pointer(l,p)     pdp_list_add_back(l, a_pointer, ((t_pdp_word)((void *)(p))))
#define pdp_list_add_pointer_to_set(l,p)   pdp_list_add_to_set(l, a_pointer, ((t_pdp_word)((void *)(p))))
#define pdp_list_remove_pointer(l,p)       pdp_list_remove(l, a_pointer, ((t_pdp_word)((void *)(p))))
#define pdp_list_contains_pointer(l,p)     pdp_list_contains(l, a_pointer, ((t_pdp_word)((void *)(p))))



  
#endif
