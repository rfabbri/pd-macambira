/*
 *   Pure Data Packet system implementation: Packet Manager
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


#include "pdp.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>


/* TODO:
   implement an indexing system to the packet pool (array) to speed up searches.
   -> a list of lists of recycled packes, arranged by packet type.
   -> a list of unused slots
*/
   


#define D if (0)

/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif


/* pdp specific constants */
#define PDP_ALIGN 8

/* this needs to be able to grow dynamically, think about it later */
/* for ordinary work, this is enough and can help finding memory leaks */

#define PDP_INITIAL_POOL_SIZE 1
#define PDP_PACKET_MAX_MEM_USAGE -1

/* the pool */
static unsigned int pdp_packet_mem_usage;
static unsigned int pdp_packet_max_mem_usage;
static int pdp_pool_size;
static t_pdp** pdp_pool;

/* this is for detecting memory leaks */
static int pdp_packet_count;

/* some global vars */
static t_symbol* pdp_sym_register_rw;
static t_symbol* pdp_sym_register_ro;
static t_symbol* pdp_sym_process;

/* mutex */
static pthread_mutex_t pdp_pool_mutex;

/* the list of classes */
static t_pdp_list *class_list;

/* debug */
void
pdp_packet_print_debug(int packet)
{
    t_pdp *h = pdp_packet_header(packet);
    post("debug info for packet %d", packet);
    if (!h){
	post("invalid packet");
    }
    else{
	post ("\ttype: %d", h->type);
	post ("\tdesc: %s", h->desc ? h->desc->s_name : "unknown");
	post ("\tsize: %d", h->size);
	post ("\tflags: %x", h->flags);
	post ("\tusers: %d", h->users);
	post ("\trefloc: %x", h->refloc);
	post ("\tclass: %x", h->theclass);
    }
}



/* setup methods */

void 
pdp_packet_setup(void)
{

    pdp_pool_size = PDP_INITIAL_POOL_SIZE;
    pdp_packet_count = 0;
    pdp_packet_mem_usage = 0;
    pdp_packet_max_mem_usage = PDP_PACKET_MAX_MEM_USAGE;
    pdp_pool = (t_pdp **)malloc(PDP_INITIAL_POOL_SIZE * sizeof(t_pdp *));
    bzero(pdp_pool, pdp_pool_size * sizeof(t_pdp *));
    pdp_sym_register_rw = gensym("register_rw");
    pdp_sym_register_ro = gensym("register_ro");
    pdp_sym_process = gensym("process");
    pdp_packet_count = 0;
    class_list = pdp_list_new(0);

    pthread_mutex_init(&pdp_pool_mutex, NULL);
}

/* class methods */
t_pdp_class *pdp_class_new(t_pdp_symbol *type, t_pdp_factory_method create){
    t_pdp_class *c = (t_pdp_class *)pdp_alloc(sizeof(t_pdp_class));
    memset(c, 0, sizeof(t_pdp_class));
    c->create = create;
    c->type = type; // set type
    //c->attributes = pdp_list_new(0); // create an empty attribute list
    pdp_list_add(class_list, a_pointer, (t_pdp_word)((void *)c));
    //post("added class %s %x", c->type->s_name, c->create);

    return c;
}

#if 0
void pdp_class_addmethod(t_pdp_class *c, t_pdp_symbol *name, t_pdp_attribute_method method,
			 t_pdp_list *in_spec, t_pdp_list *out_spec)
{
    t_pdp_attribute *attr = (t_pdp_attribute *)pdp_alloc(sizeof(t_pdp_attribute));
    attr->name = name;
    attr->method = method;
    attr->in_spec = in_spec;
    attr->out_spec = out_spec;
    pdp_list_add_pointer(c->attributes, attr);
    
}
#endif

/* the packet factory */
int pdp_factory_newpacket(t_pdp_symbol *type)
{
    t_pdp_class *c;
    t_pdp_atom *a = class_list->first;
    while(a){
	c = (t_pdp_class *)(a->w.w_pointer);
	if (c->type && pdp_type_description_match(type, c->type)){
	    //post("method %x, type %s", c->create, type->s_name);
	    return (c->create) ? (*c->create)(type) : -1;
	}
	a = a->next;
    }
    return -1;
}

#if 0
/* generic methods. actually a forth word operation on a stack.
   first item on stack must be a packet and it's type will
   determine the place to look for the operator */
