
/*
 *   Pure Data Packet internal header file.
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


/* this file contains prototypes for "private" pdp methods.
   DON'T CALL THESE FROM OUTSIDE OF PDP! unless you really
   know what you are doing.
 */


/* pdp system code is not very well organized, this is an
   attempt to do better. */


#ifndef PDP_INTERNALS_H
#define PDP_INTERNALS_H

#ifdef __cplusplus
extern "C" 
{
#endif

//#include "pdp.h"
//#include <pthread.h>
//#include <unistd.h>
//#include <stdio.h>

/* set/unset main pdp thread usage */
void pdp_queue_use_thread(int t);

/* create a new packet, reuse if possible.
   ONLY USE THIS IN A TYPE SPECIFIC CONSTRUCTOR! */
int pdp_packet_new(unsigned int datatype, unsigned int datasize); 


/* send a packet to an outlet: it is only legal to call this on a "passing packet"
   or a "read only packet".
   this means it is illegal to change a packet after you have passed it to others,
   since this would mess up all read only references to the packet.
*/

/* this seems like a nice place to hide a comment on the notion of read/write in pdp
   which packets are writable? all packets with exactly 1 user. this includes all packets 
   aquired with pdp_packet_*_rw or a constructor, and all packets that are not registered
   after being sent out by outlet_pdp.
   which packets are readable? all packets */

void outlet_pdp(t_outlet *out, int packetid);

/* send an accumulation (context) packet to an outlet. this is for usage in the dpd
   base class. */
void outlet_dpd(t_outlet *out, int packetid);



#ifdef __cplusplus
}
#endif


#endif 
