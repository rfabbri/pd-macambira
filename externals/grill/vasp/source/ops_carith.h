/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __VASP_OPS_CARITH_H
#define __VASP_OPS_CARITH_H

#include "opbase.h"

// Arithmetic math functions

namespace VecOp {
	BL d_cadd(OpParam &p); 
	BL d_csub(OpParam &p); 
	BL d_csubr(OpParam &p); 
	BL d_cmul(OpParam &p); 
	BL d_cdiv(OpParam &p); 
	BL d_cdivr(OpParam &p);
	
	BL d_csqr(OpParam &p); 
	BL d_cpowi(OpParam &p); 

	BL d_cabs(OpParam &p); 
}

namespace VaspOp {
	inline Vasp *m_cadd(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst = NULL) { return m_cbin(p,src,arg,dst,VecOp::d_cadd); }  // complex add (pairs of vecs or complex)
	inline Vasp *m_csub(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst = NULL) { return m_cbin(p,src,arg,dst,VecOp::d_csub); }  // complex sub (pairs of vecs or complex)
	inline Vasp *m_csubr(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst = NULL) { return m_cbin(p,src,arg,dst,VecOp::d_csubr); }  // reverse complex sub (pairs of vecs or complex)
	inline Vasp *m_cmul(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst = NULL) { return m_cbin(p,src,arg,dst,VecOp::d_cmul); }  // complex mul (pairs of vecs or complex)
	inline Vasp *m_cdiv(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst = NULL) { return m_cbin(p,src,arg,dst,VecOp::d_cdiv); }  // complex div (pairs of vecs or complex)
	inline Vasp *m_cdivr(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst = NULL) { return m_cbin(p,src,arg,dst,VecOp::d_cdivr); }  // complex reverse div (pairs of vecs or complex)

	inline Vasp *m_csqr(OpParam &p,Vasp &src,Vasp *dst = NULL) { return m_cun(p,src,dst,VecOp::d_csqr); }  // complex square (with each two channels)
//	inline Vasp *m_csqrt(OpParam &p,Vasp &src,Vasp *dst = NULL) { return m_cun(p,src,dst,VecOp::d_csqrt); }  // complex square root (how about branches?)

	Vasp *m_cpowi(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst = NULL); // complex integer power (with each two channels)

	inline Vasp *m_cabs(OpParam &p,Vasp &src,Vasp *dst = NULL) { return m_cun(p,src,dst,VecOp::d_cabs); }  // absolute values
}

#endif