int pdp_packet_op(t_pdp_symbol *operation, struct _pdp_list *stack)
{
    int packet = stack->first->w.w_packet;
    t_pdp *h = pdp_packet_header(packet);
    t_pdp_atom *i;
    t_pdp_attribute *attr;

    if (!(h && h->theclass)) goto exit;       

    PDP_POINTER_IN(h->theclass->attributes, i, attr){
	//post("attribute:%s", attr->name->s_name);
	if (attr->name == operation){
	    
	    /* FOUND -> exec (check arguments first ???) */
	    return attr->method (stack);
	}
	
    }
 exit:
    // method not found
    post ("WARNING: pdp_packet_op: packet %d from class %s has no operation %s", 
	  packet, (h && h->theclass) ? h->theclass->type->s_name : "UNKNOWN", operation->s_name);
    return 0;
}

#endif

static void
_pdp_pool_expand(void){
    int i;

    /* double the size */
    int new_pool_size = pdp_pool_size << 1;
    t_pdp **new_pool = (t_pdp **)malloc(new_pool_size * sizeof(t_pdp *));
    bzero(new_pool, new_pool_size * sizeof(t_pdp *));
    memcpy(new_pool, pdp_pool, pdp_pool_size * sizeof(t_pdp *));
    free(pdp_pool);
    pdp_pool = new_pool;
    pdp_pool_size = new_pool_size;

    D post("DEBUG: _pdp_pool_expand: resized pool to contain %d packets", pdp_pool_size);
}




/* private _pdp_packet methods */

/* packets can only be created and destroyed using these 2 methods */
/* it updates the mem usage and total packet count */

static void
_pdp_packet_dealloc(t_pdp *p)
{
    unsigned int size = p->size;
    if (p->theclass && p->theclass->cleanup)
	(*p->theclass->cleanup)(p);

    free (p);
    pdp_packet_mem_usage -= size;
    pdp_packet_count--;
    D post("DEBUG: _pdp_packet_new_dealloc: freed packet. pool contains %d packets", pdp_packet_count);
}

static t_pdp*
_pdp_packet_alloc(unsigned int datatype, unsigned int datasize)
{
    unsigned int totalsize = datasize + PDP_HEADER_SIZE;
    unsigned int align;
    t_pdp *p = 0;

    /* check if there is a memory usage limit */
    if (pdp_packet_max_mem_usage){
	/* if it would exceed the limit, fail */
	if (pdp_packet_mem_usage + totalsize > pdp_packet_max_mem_usage){
	    D post("DEBUG: _pdp_packet_new_alloc: memory usage limit exceeded");
	    return 0;
	}
    }
    p = (t_pdp *)malloc(totalsize);
    if (p){
	align = ((unsigned int)p) & (PDP_ALIGN - 1);
	if (align) post("WARNING: _pdp_packet_alloc: data misaligned by %x", align);
	memset(p, 0, PDP_HEADER_SIZE); //initialize header to 0
	p->type = datatype;
	p->size = totalsize;
	p->users = 1;
	pdp_packet_mem_usage += totalsize;
	pdp_packet_count++;
	D post("DEBUG: _pdp_packet_new_alloc: allocated new packet. pool contains %d packets, using %d bytes",
	       pdp_packet_count, pdp_packet_mem_usage);
    }

    return p;
}


void 
pdp_packet_destroy(void)
{
    int i = 0;
    /* dealloc all the data in object stack */
    post("DEBUG: pdp_packet_destroy: clearing object pool.");
    while ((i < pdp_pool_size) && (pdp_pool[i])) _pdp_packet_dealloc(pdp_pool[i++]);
}



/* try to find a packet based on main type and datasize */
static int
_pdp_packet_reuse_type_size(unsigned int datatype, unsigned int datasize)
{
    unsigned int totalsize = datasize + PDP_HEADER_SIZE;
    int i = 0;
    int return_packet = -1;
    t_pdp* p;

    for (i=0; i < pdp_pool_size; i++){
	p = pdp_pool[i];
	/* check if we can reuse this one if it is already allocated */
	if (p) {
	    /* search for unused packets of equal size & type */
	    if ((p->users == 0) && (p->size == totalsize) && (p->type == datatype)){
		D post("DEBUG: _pdp_packet_reuse_type_size: can reuse %d", i);
		
		/* if possible, a packet will be reused and reinitialized
		   i haven't found a use for this, so it's only used for discriminating
		   between pure and not-so-pure packets */ 
		if (p->theclass && p->theclass->reinit){
		    (*p->theclass->reinit)(p);
		}
		/* if no re-init method is found, the header will be reset to all 0
		   this ensures the header is in a predictable state */
		else {
		    memset(p, 0, PDP_HEADER_SIZE);
		    p->type = datatype;
		    p->size = totalsize;
		}

		p->users = 1;
		return_packet = i;
		goto exit;
	    }
	    else{
	      D post("DEBUG _pdp_packet_reuse_type_size: can't reuse %d, (%d users)", i, p->users);
	    }
	}
    }

    D post("DEBUG: _pdp_packet_reuse_type_size: no reusable packet found");

 exit:
    return return_packet;
}


