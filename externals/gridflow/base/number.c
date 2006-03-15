/*
	$Id: number.c,v 1.2 2006-03-15 04:37:08 matju Exp $

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
#include <limits.h>

#ifdef PASS1
NumberType number_type_table[] = {
#define FOO(_sym_,_size_,_flags_,args...) NumberType( #_sym_, _size_, _flags_, args ),
NUMBER_TYPES(FOO)
#undef FOO
};
const long number_type_table_n = COUNT(number_type_table);
#endif

// those are bogus class-templates in the sense that you don't create
// objects from those, you just call static functions. The same kind
// of pattern is present in STL to overcome some limitations of C++.

template <class T> class Op {
public:
	// I call abort() on those because I can't say they're purevirtual.
	static T f(T a, T b) {abort();}
	static bool is_neutral(T x, LeftRight side)   {assert(!"Op::is_neutral called?");   return false;}
	static bool is_absorbent(T x, LeftRight side) {assert(!"Op::is_absorbent called?"); return false;}
};

template <class O> class OpLoops {
public:
	template <class T> static void op_map (int n, T *as, T b) {
		if (!n) return;
#define FOO(I) as[I]=O::f(as[I],b);
	UNROLL_8(FOO,n,as)
#undef FOO
	}
	template <class T> static void op_zip (int n, T *as, T *bs) {
		if (!n) return;
		ptrdiff_t ba=bs-as; // really!
#define FOO(I) as[I]=O::f(as[I],as[ba+I]);
	UNROLL_8(FOO,n,as)
#undef FOO
	}
	// disabled
	template <class T> static void op_zip2 (int n, T *as, T *bs, T *cs) {
		if (!n) return;
		ptrdiff_t ba=bs-as, ca=cs-as;
#define FOO(I) as[ca+I]=O::f(as[I],as[ba+I]);
	UNROLL_8(FOO,n,as)
#undef FOO
	}
#define W(i) as[i]=O::f(as[i],bs[i]);
#define Z(i,j) as[i]=O::f(O::f(O::f(O::f(as[i],bs[i]),bs[i+j]),bs[i+j+j]),bs[i+j+j+j]);
	template <class T> static void op_fold (int an, int n, T *as, T *bs) {
		switch (an) {
		case 1: for (; (n&3)!=0; bs++, n--) W(0);
			for (; n; bs+=4, n-=4) { Z(0,1); } break;
		case 2: for (; (n&3)!=0; bs+=2, n--) { W(0); W(1); }
			for (; n; bs+=8, n-=4) { Z(0,2); Z(1,2); } break;
		case 3: for (; (n&3)!=0; bs+=3, n--) { W(0); W(1); W(2); }
			for (; n; bs+=12, n-=4) { Z(0,3); Z(1,3); Z(2,3); } break;
		case 4: for (; (n&3)!=0; bs+=4, n--) { W(0); W(1); W(2); W(3); }
			for (; n; bs+=16, n-=4) { Z(0,4); Z(1,4); Z(2,4); Z(3,4); } break;
		default:for (; n--; ) {
				int i=0;
				for (; i<(an&-4); i+=4, bs+=4) {
					as[i+0]=O::f(as[i+0],bs[0]);
					as[i+1]=O::f(as[i+1],bs[1]);
					as[i+2]=O::f(as[i+2],bs[2]);
					as[i+3]=O::f(as[i+3],bs[3]);
				}
				for (; i<an; i++, bs++) as[i] = O::f(as[i],*bs);
			}
		}
	}
	template <class T> static void op_scan (int an, int n, T *as, T *bs) {
		for (; n--; as=bs-an) {
			for (int i=0; i<an; i++, as++, bs++) *bs=O::f(*as,*bs);
		}
	}
};

template <class T>
static void quick_mod_map (int n, T *as, T b) {
	if (!b) return;
#define FOO(I) as[I]=mod(as[I],b);
	UNROLL_8(FOO,n,as)
#undef FOO
}

template <class T> static void quick_ign_map (int n, T *as, T b) {}
template <class T> static void quick_ign_zip (int n, T *as, T *bs) {}
template <class T> static void quick_put_map (int n, T *as, T b) {
#define FOO(I) as[I]=b;
	UNROLL_8(FOO,n,as)
#undef FOO
}

#ifdef PASS1
void quick_put_map (int n, int16 *as, int16 b) {
	if (n&1!=0 && (long)as&4!=0) { *as++=b; n--; }
	quick_put_map (n>>1, (int32 *)as, (int32)(b<<16)+b);
	if (n&1!=0) *as++=b;
}
void quick_put_map (int n, uint8 *as, uint8 b) {
	while (n&3!=0 && (long)as&4!=0) { *as++=b; n--; }
	int32 c=(b<<8)+b; c+=c<<16;
	quick_put_map (n>>2, (int32 *)as, c);
	while (n&3!=0) *as++=b;
}
#endif
template <class T> static void quick_put_zip (int n, T *as, T *bs) {
	gfmemcopy((uint8 *)as, (uint8 *)bs, n*sizeof(T));
}

// classic two-input operator
#define DEF_OP(op,expr,neu,isneu,isorb) \
	template <class T> class Y##op : Op<T> { public: \
		inline static T f(T a, T b) { return expr; } \
		inline static T neutral (LeftRight side) {return neu;} \
		inline static bool is_neutral  (T x, LeftRight side) {return isneu;} \
		inline static bool is_absorbent(T x, LeftRight side) {return isorb;}};
#define DEF_OPFT(op,expr,neu,isneu,isorb,T) \
	template <> class Y##op<T> : Op<T> { public: \
		inline static T f(T a, T b) { return expr; } \
		inline static T neutral (LeftRight side) {return neu;} \
		inline static bool is_neutral  (T x, LeftRight side) {return isneu;} \
		inline static bool is_absorbent(T x, LeftRight side) {return isorb;}};
// this macro is for operators that have different code for the float version
#define DEF_OPF(op,expr,expr2,neu,isneu,isorb) \
	DEF_OP( op,expr,      neu,isneu,isorb) \
	DEF_OPFT(op,expr2,neu,isneu,isorb,float32) \
	DEF_OPFT(op,expr2,neu,isneu,isorb,float64)

#define DECL_OPON(base,op,T) NumopOn<T>( \
	&base<Y##op<T> >::op_map, &base<Y##op<T> >::op_zip, \
	&base<Y##op<T> >::op_fold, &base<Y##op<T> >::op_scan, \
	&Y##op<T>::neutral, &Y##op<T>::is_neutral, &Y##op<T>::is_absorbent)
#define DECL_OPON_NOFOLD(base,op,T) NumopOn<T>( \
	&base<Y##op<T> >::op_map, &base<Y##op<T> >::op_zip, 0,0, \
	&Y##op<T>::neutral, &Y##op<T>::is_neutral, &Y##op<T>::is_absorbent)
#define DECL_OP(op,sym,flags) Numop(0, sym, \
	DECL_OPON(OpLoops,op,uint8), DECL_OPON(OpLoops,op,int16), \
	DECL_OPON(OpLoops,op,int32) NONLITE(, DECL_OPON(OpLoops,op,int64), \
	DECL_OPON(OpLoops,op,float32), DECL_OPON(OpLoops,op,float64), \
	DECL_OPON(OpLoops,op,ruby)), flags)
#define DECL_OP_NOFLOAT(op,sym,flags) Numop(0, sym, \
	DECL_OPON(OpLoops,op,uint8), DECL_OPON(OpLoops,op,int16), \
	DECL_OPON(OpLoops,op,int32) NONLITE(, DECL_OPON(OpLoops,op,int64), \
	NumopOn<float32>(0,0,0,0,0,0,0), NumopOn<float64>(0,0,0,0,0,0,0), \
	DECL_OPON(OpLoops,op,ruby)), flags)
#define DECL_OP_NOFOLD(op,sym,flags) Numop(0, sym, \
	DECL_OPON_NOFOLD(OpLoops,op,uint8), DECL_OPON_NOFOLD(OpLoops,op,int16), \
	DECL_OPON_NOFOLD(OpLoops,op,int32) NONLITE(, DECL_OPON_NOFOLD(OpLoops,op,int64), \
	DECL_OPON_NOFOLD(OpLoops,op,float32), DECL_OPON_NOFOLD(OpLoops,op,float64), \
	DECL_OPON_NOFOLD(OpLoops,op,ruby)), flags)

template <class T> static inline T gf_floor (T a) {
	return (T) floor((double)a); }
template <class T> static inline T gf_trunc (T a) {
	return (T) floor(abs((double)a)) * (a<0?-1:1); }

// trying to avoid GCC warning about uint8 too small for ==256
template <class T> static bool equal256 (T     x) {return x==256;}
template <>        static bool equal256 (uint8 x) {return false;}

#ifdef PASS1
DEF_OP(ignore, a, 0, side==at_right, side==at_left)
DEF_OP(put,    b, 0, side==at_left, side==at_right)
DEF_OP(add,  a+b, 0, x==0, false)
DEF_OP(sub,  a-b, 0, side==at_right && x==0, false)
DEF_OP(bus,  b-a, 0, side==at_left  && x==0, false)
DEF_OP(mul,  a*b, 1, x==1, x==0)
DEF_OP(mulshr8, (a*b)>>8, 256, equal256(x), x==0)
DEF_OP(div,  b==0 ? (T)0 :      a/b , 1, side==at_right && x==1, false)
DEF_OP(div2, b==0 ?    0 : div2(a,b), 1, side==at_right && x==1, false)
DEF_OP(vid,  a==0 ? (T)0 :      b/a , 1, side==at_left  && x==1, false)
DEF_OP(vid2, a==0 ?    0 : div2(b,a), 1, side==at_left  && x==1, false)
DEF_OPF(mod, b==0 ? 0 : mod(a,b), b==0 ? 0 : a-b*gf_floor(a/b), 0, false, side==at_left && x==0 || side==at_right && x==1)
DEF_OPF(dom, a==0 ? 0 : mod(b,a), a==0 ? 0 : b-a*gf_floor(b/a), 0, false, side==at_left && x==0 || side==at_right && x==1)
//DEF_OPF(rem, b==0 ? 0 : a%b, b==0 ? 0 : a-b*gf_trunc(a/b))
//DEF_OPF(mer, a==0 ? 0 : b%a, a==0 ? 0 : b-a*gf_trunc(b/a))
DEF_OP(rem, b==0?(T)0:a%b, 0, false, side==at_left&&x==0 || side==at_right&&x==1)
DEF_OP(mer, a==0?(T)0:b%a, 0, false, side==at_left&&x==0 || side==at_right&&x==1)
#endif
#ifdef PASS2
DEF_OP(gcd,   gcd(a,b), 0, x==0, x==1)
DEF_OP(gcd2, gcd2(a,b), 0, x==0, x==1) // should test those and pick one of the two
DEF_OP(lcm, a==0 || b==0 ? (T)0 : lcm(a,b), 1, x==1, x==0)
DEF_OPF(or , a|b, (float32)((int32)a | (int32)b), 0, x==0, x==nt_all_ones(&x))
DEF_OPF(xor, a^b, (float32)((int32)a ^ (int32)b), 0, x==0, false)
DEF_OPF(and, a&b, (float32)((int32)a & (int32)b), -1 /*nt_all_ones((T*)0)*/, x==nt_all_ones(&x), x==0)
DEF_OPF(shl, a<<b, a*pow(2.0,+b), 0, side==at_right && x==0, false)
DEF_OPF(shr, a>>b, a*pow(2.0,-b), 0, side==at_right && x==0, false)
DEF_OP(sc_and, a ? b : a, 1, side==at_left && x!=0, side==at_left && x==0)
DEF_OP(sc_or,  a ? a : b, 0, side==at_left && x==0, side==at_left && x!=0)
DEF_OP(min, min(a,b), nt_greatest((T*)0), x==nt_greatest(&x), x==nt_smallest(&x))
DEF_OP(max, max(a,b), nt_smallest((T*)0), x==nt_smallest(&x), x==nt_greatest(&x))
DEF_OP(cmp, cmp(a,b), 0, false, false)
DEF_OP(eq,  a == b, 0, false, false)
DEF_OP(ne,  a != b, 0, false, false)
DEF_OP(gt,  a >  b, 0, false, side==at_left&&x==nt_smallest(&x)||side==at_right&&x==nt_greatest(&x))
DEF_OP(le,  a <= b, 0, false, side==at_left&&x==nt_smallest(&x)||side==at_right&&x==nt_greatest(&x))
DEF_OP(lt,  a <  b, 0, false, side==at_left&&x==nt_greatest(&x)||side==at_right&&x==nt_smallest(&x))
DEF_OP(ge,  a >= b, 0, false, side==at_left&&x==nt_greatest(&x)||side==at_right&&x==nt_smallest(&x))
#endif
#ifdef PASS3
DEF_OP(sin, (T)((float64)b * sin((float64)a * (M_PI / 18000))), 0, false, false) // "LN=9000+36000n RA=0 LA=..."
DEF_OP(cos, (T)((float64)b * cos((float64)a * (M_PI / 18000))), 0, false, false) // "LN=36000n RA=0 LA=..."
DEF_OP(atan, (T)(atan2(a,b) * (18000 / M_PI)), 0, false, false) // "LA=0"
DEF_OP(tanh, (T)((float64)b * tanh((float64)a * (M_PI / 18000))), 0, false, x==0)
DEF_OP(gamma, b<=0 ? (T)0 : (T)(0+floor(pow((float64)a/256.0,256.0/(float64)b)*256.0)), 0, false, false) // "RN=256"
DEF_OP(pow, ipow(a,b), 0, false, false) // "RN=1"
DEF_OP(log, (T)(a==0 ? (T)0 : (T)((float64)b * log((float64)gf_abs(a)))), 0, false, false) // "RA=0"
// 0.8
DEF_OPF(clipadd, clipadd(a,b), a+b, 0, x==0, false)
DEF_OPF(clipsub, clipsub(a,b), a-b, 0, side==at_right && x==0, false)
DEF_OP(abssub,  gf_abs(a-b), 0, false, false)
DEF_OP(sqsub,   (a-b)*(a-b), 0, false, false)
DEF_OP(avg,         (a+b)/2, 0, false, false)
DEF_OP(hypot, (T)(0+floor(sqrt(a*a+b*b))), 0, false, false)
DEF_OP(sqrt,  (T)(0+floor(sqrt(a))),       0, false, false)
DEF_OP(rand, a==0 ? (T)0 : (T)(random()%(int32)a), 0, false, false)
//DEF_OP(erf,"erf*", 0)
#endif

