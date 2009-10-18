/*
	$Id: rubyext.c 3621 2008-04-19 01:47:38Z matju $

	GridFlow
	Copyright (c) 2001-2009 by Mathieu Bouchard

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

#include "gridflow.h.fcs"
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
//#include <cstdlib>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <limits.h>

#ifdef HAVE_GEM
struct CPPExtern {
  static t_object *m_holder;
  static char *m_holdname;
};
#endif

#ifndef HAVE_DESIREDATA
#include "bundled/g_canvas.h"
#endif
// from pd/src/g_canvas.c
struct _canvasenvironment {
    t_symbol *ce_dir;   /* directory patch lives in */
    int ce_argc;        /* number of "$" arguments */
    t_atom *ce_argv;    /* array of "$" arguments */
    int ce_dollarzero;  /* value of "$0" */
};

/* for exception-handling in 0.9.0... Linux-only */
#ifndef MACOSX
#include <exception>
#include <execinfo.h>
#endif
#undef check

std::map<string,FClass *> fclasses;
std::map<t_class *,FClass *> fclasses_pd;

//using namespace std;

BuiltinSymbols bsym;

Barf::Barf(const char *s, ...) {
    std::ostringstream os;
    va_list ap;
    va_start(ap,s);
    voprintf(os,s,ap);
    va_end(ap);
    text = os.str();
}
Barf::Barf(const char *file, int line, const char *func, const char *fmt, ...) {
    std::ostringstream os;
    va_list ap;
    va_start(ap,fmt);
    voprintf(os,fmt,ap);
    //oprintf(os,"\n%s:%d:in `%s'",file,line,func);
    va_end(ap);
    text = os.str();
}

void Barf::error(BFObject *bself) {
	if (bself) pd_error(bself,"%s: %s",bself->binbuf_string().data(),text.data());
	else        ::error(      "%s: %s",bself->binbuf_string().data(),text.data());
}

void pd_oprint (std::ostream &o, int argc, t_atom *argv) {
	for (int i=0; i<argc; i++) {
		t_atomtype t = argv[i].a_type;
		if (t==A_FLOAT) o << argv[i].a_float;
		else if (t==A_SYMBOL) o << argv[i].a_symbol->s_name;
		else if (t==A_POINTER) o << "(pointer)";
 		else if (t==A_COMMA) o << ",";
 		else if (t==A_SEMI) o << ";";
		else if (t==A_LIST) {
			t_binbuf *b = (t_binbuf *)argv[i].a_gpointer;
			o << "[";
			pd_oprint(o,binbuf_getnatom(b),binbuf_getvec(b));
			o << "]";
		} else o << "(atom of type " << t << ")";
		if (i!=argc-1) o << " ";
	}
}

void pd_post (const char *s, int argc, t_atom *argv) {
	std::ostringstream os;
	if (s) os << s << ": ";
	pd_oprint(os,argc,argv);
	post("%s",os.str().data());
}

void pd_oprintf (std::ostream &o, const char *s, int argc, t_atom *argv) {
	int i=0;
	for (; *s; s++) {
		if (*s!='%') {o << (char)*s; continue;}
		s++; // skip the %
		switch (*s) {
		  case 'f':
			if (!argc) RAISE("not enough args");
			if (argv[i].a_type != A_FLOAT) RAISE("expected float");
			o << argv[i].a_float;
		  break;
		  case 's':
			if (!argc) RAISE("not enough args");
			if (argv[i].a_type != A_SYMBOL) RAISE("expected symbol");
			o << argv[i].a_symbol->s_name;
		  break;
		  case '_':
			if (!argc) RAISE("not enough args");
			char buf[MAXPDSTRING];
			atom_string(&argv[i],buf,MAXPDSTRING);
			o << buf;
		  break;
		  case '%':
			o << "%";
		  break;
		  default:
			RAISE("sorry, the format character '%c' is not supported yet",*s);
		}
	}
}

//----------------------------------------------------------------
// Dim

