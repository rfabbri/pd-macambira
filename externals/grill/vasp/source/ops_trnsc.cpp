/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include "ops_trnsc.h"
#include "opdefs.h"
#include "util.h"
#include <math.h>

// --------------------------------------------------------------

template<class T> V f_rpow(T &v,T a,T b) { v = pow(fabs(a),b)*sgn(a); } 

BL VecOp::d_pow(OpParam &p) { d__rbin(f_rpow<S>,p); }

template<class T> V f_crpow(T &rv,T &iv,T ra,T ia,T rb,T) 
{ 
	register const R _abs = sqrt(sqabs(ra,ia));
	if(_abs) {
		register const R _p = pow(_abs,rb)/_abs;
		rv = _p*ra,iv = _p*ia;
	}
	else
		rv = iv = 0;
} 

BL VecOp::d_rpow(OpParam &p) { d__cbin(f_crpow<S>,p); }

Vasp *VaspOp::m_rpow(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst) 
{ 
	Vasp *ret = NULL;
	CVecBlock *vecs = GetCVecs(p.opname,src,dst);
	if(vecs) {
		if(arg.IsList() && arg.GetList().Count() >= 1 && flext::CanbeFloat(arg.GetList()[0]))
			p.cbin.rarg = flext::GetAFloat(arg.GetList()[0]);
		else {
			post("%s - argument is invalid -> set to 1",p.opname);
			p.cbin.rarg = 1;
		}
		p.cbin.iarg = 0; // not used anyway

		ret = DoOp(vecs,VecOp::d_rpow,p);
		delete vecs;
	}
	return ret;
}

VASP_BINARY("vasp.pow",pow,true,VASP_ARG_R(1),"Real power function") 
VASP_ANYOP("vasp.rpow",rpow,0,true,VASP_ARG_R(1),"Power function acting on complex radius") 


// --------------------------------------------------------------

template<class T> V f_rsqrt(T &v,T a) { v = sqrt(fabs(a)); } 
template<class T> V f_rssqrt(T &v,T a) { v = sqrt(fabs(a))*sgn(a); } 

BL VecOp::d_sqrt(OpParam &p) { d__run(f_rsqrt<S>,p); }
BL VecOp::d_ssqrt(OpParam &p) { d__run(f_rssqrt<S>,p); }


VASP_UNARY("vasp.sqrt",sqrt,true,"Square root") 
VASP_UNARY("vasp.ssqrt",ssqrt,true,"Square root preserving the sign") 

// --------------------------------------------------------------


template<class T> V f_rexp(T &v,T a) { v = exp(a); } 
template<class T> V f_rlog(T &v,T a) { v = log(a); }  // \todo detect NANs

BL VecOp::d_exp(OpParam &p) { d__run(f_rexp<S>,p); }
BL VecOp::d_log(OpParam &p) { d__run(f_rlog<S>,p); }


VASP_UNARY("vasp.exp",exp,true,"Exponential function") 
VASP_UNARY("vasp.log",log,true,"Natural logarithm") 


