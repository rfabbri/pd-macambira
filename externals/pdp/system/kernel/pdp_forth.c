/*
 *   Pure Data Packet header file. Packet processor system
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

#include <stdlib.h>
#include <math.h>
#include "pdp.h"
#include "pdp_forth.h"

#define D if (0)



t_pdp_stack *pdp_stack_new(void) {return pdp_list_new(0);}

void pdp_stack_free(t_pdp_stack *s) {
    pdp_tree_strip_packets(s);
    pdp_list_free(s);
}


/* some stack manips */
t_pdp_word_error pdp_stack_dup(t_pdp_stack *s)
{
    if (!s->first) return e_underflow;
    pdp_list_add(s, s->first->t, s->first->w);

    /* copy it properly if its a packet */
    if (s->first->t == a_packet){
	s->first->w.w_packet = pdp_packet_copy_ro(s->first->w.w_packet);
    }
    return e_ok;
}

t_pdp_word_error pdp_stack_drop(t_pdp_stack *s)
{
    if (!s->first) return e_underflow;

    /* delete it properly if its a packet */
    if (s->first->t == a_packet){
	pdp_packet_mark_unused(s->first->w.w_packet);
    }
    pdp_list_pop(s);

    return e_ok;
}

t_pdp_word_error pdp_stack_over(t_pdp_stack *s)
{
    if (s->elements < 2) return e_underflow;
    pdp_list_add(s, s->first->next->t, s->first->next->w);

    /* copy it properly if its a packet */
    if (s->first->t == a_packet){
	s->first->w.w_packet = pdp_packet_copy_ro(s->first->w.w_packet);
    }
    
    return e_ok;
}

t_pdp_word_error pdp_stack_swap(t_pdp_stack *s)
{
    t_pdp_word w;
    t_pdp_word_type t;
    if (s->elements < 2) return e_underflow;
    w = s->first->w;
    t = s->first->t;
    s->first->w = s->first->next->w;
    s->first->t = s->first->next->t;
    s->first->next->w = w;
    s->first->next->t = t;
    return e_ok;
    
}

/* pushing and popping the stack */

t_pdp_word_error pdp_stack_push_float(t_pdp_stack *s,  float f)         {pdp_list_add(s, a_float,   (t_pdp_word)f); return e_ok;}
t_pdp_word_error pdp_stack_push_int(t_pdp_stack *s,    int i)           {pdp_list_add(s, a_int,     (t_pdp_word)i); return e_ok;}
t_pdp_word_error pdp_stack_push_pointer(t_pdp_stack *s,    void *x)     {pdp_list_add(s, a_pointer, (t_pdp_word)x); return e_ok;}
t_pdp_word_error pdp_stack_push_symbol(t_pdp_stack *s, t_pdp_symbol *x) {pdp_list_add(s, a_symbol,  (t_pdp_word)x); return e_ok;}

/* note: packets pushed stack are owned by the stack. if a caller wants to keep a packet that
   will be deleted by the word, it should make a copy before transferring it to the stack.
   if a stack processor wants to write to a packet, it should replace it with a writable copy first */

t_pdp_word_error pdp_stack_push_packet(t_pdp_stack *s, int p)           {pdp_list_add(s, a_packet, (t_pdp_word)p); return e_ok;}





t_pdp_word_error pdp_stack_pop_float(t_pdp_stack *s, float *f)
{
    if (!s->first) return e_underflow;

    if (s->first->t == a_float) *f = s->first->w.w_float; 
    else if (s->first->t == a_int) *f = (float)s->first->w.w_int; 
    else *f = 0.0f;
    pdp_stack_drop(s);
    return e_ok;
}

t_pdp_word_error pdp_stack_pop_int(t_pdp_stack *s, int *i)
{
    if (!s->first) return e_underflow;
    if (s->first->t == a_int) *i = s->first->w.w_int; 
    else if (s->first->t == a_float) *i = (int)s->first->w.w_float; 
    else *i = 0;
    pdp_stack_drop(s);
    return e_ok;
}

t_pdp_word_error pdp_stack_pop_pointer(t_pdp_stack *s, void **x)
{
    if (!s->first) return e_underflow;
    *x = (s->first->t == a_pointer) ? s->first->w.w_pointer : 0; 
    pdp_stack_drop(s);
    return e_ok;
}

t_pdp_word_error pdp_stack_pop_symbol(t_pdp_stack *s, t_pdp_symbol **x)
{
    if (!s->first) return e_underflow;
    *x = (s->first->t == a_symbol) ? s->first->w.w_symbol : pdp_gensym("invalid"); 
    pdp_stack_drop(s);
    return e_ok;
}

/* packets popped from the stack are owned by the caller */

