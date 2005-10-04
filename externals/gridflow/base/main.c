/*
	$Id: main.c,v 1.1 2005-10-04 02:02:13 matju Exp $

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

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>

#include "grid.h.fcs"
#include "../config.h"
#include <assert.h>
#include <limits.h>

BuiltinSymbols bsym;
GFStack gf_stack;
Ruby mGridFlow;
Ruby cFObject;

extern "C"{
void rb_raise0(
const char *file, int line, const char *func, VALUE exc, const char *fmt, ...) {
	va_list args;
	char buf[BUFSIZ];
	va_start(args,fmt);
	vsnprintf(buf, BUFSIZ, fmt, args);
	buf[BUFSIZ-1]=0;
	va_end(args);
	VALUE e = rb_exc_new2(exc, buf);
	char buf2[BUFSIZ];
	snprintf(buf2, BUFSIZ, "%s:%d:in `%s'", file, line, func);
	buf2[BUFSIZ-1]=0;
	VALUE ary = rb_funcall(e,SI(caller),0);
	if (gf_stack.n) {
		rb_funcall(ary,SI(unshift),2,rb_str_new2(buf2),
			rb_str_new2(INFO(gf_stack.s[gf_stack.n-1].o)));
	} else {
		rb_funcall(ary,SI(unshift),1,rb_str_new2(buf2));
	}
	rb_funcall(e,SI(set_backtrace),1,ary);
	rb_exc_raise(e);
}};

Ruby rb_ary_fetch(Ruby rself, int i) {
	Ruby argv[] = { INT2NUM(i) };
	return rb_ary_aref(COUNT(argv),argv,rself);
}

//----------------------------------------------------------------
// CObject

static void CObject_mark (void *z) {}
void CObject_free (void *foo) {
	CObject *self = (CObject *)foo;
	self->check_magic();
	if (!self->rself) {
		fprintf(stderr,"attempt to free object that has no rself\n");
		abort();
	}
	self->rself = 0; /* paranoia */
	delete self;
}

//----------------------------------------------------------------
// Dim

