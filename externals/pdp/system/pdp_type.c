/*
 *   Pure Data Packet system implementation. : code for handling different packet types
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


/****************** packet type checking methods ********************/


/* check if two packets are allocated and of the same type */
int pdp_type_compat(int packet0, int packet1)
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

/* check if two image packets are allocated and of the same type */
int pdp_type_compat_image(int packet0, int packet1)
{
    t_pdp *header0 = pdp_packet_header(packet0);
    t_pdp *header1 = pdp_packet_header(packet1);


    if (!(pdp_type_compat(packet0, packet1))) return 0;
    if (header0->type != PDP_IMAGE){
	//post("pdp_type_compat_image: not a PDP_IMAGE");
	return 0;
    }
    if (header0->info.image.encoding != header1->info.image.encoding){
	//post("pdp_type_compat_image: encodings differ");
	return 0;
    }
    if (header0->info.image.width != header1->info.image.width){
	//post("pdp_type_compat_image: image withs differ");
	return 0;
    }
    if (header0->info.image.height != header1->info.image.height){
	//post("pdp_type_compat_image: image heights differ");
	return 0;
    }
    return 1;
}

/* check if packet is a valid image packet */
int pdp_type_isvalid_image(int packet)
{
    t_pdp *header = pdp_packet_header(packet);
    if (!header) return 0;
    if (PDP_IMAGE != header->type) return 0;
    if ((PDP_IMAGE_YV12 != header->info.image.encoding)
	&& (PDP_IMAGE_GREY != header->info.image.encoding)) return 0;

    return 1;

}



int pdp_packet_new_image_yv12(u32 w, u32 h)
{
    t_pdp *header;
    int packet;


    u32 size = w*h;
    u32 totalnbpixels = size + (size >> 1);
    u32 packet_size = totalnbpixels << 1;

    packet = pdp_packet_new(PDP_IMAGE, packet_size);
    header = pdp_packet_header(packet);

    header->info.image.encoding = PDP_IMAGE_YV12;
    header->info.image.width = w;
    header->info.image.height = h;

    return packet;
}

int pdp_packet_new_image_grey(u32 w, u32 h)
{
    t_pdp *header;
    int packet;


    u32 size = w*h;
    u32 totalnbpixels = size;
    u32 packet_size = totalnbpixels << 1;

    packet = pdp_packet_new(PDP_IMAGE, packet_size);
    header = pdp_packet_header(packet);

    header->info.image.encoding = PDP_IMAGE_GREY;
    header->info.image.width = w;
    header->info.image.height = h;

    return packet;
}




#ifdef __cplusplus
}
#endif
