/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __VASP_OPFUNS_H
#define __VASP_OPFUNS_H

#include "opdefs.h"


namespace VecOp {

    // multi-layer templates

    template<class T,V FUN(T &v,T a),I N>
    static V vec_un(T *v,const T *a,I n = 0) { 
        const I _n = N?N:n;
        for(I i = 0; i < _n; ++i) FUN(v[i],a[i]); 
    }

    template<class T,V FUN(T &v,T a),I N>
    static V vec_un(T *v,T a,I n = 0) { 
        const I _n = N?N:n;
        for(I i = 0; i < _n; ++i) FUN(v[i],a); 
    }

    template<class T,V FUN(T &v,T a,T b),I N>
    static V vec_bin(T *v,const T *a,const T *b,I n = 0) { 
        const I _n = N?N:n;
        for(I i = 0; i < _n; ++i) FUN(v[i],a[i],b[i]); 
    }

    template<class T,V FUN(T &v,T a,T b),I N>
    static V vec_bin(T *v,const T *a,T b,I n = 0) { 
        const I _n = N?N:n;
        for(I i = 0; i < _n; ++i) FUN(v[i],a[i],b); 
    }


    template<class T,class CL,I N>
    static V cvec_un(T *v,const T *a,I n = 0) { vec_un<T,CL::run,N>(v,a,n); }

    template<class T,class CL,I N>
    static V cvec_bin(T *v,const T *a,const T *b,I n = 0) { vec_bin<T,n,CL::rbin>(v,a,b,n); }



    // assignment

    template<class T> class f_copy { 
    public: 
        static V run(T &v,T a) { v = a; }
        static V cun(T &rv,T &iv,T ra,T ia) { rv = ra,iv = ia; } 
    };

    template<class T> class f_set { 
    public: 
        static V rbin(T &v,T,T b) { v = b; }
        static V cbin(T &rv,T &iv,T,T,T rb,T ib) { rv = rb,iv = ib; } 
    };

    // arithmetic

    template<class T> class f_add {
    public: 
        static V rbin(T &v,T a,T b) { v = a+b; }
        static V cbin(T &rv,T &iv,T ra,T ia,T rb,T ib) { rv = ra+rb,iv = ia+ib; }
    };

    template<class T> class f_sub {
    public: 
        static V rbin(T &v,T a,T b) { v = a-b; }
        static V cbin(T &rv,T &iv,T ra,T ia,T rb,T ib) { rv = ra-rb,iv = ia-ib; }
    };

    template<class T> class f_subr {
    public: 
        static V rbin(T &v,T a,T b) { v = b-a; }
        static V cbin(T &rv,T &iv,T ra,T ia,T rb,T ib) { rv = rb-ra,iv = ib-ia; }
    };

    template<class T> class f_mul {
    public: 
        static V rbin(T &v,T a,T b) { v = a*b; }
        static V cbin(T &rv,T &iv,T ra,T ia,T rb,T ib) { rv = ra*rb-ia*ib, iv = ra*ib+rb*ia; }
    };

    template<class T> class f_div {
    public: 
        static V rbin(T &v,T a,T b) { v = a/b; }

        static V cbin(T &rv,T &iv,T ra,T ia,T rb,T ib) 
        { 
	        register const T den = sqabs(rb,ib);
	        rv = (ra*rb+ia*ib)/den;
	        iv = (ia*rb-ra*ib)/den;
        }
    };

    template<class T> class f_divr {
    public: 
        static V rbin(T &v,T a,T b) { v = b/a; }

        static V cbin(T &rv,T &iv,T ra,T ia,T rb,T ib)
        { 
	        register const T den = sqabs(ra,ia);
	        rv = (rb*ra+ib*ia)/den;
	        iv = (ib*ra-rb*ia)/den;
        }
    };

    template<class T> class f_mod { 
    public: 
        static V rbin(T &v,T a,T b) { v = fmod(a,b); } 
    };

