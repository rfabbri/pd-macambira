/*
	$Id: flow_objects.c 4548 2009-10-31 20:26:25Z matju $

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

#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <errno.h>
#include "gridflow.hxx.fcs"
#ifdef DESIRE
#include "desire.h"
#else
extern "C" {
#include "bundled/g_canvas.h"
};
extern "C" t_canvas *canvas_getrootfor(t_canvas *x);
#endif

//using namespace std; // can't

//#undef GRID_INPUT
//#define GRID_INPUT(I,V) GRID_INLET(I) {in->buf=V=new Grid(in->dim,NumberTypeE_type_of(data));} GRID_FINISH

/* ---------------------------------------------------------------- */

// BAD HACK: GCC complains: unimplemented (--debug mode only) (i don't remember which GCC this was)
#ifdef HAVE_DEBUG
#define SCOPY(a,b,n) COPY(a,b,n)
#else
#define SCOPY(a,b,n) SCopy<n>::f(a,b)
#endif

template <long n> class SCopy {
public: template <class T> static inline void __attribute__((always_inline)) f(T *a, T *b) {
		*a=*b; SCopy<n-1>::f(a+1,b+1);}};
template <> class SCopy<0> {
public: template <class T> static inline void __attribute__((always_inline)) f(T *a, T *b) {}};

/*template <> class SCopy<4> {
public: template <class T>
	static inline void __attribute__((always_inline)) f(T *a, T *b) {
		*a=*b; SCopy<3>::f(a+1,b+1);}
	static inline void __attribute__((always_inline)) f(uint8 *a, uint8 *b)
	{ *(int32 *)a=*(int32 *)b; }
};*/

Numop *op_add, *op_sub, *op_mul, *op_div, *op_mod, *op_shl, *op_and, *op_put;

static void expect_dim_dim_list (P<Dim> d) {
	if (d->n!=1) RAISE("dimension list should be Dim[n], not %s",d->to_s());}
//static void expect_min_one_dim (P<Dim> d) {
//	if (d->n<1) RAISE("minimum 1 dimension");}
static void expect_max_one_dim (P<Dim> d) {
	if (d->n>1) RAISE("expecting Dim[] or Dim[n], got %s",d->to_s());}
//static void expect_exactly_one_dim (P<Dim> d) {
//	if (d->n!=1) RAISE("expecting Dim[n], got %s",d->to_s());}

//****************************************************************
\class GridCast : FObject {
	\attr NumberTypeE nt;
	\constructor (NumberTypeE nt) {this->nt = nt;}
	\grin 0
};
GRID_INLET(0) {
	out = new GridOutlet(this,0,in->dim,nt);
} GRID_FLOW {
	out->send(n,data);
} GRID_END
\end class {install("#cast",1,1); add_creator("@cast");}

//****************************************************************

GridHandler *stromgol; // remove this asap

//{ ?,Dim[B] -> Dim[*Cs] }
// out0 nt to be specified explicitly
\class GridImport : FObject {
	\attr NumberTypeE cast;
	\attr P<Dim> dim; // size of grids to send
	PtrGrid dim_grid;
	\constructor (...) {
		dim_grid.constrain(expect_dim_dim_list);
		this->cast = argc>=2 ? NumberTypeE_find(argv[1]) : int32_e;
		if (argc>2) RAISE("too many arguments");
		if (argc>0 && argv[0]!=gensym("per_message")) {
			dim_grid=new Grid(argv[0]);
			dim = dim_grid->to_dim();
			if (!dim->prod()) RAISE("target grid size must not be zero");
		}
	}
	~GridImport() {}
	\decl 0 reset();
	\decl 0 symbol(t_symbol *x);
	\decl 0 to_ascii(...);
	\decl 0 bang();
	//\decl 0 list(...);
	\decl 1 per_message();
	\grin 0
	\grin 1 int32
	template <class T> void process (long n, T *data) {
		if (in.size()<=0) in.resize(1);
		if (!in[0]) in[0]=new GridInlet((FObject *)this,stromgol);
		while (n) {
			if (!out || !out->dim) out = new GridOutlet(this,0,dim?dim:in[0]->dim,cast);
			long n2 = min((long)n,out->dim->prod()-out->dex);
			out->send(n2,data);
			n-=n2; data+=n2;
		}
	}
};

GRID_INLET(0) {} GRID_FLOW {process(n,data);} GRID_END
GRID_INPUT(1,dim_grid) {
	P<Dim> d = dim_grid->to_dim();
	if (!d->prod()) RAISE("target grid size must not be zero");
	dim = d;
} GRID_END

\def 0 symbol(t_symbol *x) {
	const char *name = x->s_name;
	long n = strlen(name);
	if (!dim) out=new GridOutlet(this,0,new Dim(n));
	process(n,(uint8 *)name);
}
\def 0 to_ascii(...) {
	std::ostringstream os;
	pd_oprint(os,argc,argv);
	string s = os.str();
	long n = s.length();
	if (!dim) out=new GridOutlet(this,0,new Dim(n),cast);
	process(n,(uint8 *)s.data());
}

\def 0 bang() {_0_list(0,0);}
\def 0 list(...) {//first two lines are there until grins become strictly initialized.
	if (in.size()<=0) in.resize(1);
	if (!in[0]) in[0]=new GridInlet((FObject *)this,stromgol);
	in[0]->from_list(argc,argv,cast);
	if (!argc && !dim) out = new GridOutlet(this,0,new Dim(0),cast);
}
\def 1 per_message() {dim=0; dim_grid=0;}

\def 0 reset() {int32 foo[1]={0}; if (out) while (out->dim) out->send(1,foo);}
\end class {install("#import",2,1); add_creator("@import"); stromgol = &GridImport_grid_0_hand;}

//****************************************************************
/*{ Dim[*As] -> ? }*/
/* in0: integer nt */
\class GridToFloat : FObject {
	\constructor () {}
	\grin 0
};
GRID_INLET(0) {
} GRID_FLOW {
	for (int i=0; i<n; i++) outlet_float(bself->outlets[0],data[i]);
} GRID_END
\end class {install("#to_float",1,1); add_creator("#export"); add_creator("@export");}

\class GridToSymbol : FObject {
	\constructor () {}
	\grin 0
};
GRID_INLET(0) {
	in->set_chunk(0);
} GRID_FLOW {
	char c[n+1];
	for (int i=0; i<n; i++) c[i]=(char)data[i];
	c[n]=0;
	outlet_symbol(bself->outlets[0],gensym(c));
} GRID_END
\end class {install("#to_symbol",1,1); add_creator("#export_symbol"); add_creator("@export_symbol");}

/*{ Dim[*As] -> ? }*/
/* in0: integer nt */
\class GridExportList : FObject {
	\constructor () {}
	int n;
	\grin 0
};

GRID_INLET(0) {
	long n = in->dim->prod();
	if (n>1000000) RAISE("list too big (%ld elements, max 1000000)", n);
	this->n = n;
	in->set_chunk(0);
} GRID_FLOW {
	send_out(0,n,data);
} GRID_FINISH {
	if (in->dim->prod()==0) send_out(0,0,data);
} GRID_END

\end class {install("#to_list",1,1); add_creator("#export_list"); add_creator("@export_list");}

/* **************************************************************** */
\class GridPrint : FObject {
	\constructor (t_symbol *name=0) {
		this->dest = 0;
		this->name = name;
		base=10; trunc=70; maxrows=50;
	}
	\attr t_symbol *name;
	\grin 0
	int base;
	uint32 trunc;
	int maxrows;
	int columns;
	t_pd *dest;
	\decl 0 dest (void *p);
	\decl void end_hook ();
	\decl 0 base (int x);
	\decl 0 trunc (int x);
	\decl 0 maxrows (int y);
	void puts (const char *s) {
		if (!dest) post("%s",s);
		else {
			int n = strlen(s);
			t_atom a[n];
			for (int i=0; i<n; i++) SETFLOAT(a+i,s[i]);
			//fprintf(stderr,"dest=%p\n",dest);
			//fprintf(stderr,"*dest={%08x,%08x,%08x,%08x,...}\n",dest[0],dest[1],dest[2],dest[3]);
			pd_typedmess(dest,gensym("very_long_name_that_nobody_uses"),n,a);
		}
	}
	void puts (std::string s) {puts(s.data());}
	void puts (std::ostringstream &s) {puts(s.str());}
	template <class T> void make_columns (int n, T *data);
	template <class T> void dump(std::ostream &s, int n, T *data, char sep=' ', int trunc=-1) {
		if (trunc<0) trunc=this->trunc;
		std::string f = format(NumberTypeE_type_of(data));
		for (int i=0; i<n; i++) {
			if (base!=2) oprintf(s,f.data(),data[i]);
			else {
				T x = gf_abs(data[i]);
				int ndigits = 1+highest_bit(uint64(x));
				for (int j=columns-ndigits-(data[i]!=x); j>=0; j--) s<<' ';
				if (data[i]!=x) s<<'-';
				for (int j=ndigits-1; j>=0; j--) {
					s<<char('0'+(((long)x>>j)&1));
				}
			}
			if (i<n-1) s << sep;
			if (s.tellp()>trunc) return;
		}
	}
	void dump_dims(std::ostream &s, GridInlet *in) {
		if (name && name!=&s_) s << name->s_name << ": ";
		s << "Dim[";
		for (int i=0; i<in->dim->n; i++) {
			s << in->dim->v[i];
			if (i<in->dim->n-1) s << ',';
		}
		s << "]";
		if (in->nt!=int32_e) s << "(" << number_type_table[in->nt].name << ")";
		s << ": ";
	}
	std::string format (NumberTypeE nt) {
		if (nt==float32_e) return "%6.6f";
		if (nt==float64_e) return "%14.14f";
		std::ostringstream r;
		r << "%";
		r << columns;
		//if (nt==int64_e) r << "l";
		if (base==2)  r << "b"; else
		if (base==8)  r << "o"; else
		if (base==10) r << "d"; else
		if (base==16) r << "x";
		return r.str();
	}
};
\def 0 dest (void *p) {dest = (t_pd *)p;}
\def void end_hook () {}
\def 0 base (int x) { if (x==2 || x==8 || x==10 || x==16) base=x; else RAISE("base %d not supported",x); }
\def 0 trunc (int x) {
	if (x<0 || x>240) RAISE("out of range (not in 0..240 range)");
	trunc = x;
}
\def 0 maxrows (int y) {maxrows = y;}
template <class T> void GridPrint::make_columns (int n, T *data) {
	long maxv=0;
	long minv=0;
	for (int i=0; i<n; i++) {
		if (maxv<data[i]) maxv=long(data[i]);
		if (minv>data[i]) minv=long(data[i]);
	}
	int maxd = 1 + (maxv<0) + int(log(max(1.,fabs(maxv)))/log(base));
	int mind = 1 + (minv<0) + int(log(max(1.,fabs(minv)))/log(base));
	//fprintf(stderr,"v=(%d,%d) d=(%d,%d)\n",minv,maxv,mind,maxd);
	columns = max(maxd,mind);
}
GRID_INLET(0) {
	in->set_chunk(0);
} GRID_FLOW {
	std::ostringstream head;
	dump_dims(head,in);
	int ndim = in->dim->n;
	if (ndim > 3) {
		head << " (not printed)";
		puts(head);
	} else if (ndim < 2) {
		make_columns(n,data);
		dump(head,n,data,' ',trunc);
		puts(head);
	} else if (ndim == 2) {
		puts(head);
		make_columns(n,data);
		long sy = in->dim->v[0];
		long sx = n/sy;
		for (int row=0; row<sy; row++) {
			std::ostringstream body;
			dump(body,sx,&data[sx*row],' ',trunc);
			if (body.tellp()>trunc) body << "...";
			puts(body);
			if (row>maxrows) {puts("..."); break;}
		}
	} else if (ndim == 3) {
		puts(head);
		make_columns(n,data);
		int sy = in->dim->v[0];
		int sx = in->dim->v[1];
		int sz = n/sy;
		int sz2 = sz/in->dim->v[1];
		for (int row=0; row<sy; row++) {
			std::ostringstream str;
			for (int col=0; col<sx; col++) {
				str << "(";
				dump(str,sz2,&data[sz*row+sz2*col],' ',trunc);
				if (str.tellp()>trunc) {str << "..."; break;} else str << ")";
			}
			puts(str);
			if (row>maxrows) {puts("..."); break;}
		}
	}
	end_hook(0,0);
} GRID_FINISH {
	std::ostringstream head;
	dump_dims(head,in);
	if (in->dim->prod()==0) puts(head);
} GRID_END
\end class {install("#print",1,1); add_creator("@print");}

