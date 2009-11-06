/*
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

#include "gridflow.hxx.fcs"
#include <fftw3.h>

#define C(x) ((fftwf_complex *)x)

\class GridFFT : FObject {
	fftwf_plan plan;
	P<Dim> lastdim; /* of last input (for plan cache) */
	long lastchans; /* of last input (for plan cache) */
	\attr int sign; /* -1 or +1 */
	\attr int skip; /* 0 (y and x) or 1 (x only) */
	\attr bool real;
	bool lastreal;
	\constructor () {sign=-1; plan=0; lastdim=0; lastchans=0; skip=0; real=false;}
	\grin 0 float32
};
\def 0 sign (int sign) {
	if (sign!=-1 && sign!=1) RAISE("sign should be -1 or +1");
	this->sign=sign;
	fftwf_destroy_plan(plan);
}
\def 0 skip (int skip) {
	if (skip<0 || skip>1) RAISE("skip should be 0 or 1");
	this->skip=skip;
	if (plan) {fftwf_destroy_plan(plan); plan=0;}
}
GRID_INLET(0) {
	if (in->nt != float32_e)                  RAISE("expecting float32");
	if (real && sign==-1) {
	  if (in->dim->n != 2 && in->dim->n != 3) RAISE("expecting 2 or 3 dimensions: rows,columns,channels?");
	} else {
	  if (in->dim->n != 3 && in->dim->n != 4) RAISE("expecting 3 or 4 dimensions: rows,columns,channels?,complex");
	  if (in->dim->get(in->dim->n-1)!=2)      RAISE("expecting Dim(...,2): real,imaginary (got %d)",in->dim->get(2));
	}
	in->set_chunk(0);
} GRID_FLOW {
	if (skip==1 && !real) RAISE("can't do 1-D FFT in real mode, sorry");
	Dim *dim;
	if (!real) dim = in->dim;
	else if (sign==-1) {
		int v[Dim::MAX_DIM];
		for (int i=0; i<in->dim->n; i++) v[i]=in->dim->v[i];
		v[in->dim->n] = 2;
		dim = new Dim(in->dim->n+1,v);
	} else dim = new Dim(in->dim->n-1,in->dim->v);
	GridOutlet out(this,0,dim,in->nt);
	float32 *tada = (float32 *)memalign(16,dim->prod()*sizeof(float32));
	long chans = in->dim->n>=3 ? in->dim->get(2) : 1;
	CHECK_ALIGN16(data,in->nt)
	CHECK_ALIGN16(tada,in->nt)
	if (plan && lastdim && lastdim!=in->dim && chans!=lastchans && real==lastreal) {fftwf_destroy_plan(plan); plan=0;}
	int v[] = {in->dim->v[0],in->dim->v[1],in->dim->n>2?in->dim->v[2]:1};
//	if (chans==1) {
//		if (skip==0) plan = fftwf_plan_dft_2d(v[0],v[1],data,tada,sign,0);
//		if (skip==1) plan = fftwf_plan_many_dft(1,&v[1],v[0],data,0,1,v[1],tada,0,1,v[1],sign,0);
//	}
	if (skip==0) {
		//plan = fftwf_plan_dft_2d(v[0],v[1],data,tada,sign,0);
		if (!plan) {
			int embed[] = {dim->v[0],dim->v[1]};
			if (!real)         {plan=fftwf_plan_many_dft(    2,&v[0],chans,C(data),0    ,chans,1,C(tada),0    ,chans,1,sign,0);}
			else if (sign==-1) {plan=fftwf_plan_many_dft_r2c(2,&v[0],chans,  data ,embed,chans,1,C(tada),embed,chans,1,0);}
			else               {plan=fftwf_plan_many_dft_c2r(2,&v[0],chans,C(data),embed,chans,1,  tada ,embed,chans,1,0);}
		}
		if (!real)         fftwf_execute_dft(    plan,C(data),C(tada));
		else if (sign==-1) fftwf_execute_dft_r2c(plan,  data ,C(tada));
		else               fftwf_execute_dft_c2r(plan,C(data),  tada );
	}
	if (skip==1) {
		if (!plan) plan=fftwf_plan_many_dft(1,&v[1],chans,C(data),0,chans,1,C(tada),0,chans,1,sign,0);
		//plan = fftwf_plan_many_dft(1,&v[1],v[0],C(data),0,1,v[1],C(tada),0,1,v[1],sign,0);
		long incr = v[1]*chans;
		for (int i=0; i<v[0]; i++) fftwf_execute_dft(plan,C(data)+i*incr,C(tada)+i*incr);
	}
	if (real && sign==-1) {
		for (int i=0; i<v[0]; i++) {
			int h = mod(-i,v[0]);
			T *tada2 = tada + (h*v[1]+v[1]/2)*v[2]*2;
			T *tada3 = tada + (i*v[1]+v[1]/2)*v[2]*2;
			for (int j=1+v[1]/2; j<v[1]; j++) {
				tada2-=v[2]*2; tada3+=v[2]*2;
				for (int k=0; k<v[2]; k++) {tada3[k+k]=tada2[k+k]; tada3[k+k+1]=-tada2[k+k+1];}
			}
		}
	}
	out.send(out.dim->prod(),tada);
	free(tada);
	lastdim=in->dim; lastchans=chans; lastreal=real;
} GRID_END
\end class {install("#fft",1,1);}
void startup_fftw () {
	\startall
}
