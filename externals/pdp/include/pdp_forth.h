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

#ifndef __PDP_PROCESSOR__
#define __PDP_PROCESSOR__

#include "pdp_list.h"

/*

*/   


/*
  PDP Processor Model

  The model consists of two parts. 
  
  One being the forth system to provide a lowish level description of packet operations, 
  with a form of polymorphy that enables the system to call the right routines or 
  provide automatic conversion of packet types.

  Two being the object description language. It describes processors in terms of packet
  operations. In fact it is a forth process that sets up the input and output of the
  processors, and organizes the local storage into registers.

*/


/* PDP FORTH (low level stack processors) */

/* the error codes for a processor word */
typedef enum {
    e_ok = 0,
    e_type,        // a recoverable type error (stack untouched)
    e_garbled,     // a non recoverable error (stack messed up)
    e_underflow,   // a stack underflow error 
    e_undef,       // an undefined word error
    e_internal     // an internal error (abort)
} t_pdp_word_error;

typedef struct _pdp_list t_pdp_stack;

/* a pdp word operates on a stack containing atoms (floats, ints, symbols, packets)
   it should return zero on success and an error value on failure.
   whenever a (type) error occurs, the stack should be left untouched (this ensures the system
   can try another implementation of the same word. 

   the type of the first packet on the stack determines the 'type' of the processor method.
   this poses a problem for generating words (i.e pdp_noise or pdp_plasma). in that case
   the first word should be a symbol, indicating the type. if the first item is neither
   a symbol or a packet, the first word in the word list for the respective word symbol
   is called.

   words cannot be removed from the symbol table. (because compiled scripts will have
   their addresses stored) however, they can be redefined, since new definitions are
   appended to the start of a word list, and upon execution, the first (type) matching word
   is called.

   words can be accessed in two ways:
   * the central dispatching system, which chooses a word based on it's symbol and 
     the first item on the stack (a packet's class (or unique type description) or 
     a symbol describing a type) 
   * by calling it's code directly. this can be used to optimize (compile) scripts.

   a word may place invalid packets (-1) on the stack and must be robust at receiving
   them. this is a way to silently recover from non fatal errors.

   a stack error is always fatal and should abort execution. (e_garbled)

*/

typedef int (*t_pdp_forthword)(t_pdp_stack *); 


/* a dictionary item is the definition of a word.
   if a word is type oblivious, it is a high level word, which means it can operate on any type */


/* a struct describing a forth word implementation */
typedef struct _pdp_forthword_imp
{
    struct _pdp_symbol *name;             // the name of this word
    struct _pdp_list   *def;              // the word definition with symbols (word or immediate symbol),
                                          // pointer (primitive word) or int, float and packet immediates.
    struct _pdp_symbol *type;             // the type template this low level word operates on (0 == high level word)
} t_pdp_forthword_imp;

/* a struct describing a forth word specification 
   one specification can have multiple implementations (for type multiplexing) */
typedef struct _pdp_forthword_spec
{
    /* name of this forth word */
    struct _pdp_symbol *name;

    /* stack effect */
    int input_size; 
    int output_size;

    /* index of the stack element to be used for implementation
       multiplexing based on the implementation type template */
    int type_index;

    /* a list of implementations for this word */
    t_pdp_list *implementations;

} t_pdp_forthword_spec;



/* compile a string definition to a list definition */
t_pdp_list *pdp_forth_compile_def(char *chardef);


/* add words to the forth dictionary 
   if the word is type oblivious, the type index should be -1 and the type symbol NULL */
void pdp_forthdict_add_word(t_pdp_symbol *name, t_pdp_list *def, int input_size, int output_size,
			    int type_index, t_pdp_symbol *type);
void pdp_forthdict_add_primitive(t_pdp_symbol *name, t_pdp_forthword w,int input_size, int output_size,
			    int type_index, t_pdp_symbol *type);
void pdp_forthdict_compile_word(t_pdp_symbol *name, char *chardef, int input_size, int output_size,
			    int type_index, t_pdp_symbol *type);



/* execute a word definition (a program) or a single symbolic word */
t_pdp_word_error pdp_forth_execute_def(t_pdp_stack *stack, t_pdp_list *def);
t_pdp_word_error pdp_forth_execute_word(t_pdp_stack *stack, t_pdp_symbol *word);


/* PDP PDL (high level processor description language) */

/* this uses the forth words to describe a processor: defining inputs and outputs,
   preparing the stack, storing intermediates, .. */

/* TODO */


void pdp_forth_word_print_debug(t_pdp_symbol *s);





/* STACK OPERATIONS */


/* connecting the stack to the world */
t_pdp_word_error pdp_stack_push_float(t_pdp_stack *, float f);    // push thing to tos
t_pdp_word_error pdp_stack_push_int(t_pdp_stack *s,    int i);
t_pdp_word_error pdp_stack_push_symbol(t_pdp_stack *s, t_pdp_symbol *x);
t_pdp_word_error pdp_stack_push_packet(t_pdp_stack *s, int p);

t_pdp_word_error pdp_stack_pop_float(t_pdp_stack *, float *f);          // pop thing from tos
t_pdp_word_error pdp_stack_pop_int(t_pdp_stack *, int *i);
t_pdp_word_error pdp_stack_pop_symbol(t_pdp_stack *, t_pdp_symbol **s);
t_pdp_word_error pdp_stack_pop_packet(t_pdp_stack *, int *p);


/* stack operations */

// DUP SWAP DROP OVER ...
t_pdp_word_error pdp_stack_dup(t_pdp_stack *s);
t_pdp_word_error pdp_stack_swap(t_pdp_stack *s);
t_pdp_word_error pdp_stack_drop(t_pdp_stack *s);
t_pdp_word_error pdp_stack_over(t_pdp_stack *s);


/* some util stuff */
t_pdp_stack *pdp_stack_new(void);
void pdp_stack_free(t_pdp_stack *s);



#endif
