
/*
 *   Pure Data Packet header file. Scalar type definitions.
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

/* some typedefs and utility classes */

#ifndef PDP_TYPES_H
#define PDP_TYPES_H

typedef signed char        s8;
typedef signed short       s16;
typedef signed long        s32;
typedef signed long long   s64;

typedef unsigned char      u8;
typedef unsigned short     u16;
typedef unsigned long      u32;
typedef unsigned long long u64;


#ifndef __cplusplus
typedef int bool;
#define true 1;
#define false 0;
#endif


typedef struct _pdp t_pdp;
typedef void (*t_pdp_packet_method1)(t_pdp *);              /* dst */
typedef void (*t_pdp_packet_method2)(t_pdp *, t_pdp *);     /* dst, src */


/*
  The pdp symbol type manages the pdp name space. It maps
  gives a symbol to something in a certain name space:

  * packet classes 
  * forth words
  * processor instances
  * type description lists (for accelerating type matching)

  symbols have an infinite lifespan, so this is also true
  for things attached to it.

*/

/* the pdp symbol type */
typedef struct _pdp_symbol
{
    /* the symbol name */
    char *s_name;

    /* the items this symbol is associated to in different namespaces */
    struct _pdp_forthword_spec *s_word_spec; // a forth word
    struct _pdp_list *s_processor;           // an atom processor object
    struct _pdp_class *s_class;              // packet class
    struct _pdp_list *s_type;                // a type description

    struct _pdp_symbol *s_next;

} t_pdp_symbol;

static inline void _pdp_symbol_clear_namespaces(t_pdp_symbol *s)
{
    s->s_word_spec = 0;
    s->s_processor = 0;
    s->s_class = 0;
    s->s_type = 0;
}



/* generic packet subheader */
//typedef unsigned char t_raw[PDP_SUBHEADER_SIZE];
typedef unsigned int t_raw;


#endif
