/*
 *   Pure Data Packet module.
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


/*

example object definitions:

; stack: (mix_float in_packet state_packet)

(motion-blur
 ((in 1) (feedback 0))
 ((out 0))
 (ip ip 0.0)
 (dup down mix dup down down))


; stack: (ouput_packet packet_type)

(noise-gen
 ((bang -1) (type 1))
 ((out 0))
 (image/YCrCb/320x240 ip)
 (dup noise))

; constraints:
; if a processor accepts more than one atom message (float, list, pdp)
; of the same type they MUST be remapped to other inlets

(pd-mappings
 (motion-blur ((in 0) (feedback 1) (out 0)))
 (noise       ((out 0))))


*/


#include "pdp.h"
#include "pdp_forth.h"

#define MAXINLETS 4

/* this object instantiates a forth processor */

typedef struct pdp_forthproc_struct
{
    t_object x_obj;
    t_pdp_list *x_processor; // the processor definition

    
    t_pdp_list *x_passive_stack; // we're still into thread processing
    t_pdp_list *x_stack;
    t_pdp_list *x_program;

} t_pdp_forthproc;



static int _check_portlist(t_pdp_list *l)
{
    t_pdp_atom *a;
    post ("number of ports = %d", l->elements);
    for (a = l->first; a; a=a->next){
	t_pdp_atom *b;
	if (a->t != a_list) goto error;
	if (a->w.w_list->elements != 2) goto error;
	b = a->w.w_list->first;
	if ((b->t != a_symbol) && (b->t != a_int)) goto error;
	b = b->next;
	if (b->t != a_int) goto error;
    }
    post ("port mappings:");
    for (a = l->first; a; a=a->next) pdp_list_print(a->w.w_list);
    return 1;
 error:
    return 0;
}

static int _check_processor(t_pdp_list *p)
{
    t_pdp_atom *a;
    t_pdp_list *l;

    post ("list elements = %d", p->elements);
    if (p->elements != 4) goto error;
    for (a=p->first; a; a=a->next) if (a->t != a_list) goto error;
    post ("all elements are lists: OK");

    a = p->first;
    post ("checking inlets");
    if (!_check_portlist(a->w.w_list)) goto error;

    a = a->next;
    post ("checking outlets");
    if (!_check_portlist(a->w.w_list)) goto error;

    a = a->next; post ("init program:"); pdp_list_print(a->w.w_list);
    a = a->next; post ("loop program:"); pdp_list_print(a->w.w_list);


    return 1;

 error:
    post("not a valid processor");
    return 0;
}

/* this function maps an input symbol to a type (how to interpret the
   anything message) and a stack index */
static inline void pdp_forthproc_map_symbol(t_pdp_forthproc *x, t_symbol *s, 
					    t_pdp_word_type *t, int *i)
{
}

/* this function stores a new item in an index on the stack 
   and executes the forth process if this is an active location */
static inline void pdp_forthproc_write_stack(t_pdp_forthproc *x, int i, t_pdp_word w)
{
}

static void pdp_forthproc_anything(t_pdp_forthproc *x, t_symbol *s, int argc, t_atom *argv)
{
    /* if the symbol's length is one character, it's a remapped input.
       this instance contains a mapping from symbol->(type, stack entry) */

    t_pdp_word_type type = a_undef;
    t_pdp_word word = (t_pdp_word)0;
    int index = -1;

    /* determine what we got and where we need to put it */
    pdp_forthproc_map_symbol(x, s, &type, &index); 
    
    /* interprete the anything message according to expected type.
     and put the result in w */

    /* TODO: put pd list <-> pdp list code in core (also in pdp_guile) */
    switch(type){
    case a_float:
    case a_int:
    case a_symbol:
    case a_list:
    case a_packet:
    default:
    post("got %s, and dunno what to do with it", s->s_name);
	return;
	
    }

    /* write the word to the stack 
       and run the word if this was an active location */
    pdp_forthproc_write_stack(x, index, word); 

}

static void pdp_forthproc_free(t_pdp_forthproc *x)
{
    pdp_tree_strip_packets(x->x_stack);
    pdp_tree_free(x->x_stack);
}

t_class *pdp_forthproc_class;


void *pdp_forthproc_new(t_symbol *s)
{
    t_pdp_forthproc *x;

    /* get processor */
    t_pdp_symbol *procname = pdp_gensym(s->s_name);
    if (!procname->s_processor) return 0;

    /* allocate */
    x = (t_pdp_forthproc *)pd_new(pdp_forthproc_class);
    x->x_processor = procname->s_processor;
    post("pdp_forthproc %s", s->s_name);
    pdp_list_print(x->x_processor);
    

    if (_check_processor(x->x_processor)) post ("processor OK");

    /* create state stack */
    x->x_stack = pdp_stack_new();
    x->x_passive_stack = pdp_stack_new();
    pdp_forth_execute_def(x->x_stack, x->x_processor->first->next->next->w.w_list);
    pdp_forth_execute_def(x->x_passive_stack, x->x_processor->first->next->next->w.w_list);
    post("initial stack:");
    pdp_list_print(x->x_stack);


    /* create inlets from description */
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_bang, gensym("A"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_bang, gensym("B"));
    
    

    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif


void pdp_forthproc_setup(void)
{
    int i;
    char iname[] = "i1";
    char def[] = "(((0 1) (1 0)) ((0 0)) (ip ip 0.0) (dup down mix dup down down))";

    /* create a test processor */
    pdp_gensym("testproc")->s_processor = pdp_forth_compile_def(def);

    /* create a standard pd class */
    pdp_forthproc_class = class_new(gensym("pdp_forthproc"), (t_newmethod)pdp_forthproc_new,
   	(t_method)pdp_forthproc_free, sizeof(t_pdp_forthproc), 0, A_SYMBOL, A_NULL);

    /* add global message handler */
    class_addanything(pdp_forthproc_class, (t_method)pdp_forthproc_anything);

}

#ifdef __cplusplus
}
#endif