t_pdp_word_error pdp_stack_pop_packet(t_pdp_stack *s, int *p)
{
    if (!s->first) return e_underflow;
    *p = (s->first->t == a_packet) ? s->first->w.w_packet : -1; 
    pdp_list_pop(s); //ownership is transferred to receiver, drop kills the packet
    return e_ok;
}


t_pdp_word_error pdp_stack_mov(t_pdp_stack *s)
{
    int position;
    t_pdp_atom *a, *a_before;
    if (s->elements < 2) return e_underflow;
    if (s->first->t != a_int) return e_type;

    pdp_stack_pop_int(s, &position); // get insert point
    if (position < 1) return e_ok; // < 0 : invalid; do nothing, 0 : nop (= insert at start, but already at start)
    if ((s->elements-1) < position) return e_underflow;

    a = s->first; // get first atom
    s->first = a->next;

    if (s->elements-1 == position){ //insert at end
	s->last->next = a;
	a->next = 0;
	s->last = a;
    }
    else { //insert somewhere in the middle
	a_before = s->first;
	while (--position) a_before = a_before->next;
	a->next = a_before->next;
	a_before->next = a;
    }
    return e_ok;
}

/* rotate stack down (tos -> bottom) */
t_pdp_word_error pdp_stack_rdown(t_pdp_stack *s)
{
    t_pdp_word_type t = s->first->t;
    t_pdp_word w = s->first->w;
    pdp_list_pop(s);
    pdp_list_add_back(s, t, w);
    return e_ok;
}


/* convert to int */
t_pdp_word_error pdp_stack_int(t_pdp_stack *s)
{
    int i;
    pdp_stack_pop_int(s, &i);
    pdp_stack_push_int(s, i);
    return e_ok;
}

/* convert to float */
t_pdp_word_error pdp_stack_float(t_pdp_stack *s)
{
    float f;
    pdp_stack_pop_float(s, &f);
    pdp_stack_push_float(s, f);
    return e_ok;
}


#define OP1(name, type, op)					\
t_pdp_word_error pdp_stack_##name##_##type (t_pdp_stack *s)	\
{								\
    type x0;							\
    pdp_stack_pop_##type (s, &(x0));				\
    pdp_stack_push_##type (s, op (x0));				\
    return e_ok;						\
}

#define OP2(name, type, op)					\
t_pdp_word_error pdp_stack_##name##_##type (t_pdp_stack *s)	\
{								\
    type x0, x1;						\
    pdp_stack_pop_##type (s, &(x0));				\
    pdp_stack_pop_##type (s, &(x1));				\
    pdp_stack_push_##type (s, x1 op x0);			\
    return e_ok;						\
}

/* some floating point and integer stuff */

OP2(add, float, +);
OP2(sub, float, -);
OP2(mul, float, *);
OP2(div, float, /);

OP1(sin, float, sin);
OP1(cos, float, cos);

OP2(add, int, +);
OP2(sub, int, -);
OP2(mul, int, *);
OP2(div, int, /);
OP2(mod, int, %);

OP2(and, int, &);
OP2(or, int, |);
OP2(xor, int, ^);


/* some integer stuff */

t_pdp_word_error pdp_stack_push_invalid_packet(t_pdp_stack *s)
{
    pdp_stack_push_packet(s, -1);
    return e_ok;
}

/* dictionary manipulation */

void pdp_forth_word_print_debug(t_pdp_symbol *s)
{
    t_pdp_atom *a;
    if (!s->s_word_spec){
	post("%s is not a forth word", s->s_name);
    }
    else{
	post("");
	post("forth word %s", s->s_name);
	post("\tinput: %d", s->s_word_spec->input_size);
	post("\toutput: %d", s->s_word_spec->output_size);
	post("\ttype index: %d", s->s_word_spec->type_index);

	post("\nimplementations:");
	for(a=s->s_word_spec->implementations->first; a; a=a->next){
	    t_pdp_forthword_imp *i = a->w.w_pointer;
	    startpost("\t%s\t", i->type ? i->type->s_name : "anything");
	    pdp_list_print(i->def);
	    
	}
	post("");
    }
}

