/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __VASP_OPDEFS_H
#define __VASP_OPDEFS_H

#include "oploop.h"

#ifdef VASP_CHN1  
#define _D_ALWAYS1 1
#else
#define _D_ALWAYS1 0
#endif

/*! \brief skeleton for unary real operations
*/
#define _D__run(fun,p)											\
{																\
	register const S *sr = p.rsdt;								\
	register S *dr = p.rddt;									\
	register I i;												\
	if(sr == dr)												\
		if(_D_ALWAYS1 || p.rds == 1)							\
			_D_LOOP(i,p.frames)									\
			{ fun(*dr,*dr); dr++; }								\
			_E_LOOP												\
		else													\
			_D_LOOP(i,p.frames)									\
			{ fun(*dr,*dr); dr += p.rds; }						\
			_E_LOOP												\
	else														\
		if(_D_ALWAYS1 || (p.rss == 1 && p.rds == 1))			\
			_D_LOOP(i,p.frames)									\
			{ fun(*dr,*sr); sr++,dr++; }						\
			_E_LOOP												\
		else													\
			_D_LOOP(i,p.frames)									\
			{ fun(*dr,*sr); sr += p.rss,dr += p.rds; }			\
			_E_LOOP												\
	return true;												\
}

#define d__run(fun,p) { return _d__run(fun,p); }

/*! \brief skeleton for unary complex operations
*/
#define _D__cun(fun,p)											\
{																\
	register const S *sr = p.rsdt,*si = p.isdt;					\
	register S *dr = p.rddt,*di = p.iddt;						\
	register I i;												\
	if(sr == dr && si == di)									\
		if(_D_ALWAYS1 || (p.rds == 1 && p.ids == 1))			\
			_D_LOOP(i,p.frames)									\
			{ fun(*dr,*di,*dr,*di); dr++,di++; }				\
			_E_LOOP												\
		else													\
			_D_LOOP(i,p.frames)									\
			{ fun(*dr,*di,*dr,*di); dr += p.rds,di += p.ids; }	\
			_E_LOOP												\
	else														\
		if(_D_ALWAYS1 || (p.rss == 1 && p.iss == 1 && p.rds == 1 && p.ids == 1)) \
			_D_LOOP(i,p.frames)									\
			{ fun(*dr,*di,*sr,*si); sr++,si++,dr++,di++; }		\
			_E_LOOP												\
		else													\
			_D_LOOP(i,p.frames)									\
			{ fun(*dr,*di,*sr,*si);	sr += p.rss,si += p.iss,dr += p.rds,di += p.ids; } \
			_E_LOOP												\
	return true;												\
}

#define d__cun(fun,p) { return _d__cun(fun,p); }