    template<class T> class f_abs {
    public: 
        static V run(T &v,T a) { v = fabs(a); }
        static V cun(T &rv,T &iv,T ra,T ia) { rv = sqrt(ra*ra+ia*ia),iv = 0; }
    };

    template<class T> class f_sign { 
    public: 
        static V run(T &v,T a) { v = (a == 0?0:(a < 0?-1.:1.)); } 
    };

    template<class T> class f_sqr {
    public: 
        static V run(T &v,T a) { v = a*a; } 
        static V cun(T &rv,T &iv,T ra,T ia) { rv = ra*ra-ia*ia; iv = ra*ia*2; }
    };

    template<class T> class f_ssqr { 
    public: 
        static V run(T &v,T a) { v = a*fabs(a); } 
    };


    // transcendent

    template<class T> class f_powi {
    public: 
        static V cop(T &rv,T &iv,T ra,T ia,OpParam &p) 
        { 
	        register const I powi = p.ibin.arg;
            register T rt,it; f_sqr<T>::cun(rt,it,ra,ia);
            for(I i = 2; i < powi; ++i) f_mul<T>::cbin(rt,it,rt,it,ra,ia);
	        rv = rt,iv = it;
        } 
    };

    template<class T> class f_pow {
    public: 
        static V rbin(T &v,T a,T b) { v = pow(fabs(a),b)*sgn(a); } 

        static V cbin(T &rv,T &iv,T ra,T ia,T rb,T) 
        { 
	        register const T _abs = sqrt(sqabs(ra,ia));
	        if(_abs) {
		        register const T _p = pow(_abs,rb)/_abs;
		        rv = _p*ra,iv = _p*ia;
	        }
	        else
		        rv = iv = 0;
        } 
    };

    template<class T> class f_sqrt {
    public: 
        static V run(T &v,T a) { v = sqrt(fabs(a)); } 
    };

    template<class T> class f_ssqrt {
    public:
        static V run(T &v,T a) { v = sqrt(fabs(a))*sgn(a); } 
    };


    template<class T> class f_exp {
    public: 
        static V run(T &v,T a) { v = exp(a); } 
    };

    template<class T> class f_log {
    public: 
        static V run(T &v,T a) { v = log(a); }  // \todo detect NANs
    };

    // comparisons

    template<class T> class f_lwr {
    public: 
        static V rbin(T &v,T a,T b) { v = a < b?1:0; }
    };

    template<class T> class f_gtr {
    public: 
        static V rbin(T &v,T a,T b) { v = a > b?1:0; }
    };

    template<class T> class f_alwr {
    public: 
        static V rbin(T &v,T a,T b) { v = fabs(a) < fabs(b)?1:0; }
    };

    template<class T> class f_agtr {
    public: 
        static V rbin(T &v,T a,T b) { v = fabs(a) > fabs(b)?1:0; }
    };

    template<class T> class f_leq {
    public: 
        static V rbin(T &v,T a,T b) { v = a <= b?1:0; }
    };

    template<class T> class f_geq {
    public: 
        static V rbin(T &v,T a,T b) { v = a >= b?1:0; }
    };

    template<class T> class f_aleq {
    public: 
        static V rbin(T &v,T a,T b) { v = fabs(a) <= fabs(b)?1:0; }
    };

    template<class T> class f_ageq {
    public: 
        static V rbin(T &v,T a,T b) { v = fabs(a) >= fabs(b)?1:0; }
    };

    template<class T> class f_equ {
    public: 
        static V rbin(T &v,T a,T b) { v = a == b?1:0; }
    };

    template<class T> class f_neq {
    public: 
        static V rbin(T &v,T a,T b) { v = a != b?1:0; }
    };

    // min/max

    template<class T> class f_min {
    public: 
        static V rbin(T &v,T a,T b) { v = a < b?a:b; }

        static V cbin(T &rv,T &iv,T ra,T ia,T rb,T ib) 
        { 
	        if(sqabs(ra,ia) < sqabs(rb,ib))	rv = ra,iv = ia; 
	        else rv = rb,iv = ib; 
        }
    };

