/*
 *   Pure Data Packet header file.
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



#ifndef PDP_H
#define PDP_H

/* header and subheader size in bytes */
#define PDP_HEADER_SIZE 256

#include <string.h>
#include <stdlib.h>

/* some typedefs */
#include "pdp_types.h"

/* PDP_IMAGE COMPONENTS */

/* header and methods for the built in image packet type */
#include "pdp_image.h"

/* low level image processing and high level dispatching routines */
#include "pdp_imageproc.h"

/* low level image conversion routines */
#include "pdp_llconv.h"

/* low level image resampling routines */
#include "pdp_resample.h"



/* PDP_BITMAP COMPONENTS */

/* header and methods for the built in bitmap packet type */
#include "pdp_bitmap.h"


/* PDP_MATRIX COMPONENTS */
#include "pdp_matrix.h"


// packet class
#include "pdp_list.h"

typedef struct _pdp_list* (*t_pdp_attribute_method)(int, struct _pdp_list*);

/* packet class attribute (method) */
typedef struct _pdp_attribute
{
    t_pdp_symbol *name;
    t_pdp_attribute_method method;

    /* problem: do we support argument type checking ??
       this seems to be better solved in a "spec doc" or a central place
       where "reserved" methods can be defined. */

    /* if null this means the input or output list can be anything */
    struct _pdp_list *in_spec;      // template for the input list (including default arguments)
    struct _pdp_list *out_spec;     // template for the output list
} t_pdp_attribute;


/* packet class header */
typedef struct _pdp_class
{
    /* packet manips: non-pure data packets (using external resources) must define these */
    t_pdp_packet_method1 reinit;     /* reuse method for pdp_packet_new() */
    t_pdp_packet_method2 clone;      /* init from template for pdp_packet_clone_rw() */
    t_pdp_packet_method2 copy;       /* init & copy from template for pdp_packet_copy_rw() */
    t_pdp_packet_method1 cleanup;    /* free packet's resources (to be used by the garbage collector) */

    t_pdp_symbol *type;              /* type template for packet class */
    struct _pdp_list *attributes;    /* list of attributes (packet methods) */
}t_pdp_class;


/* packet object header */
struct _pdp
{
    /* meta info */
    unsigned int type;             /* main datatype of this object */
    t_pdp_symbol *desc;            /* high level type description (sort of a mime type) */
    unsigned int size;             /* datasize including header */
    unsigned int flags;            /* packet flags */

    /* reference count */
    unsigned int users;            /* nb users of this object, readonly if > 1 */
    unsigned int *refloc;          /* location of reference to packet for passing packets */

    /* class object */
    t_pdp_class *pclass;            /* if zero, the packet is a pure packet (just data, no member functions) */

    /* class */
    //struct _pdp_class *class;    /* the packet class */

    u32 pad[9];                    /* reserved to provide binary compatibility for future extensions */

    union                          /* each packet type has a unique subheader */
    {
	t_raw    raw;              /* raw subheader (for extensions unkown to pdp core system) */
	t_image  image;            /* (nonstandard internal) 16 bit signed planar bitmap image format */
	t_bitmap bitmap;           /* (standard) bitmap image (fourcc coded) */
	//t_ca     ca;             /* cellular automaton state data */
	//t_ascii  ascii;          /* ascii packet */
    } info;

};


/* pdp data packet type id */
#define PDP_IMAGE                 1  /* 16bit signed planar scanline encoded image packet */
//RESERVED: #define PDP_CA        2  /* 1bit toroidial shifted scanline encoded cellular automaton */
//RESERVED: #define PDP_ASCII     3  /* ascii packet */
//RESERVED: #define PDP_TEXTURE   4  /* opengl texture object */
//RESERVED: #define PDP_3DCONTEXT 5  /* opengl render context */
#define PDP_BITMAP                6  /* 8bit image packet (fourcc coded??) */
//RESERVED: #define PDP_MATRIX    7  /* floating point/double matrix/vector packet (from gsl) */

/* PACKET FLAGS */

#define PDP_FLAG_DONOTCOPY (1<<0)   /* don't copy the packet on register_rw, instead return an invalid packet */


/* PDP SYSTEM COMPONENTS */

/* packet class and pool manager */
#include "pdp_packet.h"

/* processing queue manager */
#include "pdp_queue.h"

/* several communication helper methods (pd specific) */
#include "pdp_comm.h"

/* type handling subsystem */
#include "pdp_type.h"

/* dpd command stuff */
#include "pdp_dpd_command.h"


/* BACKWARDS COMPAT STUFF */
#include "pdp_compat.h"




#endif 

/*

   PDP CORE API OVERVIEW

   pdp_packet_* : packet methods, first argument is packet id

      new:                construct a raw packet (depreciated)
      new_*:              construct packet of specific type/subtype/...
      mark_unused:        release
      mark_passing:       conditional release (release on first copy ro/rw)
      copy_ro:            readonly (shared) copy
      copy_rw:            private copy
      clone_rw:           private copy (copies only meta data, not the content)
      header:             get the raw header (t_pdp *)
      data:               get the raw data (void *)
      pass_if_valid:      send a packet to pd outlet, if it is valid
      replace_if_valid    delete packet and replace with new one, if new is valid
      copy_ro_or_drop:    copy readonly, or don't copy if dest slot is full + send drop notify
      copy_rw_or_drop:    same, but private copy
      get_description:    retrieve type info
      convert_ro:         same as copy_ro, but with an automatic conversion matching a type template
      convert_rw:         same as convert_ro, but producing a private copy

   pdp_pool_* : packet pool methods

      collect_garbage:    manually free all unused resources in packet pool

   pdp_queue_* : processing queue methods

      add:                add a process method + callback
      finish:             wait until a specific task is done
      wait:               wait until processing queue is done

   pdp_control_* : central pdp control hub methods

      notify_drop:        notify that a packet has been dropped

   pdp_type_* : packet type mediator methods

      description_match:   check if two type templates match
      register_conversion: register a type conversion program



   NOTE: it is advised to derive your module from the pdp base class defined in pdp_base.h
         instead of communicating directly with the pdp core

*/
