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

/* this file contains misc communication (packet) methods for pd */


#include "pdp.h"
#include "pdp_internals.h"
#include <stdio.h>

/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif

t_symbol *_pdp_sym_pdp;
t_symbol *_pdp_sym_rro;
t_symbol *_pdp_sym_rrw;
t_symbol *_pdp_sym_prc;
t_symbol *_pdp_sym_dpd;
t_symbol *_pdp_sym_ins;
t_symbol *_pdp_sym_acc;

t_symbol *pdp_sym_pdp() {return _pdp_sym_pdp;}
t_symbol *pdp_sym_rro() {return _pdp_sym_rro;}
t_symbol *pdp_sym_rrw() {return _pdp_sym_rrw;}
t_symbol *pdp_sym_prc() {return _pdp_sym_prc;}
t_symbol *pdp_sym_dpd() {return _pdp_sym_dpd;}
t_symbol *pdp_sym_ins() {return _pdp_sym_ins;}
t_symbol *pdp_sym_acc() {return _pdp_sym_acc;}

/************** packet management and communication convenience functions ************/

/* send a packet to an outlet */
void outlet_pdp(t_outlet *out, int packetid)
{
    t_atom atom[2];

    SETFLOAT(atom+1, (float)packetid);

    SETSYMBOL(atom+0, pdp_sym_rro());
    outlet_anything(out, pdp_sym_pdp(), 2, atom);

    SETSYMBOL(atom+0, pdp_sym_rrw());
    outlet_anything(out, pdp_sym_pdp(), 2, atom);

    SETSYMBOL(atom+0, pdp_sym_prc());
    outlet_anything(out, pdp_sym_pdp(), 2, atom);

}

/* send an accumulation packet to an outlet */
void outlet_dpd(t_outlet *out, int packetid)
{
    t_atom atom[2];

    SETFLOAT(atom+1, (float)packetid);

    SETSYMBOL(atom+0, pdp_sym_ins());
    outlet_anything(out, pdp_sym_dpd(), 2, atom);

    SETSYMBOL(atom+0, pdp_sym_acc());
    outlet_anything(out, pdp_sym_dpd(), 2, atom);

}

/* unregister a packet and send it to an outlet */
void

pdp_packet_pass_if_valid(t_outlet *outlet, int *packet_ptr)
{
    t_pdp *header = pdp_packet_header(*packet_ptr);
    //if (-1 != *packet){
    if (header){
	/* if packet is exclusively owned, mark as passing */
	if (1 == header->users)  pdp_packet_mark_passing(packet_ptr);

	/* send */
	outlet_pdp(outlet, *packet_ptr);

	/* if the packet is still here (was passing and not registered, 
	   or it was a shared copy): unregister it */
	if (-1 != *packet_ptr){
	    pdp_packet_unmark_passing(*packet_ptr);
	    pdp_packet_mark_unused(*packet_ptr);
	    *packet_ptr = -1;
	}
    }
}

void
pdp_packet_replace_if_valid(int *dpacket, int *spacket)
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

int
pdp_packet_convert_ro_or_drop(int *dpacket, int spacket, t_pdp_symbol *template)
{
    int drop = 0;

    if (!template) return pdp_packet_copy_ro_or_drop(dpacket, spacket);

    if (*dpacket == -1) *dpacket = pdp_packet_convert_ro(spacket, template);
    else {
	/* send a notification there is a dropped packet */
	pdp_control_notify_drop(spacket);
	drop = 1;
    }
    return drop;
}


int
pdp_packet_convert_rw_or_drop(int *dpacket, int spacket, t_pdp_symbol *template)
{
    int drop = 0;

    if (!template) return pdp_packet_copy_rw_or_drop(dpacket, spacket);

    if (*dpacket == -1) *dpacket = pdp_packet_convert_rw(spacket, template);
    else {
	/* send a notification there is a dropped packet */
	pdp_control_notify_drop(spacket);
	drop = 1;
    }
    return drop;
}


void
pdp_sym_setup(void)
{
    _pdp_sym_pdp = gensym("pdp");
    _pdp_sym_rro = gensym("register_ro");
    _pdp_sym_rrw = gensym("register_rw");
    _pdp_sym_prc = gensym("process");
    _pdp_sym_dpd = gensym("dpd");
    _pdp_sym_ins = gensym("inspect");
    _pdp_sym_acc = gensym("accumulate");
}



#ifdef __cplusplus
}
#endif
