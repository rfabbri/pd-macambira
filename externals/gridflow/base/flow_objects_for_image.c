/*
	$Id: flow_objects_for_image.c,v 1.2 2006-03-15 04:37:08 matju Exp $

	GridFlow
	Copyright (c) 2001,2002,2003,2004,2005 by Mathieu Bouchard

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

#include <math.h>
#include "grid.h.fcs"

static void expect_picture (P<Dim> d) {
	if (d->n!=3) RAISE("(height,width,chans) dimensions please");}
static void expect_rgb_picture (P<Dim> d) {
	expect_picture(d);
	if (d->get(2)!=3) RAISE("(red,green,blue) channels please");}
static void expect_rgba_picture (P<Dim> d) {
	expect_picture(d);
	if (d->get(2)!=4) RAISE("(red,green,blue,alpha) channels please");}
static void expect_max_one_dim (P<Dim> d) {
	if (d->n>1) { RAISE("expecting Dim[] or Dim[n], got %s",d->to_s()); }}

//****************************************************************
//{ Dim[A,B,*Cs]<T>,Dim[D,E]<T> -> Dim[A,B,*Cs]<T> }

static void expect_convolution_matrix (P<Dim> d) {
	if (d->n != 2) RAISE("only exactly two dimensions allowed for now (got %d)",
		d->n);
}

// entry in a compiled convolution kernel
struct PlanEntry { int y,x; bool neutral; };

\class GridConvolve < GridObject
struct GridConvolve : GridObject {
	\attr Numop *op_para;
	\attr Numop *op_fold;
	\attr PtrGrid seed;
	\attr PtrGrid b;
	PtrGrid a;
	int plann;
	PlanEntry *plan; //Pt?
	int margx,margy; // margins
	GridConvolve () : plan(0) { b.constrain(expect_convolution_matrix); plan=0; }
	\decl void initialize (Grid *r=0);
	\decl void _0_op   (Numop *op);
	\decl void _0_fold (Numop *op);
	\decl void _0_seed (Grid *seed);
	\grin 0
	\grin 1
	template <class T> void copy_row (Pt<T> buf, int sx, int y, int x);
	template <class T> void make_plan (T bogus);
	~GridConvolve () {if (plan) delete[] plan;}
};

template <class T> void GridConvolve::copy_row (Pt<T> buf, int sx, int y, int x) {
	int day = a->dim->get(0), dax = a->dim->get(1), dac = a->dim->prod(2);
	y=mod(y,day); x=mod(x,dax);
	Pt<T> ap = (Pt<T>)*a + y*dax*dac;
	while (sx) {
		int sx1 = min(sx,dax-x);
		COPY(buf,ap+x*dac,sx1*dac);
		x=0;
		buf += sx1*dac;
		sx -= sx1;
	}
}

static Numop *OP(Ruby x) {return FIX2PTR(Numop,rb_hash_aref(op_dict,x));}

template <class T> void GridConvolve::make_plan (T bogus) {
	P<Dim> da = a->dim, db = b->dim;
	int dby = db->get(0);
	int dbx = db->get(1);
	if (plan) delete[] plan;
	plan = new PlanEntry[dbx*dby];
	int i=0;
	for (int y=0; y<dby; y++) {
		for (int x=0; x<dbx; x++) {
			T rh = ((Pt<T>)*b)[y*dbx+x];
			bool neutral = op_para->on(rh)->is_neutral(rh,at_right);
			bool absorbent = op_para->on(rh)->is_absorbent(rh,at_right);
			STACK_ARRAY(T,foo,1);
			if (absorbent) {
				foo[0] = 0;
				op_para->map(1,foo,rh);
				absorbent = op_fold->on(rh)->is_neutral(foo[0],at_right);
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

GRID_INLET(GridConvolve,0) {
	SAME_TYPE(in,b);
	SAME_TYPE(in,seed);
	P<Dim> da = in->dim, db = b->dim;
	if (!db) RAISE("right inlet has no grid");
	if (!seed) RAISE("seed missing");
	if (db->n != 2) RAISE("right grid must have two dimensions");
	if (da->n < 2) RAISE("left grid has less than two dimensions");
	if (seed->dim->n != 0) RAISE("seed must be scalar");
	if (da->get(0) < db->get(0)) RAISE("grid too small (y): %d < %d", da->get(0), db->get(0));
	if (da->get(1) < db->get(1)) RAISE("grid too small (x): %d < %d", da->get(1), db->get(1));
	margy = (db->get(0)-1)/2;
	margx = (db->get(1)-1)/2;
	a=new Grid(in->dim,in->nt);
	out=new GridOutlet(this,0,da,in->nt);
} GRID_FLOW {
	COPY((Pt<T>)*a+in->dex, data, n);
} GRID_FINISH {
	Numop *op_put = OP(SYM(put));
	make_plan((T)0);
	int dbx = b->dim->get(1);
	int day = a->dim->get(0);
	int n = a->dim->prod(1);
	int sx = a->dim->get(1)+dbx-1;
	int n2 = sx*a->dim->prod(2);
	STACK_ARRAY(T,buf,n);
	STACK_ARRAY(T,buf2,n2);
	T orh=0;
	for (int iy=0; iy<day; iy++) {
		op_put->map(n,buf,*(T *)*seed);
		for (int i=0; i<plann; i++) {
			int jy = plan[i].y;
			int jx = plan[i].x;
			T rh = ((Pt<T>)*b)[jy*dbx+jx];
			if (i==0 || plan[i].y!=plan[i-1].y || orh!=rh) {
				copy_row(buf2,sx,iy+jy-margy,-margx);
				if (!plan[i].neutral) op_para->map(n2,buf2,rh);
			}
			op_fold->zip(n,buf,buf2+jx*a->dim->prod(2));
			orh=rh;
		}
		out->send(n,buf);
	}
	a=0;
} GRID_END

GRID_INPUT(GridConvolve,1,b) {} GRID_END

\def void _0_op   (Numop *op ) { this->op_para=op; }
\def void _0_fold (Numop *op ) { this->op_fold=op; }
\def void _0_seed (Grid *seed) { this->seed=seed; }

\def void initialize (Grid *r) {
	rb_call_super(argc,argv);
	this->op_para = op_mul;
	this->op_fold = op_add;
	this->seed = new Grid(new Dim(),int32_e,true);
	this->b= r ? r : new Grid(new Dim(1,1),int32_e,true);
}

\classinfo { IEVAL(rself,"install '#convolve',2,1"); }
\end class GridConvolve

/* ---------------------------------------------------------------- */
/* "#scale_by" does quick scaling of pictures by integer factors */
/*{ Dim[A,B,3]<T> -> Dim[C,D,3]<T> }*/
\class GridScaleBy < GridObject
struct GridScaleBy : GridObject {
	\attr PtrGrid scale; // integer scale factor
	int scaley;
	int scalex;
	\decl void initialize (Grid *factor=0);
	\grin 0
	\grin 1
	void prepare_scale_factor () {
		scaley = ((Pt<int32>)*scale)[0];
		scalex = ((Pt<int32>)*scale)[scale->dim->prod()==1 ? 0 : 1];
		if (scaley<1) scaley=2;
		if (scalex<1) scalex=2;
	}
};

