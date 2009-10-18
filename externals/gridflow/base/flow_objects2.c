/*
	$Id: flow_objects.c 4097 2008-10-03 19:49:03Z matju $

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

#include "../gridflow.h.fcs"
#define OP(x) op_dict[string(#x)]

static void expect_min_one_dim (P<Dim> d) {
	if (d->n<1) RAISE("expecting at least one dimension, got %s",d->to_s());}

// obviously unfinished
\class GridExpr : FObject {
	\constructor (...) {
		std::ostringstream os;
		for (int i=0; i<argc; i++) os << " " << argv[i];
		string s = os.str();
		post("expr = '%s'",s.data());
	}
};
\end class {install("#expr",1,1);}

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

void startup_flow_objects2 () {
	\startall
}