void Dim::check() {
	if (n>MAX_DIM) RAISE("too many dimensions");
	for (int i=0; i<n; i++) if (v[i]<0) RAISE("Dim: negative dimension");
}

// !@#$ big leak machine?
// returns a string like "Dim[240,320,3]"
char *Dim::to_s() {
	// if you blow 256 chars it's your own fault
	char buf[256];
	char *s = buf;
	s += sprintf(s,"Dim[");
	for(int i=0; i<n; i++) s += sprintf(s,"%s%d", ","+!i, v[i]);
	s += sprintf(s,"]");
	return strdup(buf);
}

NumberTypeE NumberTypeE_find (string s) {
	if (number_type_dict.find(s)==number_type_dict.end()) RAISE("unknown number type \"%s\"", s.data());
	return number_type_dict[s]->index;
}

NumberTypeE NumberTypeE_find (const t_atom &x) {
	if (x.a_type!=A_SYMBOL) RAISE("expected number-type (as symbol)");
	return NumberTypeE_find(string(x.a_symbol->s_name));
}

// don't touch.
static void gfmemcopy32(int32 *as, int32 *bs, long n) {
	ptrdiff_t ba = bs-as;
#define FOO(I) as[I] = (as+ba)[I];
		UNROLL_8(FOO,n,as)
#undef FOO
}

void gfmemcopy(uint8 *out, const uint8 *in, long n) {
	for (; n>16; in+=16, out+=16, n-=16) {
		((int32*)out)[0] = ((int32*)in)[0];
		((int32*)out)[1] = ((int32*)in)[1];
		((int32*)out)[2] = ((int32*)in)[2];
		((int32*)out)[3] = ((int32*)in)[3];
	}
	for (; n>4; in+=4, out+=4, n-=4) *(int32*)out = *(int32*)in;
	for (; n; in++, out++, n--) *out=*in;
}

//----------------------------------------------------------------

uint64 gf_timeofday () {
	timeval t;
	gettimeofday(&t,0);
	return t.tv_sec*1000000+t.tv_usec;
}

#define CONVERT0(z) ((in[z] >> chop[z]) << slide[z])
#define CONVERT1 t = CONVERT0(0) | CONVERT0(1) | CONVERT0(2)
#define CONVERT2 for (t=0,i=0; i<self->size; i++) t |= CONVERT0(i);

#define WRITE_LE \
	for (int bytes = self->bytes; bytes; bytes--, t>>=8) *out++ = t;

#define WRITE_BE {int bytes; \
	bytes = self->bytes; \
	while (bytes--) {out[bytes] = t; t>>=8;}\
	out += self->bytes;}

/* this macro would be faster if the _increment_
   was done only once every loop. or maybe gcc does it, i dunno */
#define NTIMES(_x_) \
	for (; n>=4; n-=4) {_x_ _x_ _x_ _x_} \
	for (; n; n--) {_x_}

/* this could be faster (use asm) */
void swap64 (long n, uint64 *data) {
	NTIMES({
		uint64 x = *data;
		x = (x<<32) | (x>>32);
		x = ((x&0x0000ffff0000ffffLL)<<16) | ((x>>16)&0x0000ffff0000ffffLL);
		x = ((x&0x00ff00ff00ff00ffLL)<< 8) | ((x>> 8)&0x00ff00ff00ff00ffLL);
		*data++ = x;
	})
}

/* this could be faster (use asm) */
void swap32 (long n, uint32 *data) {
	NTIMES({
		uint32 x = *data;
		x = (x<<16) | (x>>16);
		x = ((x&0xff00ff)<<8) | ((x>>8)&0xff00ff);
		*data++ = x;
	})
}

/* this could be faster (use asm or do it in int32 chunks) */
void swap16 (long n, uint16 *data) {NTIMES({ uint16 x = *data; *data++ = (x<<8) | (x>>8); })}

/* **************************************************************** */

template <class T>
static void default_pack(BitPacking *self, long n, T *in, uint8 *out) {
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
			*out = (t<<(7-hb[i]))|(t>>(hb[i]-7));}}