    template<class T> class f_max {
    public: 
        static V rbin(T &v,T a,T b) { v = a > b?a:b; }

        static V cbin(T &rv,T &iv,T ra,T ia,T rb,T ib) 
        { 
	        if(sqabs(ra,ia) > sqabs(rb,ib))	rv = ra,iv = ia; 
	        else rv = rb,iv = ib; 
        }
    };

    template<class T> class f_minmax {
    public:
        static V cun(T &rv,T &iv,T ra,T ia) 
        { 
	        if(ra < ia)	rv = ra,iv = ia; 
	        else rv = ia,iv = ra; 
        } 
    };

    template<class T> class f_minq {
    public: 
        static V rop(T &,T ra,OpParam &p) 
        { 
	        if(ra < p.norm.minmax) p.norm.minmax = ra; 
        } 

        static V cop(T &,T &,T ra,T ia,OpParam &p) 
        { 
	        register T s = sqabs(ra,ia); 
	        if(s < p.norm.minmax) p.norm.minmax = s; 
        } 
    };

    template<class T> class f_maxq {
    public: 
        static V rop(T &,T ra,OpParam &p) 
        { 
	        if(ra > p.norm.minmax) p.norm.minmax = ra; 
        } 

        static V cop(T &,T &,T ra,T ia,OpParam &p) 
        { 
	        register T s = sqabs(ra,ia); 
	        if(s > p.norm.minmax) p.norm.minmax = s; 
        } 
    };

    template<class T> class f_aminq {
    public: 
        static V rop(T &,T ra,OpParam &p) 
        { 
	        register T s = fabs(ra); 
	        if(s < p.norm.minmax) p.norm.minmax = s; 
        } 
    };

    template<class T> class f_amaxq {
    public: 
        static V rop(T &,T ra,OpParam &p) 
        { 
	        register T s = fabs(ra); 
	        if(s > p.norm.minmax) p.norm.minmax = s; 
        } 
    };


    // gating

    template<class T> class f_gate {
    public:
        static V rbin(T &rv,T ra,T rb) { rv = fabs(ra) >= rb?ra:0; } 

        static V cbin(T &rv,T &iv,T ra,T ia,T rb,T) 
        { 
	        register const T _abs = sqabs(ra,ia);

	        if(_abs >= rb*rb) rv = ra,iv = ia;
	        else rv = iv = 0;
        } 
    };

    template<class T> class f_igate {
    public:
        static V rbin(T &rv,T ra,T rb) { rv = fabs(ra) <= rb?ra:0; } 

        static V cbin(T &rv,T &iv,T ra,T ia,T rb,T) 
        { 
	        register const T _abs = sqabs(ra,ia);

	        if(_abs <= rb*rb) rv = ra,iv = ia;
	        else rv = iv = 0;
        } 
    };
    
    // complex

    template<class T> class f_norm {
    public:
        static V cun(T &rv,T &iv,T ra,T ia) 
        { 
	        register T f = sqabs(ra,ia);
	        if(f) { f = 1./sqrt(f); rv = ra*f,iv = ia*f; }
	        else rv = iv = 0;
        }
    };

    template<class T> class f_conj {
    public:
        static V cun(T &,T &iv,T,T ia) { iv = -ia; }
    };

    template<class T> class f_polar {
    public:
        static V cun(T &rv,T &iv,T ra,T ia) { rv = sqrt(sqabs(ra,ia)),iv = arg(ra,ia); }
    };

    template<class T> class f_rect {
    public:
        static V cun(T &rv,T &iv,T ra,T ia) { rv = ra*cos(ia),iv = ra*sin(ia); }
    };