/* create a new packet in an empty slot.
   if this fails, the garbage collector needs to be called */
static int 
_pdp_packet_create_in_empty_slot(unsigned int datatype, unsigned int datasize /*without header*/)
{
    unsigned int totalsize = datasize + PDP_HEADER_SIZE;
    int i = 0;
    int return_packet = -1;
    int out_of_mem = 0;
    t_pdp* p;


    /* no reusable packets found, try to find an empty slot */
    for (i=0; i < pdp_pool_size; i++){
	p = pdp_pool[i];
	if (!p) {
	    p = _pdp_packet_alloc(datatype, datasize);

	    if (!p) {
		D post("DEBUG: _pdp_packet_create_in_empty_slot: out of memory (malloc returned NULL)");
		return_packet = -1;
		goto exit;
	    }

	    pdp_pool[i] = p;
	    return_packet = i;
	    goto exit;
	}
    }

    /* if we got here the pool is full: resize the pool and try again */
    _pdp_pool_expand();
    return_packet = _pdp_packet_create_in_empty_slot(datatype, datasize);

 exit:
    return return_packet;

}

/* find an unused packet, free it and create a new packet.
   if this fails, something is seriously wrong */
static int 
_pdp_packet_create_in_unused_slot(unsigned int datatype, unsigned int datasize /*without header*/)
{
    unsigned int totalsize = datasize + PDP_HEADER_SIZE;
    int i = 0;
    int return_packet = -1;
    int out_of_mem = 0;
    t_pdp* p;

    D post("DEBUG: _pdp_packet_create_in_unused_slot: collecting garbage");
		
    /* search backwards */
    for (i=pdp_pool_size-1; i >= 0; i--){
	p = pdp_pool[i];
	if (p){
	    if (p->users == 0){
		_pdp_packet_dealloc(p);
		p = _pdp_packet_alloc(datatype, datasize);
		pdp_pool[i] = p;

		/* alloc succeeded, return */
		if (p) {
		    post("DEBUG _pdp_packet_create_in_unused_slot: garbage collect succesful");
		    return_packet = i;
		    goto exit;
		}

		/* alloc failed, continue collecting garbage */
		D post("DEBUG _pdp_packet_create_in_unused_slot: freed one packet, still out of memory (malloc returned NULL)");
		out_of_mem = 1;
	    
	    }
	}
    }

    /* if we got here, we ran out of memory */
    D post("DEBUG: _pdp_packet_create_in_unused_slot: out of memory after collecting garbage");
    return_packet = -1;

exit:
    return return_packet;
}

/* warning: for "not so pure" packets, this method will only return an initialized
   packet if it can reuse a privious one. that is, if it finds a reinit method
   in the packet. use the pdp_packet_new_<type> constructor if possible */

static int 
_pdp_packet_brandnew(unsigned int datatype, unsigned int datasize /*without header*/)
{
    int return_packet = -1;

    /* try to create a new packet in an empty slot */
    return_packet = _pdp_packet_create_in_empty_slot(datatype, datasize);
    if (return_packet != -1) goto exit;

    /* if we still don't have a packet, we need to call the garbage collector until we can allocate */
    return_packet = _pdp_packet_create_in_unused_slot(datatype, datasize);

 exit:
    return return_packet;
}


static int
_pdp_packet_new(unsigned int datatype, unsigned int datasize /*without header*/)
{
    int return_packet = -1;

    /* try to reuse a packet based on main type and datasize */
    return_packet = _pdp_packet_reuse_type_size(datatype, datasize);
    if (return_packet != -1) goto exit;

    /* create a brandnew packet */
    return_packet = _pdp_packet_brandnew(datatype, datasize);
    if (return_packet != -1) goto exit;

    post("WARNING: maximum packet memory usage limit (%d bytes) reached after garbage collect.", pdp_packet_max_mem_usage);
    post("WARNING: increase memory limit or decrease packet usage (i.e. pdp_loop, pdp_delay).");
    post("WARNING: pool contains %d packets, using %d bytes.", pdp_packet_count, pdp_packet_mem_usage);

 exit:
    return return_packet;
       
}