/* add a definition (list of high level words (symbols) or primitive routines) */
void pdp_forthdict_add_word(t_pdp_symbol *name, t_pdp_list *def, int input_size, int output_size,
			    int type_index, t_pdp_symbol *type)
{
    t_pdp_forthword_spec *spec = 0;
    t_pdp_forthword_imp *imp = 0;
    t_pdp_forthword_imp *old_imp = 0;
    t_pdp_atom *a;
    /* check if the word complies to a previously defined word spec with the same name */
    if (spec = name->s_word_spec){
	if ((spec->input_size != input_size)
	    ||(spec->output_size != output_size)
	    ||(spec->type_index != type_index)){
	    post("ERROR: pdp_forthdict_add_word: new implementation of [%s] does not comply to old spec",
		 name->s_name);
	    return;
	}

    }
    /* no previous word spec with this name, so create a new spec */
    else{
	spec = name->s_word_spec = (t_pdp_forthword_spec *)pdp_alloc(sizeof(t_pdp_forthword_spec));
	spec->name = name;
	spec->input_size = input_size;
	spec->output_size = output_size;
	spec->type_index = type_index;
	spec->implementations = pdp_list_new(0);
    }
	
    /* create the new implementation and add it */
    imp = (t_pdp_forthword_imp *)pdp_alloc(sizeof(t_pdp_forthword_imp));
    imp->name = name;
    imp->def = def;
    imp->type = type;

    /* can't delete old implemetations because of thread safety */
    pdp_list_add_pointer(spec->implementations, imp);
	
}

/* add a primitive */
void pdp_forthdict_add_primitive(t_pdp_symbol *name, t_pdp_forthword w, int input_size, int output_size,
				 int type_index, t_pdp_symbol *type)
{
    t_pdp_list *def = pdp_list_new(1);
    def->first->t = a_pointer;
    def->first->w.w_pointer = w;
    pdp_forthdict_add_word(name, def, input_size, output_size, type_index, type);
}

/* parse a new definition from a null terminated string */
t_pdp_list *pdp_forth_compile_def(char *chardef)
{
    t_pdp_list *l;
    char *c;

    if (!(l = pdp_list_from_cstring(chardef, &c))){
	post ("ERROR: pdp_forth_compile_def: parse error parsing: %s", chardef);
	if (*c) post ("ERROR: remaining input: %s", c);
    }
    if (*c){
	post ("WARNING: pdp_forth_compile_def: parsing: %s", chardef);
	if (*c) post ("garbage at end of string: %s", c);
    }

    return l;

}

void pdp_forthdict_compile_word(t_pdp_symbol *name, char *chardef, int input_size, int output_size,
				int type_index, t_pdp_symbol *type)
{
    /* add the definition list to the dictionary */
    t_pdp_list *def;

    if (def = pdp_forth_compile_def (chardef))
	pdp_forthdict_add_word(name, def, input_size, output_size, type_index, type);
    
    
}



/* execute a definition list
   a def list is a list of primitives, immediates or symbolic words */
t_pdp_word_error pdp_forth_execute_def(t_pdp_stack *stack, t_pdp_list *def)
{
    t_pdp_atom *a;
    t_pdp_word_error e;
    t_pdp_forthword w;
    float f;
    int i,p;
    
    D post("pdp_forth_execute_def %x %x", stack, def);
    D pdp_list_print(def);

    for (a = def->first; a; a=a->next){
	switch(a->t){
	case a_float: // an immidiate float
            f = a->w.w_float;
	    D post("pushing %f onto the stack", f);
	    pdp_stack_push_float(stack, f);
	    break;
	case a_int:  // an immidiate int
            i = a->w.w_int;
	    D post("pushing %d onto the stack", i);
	    pdp_stack_push_int(stack, i);
	    break;
	case a_packet:  // an immidiate int
            p = a->w.w_packet;
	    D post("pushing packet %d onto the stack", p);
	    pdp_stack_push_packet(stack, pdp_packet_copy_ro(p));
	    break;
	case a_symbol:  // a high level word or an immediate symbol
	    D post("interpeting symbol %s", a->w.w_symbol->s_name);
	    if (e = pdp_forth_execute_word(stack, a->w.w_symbol)) return e;
	    break;
	case a_pointer: // a primitive
	    w = a->w.w_pointer;
	    D post("exec primitive %x", w);
	    if (e = (w(stack))) return e;
	    break;
	default:
	    return e_internal;

	}
    }
    return e_ok;
}

/* execute a symbol (a high level word or an immediate) 
   this routine does the type based word multiplexing and stack checking */
