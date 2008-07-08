/*
	$Id$

	GridFlow
	Copyright (c) 2001-2007 by Mathieu Bouchard

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

#ifndef __GRIDFLOW_H
#define __GRIDFLOW_H

#define GF_VERSION "0.9.4"

#include "m_pd.h"
#include "config.h"
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <math.h>
#ifdef __APPLE__
static inline void *memalign (size_t a, size_t n) {return malloc(n);}
#else
#include <malloc.h>
#endif

#ifndef a_float
#define a_float    a_w.w_float
#define a_symbol   a_w.w_symbol
#define a_gpointer a_w.w_gpointer
#endif

#define gensym(s) gensym(const_cast<char *>(s))
#define sys_vgui(FMT,ARGS...) sys_vgui(const_cast<char *>(FMT),ARGS)
#define sys_gui(s) sys_gui(const_cast<char *>(s));

#ifndef DESIREDATA
#define A_LIST t_atomtype(13) /* (t_binbuf *) */
#endif
// the use of w_gpointer here is fake, just because there's no suitable member in the union
static inline void SETLIST(t_atom *a, t_binbuf *b) {a->a_type = A_LIST; a->a_gpointer = (t_gpointer *)b;}
static inline void SETNULL(t_atom *a)              {a->a_type = A_NULL; a->a_gpointer = 0;}

typedef char       int8; typedef unsigned char      uint8;
typedef short     int16; typedef unsigned short     uint16;
typedef int       int32; typedef unsigned int       uint32;
typedef long long int64; typedef unsigned long long uint64;
typedef float   float32;
typedef double  float64;

// three-way comparison (T is assumed Comparable)
template <class T> static inline T cmp(T a, T b) { return a<b ? -1 : a>b; }

// a remainder function such that div2(a,b)*b+mod(a,b) = a and for which mod(a,b) is in [0;b) or (b;0].
// in contrast to C-language builtin a%b, this one has uniform behaviour around zero,
// that is, the same as around any other whole number.
static inline int mod(int a, int b) {int c=a%b; c+=b&-(c&&(a<0)^(b<0)); return c;}

// counterpart of mod(a,b), just like a/b and a%b are counterparts
static inline int div2(int a, int b) {return (a/b)-((a<0)&&!!(a%b));}

static inline int32   gf_abs(  int32 a) {return a>0?a:-a;}
static inline int64   gf_abs(  int64 a) {return a>0?a:-a;}
static inline float32 gf_abs(float32 a) {return fabs(a);}
static inline float64 gf_abs(float64 a) {return fabs(a);}

// integer powers in log(b) time. T is assumed Integer
template <class T> static inline T ipow(T a, T b) {T r=1; for(;;) {if (b&1) r*=a; b>>=1; if (!b) return r; a*=a;}}
static inline float32 ipow(float32 a, float32 b) {return pow(a,b);}
static inline float64 ipow(float64 a, float64 b) {return pow(a,b);}

#undef min
#undef max
// minimum/maximum functions; T is assumed to be Comparable
template <class T> static inline T min(T a, T b) {return a<b?a:b;}
template <class T> static inline T max(T a, T b) {return a>b?a:b;}
template <class T> static inline T clip(T a, T lower, T upper) {return a<lower?lower:a>upper?upper:a;}
//template <class T> inline T min(T a, T b) { T c = (a-b)>>31; return (a&c)|(b&~c); }

// greatest common divisor, by euclid's algorithm
// this runs in log(a+b) number operations
template <class T> static T gcd (T a, T b) {
	while (b) {T c=mod(a,b); a=b; b=c;}
	return a;
}

// greatest common divisor, the binary algorithm. haven't tried yet.
template <class T> static T gcd2 (T a, T b) {
	int s=0;
	while (((a|b)&1)==0) { a>>=1; b>>=1; s++; }
	while (a) {
		if ((a&1)==0) a>>=1;
		else if ((b&1)==0) b>>=1;
		else {T t=abs(a-b); if (a<b) b=t; else a=t;}
	}
	return b<<s;
}

// least common multiple; this runs in log(a+b) like gcd.
template <class T> static inline T lcm (T a, T b) {return a*b/gcd(a,b);}

// returns the position (0..63) of highest bit set in a word, or 0 if none.
#define Z(N) if ((x>>N)&(((typeof(x))1<<N)-1)) { x>>=N; i+=N; }
static int highest_bit(uint8  x) {int i=0;              Z(4)Z(2)Z(1)return i;}
static int highest_bit(uint16 x) {int i=0;          Z(8)Z(4)Z(2)Z(1)return i;}
static int highest_bit(uint32 x) {int i=0;     Z(16)Z(8)Z(4)Z(2)Z(1)return i;}
static int highest_bit(uint64 x) {int i=0;Z(32)Z(16)Z(8)Z(4)Z(2)Z(1)return i;}
#undef Z
// returns the position (0..63) of lowest bit set in a word, or 0 if none.
template <class T> static int lowest_bit(T n) { return highest_bit((~n+1)&n); }

static double drand() { return 1.0*rand()/(RAND_MAX+1.0); }

// is little-endian
static inline bool is_le() {int x=1; return ((char *)&x)[0];}

