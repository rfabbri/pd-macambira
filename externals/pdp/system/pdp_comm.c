/*
 *   Pure Data Packet system implementation.
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

/* this file contains misc communication methods */


#include "pdp.h"
#include "pdp_internals.h"
#include <stdio.h>

/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif


/************** packet management and communication convenience functions ************/

/* send a packet to an outlet */
void outlet_pdp(t_outlet *out, int packetid)
{
    t_atom atom[2];
    t_symbol *s = gensym("pdp");
    t_symbol *rro = gensym("register_ro");
    t_symbol *rrw = gensym("register_rw");
    t_symbol *proc = gensym("process");


    SETFLOAT(atom+1, (float)packetid);

    SETSYMBOL(atom+0, rro);
    outlet_anything(out, s, 2, atom);

    SETSYMBOL(atom+0, rrw);
    outlet_anything(out, s, 2, atom);

    /* this is not really necessary, it can be triggered by the rw message */
    SETSYMBOL(atom+0, proc);
    outlet_anything(out, s, 1, atom);

}


/* unregister a packet and send it to an outlet */
void
pdp_pass_if_valid(t_outlet *outlet, int *packet)
{
    if (-1 != *packet){
	pdp_packet_mark_unused(*packet);
	outlet_pdp(outlet, *packet);
	*packet = -1;
    }
}

void
pdp_replace_if_valid(int *dpacket, int *spacket)
{
    if (-1 != *spacket){
	pdp_packet_mark_unused(*dpacket);
	*dpacket = *spacket;
	*spacket = -1;
    }
    
}


int
pdp_packet_copy_ro_or_drop(int *dpacket, int spacket)
{
    int drop = 0;
    if (*dpacket == -1) *dpacket = pdp_packet_copy_ro(spacket);
    else {
	/* send a notification there is a dropped packet */
	pdp_control_notify_drop(spacket);
	drop = 1;
    }
    return drop;
}


int
pdp_packet_copy_rw_or_drop(int *dpacket, int spacket)
{
    int drop = 0;
    if (*dpacket == -1) *dpacket = pdp_packet_copy_rw(spacket);
    else {
	/* send a notification there is a dropped packet */
	pdp_control_notify_drop(spacket);
	drop = 1;
    }
    return drop;
}






#ifdef __cplusplus
}
#endif