/* **************************************************************** */
// [#store] is the class for storing a grid and restituting it on demand.
// The right inlet receives the grid. The left inlet receives either a bang
// (which forwards the whole image) or a grid describing what to send.
//{ Dim[*As,B],Dim[*Cs,*Ds] -> Dim[*As,*Ds] }
// in0: integer nt
// in1: whatever nt
// out0: same nt as in1
\class GridStore : FObject {

	PtrGrid r; // can't be \attr
	PtrGrid put_at; // can't be //\attr
	\attr Numop *op;
	int32 *wdex ; // temporary buffer, copy of put_at
	int32 *fromb;
	int32 *to2  ;
	int lsd; // lsd = Last Same Dimension (for put_at)
	int d; // goes with wdex
	long cs; // chunksize used in put_at
	\constructor (Grid *r=0) {
		put_at.constrain(expect_max_one_dim);
		this->r = r?r:new Grid(new Dim(),int32_e,true);
		op = op_put;
		wdex  = NEWBUF(int32,Dim::MAX_DIM); // temporary buffer, copy of put_at
		fromb = NEWBUF(int32,Dim::MAX_DIM);
		to2   = NEWBUF(int32,Dim::MAX_DIM);
	}
	~GridStore () {
		DELBUF(wdex);
		DELBUF(fromb);
		DELBUF(to2);
	}
	\decl 0 bang ();
	\decl 1 reassign ();
	\decl 1 put_at (...);
	\grin 0 int
	\grin 1
	template <class T> void compute_indices(T *v, long nc, long nd);
};

// takes the backstore of a grid and puts it back into place. a backstore
// is a grid that is filled while the grid it would replace has not
// finished being used.
static void snap_backstore (PtrGrid &r) {if (r.next) {r=r.next.p; r.next=0;}}

template <class T> void GridStore::compute_indices(T *v, long nc, long nd) {
	for (int i=0; i<nc; i++) {
		uint32 wrap = r->dim->v[i];
		bool fast = lowest_bit(wrap)==highest_bit(wrap); // is power of two?
		if (i) {
			if (fast) op_shl->map(nd,v,(T)highest_bit(wrap));
			else      op_mul->map(nd,v,(T)wrap);
		}
		if (fast) op_and->map(nd,v+nd*i,(T)(wrap-1));
		else      op_mod->map(nd,v+nd*i,(T)(wrap));
		if (i) op_add->zip(nd,v,v+nd*i);
	}
}

// !@#$ i should ensure that n is not exceedingly large
// !@#$ worse: the size of the foo buffer may still be too large
GRID_INLET(0) {
	// snap_backstore must be done before *anything* else
	snap_backstore(r);
	int na = in->dim->n;
	int nb = r->dim->n;
	int32 v[Dim::MAX_DIM];
	if (na<1) RAISE("must have at least 1 dimension.",na,1,1+nb);
	long nc = in->dim->get(na-1);
	if (nc>nb) RAISE("got %d elements in last dimension, expecting <= %d", nc, nb);
	long nnc = r->dim->prod(nc);
	int lastindexable = nnc ? r->dim->prod()/nnc-1 : 0; // SIGFPE happened when r was especially empty (nnc==0)
	int ngreatest = nt_greatest((T *)0);
	if (lastindexable > ngreatest) RAISE("lastindexable=%d > ngreatest=%d (ask matju)",lastindexable,ngreatest);
	int nd = nb-nc+na-1;
	COPY(v,in->dim->v,na-1);
	COPY(v+na-1,r->dim->v+nc,nb-nc);
	out=new GridOutlet(this,0,new Dim(nd,v),r->nt);
	if (nc>0) in->set_chunk(na-1);
} GRID_FLOW {
	int na = in->dim->n;
	int nc = in->dim->get(na-1);
	long size = r->dim->prod(nc);
	long nd = n/nc;
	T w[n];
	T *v=w;
	if (sizeof(T)==1 && nc==1 && r->dim->v[0]<=256) {
		// bug? shouldn't modulo be done here?
		v=data;
	} else {
		COPY(v,data,n);
		for (long k=0,i=0; i<nc; i++) for (long j=0; j<n; j+=nc) v[k++] = data[i+j];
		compute_indices(v,nc,nd);
	}
#define FOO(type) { \
	type *p = (type *)*r; \
	if (size<=16) { \
		type tada[nd*size]; \
		type *foo = tada; \
		long i=0; \
		switch (size) { \
		case 1: for (; i<(nd&-4); i+=4, foo+=4) { \
			foo[0] = p[v[i+0]]; \
			foo[1] = p[v[i+1]]; \
			foo[2] = p[v[i+2]]; \
			foo[3] = p[v[i+3]]; \
		} break; \
		case 2: for (; i<nd; i++, foo+=2) SCOPY(foo,p+2*v[i],2); break; \
		case 3: for (; i<nd; i++, foo+=3) SCOPY(foo,p+3*v[i],3); break; \
		case 4: for (; i<nd; i++, foo+=4) SCOPY(foo,p+4*v[i],4); break; \
		default:; }; \
		for (; i<nd; i++, foo+=size) COPY(foo,p+size*v[i],size); \
		out->send(size*nd,tada); \
	} else { \
		for (int i=0; i<nd; i++) out->send(size,p+size*v[i]); \
	} \
}
	TYPESWITCH(r->nt,FOO,)
#undef FOO
} GRID_FINISH {
	if (in->dim->prod()==0) {
		long n = in->dim->prod(0,-2);
		long size = r->dim->prod();
#define FOO(T) while (n--) out->send(size,(T *)*r);
		TYPESWITCH(r->nt,FOO,)
#undef FOO
	}
} GRID_END

GRID_INLET(1) {
	NumberTypeE nt = NumberTypeE_type_of(data);
	if (!put_at) { // reassign
		if (in[0].dim) r.next = new Grid(in->dim,nt);
		else           r      = new Grid(in->dim,nt);
		return;
	}
	// put_at ( ... )
	snap_backstore(r);
	SAME_TYPE(in,r);
	//!@#$ should check types. if (r->nt!=in->nt) RAISE("shoo");
	long nn=r->dim->n, na=put_at->dim->v[0], nb=in->dim->n;
	int32 sizeb[nn];
	for (int i=0; i<nn; i++) { fromb[i]=0; sizeb[i]=1; }
	COPY(wdex       ,(int32 *)*put_at   ,put_at->dim->prod());
	COPY(fromb+nn-na,(int32 *)*put_at   ,na);
	COPY(sizeb+nn-nb,(int32 *)in->dim->v,nb);
	for (int i=0; i<nn; i++) to2[i] = fromb[i]+sizeb[i];
	d=0;
	// find out when we can skip computing indices
	//!@#$ should actually also stop before blowing up packet size
	lsd=nn;
	while (lsd>=nn-in->dim->n) {
		lsd--;
		//int cs = in->dim->prod(lsd-nn+in->dim->n);
		if (/*cs*(number_type_table[in->nt].size/8)>GridOutlet::MAX_PACKET_SIZE ||*/
			fromb[lsd]!=0 || sizeb[lsd]!=r->dim->v[lsd]) break;
	}
	lsd++;
	long chunk = lsd-nn+in->dim->n;
	in->set_chunk(     chunk);
	cs = in->dim->prod(chunk);
} GRID_FLOW {
	//fprintf(stderr,"d=%d\n",d);
	if (!put_at) { // reassign
		COPY(((T *)*(r.next ? r.next.p : &*r.p))+dex, data, n);
		return;
	}
	// put_at (...)
	int32 v[lsd];
	int32 *x = wdex;
	while (n) {
		// here d is the dim# to reset; d=n for none
		for(;d<lsd;d++) x[d]=fromb[d];
		COPY(v,x,lsd);
		compute_indices(v,lsd,1);
		op->zip(cs,(T *)*r+v[0]*cs,data);
		data+=cs;
		n-=cs;
		// find next set of indices; here d is the dim# to increment
		for(;;) {d--; if (d<0) return; x[d]++; if (x[d]<to2[d]) break;}
		d++;
	}
} GRID_END
\def 0 bang () {
	t_atom a[2];
	SETFLOAT(a+0,0);
	SETSYMBOL(a+1,gensym("#"));
	pd_list((t_pd *)bself,&s_list,2,a);
}
\def 1 reassign () {put_at=0;}
\def 1 put_at (...) {
	if (argv[0].a_type==A_LIST) put_at=convert(argv[0],(Grid **)0);
	else {
		put_at=new Grid(new Dim(argc),int32_e);
		int32 *v = (int32 *)*put_at;
		for (int i=0; i<argc; i++) v[i]=convert(argv[i],(int32 *)0);
	}
}
\end class {install("#store",2,1); add_creator("@store");}

//****************************************************************
//{ Dim[*As]<T> -> Dim[*As]<T> } or
//{ Dim[*As]<T>,Dim[*Bs]<T> -> Dim[*As]<T> }
\class GridOp : FObject {
	\attr Numop *op;
	PtrGrid r;
	\constructor (Numop *op, Grid *r=0) {
		this->op=op;
		this->r=r?r:new Grid(new Dim(),int32_e,true);
	}
	\grin 0
	\grin 1
};

GRID_INLET(0) {
	snap_backstore(r);
	SAME_TYPE(in,r);
	out=new GridOutlet(this,0,in->dim,in->nt);
	if (op->size>1 && (in->dim->get(in->dim->n-1)!=op->size || r->dim->get(r->dim->n-1)!=op->size))
		RAISE("using %s requires Dim(...,%d) in both inlets but got: left=%s right=%s",
			op->name,op->size,in->dim->to_s(),r->dim->to_s());
	//if (out->inlets.size()==1) post("[#]: 1 receiver with bugger size %s",out->inlets[0]->dim->to_s());
} GRID_FLOW {
	T *rdata = (T *)*r;
	long loop = r->dim->prod();
	T tada[n];
	COPY(tada,data,n);
	if (loop>1) {
		if (dex+n <= loop) {
			op->zip(n/op->size,tada,rdata+dex);
		} else {
			// !@#$ should prebuild and reuse this array when "loop" is small
			T data2[n];
			long ii = mod(dex,loop);
			long m = min(loop-ii,n);
			COPY(data2,rdata+ii,m);
			long nn = m+((n-m)/loop)*loop;
			for (long i=m; i<nn; i+=loop) COPY(data2+i,rdata,loop);
			if (n>nn) COPY(data2+nn,rdata,n-nn);
			op->zip(n/op->size,tada,data2);
		}
	} else op->map(n,tada,*rdata);
	out->send(n,tada);
} GRID_END

GRID_INPUT2(1,r) {} GRID_END
\end class {install("#",2,1); add_creator("@");}

//****************************************************************
\class GridFold : FObject {
	\attr Numop *op;
	\attr PtrGrid seed;
	\constructor (Numop *op) {this->op=op;}
	\grin 0
};

