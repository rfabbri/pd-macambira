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

#ifndef PDP_COMM_H
#define PDP_COMM_H


/* all symbols are C style */
#ifdef __cplusplus
extern "C"
{
#endif

/* pdp symbols */
t_symbol *pdp_sym_pdp(void);
t_symbol *pdp_sym_rro(void);
t_symbol *pdp_sym_rrw(void);
t_symbol *pdp_sym_prc(void);
t_symbol *pdp_sym_dpd(void);
t_symbol *pdp_sym_ins(void);
t_symbol *pdp_sym_acc(void);


/* utility methods */



/* if packet is valid, mark it unused and send it to an outlet */
void pdp_packet_pass_if_valid(t_outlet *outlet, int *packet);

/* if source packet is valid, release dest packet and move src->dest */
void pdp_packet_replace_if_valid(int *dpacket, int *spacket);

/* copy_ro if dest packet if invalid, else drop source 
   (don't copy) + send drop notif to pdp system 
   returns 1 if dropped, 0 if copied */
int pdp_packet_copy_ro_or_drop(int *dpacket, int spacket);
int pdp_packet_convert_ro_or_drop(int *dpacket, int spacket, t_pdp_symbol *type_template);

/* copy_rw if dest packit is invalid, else drop source 
   (don't copy) + send drop notif to pdp system 
   returns 1 if dropped, zero if copied */
int pdp_packet_copy_rw_or_drop(int *dpacket, int spacket);
int pdp_packet_convert_rw_or_drop(int *dpacket, int spacket, t_pdp_symbol *type_template);




#ifdef __cplusplus
}
#endif



#endif
