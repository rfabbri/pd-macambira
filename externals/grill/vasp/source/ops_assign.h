/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __VASP_OPS_ASSIGN_H
#define __VASP_OPS_ASSIGN_H

#include "opbase.h"

// Assignment functions

namespace VecOp {
	BL d_copy(OpParam &p); 
	BL d_set(OpParam &p); 

	BL d_ccopy(OpParam &p); 
	BL d_cset(OpParam &p); 
}

namespace VaspOp {
	inline Vasp *m_set(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_rbin(p,src,arg,dst,VecOp::d_set); } // copy to (one vec or real)
	inline Vasp *m_cset(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst = NULL) { return m_cbin(p,src,arg,dst,VecOp::d_cset); }  // complex copy (pairs of vecs or complex)

	Vasp *m_copy(OpParam &p,CVasp &src,CVasp &dst);
	Vasp *m_ccopy(OpParam &p,CVasp &src,CVasp &dst);
}

#endif
