/*
 *   Pure Data Packet system implementation. : code for handling different packet types
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


/* this file contains type handling routines */

#include <stdarg.h>
#include <string.h>
#include <pthread.h>
#include "pdp.h"


// debug
#define D if (0)


static t_pdp_list *conversion_list;

#define INIT_MAX_CACHE_SIZE 32

static t_pdp_list *cached_conversion_list;
static int max_cache_size;

/* mutex */
static pthread_mutex_t pdp_hash_mutex;
static pthread_mutex_t pdp_conversion_mutex;
static pthread_mutex_t pdp_cache_mutex;

#define HASHSIZE 1024
static t_pdp_symbol *pdp_symhash[HASHSIZE];


/* shamelessly copied from pd src and made thread safe */
t_pdp_symbol *_pdp_dogensym(char *s, t_pdp_symbol *oldsym)
{
    t_pdp_symbol **sym1, *sym2;
    unsigned int hash1 = 0,  hash2 = 0;
    int length = 0;
    char *s2 = s;
    while (*s2)
    {
        hash1 += *s2;
        hash2 += hash1;
        length++;
        s2++;
    }
    sym1 = pdp_symhash + (hash2 & (HASHSIZE-1));

    /* lock hash */
    pthread_mutex_lock(&pdp_hash_mutex);

    while (sym2 = *sym1)
    {
        if (!strcmp(sym2->s_name, s)) goto gotit;
        sym1 = &sym2->s_next;
    }
    if (oldsym) sym2 = oldsym;
    else
    {
        sym2 = (t_pdp_symbol *)pdp_alloc(sizeof(*sym2));
        sym2->s_name = pdp_alloc(length+1);
	_pdp_symbol_clear_namespaces(sym2);
        sym2->s_next = 0;
        strcpy(sym2->s_name, s);
    }
    *sym1 = sym2;

 gotit:

    /* unlock hash */
    pthread_mutex_unlock(&pdp_hash_mutex);
    return (sym2);
}

t_pdp_symbol *pdp_gensym(char *s)
{
    return(_pdp_dogensym(s, 0));
}

/* convert a type to a list */
t_pdp_list *pdp_type_to_list(t_pdp_symbol *type)
{
    char *c = type->s_name;
    char *lastname = c;
    char tmp[100];
    int n = 0;
    t_pdp_list *l = pdp_list_new(0);

    while(*c){
	if (*c == '/'){
	    strncpy(tmp, lastname, n);
	    tmp[n] = 0;
	    pdp_list_add_back(l, a_symbol, (t_pdp_word)pdp_gensym(tmp));
	    c++;
	    lastname = c;
	    n = 0;
	}
	else{
	    c++;
	    n++;
	}
    }
    pdp_list_add_back(l, a_symbol, (t_pdp_word)pdp_gensym(lastname));

    return l;
}


/* get the description symbol. this includes some compat transition stuff with a warning */
t_pdp_symbol *pdp_packet_get_description(int packet)
{
    t_pdp *header = pdp_packet_header(packet);

    if (!header) return pdp_gensym("invalid");
    else if (!header->desc){

	post("ERROR: pdp_packet_get_description: packet %d has no description.", packet);
	pdp_packet_print_debug(packet);
	return pdp_gensym("unknown");
    }
    else return header->desc;
}



/* this runs a conversion program */
int _pdp_type_run_conversion_program(t_pdp_conversion_program *program,
	 			    int packet, t_pdp_symbol *dest_template)
{
    /* run a conversion program:
       treat the source packet as readonly, and cleanup intermediates, such
       that the net result is the production of a new packet, with the
       source packet intact. */

    int p, tmp;
    t_pdp_atom *a;
    t_pdp_conversion_method m;

    // run the first line of the program
    a = program->first;
    m = a->w.w_pointer;
    D post("DEBUG: _pdp_type_run_conversion_program: method = %x", m);
    p = m(packet, dest_template);
    D post("DEBUG: _pdp_type_run_conversion_program: packet returned = %d, type = %s", p, pdp_packet_get_description(p)->s_name);

    // run the remaining lines + cleanup intermediates
    for (a=a->next; a; a=a->next){
	m = a->w.w_pointer;
	D post("DEBUG: _pdp_type_run_conversion_program: next method ptr = %x", m);
	tmp = m(p, dest_template);
	pdp_packet_mark_unused(p);
	p = tmp;
    }
    return p;
}