void Dim::check() {
	if (n>MAX_DIMENSIONS) RAISE("too many dimensions");
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

//----------------------------------------------------------------
\class FObject < CObject

static void FObject_prepare_message(int &argc, Ruby *&argv, Ruby &sym, FObject *foo=0) {
	if (argc<1) {
		sym = bsym._bang;
	} else if (argc>1 && !SYMBOL_P(*argv)) {
		sym = bsym._list;
	} else if (INTEGER_P(*argv)||FLOAT_P(*argv)) {
		sym = bsym._float;
	} else if (SYMBOL_P(*argv)) {
		sym = *argv;
		argc--, argv++;
	} else if (argc==1 && TYPE(*argv)==T_ARRAY) {
		sym = bsym._list;
		argc = rb_ary_len(*argv);
		argv = rb_ary_ptr(*argv);
	} else {
		RAISE("%s received bad message: argc=%d; argv[0]=%s",foo?INFO(foo):"", argc,
			argc ? rb_str_ptr(rb_inspect(argv[0])) : "");
	}
}

struct Helper {
	int argc;
	Ruby *argv;
	FObject *self;
	Ruby rself;
	int n; // stack level
};

static Ruby GridFlow_handle_braces(Ruby rself, Ruby argv);

// inlet #-1 is reserved for SystemInlet messages
// inlet #-2 is for inlet #0 messages that happen at start time
static void send_in_2 (Helper *h) { PROF(h->self) {
	int argc = h->argc;
	Ruby *argv = h->argv;
	if (h->argc<1) RAISE("not enough args");
	int inlet = INT(argv[0]);
	argc--, argv++;
	Ruby foo;
	if (argc==1 && TYPE(argv[0])==T_STRING /* && argv[0] =~ / / */) {
		foo = rb_funcall(mGridFlow,SI(parse),1,argv[0]);
		argc = rb_ary_len(foo);
		argv = rb_ary_ptr(foo);
	}
	if (argc>1) {
		foo = rb_ary_new4(argc,argv);
		GridFlow_handle_braces(0,foo);
		argc = rb_ary_len(foo);
		argv = rb_ary_ptr(foo);
	}
	if (inlet==-2) {
		Array init_messages = rb_ivar_get(h->rself,SI(@init_messages));
		rb_ary_push(init_messages, rb_ary_new4(argc,argv));
		inlet=0;
	}
	if (inlet<0 || inlet>9 /*|| inlet>real_inlet_max*/)
		if (inlet!=-3 && inlet!=-1) RAISE("invalid inlet number: %d", inlet);
	Ruby sym;
	FObject_prepare_message(argc,argv,sym,h->self);
//	if (rb_const_get(mGridFlow,SI(@verbose))==Qtrue) gfpost m.inspect
	char buf[256];
	if (inlet==-1) sprintf(buf,"_sys_%s",rb_sym_name(sym));
	else           sprintf(buf,"_%d_%s",inlet,rb_sym_name(sym));
	rb_funcall2(h->rself,rb_intern(buf),argc,argv);
} /* PROF */ }

static void send_in_3 (Helper *h) {
	while (gf_stack.n > h->n) gf_stack.pop();
}

\def void send_in (...) {
	Helper h = {argc,argv,this,rself,gf_stack.n};
	rb_ensure(
		(RMethod)send_in_2,(Ruby)&h,
		(RMethod)send_in_3,(Ruby)&h);
}

\def void send_out (...) {
	int n=0;
	if (argc<1) RAISE("not enough args");
	int outlet = INT(*argv);
	if (outlet<0 || outlet>9 /*|| outlet>real_outlet_max*/)
		RAISE("invalid outlet number: %d",outlet);
	argc--, argv++;
	Ruby sym;
	FObject_prepare_message(argc,argv,sym,this);
	Ruby noutlets2 = rb_ivar_get(rb_obj_class(rself),SYM2ID(SYM(@noutlets)));
	if (TYPE(noutlets2)!=T_FIXNUM) {
		IEVAL(rself,"STDERR.puts inspect");
		RAISE("don't know how many outlets this has");
	}
	int noutlets = INT(noutlets2);
	//if (outlet<0 || outlet>=noutlets) RAISE("outlet %d does not exist",outlet);
	// was PROF(0) a hack because of exception-handling problems?
	PROF(0) {
	Ruby argv2[argc+2];
	for (int i=0; i<argc; i++) argv2[2+i] = argv[i];
	argv2[0] = INT2NUM(outlet);
	argv2[1] = sym;
	rb_funcall2(rself,SI(send_out2), argc+2, argv2);

	Ruby ary = rb_ivar_defined(rself,SYM2ID(bsym.iv_outlets)) ?
		rb_ivar_get(rself,SYM2ID(bsym.iv_outlets)) : Qnil;
	if (ary==Qnil) goto end;
	if (TYPE(ary)!=T_ARRAY) RAISE("send_out: expected array");
	ary = rb_ary_fetch(ary,outlet);
	if (ary==Qnil) goto end;
	if (TYPE(ary)!=T_ARRAY) RAISE("send_out: expected array");
	n = rb_ary_len(ary);

	for (int i=0; i<n; i++) {
		Ruby conn = rb_ary_fetch(ary,i);
		Ruby rec = rb_ary_fetch(conn,0);
		int inl = INT(rb_ary_fetch(conn,1));
		argv2[0] = INT2NUM(inl);
		rb_funcall2(rec,SI(send_in),argc+2,argv2);
	}
	} /* PROF */
end:;
}

Ruby FObject_s_new(Ruby argc, Ruby *argv, Ruby qlass) {
	Ruby allocator = rb_ivar_defined(qlass,SI(@allocator)) ?
		rb_ivar_get(qlass,SI(@allocator)) : Qnil;
	FObject *self;
	if (allocator==Qnil) {
		// this is a pure-ruby FObject/GridObject
		// !@#$ GridObject is in FObject constructor (ugly)
		self = new GridObject;
	} else {
		// this is a C++ FObject/GridObject
		void*(*alloc)() = (void*(*)())FIX2PTR(void,allocator);
		self = (FObject *)alloc();
	}
	self->check_magic();
	Ruby keep = rb_ivar_get(mGridFlow, SI(@fobjects));
	self->bself = 0;
	Ruby rself = Data_Wrap_Struct(qlass, CObject_mark, CObject_free, self);
	self->rself = rself;
	rb_hash_aset(keep,rself,Qtrue); // prevent sweeping
	rb_funcall2(rself,SI(initialize),argc,argv);
	return rself;
}

Ruby FObject_s_install(Ruby rself, Ruby name, Ruby inlets2, Ruby outlets2) {
	int inlets, outlets;
	Ruby name2;
	if (SYMBOL_P(name)) {
		name2 = rb_funcall(name,SI(to_str),0);
	} else if (TYPE(name) == T_STRING) {
		name2 = rb_funcall(name,SI(dup),0);
	} else {
		RAISE("expect symbol or string");
	}
	inlets  =  INT(inlets2); if ( inlets<0 ||  inlets>9) RAISE("...");
	outlets = INT(outlets2); if (outlets<0 || outlets>9) RAISE("...");
	rb_ivar_set(rself,SI(@ninlets),INT2NUM(inlets));
	rb_ivar_set(rself,SI(@noutlets),INT2NUM(outlets));
	rb_ivar_set(rself,SI(@foreign_name),name2);
	rb_hash_aset(rb_ivar_get(mGridFlow,SI(@fclasses)), name2, rself);
	rb_funcall(rself, SI(install2), 1, name2);
	return Qnil;
}

\def Ruby total_time_get () {return gf_ull2num(total_time);}

\def Ruby total_time_set (Ruby x) {
	if (argc<1) RAISE("muh");
	total_time = TO(uint64,x);
	return argv[0];
}

\def void delete_m () {
	Ruby keep = rb_ivar_get(mGridFlow, SI(@fobjects));
	rb_funcall(keep,SI(delete),1,rself);
}

\classinfo
\end class FObject

/* ---------------------------------------------------------------- */
/* C++<->Ruby bridge for classes/functions in base/number.c */

static Ruby String_swap32_f (Ruby rself) {
	int n = rb_str_len(rself)/4;
	swap32(n,Pt<uint32>((uint32 *)rb_str_ptr(rself),n));
	return rself;
}

static Ruby String_swap16_f (Ruby rself) {
	int n = rb_str_len(rself)/2;
	swap16(n,Pt<uint16>((uint16 *)rb_str_ptr(rself),n));
	return rself;
}

NumberTypeE NumberTypeE_find (Ruby sym) {
	if (TYPE(sym)!=T_SYMBOL) RAISE("expected symbol (not %s)",
		rb_str_ptr(rb_inspect(rb_obj_class(sym))));
	Ruby nt_dict = rb_ivar_get(mGridFlow,SI(@number_type_dict));
	Ruby v = rb_hash_aref(nt_dict,sym);
	if (v!=Qnil) return FIX2PTR(NumberType,v)->index;
	RAISE("unknown number type \"%s\"", rb_sym_name(sym));
}

/* **************************************************************** */
\class BitPacking < CObject

\def void initialize(Ruby foo1, Ruby foo2, Ruby foo3) {}

// !@#$ doesn't support number types
\def String pack2 (String ins, String outs=Qnil) {
	int n = rb_str_len(ins) / sizeof(int32) / size;
	Pt<int32> in = Pt<int32>((int32 *)rb_str_ptr(ins),rb_str_len(ins));
	int bytes2 = n*bytes;
	Ruby out = outs!=Qnil ? rb_str_resize(outs,bytes2) : rb_str_new("",bytes2);
	rb_str_modify(out);
	pack(n,Pt<int32>(in,n),Pt<uint8>((uint8 *)rb_str_ptr(out),bytes2));
	return out;
}

// !@#$ doesn't support number types
\def String unpack2 (String ins, String outs=Qnil) {
	int n = rb_str_len(argv[0]) / bytes;
	Pt<uint8> in = Pt<uint8>((uint8 *)rb_str_ptr(ins),rb_str_len(ins));
	int bytes2 = n*size*sizeof(int32);
	Ruby out = outs!=Qnil ? rb_str_resize(outs,bytes2) : rb_str_new("",bytes2);
	rb_str_modify(out);
	unpack(n,Pt<uint8>((uint8 *)in,bytes2),Pt<int32>((int32 *)rb_str_ptr(out),n));
	return out;
}

static Ruby BitPacking_s_new(Ruby argc, Ruby *argv, Ruby qlass) {
	Ruby keep = rb_ivar_get(mGridFlow, rb_intern("@fobjects"));
	if (argc!=3) RAISE("bad args");
	if (TYPE(argv[2])!=T_ARRAY) RAISE("bad mask");
	int endian = INT(argv[0]);
	int bytes = INT(argv[1]);
	Ruby *masks = rb_ary_ptr(argv[2]);
	uint32 masks2[4];
	int size = rb_ary_len(argv[2]);
	if (size<1) RAISE("not enough masks");
	if (size>4) RAISE("too many masks (%d)",size);
	for (int i=0; i<size; i++) masks2[i] = NUM2UINT(masks[i]);
	BitPacking *self = new BitPacking(endian,bytes,size,masks2);
	Ruby rself = Data_Wrap_Struct(qlass, 0, CObject_free, self);
	self->rself = rself;
	rb_hash_aset(keep,rself,Qtrue); // prevent sweeping (leak) (!@#$ WHAT???)
	rb_funcall2(rself,SI(initialize),argc,argv);
	return rself;
}

\classinfo
\end class BitPacking

void gfpost(const char *fmt, ...) {
	va_list args;
	int length;
	va_start(args,fmt);
	const int n=256;
	char post_s[n];
	length = vsnprintf(post_s,n,fmt,args);
	if (length<0 || length>=n) sprintf(post_s+n-6,"[...]"); /* safety */
	va_end(args);
	rb_funcall(mGridFlow,SI(gfpost2),2,rb_str_new2(fmt),rb_str_new2(post_s));
}

void define_many_methods(Ruby rself, int n, MethodDecl *methods) {
	for (int i=0; i<n; i++) {
		MethodDecl *md = &methods[i];
		char *buf = strdup(md->selector);
		if (strlen(buf)>2 && strcmp(buf+strlen(buf)-2,"_m")==0)
			buf[strlen(buf)-2]=0;
		rb_define_method(rself,buf,(RMethod)md->method,-1);
		rb_enable_super(rself,buf);
		free(buf);
	}
}

static Ruby GridFlow_fclass_install(Ruby rself_, Ruby fc_, Ruby super) {
	FClass *fc = FIX2PTR(FClass,fc_);
	Ruby rself = super!=Qnil ?
		rb_define_class_under(mGridFlow, fc->name, super) :
		rb_funcall(mGridFlow,SI(const_get),1,rb_str_new2(fc->name));
	define_many_methods(rself,fc->methodsn,fc->methods);
	rb_ivar_set(rself,SI(@allocator),PTR2FIX((void*)(fc->allocator))); //#!@$??
	if (fc->startup) fc->startup(rself);
	return Qnil;
}

//----------------------------------------------------------------
// GridFlow.class
//\class GridFlow_s < patate

typedef void (*Callback)(void*);
static Ruby GridFlow_exec (Ruby rself, Ruby data, Ruby func) {
	void *data2 = FIX2PTR(void,data);
	Callback func2 = (Callback) FIX2PTR(void,func);
	func2(data2);
	return Qnil;
}

static Ruby GridFlow_get_id (Ruby rself, Ruby arg) {
	fprintf(stderr,"%ld\n",arg);
	return INT2NUM((int)arg);
}

Ruby GridFlow_rdtsc (Ruby rself) { return gf_ull2num(rdtsc()); }

/* This code handles nested lists because PureData (0.38) doesn't do it */
static Ruby GridFlow_handle_braces(Ruby rself, Ruby argv) {
	int stack[16];
	int stackn=0;
	Ruby *av = rb_ary_ptr(argv);
	int ac = rb_ary_len(argv);
	int j=0;
	for (int i=0; i<ac; ) {
		int close=0;
		if (SYMBOL_P(av[i])) {
			const char *s = rb_sym_name(av[i]);
			while (*s=='(' || *s=='{') {
				if (stackn==16) RAISE("too many nested lists (>16)");
				stack[stackn++]=j;
				s++;
			}
			const char *se = s+strlen(s);
			while (se[-1]==')' || se[-1]=='}') { se--; close++; }
			if (s!=se) {
				Ruby u = rb_str_new(s,se-s);
				av[j++] = rb_funcall(rself,SI(FloatOrSymbol),1,u);
			}
		} else {
			av[j++]=av[i];
		}
		i++;
		while (close--) {
			if (!stackn) RAISE("unbalanced '}' or ')'",av[i]);
			Ruby a2 = rb_ary_new();
			int j2 = stack[--stackn];
			for (int k=j2; k<j; k++) rb_ary_push(a2,av[k]);
			j=j2;
			av[j++] = a2;
		}
	}
	if (stackn) RAISE("unbalanced '{' or '(' (stackn=%d)",stackn);
	RARRAY(argv)->len = j;
	return rself;
}

/* ---------------------------------------------------------------- */

static uint32 memcpy_calls = 0;
static uint64 memcpy_bytes = 0;
static uint64 memcpy_time  = 0;
static uint32 malloc_calls = 0; /* only new not delete */
static uint64 malloc_bytes = 0; /* only new not delete */
static uint64 malloc_time  = 0; /* in cpu ticks */

// don't touch.
static void gfmemcopy32(int32 *as, int32 *bs, int n) {
	int32 ba = bs-as;
#define FOO(I) as[I] = (as+ba)[I];
		UNROLL_8(FOO,n,as)
#undef FOO

}

void gfmemcopy(uint8 *out, const uint8 *in, int n) {
	uint64 t = rdtsc();
	memcpy_calls++;
	memcpy_bytes+=n;
	for (; n>16; in+=16, out+=16, n-=16) {
		((int32*)out)[0] = ((int32*)in)[0];
		((int32*)out)[1] = ((int32*)in)[1];
		((int32*)out)[2] = ((int32*)in)[2];
		((int32*)out)[3] = ((int32*)in)[3];
	}
	for (; n>4; in+=4, out+=4, n-=4) { *(int32*)out = *(int32*)in; }
	for (; n; in++, out++, n--) { *out = *in; }
	t=rdtsc()-t;
	memcpy_time+=t;
}

extern "C" {
void *gfmalloc(size_t n) {
	uint64 t = rdtsc();
	void *p = malloc(n);
	long align = (long)p & 7;
	if (align) fprintf(stderr,"malloc alignment = %ld mod 8\n",align);
	t=rdtsc()-t;
	malloc_time+=t;
	malloc_calls++;
	malloc_bytes+=n;
	return p;
}
void gffree(void *p) {
	uint64 t = rdtsc();
	free(p);
	t=rdtsc()-t;
	malloc_time+=t;
}};

Ruby GridFlow_memcpy_calls (Ruby rself) { return   LONG2NUM(memcpy_calls); }
Ruby GridFlow_memcpy_bytes (Ruby rself) { return gf_ull2num(memcpy_bytes); }
Ruby GridFlow_memcpy_time  (Ruby rself) { return gf_ull2num(memcpy_time); }
Ruby GridFlow_malloc_calls (Ruby rself) { return   LONG2NUM(malloc_calls); }
Ruby GridFlow_malloc_bytes (Ruby rself) { return gf_ull2num(malloc_bytes); }
Ruby GridFlow_malloc_time  (Ruby rself) { return gf_ull2num(malloc_time); }

Ruby GridFlow_profiler_reset2 (Ruby rself) {
	memcpy_calls = memcpy_bytes = memcpy_time = 0;
	malloc_calls = malloc_bytes = malloc_time = 0;
	return Qnil;
}

/* ---------------------------------------------------------------- */

void startup_number();
void startup_grid();
void startup_flow_objects();
void startup_flow_objects_for_image();
void startup_flow_objects_for_matrix();

Ruby cFormat;

#define SDEF(_class_,_name_,_argc_) \
	rb_define_singleton_method(c##_class_,#_name_,(RMethod)_class_##_s_##_name_,_argc_)
#define SDEF2(_name1_,_name2_,_argc_) \
	rb_define_singleton_method(mGridFlow,_name1_,(RMethod)_name2_,_argc_)

STARTUP_LIST(void)

// Ruby's entrypoint.
void Init_gridflow () {
#define FOO(_sym_,_name_) bsym._sym_ = ID2SYM(rb_intern(_name_));
BUILTIN_SYMBOLS(FOO)
#undef FOO
	signal(11,SIG_DFL); // paranoia
	mGridFlow = EVAL("module GridFlow; CObject = ::Object; "
		"class<<self; attr_reader :bridge_name; end; "
		"def post_string(s) STDERR.puts s end; "
		"self end");
	SDEF2("exec",GridFlow_exec,2);
	SDEF2("get_id",GridFlow_get_id,1);
	SDEF2("rdtsc",GridFlow_rdtsc,0);
	SDEF2("profiler_reset2",GridFlow_profiler_reset2,0);
	SDEF2("memcpy_calls",GridFlow_memcpy_calls,0);
	SDEF2("memcpy_bytes",GridFlow_memcpy_bytes,0);
	SDEF2("memcpy_time", GridFlow_memcpy_time,0);
	SDEF2("malloc_calls",GridFlow_malloc_calls,0);
	SDEF2("malloc_bytes",GridFlow_malloc_bytes,0);
	SDEF2("malloc_time", GridFlow_malloc_time,0);
	SDEF2("handle_braces!",GridFlow_handle_braces,1);
	SDEF2("fclass_install",GridFlow_fclass_install,2);

//#define FOO(A) fprintf(stderr,"sizeof("#A")=%d\n",sizeof(A));
//FOO(Dim) FOO(BitPacking) FOO(GridHandler) FOO(GridInlet) FOO(GridOutlet) FOO(GridObject)
//#undef FOO

	rb_ivar_set(mGridFlow, SI(@fobjects), rb_hash_new());
	rb_ivar_set(mGridFlow, SI(@fclasses), rb_hash_new());
	rb_ivar_set(mGridFlow, SI(@bsym), PTR2FIX(&bsym));
	rb_define_const(mGridFlow, "GF_VERSION", rb_str_new2(GF_VERSION));
	rb_define_const(mGridFlow, "GF_COMPILE_TIME", rb_str_new2(GF_COMPILE_TIME));

	cFObject = rb_define_class_under(mGridFlow, "FObject", rb_cObject);
	EVAL(
\ruby
	module GridFlow
		class FObject
		def send_out2(*) end
		def self.install2(*) end
		def self.add_creator(name)
			name=name.to_str.dup
			GridFlow.fclasses[name]=self
			GridFlow.add_creator_2 name end
		end
	end
\end ruby
);
	define_many_methods(cFObject,COUNT(FObject_methods),FObject_methods);
	SDEF(FObject, install, 3);
	SDEF(FObject, new, -1);
	ID gbi = SI(gf_bridge_init);
	if (rb_respond_to(rb_cData,gbi)) rb_funcall(rb_cData,gbi,0);
	Ruby cBitPacking =
		rb_define_class_under(mGridFlow, "BitPacking", rb_cObject);
	define_many_methods(cBitPacking,
		ciBitPacking.methodsn,
		ciBitPacking.methods);
	SDEF(BitPacking,new,-1);
	rb_define_method(rb_cString, "swap32!", (RMethod)String_swap32_f, 0);
	rb_define_method(rb_cString, "swap16!", (RMethod)String_swap16_f, 0);

	startup_number();
	startup_grid();
	startup_flow_objects();
	startup_flow_objects_for_image();
	startup_flow_objects_for_matrix();
	if (!EVAL("begin require 'gridflow/base/main.rb'; true\n"
		"rescue Exception => e; "
		"STDERR.puts \"can't load: #{$!}\n"
		"backtrace: #{$!.backtrace.join\"\n\"}\n"
		"$: = #{$:.inspect}\"\n; false end")) return;
	cFormat = EVAL("GridFlow::Format");
	STARTUP_LIST()
	EVAL("h=GridFlow.fclasses; h['#io:window'] = h['#io:quartz']||h['#io:x11']||h['#io:sdl']");
	EVAL("GridFlow.load_user_config");
	signal(11,SIG_DFL); // paranoia
}

void GFStack::push (FObject *o) {
	void *bp = &o; // really. just finding our position on the stack.
	if (n>=GF_STACK_MAX)
		RAISE("stack overflow (maximum %d FObject activations at once)", GF_STACK_MAX);
	uint64 t = rdtsc();
	if (n) s[n-1].time = t - s[n-1].time;
	s[n].o = o;
	s[n].bp = bp;
	s[n].time = t;
	n++;
}

void GFStack::pop () {
	uint64 t = rdtsc();
	if (!n) RAISE("stack underflow (WHAT?)");
	n--;
	if (s[n].o) s[n].o->total_time += t - s[n].time;
	if (n) s[n-1].time = t - s[n-1].time;
}

uint64 gf_timeofday () {
	timeval t;
	gettimeofday(&t,0);
	return t.tv_sec*1000000+t.tv_usec;
}