GRID_INLET(0) {
	//{ Dim[*As,B,*Cs]<T>,Dim[*Cs]<T> -> Dim[*As,*Cs]<T> }
	if (seed) SAME_TYPE(in,seed);
	int an = in->dim->n;
	int bn = seed?seed->dim->n:0;
	if (an<=bn) RAISE("minimum 1 more dimension than the seed (%d vs %d)",an,bn);
	int32 v[an-1];
	int yi = an-bn-1;
	COPY(v,in->dim->v,yi);
	COPY(v+yi,in->dim->v+an-bn,bn);
	if (seed) SAME_DIM(an-(yi+1),in->dim,(yi+1),seed->dim,0);
	out=new GridOutlet(this,0,new Dim(an-1,v),in->nt);
	in->set_chunk(yi);
	if (in->dim->prod(yi)==0) {
		long n = out->dim->prod();
		T x=0; op->on(x)->neutral(&x,at_left);
		for(long i=0; i<n; i++) out->send(1,&x);
	}
} GRID_FLOW {
	int an = in->dim->n;
	int bn = seed?seed->dim->n:0;
	long yn = in->dim->v[an-bn-1];
	long zn = in->dim->prod(an-bn);
	T buf[n/yn];
	long nn=n;
	long yzn=yn*zn;
	for (long i=0; n; i+=zn, data+=yzn, n-=yzn) {
		if (seed) COPY(buf+i,((T *)*seed),zn);
		else {T neu; op->on(*buf)->neutral(&neu,at_left); op_put->map(zn,buf+i,neu);}
		op->fold(zn,yn,buf+i,data);
	}
	out->send(nn/yn,buf);
} GRID_FINISH {
} GRID_END

\end class {install("#fold",1,1);}

\class GridScan : FObject {
	\attr Numop *op;
	\attr PtrGrid seed;
	\constructor (Numop *op) {this->op = op;}
	\grin 0
};

GRID_INLET(0) {
	//{ Dim[*As,B,*Cs]<T>,Dim[*Cs]<T> -> Dim[*As,B,*Cs]<T> }
	if (seed) SAME_TYPE(in,seed);
	int an = in->dim->n;
	int bn = seed?seed->dim->n:0;
	if (an<=bn) RAISE("minimum 1 more dimension than the right hand");
	if (seed) SAME_DIM(bn,in->dim,an-bn,seed->dim,0);
	out=new GridOutlet(this,0,in->dim,in->nt);
	in->set_chunk(an-bn-1);
} GRID_FLOW {
	int an = in->dim->n;
	int bn = seed?seed->dim->n:0;
	long yn = in->dim->v[an-bn-1];
	long zn = in->dim->prod(an-bn);
	long factor = yn*zn;
	T buf[n];
	COPY(buf,data,n);
	if (seed) {
		for (long i=0; i<n; i+=factor) op->scan(zn,yn,(T *)*seed,buf+i);
	} else {
		T neu; op->on(*buf)->neutral(&neu,at_left);
		T seed[zn]; op_put->map(zn,seed,neu);
		for (long i=0; i<n; i+=factor) op->scan(zn,yn,      seed,buf+i);
	}
	out->send(n,buf);
} GRID_END

\end class {install("#scan",1,1);}

//****************************************************************
// L      is a Dim[*si,sj,    *ss]<T>
// R      is a Dim[    sj,*sk,*ss]<T>
// Seed   is a Dim[           *ss]<T>
// result is a Dim[*si,   *sk,*ss]<T>
// Currently *ss can only be = Dim[]
\class GridInner : FObject {
	\attr Numop *op;
	\attr Numop *fold;
	\attr PtrGrid seed;
	PtrGrid r;
	PtrGrid r2; // temporary
	bool use_dot;
	\constructor (Grid *r=0) {
		this->op = op_mul;
		this->fold = op_add;
		this->seed = new Grid(new Dim(),int32_e,true);
		this->r    = r ? r : new Grid(new Dim(),int32_e,true);
	}
	\grin 0
	\grin 1
};

// let's see this as a matrix product like L[i,j]*R[j,k] in Einstein notation
//   L: matrix of size si by sj
//   R: matrix of size sj by sk
//  LR: matrix of size si by sk
template <class T> void inner_child_a (T *as, T *bs, int sj, int sk, int chunk) {
	for (int j=0; j<chunk; j++, as+=sk, bs+=sj) op_put->map(sk,as,*bs);}
template <class T, int sk> void inner_child_b (T *as, T *bs, int sj, int chunk) {
	for (int j=0; j<chunk; j++, as+=sk, bs+=sj) op_put->map(sk,as,*bs);}

// Inner product in a Module on the (+,*) Ring
//      | BBBBB
//      j BBBBB
//      | BBBBB
// --j--*---k---
// AAAAA  CCCCC
template <class T> void dot_add_mul (long sk, long sj, T *cs, T *as, T *bs) {
	for (long k=0; k<sk; k++) {T c=0; for (long j=0; j<sj; j++) {c+=as[j]*bs[j*sk+k];} *cs++=c;}}
template <class T, long sj> void dot_add_mul (long sk, T *cs, T *as, T *bs) {
	for (long k=0; k<sk; k++) {T c=0; for (long j=0; j<sj; j++) {c+=as[j]*bs[j*sk+k];} *cs++=c;}}
template <class T, long sj, long sk> void dot_add_mul (T *cs, T *as, T *bs) {
	for (long k=0; k<sk; k++) {T c=0; for (long j=0; j<sj; j++) {c+=as[j]*bs[j*sk+k];} *cs++=c;}}

GRID_INLET(0) {
	SAME_TYPE(in,r);
	SAME_TYPE(in,seed);
	P<Dim> a=in->dim, b=r->dim;
	if (a->n<1) RAISE("a: minimum 1 dimension");
	if (b->n<1) RAISE("b: minimum 1 dimension");
	if (seed->dim->n != 0) RAISE("seed must be a scalar");
	int n = a->n+b->n-2;
	SAME_DIM(1,a,a->n-1,b,0);
	int32 v[n];
	COPY(v,a->v,a->n-1);
	COPY(v+a->n-1,b->v+1,b->n-1);
	out=new GridOutlet(this,0,new Dim(n,v),in->nt);
	in->set_chunk(a->n-1);
	long sjk=r->dim->prod(), sj=in->dim->prod(a->n-1), sk=sjk/sj;
	long chunk = max(1L,GridOutlet::MAX_PACKET_SIZE/sjk);
	T *rdata = (T *)*r;
	r2=new Grid(new Dim(chunk*sjk),r->nt);
	T *buf3 = (T *)*r2;
	for (long i=0; i<sj; i++)
		for (long j=0; j<chunk; j++)
			COPY(buf3+(j+i*chunk)*sk,rdata+i*sk,sk);
	use_dot = op==op_mul && fold==op_add && seed->dim->n==0 && *(T *)*seed==0;
} GRID_FLOW {
    long sjk=r->dim->prod(), sj=in->dim->prod(in->dim->n-1), sk=sjk/sj;
    long chunk = max(1L,GridOutlet::MAX_PACKET_SIZE/sjk), off=chunk;
    T buf [chunk*sk];
    T buf2[chunk*sk];
    if (use_dot) {
	while (n) {
		if (chunk*sj>n) chunk=n/sj;
		if (sj<=4 && sk<=4) switch ((sj-1)*4+(sk-1)) {
#define DOT_ADD_MUL_3(sj,sk) for (int i=0; i<chunk; i++) dot_add_mul<T,sj,sk>( buf2+sk*i,data+sj*i,(T *)*r);
		case  0: DOT_ADD_MUL_3(1,1); break;
		case  1: DOT_ADD_MUL_3(1,2); break;
		case  2: DOT_ADD_MUL_3(1,3); break;
		case  3: DOT_ADD_MUL_3(1,4); break;
		case  4: DOT_ADD_MUL_3(2,1); break;
		case  5: DOT_ADD_MUL_3(2,2); break;
		case  6: DOT_ADD_MUL_3(2,3); break;
		case  7: DOT_ADD_MUL_3(2,4); break;
		case  8: DOT_ADD_MUL_3(3,1); break;
		case  9: DOT_ADD_MUL_3(3,2); break;
		case 10: DOT_ADD_MUL_3(3,3); break;
		case 11: DOT_ADD_MUL_3(3,4); break;
		case 12: DOT_ADD_MUL_3(4,1); break;
		case 13: DOT_ADD_MUL_3(4,2); break;
		case 14: DOT_ADD_MUL_3(4,3); break;
		case 15: DOT_ADD_MUL_3(4,4); break;
		} else switch (sj) {
#define DOT_ADD_MUL_2(sj) for (int i=0; i<chunk; i++) dot_add_mul<T,sj>(sk,buf2+sk*i,data+sj*i,(T *)*r);
		case 1: DOT_ADD_MUL_2(1); break;
		case 2: DOT_ADD_MUL_2(2); break;
		case 3: DOT_ADD_MUL_2(3); break;
		case 4: DOT_ADD_MUL_2(4); break;
		default:for (int i=0; i<chunk; i++) dot_add_mul(sk,sj,buf2+sk*i,data+sj*i,(T *)*r);
		}
		out->send(chunk*sk,buf2);
		n-=chunk*sj;
		data+=chunk*sj;
	}
    } else {
	while (n) {
		if (chunk*sj>n) chunk=n/sj;
		op_put->map(chunk*sk,buf2,*(T *)*seed);
		for (long i=0; i<sj; i++) {
			switch (sk) {
			case 1:  inner_child_b<T,1>(buf,data+i,sj,chunk); break;
			case 2:  inner_child_b<T,2>(buf,data+i,sj,chunk); break;
			case 3:  inner_child_b<T,3>(buf,data+i,sj,chunk); break;
			case 4:  inner_child_b<T,4>(buf,data+i,sj,chunk); break;
			default: inner_child_a(buf,data+i,sj,sk,chunk);
			}
			op->zip(chunk*sk,buf,(T *)*r2+i*off*sk);
			fold->zip(chunk*sk,buf2,buf);
		}
		out->send(chunk*sk,buf2);
		n-=chunk*sj;
		data+=chunk*sj;
	}
    }
} GRID_FINISH {
	r2=0;
} GRID_END

GRID_INPUT(1,r) {} GRID_END

\end class {install("#inner",2,1);}

/* **************************************************************** */
/*{ Dim[*As]<T>,Dim[*Bs]<T> -> Dim[*As,*Bs]<T> }*/
\class GridOuter : FObject {
	\attr Numop *op;
	PtrGrid r;
	\constructor (Numop *op, Grid *r=0) {
		this->op = op;
		this->r = r ? r : new Grid(new Dim(),int32_e,true);
	}
	\grin 0
	\grin 1
};

GRID_INLET(0) {
	SAME_TYPE(in,r);
	P<Dim> a = in->dim;
	P<Dim> b = r->dim;
	int n = a->n+b->n;
	int32 v[n];
	COPY(v,a->v,a->n);
	COPY(v+a->n,b->v,b->n);
	out=new GridOutlet(this,0,new Dim(n,v),in->nt);
} GRID_FLOW {
	long b_prod = r->dim->prod();
	if (!b_prod) return; /* nothing to do... and avoid deadly divisions by zero */
	if (b_prod > 4) {
		T buf[b_prod];
		while (n) {
			for (long j=0; j<b_prod; j++) buf[j] = *data;
			op->zip(b_prod,buf,(T *)*r);
			out->send(b_prod,buf);
			data++; n--;
		}
		return;
	}
	n*=b_prod;
	T buf[n];
	T buf2[b_prod*64];
	for (int i=0; i<64; i++) COPY(buf2+i*b_prod,(T *)*r,b_prod);
	switch (b_prod) {
	#define Z buf[k++]=data[i]
	case 1:	for (long i=0,k=0; k<n; i++) {Z;} break;
	case 2: for (long i=0,k=0; k<n; i++) {Z;Z;} break;
	case 3:	for (long i=0,k=0; k<n; i++) {Z;Z;Z;} break;
	case 4:	for (long i=0,k=0; k<n; i++) {Z;Z;Z;Z;} break;
	default:for (long i=0,k=0; k<n; i++) for (int j=0; j<b_prod; j++, k++) Z;
	}
	#undef Z
	int ch=64*b_prod;
	int nn=(n/ch)*ch;
	for (int j=0; j<nn; j+=ch) op->zip(ch,buf+j,buf2);
	op->zip(n-nn,buf+nn,buf2);
	out->send(n,buf);
} GRID_END

