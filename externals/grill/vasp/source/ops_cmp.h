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

DEFOP(S,d_lwr,lwr,rbin)
DEFOP(S,d_gtr,gtr,rbin)
DEFOP(S,d_alwr,alwr,rbin)
DEFOP(S,d_agtr,agtr,rbin)
DEFOP(S,d_leq,leq,rbin)
DEFOP(S,d_geq,geq,rbin)
DEFOP(S,d_aleq,aleq,rbin)
DEFOP(S,d_ageq,ageq,rbin)
DEFOP(S,d_equ,equ,rbin)
DEFOP(S,d_neq,neq,rbin)

DEFOP(S,d_min,min,rbin)
DEFOP(S,d_max,max,rbin)
DEFOP(S,d_rmin,min,cbin)
DEFOP(S,d_rmax,max,cbin)

DEFOP(S,d_minmax,minmax,cun)

DEFOP(S,d_minq,minq,rop)
DEFOP(S,d_maxq,maxq,rop)
DEFOP(S,d_rminq,minq,cop)
DEFOP(S,d_rmaxq,maxq,cop)
DEFOP(S,d_aminq,aminq,rop)
DEFOP(S,d_amaxq,amaxq,rop)

DEFOP(S,d_gate,gate,rbin)
DEFOP(S,d_igate,igate,rbin)
DEFOP(S,d_rgate,gate,cbin)
DEFOP(S,d_rigate,igate,cbin)


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