/*! \brief skeleton for binary real operations
*/
#define _D__rbin(fun,p)											\
{																\
	register const S *sr = p.rsdt;								\
	register S *dr = p.rddt;									\
	register I i;												\
	if(p.HasArg() && p.arg[0].Is()) {											\
		switch(p.arg[0].argtp) {									\
		case OpParam::Arg::arg_v: {								\
			register const S *ar = p.arg[0].v.rdt;					\
			if(p.rsdt == p.rddt)									\
				if(_D_ALWAYS1 || (p.rds == 1 && p.arg[0].v.rs == 1))				\
					_D_LOOP(i,p.frames)									\
					{ fun(*dr,*dr,*ar); dr++,ar++; }				\
					_E_LOOP												\
				else												\
					_D_LOOP(i,p.frames)									\
					{ fun(*dr,*dr,*ar);	dr += p.rds,ar += p.arg[0].v.rs; } \
					_E_LOOP												\
			else													\
				if(_D_ALWAYS1 || (p.rss == 1 && p.rds == 1 && p.arg[0].v.rs == 1))	\
					_D_LOOP(i,p.frames)									\
					{ fun(*dr,*sr,*ar); sr++,dr++,ar++; }			\
					_E_LOOP												\
				else												\
					_D_LOOP(i,p.frames)									\
					{ fun(*dr,*sr,*ar);	sr += p.rss,dr += p.rds,ar += p.arg[0].v.rs; } \
					_E_LOOP												\
			break;												\
		}														\
		case OpParam::Arg::arg_env: {							\
			Env::Iter it(*p.arg[0].e.env); it.Init(0);			\
			if(p.rsdt == p.rddt)									\
				if(_D_ALWAYS1 || p.rds == 1)										\
					_D_LOOP(i,p.frames)									\
					{ fun(*dr,*dr,it.ValFwd(i)); dr++; }			\
					_E_LOOP												\
				else												\
					_D_LOOP(i,p.frames)									\
					{ fun(*dr,*dr,it.ValFwd(i)); dr += p.rds; }		\
					_E_LOOP												\
			else													\
				if(_D_ALWAYS1 || (p.rss == 1 && p.rds == 1))						\
					_D_LOOP(i,p.frames)									\
					{ fun(*dr,*sr,it.ValFwd(i)); sr++,dr++; }		\
					_E_LOOP												\
				else												\
					_D_LOOP(i,p.frames)								 	\
					{ fun(*dr,*sr,it.ValFwd(i)); sr += p.rss,dr += p.rds; }	\
					_E_LOOP												\
			break;												\
		}														\
		case OpParam::Arg::arg_x: {							\
			const R v =  p.arg[0].x.r;							\
			if(p.rsdt == p.rddt)									\
				if(_D_ALWAYS1 || p.rds == 1)										\
					_D_LOOP(i,p.frames)									\
					{ fun(*dr,*dr,v); dr++; }						\
					_E_LOOP												\
				else												\
					_D_LOOP(i,p.frames)								 	\
					{ fun(*dr,*dr,v); dr += p.rds; }				\
					_E_LOOP												\
			else													\
				if(_D_ALWAYS1 || (p.rss == 1 && p.rds == 1))						\
					_D_LOOP(i,p.frames)									\
					{ fun(*dr,*sr,v); sr++,dr++; }					\
					_E_LOOP												\
				else												\
					_D_LOOP(i,p.frames)								 	\
					{ fun(*dr,*sr,v); sr += p.rss,dr += p.rds; }	\
					_E_LOOP												\
			break;												\
		}														\
		}														\
	}															\
	else {														\
		register const S v = p.rbin.arg;						\
		if(p.rsdt == p.rddt)									\
			if(_D_ALWAYS1 || p.rds == 1)										\
				_D_LOOP(i,p.frames)									\
				{ fun(*dr,*dr,v); dr++; }						\
				_E_LOOP												\
			else												\
				_D_LOOP(i,p.frames)									\
				{ fun(*dr,*dr,v); dr += p.rds; }				\
				_E_LOOP												\
		else													\
			if(_D_ALWAYS1 || (p.rss == 1 && p.rds == 1))						\
				_D_LOOP(i,p.frames)									\
				{ fun(*dr,*sr,v); sr++,dr++; }					\
				_E_LOOP												\
			else												\
				_D_LOOP(i,p.frames)									\
				{ fun(*dr,*sr,v); sr += p.rss,dr += p.rds; }	\
				_E_LOOP												\
	}															\
	return true;												\
}

#define d__rbin(fun,p) { return _d__rbin(fun,p); }