GRID_INPUT(1,r) {} GRID_END

\end class {install("#outer",2,1); add_creator("@outer");}

//****************************************************************
//{ Dim[]<T>,Dim[]<T>,Dim[]<T> -> Dim[A]<T> } or
//{ Dim[B]<T>,Dim[B]<T>,Dim[B]<T> -> Dim[*As,B]<T> }
\class GridFor : FObject {
	\attr PtrGrid from;
	\attr PtrGrid to;
	\attr PtrGrid step;
	\constructor (Grid *from, Grid *to, Grid *step) {
		this->from.constrain(expect_max_one_dim);
		this->to  .constrain(expect_max_one_dim);
		this->step.constrain(expect_max_one_dim);
		this->from=from;
		this->to  =to;
		this->step=step;
	}
	\decl 0 set (Grid *r=0);
	\decl 0 bang ();
	\grin 0 int
	\grin 1 int
	\grin 2 int
	template <class T> void trigger (T bogus);
};

template <class T>
void GridFor::trigger (T bogus) {
	int n = from->dim->prod();
	int32 nn[n+1];
	T x[64*n];
	T *fromb = (T *)*from;
	T *  tob = (T *)*to  ;
	T *stepb = (T *)*step;
	T to2[n];
	
	for (int i=step->dim->prod()-1; i>=0; i--)
		if (!stepb[i]) RAISE("step must not contain zeroes");
	for (int i=0; i<n; i++) {
		nn[i] = (tob[i] - fromb[i] + stepb[i] - cmp(stepb[i],(T)0)) / stepb[i];
		if (nn[i]<0) nn[i]=0;
		to2[i] = fromb[i]+stepb[i]*nn[i];
	}
	P<Dim> d;
	if (from->dim->n==0) { d = new Dim(*nn); }
	else { nn[n]=n;        d = new Dim(n+1,nn); }
	int total = d->prod();
	out=new GridOutlet(this,0,d,from->nt);
	if (total==0) return;
	int k=0;
	for(int d=0;;d++) {
		// here d is the dim# to reset; d=n for none
		for(;d<n;d++) x[k+d]=fromb[d];
		k+=n;
		if (k==64*n) {out->send(k,x); k=0; COPY(x,x+63*n,n);}
		else {                             COPY(x+k,x+k-n,n);}
		d--;
		// here d is the dim# to increment
		for(;;d--) {
			if (d<0) goto end;
			x[k+d]+=stepb[d];
			if (x[k+d]!=to2[d]) break;
		}
	}
	end: if (k) out->send(k,x);
}

\def 0 bang () {
	SAME_TYPE(from,to);
	SAME_TYPE(from,step);
	if (!from->dim->equal(to->dim) || !to->dim->equal(step->dim))
		RAISE("dimension mismatch: from:%s to:%s step:%s",
			from->dim->to_s(),to->dim->to_s(),step->dim->to_s());
#define FOO(T) trigger((T)0);
	TYPESWITCH_JUSTINT(from->nt,FOO,);
#undef FOO
}

\def 0 set (Grid *r) { from=new Grid(argv[0]); }
GRID_INPUT(2,step) {} GRID_END
GRID_INPUT(1,to) {} GRID_END
GRID_INPUT(0,from) {_0_bang(0,0);} GRID_END
\end class {install("#for",3,1); add_creator("@for");}

//****************************************************************
\class GridFinished : FObject {
	\constructor () {}
	\grin 0
};
GRID_INLET(0) {
	//in->set_mode(0);
} GRID_FINISH {
	outlet_bang(bself->outlets[0]);
} GRID_END
\end class {install("#finished",1,1); add_creator("@finished");}
\class GridDim : FObject {
	\constructor () {}
	\grin 0
};
GRID_INLET(0) {
	GridOutlet out(this,0,new Dim(in->dim->n));
	out.send(in->dim->n,in->dim->v);
	//in->set_mode(0);
} GRID_END
\end class {install("#dim",1,1); add_creator("@dim");}
\class GridType : FObject {
	\constructor () {}
	\grin 0
};
GRID_INLET(0) {
	outlet_symbol(bself->outlets[0],gensym(const_cast<char *>(number_type_table[in->nt].name)));
	/*in->set_mode(0);*/
} GRID_END
\end class {install("#type",1,1); add_creator("@type");}

//****************************************************************
//{ Dim[*As]<T>,Dim[B] -> Dim[*Cs]<T> }
\class GridRedim : FObject {
	\attr P<Dim> dim;
	PtrGrid dim_grid;
	PtrGrid temp; // temp->dim is not of the same shape as dim
	~GridRedim() {}
	\constructor (Grid *d) {
		dim_grid.constrain(expect_dim_dim_list);
		dim_grid=d;
		dim = dim_grid->to_dim();
	//	if (!dim->prod()) RAISE("target grid size must not be zero");
	}
	\grin 0
	\grin 1 int32
};

GRID_INLET(0) {
	long a=in->dim->prod(), b=dim->prod();
	if (a<b) temp=new Grid(new Dim(a),in->nt);
	out=new GridOutlet(this,0,dim,in->nt);
} GRID_FLOW {
	long i = dex;
	if (!temp) {
		long n2 = min(n,dim->prod()-i);
		if (n2>0) out->send(n2,data);
		// discard other values if any
	} else {
		long n2 = min(n,in->dim->prod()-i);
		COPY((T *)*temp+i,data,n2);
		if (n2>0) out->send(n2,data);
	}
} GRID_FINISH {
	if (!!temp) {
		long a = in->dim->prod(), b = dim->prod();
		if (a) {
			for (long i=a; i<b; i+=a) out->send(min(a,b-i),(T *)*temp);
		} else {
			T foo[1]={0}; for (long i=0; i<b; i++) out->send(1,foo);
		}
	}
	temp=0;
} GRID_END

GRID_INPUT(1,dim_grid) {
	P<Dim> d = dim_grid->to_dim();
//	if (!d->prod()) RAISE("target grid size must not be zero"); else post("d->prod=%d",d->prod());
	dim = d;
} GRID_END

\end class {install("#redim",2,1); add_creator("@redim");}

//****************************************************************
\class GridJoin : FObject {
	\attr int which_dim;
	PtrGrid r;
	\grin 0
	\grin 1
	\constructor (int which_dim=-1, Grid *r=0) {
		this->which_dim = which_dim;
		this->r=r;
	}
};

GRID_INLET(0) {
	NOTEMPTY(r);
	SAME_TYPE(in,r);
	P<Dim> d = in->dim;
	if (d->n != r->dim->n) RAISE("wrong number of dimensions");
	int w = which_dim;
	if (w<0) w+=d->n;
	if (w<0 || w>=d->n)
		RAISE("can't join on dim number %d on %d-dimensional grids",
			which_dim,d->n);
	int32 v[d->n];
	for (int i=0; i<d->n; i++) {
		v[i] = d->get(i);
		if (i==w) {
			v[i]+=r->dim->v[i];
		} else {
			if (v[i]!=r->dim->v[i]) RAISE("dimensions mismatch: dim #%i, left is %d, right is %d",i,v[i],r->dim->v[i]);
		}
	}
	out=new GridOutlet(this,0,new Dim(d->n,v),in->nt);
	in->set_chunk(w);
} GRID_FLOW {
	int w = which_dim;
	if (w<0) w+=in->dim->n;
	long a = in->dim->prod(w);
	long b = r->dim->prod(w);
	T *data2 = (T *)*r + dex*b/a;
	if (a==3 && b==1) {
		int m = n+n*b/a;
		T data3[m];
		T *data4 = data3;
		while (n) {
			SCOPY(data4,data,3); SCOPY(data4+3,data2,1);
			n-=3; data+=3; data2+=1; data4+=4;
		}
		out->send(m,data3);
	} else if (a+b<=16) {
		int m = n+n*b/a;
		T data3[m];
		int i=0;
		while (n) {
			COPY(data3+i,data,a); data+=a; i+=a; n-=a;
			COPY(data3+i,data2,b); data2+=b; i+=b;
		}
		out->send(m,data3);
	} else {
		while (n) {
			out->send(a,data);
			out->send(b,data2);
			data+=a; data2+=b; n-=a;
		}
	}
} GRID_FINISH {
	if (in->dim->prod()==0) out->send(r->dim->prod(),(T *)*r);
} GRID_END

GRID_INPUT(1,r) {} GRID_END

\end class {install("#join",2,1); add_creator("@join");}

//****************************************************************
\class GridGrade : FObject {
	\constructor () {}
	\grin 0
};

typedef int (*comparator_t)(const void *, const void *);

template <class T> struct GradeFunction {
	static int comparator (T **a, T **b) {return **a-**b;}};
#define FOO(T) \
template <> struct GradeFunction<T> { \
	static int comparator (T **a, T **b) {T x = **a-**b; return x<0 ? -1 : x>0;}};
FOO(int64)
FOO(float32)
FOO(float64)
#undef FOO

GRID_INLET(0) {
	out=new GridOutlet(this,0,in->dim);
	in->set_chunk(in->dim->n-1);
} GRID_FLOW {
	long m = in->dim->prod(in->dim->n-1);
	T *foo[m];
	T  bar[m];
	for (; n; n-=m,data+=m) {
		for (int i=0; i<m; i++) foo[i] = &data[i];
		qsort(foo,m,sizeof(T *),(comparator_t)GradeFunction<T>::comparator);
		for (int i=0; i<m; i++) bar[i] = foo[i]-(T *)data;
		out->send(m,bar);
	}
} GRID_END

\end class {install("#grade",1,1); add_creator("@grade");}

//****************************************************************
//\class GridMedian : FObject
//****************************************************************

\class GridTranspose : FObject {
	\attr int dim1;
	\attr int dim2;
	int d1,d2,na,nb,nc,nd; // temporaries
	\constructor (int dim1=0, int dim2=1) {
		this->dim1 = dim1;
		this->dim2 = dim2;
	}
	\decl 1 float (int dim1);
	\decl 2 float (int dim2);
	\grin 0
};

\def 1 float (int dim1) { this->dim1=dim1; }
\def 2 float (int dim2) { this->dim2=dim2; }