/* find a conversion program */
t_pdp_conversion_program *
_pdp_type_find_conversion_program(t_pdp_symbol *src_pattern, t_pdp_symbol *dst_pattern)
{
    t_pdp_conversion *c;
    t_pdp_atom *a;
    t_pdp_conversion_program *retval = 0;

    /* lock conversion list */
    pthread_mutex_lock(&pdp_conversion_mutex);

    for (a = conversion_list->first; a; a=a->next){
	c = a->w.w_pointer;
	/* can be a wildcard match */
	if (pdp_type_description_match(src_pattern, c->src_pattern) &&
	    pdp_type_description_match(dst_pattern, c->dst_pattern)) {
	    /* found a program */
	    D post("DEBUG: _pdp_type_find_conversion_program: found: %s -> %s", c->src_pattern->s_name, c->dst_pattern->s_name);
	    retval = c->program;
	    goto gotit;
	}
    }

    /* no conversion program was found */
    retval = 0;
 gotit:

    /* lock conversion list */
    pthread_mutex_unlock(&pdp_conversion_mutex);
    return retval;
}

/* find a cached conversion program 
 if one is found it will be moved to the back of the queue (MRU) */
t_pdp_conversion_program *
_pdp_type_find_cached_conversion_program(t_pdp_symbol *src_pattern, t_pdp_symbol *dst_pattern)
{
    t_pdp_conversion *c, *c_tmp;
    t_pdp_atom *a;
    t_pdp_conversion_program *retval = 0;

    /* lock cached list */
    pthread_mutex_lock(&pdp_cache_mutex);

    for (a = cached_conversion_list->first; a; a=a->next){
	c = a->w.w_pointer;
	/* must be exact match */
	if ((src_pattern == c->src_pattern) &&
	    (dst_pattern == c->dst_pattern)) {

	    /* found a program */
	    D post("DEBUG: _pdp_type_find_cached_conversion_program: found: %s -> %s", c->src_pattern->s_name, c->dst_pattern->s_name);
	    retval = c->program;

	    /* make MRU (move to back) */
	    c_tmp = cached_conversion_list->last->w.w_pointer;
	    cached_conversion_list->last->w.w_pointer = c;
	    a->w.w_pointer = c_tmp;
	    goto gotit;
	}
    }

    retval = 0;

 gotit:


    /* un lock cached list */
    pthread_mutex_unlock(&pdp_cache_mutex);

    /* no conversion program was found */
    return retval;
}


/* conversion program manipulations */
void pdp_conversion_program_free(t_pdp_conversion_program *program)
{
    pdp_list_free(program);
}

/* debug print */
void _pdp_conversion_program_print(t_pdp_conversion_program *program)
{
    post("_pdp_conversion_program_print %x", program);
    pdp_list_print(program);
}

t_pdp_conversion_program *pdp_conversion_program_new(t_pdp_conversion_method method, ...)
{
    t_pdp_conversion_program *p = pdp_list_new(0);
    t_pdp_conversion_method m = method;
    va_list ap;

    D post("DEBUG: pdp_conversion_program_new:BEGIN");

    pdp_list_add_back_pointer(p, m);
    va_start(ap, method);
    while (m = va_arg(ap, t_pdp_conversion_method)) pdp_list_add_back_pointer(p, m);
    va_end(ap);

    D post("DEBUG: pdp_conversion_program_new:END");
 
    return p;
}

