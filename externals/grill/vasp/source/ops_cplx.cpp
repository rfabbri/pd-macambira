/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include "ops_cplx.h"
#include "opdefs.h"
#include "util.h"
#include <math.h>

// -----------------------------------------------------

template<class T> V f_polar(T &rv,T &iv,T ra,T ia) { rv = sqrt(sqabs(ra,ia)),iv = arg(ra,ia); }
template<class T> V f_rect(T &rv,T &iv,T ra,T ia) { rv = ra*cos(ia),iv = ra*sin(ia); }

BL VecOp::d_polar(OpParam &p) { d__cun(f_polar<S>,p); }
BL VecOp::d_rect(OpParam &p) { d__cun(f_rect<S>,p); }


VASP_UNARY("vasp.polar",polar,true,"convert complex vector pair from rectangular to polar coordinates") 
VASP_UNARY("vasp.rect",rect,true,"convert complex vector pair from polar to rectangular coordinates") 


// -----------------------------------------------------


template<class T> V f_radd(T &rv,T &iv,T ra,T ia,T rb,T) 
{ 
	register const R _abs = sqrt(sqabs(ra,ia))+rb;
	register const R _phi = arg(ra,ia);

	rv = _abs*cos(_phi),iv = _abs*sin(_phi);
} 

BL VecOp::d_radd(OpParam &p) { d__cbin(f_radd<S>,p); }

Vasp *VaspOp::m_radd(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst) 
{ 
	Vasp *ret = NULL;
	CVecBlock *vecs = GetCVecs(p.opname,src,dst);
	if(vecs) {
		if(arg.IsList() && arg.GetList().Count() >= 1 && flext::CanbeFloat(arg.GetList()[0]))
			p.cbin.rarg = flext::GetAFloat(arg.GetList()[0]);
		else {
			post("%s - argument is invalid -> set to 0",p.opname);
			p.cbin.rarg = 0;
		}
		p.cbin.iarg = 0; // not used anyway

		ret = DoOp(vecs,VecOp::d_radd,p);
		delete vecs;
	}
	return ret;
}


VASP_ANYOP("vasp.r+",radd,0,true,VASP_ARG_R(0),"add offset to complex radius (of complex vector pair)") 


// -----------------------------------------------------

template<class T> V f_cnorm(T &rv,T &iv,T ra,T ia) 
{ 
	register T f = sqabs(ra,ia);
	if(f) { f = 1./sqrt(f); rv = ra*f,iv = ia*f; }
	else rv = iv = 0;
}

BL VecOp::d_cnorm(OpParam &p) { d__cun(f_cnorm<S>,p); }

VASP_UNARY("vasp.cnorm",cnorm,true,"normalize complex radius to 1 (but preserve angle)")

// -----------------------------------------------------

template<class T> inline V f_cconj(T &,T &iv,T,T ia) { iv = -ia; }

BL VecOp::d_cconj(OpParam &p) { D__cun(f_cconj<S>,p); }

VASP_UNARY("vasp.cconj",cconj,true,"complex conjugate: multiply imaginary part with -1")  // should be replaced by an abstraction