/*! \brief skeleton for binary complex operations
*/
#define _D__cbin(fun,p)											\
{																\
	register const S *sr = p.rsdt,*si = p.isdt;					\
	register S *dr = p.rddt,*di = p.iddt;						\
	register I i;												\
	if(p.HasArg() && p.arg[0].Is()) {											\
		switch(p.arg[0].argtp) {									\
		case OpParam::Arg::arg_v: {									\
			register const S *ar = p.arg[0].v.rdt,*ai = p.arg[0].v.idt;				\
			if(ai)													\
				if(sr == dr && si == di)							\
					if(_D_ALWAYS1 || (p.rds == 1 && p.ids == 1 && p.arg[0].v.rs == 1 && p.arg[0].v.is == 1)) \
						_D_LOOP(i,p.frames)				 \
						{ fun(*dr,*di,*dr,*di,*ar,*ai);	dr++,di++,ar++,ai++; }		\
						_E_LOOP												\
					else											\
						_D_LOOP(i,p.frames)				 \
						{ fun(*dr,*di,*dr,*di,*ar,*ai);	dr += p.rds,di += p.ids,ar += p.arg[0].v.rs,ai += p.arg[0].v.is; }		\
						_E_LOOP												\
				else												\
					_D_LOOP(i,p.frames)				 \
					{ fun(*dr,*di,*sr,*si,*ar,*ai);	sr += p.rss,si += p.iss,dr += p.rds,di += p.ids,ar += p.arg[0].v.rs,ai += p.arg[0].v.is; }			\
					_E_LOOP												\
			else													\
				if(sr == dr && si == di)							\
					_D_LOOP(i,p.frames)				 \
					{ fun(*dr,*di,*dr,*di,*ar,0); dr += p.rds,di += p.ids,ar += p.arg[0].v.rs; }	\
					_E_LOOP												\
				else												\
					_D_LOOP(i,p.frames)				 \
					{ fun(*dr,*di,*sr,*si,*ar,0); sr += p.rss,si += p.iss,dr += p.rds,di += p.ids,ar += p.arg[0].v.rs; } \
					_E_LOOP												\
			break;												\
		}														\
		case OpParam::Arg::arg_env: {									\
			Env::Iter it(*p.arg[0].e.env); it.Init(0);				\
			if(sr == dr && si == di)							\
				if(_D_ALWAYS1 || (p.rds == 1 && p.ids == 1)) \
					_D_LOOP(i,p.frames)				 \
					{ fun(*dr,*di,*dr,*di,it.ValFwd(i),0); dr++,di++; }			\
					_E_LOOP												\
				else											\
					_D_LOOP(i,p.frames)				 \
					{ fun(*dr,*di,*dr,*di,it.ValFwd(i),0); dr += p.rds,di += p.ids; }			\
					_E_LOOP												\
			else												\
				_D_LOOP(i,p.frames)				 \
					{ fun(*dr,*di,*sr,*si,it.ValFwd(i),0); sr += p.rss,si += p.iss,dr += p.rds,di += p.ids; }	\
				_E_LOOP												\
			break;												\
		}														\
		case OpParam::Arg::arg_x: {									\
			register const R ar = p.arg[0].x.r,ai = p.arg[0].x.i;				\
			if(sr == dr && si == di)							\
				if(_D_ALWAYS1 || (p.rds == 1 && p.ids == 1)) \
					_D_LOOP(i,p.frames)				 \
					{ fun(*dr,*di,*dr,*di,ar,ai); dr++,di++; }			\
					_E_LOOP												\
				else											\
					_D_LOOP(i,p.frames)				 \
					{ fun(*dr,*di,*dr,*di,ar,ai); dr += p.rds,di += p.ids; }			\
					_E_LOOP												\
			else												\
				_D_LOOP(i,p.frames)				 \
				{ fun(*dr,*di,*sr,*si,ar,ai); sr += p.rss,si += p.iss,dr += p.rds,di += p.ids; }	\
				_E_LOOP												\
			break;												\
		}														\
		}														\
	}															\
	else {														\
		register const S rv = p.cbin.rarg,iv = p.cbin.iarg;		\
		if(sr == dr && si == di)								\
			if(_D_ALWAYS1 || (p.rds == 1 && p.ids == 1))						\
				_D_LOOP(i,p.frames)						\
				{ fun(*dr,*di,*dr,*di,rv,iv); dr++,di++; }					\
				_E_LOOP												\
			else												\
				_D_LOOP(i,p.frames)				 \
				{ fun(*dr,*di,*dr,*di,rv,iv); dr += p.rds,di += p.ids; }					\
				_E_LOOP												\
		else													\
			if(_D_ALWAYS1 || (p.rds == 1 && p.ids == 1 && p.rss == 1 && p.iss == 1)) \
				_D_LOOP(i,p.frames)				 \
				{ fun(*dr,*di,*sr,*si,rv,iv); sr++,si++,dr++,di++; }					\
				_E_LOOP												\
			else												\
				_D_LOOP(i,p.frames)				 \
				{ fun(*dr,*di,*sr,*si,rv,iv); sr += p.rss,si += p.iss,dr += p.rds,di += p.ids; }					\
				_E_LOOP												\
	}															\
	return true;												\
}