extern Numop      op_table1[], op_table2[], op_table3[];
extern const long op_table1_n, op_table2_n, op_table3_n;

#ifdef PASS1
Numop op_table1[] = {
	DECL_OP(ignore, "ignore", OP_ASSOC),
	DECL_OP(put, "put", OP_ASSOC),
	DECL_OP(add, "+", OP_ASSOC|OP_COMM), // "LINV=sub"
	DECL_OP(sub, "-", 0),
	DECL_OP(bus, "inv+", 0),
	DECL_OP(mul, "*", OP_ASSOC|OP_COMM),
	DECL_OP_NOFLOAT(mulshr8, "*>>8", OP_ASSOC|OP_COMM),
	DECL_OP(div, "/", 0),
	DECL_OP_NOFLOAT(div2, "div", 0),
	DECL_OP(vid, "inv*", 0),
	DECL_OP_NOFLOAT(vid2, "swapdiv", 0),
	DECL_OP_NOFLOAT(mod, "%", 0),
	DECL_OP_NOFLOAT(dom, "swap%", 0),
	DECL_OP_NOFLOAT(rem, "rem", 0),
	DECL_OP_NOFLOAT(mer, "swaprem", 0),
};
const long op_table1_n = COUNT(op_table1);
#endif
#ifdef PASS2
Numop op_table2[] = {
	DECL_OP_NOFLOAT(gcd, "gcd", OP_ASSOC|OP_COMM),
	DECL_OP_NOFLOAT(gcd2, "gcd2", OP_ASSOC|OP_COMM),
	DECL_OP_NOFLOAT(lcm, "lcm", OP_ASSOC|OP_COMM),
	DECL_OP(or , "|", OP_ASSOC|OP_COMM),
	DECL_OP(xor, "^", OP_ASSOC|OP_COMM),
	DECL_OP(and, "&", OP_ASSOC|OP_COMM),
	DECL_OP_NOFOLD(shl, "<<", 0),
	DECL_OP_NOFOLD(shr, ">>", 0),
	DECL_OP_NOFOLD(sc_and,"&&", 0),
	DECL_OP_NOFOLD(sc_or, "||", 0),
	DECL_OP(min, "min", OP_ASSOC|OP_COMM),
	DECL_OP(max, "max", OP_ASSOC|OP_COMM),
	DECL_OP_NOFOLD(eq,   "==", OP_COMM),
	DECL_OP_NOFOLD(ne,   "!=", OP_COMM),
	DECL_OP_NOFOLD(gt,   ">", 0),
	DECL_OP_NOFOLD(le,   "<=", 0),
	DECL_OP_NOFOLD(lt,   "<", 0),
	DECL_OP_NOFOLD(ge,   ">=", 0),
	DECL_OP_NOFOLD(cmp,  "cmp", 0),
};
const long op_table2_n = COUNT(op_table2);
#endif
#ifdef PASS3
uint8 clipadd(uint8 a, uint8 b) { int32 c=a+b; return c<0?0:c>255?255:c; }
int16 clipadd(int16 a, int16 b) { int32 c=a+b; return c<-0x8000?-0x8000:c>0x7fff?0x7fff:c; }
int32 clipadd(int32 a, int32 b) { int64 c=a+b; return c<-0x80000000?-0x80000000:c>0x7fffffff?0x7fffffff:c; }
int64 clipadd(int64 a, int64 b) { int64 c=(a>>1)+(b>>1)+(a&b&1), p=nt_smallest((int64 *)0), q=nt_greatest((int64 *)0);
	return c<p/2?p:c>q/2?q:a+b; }
