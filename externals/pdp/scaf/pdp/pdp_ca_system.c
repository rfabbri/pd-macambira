/*
 *   Cellular Automata Extension Module for pdp - Main system code
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

#include "pdp_ca.h"

/* all symbols are C-style */
#ifdef __cplusplus
extern "C"
{
#endif


/* check if packet is a valid ca packet */
int pdp_type_isvalid_ca(int packet)
{
    t_pdp *header = pdp_packet_header(packet);
    if (!header) return 0;
    if (PDP_CA != header->type) return 0;
    if (PDP_CA_STANDARD != pdp_type_ca_info(header)->encoding) return 0;

    return 1;
}

/* convert a CA packet to greyscale */

inline void _pdp_type_ca2grey_convert_word(unsigned short int source, short int  *dest)
{

    int i;
    for (i = 15; i>=0; i--){
	dest[i] =  ((unsigned short)(((short int)(source & 0x8000)) >> 14)) >> 1;  
	source <<= 1;
    }
}

int pdp_type_ca2grey(int packet)
{
    int w, h, s, x, y, srcindex;
    long long offset, xoffset, yoffset;
    short int *dest;
    unsigned short int *source;
    t_pdp *header;
    t_pdp *newheader;
    int newpacket;
    if (!(pdp_type_isvalid_ca(packet))) return -1;

    header = pdp_packet_header(packet);
    w = pdp_type_ca_info(header)->width;
    h = pdp_type_ca_info(header)->height;
    s = w*h;
    source = (unsigned short int *)pdp_packet_data(packet);
    offset = pdp_type_ca_info(header)->offset;
    yoffset = (offset / w) * w;
    xoffset = offset % w;

    //post("pdp_type_ca2grey: offset: %d, xoffset: %d, yoffset: %d", offset, xoffset, yoffset);

    newpacket = pdp_packet_new(PDP_IMAGE, s<<1);
    newheader = pdp_packet_header(newpacket);
    newheader->info.image.width = w;
    newheader->info.image.height = h;
    newheader->info.image.encoding = PDP_IMAGE_GREY;
    dest = (short int *)pdp_packet_data(newpacket);


#define check_srcindex \
if (srcindex >= (s >> 4)) post ("pdp_type_ca2grey: srcindex out of bound");

#define check_dstindex \
if ((x+y) >= s) post ("pdp_type_ca2grey: dstindex out of bound");


    /* dont' shift offset */
    if (0){
	for(y=0; y< (h*w); y+=w){
	    for(x=0; x<w; x+=16){
		 _pdp_type_ca2grey_convert_word (source[(x+y)>>4],  &dest[x+y]);
	    }
	}
	return newpacket;
    }


    /* create top left */
    for (y=0; y < (h*w) - yoffset; y+=w) { 
	for (x=0; x< (w - xoffset); x+=16) {
	    srcindex = (x+xoffset + y+yoffset) >> 4;
	    //check_srcindex;
	    //check_dstindex;
	    _pdp_type_ca2grey_convert_word (source[srcindex],  &dest[x+y]);
	}
    }
	    
    /* create top right */
    for (y=0; y < (h*w) - yoffset; y+=w) { 
	for (x = (w - xoffset); x < w; x+=16) {
	    srcindex = (x+xoffset-w + y+yoffset) >> 4;
	    //check_srcindex;
	    //check_dstindex;
	    _pdp_type_ca2grey_convert_word (source[srcindex],  &dest[x+y]);
	}
    }
	    
    /* create bottom left */
    for (y=(h*w) - yoffset; y < h*w; y+=w) { 
	for (x=0; x< (w - xoffset); x+=16) {
	    srcindex = (x+xoffset + y+yoffset-(w*h)) >> 4;
	    //check_srcindex;
	    //check_dstindex;
	    _pdp_type_ca2grey_convert_word (source[srcindex],  &dest[x+y]);
	}
    }
	    
    /* create bottom right */
    for (y=(h*w) - yoffset; y < h*w; y+=w) { 
	for (x = (w - xoffset); x < w; x+=16) {
	    srcindex = (x+xoffset-w + y+yoffset-(w*h)) >> 4;
	    //check_srcindex;
	    //check_dstindex;
	    _pdp_type_ca2grey_convert_word (source[srcindex],  &dest[x+y]);
	}
    }
	    

    return newpacket;

}


inline unsigned short int _pdp_type_grey2ca_convert_word(short int  *src, short int threshold)
{
    short int tmp;
    short int dest = 0;
    int i;

    for (i = 15; i >= 0; i--){
	dest <<= 1;
	dest |= (src[i] > threshold);
    }

    return dest;
}



int pdp_type_grey2ca(int packet, short int threshold)
{
    int w, h, s, x, y, srcindex;
    long long offset, xoffset, yoffset;
    short int *dest;
    short int *source;
    t_pdp *header;
    t_pdp *newheader;
    int newpacket;
    if (!(pdp_type_isvalid_image(packet))) return -1;

    header = pdp_packet_header(packet);
    w = header->info.image.width;
    h = header->info.image.height;
    s = w*h;
    source = (unsigned short int *)pdp_packet_data(packet);

    if ( (PDP_IMAGE_GREY != header->info.image.encoding)
	 && (PDP_IMAGE_YV12 != header->info.image.encoding)) return -1;

    newpacket = pdp_packet_new(PDP_CA, s>>3);
    newheader = pdp_packet_header(newpacket);
    pdp_type_ca_info(newheader)->width = w;
    pdp_type_ca_info(newheader)->height = h;
    pdp_type_ca_info(newheader)->encoding = PDP_CA_STANDARD;
    pdp_type_ca_info(newheader)->offset = 0;

    dest = (short int *)pdp_packet_data(newpacket);

    for(y=0; y< (h*w); y+=w){
	for(x=0; x<w; x+=16){
	    dest[(x+y)>>4] = _pdp_type_grey2ca_convert_word (&source[x+y], threshold);
	}
    }
    return newpacket;


}

/* returns a pointer to the ca subheader given the pdp header */
t_ca *pdp_type_ca_info(t_pdp *x){return (t_ca *)(&x->info.raw);}


void pdp_ca_setup(void);
void pdp_ca2image_setup(void);
void pdp_image2ca_setup(void);


void pdp_scaf_setup(void)
{
    /* babble */
    post ("PDP: pdp_scaf extension lib (mmx version)");

    /* setup modules */
    pdp_ca_setup();
    pdp_ca2image_setup();
    pdp_image2ca_setup();
    
}




#ifdef __cplusplus
}
#endif
