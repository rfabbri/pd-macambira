/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __VASP_OPS_CPLX_H
#define __VASP_OPS_CPLX_H

#include "opfuns.h"

// Complex functions

namespace VecOp {
    inline BL d_polar(OpParam &p) { return d__cun<S,f_polar<S> >(p); }
    inline BL d_rect(OpParam &p) { return d__cun<S,f_rect<S> >(p); }

    inline BL d_radd(OpParam &p) { return d__cbin<S,f_radd<S> >(p); }

    inline BL d_cnorm(OpParam &p) { return d__cun<S,f_norm<S> >(p); }

    inline BL d_cconj(OpParam &p) { return D__cun<S,f_conj<S> >(p); }
}

namespace VaspOp {
	inline Vasp *m_polar(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_cun(p,src,dst,VecOp::d_polar); } // cartesian -> polar (each two)
	inline Vasp *m_rect(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_cun(p,src,dst,VecOp::d_rect); } // polar -> cartesian (each two)

	Vasp *m_radd(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL); // radius offset

	inline Vasp *m_cnorm(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_cun(p,src,dst,VecOp::d_cnorm); } // complex normalize

//	inline Vasp *m_cswap(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_cun(p,src,dst,VecOp::d_cswap); }  // swap real and imaginary parts
	inline Vasp *m_cconj(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_cun(p,src,dst,VecOp::d_cconj); }  // complex conjugate
}

#endif