GRID_INLET(0) {
	int32 v[in->dim->n];
	COPY(v,in->dim->v,in->dim->n);
	d1=dim1; d2=dim2;
	if (d1<0) d1+=in->dim->n;
	if (d2<0) d2+=in->dim->n;
	if (d1>=in->dim->n || d2>=in->dim->n || d1<0 || d2<0)
		RAISE("would swap dimensions %d and %d but this grid has only %d dimensions", dim1,dim2,in->dim->n);
	memswap(v+d1,v+d2,1);
	if (d1==d2) {
		out=new GridOutlet(this,0,new Dim(in->dim->n,v), in->nt);
	} else {
		nd = in->dim->prod(1+max(d1,d2));
		nc = in->dim->v[max(d1,d2)];
		nb = nc&&nd ? in->dim->prod(1+min(d1,d2))/nc/nd : 0;
		na = in->dim->v[min(d1,d2)];
		out=new GridOutlet(this,0,new Dim(in->dim->n,v), in->nt);
		in->set_chunk(min(d1,d2));
	}
	// Turns a Grid[*,na,*nb,nc,*nd] into a Grid[*,nc,*nb,na,*nd].
} GRID_FLOW {
	//T res[na*nb*nc*nd];
	T *res = NEWBUF(T,na*nb*nc*nd);
	if (dim1==dim2) { out->send(n,data); return; }
	int prod = na*nb*nc*nd;
	for (; n; n-=prod, data+=prod) {
		for (long a=0; a<na; a++)
			for (long b=0; b<nb; b++)
				for (long c=0; c<nc; c++)
					COPY(res +((c*nb+b)*na+a)*nd,
					     data+((a*nb+b)*nc+c)*nd,nd);
		out->send(na*nb*nc*nd,res);
	}
	DELBUF(res); //!@#$ if an exception was thrown by out->send, this never gets done
} GRID_END

\end class {install("#transpose",3,1); add_creator("@transpose");}

//****************************************************************
\class GridReverse : FObject {
	\attr int dim1; // dimension to act upon
	int d; // temporaries
	\constructor (int dim1=0) {this->dim1 = dim1;}
	\decl 1 float (int dim1);
	\grin 0
};

\def 1 float (int dim1) { this->dim1=dim1; }

GRID_INLET(0) {
	d=dim1;
	if (d<0) d+=in->dim->n;
	if (d>=in->dim->n || d<0)
		RAISE("would reverse dimension %d but this grid has only %d dimensions",
			dim1,in->dim->n);
	out=new GridOutlet(this,0,new Dim(in->dim->n,in->dim->v), in->nt);
	in->set_chunk(d);
} GRID_FLOW {
	long f1=in->dim->prod(d), f2=in->dim->prod(d+1);
	while (n) {
		long hf1=f1/2;
		T *data2 = data+f1-f2;
		for (long i=0; i<hf1; i+=f2) memswap(data+i,data2-i,f2);
		out->send(f1,data);
		data+=f1; n-=f1;
	}
} GRID_END

\end class {install("#reverse",2,1);}

//****************************************************************
\class GridCentroid : FObject {
	\constructor () {}
	\grin 0 int
	int sumx,sumy,sum,y; // temporaries
};

GRID_INLET(0) {
	if (in->dim->n != 3) RAISE("expecting 3 dims");
	if (in->dim->v[2] != 1) RAISE("expecting 1 channel");
	in->set_chunk(1);
	out=new GridOutlet(this,0,new Dim(2), in->nt);
	sumx=0; sumy=0; sum=0; y=0;
} GRID_FLOW {
	int sx = in->dim->v[1];
	while (n) {
		for (int x=0; x<sx; x++) {
			sumx+=x*data[x];
			sumy+=y*data[x];
			sum +=  data[x];
		}
		n-=sx;
		data+=sx;
		y++;
	}
} GRID_FINISH {
	int32 blah[2];
	blah[0] = sum ? sumy/sum : 0;
	blah[1] = sum ? sumx/sum : 0;
	out->send(2,blah);
	outlet_float(bself->outlets[1],blah[0]);
	outlet_float(bself->outlets[2],blah[1]);
} GRID_END

\end class {install("#centroid",1,3);}

//****************************************************************
static void expect_pair (P<Dim> dim) {if (dim->prod()!=2) RAISE("expecting only two numbers. Dim(2)");}

\class GridMoment : FObject {
	\constructor (int order=1) {
		offset.constrain(expect_pair);
		//t_atom2 a[2] = {t_atom2(0),t_atom2(0)};
		t_atom a[2]; SETFLOAT(a,0); SETFLOAT(a+1,0);
		offset=new Grid(2,a,int32_e);
		if (order!=1 && order!=2) RAISE("supports only orders 1 and 2 for now");
		this->order=order;
	}
	\grin 0 int
	\grin 1 int
	\attr int order; // order
	\attr PtrGrid offset;
	int64 sumy,sumxy,sumx,sum,y; // temporaries
};

GRID_INLET(0) {
	if (in->dim->n != 3) RAISE("expecting 3 dims");
	if (in->dim->v[2] != 1) RAISE("expecting 1 channel");
	in->set_chunk(1);
	switch (order) {
	    case 1: out=new GridOutlet(this,0,new Dim(2  ), in->nt); break;
	    case 2: out=new GridOutlet(this,0,new Dim(2,2), in->nt); break;
	    default: RAISE("supports only orders 1 and 2 for now");
	}
	sumx=0; sumy=0; sumxy=0; sum=0; y=0;
} GRID_FLOW {
	int sx = in->dim->v[1];
	int oy = ((int*)*offset)[0];
	int ox = ((int*)*offset)[1];
	while (n) {
		switch (order) {
		    case 1:
			for (int x=0; x<sx; x++) {
				sumy+=y*data[x];
				sumx+=x*data[x];
				sum +=  data[x];
			}
		    break;
		    case 2:
			for (int x=0; x<sx; x++) {
				int ty=y-oy;
				int tx=x-ox;
				sumy +=ty*ty*data[x];
				sumxy+=tx*ty*data[x];
				sumx +=tx*tx*data[x];
				sum  +=      data[x];
			}
		}
		n-=sx;
		data+=sx;
		y++;
	}
} GRID_FINISH {
	int32 blah[4];
	switch (order) {
	    case 1: /* centroid vector */
		blah[0] = sum ? sumy/sum : 0;
		blah[1] = sum ? sumx/sum : 0;
		out->send(2,blah);
	    break;
	    case 2: /* covariance matrix */
		blah[0] = sum ? sumy/sum : 0;
		blah[1] = sum ? sumxy/sum : 0;
		blah[2] = sum ? sumxy/sum : 0;
		blah[3] = sum ? sumx/sum : 0;
		out->send(4,blah);
	    break;
	}
} GRID_END

GRID_INPUT(1,offset) {} GRID_END

\end class {install("#moment",2,1);}

//****************************************************************
\class GridLabelling : FObject {
	\grin 0
	\attr int form();
	\attr int form_val;
	\constructor (int form=0) {form_val=form; initialize3();}
	void initialize3() {bself->noutlets_set(form_val ? 2 : 4);}
};
struct Stats {
	int64 yy,yx,xx,y,x,area;
	int64 x1,x2;
	Stats() {yy=yx=xx=y=x=area=0;}
};
#define AT(y,x) dat[(y)*sx+(x)]
template <class T> void flood_fill(T *dat, int sy, int sx, int y, int x, Stats *stat, int label, int form) {
	/* find x1,x2 such that all the x of that horizontal segment are x1<=x<x2 */
	int x2; for (x2=x; x2<sx; x2++) if (AT(y,x2)!=1) break;
	int x1; for (x1=x; x1>=0; x1--) if (AT(y,x1)!=1) break;
	x1++;
	if (form==0) {
		for (x=x1; x<x2; x++) {
			AT(y,x)=label;
			stat->yy += y*y; stat->y += y;
			stat->yx += y*x; stat->area++;
			stat->xx += x*x; stat->x += x;
		}
		for (x=x1; x<x2; x++) {
			if (y>0    && AT(y-1,x)==1) flood_fill(dat,sy,sx,y-1,x,stat,label,form);
			if (y<sy-1 && AT(y+1,x)==1) flood_fill(dat,sy,sx,y+1,x,stat,label,form);
		}
	} else {
		for (x=x1; x<x2; x++) {
			AT(y,x)=label;
		}
		stat->y=y;
		stat->x1=x1;
		stat->x2=x2;
	}
}

GRID_INLET(0) {
	if (in->dim->n<2 || in->dim->prod(2)!=1) RAISE("requires dim (y,x) or (y,x,1)");
	in->set_chunk(0);
} GRID_FLOW {
	int sy=in->dim->v[0], sx=in->dim->v[1];
	T *dat = NEWBUF(T,n);
	for (int i=0; i<n; i++) dat[i]=data[i];
	int y,x=0,label=2;
	for (y=0; y<sy; y++) for (x=0; x<sx; x++) {
		if (dat[y*sx+x]!=1) continue;
		Stats s;
		flood_fill(dat,sy,sx,y,x,&s,label,form_val);
		if (form_val==0) {
			float32 cooked[6] = {
				(s.yy-s.y*s.y/s.area)/s.area,
				(s.yx-s.y*s.x/s.area)/s.area,
				(s.yx-s.y*s.x/s.area)/s.area,
				(s.xx-s.x*s.x/s.area)/s.area,
				s.y/s.area,
				s.x/s.area};
			float a[] = {s.area};
			send_out(3,1,a);
			GridOutlet o2(this,2,new Dim(2));   o2.send(2,cooked+4);
			GridOutlet o1(this,1,new Dim(2,2)); o1.send(4,cooked);
		} else {
			float32 cooked[4] = {s.y,s.x1,s.y,s.x2};
			GridOutlet o1(this,1,new Dim(2,2)); o1.send(4,cooked);
		}
		label++;
	}
	out = new GridOutlet(this,0,new Dim(sy,sx,1),in->nt);
	out->send(n,dat);
	DELBUF(dat);
} GRID_END

\def int form() {return form_val;}
\def 0 form(int form) {
	if (form<0 || form>1) RAISE("form must be 0 or 1, not %d",form);
	form_val=form;
	initialize3();
}
\end class {install("#labelling",1,0); add_creator("#labeling");}

//****************************************************************
\class GridPerspective : FObject {
	\attr int32 z;
	\grin 0
	\constructor (int32 z=256) {this->z=z;}
};
GRID_INLET(0) {
	int n = in->dim->n;
	int32 v[n];
	COPY(v,in->dim->v,n);
	v[n-1]--;
	in->set_chunk(in->dim->n-1);
	out=new GridOutlet(this,0,new Dim(n,v),in->nt);
} GRID_FLOW {
	int m = in->dim->prod(in->dim->n-1);
	for (; n; n-=m,data+=m) {
		op_mul->map(m-1,data,(T)z);
		op_div->map(m-1,data,data[m-1]);
		out->send(m-1,data);
	}	
} GRID_END
\end class {install("#perspective",1,1); add_creator("@perspective");}

//****************************************************************
\class GridBorder : FObject {
	\attr P<Dim> diml;
	\attr P<Dim> dimr;
	PtrGrid diml_grid;
	PtrGrid dimr_grid;
	\grin 0
	\grin 1 int
	\grin 2 int
	\constructor (Grid *dl=0, Grid *dr=0) {
		t_atom a[2]; SETFLOAT(a+0,1); SETFLOAT(a+1,1); SETFLOAT(a+2,0);
		diml_grid=dl?dl:new Grid(3,a,int32_e);
		dimr_grid=dr?dr:new Grid(3,a,int32_e);
		diml = diml_grid->to_dim();
		dimr = dimr_grid->to_dim();
	}
};

GRID_INLET(0) {
	int n = in->dim->n;
	if (n!=3) RAISE("only 3 dims supported for now");
	if (diml->n != n) RAISE("diml mismatch");
	if (dimr->n != n) RAISE("dimr mismatch");
	if (diml->v[2] || dimr->v[2]) RAISE("can't augment channels (todo)");
	int32 v[n];
	for (int i=0; i<n; i++) v[i]=in->dim->v[i]+diml->v[i]+dimr->v[i];
	in->set_chunk(0);
	out=new GridOutlet(this,0,new Dim(n,v),in->nt);
} GRID_FLOW {
	int sy = in->dim->v[0];
	int sx = in->dim->v[1]; int zx = sx+diml->v[1]+dimr->v[1];
	int sc = in->dim->v[2]; int zc = sc+diml->v[2]+dimr->v[2];
	int sxc = sx*sc; int zxc = zx*zc;
	int32 duh[zxc];
	for (int x=0; x<zxc; x++) duh[x]=0;
	for (int y=0; y<diml->v[0]; y++) out->send(zxc,duh);
	for (int y=0; y<sy; y++) {
		out->send(diml->v[1]*sc,duh);
		out->send(sxc,data+y*sxc);
		out->send(dimr->v[1]*sc,duh);
	}	
	for (int i=0; i<dimr->v[0]; i++) out->send(zxc,duh);
} GRID_END