static inline uint64 rdtsc()
#if defined(HAVE_PENTIUM)
{uint64 x; __asm__ volatile (".byte 0x0f, 0x31":"=A"(x)); return x;}
#else
{return 0;}
#endif

#ifdef HAVE_LITE
#define EACH_INT_TYPE(MACRO) MACRO(uint8) MACRO(int16) MACRO(int32)
#define EACH_FLOAT_TYPE(MACRO) MACRO(float32)
#else
#define EACH_INT_TYPE(MACRO) MACRO(uint8) MACRO(int16) MACRO(int32) MACRO(int64)
#define EACH_FLOAT_TYPE(MACRO) MACRO(float32) MACRO(float64)
#endif
#define EACH_NUMBER_TYPE(MACRO) EACH_INT_TYPE(MACRO) EACH_FLOAT_TYPE(MACRO)

// note: loop unrolling macros assume N!=0
// btw this may cause alignment problems when 8 does not divide N
#define UNROLL_8(MACRO,N,PTR,ARGS...) \
	int n__=(-N)&7; PTR-=n__; N+=n__; \
	switch (n__) { start: \
		case 0:MACRO(0); case 1:MACRO(1); case 2:MACRO(2); case 3:MACRO(3); \
		case 4:MACRO(4); case 5:MACRO(5); case 6:MACRO(6); case 7:MACRO(7); \
		PTR+=8; N-=8; ARGS; if (N) goto start; }
#define UNROLL_4(MACRO,N,PTR,ARGS...) \
	int n__=(-N)&3; PTR-=n__; N+=n__; \
	switch (n__) { start: \
		case 0:MACRO(0); case 1:MACRO(1); case 2:MACRO(2); case 3:MACRO(3); \
		PTR+=4; N-=4; ARGS; if (N) goto start; }

struct Barf {
  char *text;
  Barf(const char *s, ...);
  Barf(const char *file, int line, const char *func, const char *s, ...);
  ~Barf() {free(text);}
};

#include <stdio.h>
extern "C" void *gfmalloc(size_t n);
extern "C" void gffree(void *p);
#if __GNUC__<4
#define ALLOCPREFIX static inline
#else
#define ALLOCPREFIX inline
#endif
#include "base/new.h"
ALLOCPREFIX void *operator new   (size_t n)          throw (std::bad_alloc) {return gfmalloc(n);}
ALLOCPREFIX void *operator new[] (size_t n)          throw (std::bad_alloc) {return gfmalloc(n);}
ALLOCPREFIX void *operator new   (size_t n, const std::nothrow_t&) throw () {return gfmalloc(n);}
ALLOCPREFIX void *operator new[] (size_t n, const std::nothrow_t&) throw () {return gfmalloc(n);}
ALLOCPREFIX void operator delete  (void*p) throw() {gffree(p);}
ALLOCPREFIX void operator delete[](void*p) throw() {gffree(p);}
ALLOCPREFIX void operator delete  (void*p, const std::nothrow_t&) throw() {gffree(p);}
ALLOCPREFIX void operator delete[](void*p, const std::nothrow_t&) throw() {gffree(p);}

#define NEWBUF(T,N) (new T[N])
#define DELBUF(A) (delete[] A)

#ifdef __WIN32__
#define INT winINT
#define random rand
#undef send
#undef close
#define sigjmp_buf jmp_buf
#define siglongjmp longjmp
#endif

//#define _L_ post("%s:%d in %s",__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define _L_ fprintf(stderr,"%s:%d in %s\n",__FILE__,__LINE__,__PRETTY_FUNCTION__);
#define RAISE(args...) throw Barf(__FILE__,__LINE__,__PRETTY_FUNCTION__,args)
#define VA int argc, t_atom2 *argv
// returns the size of a statically defined array
#define COUNT(_array_) ((int)(sizeof(_array_) / sizeof((_array_)[0])))
#define WATCH(n,ar) { \
	char foo[16*1024], *p=foo; p += sprintf(p,"%s: ",#ar); \
	for (int q=0; q<n; q++) p += sprintf(p,"%lld ",(long long)ar[q]); \
	post("%s",foo);}

//****************************************************************

struct FObject;
struct t_atom2;
typedef void (*FMethod)(FObject *, int, t_atom2 *);
typedef std::string string;

#define BUILTIN_SYMBOLS(MACRO) \
	MACRO(_grid,"grid") MACRO(_bang,"bang") MACRO(_float,"float") \
	MACRO(_list,"list") MACRO(_sharp,"#") \
	MACRO(_in,"in") MACRO(_out,"out")

extern struct BuiltinSymbols {
#define FOO(_sym_,_str_) t_symbol *_sym_;
BUILTIN_SYMBOLS(FOO)
#undef FOO
} bsym;

struct Numop;
struct Pointer;
#define INT(x)  convert(x,(int32*)0)
#define TO(T,x) convert(x,(T*)0)

