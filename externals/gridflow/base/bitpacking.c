/*
	$Id: bitpacking.c,v 1.2 2006-03-15 04:37:06 matju Exp $

	GridFlow
	Copyright (c) 2001,2002,2003,2004 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "grid.h.fcs"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

//#define CONVERT0(z) (((in[z] << hb[z]) >> 7) & mask[z])
#define CONVERT0(z) ((in[z] >> chop[z]) << slide[z])

#define CONVERT1 t = \
	CONVERT0(0) | CONVERT0(1) | CONVERT0(2)

#define CONVERT2 \
	for (t=0,i=0; i<self->size; i++) t |= CONVERT0(i);

#define CONVERT3 \
	for (t=0,i=0; i<self->size; i++) \
		t |= (((unsigned)in[i]>>(7-hb[i]))|(in[i]<<(hb[i]-7))) & mask[i];

#define WRITE_LE \
	for (int bytes = self->bytes; bytes; bytes--, t>>=8) *out++ = t;

#define WRITE_BE { int bytes; \
	bytes = self->bytes; \
	while (bytes--) { out[bytes] = t; t>>=8; }\
	out += self->bytes; }

/* this macro would be faster if the _increment_
   was done only once every loop. or maybe gcc does it, i dunno */
#define NTIMES(_x_) \
	for (; n>=4; n-=4) { _x_ _x_ _x_ _x_ } \
	for (; n; n--) { _x_ }

/* this could be faster (use asm) */
void swap32 (int n, Pt<uint32> data) {
	NTIMES({
		uint32 x = *data;
		x = (x<<16) | (x>>16);
		x = ((x&0xff00ff)<<8) | ((x>>8)&0xff00ff);
		*data++ = x;
	})
}

/* this could be faster (use asm or do it in int32 chunks) */
void swap16 (int n, Pt<uint16> data) {
	NTIMES({ uint16 x = *data; *data++ = (x<<8) | (x>>8); })
}

/* **************************************************************** */

template <class T>
static void default_pack(BitPacking *self, int n, Pt<T> in, Pt<uint8> out) {
	uint32 t;
	int i;
	int sameorder = self->endian==2 || self->endian==::is_le();
	int size = self->size;
	uint32  mask[4]; memcpy(mask,self->mask,size*sizeof(uint32));
	uint32    hb[4]; for (i=0; i<size; i++) hb[i] = highest_bit(mask[i]);
	uint32  span[4]; for (i=0; i<size; i++) span[i] = hb[i] - lowest_bit(mask[i]);
	uint32  chop[4]; for (i=0; i<size; i++) chop[i] = 7-span[i];
	uint32 slide[4]; for (i=0; i<size; i++) slide[i] = hb[i]-span[i];
	
	if (sameorder && size==3) {
		switch(self->bytes) {
		case 2:	NTIMES(CONVERT1; *((int16 *)out)=t; out+=2; in+=3;) return;
		case 4:	NTIMES(CONVERT1; *((int32 *)out)=t; out+=4; in+=3;) return;
		}
	}
	if (self->is_le()) {
		switch (size) {
		case 3: for (; n--; in+=3) {CONVERT1; WRITE_LE;} break;
		case 4:	for (; n--; in+=4) {CONVERT1; WRITE_LE;} break;
		default:for (; n--; in+=size) {CONVERT2; WRITE_LE;}}
	} else {
		switch (size) {
		case 3: for (; n--; in+=3) {CONVERT1; WRITE_BE;} break;
		case 4: for (; n--; in+=4) {CONVERT1; WRITE_BE;} break;
		default:for (; n--; in+=size) {CONVERT2; WRITE_BE;}}
	}
}

#define LOOP_UNPACK(_reader_) \
	for (; n; n--) { \
		int bytes=0; uint32 temp=0; _reader_; \
		for (int i=0; i<self->size; i++, out++) { \
			uint32 t=temp&self->mask[i]; \
			*out = (t<<(7-hb[i]))|(t>>(hb[i]-7)); \
		} \
	}
//			*out++ = ((temp & self->mask[i]) << 7) >> hb[i];