template <class T>
static void default_unpack(BitPacking *self, long n, uint8 * in, T * out) {
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
static void pack2_565(BitPacking *self, long n, T * in, uint8 * out) {
//	const int hb[3] = {15,10,4};
//	const uint32 mask[3] = {0x0000f800,0x000007e0,0x0000001f};
//	uint32 span[3] = {4,5,4};
	uint32 chop[3] = {3,2,3};
	uint32 slide[3] = {11,5,0};
	uint32 t;
	NTIMES(CONVERT1; *((short *)out)=t; out+=2; in+=3;)
}

template <class T>
static void pack3_888(BitPacking *self, long n, T *in, uint8 *out) {
	int32 * o32 = (int32 *)out;
	while (n>=4) {
		o32[0] = (in[5]<<24) | (in[ 0]<<16) | (in[ 1]<<8) | in[2];
		o32[1] = (in[7]<<24) | (in[ 8]<<16) | (in[ 3]<<8) | in[4];
		o32[2] = (in[9]<<24) | (in[10]<<16) | (in[11]<<8) | in[6];
		o32+=3; in+=12;
		n-=4;
	}
	out = (uint8 *)o32;
	NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3; )
}

template <class T>
static void unpack3_888(BitPacking *self, long n, uint8 *in, T *out) {
	NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3; )
}

/*
template <>
static void pack3_888(BitPacking *self, long n, uint8 * in, uint8 * out) {
	uint32 * o32 = uint32 *((uint32 *)out.p,n*3/4);
	uint32 * i32 = uint32 *((uint32 *)in.p,n*3/4);
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
	out = (uint8 *)o32;
	in  = (uint8 *)i32;
	NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=3; in+=3; )
}
*/