// trick to be able to define methods in t_atom
struct t_atom2 : t_atom {
	bool operator == (t_symbol *b) {return this->a_type==A_SYMBOL && this->a_symbol==b;}
	bool operator != (t_symbol *b) {return !(*this==b);}
	operator float32 () const {
		if (a_type!=A_FLOAT) RAISE("expected float");
		return a_float;}
	operator bool () const {float f = round(float32(*this));
		if (f<0 || f>=2) RAISE("value %f is out of range",f);
		return (bool)f;}
	operator uint8 () const {float f = round(float32(*this));
		if (f<0 || f>=0x100) RAISE("value %f is out of range",f);
		return (uint8)f;}
	operator int16 () const {float f = round(float32(*this));
		if (f<-0x8000 || f>=0x8000) RAISE("value %f is out of range",f);
		return (int16)f;}
	operator uint16 () const {float f = round(float32(*this));
		if (f<0 || f>=0x10000) RAISE("value %f is out of range",f);
		return (uint16)f;}
	operator int32 () const {float f = round(float32(*this));
		if (f<-0x80000000LL || f>=0x80000000LL) RAISE("value %f is out of range",f);
		return (int32)f;}
	operator uint32 () const {float f = round(float32(*this));
		if (f<0 || f>=0x100000000LL) RAISE("value %f is out of range",f);
		return (uint32)f;}
	operator long () const {
		return sizeof(long)==sizeof(int32) ? (int32)*this : (int64)*this;}
	operator uint64 () const {
		if (a_type!=A_FLOAT) RAISE("expected float");
		return (uint64)round(a_float);}
	operator int64 () const {
		if (a_type!=A_FLOAT) RAISE("expected float");
		return (int64)round(a_float);}
	operator float64 () const {
		if (a_type!=A_FLOAT) RAISE("expected float");
		return a_float;}
	operator t_symbol * () const {
		if (a_type!=A_SYMBOL) RAISE("expected symbol");
 		return a_symbol;}
	operator std::string () const {
		if (a_type!=A_SYMBOL) RAISE("expected symbol");
		return std::string(a_symbol->s_name);}
	operator void * () const {
		if (a_type!=A_POINTER) RAISE("expected pointer");
 		return a_gpointer;}
	operator t_binbuf * () const {
		if (a_type!=A_LIST) RAISE("expected nested list");
 		return (t_binbuf *)a_gpointer;}
};

template <class T> T convert(const t_atom &x, T *foo) {const t_atom2 *xx = (const t_atom2 *)&x; return (T)*xx;}

//****************************************************************

//template <class T> class P : T * {};
//a reference counting pointer class
template <class T> class P {
public:
#define INCR if (p) p->refcount++;
#define DECR if (p) {p->refcount--; if (!p->refcount) delete p;}
	T *p;
	P()               {p=0;}
	P(T *_p)          {p=_p  ; INCR;}
	P(const P<T> &_p) {p=_p.p; INCR;}
	P<T> &operator = (T *  _p) {DECR; p=_p;   INCR; return *this;}
	P<T> &operator = (P<T> _p) {DECR; p=_p.p; INCR; return *this;}
	bool operator == (P<T> _p) {return p==_p.p;}
	bool operator != (P<T> _p) {return p!=_p.p;}
	~P() {DECR;}
	bool operator !() {return  !p;}
	operator bool()   {return !!p;}
	T &operator *()   {return  *p;}
	T *operator ->()  {return   p;}
	operator T *() {return p;}
//#undef INCR
//#undef DECR
};

void gfmemcopy(uint8 *out, const uint8 *in, long n);
template <class T> inline void COPY(T *dest, T *src, long n) {
	gfmemcopy((uint8*)dest,(const uint8*)src,n*sizeof(T));
}
template <class T> inline void CLEAR(T *dest, long n) {
	memset(dest,0,n*sizeof(T));
}
template <class T> static void memswap (T *a, T *b, long n) {
	T c[n]; COPY(c,a,n); COPY(a,b,n); COPY(b,c,n);
}

//****************************************************************

struct CObject {
	int32 refcount;
	CObject() : refcount(0) {}
	virtual ~CObject() {}
	virtual void changed (t_symbol *s=0) {}
};

// you shouldn't use MethodDecl directly (used by source_filter.rb)
struct MethodDecl {const char *selector; FMethod method;};

#undef check

//****************************************************************
// a Dim is a list of dimensions that describe the shape of a grid
typedef int32 Card; /* should be switched to long int soon */
struct Dim : CObject {
	static const Card MAX_DIM=16; // maximum number of dimensions in a grid
	Card n;
	Card v[MAX_DIM]; // real stuff
	void check(); // test invariants
	Dim(Card n, Card *v){this->n=n; COPY(this->v,v,n); check();}
	Dim()                    {n=0;                     check();}
	Dim(Card a)              {n=1;v[0]=a;              check();}
	Dim(Card a,Card b)       {n=2;v[0]=a;v[1]=b;       check();}
	Dim(Card a,Card b,Card c){n=3;v[0]=a;v[1]=b;v[2]=c;check();}
	Dim(Dim *a, Dim *b, Dim *c=0) {
		n=a->n+b->n; if(c) n+=c->n;
		if (n>Dim::MAX_DIM) RAISE("too many dims");
		COPY(v     ,a->v,a->n);
		COPY(v+a->n,b->v,b->n);
		if(c) COPY(v+a->n+b->n,c->v,c->n);
	}
	Card count() {return n;}
	Card get(Card i) {return v[i];}
/*	Dim *range(Card i, Card j) {return new Dim(...);} */
	Card prod(Card start=0, Card end=-1) {
		if (end<0) end+=n;
		Card tot=1;
		for (Card i=start; i<=end; i++) tot *= v[i];
		return tot;
	}
	char *to_s();
	bool equal(P<Dim> o) {
		if (n!=o->n) return false;
		for (Card i=0; i<n; i++) if (v[i]!=o->v[i]) return false;
		return true;
	}
};

