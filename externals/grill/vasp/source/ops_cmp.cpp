/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "main.h"
#include "ops_cmp.h"
#include "opdefs.h"
#include "util.h"
#include <math.h>

// --------------------------------------------------------------

template<class T> inline V f_rlwr(T &v,T a,T b) { v = a < b?1:0; }
template<class T> inline V f_rgtr(T &v,T a,T b) { v = a > b?1:0; }
template<class T> inline V f_ralwr(T &v,T a,T b) { v = fabs(a) < fabs(b)?1:0; }
template<class T> inline V f_ragtr(T &v,T a,T b) { v = fabs(a) > fabs(b)?1:0; }
template<class T> inline V f_rleq(T &v,T a,T b) { v = a <= b?1:0; }
template<class T> inline V f_rgeq(T &v,T a,T b) { v = a >= b?1:0; }
template<class T> inline V f_raleq(T &v,T a,T b) { v = fabs(a) <= fabs(b)?1:0; }
template<class T> inline V f_rageq(T &v,T a,T b) { v = fabs(a) >= fabs(b)?1:0; }
template<class T> inline V f_requ(T &v,T a,T b) { v = a == b?1:0; }
template<class T> inline V f_rneq(T &v,T a,T b) { v = a != b?1:0; }

BL VecOp::d_lwr(OpParam &p) { D__rbin(f_rlwr<S>,p); }
BL VecOp::d_gtr(OpParam &p) { D__rbin(f_rgtr<S>,p); }
BL VecOp::d_alwr(OpParam &p) { D__rbin(f_ralwr<S>,p); }
BL VecOp::d_agtr(OpParam &p) { D__rbin(f_ragtr<S>,p); }
BL VecOp::d_leq(OpParam &p) { D__rbin(f_rleq<S>,p); }
BL VecOp::d_geq(OpParam &p) { D__rbin(f_rgeq<S>,p); }
BL VecOp::d_aleq(OpParam &p) { D__rbin(f_raleq<S>,p); }
BL VecOp::d_ageq(OpParam &p) { D__rbin(f_rageq<S>,p); }
BL VecOp::d_equ(OpParam &p) { D__rbin(f_requ<S>,p); }
BL VecOp::d_neq(OpParam &p) { D__rbin(f_rneq<S>,p); }

VASP_BINARY("vasp.<",lwr,true,VASP_ARG_R(0),"set destination to 1 if source < argument, 0 otherwise")
VASP_BINARY("vasp.>",gtr,true,VASP_ARG_R(0),"set destination to 1 if source > argument, 0 otherwise")
VASP_BINARY("vasp.a<",alwr,true,VASP_ARG_R(0),"set destination to 1 if abs(source) < abs(argument), 0 otherwise")
VASP_BINARY("vasp.a>",agtr,true,VASP_ARG_R(0),"set destination to 1 if abs(source) > abs(argument), 0 otherwise")
VASP_BINARY("vasp.<=",leq,true,VASP_ARG_R(0),"set destination to 1 if source <= argument, 0 otherwise")
VASP_BINARY("vasp.>=",geq,true,VASP_ARG_R(0),"set destination to 1 if source >= argument, 0 otherwise")
VASP_BINARY("vasp.a<=",aleq,true,VASP_ARG_R(0),"set destination to 1 if abs(source) <= abs(argument), 0 otherwise")
VASP_BINARY("vasp.a>=",ageq,true,VASP_ARG_R(0),"set destination to 1 if abs(source) >= abs(argument), 0 otherwise")
VASP_BINARY("vasp.==",equ,true,VASP_ARG_R(0),"set destination to 1 if source == argument, 0 otherwise")
VASP_BINARY("vasp.!=",neq,true,VASP_ARG_R(0),"set destination to 1 if source != argument, 0 otherwise")


// --------------------------------------------------------------

template<class T> inline V f_min(T &v,T a,T b) { v = a < b?a:b; }
template<class T> inline V f_max(T &v,T a,T b) { v = a > b?a:b; }

template<class T> inline V f_rmin(T &rv,T &iv,T ra,T ia,T rb,T ib) 
{ 
	if(sqabs(ra,ia) < sqabs(rb,ib))	rv = ra,iv = ia; 
	else rv = rb,iv = ib; 
}

template<class T> inline V f_rmax(T &rv,T &iv,T ra,T ia,T rb,T ib) 
{ 
	if(sqabs(ra,ia) > sqabs(rb,ib))	rv = ra,iv = ia; 
	else rv = rb,iv = ib; 
}

BL VecOp::d_min(OpParam &p) { D__rbin(f_min<S>,p); }
BL VecOp::d_max(OpParam &p) { D__rbin(f_max<S>,p); }
BL VecOp::d_rmin(OpParam &p) { d__cbin(f_rmin<S>,p); }
BL VecOp::d_rmax(OpParam &p) { d__cbin(f_rmax<S>,p); }


VASP_BINARY("vasp.min",min,true,VASP_ARG_R(0),"assigns the minimum of the comparison with a value or vasp")
VASP_BINARY("vasp.max",max,true,VASP_ARG_R(0),"assigns the maximum of the comparison with a value or vasp")

VASP_BINARY("vasp.rmin",rmin,true,VASP_ARG_R(0),"assigns the minimum of the radius comparison with a complex value or vasp")
VASP_BINARY("vasp.rmax",rmax,true,VASP_ARG_R(0),"assigns the maximum of the radius comparison with a complex value or vasp")


// --------------------------------------------------------------

template<class T> inline V f_minmax(T &rv,T &iv,T ra,T ia) 
{ 
	if(ra < ia)	rv = ra,iv = ia; 
	else rv = ia,iv = ra; 
} 

BL VecOp::d_minmax(OpParam &p) { d__cun(f_minmax<S>,p); }

VASP_UNARY("vasp, vasp.minmax",minmax,true,"compare two vectors, assign the lower values to the first and the higher to the second one") 