GRID_INPUT(1,diml_grid) {diml = diml_grid->to_dim();} GRID_END
GRID_INPUT(2,dimr_grid) {dimr = dimr_grid->to_dim();} GRID_END

\end class {install("#border",3,1);}

static void expect_picture (P<Dim> d) {
	if (d->n!=3) RAISE("(height,width,chans) dimensions please");}
static void expect_rgb_picture (P<Dim> d) {
	expect_picture(d);
	if (d->get(2)!=3) RAISE("(red,green,blue) channels please");}
static void expect_rgba_picture (P<Dim> d) {
	expect_picture(d);
	if (d->get(2)!=4) RAISE("(red,green,blue,alpha) channels please");}

//****************************************************************
//{ Dim[A,B,*Cs]<T>,Dim[D,E]<T> -> Dim[A,B,*Cs]<T> }

static void expect_convolution_matrix (P<Dim> d) {
	if (d->n != 2) RAISE("only exactly two dimensions allowed for now (got %d)",
		d->n);
}

// entry in a compiled convolution kernel
struct PlanEntry {long y,x; bool neutral;};

\class GridConvolve : FObject {
	\attr Numop *op;
	\attr Numop *fold;
	\attr PtrGrid seed;
	\attr PtrGrid b;
	\attr bool wrap;
	\attr bool anti;
	PtrGrid a;
	int plann;
	PlanEntry *plan;
	int margx,margy; // margins
	\constructor (Grid *r=0) {
		plan=0;
		b.constrain(expect_convolution_matrix); plan=0;
		this->op = op_mul;
		this->fold = op_add;
		this->seed = new Grid(new Dim(),int32_e,true);
		this->b= r ? r : new Grid(new Dim(1,1),int32_e,true);
		this->wrap = true;
		this->anti = true;
	}
	\grin 0
	\grin 1
	template <class T> void copy_row (T *buf, long sx, long y, long x);
	template <class T> void make_plan (T bogus);
	~GridConvolve () {if (plan) delete[] plan;}
};

template <class T> void GridConvolve::copy_row (T *buf, long sx, long y, long x) {
	long day = a->dim->get(0), dax = a->dim->get(1), dac = a->dim->prod(2);
	y=mod(y,day); x=mod(x,dax);
	T *ap = (T *)*a + y*dax*dac;
	while (sx) {
		long sx1 = min(sx,dax-x);
		COPY(buf,ap+x*dac,sx1*dac);
		x=0;
		buf += sx1*dac;
		sx -= sx1;
	}
}

template <class T> void GridConvolve::make_plan (T bogus) {
	P<Dim> da = a->dim, db = b->dim;
	long dby = db->get(0);
	long dbx = db->get(1);
	if (plan) delete[] plan;
	plan = new PlanEntry[dbx*dby];
	long i=0;
	for (long y=0; y<dby; y++) {
		for (long x=0; x<dbx; x++) {
			long k = anti ? y*dbx+x : (dby-1-y)*dbx+(dbx-1-x);
			T rh = ((T *)*b)[k];
			bool neutral   = op->on(rh)->is_neutral(  rh,at_right);
			bool absorbent = op->on(rh)->is_absorbent(rh,at_right);
			T foo[1]={0};
			if (absorbent) {
				op->map(1,foo,rh);
				absorbent = fold->on(rh)->is_neutral(foo[0],at_right);
			}
			if (absorbent) continue;
			plan[i].y = y;
			plan[i].x = x;
			plan[i].neutral = neutral;
			i++;
		}
	}
	plann = i;
}

GRID_INLET(0) {
	SAME_TYPE(in,b);
	SAME_TYPE(in,seed);
	P<Dim> da=in->dim, db=b->dim;
	if (!db) RAISE("right inlet has no grid");
	if (!seed) RAISE("seed missing");
	if (db->n != 2) RAISE("right grid must have two dimensions");
	if (da->n < 2) RAISE("left grid has less than two dimensions");
	if (seed->dim->n != 0) RAISE("seed must be scalar");
	if (da->get(0) < db->get(0)) RAISE("grid too small (y): %d < %d", da->get(0), db->get(0));
	if (da->get(1) < db->get(1)) RAISE("grid too small (x): %d < %d", da->get(1), db->get(1));
	margy = (db->get(0)-1)/2;
	margx = (db->get(1)-1)/2;
	//if (a) post("for %p, a->dim=%s da=%s",this,a->dim->to_s(),da->to_s());
	if (!a || !a->dim->equal(da)) a=new Grid(da,in->nt); // with this condition it's 2% faster on Linux but takes more RAM.
	//a=new Grid(da,in->nt); // with this condition it's 2% faster but takes more RAM.
	int v[da->n]; COPY(v,da->v,da->n);
	if (!wrap) {v[0]-=db->v[0]-1; v[1]-=db->v[1]-1;}
	out=new GridOutlet(this,0,new Dim(da->n,v),in->nt);
} GRID_FLOW {
	COPY((T *)*a+dex, data, n);
} GRID_FINISH {
	make_plan((T)0);
	long dbx = b->dim->get(1);
	long dby = b->dim->get(0);
	long day = out->dim->get(0);
	long n =   out->dim->prod(1);
	long sx =  out->dim->get(1)+dbx-1;
	long sxc = out->dim->prod(2)*sx;
	T buf[n];
	T buf2[sxc];
	T orh=0;
	for (long ay=0; ay<day; ay++) {
		op_put->map(n,buf,*(T *)*seed);
		for (long i=0; i<plann; i++) {
			long by = plan[i].y;
			long bx = plan[i].x;
			long k = anti ? by*dbx+bx : (dby-1-by)*dbx+(dbx-1-bx);
			T rh = ((T *)*b)[k];
			if (i==0 || by!=plan[i-1].y || orh!=rh) {
				if (wrap) copy_row(buf2,sx,ay+by-margy,-margx);
				else      copy_row(buf2,sx,ay+by,0);
				if (!plan[i].neutral) op->map(sxc,buf2,rh);
			}
			fold->zip(n,buf,buf2+bx*out->dim->prod(2));
			orh=rh;
		}
		out->send(n,buf);
	}
	//a=0; // comment this out when trying to recycle a (use the dim->equal above)
} GRID_END

GRID_INPUT(1,b) {} GRID_END

\end class {install("#convolve",2,1);}

/* ---------------------------------------------------------------- */
/* "#scale_by" does quick scaling of pictures by integer factors */
/*{ Dim[A,B,3]<T> -> Dim[C,D,3]<T> }*/

static void expect_scale_factor (P<Dim> dim) {
	if (dim->prod()!=1 && dim->prod()!=2)
		RAISE("expecting only one or two numbers");
}

\class GridScaleBy : FObject {
	\attr PtrGrid scale; // integer scale factor
	int scaley;
	int scalex;
	\constructor (Grid *factor=0) {
		scale.constrain(expect_scale_factor);
		t_atom a[1]; SETFLOAT(a,2);
		scale = factor?factor:new Grid(1,a,int32_e);
		prepare_scale_factor();
	}
	\grin 0
	\grin 1
	void prepare_scale_factor () {
		scaley = ((int32 *)*scale)[0];
		scalex = ((int32 *)*scale)[scale->dim->prod()==1 ? 0 : 1];
		if (scaley<1) scaley=2;
		if (scalex<1) scalex=2;
	}
};

GRID_INLET(0) {
	P<Dim> a = in->dim;
	expect_picture(a);
	out=new GridOutlet(this,0,new Dim(a->get(0)*scaley,a->get(1)*scalex,a->get(2)),in->nt);
	in->set_chunk(1);
} GRID_FLOW {
	int rowsize = in->dim->prod(1);
	T buf[rowsize*scalex];
	int chans = in->dim->get(2);
	#define Z(z) buf[p+z]=data[i+z]
	for (; n>0; data+=rowsize, n-=rowsize) {
		int p=0;
		#define LOOP(z) \
			for (int i=0; i<rowsize; i+=z) \
			for (int k=0; k<scalex; k++, p+=z)
		switch (chans) {
		case 3: LOOP(3) {Z(0);Z(1);Z(2);} break;
		case 4: LOOP(4) {Z(0);Z(1);Z(2);Z(3);} break;
		default: LOOP(chans) {for (int c=0; c<chans; c++) Z(c);}
		}
		#undef LOOP
		for (int j=0; j<scaley; j++) out->send(rowsize*scalex,buf);
	}
	#undef Z
} GRID_END

GRID_INPUT(1,scale) {prepare_scale_factor();} GRID_END

\end class {install("#scale_by",2,1); add_creator("@scale_by");}

// ----------------------------------------------------------------
//{ Dim[A,B,3]<T> -> Dim[C,D,3]<T> }
\class GridDownscaleBy : FObject {
	\attr PtrGrid scale;
	\attr bool smoothly;
	int scaley;
	int scalex;
	PtrGrid temp;
	\constructor (Grid *factor=0, t_symbol *option=0) {
		scale.constrain(expect_scale_factor);
		t_atom a[1]; SETFLOAT(a,2);
		scale = factor?factor:new Grid(1,a,int32_e);
		prepare_scale_factor();
		smoothly = option==gensym("smoothly");
	}
	\grin 0
	\grin 1
	void prepare_scale_factor () {
		scaley = ((int32 *)*scale)[0];
		scalex = ((int32 *)*scale)[scale->dim->prod()==1 ? 0 : 1];
		if (scaley<1) scaley=2;
		if (scalex<1) scalex=2;
	}
};

GRID_INLET(0) {
	P<Dim> a = in->dim;
	if (a->n!=3) RAISE("(height,width,chans) please");
	out=new GridOutlet(this,0,new Dim(a->get(0)/scaley,a->get(1)/scalex,a->get(2)),in->nt);
	in->set_chunk(1);
	// i don't remember why two rows instead of just one.
	temp=new Grid(new Dim(2,in->dim->get(1)/scalex,in->dim->get(2)),in->nt);
} GRID_FLOW {
	int rowsize = in->dim->prod(1);
	int rowsize2 = temp->dim->prod(1);
	T *buf = (T *)*temp; //!@#$ maybe should be something else than T ?
	int xinc = in->dim->get(2)*scalex;
	int y = dex / rowsize;
	int chans=in->dim->get(2);
	#define Z(z) buf[p+z]+=data[i+z]
	if (smoothly) {
		while (n>0) {
			if (y%scaley==0) CLEAR(buf,rowsize2);
			#define LOOP(z) \
				for (int i=0,p=0; p<rowsize2; p+=z) \
				for (int j=0; j<scalex; j++,i+=z)
			switch (chans) {
			case 1: LOOP(1) {Z(0);} break;
			case 2: LOOP(2) {Z(0);Z(1);} break;
			case 3: LOOP(3) {Z(0);Z(1);Z(2);} break;
			case 4: LOOP(4) {Z(0);Z(1);Z(2);Z(3);} break;
			default:LOOP(chans) {for (int k=0; k<chans; k++) Z(k);} break;
			}
			#undef LOOP
			y++;
			if (y%scaley==0 && out->dim) {
				op_div->map(rowsize2,buf,(T)(scalex*scaley));
				out->send(rowsize2,buf);
				CLEAR(buf,rowsize2);
			}
			data+=rowsize;
			n-=rowsize;
		}
	#undef Z
	} else {
	#define Z(z) buf[p+z]=data[i+z]
		for (; n>0 && out->dim; data+=rowsize, n-=rowsize,y++) {
			if (y%scaley!=0) continue;
			#define LOOP(z) for (int i=0,p=0; p<rowsize2; i+=xinc, p+=z)
			switch(in->dim->get(2)) {
			case 1: LOOP(1) {Z(0);} break;
			case 2: LOOP(2) {Z(0);Z(1);} break;
			case 3: LOOP(3) {Z(0);Z(1);Z(2);} break;
			case 4: LOOP(4) {Z(0);Z(1);Z(2);Z(3);} break;
			default:LOOP(chans) {for (int k=0; k<chans; k++) Z(k);}break;
			}
			#undef LOOP
			out->send(rowsize2,buf);
		}
	}
	#undef Z
} GRID_END