t_pdp_conversion_program *pdp_conversion_program_copy(t_pdp_conversion_program *program)
{
    if (program) return pdp_list_copy(program);
    else return 0;
}

void pdp_conversion_program_add(t_pdp_conversion_program *program, t_pdp_conversion_program *tail)
{ 
    return pdp_list_cat(program, tail);
}

/* conversion registration */
void pdp_type_register_conversion (t_pdp_symbol *src_pattern, t_pdp_symbol *dst_pattern, t_pdp_conversion_program *program)
{
    t_pdp_conversion *c = (t_pdp_conversion *)pdp_alloc(sizeof(*c));
    c->src_pattern = src_pattern;
    c->dst_pattern = dst_pattern;
    c->program = program;

    /* lock conversion list */
    pthread_mutex_lock(&pdp_conversion_mutex);

    pdp_list_add_back_pointer(conversion_list, c);

    /* unlock conversion list */
    pthread_mutex_unlock(&pdp_conversion_mutex);
    
}

/* register a cached conversion */
void pdp_type_register_cached_conversion (t_pdp_symbol *src_pattern, t_pdp_symbol *dst_pattern, t_pdp_conversion_program *program)
{

    /* create the new conversion */
    t_pdp_conversion *c = (t_pdp_conversion *)pdp_alloc(sizeof(*c));
    c->src_pattern = src_pattern;
    c->dst_pattern = dst_pattern;
    c->program = program;

    /* lock cached conversion list */
    pthread_mutex_lock(&pdp_cache_mutex);

    /* check size, and remove LRU (top) if the cache is full */
    while (cached_conversion_list->elements >= max_cache_size){
	t_pdp_conversion *c_old = pdp_list_pop(cached_conversion_list).w_pointer;
	if (c_old->program) pdp_conversion_program_free(c_old->program);
	pdp_dealloc(c_old);
    }
    
    /* add and make MRU (back) */
    pdp_list_add_back_pointer(cached_conversion_list, c);

    /* unlock cached conversion list */
    pthread_mutex_unlock(&pdp_cache_mutex);
}

/* convert a given packet to a certain type (template) */
int _pdp_packet_convert(int packet, t_pdp_symbol *dest_template)
{
    t_pdp_symbol *type = pdp_packet_get_description(packet);
    t_pdp_symbol *tmp_type = 0;
    int tmp_packet = -1;

    t_pdp_conversion_program *program = 0;
    t_pdp_conversion_program *program_last = 0;
    t_pdp_conversion_program *program_tail = 0;

    /* check if there is a program in the cached list, if so run it */
    if (program = _pdp_type_find_cached_conversion_program(type, dest_template))
	return _pdp_type_run_conversion_program(program, packet, dest_template);

    /* if it is not cached, iteratively convert 
       and save program on top of cache list if a conversion path (program) was found */

    // run first conversion that matches
    program = pdp_conversion_program_copy(_pdp_type_find_conversion_program(type, dest_template));
    program_last = program;
    if (!program){
	D post("DEBUG: pdp_type_convert: (1) can't convert %s to %s", type->s_name, dest_template->s_name);
	return -1;
    }
    tmp_packet = _pdp_type_run_conversion_program(program, packet, dest_template);
    tmp_type = pdp_packet_get_description(tmp_packet);

    // run more conversions if necessary, deleting intermediate packets
    while (!pdp_type_description_match(tmp_type, dest_template)){
	int new_packet;
	program_tail = _pdp_type_find_conversion_program(tmp_type, dest_template);
	if (!program_tail){
	    D post("DEBUG: pdp_type_convert: (2) can't convert %s to %s", tmp_type->s_name, dest_template->s_name);
	    pdp_packet_mark_unused(tmp_packet);
	    pdp_conversion_program_free(program);
	    return -1;
	}
	if (program_last == program_tail){
	    post("ERROR: pdp_packet_convert: conversion loop detected");
	}
	program_last = program_tail;

	pdp_conversion_program_add(program, program_tail);
	new_packet = _pdp_type_run_conversion_program(program_tail, tmp_packet, dest_template);
	pdp_packet_mark_unused(tmp_packet);
	tmp_packet = new_packet;
	tmp_type = pdp_packet_get_description(tmp_packet);
    }

    // save the conversion program
    pdp_type_register_cached_conversion(type, dest_template, program);

    // return resulting packet
    return tmp_packet;
	
}

