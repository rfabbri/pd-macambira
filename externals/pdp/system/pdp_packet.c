/*
 *   Pure Data Packet system implementation: 
 *   code for allocation/deallocation/copying/...
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

/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif

/* this needs to be able to grow dynamically, think about it later */
#define PDP_OBJECT_ARRAY_SIZE 1024
static t_pdp* pdp_stack[PDP_OBJECT_ARRAY_SIZE];


/* some global vars */
static t_symbol* pdp_sym_register_rw;
static t_symbol* pdp_sym_register_ro;
static t_symbol* pdp_sym_process;


/* setup methods */

void 
pdp_packet_setup(void)
{
    bzero(pdp_stack, PDP_OBJECT_ARRAY_SIZE * sizeof(t_pdp *));
    pdp_sym_register_rw = gensym("register_rw");
    pdp_sym_register_ro = gensym("register_ro");
    pdp_sym_process = gensym("process");
}

void 
pdp_packet_destroy(void)
{
    int i = 0;
    /* dealloc all the data in object stack */
    while ((i < PDP_OBJECT_ARRAY_SIZE) && (pdp_stack[i])) free(pdp_stack[i++]);
}


/* private pdp_mem methods */


/* public object manips */


/* alloc method: alloc time is linear in the number of used packets */
/* think about a better (tree) method when this number grows large */
int 
pdp_packet_new(unsigned int datatype, unsigned int datasize /*without header*/)
{
    unsigned int totalsize = datasize + PDP_HEADER_SIZE;
    int i = 0;
    unsigned int align;
    t_pdp* p;
    for (i=0; i < PDP_OBJECT_ARRAY_SIZE; i++){
	p = pdp_stack[i];
	/* check if we can reuse this one if it is already allocated */
	if (p) {
	    /* remark: if p->size >= totalsize we can give away the packet */
	    /* but that would lead to unefficient use if we have a lot of packets */
	    /* of different sizes */
	    if ((p->users == 0) && (p->size == totalsize)){
	      //post("pdp_new_object: can reuse %d", i);
		memset(p, 0, PDP_HEADER_SIZE); //initialize header to 0
		p->type = datatype;
		p->size = totalsize;
		p->users = 1;
		return i;
	    }
	    else{
	      //post("pdp_new_object: can't reuse %d, (%d users)", i, p->users);
	      //post("size:%d, newsize:%d, type:%d, newtype:%d", p->size, totalsize, p->type, datatype);
	    }
	}
	/* allocate new one */
	else {
	    p = (t_pdp *)malloc(totalsize);
	    align = ((unsigned int)p) & (PDP_ALIGN - 1);
	    if (align) post("pdp_new_object: warning data misaligned by %x", align);
	    pdp_stack[i] = p;
	    memset(p, 0, PDP_HEADER_SIZE); //initialize header to 0
	    p->type = datatype;
	    p->size = totalsize;
	    p->users = 1;
	    //post("pdp_new_object: allocating new (%d)", i);
	    return i;
	}
    }
    post("pdp_new_object: WARNING: out of memory");

    return -1;

}


t_pdp*
pdp_packet_header(int handle)
{
    if ((handle >= 0) && (handle < PDP_OBJECT_ARRAY_SIZE)) return pdp_stack[handle];
    else return 0;
}

void*
pdp_packet_data(int handle)
{
    if ((handle >= 0) && (handle < PDP_OBJECT_ARRAY_SIZE)) 
	return (char *)(pdp_stack[handle]) + PDP_HEADER_SIZE;
    else return 0;
}



int
pdp_packet_copy_ro(int handle)
{
    int out_handle;

    t_pdp* p;
    if ((handle >= 0) 
	&& (handle < PDP_OBJECT_ARRAY_SIZE) 
	&& (p = pdp_stack[handle])){
	/* increase the number of users and return */
	p->users++;
	out_handle = handle;
    }
    else out_handle = -1;

    //post("pdp_copy_ro: outhandle:%d", out_handle);

    return out_handle;
}

int
pdp_packet_copy_rw(int handle)
{
    int out_handle;

    t_pdp* p;
    if ((handle >= 0) 
	&& (handle < PDP_OBJECT_ARRAY_SIZE) 
	&& (p = pdp_stack[handle])){
	/* if there are other users, copy the object otherwize return the same handle */
	if (p->users){
	    int new_handle = pdp_packet_new(p->type, p->size - PDP_HEADER_SIZE);
	    t_pdp* new_p = pdp_packet_header(new_handle);
	    memcpy(new_p, p, p->size);
	    new_p->users = 1;
	    out_handle = new_handle;
	}
	else {
	    p->users++;
	    out_handle = handle;
	}
	//post("pdp_copy_rw: inhandle:%d outhandle:%d", handle, out_handle);

    }
    else out_handle = -1;

    return out_handle;
}

int
pdp_packet_clone_rw(int handle)
{
    int out_handle;

    t_pdp* p;
    if ((handle >= 0) 
	&& (handle < PDP_OBJECT_ARRAY_SIZE) 
	&& (p = pdp_stack[handle])){

	/* clone the packet header, don't copy the data */
	int new_handle = pdp_packet_new(p->type, p->size - PDP_HEADER_SIZE);
	t_pdp* new_p = pdp_packet_header(new_handle);
	memcpy(new_p, p, PDP_HEADER_SIZE);
	new_p->users = 1;
	out_handle = new_handle;
    }

    else out_handle = -1;

    return out_handle;
}

void
pdp_packet_mark_unused(int handle)
{
    t_pdp* p;
    if ((handle >= 0) && (handle < PDP_OBJECT_ARRAY_SIZE)){
      if (p = pdp_stack[handle]) {
	if (p->users) {
	  p->users--;
	  //post("pdp_mark_unused: handle %d, users left %d", handle, p->users);
	}
	else {
	  post("pdp_mark_unused: WARNING: handle %d has zero users (duplicate pdp_mark_unused ?)", handle);
	}
      }
      else {
	post("pdp_mark_unused: WARNING: invalid handle %d: no associated object", handle);
      }
    }
    
    else {
      /* -1 is the only invalid handle that doesn't trigger a warning */
      if (handle != -1) post("pdp_mark_unused: WARNING: invalid handle %d: out of bound", handle);
    }


}

/* remark. if an owner frees a rw copy, he can still pass it along to clients.
the first copy instruction revives the object. maybe free should not be called free but unregister.
as long as no new_object method is called, or no copy on another object is performed,
the "undead" copy can be revived. this smells a bit, i know...*/




#ifdef __cplusplus
}
#endif