/* public pool operations: have to be thread safe so each entry point
   locks the mutex */

/* reuse an old packet based on high level description */
int
pdp_packet_reuse(t_pdp_symbol *description)
{
    int i;
    int return_packet = -1;
    t_pdp *p;

    /* LOCK */
    pthread_mutex_lock(&pdp_pool_mutex);

    for (i=0; i < pdp_pool_size; i++){
	p = pdp_pool[i];
	/* check if we can reuse this one if it is already allocated */
	if ((p) && (p->users == 0) && (p->desc == description)){
	    /* mark it as used */
	    p->users = 1;
	    return_packet = i;
	    goto gotit; 
	}
    }

 gotit:
    /* LOCK */
    pthread_mutex_unlock(&pdp_pool_mutex);

    return return_packet;


}

/* create a new packet, or reuse an old one based on main type and size */
int 
pdp_packet_new(unsigned int datatype, unsigned int datasize /*without header*/)
{
    int packet;
    /* LOCK */
    pthread_mutex_lock(&pdp_pool_mutex);

    packet = _pdp_packet_new(datatype, datasize);

    /* UNLOCK */
    pthread_mutex_unlock(&pdp_pool_mutex);
    return packet;
}


/* create a brand new packet, don't reuse an old one */
int 
pdp_packet_brandnew(unsigned int datatype, unsigned int datasize /*without header*/)
{
    int packet;
    /* LOCK */
    pthread_mutex_lock(&pdp_pool_mutex);

    packet = _pdp_packet_brandnew(datatype, datasize);

    /* UNLOCK */
    pthread_mutex_unlock(&pdp_pool_mutex);
    return packet;
}


/* this returns a copy of a packet for read only access. */
int
pdp_packet_copy_ro(int handle)
{
    int out_handle = -1;
    t_pdp* p;

    /* LOCK */
    pthread_mutex_lock(&pdp_pool_mutex);

    if ((handle >= 0) 
	&& (handle < pdp_pool_size) 
	&& (p = pdp_pool[handle])){

	/* it is an error to copy a packet without an owner */
	if (!p->users){
	    post("pdp_packet_copy_ro: ERROR: request to copy packet %d which has 0 users", handle);
	    out_handle = -1;
	}

	/* if it's a passing packet, reset the reference location
	   and turn it into a normal packet */
	else if (p->refloc){
	    *p->refloc = -1;
	    p->refloc = 0;
	    out_handle = handle;
	}
	/* if it's a normal packet, increase the number of users */
	else {
	    p->users++;
	    out_handle = handle;
	}
    }
    else out_handle = -1;

    //post("pdp_copy_ro: outhandle:%d", out_handle);

    /* UNLOCK */
    pthread_mutex_unlock(&pdp_pool_mutex);

    return out_handle;
}

/* copy a packet. if the packet is marked passing, it will be aquired.
   otherwize a new packet will be created with a copy of the contents. */

int
pdp_packet_copy_rw(int handle)
{
    int out_handle = -1;
    t_pdp* p;

    /* LOCK */
    pthread_mutex_lock(&pdp_pool_mutex);

    if ((handle >= 0) 
	&& (handle < pdp_pool_size) 
	&& (p = pdp_pool[handle])){
	/* if there are other users, copy the object otherwize return the same handle */

	/* it is an error to copy a packet without an owner */
	if (!p->users){
	    post("pdp_packet_copy_rw: ERROR: request to copy packet %d which has 0 users", handle);
	    out_handle = -1;
	}

	/* if it is a passing packet, remove the owner's reference to it */
	else if (p->refloc){
	    *p->refloc = -1;
	    p->refloc = 0;
	    out_handle = handle;
	}

	/* check if packet supports copy (for fanout) */
	else if(p->flags & PDP_FLAG_DONOTCOPY) out_handle = -1;


	/* copy the packet, since it already has 1 or more users */
	else{
	    int new_handle = _pdp_packet_new(p->type, p->size - PDP_HEADER_SIZE);
	    t_pdp* new_p = pdp_packet_header(new_handle);

	    /* check if valid */
	    if (!new_p) out_handle = -1;

	    else {
		/* if a copy constructor is found, it will be called */
		if (p->theclass && p->theclass->copy){
		    (*p->theclass->copy)(new_p, p);
		}
		/* if not, the entire packet will be copied, assuming a pure data packet */
		else {
		    memcpy(new_p, p, p->size);
		}
		new_p->users = 1;
		out_handle = new_handle;
	    }
	}

	//post("pdp_copy_rw: inhandle:%d outhandle:%d", handle, out_handle);

    }
    else out_handle = -1;

    /* UNLOCK */
    pthread_mutex_unlock(&pdp_pool_mutex);
    return out_handle;
}