//****************************************************************
//NumberTypeE is a very small int identifying the type of the (smallest) elements of a grid

#define NT_UNSIGNED (1<<0)
#define NT_FLOAT    (1<<1)
#define NT_UNIMPL   (1<<2)
#define NUMBER_TYPE_LIMITS(T,a,b,c) \
	inline T nt_smallest(T *bogus) {return a;} \
	inline T nt_greatest(T *bogus) {return b;} \
	inline T nt_all_ones(T *bogus) {return c;}

NUMBER_TYPE_LIMITS(  uint8,0,255,255)
NUMBER_TYPE_LIMITS(  int16,-0x8000,0x7fff,-1)
NUMBER_TYPE_LIMITS(  int32,-0x80000000,0x7fffffff,-1)
NUMBER_TYPE_LIMITS(  int64,-0x8000000000000000LL,0x7fffffffffffffffLL,-1)
NUMBER_TYPE_LIMITS(float32,-HUGE_VAL,+HUGE_VAL,(RAISE("all_ones"),0))
NUMBER_TYPE_LIMITS(float64,-HUGE_VAL,+HUGE_VAL,(RAISE("all_ones"),0))

#ifdef HAVE_LITE
#define NT_NOTLITE NT_UNIMPL
#define NONLITE(x...)
#else
#define NT_NOTLITE 0
#define NONLITE(x...) x
#endif
#define NUMBER_TYPES(MACRO) \
	MACRO(uint8, 8,NT_UNSIGNED, "u8,b") \
	MACRO(int16,16,0,           "i16,s") \
	MACRO(int32,32,0,           "i32,i") \
	MACRO(int64,64,NT_NOTLITE,  "i64,l") \
	MACRO(float32,32,NT_FLOAT,  "f32,f") \
	MACRO(float64,64,NT_NOTLITE|NT_FLOAT, "f64,d")

enum NumberTypeE {
#define FOO(_sym_,args...) _sym_##_e,
NUMBER_TYPES(FOO)
#undef FOO
number_type_table_end
};

#define FOO(_type_) \
inline NumberTypeE NumberTypeE_type_of(_type_ *x) { \
	return _type_##_e; }
EACH_NUMBER_TYPE(FOO)
#undef FOO

struct NumberType : CObject {
	const char *name;
	int size;
	int flags;
	const char *aliases;
	NumberTypeE index;
	NumberType(const char *name_, int size_, int flags_, const char *aliases_) :
		name(name_), size(size_), flags(flags_), aliases(aliases_) {}
};

NumberTypeE NumberTypeE_find (string sym);
NumberTypeE NumberTypeE_find (const t_atom &sym);

#define TYPESWITCH(T,C,E) switch (T) { \
  case uint8_e:   C(uint8) break;         case int16_e: C(int16) break; \
  case int32_e:   C(int32) break;   NONLITE(case int64_e: C(int64) break;) \
  case float32_e: C(float32) break; NONLITE(case float64_e: C(float64) break;) \
  default: E; RAISE("type '%s' not available here",number_type_table[T].name);}
#define TYPESWITCH_JUSTINT(T,C,E) switch (T) { \
  case uint8_e: C(uint8) break; case int16_e: C(int16) break; \
  case int32_e: C(int32) break;   NONLITE(case int64_e: C(int64) break;) \
  default: E; RAISE("type '%s' not available here",number_type_table[T].name);}

//****************************************************************
//BitPacking objects encapsulate optimised loops of bitfield conversion (mostly for I/O)
struct BitPacking;
// those are the types of the optimised loops of conversion; inputs are const
struct Packer {
#define FOO(S) void (*as_##S)(BitPacking *self, long n, S *in, uint8 *out);
EACH_INT_TYPE(FOO)
#undef FOO
};
struct Unpacker {
#define FOO(S) void (*as_##S)(BitPacking *self, long n, uint8 *in, S *out);
EACH_INT_TYPE(FOO)
#undef FOO
};

struct BitPacking : CObject {
	Packer   *  packer;
	Unpacker *unpacker;
	unsigned int endian; // 0=big, 1=little, 2=same, 3=different
	int bytes;
	int size;
	uint32 mask[4];
	BitPacking(){::abort();} // don't call, but don't remove. sorry.
	BitPacking(int endian, int bytes, int size, uint32 *mask,
		Packer *packer=0, Unpacker *unpacker=0);
	bool is_le();
	bool eq(BitPacking *o);
// main entrances to Packers/Unpackers
	template <class T> void   pack(long n, T *in, uint8 *out);
	template <class T> void unpack(long n, uint8 *in, T *out);
};

