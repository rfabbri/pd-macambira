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

    class C_base {
    public:
    #ifdef FLEXT_THREADS
        static flext::ThrMutex mtx;
        static V Lock() { mtx.Lock(); }
        static V Unlock() { mtx.Unlock(); }
    #else
        static V Lock() {}
        static V Unlock() {}
    #endif
    };

    template<class T> class C_run: public C_base {
    public: 
        static BL Do(V f(T &v,T a),OpParam &p) { Lock(); fun = f; _D__run<T,C_run<T> >(p); Unlock(); return true; }
        static V run(T &v,T a) { fun(v,a); } 
        static V (*fun)(T &v,T a);
    };
    template<class T> V (*C_run<T>::fun)(T &v,T a);

    template<class T> class C_cun: public C_base {
    public: 
        static BL Do(V f(T &rv,T &iv,T ra,T ia),OpParam &p) { Lock(); fun = f; _D__cun<T,C_cun<T> >(p); Unlock(); return true; }
        static V cun(T &rv,T &iv,T ra,T ia) { fun(rv,iv,ra,ia); } 
        static V (*fun)(T &rv,T &iv,T ra,T ia);
    };
    template<class T> V (*C_cun<T>::fun)(T &rv,T &iv,T ra,T ia);

    template<class T> class C_rbin: public C_base {
    public: 
        static BL Do(V f(T &v,T a,T b),OpParam &p) { Lock(); fun = f; _D__rbin<T,C_rbin<T> >(p); Unlock(); return true; }
        static V rbin(T &v,T a,T b) { fun(v,a,b); } 
        static V (*fun)(T &v,T a,T b);
    };
    template<class T> V (*C_rbin<T>::fun)(T &v,T a,T b);

    template<class T> class C_cbin: public C_base {
    public: 
        static BL Do(V f(T &rv,T &iv,T ra,T ia,T rb,T ib),OpParam &p) { Lock(); fun = f; _D__cbin<T,C_cbin<T> >(p); Unlock(); return true; }
        static V cbin(T &rv,T &iv,T ra,T ia,T rb,T ib) { fun(rv,iv,ra,ia,rb,ib); } 
        static V (*fun)(T &rv,T &iv,T ra,T ia,T rb,T ib);
    };
    template<class T> V (*C_cbin<T>::fun)(T &rv,T &iv,T ra,T ia,T rb,T ib);

    template<class T> class C_rop: public C_base {
    public: 
        static BL Do(V f(T &v,T a,OpParam &p),OpParam &p) { Lock(); fun = f; _D__rop<T,C_rop<T> >(p); Unlock(); return true; }
        static V rop(T &v,T a,OpParam &p) { fun(v,a,p); } 
        static V (*fun)(T &v,T a,OpParam &p);
    };
    template<class T> V (*C_rop<T>::fun)(T &v,T a,OpParam &p);

    template<class T> class C_cop: public C_base {
    public: 
        static BL Do(V f(T &rv,T &iv,T ra,T ia,OpParam &p),OpParam &p) { Lock(); fun = f; _D__cop<T,C_cop<T> >(p); Unlock(); return true; }
        static V cop(T &rv,T &iv,T ra,T ia,OpParam &p) { fun(rv,iv,ra,ia,p); } 
        static V (*fun)(T &rv,T &iv,T ra,T ia,OpParam &p);
    };
    template<class T> V (*C_cop<T>::fun)(T &rv,T &iv,T ra,T ia,OpParam &p);


    template<class T> BL _d__run(V fun(T &v,T a),OpParam &p)	{ return C_run<T>::Do(fun,p); }
    template<class T> BL _d__cun(V fun(T &rv,T &iv,T ra,T ia),OpParam &p) { return C_cun<T>::Do(fun,p); }
    template<class T> BL _d__rbin(V fun(T &v,T a,T b),OpParam &p) { return C_rbin<T>::Do(fun,p); }
    template<class T> BL _d__cbin(V fun(T &rv,T &iv,T ra,T ia,T rb,T ib),OpParam &p) { return C_cbin<T>::Do(fun,p); }
    template<class T> BL _d__rop(V fun(T &v,T a,OpParam &p),OpParam &p) { return C_rop<T>::Do(fun,p); }
    template<class T> BL _d__cop(V fun(T &rv,T &iv,T ra,T ia,OpParam &p),OpParam &p) { return C_cop<T>::Do(fun,p); }
}


namespace VaspOp {
	RVecBlock *GetRVecs(const C *op,CVasp &src,CVasp *dst = NULL);
	CVecBlock *GetCVecs(const C *op,CVasp &src,CVasp *dst = NULL,BL full = false);
	RVecBlock *GetRVecs(const C *op,CVasp &src,const CVasp &arg,CVasp *dst = NULL,I multi = -1,BL ssize = true);
	CVecBlock *GetCVecs(const C *op,CVasp &src,const CVasp &arg,CVasp *dst = NULL,I multi = -1,BL ssize = true,BL full = false);
	
	Vasp *DoOp(RVecBlock *vecs,VecOp::opfun *fun,OpParam &p,BL symm = false);
	Vasp *DoOp(CVecBlock *vecs,VecOp::opfun *fun,OpParam &p,BL symm = false);

	// -------- transformations -----------------------------------

	// unary functions
	Vasp *m_run(OpParam &p,CVasp &src,CVasp *dst,VecOp::opfun fun); // real unary (one vec or real)
	Vasp *m_cun(OpParam &p,CVasp &src,CVasp *dst,VecOp::opfun fun); // complex unary (one vec or complex)
	// binary functions
	Vasp *m_rbin(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst,VecOp::opfun fun); // real binary (one vec or real)
	Vasp *m_cbin(OpParam &p,CVasp &src,const Argument &arg,CVasp *dst,VecOp::opfun fun); // complex binary (one vec or complex)
}

#endif