GRID_INPUT(1,scale) {prepare_scale_factor();} GRID_END

\end class {install("#downscale_by",2,1); add_creator("@downscale_by");}

//****************************************************************
\class GridLayer : FObject {
	PtrGrid r;
	\constructor () {r.constrain(expect_rgb_picture);}
	\grin 0 int
	\grin 1 int
};

GRID_INLET(0) {
	NOTEMPTY(r);
	SAME_TYPE(in,r);
	P<Dim> a = in->dim;
	expect_rgba_picture(a);
	if (a->get(1)!=r->dim->get(1)) RAISE("same width please");
	if (a->get(0)!=r->dim->get(0)) RAISE("same height please");
	in->set_chunk(2);
	out=new GridOutlet(this,0,r->dim);
} GRID_FLOW {
	T *rr = ((T *)*r) + dex*3/4;
	T foo[n*3/4];
#define COMPUTE_ALPHA(c,a) \
	foo[j+c] = (data[i+c]*data[i+a] + rr[j+c]*(256-data[i+a])) >> 8
	for (int i=0,j=0; i<n; i+=4,j+=3) {
		COMPUTE_ALPHA(0,3);
		COMPUTE_ALPHA(1,3);
		COMPUTE_ALPHA(2,3);
	}
#undef COMPUTE_ALPHA
	out->send(n*3/4,foo);
} GRID_END

GRID_INPUT(1,r) {} GRID_END

\end class {install("#layer",2,1); add_creator("@layer");}

// ****************************************************************
// pad1,pad2 only are there for 32-byte alignment
struct Line {int32 y1,x1,y2,x2,x,m,ox,pad2;};

static void expect_polygon (P<Dim> d) {
	if (d->n!=2 || d->get(1)!=2) RAISE("expecting Dim[n,2] polygon");
}

enum DrawMode {DRAW_FILL,DRAW_LINE,DRAW_POINT};
enum OmitMode {OMIT_NONE,OMIT_LAST,OMIT_ODD};
DrawMode convert(const t_atom &x, DrawMode *foo) {
	t_symbol *s = convert(x,(t_symbol **)0);
	if (s==gensym("fill")) return DRAW_FILL;
	if (s==gensym("line")) return DRAW_LINE;
	if (s==gensym("point")) return DRAW_POINT;
	RAISE("unknown DrawMode '%s' (want fill or line)",s->s_name);
}
OmitMode convert(const t_atom &x, OmitMode *foo) {
	t_symbol *s = convert(x,(t_symbol **)0);
	if (s==gensym("none")) return OMIT_NONE;
	if (s==gensym("last")) return OMIT_LAST;
	if (s==gensym("odd"))  return OMIT_ODD;
	RAISE("unknown OmitMode '%s' (want none or last or odd)",s->s_name);
}
\class DrawPolygon : FObject {
	\attr Numop *op;
	\attr PtrGrid color;
	\attr PtrGrid polygon;
	\attr DrawMode draw;
	\attr OmitMode omit;
	PtrGrid color2;
	PtrGrid lines;
	int lines_start;
	int lines_stop;
	\constructor (Numop *op=op_put, Grid *color=0, Grid *polygon=0) {
		draw=DRAW_FILL;
		omit=OMIT_NONE;
		this->color.constrain(expect_max_one_dim);
		this->polygon.constrain(expect_polygon);
		this->op = op;
		if (color) this->color=color;
		if (polygon) {this->polygon=polygon; init_lines();}
	}
	\grin 0
	\grin 1
	\grin 2 int32
	void init_lines();
	void changed(t_symbol *s) {init_lines();}
};
void DrawPolygon::init_lines () {
	if (!polygon) return;
	int tnl = polygon->dim->get(0);
	int nl = omit==OMIT_LAST ? tnl-1 : omit==OMIT_ODD ? (tnl+1)/2 : tnl;
	lines=new Grid(new Dim(nl,8), int32_e);
	Line *ld = (Line *)(int32 *)*lines;
	int32 *pd = *polygon;
	for (int i=0,j=0; i<nl; i++) {
		ld[i].y1 = pd[j+0];
		ld[i].x1 = pd[j+1];
		j=(j+2)%(2*tnl);
		ld[i].y2 = pd[j+0];
		ld[i].x2 = pd[j+1];
		if (omit==OMIT_ODD) j=(j+2)%(2*tnl);
		if (draw!=DRAW_POINT) if (ld[i].y1>ld[i].y2) memswap((int32 *)(ld+i)+0,(int32 *)(ld+i)+2,2);
		long dy = ld[i].y2-ld[i].y1;
		long dx = ld[i].x2-ld[i].x1;
		ld[i].m = dy ? (dx<<16)/dy : 0;
	}
}

static int order_by_starting_scanline (const void *a, const void *b) {
	return ((Line *)a)->y1 - ((Line *)b)->y1;
}

static int order_by_column (const void *a, const void *b) {
	return ((Line *)a)->x - ((Line *)b)->x;
}

GRID_INLET(0) {
	NOTEMPTY(color);
	NOTEMPTY(polygon);
	NOTEMPTY(lines);
	SAME_TYPE(in,color);
	if (in->dim->n!=3) RAISE("expecting 3 dimensions");
	if (in->dim->get(2)!=color->dim->get(0)) RAISE("image does not have same number of channels as stored color");
	out=new GridOutlet(this,0,in->dim,in->nt);
	lines_start = lines_stop = 0;
	in->set_chunk(1);
	int nl = lines->dim->get(0);
	qsort((int32 *)*lines,nl,sizeof(Line),order_by_starting_scanline);
	int cn = color->dim->prod();
	color2=new Grid(new Dim(cn*16), color->nt);
	for (int i=0; i<16; i++) COPY((T *)*color2+cn*i,(T *)*color,cn);
} GRID_FLOW {
	int nl = lines->dim->get(0);
	Line *ld = (Line *)(int32 *)*lines;
	int f = in->dim->prod(1);
	int y = dex/f;
	int cn = color->dim->prod();
	T *cd = (T *)*color2;
	while (n) {
		while (lines_stop != nl && ld[lines_stop].y1<=y) {
			Line &l = ld[lines_stop];
			l.x = l.x1 + (((y-l.y1)*l.m)>>16);
			lines_stop++;
		}
		if (draw!=DRAW_POINT) {
			int fudge = draw==DRAW_FILL?0:1;
			for (int i=lines_start; i<lines_stop; i++) {
				if (ld[i].y2<=y-fudge) {memswap(ld+i,ld+lines_start,1); lines_start++;}
			}
		}
		if (lines_start == lines_stop) {
			out->send(f,data);
		} else {
			int32 xl = in->dim->get(1);
			T data2[f];
			COPY(data2,data,f);
			for (int i=lines_start; i<lines_stop; i++) {
				Line &l = ld[i];
				l.ox = l.x;
				l.x = l.x1 + (((y-l.y1)*l.m)>>16);
			}
			if (draw!=DRAW_POINT) qsort(ld+lines_start,lines_stop-lines_start,sizeof(Line),order_by_column);
			if (draw==DRAW_FILL) {
				for (int i=lines_start; i<lines_stop-1; i+=2) {
					int xs = max(ld[i].x,(int32)0);
					int xe = min(ld[i+1].x,xl);
					if (xs>=xe) continue; /* !@#$ WHAT? */
					while (xe-xs>=16) {op->zip(16*cn,data2+cn*xs,cd); xs+=16;}
					op->zip((xe-xs)*cn,data2+cn*xs,cd);
				}
			} else if (draw==DRAW_LINE) {
				for (int i=lines_start; i<lines_stop; i++) {
					if (ld[i].y1==ld[i].y2) ld[i].ox=ld[i].x2;
					int xs = min(ld[i].x,ld[i].ox);
					int xe = max(ld[i].x,ld[i].ox);
					if (xs==xe) xe++;
					if ((xs<0 && xe<0) || (xs>=xl && xe>=xl)) continue;
					xs = max(0,xs);
					xe = min(xl,xe);
					while (xe-xs>=16) {op->zip(16*cn,data2+cn*xs,cd); xs+=16;}
					op->zip((xe-xs)*cn,data2+cn*xs,cd);
				}
			} else {
				for (int i=lines_start; i<lines_stop; i++) {
					if (y!=ld[i].y1) continue;
					int xs=ld[i].x1;
					int xe=xs+1;
					if (xs<0 || xs>=xl) continue;
					op->zip((xe-xs)*cn,data2+cn*xs,cd);
				}
				lines_start=lines_stop;
			}
			out->send(f,data2);
		}
		n-=f;
		data+=f;
		y++;
	}
} GRID_END


GRID_INPUT(1,color) {} GRID_END
GRID_INPUT(2,polygon) {init_lines();} GRID_END

\end class {install("#draw_polygon",3,1); add_creator("@draw_polygon");}

//****************************************************************
static void expect_position(P<Dim> d) {
	if (d->n!=1) RAISE("position should have 1 dimension, not %d", d->n);
	if (d->v[0]!=2) RAISE("position dim 0 should have 2 elements, not %d", d->v[0]);
}

\class DrawImage : FObject {
	\attr Numop *op;
	\attr PtrGrid image;
	\attr PtrGrid position;
	\attr bool alpha;
	\attr bool tile;
	\constructor (Numop *op=op_put, Grid *image=0, Grid *position=0) {
		alpha=false; tile=false;
		this->op = op;
		this->position.constrain(expect_position);
		this->image.constrain(expect_picture);
		if (image) this->image=image;
		if (position) this->position=position;
		else this->position=new Grid(new Dim(2),int32_e,true);
	}
	\grin 0
	\grin 1
	\grin 2 int32
	// draw row # ry of right image in row buffer buf, starting at xs
	// overflow on both sides has to be handled automatically by this method
	template <class T> void draw_segment(T *obuf, T *ibuf, int ry, int x0);
};

#define COMPUTE_ALPHA(c,a) obuf[j+(c)] = ibuf[j+(c)] + (rbuf[a])*(obuf[j+(c)]-ibuf[j+(c)])/256;
#define COMPUTE_ALPHA4(b) \
	COMPUTE_ALPHA(b+0,b+3); \
	COMPUTE_ALPHA(b+1,b+3); \
	COMPUTE_ALPHA(b+2,b+3); \
	obuf[b+3] = rbuf[b+3] + (255-rbuf[b+3])*(ibuf[j+b+3])/256;

