/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "opbase.h"
#include "opdefs.h"

Vasp *VaspOp::m_run(OpParam &p,Vasp &src,Vasp *dst,VecOp::opfun *fun) 
{ 
	Vasp *ret = NULL;
	RVecBlock *vecs = GetRVecs(p.opname,src,dst);
	if(vecs) {
		ret = DoOp(vecs,fun,p);
		delete vecs;
	}

	return ret;
}

Vasp *VaspOp::m_cun(OpParam &p,Vasp &src,Vasp *dst,VecOp::opfun *fun) 
{ 
	Vasp *ret = NULL;
	CVecBlock *vecs = GetCVecs(p.opname,src,dst);
	if(vecs) {
		ret = DoOp(vecs,fun,p);
		delete vecs;
	}

	return ret;
}

Vasp *VaspOp::m_rbin(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst,VecOp::opfun *fun) 
{ 
	Vasp *ret = NULL;
	BL argvasp = arg.IsVasp();

	RVecBlock *vecs = argvasp?GetRVecs(p.opname,src,arg.GetVasp(),dst):GetRVecs(p.opname,src,dst);
	if(vecs) {
		if(arg.CanbeDouble()) p.rbin.arg = arg.GetADouble(); 
		else if(arg.IsEnv()) {
			if(p.args != 1)
				ERRINTERNAL();
			else 
				p.arg[0].SetE(&arg.GetEnv());
		}
	
		ret = DoOp(vecs,fun,p);
		delete vecs;
	}

	return ret;
}

Vasp *VaspOp::m_cbin(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst,VecOp::opfun *fun) 
{ 
	Vasp *ret = NULL;
	BL argvasp = arg.IsVasp();

	CVecBlock *vecs = argvasp?GetCVecs(p.opname,src,arg.GetVasp(),dst):GetCVecs(p.opname,src,dst);
	if(vecs) {
		if(arg.CanbeComplex()) {
			CX z = arg.GetAComplex();
			p.cbin.rarg = z.real; 
			p.cbin.iarg = z.imag; 
		}
		else if(arg.IsEnv()) {
			if(p.args != 1)
				ERRINTERNAL();
			else 
				p.arg[0].SetE(&arg.GetEnv());
		}

		ret = DoOp(vecs,fun,p);
		delete vecs;
	}

	return ret;
}


BL VecOp::_d__run(V fun(S &v,S a),OpParam &p)	{ _D__run(fun,p); }
BL VecOp::_d__cun(V fun(S &rv,S &iv,S ra,S ia),OpParam &p) { _D__cun(fun,p); }
BL VecOp::_d__rbin(V fun(S &v,S a,S b),OpParam &p) { _D__rbin(fun,p); }
BL VecOp::_d__cbin(V fun(S &rv,S &iv,S ra,S ia,S rb,S ib),OpParam &p) { _D__cbin(fun,p); }
BL VecOp::_d__rop(V fun(S &v,S a,OpParam &p),OpParam &p) { _D__rop(fun,p); }
BL VecOp::_d__cop(V fun(S &rv,S &iv,S ra,S ia,OpParam &p),OpParam &p) { _D__cop(fun,p); }


