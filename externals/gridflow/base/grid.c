/*
	$Id: grid.c 3941 2008-06-25 18:56:09Z matju $

	GridFlow
	Copyright (c) 2001-2008 by Mathieu Bouchard

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
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "../gridflow.h.fcs"
#include <ctype.h>

//#define TRACE fprintf(stderr,"%s %s [%s:%d]\n",ARGS(parent),__PRETTY_FUNCTION__,__FILE__,__LINE__);
#define TRACE

#define CHECK_TYPE(d) \
	if (NumberTypeE_type_of(&d)!=this->nt) RAISE("%s(%s): " \
		"type mismatch during transmission (got %s expecting %s)", \
		ARGS(parent), __PRETTY_FUNCTION__, \
		number_type_table[NumberTypeE_type_of(&d)].name, \
		number_type_table[this->nt].name);

#define CHECK_BUSY1(s) \
	if (!dim) RAISE("%s: " #s " not busy",ARGS(parent));

#define CHECK_BUSY(s) \
	if (!dim) RAISE("%s: " #s " not busy (wanting to write %ld values)",ARGS(parent),(long)n);

#define CHECK_ALIGN(d) \
	{int bytes = number_type_table[nt].size/8; \
	int align = ((long)(void*)d)%bytes; \
	if (align) {_L_;post("%s(%s): Alignment Warning: %p is not %d-aligned: %d", \
		ARGS(parent), __PRETTY_FUNCTION__, (void*)d,bytes,align);}}

#define CHECK_ALIGN2(d,nt) \
	{int bytes = number_type_table[nt].size/8; \
	int align = ((long)(void*)d)%bytes; \
	if (align) {_L_;post("Alignment Warning: %p is not %d-aligned: %d", \
		(void*)d,bytes,align);}}

// **************** Grid ******************************************

void Grid::init_from_list(int n, t_atom *aa, NumberTypeE nt) {
	t_atom2 *a = (t_atom2 *)aa;
	t_symbol *delim = gensym("#");
	for (int i=0; i<n; i++) {
		if (a[i] == delim) {
			int32 v[i];
			if (i!=0 && a[i-1].a_type==A_SYMBOL) nt=NumberTypeE_find(a[--i]);
			for (int j=0; j<i; j++) v[j] = convert(a[j],(int32*)0);
			init(new Dim(i,v),nt);
			CHECK_ALIGN2(this->data,nt);
			if (a[i] != delim) i++;
			i++; a+=i; n-=i;
			goto fill;
		}
	}
	if (n!=0 && a[0].a_type==A_SYMBOL) {
		nt = NumberTypeE_find(a[0]);
		a++, n--;
	}
	init(new Dim(n),nt);
	CHECK_ALIGN2(this->data,nt);
	fill:
	int nn = dim->prod();
	n = min(n,nn);
#define FOO(T) { \
	T *p = (T *)*this; \
	if (n==0) CLEAR(p,nn); \
	else { \
		for (int i=0; i<n; i++) p[i] = a[i]; \
		for (int i=n; i<nn; i+=n) COPY(p+i,p,min(n,nn-i)); }}
	TYPESWITCH(nt,FOO,)
#undef FOO
}

void Grid::init_from_atom(const t_atom &x) {
	if (x.a_type==A_LIST) {
		t_binbuf *b = (t_binbuf *)x.a_gpointer;
		init_from_list(binbuf_getnatom(b),binbuf_getvec(b));
	} else if (x.a_type==A_FLOAT) {
		init(new Dim(),int32_e);
		CHECK_ALIGN2(this->data,nt);
		((int32 *)*this)[0] = (int32)x.a_float;
	} else RAISE("can't convert to grid");
}

// **************** GridInlet *************************************

// must be set before the end of GRID_BEGIN phase, and so cannot be changed
// afterwards. This is to allow some optimisations. Anyway there is no good reason
// why this would be changed afterwards.
void GridInlet::set_factor(long factor) {
	if(!dim) RAISE("huh?");
	if(factor<=0) RAISE("%s: factor=%d should be >= 1",ARGS(parent),factor);
	int i;
	for (i=0; i<=dim->n; i++) if (dim->prod(i)==factor) break;
	if (i>dim->n) RAISE("%s: set_factor: expecting dim->prod(i) for some i, "
		"but factor=%ld and dim=%s",ARGS(parent),factor,dim->to_s());
	if (factor > 1) {
		buf=new Grid(new Dim(factor), nt);
		bufi=0;
	} else {
		buf=0;
	}
}

void GridInlet::set_chunk(long whichdim) {
	long n = dim->prod(whichdim);
	if (n) set_factor(n);
}

bool GridInlet::supports_type(NumberTypeE nt) {
#define FOO(T) return !! gh->flow_##T;
	TYPESWITCH(nt,FOO,return false)
#undef FOO
}

void GridInlet::begin(int argc, t_atom2 *argv) {TRACE;
	GridOutlet *back_out = (GridOutlet *) (void *)argv[0];
	nt = back_out->nt;
	if (dim) RAISE("%s: grid inlet conflict; aborting %s in favour of %s, index %ld of %ld",
			ARGS(parent), ARGS(sender), ARGS(back_out->parent), (long)dex, (long)dim->prod());
	sender = back_out->parent;
	if ((int)nt<0 || (int)nt>=(int)number_type_table_end) RAISE("%s: inlet: unknown number type",ARGS(parent));
	if (!supports_type(nt)) RAISE("%s: number type %s not supported here", ARGS(parent), number_type_table[nt].name);
	P<Dim> dim = this->dim = back_out->dim;
	dex=0;
	buf=0;
	try {
#define FOO(T) gh->flow(this,-1,(T *)0); break;
		TYPESWITCH(this->nt,FOO,)
#undef FOO
	} catch (Barf &barf) {
		this->dim = 0; // hack
		throw;
	}
	this->dim = dim;
	back_out->callback(this);
}

#define CATCH_IT catch (Barf &slimy) {post("error during flow: %s",slimy.text);}

template <class T> void GridInlet::flow(int mode, long n, T *data) {TRACE;
	CHECK_BUSY(inlet);
	CHECK_TYPE(*data);
	CHECK_ALIGN(data);
	if (this->mode==0) {dex += n; return;} // ignore data
	if (n==0) return; // no data
	switch(mode) {
	case 4:{
		long d = dex + bufi;
		if (d+n > dim->prod()) {
			post("grid input overflow: %d of %d from [%s] to [%s]", d+n, dim->prod(), ARGS(sender), 0);
			n = dim->prod() - d;
			if (n<=0) return;
		}
		int bufn = factor();
		if (buf && bufi) {
			T *bufd = *buf;
			long k = min((long)n,bufn-bufi);
			COPY(bufd+bufi,data,k);
			bufi+=k; data+=k; n-=k;
			if (bufi==bufn) {
				long newdex = dex+bufn;
				if (this->mode==6) {
					T *data2 = NEWBUF(T,bufn);
					COPY(data2,bufd,bufn);
					CHECK_ALIGN(data2);
					try {gh->flow(this,bufn,data2);} CATCH_IT;
				} else {
					CHECK_ALIGN(bufd);
					try {gh->flow(this,bufn,bufd);} CATCH_IT;
				}
				dex = newdex;
				bufi = 0;
			}
		}
		int m = (n/bufn)*bufn;
		if (m) {
			int newdex = dex + m;
			if (this->mode==6) {
				T *data2 = NEWBUF(T,m);
				COPY(data2,data,m);
				CHECK_ALIGN(data2);
				try {gh->flow(this,m,data2);} CATCH_IT;
			} else {
				try {gh->flow(this,m,data);} CATCH_IT;
			}
			dex = newdex;
		}
		data += m;
		n -= m;
		if (buf && n>0) COPY((T *)*buf+bufi,data,n), bufi+=n;
	}break;
	case 6:{
		int newdex = dex + n;
		try {gh->flow(this,n,data);} CATCH_IT;
		if (this->mode==4) DELBUF(data);
		dex = newdex;
	}break;
	case 0: break; // ignore data
	default: RAISE("%s: unknown inlet mode",ARGS(parent));
	}
}

void GridInlet::finish() {TRACE;
	if (!dim) RAISE("%s: inlet not busy",ARGS(parent));
	if (dim->prod() != dex) {
		post("incomplete grid: %d of %d from [%s] to [%s]",
			dex, dim->prod(), ARGS(sender), ARGS(parent));
	}
#define FOO(T) try {gh->flow(this,-2,(T *)0);} CATCH_IT;
	TYPESWITCH(nt,FOO,)
#undef FOO
	dim=0;
	buf=0;
	dex=0;
}

template <class T> void GridInlet::from_grid2(Grid *g, T foo) {TRACE;
	nt = g->nt;
	dim = g->dim;
	int n = g->dim->prod();
	gh->flow(this,-1,(T *)0);
	if (n>0 && this->mode!=0) {
		T *data = (T *)*g;
		CHECK_ALIGN(data);
		int size = g->dim->prod();
		if (this->mode==6) {
			T *d = data;
			data = NEWBUF(T,size);
			COPY(data,d,size);
			CHECK_ALIGN(data);
			try {gh->flow(this,n,data);} CATCH_IT;
		} else {
			//int ntsz = number_type_table[nt].size;
			int m = GridOutlet::MAX_PACKET_SIZE/*/ntsz*//factor();
			if (!m) m++;
			m *= factor();
			while (n) {
				if (m>n) m=n;
				CHECK_ALIGN(data);
				try {gh->flow(this,m,data);} CATCH_IT;
				data+=m; n-=m; dex+=m;
			}
		}
	}
	try {gh->flow(this,-2,(T *)0);} CATCH_IT;
	//!@#$ add error handling.
	dim = 0;
	dex = 0;
}