t_pdp_word_error pdp_forth_execute_word(t_pdp_stack *stack, t_pdp_symbol *word)
{
    t_pdp_symbol *type = 0;
    t_pdp_atom *a;
    t_pdp_forthword_spec *spec;
    t_pdp_forthword_imp *imp = 0;
    int i;

    D post("pdp_forth_execute_word %x %x", stack, word);

    /* first check if the word is defined. if not, the symbol will be loaded
       onto the stack as an immidiate symbol */

    if (!(spec = word->s_word_spec)){
	D post ("pushing symbol %s on the stack", word->s_name);
	pdp_stack_push_symbol(stack, word);
	return e_ok;
    }

    D post("exec high level word [%s]", word->s_name);

    /* it is a word. check the stack size */
    if (stack->elements < spec->input_size){
	D post ("error executing [%s]: stack underflow", word->s_name);
	return e_underflow;
    }

    /* if the word is type oblivious, symply execute the first (only)
       implementation in the list */
    if (spec->type_index < 0){
	D post("exec type oblivious word [%s]", word->s_name);
	imp = spec->implementations->first->w.w_pointer;
	return pdp_forth_execute_def(stack , imp->def);
    }

    /* if it is not type oblivious, find the type template
       to determine the correct implementation */

    for(i=spec->type_index,a=stack->first; i--; a=a->next);
    switch (a->t){
      /* get type description from first item on*/
    case a_packet: 
	type = pdp_packet_get_description(a->w.w_packet); break;
    case a_symbol:
	type = a->w.w_symbol; break;
    case a_float:
	type = pdp_gensym("float"); break;
    case a_int:
	type = pdp_gensym("int"); break;
    case a_pointer:
	type = pdp_gensym("pointer"); break;
    default:
      /* no type description found on top of stack. */
        type = pdp_gensym("unknown");
	break;
    }

    /* scan the implementation list until a definition with matching type is found
       if the type spec for a word is NULL, it counts as a match (for generic words) */
    for (a = spec->implementations->first; a; a = a->next){
	imp = a->w.w_pointer;
	if ((!imp->type) || pdp_type_description_match(type, imp->type)){
	    return pdp_forth_execute_def(stack , imp->def);
	}
    }
    D post("ERROR: pdp_forth_execute_word: type error executing [%s] (2). stack:",word->s_name);
    D pdp_list_print(stack);

    return e_type; // type error
    
}


static void _add_2op(char *name, t_pdp_forthword w, char *type){
    pdp_forthdict_add_primitive(pdp_gensym(name), w, 2, 1, 0, pdp_gensym(type));
}

static void _add_1op(char *name, t_pdp_forthword w, char *type){
    pdp_forthdict_add_primitive(pdp_gensym(name), w, 1, 1, 0, pdp_gensym(type));
}


void pdp_forth_setup(void)
{

    /* add type oblivious (type_index = -1, type = NULL) stack manip primitives */
    pdp_forthdict_add_primitive(pdp_gensym("dup"),    (t_pdp_forthword)pdp_stack_dup,    1, 2, -1, 0);
    pdp_forthdict_add_primitive(pdp_gensym("swap"),   (t_pdp_forthword)pdp_stack_swap,   2, 2, -1, 0);
    pdp_forthdict_add_primitive(pdp_gensym("drop"),   (t_pdp_forthword)pdp_stack_drop,   1, 0, -1, 0);
    pdp_forthdict_add_primitive(pdp_gensym("over"),   (t_pdp_forthword)pdp_stack_over,   2, 3, -1, 0);
    pdp_forthdict_add_primitive(pdp_gensym("mov"),    (t_pdp_forthword)pdp_stack_mov,    2, 1, -1, 0);
    pdp_forthdict_add_primitive(pdp_gensym("down"),  (t_pdp_forthword)pdp_stack_rdown,  1, 1, -1, 0);

    /* type converters (casts) */
    pdp_forthdict_add_primitive(pdp_gensym("int"),  (t_pdp_forthword)pdp_stack_int,  1, 1, -1, 0);
    pdp_forthdict_add_primitive(pdp_gensym("float"),  (t_pdp_forthword)pdp_stack_float,  1, 1, -1, 0);

    /* add floating point ops */
    _add_2op("add", (t_pdp_forthword)pdp_stack_add_float, "float");
    _add_2op("sub", (t_pdp_forthword)pdp_stack_sub_float, "float");
    _add_2op("mul", (t_pdp_forthword)pdp_stack_mul_float, "float");
    _add_2op("div", (t_pdp_forthword)pdp_stack_div_float, "float");

    _add_1op("sin", (t_pdp_forthword)pdp_stack_sin_float, "float");
    _add_1op("cos", (t_pdp_forthword)pdp_stack_cos_float, "float");

    /* add integer ops */
    _add_2op("add", (t_pdp_forthword)pdp_stack_add_int, "int");
    _add_2op("sub", (t_pdp_forthword)pdp_stack_sub_int, "int");
    _add_2op("mul", (t_pdp_forthword)pdp_stack_mul_int, "int");
    _add_2op("div", (t_pdp_forthword)pdp_stack_div_int, "int");
    _add_2op("mod", (t_pdp_forthword)pdp_stack_mod_int, "int");

    _add_2op("and", (t_pdp_forthword)pdp_stack_and_int, "int");
    _add_2op("or", (t_pdp_forthword)pdp_stack_or_int, "int");
    _add_2op("xor", (t_pdp_forthword)pdp_stack_xor_int, "int");

    /* some immidiates */
    pdp_forthdict_add_primitive(pdp_gensym("ip"),  (t_pdp_forthword)pdp_stack_push_invalid_packet,  0, 1, -1, 0);
    
}
