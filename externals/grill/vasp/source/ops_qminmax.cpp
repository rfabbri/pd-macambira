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

template<class T> V f_maxq(T &,T ra,OpParam &p) 
{ 
	if(ra > p.norm.minmax) p.norm.minmax = ra; 
} 

template<class T> V f_minq(T &,T ra,OpParam &p) 
{ 
	if(ra < p.norm.minmax) p.norm.minmax = ra; 
} 

template<class T> V f_amaxq(T &,T ra,OpParam &p) 
{ 
	register T s = fabs(ra); 
	if(s > p.norm.minmax) p.norm.minmax = s; 
} 

template<class T> V f_aminq(T &,T ra,OpParam &p) 
{ 
	register T s = fabs(ra); 
	if(s < p.norm.minmax) p.norm.minmax = s; 
} 

template<class T> V f_rmaxq(T &,T &,T ra,T ia,OpParam &p) 
{ 
	register T s = sqabs(ra,ia); 
	if(s > p.norm.minmax) p.norm.minmax = s; 
} 

template<class T> V f_rminq(T &,T &,T ra,T ia,OpParam &p) 
{ 
	register T s = sqabs(ra,ia); 
	if(s < p.norm.minmax) p.norm.minmax = s; 
} 

BL VecOp::d_minq(OpParam &p) { D__rop(f_minq<S>,p); }
BL VecOp::d_maxq(OpParam &p) { D__rop(f_maxq<S>,p); }
BL VecOp::d_aminq(OpParam &p) { D__rop(f_aminq<S>,p); }
BL VecOp::d_amaxq(OpParam &p) { D__rop(f_amaxq<S>,p); }
BL VecOp::d_rminq(OpParam &p) { d__cop(f_rminq<S>,p); }
BL VecOp::d_rmaxq(OpParam &p) { d__cop(f_rmaxq<S>,p); }

// --------------------------------------------------------------


/*! \class vasp_qmin
	\remark \b vasp.min?
	\brief Get minimum sample value
	\since 0.0.2
	\param inlet vasp - is stored and output triggered
	\param inlet bang - triggers output
	\param inlet set - vasp to be stored 
	\retval outlet float - minimum sample value

	\todo Should we provide a cmdln default vasp?
	\todo Should we inhibit output for invalid vasps?
	\remark Returns 0 for a vasp with 0 frames
*/
class vasp_qmin:
	public vasp_unop
{
	FLEXT_HEADER(vasp_qmin,vasp_unop)

public:
	vasp_qmin(): vasp_unop(true,XletCode(xlet::tp_float,0)) {}

	virtual Vasp *do_opt(OpParam &p) 
	{ 
		p.norm.minmax = BIG;
		Vasp *ret = VaspOp::m_qmin(p,ref); 
		if(p.norm.minmax == BIG) p.norm.minmax = 0;
		return ret;
	}
		
	virtual Vasp *tx_work() 
	{ 
		OpParam p(thisName(),0);													
		Vasp *ret = do_opt(p);
		ToOutFloat(1,p.norm.minmax);
		return ret;
	}

	virtual V m_help() { post("%s - Get a vasp's minimum sample value",thisName()); }
};

FLEXT_LIB("vasp, vasp.min?",vasp_qmin)


/*! \class vasp_qamin
	\remark \b vasp.amin?
	\brief Get minimum absolute sample value
	\since 0.0.2
	\param inlet vasp - is stored and output triggered
	\param inlet bang - triggers output
	\param inlet set - vasp to be stored 
	\retval outlet float - minimum sample value

	\todo Should we provide a cmdln default vasp?
	\todo Should we inhibit output for invalid vasps?
	\remark Returns 0 for a vasp with 0 frames
*/
class vasp_qamin:
	public vasp_qmin
{
	FLEXT_HEADER(vasp_qamin,vasp_qmin)
public:
	virtual Vasp *do_opt(OpParam &p) 
	{ 
		p.norm.minmax = BIG;
		Vasp *ret = VaspOp::m_qamin(p,ref); 
		if(p.norm.minmax == BIG) p.norm.minmax = 0;
		return ret;
	}

	virtual V m_help() { post("%s - Get a vasp's minimum absolute sample value",thisName()); }
};

FLEXT_LIB("vasp, vasp.amin?",vasp_qamin)