void GridInlet::from_grid(Grid *g) {TRACE;
	if (!supports_type(g->nt))
		RAISE("%s: number type %s not supported here", ARGS(parent), number_type_table[g->nt].name);
#define FOO(T) from_grid2(g,(T)0);
	TYPESWITCH(g->nt,FOO,)
#undef FOO
}

/* **************** GridOutlet ************************************ */

GridOutlet::GridOutlet(FObject *parent_, int woutlet, P<Dim> dim_, NumberTypeE nt_) {
	parent=parent_; dim=dim_; nt=nt_; dex=0; frozen=false; bufi=0; buf=0;
	begin(woutlet,dim,nt);
}

//void GridOutlet::alloc_buf() {
//}

void GridOutlet::begin(int woutlet, P<Dim> dim, NumberTypeE nt) {TRACE;
	this->nt = nt;
	this->dim = dim;
	t_atom a[3];
	SETPOINTER(a,(t_gpointer *)this); // hack
	outlet_anything(parent->bself->outlets[woutlet],bsym._grid,1,a);
	frozen=true;
	if (!dim->prod()) {finish(); return;}
	int32 lcm_factor = 1;
	for (uint32 i=0; i<inlets.size(); i++) lcm_factor = lcm(lcm_factor,inlets[i]->factor());
	//size_t ntsz = number_type_table[nt].size;
	// biggest packet size divisible by lcm_factor
	int32 v = (MAX_PACKET_SIZE/lcm_factor)*lcm_factor;
	if (v==0) v=MAX_PACKET_SIZE; // factor too big. don't have a choice.
	buf=new Grid(new Dim(v),nt);
}