template <class T>
static void pack3_888b(BitPacking *self, long n, T * in, uint8 * out) {
	int32 * o32 = (int32 *)out;
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
static void pack3_bgrn8888(BitPacking *self, long n, T * in, uint8 * out) {
/* NTIMES( out[2]=in[0]; out[1]=in[1]; out[0]=in[2]; out+=4; in+=4; ) */
	int32 * i32 = (int32 *)in;
	int32 * o32 = (int32 *)out;
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
	{unpack3_888,  unpack3_888,  unpack3_888},
};	

static BitPacking builtin_bitpackers[] = {
	BitPacking(2, 2, 3, bp_masks[0], &bp_packers[1], &bp_unpackers[0]),
	BitPacking(1, 3, 3, bp_masks[1], &bp_packers[2], &bp_unpackers[1]),
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

BitPacking::BitPacking(int endian, int bytes, int size, uint32 *mask, Packer *packer, Unpacker *unpacker) {
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
#if 0
	::post("Bitpacking: endian=%d bytes=%d size=%d packeri=%d",
		endian, bytes, size, packeri);
	::post("  packer=0x%08x unpacker=0x%08x",this->packer,this->unpacker);
	::post("  mask=[0x%08x,0x%08x,0x%08x,0x%08x]",mask[0],mask[1],mask[2],mask[3]);
#endif
}

bool BitPacking::is_le() {
	return endian==1 || (endian ^ ::is_le())==3;
}

template <class T>
void BitPacking::pack(long n, T * in, uint8 * out) {
	switch (NumberTypeE_type_of(in)) {
	case uint8_e: packer->as_uint8(this,n,(uint8 *)in,out); break;
	case int16_e: packer->as_int16(this,n,(int16 *)in,out); break;
	case int32_e: packer->as_int32(this,n,(int32 *)in,out); break;
	default: RAISE("argh");
	}
}

template <class T>
void BitPacking::unpack(long n, uint8 * in, T * out) {
	switch (NumberTypeE_type_of(out)) {
	case uint8_e: unpacker->as_uint8(this,n,in,(uint8 *)out); break;
	case int16_e: unpacker->as_int16(this,n,in,(int16 *)out); break;
	case int32_e: unpacker->as_int32(this,n,in,(int32 *)out); break;
	default: RAISE("argh");
	}
}

// i'm sorry... see the end of grid.c for an explanation...
//static
void make_hocus_pocus () {
//	exit(1);
#define FOO(S) \
	((BitPacking*)0)->pack(0,(S *)0,(uint8 *)0); \
	((BitPacking*)0)->unpack(0,(uint8 *)0,(S *)0);
EACH_NUMBER_TYPE(FOO)
#undef FOO
}

std::vector<string> gf_data_path;
string gf_find_file (string x) {
	if (strchr(x.data(),'/')) return x;
	int n = gf_data_path.size();
	struct stat dummy;
	for (int i=0; i<n; i++) {
		string s = gf_data_path[i]+"/"+x;
		if (lstat(s.data(),&dummy)==0) return s;
	}
	return x;
}

/* **************************************************************** */

#undef pd_class
#define pd_class(x) (*(t_pd *)x)
#define pd_classname(x) (fclasses_pd[pd_class(x)]->name.data())

static FMethod funcall_lookup (FClass *fclass, const char *sel) {
	int n = fclass->methodsn;
	for (int i=0; i<n; i++) if (strcmp(fclass->methods[i].selector,sel)==0) return fclass->methods[i].method;
	if (fclass->super) return funcall_lookup(fclass->super,sel);
	return 0;
}
static FMethod funcall_lookup (BFObject *bself, const char *sel) {
	return funcall_lookup(fclasses_pd[pd_class(bself)],sel);
}

void call_super(int argc, t_atom *argv) {/* unimplemented */}

//****************************************************************
// BFObject

struct BFProxy : t_object {
	BFObject *parent;
	t_inlet *inlet;
	int id;
};

static t_class *BFProxy_class;

static void BFObject_anything (BFObject *bself, int winlet, t_symbol *selector, int ac, t_atom2 *at) {
    try {
	t_atom2 argv[ac+1];
	for (int i=0; i<ac; i++) argv[i+1] = at[i];
	int argc = handle_braces(ac,argv+1);
	SETFLOAT(argv+0,winlet);
	char buf[256];
	sprintf(buf,"_n_%s",selector->s_name);
	FMethod m;
	m = funcall_lookup(bself,buf);
	if (m) {m(bself->self,argc+1,argv); return;}
	sprintf(buf,"_%d_%s",winlet,selector->s_name);
	m = funcall_lookup(bself,buf);
	if (m) {m(bself->self,argc,argv+1); return;}
	m = funcall_lookup(bself,"anything");
	if (m) {SETSYMBOL(argv+0,gensym(buf)); m(bself->self,argc+1,argv); return;}
	pd_error((t_pd *)bself, "method '%s' not found for inlet %d in class '%s'",selector->s_name,winlet,pd_classname(bself));
    } catch (Barf &oozy) {oozy.error(bself);}
}
static void BFObject_anything0 (BFObject *self, t_symbol *s, int argc, t_atom2 *argv) {
	BFObject_anything(self,0,s,argc,argv);
}
static void BFProxy_anything   (BFProxy *self,  t_symbol *s, int argc, t_atom2 *argv) {
	BFObject_anything(self->parent,self->id,s,argc,argv);
}

static void *BFObject_new (t_symbol *classsym, int ac, t_atom *at) {
    string name = string(classsym->s_name);
    if (fclasses.find(name)==fclasses.end()) {post("GF: class not found: '%s'",classsym->s_name); return 0;}
    t_class *qlass = fclasses[name]->bfclass;
    BFObject *bself = (BFObject *)pd_new(qlass);
    try {
	int argc = ac;
	t_atom argv[argc];
	for (int i=0; i<argc; i++) argv[i] = at[i];
	argc = handle_braces(argc,argv);
	//pd_post(classsym->s_name,argc,argv);
#ifdef HAVE_GEM
	CPPExtern::m_holder = (t_object *)bself;
	CPPExtern::m_holdname = const_cast<char *>("juste_pour_que_gem_reste_content");
#endif
	int j;
	for (j=0; j<argc; j++) if (argv[j].a_type==A_COMMA) break;

	bself->self = 0;
	bself->mom = (t_canvas *)canvas_getcurrent();
	bself->ninlets  = 1;
	bself->noutlets = 0;
	bself->inlets  = new  BFProxy*[1];
	bself->outlets = new t_outlet*[1];
	bself->inlets[0] = 0; // inlet 0 of this table is not in use
	bself->ninlets_set( fclasses[classsym->s_name]->ninlets ,false);
	bself->noutlets_set(fclasses[classsym->s_name]->noutlets,false);
	t_allocator alloc = fclasses[string(classsym->s_name)]->allocator;
	bself->self = alloc(bself,0,j,(t_atom2 *)argv);
#ifdef HAVE_GEM
	CPPExtern::m_holder = 0;
#ifdef HAVE_HOLDNAME
	CPPExtern::m_holdname=0;
#endif
#endif

	while (j<argc) {
		j++;
		int k=j;
		for (; j<argc; j++) if (argv[j].a_type==A_COMMA) break;
		if (argv[k].a_type==A_SYMBOL) pd_typedmess((t_pd *)bself,argv[k].a_symbol,j-k-1,argv+k+1);
	}
	return bself;
    } catch (Barf &oozy) {oozy.error(bself); return 0;}
}

static void BFObject_delete (BFObject *bself) {
	try {delete bself->self;} catch (Barf &oozy) {oozy.error(bself);}
	bself->ninlets_set(1,false);
	delete[] bself->inlets;
	delete[] bself->outlets;
}

//****************************************************************

static void BFObject_undrawio (BFObject *bself) {
#ifndef HAVE_DESIREDATA
	if (!bself->mom || !glist_isvisible(bself->mom)) return;
	t_rtext *rt = glist_findrtext(bself->mom,bself);
	if (!rt) return;
	glist_eraseiofor(bself->mom,bself,rtext_gettag(rt));
#endif
}

static void BFObject_redraw (BFObject *bself) {
#ifndef HAVE_DESIREDATA
	if (!bself->mom || !glist_isvisible(bself->mom)) return;
	t_rtext *rt = glist_findrtext(bself->mom,bself);
	if (!rt) return;
	gobj_vis((t_gobj *)bself,bself->mom,0);
	gobj_vis((t_gobj *)bself,bself->mom,1);
	canvas_fixlinesfor(bself->mom,(t_text *)bself);
#endif
}

/* warning: deleting inlets that are connected will cause pd to crash */
void BFObject::ninlets_set (int n, bool draw) {
	if (!te_binbuf) draw=false;
	if (n<1) RAISE("ninlets_set: n=%d must be at least 1",n);
	if (draw) BFObject_undrawio(this);
	if (ninlets<n) {
		BFProxy **noo = new BFProxy*[n];
		memcpy(noo,inlets,ninlets*sizeof(BFProxy*));
		delete[] inlets;
		inlets = noo;
		while (ninlets<n) {
			BFProxy *p = inlets[ninlets] = (BFProxy *)pd_new(BFProxy_class);
			p->parent = this;
			p->id = ninlets;
			p->inlet = inlet_new(this, &p->ob_pd, 0,0);
			ninlets++;
		}
	} else {
		while (ninlets>n) {
			ninlets--;
			inlet_free(inlets[ninlets]->inlet);
			pd_free((t_pd *)inlets[ninlets]);
		}
	}
	if (draw) BFObject_redraw(this);
}
/* warning: deleting outlets that are connected will cause pd to crash */
void BFObject::noutlets_set (int n, bool draw) {
	if (!te_binbuf) draw=false;
	if (n<0) RAISE("noutlets_set: n=%d must be at least 0",n);
	if (draw) BFObject_undrawio(this);
	if (noutlets<n) {
		t_outlet **noo = new t_outlet*[n>0?n:1];
		memcpy(noo,outlets,noutlets*sizeof(t_outlet*));
		delete[] outlets;
		outlets = noo;
		while (noutlets<n) outlets[noutlets++] = outlet_new(this,&s_anything);
	} else {
		while (noutlets>n) outlet_free(outlets[--noutlets]);
	}
	if (draw) BFObject_redraw(this);
}

string BFObject::binbuf_string () {
	if (!te_binbuf) return "[???]";
	std::ostringstream s;
	int n = binbuf_getnatom(te_binbuf);
	t_atom *at = binbuf_getvec(te_binbuf);
	for (int i=0; i<n; i++) s << (i ? " " : "[") << at[i];
	s << "]";
	return s.str();
}

void add_creator2(FClass *fclass, const char *name) {
	fclasses[string(name)] = fclass;
	class_addcreator((t_newmethod)BFObject_new,gensym((char *)name),A_GIMME,0);
}
typedef struct _methodentry
{
    t_symbol *me_name;
    t_gotfn me_fun;
    t_atomtype me_arg[MAXPDARG+1];
} t_methodentry;
struct _class {
    t_symbol *c_name;                   /* name (mostly for error reporting) */
    t_symbol *c_helpname;               /* name of help file */
    t_symbol *c_externdir;              /* directory extern was loaded from */
    size_t c_size;                      /* size of an instance */
    t_methodentry *c_methods;           /* methods other than bang, etc below */
    int c_nmethod;                      /* number of methods */
    // ...
};
void add_creator3(FClass *fclass, const char *name) {
	fclasses[string(name)] = fclass;
	t_class *c = pd_objectmaker;
	t_symbol *want = gensym(name);
	for (int i=c->c_nmethod-1; i>=0; i--) {
		t_methodentry *m = c->c_methods+i;
		if (m->me_name==want) {m->me_fun = t_gotfn(BFObject_new); m->me_arg[0]=A_GIMME; m->me_arg[1]=A_NULL; break;}
	}
}

//****************************************************************

struct t_namelist;
extern t_namelist *sys_searchpath, *sys_helppath;
extern "C" t_namelist *namelist_append_files(t_namelist *, char *);
static void add_to_path(char *dir) {
	static bool debug = false;
	char bof[1024];
	if (debug) post("gridflow was found in %s",dir);
	gf_data_path.push_back(string(dir)+"/images");
	if (debug) post("adding gf_data_path %s/images",dir);
	sprintf(bof,"%s/abstractions",dir);        sys_searchpath = namelist_append_files(sys_searchpath,bof);
	if (debug) post("adding -path %s",bof);
	sprintf(bof,"%s/deprecated",dir);          sys_searchpath = namelist_append_files(sys_searchpath,bof);
	if (debug) post("adding -path %s",bof);
	sprintf(bof,"%s/doc/flow_classes",dir);    sys_helppath   = namelist_append_files(sys_helppath,  bof);
	if (debug) post("adding -helppath %s",bof);
}

//----------------------------------------------------------------

t_list *list_new (int argc, t_atom *argv) {
	t_list *b = binbuf_new();
	binbuf_add(b,argc,argv);
	return b;
}
void list_free (t_list *self) {binbuf_free(self);}

//----------------------------------------------------------------

void fclass_install(FClass *fclass, FClass *super, size_t bytes) {
	fclass->super = super;
	if (fclass->startup) fclass->startup(fclass);
	fclass->bytes = bytes;
}

void install2(FClass *fclass, const char *name, int inlets, int outlets) {
	fclass->ninlets = inlets;
	fclass->noutlets = outlets;
	fclass->name = string(name);
	fclass->bfclass = class_new(gensym((char *)name), (t_newmethod)BFObject_new, (t_method)BFObject_delete,
		sizeof(BFObject), CLASS_DEFAULT, A_GIMME,0);
	fclasses[string(name)] = fclass;
	fclasses_pd[fclass->bfclass] = fclass;
	class_addanything(fclass->bfclass,(t_method)BFObject_anything0);
}

/* This code handles nested lists because PureData (all versions including 0.40) doesn't do it */
int handle_braces(int ac, t_atom *av) {
	int stack[16];
	int stackn=0;
	int j=0;
	t_binbuf *buf = binbuf_new();
	for (int i=0; i<ac; ) {
		int close=0;
		if (av[i].a_type==A_SYMBOL) {
			const char *s = av[i].a_symbol->s_name;
			while (*s=='(') {
				if (stackn==16) {binbuf_free(buf); RAISE("too many nested lists (>16)");}
				stack[stackn++]=j;
				s++;
			}
			const char *se = s+strlen(s);
			while (se>s && se[-1]==')') {se--; close++;}
			if (s!=se) {
				binbuf_text(buf,(char *)s,se-s);
				if ((binbuf_getnatom(buf)==1 && binbuf_getvec(buf)[0].a_type==A_FLOAT) || binbuf_getvec(buf)[0].a_type==A_COMMA) {
					av[j++] = binbuf_getvec(buf)[0];
				} else {
					char ss[MAXPDSTRING];
					int n = min(long(se-s),long(MAXPDSTRING-1));
					sprintf(ss,"%.*s",n,s);
					SETSYMBOL(av+j,gensym(ss)); j++; // av[j++] = gensym(s);
				}
			}
		} else av[j++]=av[i];
		i++;
		while (close--) {
			if (!stackn) {binbuf_free(buf); RAISE("close-paren without open-paren",av[i]);}
			t_binbuf *a2 = binbuf_new(); /* leak because there is no deallocation mechanism whatsoever */
			int j2 = stack[--stackn];
			binbuf_add(a2,j-j2,av+j2);
			j=j2;
			SETLIST(av+j,a2);
			j++;
		}
	}
	binbuf_free(buf);
	if (stackn) RAISE("too many open-paren (%d)",stackn);
	return j;
}

// foreach macro from desiredata:
#define foreach(ITER,COLL) for(typeof(COLL.begin()) ITER = COLL.begin(); ITER != (COLL).end(); ITER++)

\class FObject
\def 0 get (t_symbol *s=0) {
	FClass *fc = fclasses_pd[pd_class(bself)];
	if (!s) {
		t_atom a[1];
		foreach(attr,fc->attrs) {
			SETSYMBOL(a,gensym((char *)attr->second->name.data()));
			pd_typedmess((t_pd *)bself,gensym("get"),1,a);
		}
	} else {
		//t_atom a[1];
		//outlet_anything(bself->outlets[bself->noutlets-1],s,1,a);
		FMethod m = funcall_lookup(bself,"___get");
		t_atom2 a[1];
		SETSYMBOL(a,s);
		if (m) m(this,1,a);
	}
}
\def 0 help () {
	FClass *fc = fclasses_pd[pd_class(bself)];
	post("attributes (");
	foreach(attr,fc->attrs) post("    %s %s;",attr->second->type.data(),attr->second->name.data());
	post(")");
	post("methods (");
	for (int i=0; i<fc->methodsn; i++) post("    %s",fc->methods[i].selector);
	post(")");
}
\classinfo {}
\end class

void startup_number();
void startup_flow_objects();
void startup_flow_objects2();
void startup_format();
STARTUP_LIST(void)

void blargh () {
#ifdef MACOSX
  fprintf(stderr,"unhandled exception\n");
#else
  void *array[25];
  int nSize = backtrace(array, 25);
  char **symbols = backtrace_symbols(array, nSize);
  for (int i=0; i<nSize; i++) fprintf(stderr,"%d: %s\n",i,symbols[i]);
  free(symbols);
#endif
}

static t_gobj *canvas_last (t_canvas *self) {
#ifdef DESIRE
	t_gobj *g = canvas_first(self);
	while (gobj_next(g)) g=gobj_next(g);
#else
	t_gobj *g = self->gl_list;
	while (g->g_next) g=g->g_next;
#endif
	return g;
}

#ifdef DESIRE
extern "C" void canvas_delete(t_canvas *, t_gobj *);
#define glist_delete canvas_delete
#endif

static void canvas_else (t_canvas *self, t_symbol *s, int argc, t_atom *argv) {
	t_gobj *g = canvas_last(self);
	if (pd_newest()) return;
	glist_delete(self,g);
	if (argc<1 || argv[0].a_type!=A_SYMBOL) {error("$1 must be a symbol"); return;}
	pd_typedmess((t_pd *)self,argv[0].a_w.w_symbol,argc-1,argv+1);
}

// those are not really leaks but deleting them make them disappear from valgrind
// however, there's still a problem doing it, so, we won't do it.
static void gridflow_unsetup () {
/*
	foreach(iter,fclasses_pd) {
		FClass *fc = iter->second;
		foreach(iter2,fc->attrs) delete iter2->second;
		fc->FClass::~FClass();
	}
*/
}

void allow_big_stack () {
  struct rlimit happy;
  if (0>getrlimit(RLIMIT_STACK,&happy))
    error("GF: getrlimit: %s",strerror(errno));
  happy.rlim_cur = happy.rlim_max;
  if (0>setrlimit(RLIMIT_STACK,&happy))
    error("GF: setting stack size to %ld: %s",happy.rlim_cur,strerror(errno));
  else
    post( "GF: setting stack size to %ld",happy.rlim_cur);
}

// note: contrary to what m_pd.h says, pd_getfilename() and pd_getdirname()
// don't exist; also, canvas_getcurrentdir() isn't available during setup
// (segfaults), in addition to libraries not being canvases ;-)
// AND ALSO, CONTRARY TO WHAT m_pd.h SAYS, open_via_path()'s args are reversed!!!
extern "C" void gridflow_setup () {
    post("GridFlow " GF_VERSION ", Copyright (c) 2001-2008 Mathieu Bouchard");
    post("GridFlow was compiled on "__DATE__", "__TIME__);
    //std::set_terminate(__gnu_cxx::__verbose_terminate_handler);
    std::set_terminate(blargh);
    allow_big_stack();
    try {
	char *dirname   = new char[MAXPDSTRING];
	char *dirresult = new char[MAXPDSTRING];
	char *nameresult;
	char *zz=getcwd(dirname,MAXPDSTRING); /* zz only exists because gcc 4.3.3 gives me a bogus warning otherwise. */
	if (zz<0) {post("AAAARRRRGGGGHHHH!"); exit(69);}
	int       fd=open_via_path(dirname,"gridflow/gridflow",PDSUF,dirresult,&nameresult,MAXPDSTRING,1);
	if (fd<0) fd=open_via_path(dirname,         "gridflow",PDSUF,dirresult,&nameresult,MAXPDSTRING,1);
	if (fd>=0) close(fd); else post("%s was not found via the -path!","gridflow"PDSUF);
	/* nameresult is only a pointer in dirresult space so don't delete[] it. */
	add_to_path(dirresult);
	BFProxy_class = class_new(gensym("gf.proxy"),0,0,sizeof(BFProxy),CLASS_PD|CLASS_NOINLET, A_NULL);
	class_addanything(BFProxy_class,BFProxy_anything);
        srandom(rdtsc());
#define FOO(_sym_,_name_) bsym._sym_ = gensym(_name_);
BUILTIN_SYMBOLS(FOO)
#undef FOO
	startup_number();
	\startall
	startup_flow_objects();
	startup_flow_objects2();
	startup_format();
	STARTUP_LIST()
	//sys_gui("bind . <Motion> {puts %W}\n");
	sys_vgui(".mbar.help add command -label \"GridFlow index\" -command {pd pd open index.pd %s/doc \\;}\n",dirresult);
	delete[] dirresult;
	delete[] dirname;
    } catch (Barf &oozy) {oozy.error(0);}
    signal(SIGSEGV,SIG_DFL);
    signal(SIGABRT,SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    atexit(gridflow_unsetup);
    extern t_class *canvas_class;
    class_addmethod(canvas_class,(t_method)canvas_else,gensym("else"),A_GIMME,0);
}
