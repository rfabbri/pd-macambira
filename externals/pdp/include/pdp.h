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


/* header size in bytes */
#define PDP_HEADER_SIZE 64

/* subheader size in bytes */
#define PDP_SUBHEADER_SIZE 48

#include <string.h>
#include <stdlib.h>
#include "m_pd.h"

#include "pdp_mmx.h"
#include "pdp_imageproc.h"
#include "pdp_types.h"

/* hope this won't conflict with other types */
#ifndef __cplusplus
typedef int bool;
#define true 1;
#define false 0;
#endif

/* remark: planar processing ensures (vector assembler) code reusability
   for grayscale / rgb(a) / yuv processing. so it's best
   to keep away from interleaved formats: deinterleaving
   and interleaving can be done in the source/sink modules
   (it will probably be very hard to eliminate extra copying
   anyway, and this enables the possibility to ensure alignment
   and correct (i.e. multiple of 8 or 16) image dimensions) 

   all image data is short int (16 bit)

*/




/* image data packet */   
typedef struct
{
    unsigned int encoding;  /* image encoding (data format ) */
    unsigned int width;     /* image width in pixels */
    unsigned int height;    /* image height in pixels */
    unsigned int channels;  /* number of colour planes if PDP_IMAGE_MCHP */
} t_image;


/* image encodings */
#define PDP_IMAGE_YV12   1  /* 24bbp: 16 bit Y plane followed by 16 bit 2x2 subsampled V and U planes.*/
#define PDP_IMAGE_GREY   2  /* 16bbp: 16 bit Y plane */
#define PDP_IMAGE_RGBP   3  /* 48bpp: 16 bit planar RGB */
#define PDP_IMAGE_MCHP   4  /* generic 16bit multi channel planar */

/*
PDP_IMAGE_GREY = PDP_IMAGE_MCHP, channels = 1
PDP_IMAGE_RGBP = PDP_IMAGE_MCHP, channels = 3

remark: only 1 and 2 are implemented at this moment

*/


/* generic packet subheader */
//typedef unsigned char t_raw[PDP_SUBHEADER_SIZE];
typedef unsigned int t_raw;

/* general pdp header struct */
typedef struct
{
    unsigned int type;      /* datatype of this object */
    unsigned int size;      /* datasize including header */
    unsigned int users;     /* nb users of this object, readonly if > 1 */
    unsigned int __pad__;
    union
    {
	t_raw    raw;       /* raw subheader (for extensions unkown to pdp core system) */
	t_image  image;     /* bitmap image */
	//t_ca     ca;      /* cellular automaton state data */
    } info;

} t_pdp;

/* pdp data packet types */
#define PDP_IMAGE            1  /* 16bit signed planar scanline encoded image packet */
//RESERVED: #define PDP_CA   2  /* 1bit toroidial shifted scanline encoded cellular automaton */




/* pdp specific constants */
#define PDP_ALIGN 8


/* this needs to be able to grow dynamically, think about it later */
#define PDP_OBJECT_ARRAY_SIZE 1024


/* all symbols are C-style */
#ifdef __cplusplus
extern "C"
{
#endif

    //extern t_pdp* pdp_stack[];
    //extern t_symbol* pdp_sym_register_rw;
    //extern t_symbol* pdp_sym_register_ro;
    //extern t_symbol* pdp_sym_process;

/* setup methods */

void pdp_init(void);
void pdp_destroy(void);


/* object manips */

extern int pdp_packet_new(unsigned int datatype, unsigned int datasize); /* create a new packet */
extern t_pdp* pdp_packet_header(int handle); /* get packet header */
extern void*  pdp_packet_data(int handle); /* get packet raw data */
extern int pdp_packet_copy_ro(int handle); /* get a read only copy */
extern int pdp_packet_copy_rw(int handle); /* get a read/write copy */
extern int pdp_packet_clone_rw(int handle); /* get an empty read/write packet of the same type (only copy header) */
extern void pdp_packet_mark_unused(int handle); /* indicate that you're done with the packet */

/* 

If an owner unregisters a packet, he can still pass it along to clients. More
specificly this is the desired behaviour. It is a simple way to have in place 
data processing (if there is only one client) and garbage collection. The first 
register call revives the object.

WARNING: it is an error to call pdp_packet_new inside a pdp packet handler BEFORE 
a packet is registered.

packet id -1 is the id of an invalid packet. it is not an error to unregister it.
packet id -2 can be used as a bogus id. it is an error to unregister it though.

*/


/* processor queue methods, callable from main pd thread */
/* warning: only pdp_packet_header and pdp_packet_data are legal inside process routine!! */

/* add a method to the processing queue */
void  pdp_queue_add(void *owner, void *process, void *callback, int *queue_id);

/* halt main tread until processing is done */
void pdp_queue_wait(void);

/* halt main tread until processing is done and remove 
   callback from queue(for destructors) */
void pdp_queue_finish(int queue_id);


/* misc signals to pdp */
void pdp_control_notify_drop(int packet);


/* helper methods */

/* send a packet to an outlet */
void outlet_pdp(t_outlet *out, int packetid);


/* if packet is valid, mark it unused and send it to an outlet */
void pdp_pass_if_valid(t_outlet *outlet, int *packet);

/* if source packet is valid, release dest packet and move src->dest */
void pdp_replace_if_valid(int *dpacket, int *spacket);

/* copy_ro if dest packet if invalid, else drop source 
   (don't copy) + send drop notif to pdp system 
   returns 1 if dropped, 0 if copied */
int pdp_packet_copy_ro_or_drop(int *dpacket, int spacket);

/* copy_rw if dest packit is invalid, else drop source 
   (don't copy) + send drop notif to pdp system 
   returns 1 if dropped, zero if copied */
int pdp_packet_copy_rw_or_drop(int *dpacket, int spacket);


/* check if packets are compatible */
int pdp_type_compat(int packet0, int packet1);
int pdp_type_compat_image(int packet0, int packet1);

int pdp_type_isvalid_image(int packet);


/* short cuts to create specific packets */
int pdp_packet_new_image_yv12(u32 width, u32 height);
int pdp_packet_new_image_grey(u32 width, u32 height);


#ifdef __cplusplus
}
#endif

#endif 