template <class T> void DrawImage::draw_segment(T *obuf, T *ibuf, int ry, int x0) {
	if (ry<0 || ry>=image->dim->get(0)) return; // outside of image
	int sx = in[0]->dim->get(1), rsx = image->dim->get(1);
	int sc = in[0]->dim->get(2), rsc = image->dim->get(2);
	T *rbuf = (T *)*image + ry*rsx*rsc;
	if (x0>sx || x0<=-rsx) return; // outside of buffer
	int n=rsx;
	if (x0+n>sx) n=sx-x0;
	if (x0<0) { rbuf-=rsc*x0; n+=x0; x0=0; }
	if (alpha && rsc==4 && sc==3) { // RGB by RGBA //!@#$ optimise
		int j=sc*x0;
		for (; n; n--, rbuf+=4, j+=3) {
			op->zip(sc,obuf+j,rbuf); COMPUTE_ALPHA(0,3); COMPUTE_ALPHA(1,3); COMPUTE_ALPHA(2,3);
		}
	} else if (alpha && rsc==4 && sc==4) { // RGBA by RGBA
		op->zip(n*rsc,obuf+x0*rsc,rbuf);
		int j=sc*x0;
		for (; n>=4; n-=4, rbuf+=16, j+=16) {
			COMPUTE_ALPHA4(0);COMPUTE_ALPHA4(4);
			COMPUTE_ALPHA4(8);COMPUTE_ALPHA4(12);
		}
		for (; n; n--, rbuf+=4, j+=4) {
			COMPUTE_ALPHA4(0);
		}
	} else { // RGB by RGB, etc
		op->zip(n*rsc,obuf+sc*x0,rbuf);
	}
}

GRID_INLET(0) {
	NOTEMPTY(image);
	NOTEMPTY(position);
	SAME_TYPE(in,image);
	if (in->dim->n!=3) RAISE("expecting 3 dimensions");
	if (image->dim->n!=3) RAISE("expecting 3 dimensions in right_hand");
	int lchan = in->dim->get(2);
	int rchan = image->dim->get(2);
	if (alpha && rchan!=4) {
		RAISE("alpha mode works only with 4 channels in right_hand");
	}
	if (lchan != rchan-(alpha?1:0) && lchan != rchan) {
		RAISE("right_hand has %d channels, alpha=%d, left_hand has %d, expecting %d or %d",
			rchan, alpha?1:0, lchan, rchan-(alpha?1:0), rchan);
	}
	out=new GridOutlet(this,0,in->dim,in->nt);
	in->set_chunk(1);
} GRID_FLOW {
	int f = in->dim->prod(1);
	int y = dex/f;
	if (position->nt != int32_e) RAISE("position has to be int32");
	int py = ((int32*)*position)[0], rsy = image->dim->v[0];
	int px = ((int32*)*position)[1], rsx = image->dim->v[1], sx=in->dim->get(1);
	for (; n; y++, n-=f, data+=f) {
		int ty = div2(y-py,rsy);
		if (tile || ty==0) {
			T data2[f];
			COPY(data2,data,f);
			if (tile) {
				for (int x=px-div2(px+rsx-1,rsx)*rsx; x<sx; x+=rsx) {
					draw_segment(data2,data,mod(y-py,rsy),x);
				}
			} else {
				draw_segment(data2,data,y-py,px);
			}
			out->send(f,data2);
		} else {
			out->send(f,data);
		}
	}
} GRID_END

GRID_INPUT(1,image) {} GRID_END
GRID_INPUT(2,position) {} GRID_END

\end class {install("#draw_image",3,1); add_creator("@draw_image");}

//****************************************************************
// Dim[*A],Dim[*B],Dim[C,size(A)-size(B)] -> Dim[*A]

/* NOT FINISHED */
\class GridDrawPoints : FObject {
	\attr Numop *op;
	\attr PtrGrid color;
	\attr PtrGrid points;
	\grin 0
	\grin 1 int32
	\grin 2 int32
	\constructor (Numop *op=op_put, Grid *color=0, Grid *points=0) {
		this->op = op;
		if (color) this->color=color;
		if (points) this->points=points;
	}
};

GRID_INPUT(1,color) {} GRID_END
GRID_INPUT(2,points) {} GRID_END

GRID_INLET(0) {
	NOTEMPTY(color);
	NOTEMPTY(points);
	SAME_TYPE(in,color);
	out=new GridOutlet(this,0,in->dim,in->nt);
	if (points->dim->n!=2) RAISE("points should be a 2-D grid");
	if (points->dim->v[1] != in->dim->n - color->dim->n)
		RAISE("wrong number of dimensions");
	in->set_chunk(0);
} GRID_FLOW {
	long m = points->dim->v[1];
	long cn = in->dim->prod(-color->dim->n); /* size of color (RGB=3, greyscale=1, ...) */
	int32 *pdata = (int32 *)points->data;
	T *cdata = (T *)color->data;
	for (long i=0; i<n; i++) {
		long off = 0;
		for (long k=0; k>m; k++) off = off*in->dim->v[k] + pdata[i*points->dim->v[1]+k];
		off *= cn;
		for (long j=0; j<cn; j++) data[off+j] = cdata[j];
	}
//	out->send(data);
} GRID_END
\end class {install("#draw_points",3,1);}

//****************************************************************
\class GridNoiseGateYuvs : FObject {
	\grin 0
	int thresh;
	\decl 1 float(int v);
	\constructor (int v=0) {thresh=v;}
};

GRID_INLET(0) {
	if (in->dim->n!=3) RAISE("requires 3 dimensions: dim(y,x,3)");
	if (in->dim->v[2]!=3) RAISE("requires 3 channels");
	out=new GridOutlet(this,0,in->dim,in->nt);
	in->set_chunk(2);
} GRID_FLOW {
	T tada[n];
	for (long i=0; i<n; i+=3) {
		if (data[i+0]<=thresh) {
			tada[i+0]=0;         tada[i+1]=0;         tada[i+2]=0;
		} else {
			tada[i+0]=data[i+0]; tada[i+1]=data[i+1]; tada[i+2]=data[i+2];
		}
	}
	out->send(n,tada);
} GRID_END

\def 1 float(int v) {thresh=v;}
\end class {install("#noise_gate_yuvs",2,1);}

//****************************************************************

\class GridPack : FObject {
	int n;
	PtrGrid a;
	\constructor (int n=2, NumberTypeE nt=int32_e) {
		if (n<1) RAISE("n=%d must be at least 1",n);
		if (n>32) RAISE("n=%d is too many?",n);
		a = new Grid(new Dim(n),nt,true);
		this->n=n;
		bself->ninlets_set(this->n);
	}
	\decl void _n_float (int inlet, float f);
	\decl void _n_list  (int inlet, float f);
	\decl 0 bang ();
	//\grin 0
};
\def void _n_float (int inlet, float f) {
#define FOO(T) ((T *)*a)[inlet] = (T)f;
TYPESWITCH(a->nt,FOO,);
#undef FOO
	_0_bang(argc,argv);
}
\def void _n_list (int inlet, float f) {_n_float(argc,argv,inlet,f);}
\def 0 bang () {
	out=new GridOutlet(this,0,a->dim,a->nt);
#define FOO(T) out->send(n,(T *)*a);
TYPESWITCH(a->nt,FOO,);
#undef FOO
}
\end class {install("#pack",1,1); add_creator("@pack");}

\class GridUnpack : FObject {
	int n;
	\constructor (int n=2) {
		if (n<1) RAISE("n=%d must be at least 1",n);
		if (n>32) RAISE("n=%d is too many?",n);
		this->n=n;
		bself->noutlets_set(this->n);
	}
	\grin 0
};
GRID_INLET(0) {
	if (in->dim->n!=1) RAISE("expect one dimension");
	if (in->dim->v[0]!=this->n) RAISE("expecting dim(%ld), got dim(%ld)",this->n,in->dim->v[0]);
	in->set_chunk(0);
} GRID_FLOW {
	for (int i=n-1; i>=0; i--) outlet_float(bself->outlets[i],(t_float)data[i]);
} GRID_END
\end class {install("#unpack",1,0);}

//****************************************************************

\class GridRotatificator : FObject {
	int angle;
	int from, to, n;
	\decl 0 float (int scale);
	\decl 0 axis (int from, int to, int n);
	\constructor (int from=0, int to=1, int n=2) {
		angle=0;
		_0_axis(0,0,from,to,n);
	}
	\decl 1 float(int angle);
};
\def 0 float (int scale) {
	int32 rotator[n*n];
	for (int i=0; i<n; i++) for (int j=0; j<n; j++) rotator[i*n+j] = scale * (i==j);
	float th = angle * M_PI / 18000;
	for (int i=0; i<2; i++) for (int j=0; j<2; j++)
		rotator[(i?to:from)*n+(j?to:from)] = (int32)round(scale*cos(th+(j-i)*M_PI/2));
	GridOutlet out(this,0,new Dim(n,n),int32_e);
	out.send(n*n,rotator);
}
\def 0 axis(int from, int to, int n) {
	if (n<0) RAISE("n-axis number incorrect");
	if (from<0 || from>=n) RAISE("from-axis number incorrect");
	if (to  <0 || to  >=n) RAISE(  "to-axis number incorrect");
	this->from = from;
	this->  to =   to;
	this->   n =    n;
}
\def 1 float(int angle) {this->angle = angle;}
\end class {install("#rotatificator",2,1);}

static void expect_min_one_dim (P<Dim> d) {
	if (d->n<1) RAISE("expecting at least one dimension, got %s",d->to_s());}

#define OP(x) op_dict[string(#x)]
\class GridClusterAvg : FObject {
	\attr int numClusters;
	\attr PtrGrid r;
	\attr PtrGrid sums;
	\attr PtrGrid counts;
	\constructor (int v) {_1_float(0,0,v); r.constrain(expect_min_one_dim);}
	\decl 1 float (int v);
	\grin 0 int32
	\grin 2
	template <class T> void make_stats (long n, int32 *ldata, T *rdata) {
		int32 chans = r->dim->v[r->dim->n-1];
		T     *sdata = (T     *)*sums;
		int32 *cdata = (int32 *)*counts;
		for (int i=0; i<n; i++, ldata++, rdata+=chans) {
			if (*ldata<0 || *ldata>=numClusters) RAISE("value out of range in left grid");
			OP(+)->zip(chans,sdata+(*ldata)*chans,rdata);
			cdata[*ldata]++;
		}
		for (int i=0; i<numClusters; i++) OP(/)->map(chans,sdata+i*chans,(T)cdata[i]);
		out = new GridOutlet(this,1,counts->dim,counts->nt);
		out->send(counts->dim->prod(),(int32 *)*counts);
		out = new GridOutlet(this,0,sums->dim,sums->nt);
		out->send(sums->dim->prod(),(T *)*sums);
	}
};

GRID_INLET(0) {
	NOTEMPTY(r);
	int32 v[r->dim->n];
	COPY(v,r->dim->v,r->dim->n-1);
	v[r->dim->n-1]=1;
	P<Dim> t = new Dim(r->dim->n,v);
	if (!t->equal(in->dim)) RAISE("left %s must be equal to right %s except last dimension should be 1",in->dim->to_s(),r->dim->to_s());
	in->set_chunk(0);
	int32 w[2] = {numClusters,r->dim->v[r->dim->n-1]};
	sums   = new Grid(new Dim(2,w),r->nt,  true);
	counts = new Grid(new Dim(1,w),int32_e,true);
} GRID_FLOW {
	#define FOO(U) make_stats(n,data,(U *)*r);
	TYPESWITCH(r->nt,FOO,)
	#undef FOO
} GRID_END
\def 1 float (int v) {numClusters = v;}
GRID_INPUT(2,r) {
} GRID_END

\end class {install("#cluster_avg",3,2);}

//****************************************************************

void startup_flow_objects () {
	op_add = OP(+);
	op_sub = OP(-);
	op_mul = OP(*);
	op_shl = OP(<<);
	op_mod = OP(%);
	op_and = OP(&);
	op_div = OP(/);
	op_put = OP(put);
	\startall
}