template <class T>
static void default_unpack(BitPacking *self, int n, Pt<uint8> in, Pt<T> out) {
	int hb[4];
	for (int i=0; i<self->size; i++) hb[i] = highest_bit(self->mask[i]);
	if (is_le()) { // smallest byte first
		LOOP_UNPACK(
			for(; self->bytes>bytes; bytes++, in++) temp |= *in<<(8*bytes);
		)
	} else { // biggest byte first
		LOOP_UNPACK(
			bytes=self->bytes; for (; bytes; bytes--, in++) temp=(temp<<8)|*in;
		)
	}
}

/* **************************************************************** */

template <class T>
static void pack2_565(BitPacking *self, int n, Pt<T> in, Pt<uint8> out) {
	const int hb[3] = {15,10,4};
	const uint32 mask[3] = {0x0000f800,0x000007e0,0x0000001f};
	uint32 span[3] = {4,5,4};
	uint32 chop[3] = {3,2,3};
	uint32 slide[3] = {11,5,0};
	uint32 t;
	NTIMES(CONVERT1; *((short *)out)=t; out+=2; in+=3;)
}

template <class T>
static void pack3_888(BitPacking *self, int n, Pt<T> in, Pt<uint8> out) {
	Pt<int32> o32 = (Pt<int32>)out;
	while (n>=4) {
		o32[0] = (in[5]<<24) | (in[ 0]<<16) | (in[ 1]<<8) | in[2];
		o32[1] = (in[7]<<24) | (in[ 8]<<16) | (in[ 3]<<8) | in[4];
		o32[2] = (in[9]<<24) | (in[10]<<16) | (in[11]<<8) | in[6];
		o32+=3; in+=12;
		n-=4;
	}
	out = (Pt<uint8>)o32;
	NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3; )
}

/*
template <>
static void pack3_888(BitPacking *self, int n, Pt<uint8> in, Pt<uint8> out) {
	Pt<uint32> o32 = Pt<uint32>((uint32 *)out.p,n*3/4);
	Pt<uint32> i32 = Pt<uint32>((uint32 *)in.p,n*3/4);
	while (n>=4) {
#define Z(w,i) ((word##w>>(i*8))&255)
		uint32 word0 = i32[0];
		uint32 word1 = i32[1];
		uint32 word2 = i32[2];
		o32[0] = (Z(1,1)<<24) | (Z(0,0)<<16) | (Z(0,1)<<8) | Z(0,2);
		o32[1] = (Z(1,3)<<24) | (Z(2,0)<<16) | (Z(0,3)<<8) | Z(1,0);
		o32[2] = (Z(2,1)<<24) | (Z(2,2)<<16) | (Z(2,3)<<8) | Z(1,2);
		o32+=3; i32+=3;
		n-=4;
	}
#undef Z
	out = (Pt<uint8>)o32;
	in  = (Pt<uint8>)i32;
	NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3; )
}
*/

template <class T>
static void pack3_888b(BitPacking *self, int n, Pt<T> in, Pt<uint8> out) {
	Pt<int32> o32 = (Pt<int32>)out;
	while (n>=4) {
		o32[0] = (in[0]<<16) | (in[1]<<8) | in[2];
		o32[1] = (in[3]<<16) | (in[4]<<8) | in[5];
		o32[2] = (in[6]<<16) | (in[7]<<8) | in[8];
		o32[3] = (in[9]<<16) | (in[10]<<8) | in[11];
		o32+=4; in+=12;
		n-=4;
	}
	NTIMES( o32[0] = (in[0]<<16) | (in[1]<<8) | in[2]; o32++; in+=3; )
}

// (R,G,B,?) -> B:8,G:8,R:8,0:8
// fishy
template <class T>
static void pack3_bgrn8888(BitPacking *self, int n, Pt<T> in, Pt<uint8> out) {
/* NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=4; in+=4; ) */
	Pt<int32> i32 = (Pt<int32>)in;
	Pt<int32> o32 = (Pt<int32>)out;
	while (n>=4) {
		o32[0] = ((i32[0]&0xff)<<16) | (i32[0]&0xff00) | ((i32[0]>>16)&0xff);
		o32[1] = ((i32[1]&0xff)<<16) | (i32[1]&0xff00) | ((i32[1]>>16)&0xff);
		o32[2] = ((i32[2]&0xff)<<16) | (i32[2]&0xff00) | ((i32[2]>>16)&0xff);
		o32[3] = ((i32[3]&0xff)<<16) | (i32[3]&0xff00) | ((i32[3]>>16)&0xff);
		o32+=4; i32+=4; n-=4;
	}
	NTIMES( o32[0] = ((i32[0]&0xff)<<16) | (i32[0]&0xff00) | ((i32[0]>>16)&0xff); o32++; i32++; )
}