GRID_INLET(GridScaleBy,0) {
	P<Dim> a = in->dim;
	expect_picture(a);
	out=new GridOutlet(this,0,new Dim(a->get(0)*scaley,a->get(1)*scalex,a->get(2)),in->nt);
	in->set_factor(a->get(1)*a->get(2));
} GRID_FLOW {
	int rowsize = in->dim->prod(1);
	STACK_ARRAY(T,buf,rowsize*scalex);
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

static void expect_scale_factor (P<Dim> dim) {
	if (dim->prod()!=1 && dim->prod()!=2)
		RAISE("expecting only one or two numbers");
}

GRID_INPUT(GridScaleBy,1,scale) { prepare_scale_factor(); } GRID_END

\def void initialize (Grid *factor) {
	scale.constrain(expect_scale_factor);
	rb_call_super(argc,argv);
	scale=new Grid(INT2NUM(2));
	if (factor) scale=factor;
	prepare_scale_factor();
}

\classinfo { IEVAL(rself,"install '#scale_by',2,1"); }
\end class GridScaleBy

// ----------------------------------------------------------------
//{ Dim[A,B,3]<T> -> Dim[C,D,3]<T> }
\class GridDownscaleBy < GridObject
struct GridDownscaleBy : GridObject {
	\attr PtrGrid scale;
	\attr bool smoothly;
	int scaley;
	int scalex;
	PtrGrid temp;
	\decl void initialize (Grid *factor=0, Symbol option=Qnil);
	\grin 0
	\grin 1
	void prepare_scale_factor () {
		scaley = ((Pt<int32>)*scale)[0];
		scalex = ((Pt<int32>)*scale)[scale->dim->prod()==1 ? 0 : 1];
		if (scaley<1) scaley=2;
		if (scalex<1) scalex=2;
	}
};

GRID_INLET(GridDownscaleBy,0) {

	P<Dim> a = in->dim;
	if (a->n!=3) RAISE("(height,width,chans) please");
	out=new GridOutlet(this,0,new Dim(a->get(0)/scaley,a->get(1)/scalex,a->get(2)),in->nt);
	in->set_factor(a->get(1)*a->get(2));
	// i don't remember why two rows instead of just one.
	temp=new Grid(new Dim(2,in->dim->get(1)/scalex,in->dim->get(2)),in->nt);
} GRID_FLOW {
	int rowsize = in->dim->prod(1);
	int rowsize2 = temp->dim->prod(1);
	Pt<T> buf = (Pt<T>)*temp; //!@#$ maybe should be something else than T ?
	int xinc = in->dim->get(2)*scalex;
	int y = in->dex / rowsize;
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

GRID_INPUT(GridDownscaleBy,1,scale) { prepare_scale_factor(); } GRID_END

\def void initialize (Grid *factor, Symbol option) {
	scale.constrain(expect_scale_factor);
	rb_call_super(argc,argv);
	scale=new Grid(INT2NUM(2));
	if (factor) scale=factor;
	prepare_scale_factor();
	smoothly = option==SYM(smoothly);
}

\classinfo { IEVAL(rself,"install '#downscale_by',2,1"); }
\end class GridDownscaleBy

//****************************************************************
\class GridLayer < GridObject
struct GridLayer : GridObject {
	PtrGrid r;
	GridLayer() { r.constrain(expect_rgb_picture); }
	\grin 0 int
	\grin 1 int
};

GRID_INLET(GridLayer,0) {
	NOTEMPTY(r);
	SAME_TYPE(in,r);
	P<Dim> a = in->dim;
	expect_rgba_picture(a);
	if (a->get(1)!=r->dim->get(1)) RAISE("same width please");
	if (a->get(0)!=r->dim->get(0)) RAISE("same height please");
	in->set_factor(a->prod(2));
	out=new GridOutlet(this,0,r->dim);
} GRID_FLOW {
	Pt<T> rr = ((Pt<T>)*r) + in->dex*3/4;
	STACK_ARRAY(T,foo,n*3/4);
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

GRID_INPUT(GridLayer,1,r) {} GRID_END

\classinfo { IEVAL(rself,"install '#layer',2,1"); }
\end class GridLayer

// ****************************************************************
// pad1,pad2 only are there for 32-byte alignment
struct Line { int32 y1,x1,y2,x2,x,m,pad1,pad2; };

static void expect_polygon (P<Dim> d) {
	if (d->n!=2 || d->get(1)!=2) RAISE("expecting Dim[n,2] polygon");
}

\class DrawPolygon < GridObject
struct DrawPolygon : GridObject {
	\attr Numop *op;
	\attr PtrGrid color;
	\attr PtrGrid polygon;
	PtrGrid color2;
	PtrGrid lines;
	int lines_start;
	int lines_stop;
	DrawPolygon() {
		color.constrain(expect_max_one_dim);
		polygon.constrain(expect_polygon);
	}
	\decl void initialize (Numop *op, Grid *color=0, Grid *polygon=0);
	\grin 0
	\grin 1
	\grin 2 int32
	void init_lines();

};

void DrawPolygon::init_lines () {
	int nl = polygon->dim->get(0);
	lines=new Grid(new Dim(nl,8), int32_e);
	Pt<Line> ld = Pt<Line>((Line *)(int32 *)*lines,nl);
	Pt<int32> pd = *polygon;
	for (int i=0,j=0; i<nl; i++) {
		ld[i].y1 = pd[j+0];
		ld[i].x1 = pd[j+1];
		j=(j+2)%(2*nl);
		ld[i].y2 = pd[j+0];
		ld[i].x2 = pd[j+1];
		if (ld[i].y1>ld[i].y2) memswap(Pt<int32>(ld+i)+0,Pt<int32>(ld+i)+2,2);
	}
}

static int order_by_starting_scanline (const void *a, const void *b) {
	return ((Line *)a)->y1 - ((Line *)b)->y1;
}

static int order_by_column (const void *a, const void *b) {
	return ((Line *)a)->x - ((Line *)b)->x;
}

GRID_INLET(DrawPolygon,0) {
	NOTEMPTY(color);
	NOTEMPTY(polygon);
	NOTEMPTY(lines);
	SAME_TYPE(in,color);
	if (in->dim->n!=3) RAISE("expecting 3 dimensions");
	if (in->dim->get(2)!=color->dim->get(0))
		RAISE("image does not have same number of channels as stored color");
	out=new GridOutlet(this,0,in->dim,in->nt);
	lines_start = lines_stop = 0;
	in->set_factor(in->dim->get(1)*in->dim->get(2));
	int nl = polygon->dim->get(0);
	qsort((int32 *)*lines,nl,sizeof(Line),order_by_starting_scanline);
	int cn = color->dim->prod();
	color2=new Grid(new Dim(cn*16), color->nt);
	for (int i=0; i<16; i++) COPY((Pt<T>)*color2+cn*i,(Pt<T>)*color,cn);
} GRID_FLOW {
	int nl = polygon->dim->get(0);
	Pt<Line> ld = Pt<Line>((Line *)(int32 *)*lines,nl);
	int f = in->factor();
	int y = in->dex/f;
	int cn = color->dim->prod();
	Pt<T> cd = (Pt<T>)*color2;
	
	while (n) {
		while (lines_stop != nl && ld[lines_stop].y1<=y) lines_stop++;
		for (int i=lines_start; i<lines_stop; i++) {
			if (ld[i].y2<=y) {
				memswap(ld+i,ld+lines_start,1);
				lines_start++;
			}
		}
		if (lines_start == lines_stop) {
			out->send(f,data);
		} else {
			int32 xl = in->dim->get(1);
			Pt<T> data2 = ARRAY_NEW(T,f);
			COPY(data2,data,f);
			for (int i=lines_start; i<lines_stop; i++) {
				Line &l = ld[i];
				l.x = l.x1 + (y-l.y1)*(l.x2-l.x1+1)/(l.y2-l.y1+1);
			}
			qsort(ld+lines_start,lines_stop-lines_start,
				sizeof(Line),order_by_column);
			for (int i=lines_start; i<lines_stop-1; i+=2) {
				int xs = max(ld[i].x,(int32)0), xe = min(ld[i+1].x,xl);
				if (xs>=xe) continue; /* !@#$ WHAT? */
				while (xe-xs>=16) { op->zip(16*cn,data2+cn*xs,cd); xs+=16; }
				op->zip((xe-xs)*cn,data2+cn*xs,cd);
			}
			out->give(f,data2);
		}
		n-=f;
		data+=f;
		y++;
	}
} GRID_END


GRID_INPUT(DrawPolygon,1,color) {} GRID_END
GRID_INPUT(DrawPolygon,2,polygon) {init_lines();} GRID_END

\def void initialize (Numop *op, Grid *color, Grid *polygon) {
	rb_call_super(argc,argv);
	this->op = op;
	if (color) this->color=color;
	if (polygon) { this->polygon=polygon; init_lines(); }
}

\classinfo { IEVAL(rself,"install '#draw_polygon',3,1"); }
\end class DrawPolygon

//****************************************************************
static void expect_position(P<Dim> d) {
	if (d->n!=1) RAISE("position should have 1 dimension, not %d", d->n);
	if (d->v[0]!=2) RAISE("position dim 0 should have 2 elements, not %d", d->v[0]);
}

\class DrawImage < GridObject
struct DrawImage : GridObject {
	\attr Numop *op;
	\attr PtrGrid image;
	\attr PtrGrid position;
	\attr bool alpha;
	\attr bool tile;
	
	DrawImage() : alpha(false), tile(false) {
		position.constrain(expect_position);
		image.constrain(expect_picture);
	}

	\decl void initialize (Numop *op, Grid *image=0, Grid *position=0);
	\decl void _0_alpha (bool v=true);
	\decl void _0_tile (bool v=true);
	\grin 0
	\grin 1
	\grin 2 int32
	// draw row # ry of right image in row buffer buf, starting at xs
	// overflow on both sides has to be handled automatically by this method
	template <class T> void draw_segment(Pt<T> obuf, Pt<T> ibuf, int ry, int x0);
};

#define COMPUTE_ALPHA(c,a) obuf[j+(c)] = ibuf[j+(c)] + (rbuf[a])*(obuf[j+(c)]-ibuf[j+(c)])/256;
#define COMPUTE_ALPHA4(b) \
	COMPUTE_ALPHA(b+0,b+3); \
	COMPUTE_ALPHA(b+1,b+3); \
	COMPUTE_ALPHA(b+2,b+3); \
	obuf[b+3] = rbuf[b+3] + (255-rbuf[b+3])*(ibuf[j+b+3])/256;

template <class T> void DrawImage::draw_segment(Pt<T> obuf, Pt<T> ibuf, int ry, int x0) {
	if (ry<0 || ry>=image->dim->get(0)) return; // outside of image
	int sx = in[0]->dim->get(1), rsx = image->dim->get(1);
	int sc = in[0]->dim->get(2), rsc = image->dim->get(2);
	Pt<T> rbuf = (Pt<T>)*image + ry*rsx*rsc;
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

GRID_INLET(DrawImage,0) {
	NOTEMPTY(image);
	NOTEMPTY(position);
	SAME_TYPE(in,image);
	if (in->dim->n!=3) RAISE("expecting 3 dimensions");
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
	in->set_factor(in->dim->get(1)*in->dim->get(2));
} GRID_FLOW {
	int f = in->factor();
	int y = in->dex/f;
	if (position->nt != int32_e) RAISE("position has to be int32");
	int py = ((int32*)*position)[0], rsy = image->dim->v[0], sy=in->dim->get(0);
	int px = ((int32*)*position)[1], rsx = image->dim->v[1], sx=in->dim->get(1);
	for (; n; y++, n-=f, data+=f) {
		int ty = div2(y-py,rsy);
		if (tile || ty==0) {
			Pt<T> data2 = ARRAY_NEW(T,f);
			COPY(data2,data,f);
			if (tile) {
				for (int x=px-div2(px+rsx-1,rsx)*rsx; x<sx; x+=rsx) {
					draw_segment(data2,data,mod(y-py,rsy),x);
				}
			} else {
				draw_segment(data2,data,y-py,px);
			}
			out->give(f,data2);
		} else {
			out->send(f,data);
		}
	}
} GRID_END

GRID_INPUT(DrawImage,1,image) {} GRID_END
GRID_INPUT(DrawImage,2,position) {} GRID_END
\def void _0_alpha (bool v=true) { alpha = v; gfpost("ALPHA=%d",v); }
\def void _0_tile (bool v=true) {   tile = v; }

\def void initialize (Numop *op, Grid *image, Grid *position) {
	rb_call_super(argc,argv);
	this->op = op;
	if (image) this->image=image;
	if (position) this->position=position;
	else this->position=new Grid(new Dim(2),int32_e,true);
}

\classinfo { IEVAL(rself,"install '#draw_image',3,1"); }
\end class DrawImage

void startup_flow_objects_for_image () {
	\startall
}