/* create a new packet, copying the header data of another
   packet, without copying the data */
int
pdp_packet_clone_rw(int handle)
{
    int out_handle;
    t_pdp* p;

    /* LOCK */
    pthread_mutex_lock(&pdp_pool_mutex);

    if ((handle >= 0) 
	&& (handle < pdp_pool_size) 
	&& (p = pdp_pool[handle])){

	/* clone the packet header, don't copy the data */
	int new_handle = _pdp_packet_new(p->type, p->size - PDP_HEADER_SIZE);
	t_pdp* new_p = pdp_packet_header(new_handle);

	/* if a clone initializer is found, it will be called */
	if (p->theclass && p->theclass->clone){
	    (*p->theclass->clone)(new_p, p);
	}
	/* if not, just the header will be copied, assuming a pure data packet */
	else {
	    memcpy(new_p, p, PDP_HEADER_SIZE);
	}
	new_p->users = 1;   /* the new packet has 1 user */
	new_p->refloc = 0;  /* it is not a passing packet, even if the template was */
	out_handle = new_handle;
    }

    else out_handle = -1;

    /* UNLOCK */
    pthread_mutex_unlock(&pdp_pool_mutex);

    return out_handle;
}

void
_pdp_packet_mark_unused_nolock(int handle)
{
    t_pdp* p;

    if ((handle >= 0) && (handle < pdp_pool_size)){
      if (p = pdp_pool[handle]) {
	  /* mark_unused on a passing packet has no effect
	     this is to support automatic conversion for passing packets
	     so in order to delete a passing packet, it should be marked normal first */
	  if (p->refloc){
	      post("DEBUG: pdp_mark_unused called on a passing packet. ignored.");
	      return;
	  }

	  /* decrease the refcount */
	  if (p->users) {
	      p->users--;
	      //post("pdp_mark_unused: handle %d, users left %d", handle, p->users);
	  }
	  else {
	      post("pdp_mark_unused: ERROR: handle %d has zero users (duplicate pdp_mark_unused ?)", handle);
	}
      }
      else {
	  post("pdp_mark_unused: ERROR: invalid handle %d: no associated object", handle);
      }
    }
    
    else {
	/* -1 is the only invalid handle that doesn't trigger a warning */
	if (handle != -1) post("pdp_mark_unused: WARNING: invalid handle %d: out of bound", handle);
    }

}

/* mark a packet as unused, decreasing the reference count.
   if the reference count reaches zero, the packet is ready to be recycled
   by a new packet allocation. it is illegal to reference a packet with
   reference count == zero. if the reference count is not == 1, only readonly
   access is permitted. */

void
pdp_packet_mark_unused(int handle)
{
    /* LOCK */
    pthread_mutex_lock(&pdp_pool_mutex);

    _pdp_packet_mark_unused_nolock(handle);

    /* UNLOCK */
    pthread_mutex_unlock(&pdp_pool_mutex);
}

void
pdp_packet_mark_unused_atomic(int *handle)
{
    /* LOCK */
    pthread_mutex_lock(&pdp_pool_mutex);

    _pdp_packet_mark_unused_nolock(*handle);
    *handle = -1;

    /* UNLOCK */
    pthread_mutex_unlock(&pdp_pool_mutex);
}

/* delete a packet. this is more than mark_unused:
   it actually removes any reference. can be used for
   some packet types that have "expensive" resources. 
   usually this is up to the garbage collector to call. */

void
pdp_packet_delete(int handle)
{
    t_pdp *header = pdp_packet_header(handle);

    /* LOCK */
    pthread_mutex_lock(&pdp_pool_mutex);


    /* check if it's a valid handle */
    if ((handle >= 0) && (handle < pdp_pool_size) && pdp_pool[handle]){

	/* mark it unused */
	_pdp_packet_mark_unused_nolock(handle);

	/* if no users, dealloc */
	if (!header->users){
	    _pdp_packet_dealloc(header);
	    pdp_pool[handle] = NULL;
	}

	/* print a warning if failed */
	else{
	    post("WARNING: pdp_packet_delete: packet %d was not deleted. %d users remaining", 
		 handle, header->users);
	}
    }

    /* UNLOCK */
    pthread_mutex_unlock(&pdp_pool_mutex);

}