// send modifies dex; send_direct doesn't
template <class T>
void GridOutlet::send_direct(long n, T *data) {TRACE;
	CHECK_BUSY(outlet); CHECK_TYPE(*data); CHECK_ALIGN(data);
	for (; n>0; ) {
		long pn = n;//min((long)n,MAX_PACKET_SIZE);
		for (uint32 i=0; i<inlets.size(); i++) try {inlets[i]->flow(4,pn,data);} CATCH_IT;
		data+=pn, n-=pn;
	}
}

void GridOutlet::flush() {TRACE;
	if (!bufi) return;
#define FOO(T) send_direct(bufi,(T *)*buf);
	TYPESWITCH(buf->nt,FOO,)
#undef FOO
	bufi = 0;
}

template <class T, class S>
static void convert_number_type(int n, T *out, S *in) {
	for (int i=0; i<n; i++) out[i]=(T)in[i];
}

//!@#$ buffering in outlet still is 8x faster...?
//!@#$ should use BitPacking for conversion...?
// send modifies dex; send_direct doesn't
template <class T>
void GridOutlet::send(long n, T *data) {TRACE;
	if (!n) return;
	CHECK_BUSY(outlet); CHECK_ALIGN(data);
	if (NumberTypeE_type_of(data)!=nt) {
		int bs = MAX_PACKET_SIZE;
#define FOO(T) { \
	T data2[bs]; \
	for (;n>=bs;n-=bs,data+=bs) {convert_number_type(bs,data2,data); send(bs,data2);} \
	convert_number_type(n,data2,data); \
	send(n,data2); }
		TYPESWITCH(nt,FOO,)
#undef FOO
	} else {
		dex += n;
		if (n > MIN_PACKET_SIZE || bufi + n > MAX_PACKET_SIZE) flush();
		if (n > MIN_PACKET_SIZE) {
			send_direct(n,data);
		} else {
			COPY((T *)*buf+bufi,data,n);
			bufi += n;
		}
		if (dex==dim->prod()) finish();
	}
}

template <class T>
void GridOutlet::give(long n, T *data) {TRACE;
	CHECK_BUSY(outlet);
	CHECK_ALIGN(data);
	if (NumberTypeE_type_of(data)!=nt) {
		send(n,data);
		DELBUF(data);
		return;
	}
	if (inlets.size()==1 && inlets[0]->mode == 6) {
		// this is the copyless buffer passing
		flush();
		try {inlets[0]->flow(6,n,data);} CATCH_IT;
		dex += n;
	} else {
		flush();
		send_direct(n,data);
		dex += n;
		DELBUF(data);
	}
	if (dex==dim->prod()) finish();
}

void GridOutlet::callback(GridInlet *in) {TRACE;
	CHECK_BUSY1(outlet);
	if (!(in->mode==6 || in->mode==4 || in->mode==0)) RAISE("mode error");
	inlets.push_back(in);
}

// never call this. this is a hack to make some things work.
// i'm trying to circumvent either a bug in the compiler or i don't have a clue. :-(
void make_gimmick () {
	GridOutlet foo(0,0,0);
#define FOO(S) foo.give(0,(S *)0);
EACH_NUMBER_TYPE(FOO)
#undef FOO
	//foo.send(0,(float64 *)0); // this doesn't work, when trying to fix the new link problem in --lite mode.
}