int high_bit(uint32 n);
int  low_bit(uint32 n);
void swap16(long n, uint16 *data);
void swap32(long n, uint32 *data);
void swap64(long n, uint64 *data);
inline void swap_endian(long n,   uint8 *data) {}
inline void swap_endian(long n,   int16 *data) {swap16(n,(uint16 *)data);}
inline void swap_endian(long n,   int32 *data) {swap32(n,(uint32 *)data);}
inline void swap_endian(long n,   int64 *data) {swap64(n,(uint64 *)data);}
inline void swap_endian(long n, float32 *data) {swap32(n,(uint32 *)data);}
inline void swap_endian(long n, float64 *data) {swap64(n,(uint64 *)data);}

//****************************************************************
// Numop objects encapsulate optimised loops of simple operations

enum LeftRight { at_left, at_right };

template <class T>
struct NumopOn {
	// Function Vectorisations
	typedef void (*Map )(         long n, T *as, T  b ); Map  map;
	typedef void (*Zip )(         long n, T *as, T *bs); Zip  zip;
	typedef void (*Fold)(long an, long n, T *as, T *bs); Fold fold;
	typedef void (*Scan)(long an, long n, T *as, T *bs); Scan scan;
	// Algebraic Properties (those involving simply numeric types)
	typedef bool (*AlgebraicCheck)(T x, LeftRight side);
	// neutral: right: forall y {f(x,y)=x}; left: forall x {f(x,y)=y};
	// absorbent: right: exists a forall y {f(x,y)=a}; ...
	void (*neutral)(T *,LeftRight); // default neutral: e.g. 0 for addition, 1 for multiplication
	AlgebraicCheck is_neutral, is_absorbent;
	NumopOn(Map m, Zip z, Fold f, Scan s,
	void (*neu)(T *,LeftRight), AlgebraicCheck n, AlgebraicCheck a) :
		map(m), zip(z), fold(f), scan(s), neutral(neu), is_neutral(n), is_absorbent(a) {}
	NumopOn() : map(0),zip(0),fold(0),scan(0),neutral(0),is_neutral(0),is_absorbent(0) {}
	NumopOn(const NumopOn &z) {
		map=z.map; zip=z.zip; fold=z.fold; scan=z.scan;
		is_neutral = z.is_neutral; neutral = z.neutral;
		is_absorbent = z.is_absorbent; }
};

// semigroup property: associativity: f(a,f(b,c))=f(f(a,b),c)
#define OP_ASSOC (1<<0)
// abelian property: commutativity: f(a,b)=f(b,a)
#define OP_COMM (1<<1)