/* convert or copy ro */
int pdp_packet_convert_ro(int packet, t_pdp_symbol *dest_template)
{
    t_pdp_symbol *type = pdp_packet_get_description(packet);

    /* if it is compatible, return a ro copy */
    if (pdp_type_description_match(type, dest_template)) return pdp_packet_copy_ro(packet);

    /* if not, convert to a new type */
    else return _pdp_packet_convert(packet, dest_template);
}

/* convert or copy rw */
int pdp_packet_convert_rw(int packet, t_pdp_symbol *dest_template)
{
    t_pdp_symbol *type = pdp_packet_get_description(packet);

    /* if it is compatible, just return a rw copy */
    if (pdp_type_description_match(type, dest_template)) return pdp_packet_copy_rw(packet);

    /* if not, convert to a new type */
    else return _pdp_packet_convert(packet, dest_template);
}


static void _setup_type_cache(t_pdp_symbol *s)
{
    t_pdp_list *l = pdp_type_to_list(s); 
    pthread_mutex_lock(&pdp_hash_mutex); //lock to prevent re-entry
    if (!s->s_type){
	s->s_type = l;
	l = 0;
    }
    pthread_mutex_unlock(&pdp_hash_mutex);
    if (l) pdp_list_free(l);
}

/* check if a type description fits a template
   this function is symmetric */
int pdp_type_description_match(t_pdp_symbol *description, t_pdp_symbol *pattern)
{
    int retval = 0;

    t_pdp_atom *ad, *ap;
    t_pdp_symbol *wildcard = pdp_gensym("*");


    if (!(pattern)) post("PANICA");
    if (!(description)) post("PANICB");


    /* same type -> match */
    if (description == pattern) {retval = 1; goto done;}

    /* use the description list stored in the symbol for comparison */
    if (!(description->s_type)) _setup_type_cache(description);
    if (!(pattern->s_type)) _setup_type_cache(pattern);

    if (!(pattern->s_type)) post("PANIC1");
    if (!(description->s_type)) post("PANIC2");

    /* check size */
    if (description->s_type->elements != pattern->s_type->elements){retval = 0;	goto done;}

    /* compare symbols of type list */
    for(ad=description->s_type->first, ap=pattern->s_type->first; ad; ad=ad->next, ap=ap->next){
	if (ad->w.w_symbol == wildcard) continue;
	if (ap->w.w_symbol == wildcard) continue;
	if (ad->w.w_symbol != ap->w.w_symbol) {retval = 0; goto done;} /* difference and not a wildcard */
    }

    /* type templates match */
    retval = 1;

 done:
    D post("DEBUG: testing match between %s and %s: %s", 
	   description->s_name, pattern->s_name, retval ? "match" : "no match");
    return retval;

}





/* setup method */
void pdp_type_setup(void)
{
    int i;

    // create mutexes
    pthread_mutex_init(&pdp_hash_mutex, NULL);
    pthread_mutex_init(&pdp_conversion_mutex, NULL);
    pthread_mutex_init(&pdp_cache_mutex, NULL);

    // init symbol hash
    memset(pdp_symhash, 0, HASHSIZE * sizeof(t_pdp_symbol *));

    // create conversion lists
    cached_conversion_list = pdp_list_new(0);
    conversion_list = pdp_list_new(0);
    max_cache_size = INIT_MAX_CACHE_SIZE;




}
