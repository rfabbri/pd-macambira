/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __VASP_OPS_SEARCH_H
#define __VASP_OPS_SEARCH_H

#include "opbase.h"

// Sample search functions

namespace VecOp {
	BL d_search(OpParam &p); //! find values
}

namespace VaspOp {
	// search functions
	Vasp *m_search(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst,BL st); //! find values
	inline Vasp *m_soffset(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst = NULL) { return m_search(p,src,arg,dst,true); }
	inline Vasp *m_sframes(OpParam &p,Vasp &src,const Argument &arg,Vasp *dst = NULL) { return m_search(p,src,arg,dst,false); }
}

#endif
