/*
 *   Pure Data Packet system implementation. : methods for the 16 bit image packet
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

/* image packet method implementation */


#include "pdp.h"
#include "pdp_forth.h"



// (p0, p1) -> (p0 (operator) p1)
t_pdp_word_error pdp_word_image_pp(t_pdp_stack *s, void *process_routine)
{
    int chanmask = -1;
    int packet1 = -1;
    int packet2 = -1;

    /* handle stack underflow */
    if (s->elements < 2) return e_underflow;

    /* get the packets from the stack */
    pdp_stack_pop_packet(s, &packet1);
    pdp_stack_pop_packet(s, &packet2);

    /* ensure the destination packet is writable */
    pdp_packet_replace_with_writable(&packet1);

    /* process */
    pdp_imageproc_dispatch_2buf(process_routine, 0, chanmask, packet1, packet2);

    /* push result on stack */
    pdp_stack_push_packet(s, packet1);

    /* release the second packet */
    pdp_packet_mark_unused(packet2);

    return e_ok;
}

t_pdp_word_error pdp_word_image_add(t_pdp_stack *s) {return pdp_word_image_pp(s, pdp_imageproc_add_process);}
t_pdp_word_error pdp_word_image_mul(t_pdp_stack *s) {return pdp_word_image_pp(s, pdp_imageproc_mul_process);}


/* enter a (float packet packet) -> (packet)  routine */
#define ENTER_FPP_P(f, p0, p1)			\
void *x;					\
int p0;						\
int p1;						\
float f;					\
if (s->elements < (3)) return e_underflow;	\
pdp_stack_pop_float(s, &(f));			\
pdp_stack_pop_packet(s, &(p0));			\
pdp_stack_pop_packet(s, &(p1));			\
pdp_packet_replace_with_writable(&(p0));

/* leave a (float packet packet) -> (packet) routine */
#define LEAVE_FPP_P(p0, p1)			\
pdp_stack_push_packet(s, p0);			\
pdp_packet_mark_unused(p1);			\
return e_ok;


// (f, p0, p1) -> ((1-f)*p0 + f*p1)
t_pdp_word_error pdp_word_image_mix(t_pdp_stack *s)
{
    ENTER_FPP_P(mix, p0, p1);

    /* process */
    x = pdp_imageproc_mix_new();
    pdp_imageproc_mix_setleftgain(x, 1.0f - mix);
    pdp_imageproc_mix_setrightgain(x, mix);
    pdp_imageproc_dispatch_2buf(&pdp_imageproc_mix_process, x, -1, p0, p1);
    pdp_imageproc_mix_delete(x);

    LEAVE_FPP_P(p0, p1); 
}

// (f, p0) -> (f*p0)
t_pdp_word_error pdp_word_image_gain(t_pdp_stack *s)
{
    void *gaindata;
    int chanmask = -1;
    int packet = -1;
    float gain = 0.5f;

    /* handle stack underflow */
    if (s->elements < 2) return e_underflow;

    /* get the packets from the stack */
    pdp_stack_pop_float(s, &gain);
    pdp_stack_pop_packet(s, &packet);

    /* ensure the destination packet is writable */
    pdp_packet_replace_with_writable(&packet);

    /* process */
    gaindata = pdp_imageproc_gain_new();
    pdp_imageproc_gain_setgain(gaindata, gain);
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_gain_process, gaindata, chanmask, packet);
    pdp_imageproc_mix_delete(gaindata);

    /* push result on stack */
    pdp_stack_push_packet(s, packet);

    return e_ok;

}
// (f, p0) -> (f*p0)
t_pdp_word_error pdp_word_image_chanmask(t_pdp_stack *s)
{
    unsigned int chanmask = 0;
    int packet = -1;

    /* handle stack underflow */
    if (s->elements < 2) return e_underflow;

    /* get the packets from the stack */
    pdp_stack_pop_int(s, &chanmask);
    pdp_stack_pop_packet(s, &packet);

    /* ensure the destination packet is writable */
    pdp_packet_replace_with_writable(&packet);

    /* set channel mask */
    pdp_packet_image_set_chanmask(packet, chanmask);

    /* push result on stack */
    pdp_stack_push_packet(s, packet);

    return e_ok;

}

static void _add_image_primitive(char *name, int in, int out, int type_index, void *w)
{
    pdp_forthdict_add_primitive(pdp_gensym(name), w, in, out, type_index, pdp_gensym("image/*/*"));
}

	    
void pdp_image_words_setup(t_pdp_class *c)
{
    /* add image specific words */
    _add_image_primitive ("add", 2, 1, 0, pdp_word_image_add);
    _add_image_primitive ("mix", 3, 1, 1, pdp_word_image_mix);
    _add_image_primitive ("scale", 2, 1, 1, pdp_word_image_gain);
    _add_image_primitive ("mul", 2, 1, 0, pdp_word_image_mul);
    _add_image_primitive ("chanmask", 2, 1, 1, pdp_word_image_chanmask);

    /* add some type oblivious words */
    pdp_forthdict_compile_word(pdp_gensym("add3"),    "(add dup add)",   2, 1, -1, 0);
    pdp_forthdict_compile_word(pdp_gensym("mixtest"), "(0.8 2 mov mix)", 2, 1, -1, 0);


    /* define a processor:

    def = pdp_forth_compile_def("(inlist, outlist, setup, process)");
    */
}