#define d__cbin(fun,p) { return _d__cbin(fun,p); }


/*! \brief skeleton for real operations with parameter block
*/
#define _D__rop(fun,p)											\
{																\
	register const S *sr = p.rsdt;								\
	register S *dr = p.rddt;									\
	register I i;												\
	if(sr == dr)												\
		if(_D_ALWAYS1 || p.rds == 1)											\
			_D_LOOP(i,p.frames)										\
			{ fun(*dr,*dr,p); dr++; }							\
			_E_LOOP												\
		else													\
			_D_LOOP(i,p.frames)										\
			{ fun(*dr,*dr,p); dr += p.rds; }					\
			_E_LOOP												\
	else														\
		if(_D_ALWAYS1 || (p.rss == 1 && p.rds == 1))							\
			_D_LOOP(i,p.frames)										\
			{ fun(*dr,*sr,p); sr++,dr++; }						\
			_E_LOOP												\
		else													\
			_D_LOOP(i,p.frames)										\
			{ fun(*dr,*sr,p); sr += p.rss,dr += p.rds; }		\
			_E_LOOP												\
	return true;												\
}

#define d__rop(fun,p) { return _d__rop(fun,p); }


/*! \brief skeleton for complex operations with parameter block
*/
#define _D__cop(fun,p)											\
{																\
	register const S *sr = p.rsdt,*si = p.isdt;					\
	register S *dr = p.rddt,*di = p.iddt;						\
	register I i;												\
	if(sr == dr && si == di)									\
		if(_D_ALWAYS1 || (p.rds == 1 && p.ids == 1))							\
			_D_LOOP(i,p.frames)										\
			{ fun(*dr,*di,*dr,*di,p); dr++,di++; }				\
			_E_LOOP												\
		else													\
			_D_LOOP(i,p.frames)										\
			{ fun(*dr,*di,*dr,*di,p); dr += p.rds,di += p.ids; } \
			_E_LOOP												\
	else														\
		if(_D_ALWAYS1 || (p.rss == 1 && p.iss == 1 && p.rds == 1 && p.ids == 1)) \
			_D_LOOP(i,p.frames)				 \
			{ fun(*dr,*di,*sr,*si,p); sr++,si++,dr++,di++; }	\
			_E_LOOP												\
		else													\
			_D_LOOP(i,p.frames)				 \
			{ fun(*dr,*di,*sr,*si,p); sr += p.rss,si += p.iss,dr += p.rds,di += p.ids; }	\
			_E_LOOP												\
	return true;												\
}

#define d__cop(fun,p) { return _d__cop(fun,p); }


#ifdef VASP_COMPACT
#define D__run(fun,p) d__run(fun,p)
#define D__cun(fun,p) d__cun(fun,p)
#define D__rbin(fun,p) d__rbin(fun,p)
#define D__cbin(fun,p) d__cbin(fun,p)
#define D__rop(fun,p) d__rop(fun,p)
#define D__cop(fun,p) d__cop(fun,p)
#else
#define D__run(fun,p) _D__run(fun,p)
#define D__cun(fun,p) _D__cun(fun,p)
#define D__rbin(fun,p) _D__rbin(fun,p)
#define D__cbin(fun,p) _D__cbin(fun,p)
#define D__rop(fun,p) _D__rop(fun,p)
#define D__cop(fun,p) _D__cop(fun,p)
#endif

#endif
