/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "ops_cmp.h"
#include "opdefs.h"
#include "util.h"
#include <math.h>

// --------------------------------------------------------------


template<class T> V f_gate(T &rv,T ra,T rb) { rv = fabs(ra) >= rb?ra:0; } 
template<class T> V f_igate(T &rv,T ra,T rb) { rv = fabs(ra) <= rb?ra:0; } 

template<class T> V f_rgate(T &rv,T &iv,T ra,T ia,T rb,T) 
{ 
	register const T _abs = sqabs(ra,ia);

	if(_abs >= rb*rb) rv = ra,iv = ia;
	else rv = iv = 0;
} 

template<class T> V f_rigate(T &rv,T &iv,T ra,T ia,T rb,T) 
{ 
	register const T _abs = sqabs(ra,ia);

	if(_abs <= rb*rb) rv = ra,iv = ia;
	else rv = iv = 0;
} 

BL VecOp::d_gate(OpParam &p) { D__rbin(f_gate<S>,p); }
BL VecOp::d_igate(OpParam &p) { d__rbin(f_igate<S>,p); }
BL VecOp::d_rgate(OpParam &p) { d__cbin(f_rgate<S>,p); }
BL VecOp::d_rigate(OpParam &p) { d__cbin(f_rigate<S>,p); }



Vasp *VaspOp::m_gate(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst) 
{ 
	Vasp *ret = NULL;
	RVecBlock *vecs = GetRVecs(p.opname,src,dst);
	if(vecs) {
		if(arg.IsList() && arg.GetList().Count() >= 1 && flext::CanbeFloat(arg.GetList()[0]))
			p.rbin.arg = flext::GetAFloat(arg.GetList()[0]);
		else {
			post("%s - argument is invalid -> set to 1",p.opname);
			p.rbin.arg = 1;
		}

		ret = DoOp(vecs,VecOp::d_gate,p);
		delete vecs;
	}
	return ret;
}


Vasp *VaspOp::m_rgate(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst) 
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

		ret = DoOp(vecs,VecOp::d_rgate,p);
		delete vecs;
	}
	return ret;
}

VASP_ANYOP("vasp.gate",gate,1,true,VASP_ARG_R(1),"set destination to 0 if source < argument") 
VASP_ANYOP("vasp.rgate",rgate,1,true,VASP_ARG_R(1),"complex radius gate: set destination to 0 if rad(complex source) < rad(complex argument)") 


// --------------------------------------------------------------