    template<class T> class f_radd {
    public:
        static V cbin(T &rv,T &iv,T ra,T ia,T rb,T) 
        { 
	        register const T _abs = sqrt(sqabs(ra,ia))+rb;
	        register const T _phi = arg(ra,ia);

	        rv = _abs*cos(_phi),iv = _abs*sin(_phi);
        } 
    };

    // extra

    template<class T> class f_fix {
    public:
        /*! \brief Bashes denormals and NANs to zero

	        \param a argument list 
	        \param v destination vasp (NULL for in-place operation)
	        \return normalized destination vasp
        */
        static V run(T &v,T a) 
        { 
	        if(a != a) // NAN
		        v = 0; 
	        else {
		        // denormal bashing (doesn't propagate to the next stage)

		        static const T anti_denormal = (T)1.e-18;
		        a += anti_denormal;
		        a -= anti_denormal;
		        v = a; 
	        }
        } 
    };
}


template<class T>
class VecFun {
public:
    // strided real data
    static BL r_add(I len,register T *dr,register const T *sr,I rds = 1,I rss = 1) { return VecOp::V__rbin<T,VecOp::f_add<T>::rbin>(sr,rss,dr,rds,len); }
    static BL r_sub(I len,register T *dr,register const T *sr,I rds = 1,I rss = 1) { return VecOp::V__rbin<T,VecOp::f_sub<T>::rbin>(sr,rss,dr,rds,len); }
    static BL r_subr(I len,register T *dr,register const T *sr,I rds = 1,I rss = 1) { return VecOp::V__rbin<T,VecOp::f_subr<T>::rbin>(sr,rss,dr,rds,len); }
    static BL r_mul(I len,register T *dr,register const T *sr,I rds = 1,I rss = 1) { return VecOp::V__rbin<T,VecOp::f_mul<T>::rbin>(sr,rss,dr,rds,len); }
    static BL r_div(I len,register T *dr,register const T *sr,I rds = 1,I rss = 1) { return VecOp::V__rbin<T,VecOp::f_div<T>::rbin>(sr,rss,dr,rds,len); }
    static BL r_divr(I len,register T *dr,register const T *sr,I rds = 1,I rss = 1) { return VecOp::V__rbin<T,VecOp::f_divr<T>::rbin>(sr,rss,dr,rds,len); }
    static BL r_mod(I len,register T *dr,register const T *sr,I rds = 1,I rss = 1) { return VecOp::V__rbin<T,VecOp::f_mod<T>::rbin>(sr,rss,dr,rds,len); }

    // multi-layer data (non-strided)
    static BL v_add(I len,I layers,register T *dr,register const T *sr,register const T *ar) { return VecOp::V__vbin<T,VecOp::f_add<T>::rbin>(layers,sr,dr,ar,len); }
    static BL v_sub(I len,I layers,register T *dr,register const T *sr,register const T *ar) { return VecOp::V__vbin<T,VecOp::f_sub<T>::rbin>(layers,sr,dr,ar,len); }
    static BL v_subr(I len,I layers,register T *dr,register const T *sr,register const T *ar) { return VecOp::V__vbin<T,VecOp::f_subr<T>::rbin>(layers,sr,dr,ar,len); }
    static BL v_mul(I len,I layers,register T *dr,register const T *sr,register const T *ar) { return VecOp::V__vbin<T,VecOp::f_mul<T>::rbin>(layers,sr,dr,ar,len); }
    static BL v_div(I len,I layers,register T *dr,register const T *sr,register const T *ar) { return VecOp::V__vbin<T,VecOp::f_div<T>::rbin>(layers,sr,dr,ar,len); }
    static BL v_divr(I len,I layers,register T *dr,register const T *sr,register const T *ar) { return VecOp::V__vbin<T,VecOp::f_divr<T>::rbin>(layers,sr,dr,ar,len); }
    static BL v_mod(I len,I layers,register T *dr,register const T *sr,register const T *ar) { return VecOp::V__vbin<T,VecOp::f_mod<T>::rbin>(layers,sr,dr,ar,len); }
};


#endif