uint8 clipsub(uint8 a, uint8 b) { int32 c=a-b; return c<0?0:c>255?255:c; }
int16 clipsub(int16 a, int16 b) { int32 c=a-b; return c<-0x8000?-0x8000:c>0x7fff?0x7fff:c; }
int32 clipsub(int32 a, int32 b) { int64 c=a-b; return c<-0x80000000?-0x80000000:c>0x7fffffff?0x7fffffff:c; }
int64 clipsub(int64 a, int64 b) { int64 c=(a>>1)-(b>>1); //???
	int64 p=nt_smallest((int64 *)0), q=nt_greatest((int64 *)0);
	return c<p/2?p:c>q/2?q:a-b; }

ruby clipadd(ruby a, ruby b) { return a+b; }
ruby clipsub(ruby a, ruby b) { return a-b; }

Numop op_table3[] = {
	DECL_OP_NOFOLD(sin,  "sin*", 0),
	DECL_OP_NOFOLD(cos,  "cos*", 0),
	DECL_OP_NOFOLD(atan, "atan", 0),
	DECL_OP_NOFOLD(tanh, "tanh*", 0),
	DECL_OP_NOFOLD(gamma, "gamma", 0),
	DECL_OP_NOFOLD(pow, "**", 0),
	DECL_OP_NOFOLD(log, "log*", 0),
// 0.8
	DECL_OP(clipadd,"clip+", OP_ASSOC|OP_COMM),
	DECL_OP(clipsub,"clip-", 0),
	DECL_OP_NOFOLD(abssub,"abs-", OP_COMM),
	DECL_OP_NOFOLD(sqsub,"sq-", OP_COMM),
	DECL_OP_NOFOLD(avg,"avg", OP_COMM),
	DECL_OP_NOFOLD(hypot,"hypot", OP_COMM),
	DECL_OP_NOFOLD(sqrt,"sqrt", 0),
	DECL_OP_NOFOLD(rand,"rand", 0),
	//DECL_OP_NOFOLD(erf,"erf*", 0),
};
const long op_table3_n = COUNT(op_table3);
#endif

