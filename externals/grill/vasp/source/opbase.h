/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __VASP_OPBASE_H
#define __VASP_OPBASE_H

#include "main.h"
#include "classes.h"
#include "vecblk.h"
#include "opparam.h"

namespace VecOp {
	typedef BL opfun(OpParam &p);

	BL _d__run(V fun(S &v,S a),OpParam &p);
	BL _d__cun(V fun(S &rv,S &iv,S ra,S ia),OpParam &p);
	BL _d__rbin(V fun(S &v,S a,S b),OpParam &p);
	BL _d__cbin(V fun(S &rv,S &iv,S ra,S ia,S rb,S ib),OpParam &p);
	BL _d__rop(V fun(S &v,S a,OpParam &p),OpParam &p);
	BL _d__cop(V fun(S &rv,S &iv,S ra,S ia,OpParam &p),OpParam &p);
}


namespace VaspOp {
	RVecBlock *GetRVecs(const C *op,Vasp &src,Vasp *dst = NULL);
	CVecBlock *GetCVecs(const C *op,Vasp &src,Vasp *dst = NULL,BL full = false);
	RVecBlock *GetRVecs(const C *op,Vasp &src,const Vasp &arg,Vasp *dst = NULL,I multi = -1,BL ssize = true);
	CVecBlock *GetCVecs(const C *op,Vasp &src,const Vasp &arg,Vasp *dst = NULL,I multi = -1,BL ssize = true,BL full = false);
	
	Vasp *DoOp(RVecBlock *vecs,VecOp::opfun *fun,OpParam &p,BL symm = false);
	Vasp *DoOp(CVecBlock *vecs,VecOp::opfun *fun,OpParam &p,BL symm = false);

	// -------- transformations -----------------------------------

	// unary functions
	Vasp *m_run(OpParam &p,Vasp &src,Vasp *dst,VecOp::opfun *fun); // real unary (one vec or real)
	Vasp *m_cun(OpParam &p,Vasp &src,Vasp *dst,VecOp::opfun *fun); // complex unary (one vec or complex)
	// binary functions
	Vasp *m_rbin(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst,VecOp::opfun *fun); // real binary (one vec or real)
	Vasp *m_cbin(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst,VecOp::opfun *fun); // complex binary (one vec or complex)
}

#endif