static uint32 bp_masks[][4] = {
	{0x0000f800,0x000007e0,0x0000001f,0},
	{0x00ff0000,0x0000ff00,0x000000ff,0},
};

static Packer bp_packers[] = {
	{default_pack, default_pack, default_pack},
	{pack2_565, pack2_565, pack2_565},
	{pack3_888, pack3_888, pack3_888},
	{pack3_888b, default_pack, default_pack},
	{pack3_bgrn8888, default_pack, default_pack},
};

static Unpacker bp_unpackers[] = {
	{default_unpack, default_unpack, default_unpack},
};	

static BitPacking builtin_bitpackers[] = {
	BitPacking(2, 2, 3, bp_masks[0], &bp_packers[1], &bp_unpackers[0]),
	BitPacking(1, 3, 3, bp_masks[1], &bp_packers[2], &bp_unpackers[0]),
	BitPacking(1, 4, 3, bp_masks[1], &bp_packers[3], &bp_unpackers[0]),
	BitPacking(1, 4, 4, bp_masks[1], &bp_packers[4], &bp_unpackers[0]),
};

/* **************************************************************** */

bool BitPacking::eq(BitPacking *o) {
	if (!(bytes == o->bytes)) return false;
	if (!(size == o->size)) return false;
	for (int i=0; i<size; i++) {
		if (!(mask[i] == o->mask[i])) return false;
	}
	if (endian==o->endian) return true;
	/* same==little on a little-endian; same==big on a big-endian */
	return (endian ^ o->endian ^ ::is_le()) == 2;
}

BitPacking::BitPacking(int endian, int bytes, int size, uint32 *mask,
Packer *packer, Unpacker *unpacker) {
	this->endian = endian;
	this->bytes = bytes;
	this->size = size;
	for (int i=0; i<size; i++) this->mask[i] = mask[i];
	if (packer) {
		this->packer = packer;
		this->unpacker = unpacker;
		return;
	}
	int packeri=-1;
	this->packer = &bp_packers[0];
	this->unpacker = &bp_unpackers[0];

	for (int i=0; i<(int)(sizeof(builtin_bitpackers)/sizeof(BitPacking)); i++) {
		BitPacking *bp = &builtin_bitpackers[i];
		if (this->eq(bp)) {
			this->packer = bp->packer;
			this->unpacker = bp->unpacker;
			packeri=i;
			goto end;
		}
	}
end:;
/*
	::gfpost("Bitpacking: endian=%d bytes=%d size=%d packeri=%d",
		endian, bytes, size, packeri);
	::gfpost("  packer=0x%08x unpacker=0x%08x",this->packer,this->unpacker);
	::gfpost("  mask=[0x%08x,0x%08x,0x%08x,0x%08x]",mask[0],mask[1],mask[2],mask[3]);
*/
}

bool BitPacking::is_le() {
	return endian==1 || (endian ^ ::is_le())==3;
}

template <class T>
void BitPacking::pack(int n, Pt<T> in, Pt<uint8> out) {
	switch (NumberTypeE_type_of(*in)) {
	case uint8_e: packer->as_uint8(this,n,(Pt<uint8>)in,out); break;
	case int16_e: packer->as_int16(this,n,(Pt<int16>)in,out); break;
	case int32_e: packer->as_int32(this,n,(Pt<int32>)in,out); break;
	default: RAISE("argh");
	}
}

template <class T>
void BitPacking::unpack(int n, Pt<uint8> in, Pt<T> out) {
	switch (NumberTypeE_type_of(*out)) {
	case uint8_e: unpacker->as_uint8(this,n,in,(Pt<uint8>)out); break;
	case int16_e: unpacker->as_int16(this,n,in,(Pt<int16>)out); break;
	case int32_e: unpacker->as_int32(this,n,in,(Pt<int32>)out); break;
	default: RAISE("argh");
	}
}

// i'm sorry... see the end of grid.c for an explanation...
//static
void make_hocus_pocus () {
//	exit(1);
#define FOO(S) \
	((BitPacking*)0)->pack(0,Pt<S>(),Pt<uint8>()); \
	((BitPacking*)0)->unpack(0,Pt<uint8>(),Pt<S>());
EACH_NUMBER_TYPE(FOO)
#undef FOO
}