struct Numop {
	const char *name;
	t_symbol *sym;
	int flags;
	int size; // numop=1; vecop>1
#define FOO(T) NumopOn<T> on_##T; \
  NumopOn<T> *on(T &foo) { \
    if (!on_##T.map) RAISE("operator %s does not support type "#T,name); \
    return &on_##T;}
EACH_NUMBER_TYPE(FOO)
#undef FOO
	template <class T> inline void map(long n, T *as, T b) {
		on(*as)->map(n,(T *)as,b);}
	template <class T> inline void zip(long n, T *as, T *bs) {
		on(*as)->zip(n,(T *)as,(T *)bs);}
	template <class T> inline void fold(long an, long n, T *as, T *bs) {
		typename NumopOn<T>::Fold f = on(*as)->fold;
		if (!f) RAISE("operator %s does not support fold",name);
		f(an,n,(T *)as,(T *)bs);}
	template <class T> inline void scan(long an, long n, T *as, T *bs) {
		typename NumopOn<T>::Scan f = on(*as)->scan;
		if (!f) RAISE("operator %s does not support scan",name);
		f(an,n,(T *)as,(T *)bs);}

	Numop(const char *name_,
#define FOO(T) NumopOn<T> op_##T, 
EACH_NUMBER_TYPE(FOO)
#undef FOO
	int flags_, int size_) : name(name_), flags(flags_), size(size_) {
#define FOO(T) on_##T = op_##T;
EACH_NUMBER_TYPE(FOO)
#undef FOO
		sym=gensym((char *)name);
	}
};

extern NumberType number_type_table[];
extern std::map<string,NumberType *> number_type_dict;
extern std::map<string,Numop *> op_dict;
extern std::map<string,Numop *> vop_dict;

static inline NumberTypeE convert(const t_atom &x, NumberTypeE *bogus) {
	if (x.a_type!=A_SYMBOL) RAISE("expected number-type"); return NumberTypeE_find(string(x.a_symbol->s_name));}


static Numop *convert(const t_atom &x, Numop **bogus) {
	if (x.a_type!=A_SYMBOL) RAISE("expected numop (as symbol)");
	string k = string(x.a_symbol->s_name);
	if (op_dict.find(k)==op_dict.end()) {
		if (vop_dict.find(k)==vop_dict.end()) RAISE("expected two-input-operator, not '%s'", k.data());
		return vop_dict[k];
	} else return op_dict[k];
}

// ****************************************************************
struct Grid : CObject {
	P<Dim> dim;
	NumberTypeE nt;
	void *data;
	int state; /* 0:borrowing 1:owning -1:expired(TODO) */
	Grid(P<Dim> dim, NumberTypeE nt, bool clear=false) {
		state=1; 
		if (!dim) RAISE("hell");
		init(dim,nt);
		if (clear) {long size = bytes(); CLEAR((char *)data,size);}
	}
	Grid(const t_atom &x) {state=1; init_from_atom(x);}
	Grid(int n, t_atom *a, NumberTypeE nt_=int32_e) {state=1; init_from_list(n,a,nt_);}
	template <class T> Grid(P<Dim> dim, T *data) {
		state=0; this->dim=dim;
		this->nt=NumberTypeE_type_of((T *)0);
		this->data = data;
	}
	// parens are necessary to prevent overflow at 1/16th of the word size (256 megs)
	long bytes() { return dim->prod()*(number_type_table[nt].size/8); }
	P<Dim> to_dim () { return new Dim(dim->prod(),(int32 *)*this); }
#define FOO(T) operator T *() { return (T *)data; }
EACH_NUMBER_TYPE(FOO)
#undef FOO
	Grid *dup () { /* always produce an owning grid even if from a borrowing grid */
		Grid *foo=new Grid(dim,nt);
		memcpy(foo->data,data,bytes());
		return foo;
	}
	~Grid() {if (state==1 && data) free(data);}
private:
	void init(P<Dim> dim, NumberTypeE nt) {
		this->dim = dim;
		this->nt = nt;
		data = 0;
		if (dim) {
			data = memalign(16,bytes()+16);
			if (!data) RAISE("out of memory (?) trying to get %ld bytes",bytes());
		}
		//fprintf(stderr,"rdata=%p data=%p align=%d\n",rdata,data,align);
	}
	void init_from_atom(const t_atom &x);
	void init_from_list(     int n, t_atom *a, NumberTypeE nt=int32_e);
};
static inline Grid *convert (const t_atom &r, Grid **bogus) {return new Grid(r);}

// DimConstraint interface:
// return if d is acceptable
// else RAISE with proper descriptive message
typedef void (*DimConstraint)(P<Dim> d);

struct PtrGrid : public P<Grid> {
	DimConstraint dc;
	void constrain(DimConstraint dc_) { dc=dc_; }
	P<Grid> next;
	PtrGrid()                  : P<Grid>(), dc(0), next(0) {}
	PtrGrid(const PtrGrid &_p) : P<Grid>(), dc(0), next(0) {dc=_p.dc; p=_p.p; INCR;}
//	PtrGrid(       P<Grid> _p) : P<Grid>(), dc(0), next(0) {dc=_p.dc; p=_p.p; INCR;}
	PtrGrid(         Grid *_p) : P<Grid>(), dc(0), next(0) {
          p=_p;
INCR;}
	PtrGrid &operator =(  Grid *_p) {if(dc&&_p)dc(_p->dim);
DECR; p=_p; INCR;
return *this;}
	PtrGrid &operator =(P<Grid> _p) {if(dc&&_p)dc(_p->dim); DECR; p=_p.p; INCR; return *this;}
	PtrGrid &operator =(PtrGrid _p) {if(dc&&_p)dc(_p->dim); DECR; p=_p.p; INCR; return *this;}
};
#undef INCR
#undef DECR

static inline P<Dim> convert(const t_atom &x, P<Dim> *foo) {
	Grid *d = convert(x,(Grid **)0);
	if (!d) RAISE("urgh");
	if (d->dim->n!=1) RAISE("dimension list must have only one dimension itself");
	return new Dim(d->dim->v[0],(int32 *)(d->data));
}
static inline PtrGrid convert(const t_atom &x, PtrGrid *foo) {return PtrGrid(convert(x,(Grid **)0));}

//****************************************************************
// GridInlet represents a grid-aware inlet

// four-part macro for defining the behaviour of a gridinlet in a class
// C:Class I:Inlet
#define GRID_INLET(C,I) \
	template <class T> void C::grinw_##I (GridInlet *in, long n, T *data) { \
		((C*)(in->parent))->grin_##I(in,n,data); } \
	template <class T> void  C::grin_##I (GridInlet *in, long n, T *data) { \
	if (n==-1)
#define GRID_ALLOC  else if (n==-3)
#define GRID_FLOW   else if (n>=0)
#define GRID_FINISH else if (n==-2)
#define GRID_END }

/* macro for defining a gridinlet's behaviour as just storage (no backstore) */
// V is a PtrGrid instance-var
#define GRID_INPUT(C,I,V) \
GRID_INLET(C,I) { V=new Grid(in->dim,NumberTypeE_type_of(data)); } \
GRID_FLOW { COPY((T *)*(V)+in->dex, data, n); } GRID_FINISH

// macro for defining a gridinlet's behaviour as just storage (with backstore)
// V is a PtrGrid instance-var
#define GRID_INPUT2(C,I,V) \
	GRID_INLET(C,I) { \
		if (is_busy_except(in)) { \
			V.next = new Grid(in->dim,NumberTypeE_type_of(data)); \
		} else V=        new Grid(in->dim,NumberTypeE_type_of(data)); \
	} GRID_FLOW { \
		COPY(((T *)*(V.next?V.next.p:&*V.p))+in->dex, data, n); \
	} GRID_FINISH

typedef struct GridInlet GridInlet;
typedef struct GridHandler {
#define FOO(T) \
	void (*flow_##T)(GridInlet *in, long n, T *data); \
	void flow(GridInlet *in, long n, T *data) const {flow_##T(in,n,data);}
EACH_NUMBER_TYPE(FOO)
#undef FOO
} GridHandler;

struct FObject;
struct GridInlet : CObject {
	FObject *parent;
	const GridHandler *gh;
private:
	FObject *sender;
public:
	P<Dim> dim;
	NumberTypeE nt;
	long dex;
private:
	PtrGrid buf;// factor-chunk buffer
	long bufi;   // buffer index: how much of buf is filled
public:
	int mode; // 0=ignore; 4=ro; 6=rw
//	long allocfactor,allocmin,allocmax,allocn;
//	uint8 *alloc;

	GridInlet(FObject *parent_, const GridHandler *gh_) :
		parent(parent_), gh(gh_), sender(0),
		dim(0), nt(int32_e), dex(0), bufi(0), mode(4) {}
	~GridInlet() {}
	void set_factor(long factor); // obsolete
	void set_chunk(long whichdim);
	void set_mode(int mode_) { mode=mode_; }
	int32 factor() {return buf?buf->dim->prod():1;}
	void begin(int argc, t_atom2 *argv);
	void finish(); /* i thought this should be private but maybe it shouldn't. */

	// n=-1 is begin, and n=-2 is finish; GF-0.9 may have n=-3 meaning alloc (?).
	template <class T> void flow(int mode, long n, T *data);
	void from_list(VA, NumberTypeE nt=int32_e) {Grid t(argc,argv,nt); from_grid(&t);}
	void from_atom(VA) {Grid t(argv[0]); from_grid(&t);}
	void from_grid(Grid *g);
	bool supports_type(NumberTypeE nt);
private:
	template <class T> void from_grid2(Grid *g, T foo);
};

//****************************************************************
// for use by source_filter.rb ONLY (for \grin)
#ifndef HAVE_LITE
#define GRIN(TB,TS,TI,TL,TF,TD) {TB,TS,TI,TL,TF,TD}
#else
#define GRIN(TB,TS,TI,TL,TF,TD) {TB,TS,TI,TF}
#endif // HAVE_LITE
#define MESSAGE t_symbol *sel, int argc, t_atom2 *argv
#define MESSAGE2 sel,argc,argv
#define MESSAGE3 t_symbol *, int, t_atom2 *
struct AttrDecl {
	string name;
	string type;
	AttrDecl(string name_, string type_) {name=name_; type=type_;}
};
struct BFObject;
typedef FObject *(*t_allocator)(BFObject *,MESSAGE3);
struct FClass {
	t_allocator allocator; // returns a new C++ object
	void (*startup)(FClass *);
	const char *cname; // C++ name (not PD name)
	int methodsn; MethodDecl *methods;
	FClass *super;
	int ninlets;
	int noutlets;
	t_class *bfclass;
	string name;
	size_t bytes;
	std::map<string,AttrDecl *> attrs;
};

void fclass_install(FClass *fc, FClass *super, size_t bytes);

//****************************************************************
// GridOutlet represents a grid-aware outlet
struct GridOutlet : CObject {
// number of (minimum,maximum) BYTES to send at once
// starting with version 0.8, this is amount of BYTES, not amount of NUMBERS.
	static const long MIN_PACKET_SIZE = 1<<8;
	static const long MAX_PACKET_SIZE = 1<<12;
// those are set only once
	FObject *parent; // not a P<> because of circular refs
	P<Dim> dim; // dimensions of the grid being sent
	NumberTypeE nt;
	std::vector<GridInlet *> inlets; // which inlets are we connected to
// those are updated during transmission
	long dex;  // how many numbers were already sent in this connection

	GridOutlet(FObject *parent_, int woutlet, P<Dim> dim_, NumberTypeE nt_=int32_e);
	~GridOutlet() {}
	void callback(GridInlet *in);

	// send/send_direct: data belongs to caller, may be stack-allocated,
	// receiver doesn't modify the data; in send(), there is buffering;
	// in send_direct(), there is not. When switching from buffered to
	// unbuffered mode, flush() must be called
	template <class T> void send(long n, T *data);
	void flush(); // goes with send();

	// give: data must be dynamically allocated as a whole: the data
	// will be deleted eventually, and should not be used by the caller
	// beyond the call to give().
	template <class T> void give(long n, T *data);

	// third way to send (upcoming, in GF-1.1.??) is called "ask".
	template <class T> void ask(int &n, T *&data, long factor, long min, long max);

private:
	bool frozen; // is the "begin" phase finished?
	PtrGrid buf; // temporary buffer
	long bufi; // number of bytes used in the buffer
	void begin(int woutlet, P<Dim> dim, NumberTypeE nt=int32_e);
	template <class T> void send_direct(long n, T *data);
	void finish() {
		flush();
		for (uint32 i=0; i<inlets.size(); i++) inlets[i]->finish();
		dim=0;
	}
};

//****************************************************************

#define CHECK_GRIN(class,i) \
	if (in.size()<=i) in.resize(i+1); \
	if (!in[i]) in[i]=new GridInlet((FObject *)this,&class##_grid_##i##_hand);

#define CHECK_ALIGN16(d,nt) \
	{int bytes = 16; \
	int align = ((ulong)(void*)d)%bytes; \
	if (align) {_L_;post("%s(%s): Alignment Warning: %s=%p is not %d-aligned: %d", \
		ARGS(this), __PRETTY_FUNCTION__,#d,(void*)d,bytes,align);}}

struct BFProxy;
struct BFObject : t_object {
	void *gemself; // this absolutely needs to be the first field, to make GEM happy
	FObject *self;
	int ninlets,noutlets; // per object settings (not class)
	BFProxy  **inlets;    // direct access to  inlets (not linked lists)
	t_outlet **outlets;   // direct access to outlets (not linked lists)
	t_canvas *mom;
	void  ninlets_set(int n, bool draw=true);
	void noutlets_set(int n, bool draw=true);
};

// represents objects that have inlets/outlets
\class FObject : CObject {
	BFObject *bself; // point to PD peer
	std::vector<P<GridInlet> > in;
	P<GridOutlet> out;
	FObject(BFObject *bself, MESSAGE) : bself(bself) {bself->self = this;}
	template <class T> void send_out(int outlet, int argc, T *argv) {
		t_atom foo[argc];
		for (int i=0; i<argc; i++) SETFLOAT(&foo[i],argv[i]);
		outlet_list(bself->outlets[outlet],&s_list,argc,foo);
	}
	bool is_busy_except(P<GridInlet> gin) {
		for (uint32 i=0; i<in.size(); i++) if (in[i] && in[i]!=gin && in[i]->dim) return true;
		return false;
	}
	\decl 0 get (t_symbol *s=0);
	\decl 0 help ();
};
\end class

uint64 gf_timeofday();
extern "C" void Init_gridflow ();
extern Numop *op_add,*op_sub,*op_mul,*op_div,*op_mod,*op_shl,*op_and,*op_put;

#define ARGS(OBJ) "(some object...)"
#define NOTEMPTY(_a_) if (!(_a_)) RAISE("in [%s], '%s' is empty",ARGS(this), #_a_);
#define SAME_TYPE(_a_,_b_) \
	if ((_a_)->nt != (_b_)->nt) RAISE("%s: same type please (%s has %s; %s has %s)", ARGS(this), \
		#_a_, number_type_table[(_a_)->nt].name, \
		#_b_, number_type_table[(_b_)->nt].name);
static void SAME_DIM(int n, P<Dim> a, int ai, P<Dim> b, int bi) {
	if (ai+n > a->n) RAISE("left hand: not enough dimensions");
	if (bi+n > b->n) RAISE("right hand: not enough dimensions");
	for (int i=0; i<n; i++) {
		if (a->v[ai+i] != b->v[bi+i]) {
			RAISE("mismatch: left dim #%d is %d, right dim #%d is %d",
				ai+i, a->v[ai+i],
				bi+i, b->v[bi+i]);}}}

void suffixes_are (const char *name, const char *suffixes);
#define install(name,inlets,outlets) install2(fclass,name,inlets,outlets)
void install2(FClass *fclass, const char *name, int inlets, int outlets);
#define add_creator(name) add_creator2(fclass,name)
void add_creator2(FClass *fclass, const char *name);
#define install_format(name,mode,suffixes) do {install(name,1,1); suffixes_are(name,suffixes);} while(0)
void call_super(int argc, t_atom *argv);
#define SUPER call_super(argc,argv);

\class Format : FObject {
	int mode;
	int fd;
	FILE *f;
	NumberTypeE cast;
	long frame;
	Format(BFObject *, MESSAGE);
	\decl 0 open (t_symbol *mode, string filename);
	\decl 0 close ();
	\decl 0 cast (NumberTypeE nt);
	\decl 0 seek(int frame);
	\decl 0 rewind ();
	~Format ();
};

extern std::vector<string> gf_data_path;
string gf_find_file (string x);
void pd_oprintf (std::ostream &o, const char *s, int argc, t_atom *argv);
void pd_oprint (std::ostream &o, int argc, t_atom *argv);
void pd_post (const char *s, int argc, t_atom *argv);

inline void set_atom (t_atom *a, uint8     v) {SETFLOAT(a,(float)v);}
inline void set_atom (t_atom *a, int16     v) {SETFLOAT(a,(float)v);}
inline void set_atom (t_atom *a, int32     v) {SETFLOAT(a,(float)v);}
inline void set_atom (t_atom *a, uint32    v) {SETFLOAT(a,(float)v);}
inline void set_atom (t_atom *a, long      v) {SETFLOAT(a,(float)v);}
inline void set_atom (t_atom *a, float32   v) {SETFLOAT(a,v);}
inline void set_atom (t_atom *a, float64   v) {SETFLOAT(a,v);}
inline void set_atom (t_atom *a, t_symbol *v) {SETSYMBOL(a,v);}
inline void set_atom (t_atom *a, Numop    *v) {SETSYMBOL(a,v->sym);}
inline void set_atom (t_atom *a, t_binbuf *v) {SETLIST(a,v);}

extern std::map<string,FClass *> fclasses;
int handle_braces(int ac, t_atom *av);

extern FClass ciFObject, ciFormat;

#endif // __GF_GRID_H