/* this turns any packet into a normal (non-passing) packet */

void
pdp_packet_unmark_passing(int packet)
{
    t_pdp* header = pdp_packet_header(packet);
    if (!header) return;

    /* LOCK */
    pthread_mutex_lock(&pdp_pool_mutex);

    header->refloc = 0;

    /* UNLOCK */
    pthread_mutex_unlock(&pdp_pool_mutex);

}


/* this marks a packet as passing. this means it changes
   owner on first copy_ro or copy_rw. the previous owner is
   notified by setting the handler to -1. */

void
pdp_packet_mark_passing(int *phandle)
{
    t_pdp* header = pdp_packet_header(*phandle);

    /* LOCK */
    pthread_mutex_lock(&pdp_pool_mutex);

    if (header){
	if (header->refloc){
	    post("pdp_packet_mark_passing: ERROR: duplicate mark_passing on packet %d", *phandle);
	}
	else if (1 != header->users){
	    post("pdp_packet_mark_passing: ERROR: packet %d is not exclusively owned by caller, it has %d users", *phandle, header->users);
	}
	else {
	    header->refloc = phandle;
	}
    }

    /* UNLOCK */
    pthread_mutex_unlock(&pdp_pool_mutex);

}


/* public data access methods */

t_pdp*
pdp_packet_header(int handle)
{
    if ((handle >= 0) && (handle < pdp_pool_size)) return pdp_pool[handle];
    else return 0;
}

void*
pdp_packet_subheader(int handle)
{
    t_pdp* header = pdp_packet_header(handle);
    if (!header) return 0;
    return (void *)(&header->info.raw);
}

void*
pdp_packet_data(int handle)
{
    t_pdp *h;
    if ((handle >= 0) && (handle < pdp_pool_size)) 
	{
	    h = pdp_pool[handle];
	    if (!h) return 0;
	    return (char *)(h) + PDP_HEADER_SIZE;
	}
    else return 0;
}


/* OBSOLETE: use packet description */
/* check if two packets are allocated and of the same type */
bool pdp_packet_compat(int packet0, int packet1)
{

    t_pdp *header0 = pdp_packet_header(packet0);
    t_pdp *header1 = pdp_packet_header(packet1);

    if (!(header1)){
	//post("pdp_type_compat: invalid header packet1");
	return 0;
    }
    if (!(header0)){
	//post("pdp_type_compat: invalid header packet 0");
	return 0;
    }
    if (header0->type != header1->type){
	//post("pdp_type_compat: types do not match");
	return 0;
    }

    return 1;
}

int pdp_packet_writable(int packet) /* returns true if packet is writable */
{
    t_pdp *h = pdp_packet_header(packet);
    if (!h) return 0;
    return (h->users == 1);
}

void pdp_packet_replace_with_writable(int *packet) /* replaces a packet with a writable copy */
{
    int new_p;
    if (!pdp_packet_writable(*packet)){
	new_p = pdp_packet_copy_rw(*packet);
	pdp_packet_mark_unused(*packet);
	*packet = new_p;
    }
	
}

/* pool stuff */

int
pdp_pool_collect_garbage(void)
{
    t_pdp *p;
    int i;
    int nbpackets = pdp_packet_count;

    /* LOCK */
    pthread_mutex_lock(&pdp_pool_mutex);

    for (i=0; i < pdp_pool_size; i++){
	p = pdp_pool[i];
	if(p && !p->users) {
	    _pdp_packet_dealloc(p);
	    pdp_pool[i] = 0;
	}
    }
    nbpackets -= pdp_packet_count;
    //post("pdp_pool_collect_garbage: deleted %d unused packets", nbpackets);

    /* UNLOCK */
    pthread_mutex_unlock(&pdp_pool_mutex);

    return nbpackets;
}

void
pdp_pool_set_max_mem_usage(int max)
{
    if (max < 0) max = 0;
    pdp_packet_max_mem_usage = max;
    
}






/* malloc wrapper that calls garbage collector */
void *pdp_alloc(int size)
{
    void *ptr = malloc(size);
    
    //post ("malloc called for %d bytes", size);

    if (ptr) return ptr;

    post ("malloc failed in a pdp module: running garbage collector.");

    pdp_pool_collect_garbage();
    return malloc(size);
}


void pdp_dealloc(void *stuff)
{
    free (stuff);
}

#ifdef __cplusplus
}
#endif
