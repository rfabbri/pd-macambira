/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __VASP_OPS_ARITH_H
#define __VASP_OPS_ARITH_H

#include "opfuns.h"

// Arithmetic math functions

namespace VaspOp {
    inline BL d_add(OpParam &p) { return VecOp::D__rbin<S,VecOp::f_add<S> >(p); }
    inline BL d_sub(OpParam &p) { return VecOp::D__rbin<S,VecOp::f_sub<S> >(p); }
    inline BL d_subr(OpParam &p) { return VecOp::D__rbin<S,VecOp::f_subr<S> >(p); }
    inline BL d_mul(OpParam &p) { return VecOp::D__rbin<S,VecOp::f_mul<S> >(p); }
    inline BL d_div(OpParam &p) { return VecOp::D__rbin<S,VecOp::f_div<S> >(p); }
    inline BL d_divr(OpParam &p) { return VecOp::D__rbin<S,VecOp::f_divr<S> >(p); }
    inline BL d_mod(OpParam &p) { return VecOp::D__rbin<S,VecOp::f_mod<S> >(p); }

    inline BL d_sqr(OpParam &p) { return VecOp::D__run<S,VecOp::f_sqr<S> >(p); }
    inline BL d_ssqr(OpParam &p) { return VecOp::d__run<S,VecOp::f_ssqr<S> >(p); }

    inline BL d_sign(OpParam &p) { return VecOp::D__run<S,VecOp::f_sign<S> >(p); }
    inline BL d_abs(OpParam &p) { return VecOp::D__run<S,VecOp::f_abs<S> >(p); }


    inline Vasp *m_add(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,d_add); } // add to (one vec or real)
	inline Vasp *m_sub(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,d_sub); } // sub from (one vec or real)
	inline Vasp *m_subr(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,d_subr); } // reverse sub from (one vec or real)
	inline Vasp *m_mul(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,d_mul); } // mul with (one vec or real)
	inline Vasp *m_div(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,d_div); } // div by (one vec or real)
	inline Vasp *m_divr(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,d_divr); } // reverse div by (one vec or real)
	inline Vasp *m_mod(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,d_mod); } // modulo by (one vec or real)

	inline Vasp *m_sqr(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_run(p,src,dst,d_sqr); }    // unsigned square 
	inline Vasp *m_ssqr(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_run(p,src,dst,d_ssqr); }   // signed square 

	inline Vasp *m_sign(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_run(p,src,dst,d_sign); }  // sign function 
	inline Vasp *m_abs(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_run(p,src,dst,d_abs); }  // absolute values
}

#endif
