/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include "ops_carith.h"
#include "ops_assign.h"
#include "opdefs.h"
#include "util.h"
#include <math.h>

template<class T> inline V f_cadd(T &rv,T &iv,T ra,T ia,T rb,T ib) { rv = ra+rb,iv = ia+ib; }
template<class T> inline V f_csub(T &rv,T &iv,T ra,T ia,T rb,T ib) { rv = ra-rb,iv = ia-ib; }
template<class T> inline V f_csubr(T &rv,T &iv,T ra,T ia,T rb,T ib) { rv = rb-ra,iv = ib-ia; }
template<class T> inline V f_cmul(T &rv,T &iv,T ra,T ia,T rb,T ib) { rv = ra*rb-ia*ib, iv = ra*ib+rb*ia; }

template<class T> inline V f_cdiv(T &rv,T &iv,T ra,T ia,T rb,T ib) 
{ 
	register const R den = sqabs(rb,ib);
	rv = (ra*rb+ia*ib)/den;
	iv = (ia*rb-ra*ib)/den;
}

template<class T> inline V f_cdivr(T &rv,T &iv,T ra,T ia,T rb,T ib)
{ 
	register const R den = sqabs(ra,ia);
	rv = (rb*ra+ib*ia)/den;
	iv = (ib*ra-rb*ia)/den;
}

BL VecOp::d_cadd(OpParam &p) { D__cbin(f_cadd<S>,p); }
BL VecOp::d_csub(OpParam &p) { D__cbin(f_csub<S>,p); }
BL VecOp::d_csubr(OpParam &p) { D__cbin(f_csubr<S>,p); }
BL VecOp::d_cmul(OpParam &p) { D__cbin(f_cmul<S>,p); }
BL VecOp::d_cdiv(OpParam &p) { d__cbin(f_cdiv<S>,p); }
BL VecOp::d_cdivr(OpParam &p) { d__cbin(f_cdivr<S>,p); }


VASP_BINARY("vasp.c+",cadd,true,VASP_ARG_R(0),"adds a complex value or vasp")
VASP_BINARY("vasp.c-",csub,true,VASP_ARG_R(0),"subtracts a complex value or vasp")
VASP_BINARY("vasp.c!-",csubr,true,VASP_ARG_R(0),"reverse subtracts a complex value or vasp")
VASP_BINARY("vasp.c*",cmul,true,VASP_ARG_R(1),"multiplies by a complex value or vasp")
VASP_BINARY("vasp.c/",cdiv,true,VASP_ARG_R(1),"divides by a complex value or vasp")
VASP_BINARY("vasp.c!/",cdivr,true,VASP_ARG_R(1),"reverse divides by a complex value or vasp")


// -----------------------------------------------------


template<class T> inline V f_csqr(T &rv,T &iv,T ra,T ia) { rv = ra*ra-ia*ia; iv = ra*ia*2; }

BL VecOp::d_csqr(OpParam &p) { D__cun(f_csqr<S>,p); }

VASP_UNARY("vasp.csqr",csqr,true,"complex square") 

// -----------------------------------------------------

template<class T> V f_cpowi(T &rv,T &iv,T ra,T ia,OpParam &p) 
{ 
	register const I powi = p.ibin.arg;
	register S rt,it; f_csqr(rt,it,ra,ia);
	for(I i = 2; i < powi; ++i) f_cmul(rt,it,rt,it,ra,ia);
	rv = rt,iv = it;
} 

BL VecOp::d_cpowi(OpParam &p) { d__cop(f_cpowi<S>,p); }

Vasp *VaspOp::m_cpowi(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst) 
{ 
	Vasp *ret = NULL;
	CVecBlock *vecs = GetCVecs(p.opname,src,dst);
	if(vecs) {
		I powi = 1;
		if(arg.IsList() && arg.GetList().Count() >= 1 && flext::CanbeInt(arg.GetList()[0]))
			powi = flext::GetAInt(arg.GetList()[0]);
		else 
			post("%s - power arg is invalid -> set to 1",p.opname);

		if(powi < 0) {
			post("%s - negative integer power is not allowed",p.opname);
		}
		else {
			switch(powi) {
			case 0: {
				p.cbin.rarg = 1,p.cbin.iarg = 0;
				ret = DoOp(vecs,VecOp::d_cset,p);
				break;
			}
			case 1: {
				// set arg to src
				ret = DoOp(vecs,VecOp::d_ccopy,p);
				break;
			}
			case 2: {
				ret = DoOp(vecs,VecOp::d_csqr,p);
				break;
			}
			default: {
				p.ibin.arg = powi;
				ret = DoOp(vecs,VecOp::d_cpowi,p);
				break;
			}
			}
		}

		delete vecs;
	}
	return ret;
}

VASP_ANYOP("vasp.cpowi",cpowi,0,true,VASP_ARG_I(1),"complex integer power") 

// -----------------------------------------------------

template<class T> inline V f_cabs(T &rv,T &iv,T ra,T ia) { rv = sqrt(ra*ra+ia*ia),iv = 0; }

BL VecOp::d_cabs(OpParam &p) { D__cun(f_cabs<S>,p); }

VASP_UNARY("vasp.cabs",cabs,true,"set real part to complex absolute value, imaginary part becomes zero") 