// D=dictionary, A=table, A##_n=table count.
#define INIT_TABLE(D,A) { D=IEVAL(mGridFlow,"@"#D" ||= {}"); \
	for(int i=0; i<A##_n; i++) { \
		A[i].sym = ID2SYM(rb_intern(A[i].name)); \
		rb_hash_aset(D,A[i].sym,PTR2FIX((A+i)));}}

#ifdef PASS1
Ruby op_dict = Qnil;
Ruby number_type_dict = Qnil;
void startup_number () {
	INIT_TABLE(op_dict,op_table1)
	INIT_TABLE(op_dict,op_table2)
	INIT_TABLE(op_dict,op_table3)
	INIT_TABLE(number_type_dict,number_type_table)

	for (int i=0; i<COUNT(number_type_table); i++) {
		number_type_table[i].index = (NumberTypeE) i;
		char a[64];
		strcpy(a,number_type_table[i].aliases);
		char *b = strchr(a,',');
		if (b) {
			*b=0;
			rb_hash_aset(number_type_dict, ID2SYM(rb_intern(b+1)),
				PTR2FIX(&number_type_table[i]));
		}
		rb_hash_aset(number_type_dict, ID2SYM(rb_intern(a)),
				PTR2FIX(&number_type_table[i]));
	}
// S:name; M:mode; F:replacement function;
#define OVERRIDE_INT(S,M,F) { \
	Numop *foo = FIX2PTR(Numop,rb_hash_aref(op_dict,SYM(S))); \
	foo->on_uint8.op_##M=F; \
	foo->on_int16.op_##M=F; \
	foo->on_int32.op_##M=F; }
	OVERRIDE_INT(ignore,map,quick_ign_map);
	OVERRIDE_INT(ignore,zip,quick_ign_zip);
	//OVERRIDE_INT(put,map,quick_put_map);
	//OVERRIDE_INT(put,zip,quick_put_zip);
	//OVERRIDE_INT(%,map,quick_mod_map); // !@#$ does that make an improvement at all?
}
#endif
