/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __VASP_OPS_ARITH_H
#define __VASP_OPS_ARITH_H

#include "opbase.h"

// Arithmetic math functions

namespace VecOp {
	BL d_add(OpParam &p); 
	BL d_sub(OpParam &p); 
	BL d_subr(OpParam &p); 
	BL d_mul(OpParam &p); 
	BL d_div(OpParam &p); 
	BL d_divr(OpParam &p); 
	BL d_mod(OpParam &p); 

	BL d_cadd(OpParam &p); 
	BL d_csub(OpParam &p); 
	BL d_csubr(OpParam &p); 
	BL d_cmul(OpParam &p); 
	BL d_cdiv(OpParam &p); 
	BL d_cdivr(OpParam &p);
	
	BL d_sqr(OpParam &p); 
	BL d_ssqr(OpParam &p); 
	BL d_csqr(OpParam &p); 
	BL d_cpowi(OpParam &p); 

	BL d_sign(OpParam &p); 
	BL d_abs(OpParam &p); 
	BL d_cabs(OpParam &p); 
}

namespace VaspOp {
	inline Vasp *m_add(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_add); } // add to (one vec or real)
	inline Vasp *m_sub(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_sub); } // sub from (one vec or real)
	inline Vasp *m_subr(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_subr); } // reverse sub from (one vec or real)
	inline Vasp *m_mul(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_mul); } // mul with (one vec or real)
	inline Vasp *m_div(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_div); } // div by (one vec or real)
	inline Vasp *m_divr(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_divr); } // reverse div by (one vec or real)
	inline Vasp *m_mod(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_mod); } // modulo by (one vec or real)

	inline Vasp *m_cadd(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_cbin(p,src,arg,dst,VecOp::d_cadd); }  // complex add (pairs of vecs or complex)
	inline Vasp *m_csub(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_cbin(p,src,arg,dst,VecOp::d_csub); }  // complex sub (pairs of vecs or complex)
	inline Vasp *m_csubr(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_cbin(p,src,arg,dst,VecOp::d_csubr); }  // reverse complex sub (pairs of vecs or complex)
	inline Vasp *m_cmul(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_cbin(p,src,arg,dst,VecOp::d_cmul); }  // complex mul (pairs of vecs or complex)
	inline Vasp *m_cdiv(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_cbin(p,src,arg,dst,VecOp::d_cdiv); }  // complex div (pairs of vecs or complex)
	inline Vasp *m_cdivr(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_cbin(p,src,arg,dst,VecOp::d_cdivr); }  // complex reverse div (pairs of vecs or complex)

	inline Vasp *m_sqr(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_run(p,src,dst,VecOp::d_sqr); }    // unsigned square 
	inline Vasp *m_ssqr(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_run(p,src,dst,VecOp::d_ssqr); }   // signed square 
	inline Vasp *m_csqr(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_cun(p,src,dst,VecOp::d_csqr); }  // complex square (with each two channels)
//	inline Vasp *m_csqrt(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_cun(p,src,dst,VecOp::d_csqrt); }  // complex square root (how about branches?)

	Vasp *m_cpowi(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL); // complex integer power (with each two channels)

	inline Vasp *m_sign(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_run(p,src,dst,VecOp::d_sign); }  // sign function 
	inline Vasp *m_abs(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_run(p,src,dst,VecOp::d_abs); }  // absolute values
	inline Vasp *m_cabs(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_cun(p,src,dst,VecOp::d_cabs); }  // absolute values
}

#endif
