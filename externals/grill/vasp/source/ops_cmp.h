/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __VASP_OPS_CMP_H
#define __VASP_OPS_CMP_H

#include "opfuns.h"

// Comparison functions

namespace VecOp {
    inline BL d_lwr(OpParam &p) { return D__rbin<S,f_lwr<S> >(p); }
    inline BL d_gtr(OpParam &p) { return D__rbin<S,f_gtr<S> >(p); }
    inline BL d_alwr(OpParam &p) { return D__rbin<S,f_alwr<S> >(p); }
    inline BL d_agtr(OpParam &p) { return D__rbin<S,f_agtr<S> >(p); }
    inline BL d_leq(OpParam &p) { return D__rbin<S,f_leq<S> >(p); }
    inline BL d_geq(OpParam &p) { return D__rbin<S,f_geq<S> >(p); }
    inline BL d_aleq(OpParam &p) { return D__rbin<S,f_aleq<S> >(p); }
    inline BL d_ageq(OpParam &p) { return D__rbin<S,f_ageq<S> >(p); }
    inline BL d_equ(OpParam &p) { return D__rbin<S,f_equ<S> >(p); }
    inline BL d_neq(OpParam &p) { return D__rbin<S,f_neq<S> >(p); }

    inline BL d_min(OpParam &p) { return D__rbin<S,f_min<S> >(p); }
    inline BL d_max(OpParam &p) { return D__rbin<S,f_max<S> >(p); }
    inline BL d_rmin(OpParam &p) { return d__cbin<S,f_min<S> >(p); }
    inline BL d_rmax(OpParam &p) { return d__cbin<S,f_max<S> >(p); }

    inline BL d_minmax(OpParam &p) { return d__cun<S,f_minmax<S> >(p); }

    inline BL d_minq(OpParam &p) { return D__rop<S,f_minq<S> >(p); }
    inline BL d_maxq(OpParam &p) { return D__rop<S,f_maxq<S> >(p); }
    inline BL d_rminq(OpParam &p) { return d__cop<S,f_minq<S> >(p); }
    inline BL d_rmaxq(OpParam &p) { return d__cop<S,f_maxq<S> >(p); }
    inline BL d_aminq(OpParam &p) { return d__rop<S,f_aminq<S> >(p); }
    inline BL d_amaxq(OpParam &p) { return d__rop<S,f_amaxq<S> >(p); }

    inline BL d_gate(OpParam &p) { return D__rbin<S,f_gate<S> >(p); }
    inline BL d_igate(OpParam &p) { return d__rbin<S,f_igate<S> >(p); }
    inline BL d_rgate(OpParam &p) { return d__cbin<S,f_gate<S> >(p); }
    inline BL d_rigate(OpParam &p) { return d__cbin<S,f_igate<S> >(p); }
}

namespace VaspOp {
	inline Vasp *m_lwr(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_lwr); } // lower than
	inline Vasp *m_gtr(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_gtr); } // greater than
	inline Vasp *m_alwr(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_alwr); } // abs lower than
	inline Vasp *m_agtr(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_agtr); } // abs greater than
	inline Vasp *m_leq(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_leq); } // abs lower than
	inline Vasp *m_geq(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_geq); } // abs greater than
	inline Vasp *m_aleq(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_aleq); } // lower than
	inline Vasp *m_ageq(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_ageq); } // greater than
	inline Vasp *m_equ(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_equ); } // lower than
	inline Vasp *m_neq(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_neq); } // greater than

	inline Vasp *m_min(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_min); } // min (one vec or real)
	inline Vasp *m_max(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_max); } // max (one vec or real)

	inline Vasp *m_rmin(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_cbin(p,src,arg,dst,VecOp::d_rmin); }  // complex (radius) min (pairs of vecs or complex)
	inline Vasp *m_rmax(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_cbin(p,src,arg,dst,VecOp::d_rmax); }  // complex (radius) max (pairs of vecs or complex)

	inline Vasp *m_minmax(OpParam &p,CVasp &src,CVasp *dst = NULL) { return m_cun(p,src,dst,VecOp::d_minmax); } // min/max 

	inline Vasp *m_qmin(OpParam &p,CVasp &src) { return m_run(p,src,NULL,VecOp::d_minq); } // get minimum sample value
	inline Vasp *m_qmax(OpParam &p,CVasp &src) { return m_run(p,src,NULL,VecOp::d_maxq); } // get maximum sample value
	inline Vasp *m_qamin(OpParam &p,CVasp &src) { return m_run(p,src,NULL,VecOp::d_aminq); } // get minimum sample value
	inline Vasp *m_qamax(OpParam &p,CVasp &src) { return m_run(p,src,NULL,VecOp::d_amaxq); } // get maximum sample value

	inline Vasp *m_qrmin(OpParam &p,CVasp &src) { return m_cun(p,src,NULL,VecOp::d_rminq); } // get minimum sample value
	inline Vasp *m_qrmax(OpParam &p,CVasp &src) { return m_cun(p,src,NULL,VecOp::d_rmaxq); } // get maximum sample value

	Vasp *m_gate(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL); // gate
	Vasp *m_igate(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL); // inverse gate
	Vasp *m_rgate(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL); // radius gate
	Vasp *m_rigate(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL); // inverse radius gate
}

#endif