/*! \class vasp_qmax
	\remark \b vasp.max?
	\brief Get maximum sample value
	\since 0.0.2
	\param inlet vasp - is stored and output triggered
	\param inlet bang - triggers output
	\param inlet set - vasp to be stored 
	\retval outlet float - minimum sample value

	\todo Should we provide a cmdln default vasp?
	\todo Should we inhibit output for invalid vasps?
	\remark Returns 0 for a vasp with 0 frames
*/
class vasp_qmax:
	public vasp_qmin
{
	FLEXT_HEADER(vasp_qmax,vasp_qmin)
public:
	virtual Vasp *do_opt(OpParam &p) 
	{ 
		p.norm.minmax = -BIG;
		Vasp *ret = VaspOp::m_qmax(p,ref); 
		if(p.norm.minmax == -BIG) p.norm.minmax = 0;
		return ret;
	}

	virtual V m_help() { post("%s - Get a vasp's maximum sample value",thisName()); }
};

FLEXT_LIB("vasp, vasp.max?",vasp_qmax)



/*! \class vasp_qamax
	\remark \b vasp.amax?
	\brief Get minimum absolute sample value
	\since 0.0.2
	\param inlet vasp - is stored and output triggered
	\param inlet bang - triggers output
	\param inlet set - vasp to be stored 
	\retval outlet float - minimum sample value

	\todo Should we provide a cmdln default vasp?
	\todo Should we inhibit output for invalid vasps?
	\remark Returns 0 for a vasp with 0 frames
*/
class vasp_qamax:
	public vasp_qmax
{
	FLEXT_HEADER(vasp_qamax,vasp_qmax)
public:
	virtual Vasp *do_opt(OpParam &p) 
	{ 
		p.norm.minmax = 0;
		return VaspOp::m_qamax(p,ref); 
	}

	virtual V m_help() { post("%s - Get a vasp's maximum absolute sample value",thisName()); }
};

FLEXT_LIB("vasp, vasp.amax?",vasp_qamax)




/*! \class vasp_qrmin
	\remark \b vasp.rmin?
	\brief Get minimum complex radius of samples
	\since 0.0.2
	\param inlet vasp - is stored and output triggered
	\param inlet bang - triggers output
	\param inlet set - vasp to be stored 
	\retval outlet float - minimum sample value

	\todo Should we provide a cmdln default vasp?
	\todo Should we inhibit output for invalid vasps?
	\remark Returns 0 for a vasp with 0 frames
*/
class vasp_qrmin:
	public vasp_unop
{
	FLEXT_HEADER(vasp_qrmin,vasp_unop)

public:
	vasp_qrmin(): vasp_unop(true,XletCode(xlet::tp_float,0)) {}

	virtual Vasp *do_opt(OpParam &p) 
	{ 
		p.norm.minmax = BIG;
		Vasp *ret = VaspOp::m_qrmin(p,ref); 
		if(p.norm.minmax == BIG) p.norm.minmax = 0;
		return ret;
	}
		
	virtual Vasp *tx_work() 
	{ 
		OpParam p(thisName(),0);													
		Vasp *ret = do_opt(p);
		ToOutFloat(1,sqrt(p.norm.minmax));
		return ret;
	}

	virtual V m_help() { post("%s - Get a vasp's minimum complex radius",thisName()); }
};

FLEXT_LIB("vasp, vasp.rmin?",vasp_qrmin)



/*! \class vasp_qrmax
	\remark \b vasp.rmax?
	\brief Get maximum complex radius of samples
	\since 0.0.2
	\param inlet vasp - is stored and output triggered
	\param inlet bang - triggers output
	\param inlet set - vasp to be stored 
	\retval outlet float - minimum sample value

	\todo Should we provide a cmdln default vasp?
	\todo Should we inhibit output for invalid vasps?
	\remark Returns 0 for a vasp with 0 frames
*/
class vasp_qrmax:
	public vasp_qrmin
{
	FLEXT_HEADER(vasp_qrmax,vasp_qrmin)
public:
	virtual Vasp *do_opt(OpParam &p) 
	{ 
		p.norm.minmax = 0;
		return VaspOp::m_qrmax(p,ref); 
	}

	virtual V m_help() { post("%s - Get a vasp's maximum complex radius",thisName()); }
};

FLEXT_LIB("vasp, vasp.rmax?",vasp_qrmax)


