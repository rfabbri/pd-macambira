/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __VASP_OPDEFS_H
#define __VASP_OPDEFS_H

#include "oploop.h"
#include "opbase.h"

#ifdef VASP_CHN1  
#define _D_ALWAYS1 1
#else
#define _D_ALWAYS1 0
#endif

namespace VecOp {

/*! \brief skeleton for unary real operations
*/
template<class T,V FUN(T &v,T a)> BL _F__run(OpParam &p)										
{																
	register const S *sr = p.rsdt;								
	register S *dr = p.rddt;									
	register I i;												
	if(sr == dr)												
		if(_D_ALWAYS1 || p.rds == 1)							
			_D_LOOP(i,p.frames)									
            { FUN(*dr,*dr); dr++; }								
			_E_LOOP												
		else													
			_D_LOOP(i,p.frames)									
			{ FUN(*dr,*dr); dr += p.rds; }						
			_E_LOOP												
	else														
		if(_D_ALWAYS1 || (p.rss == 1 && p.rds == 1))			
			_D_LOOP(i,p.frames)									
			{ FUN(*dr,*sr); sr++,dr++; }						
			_E_LOOP												
		else													
			_D_LOOP(i,p.frames)									
			{ FUN(*dr,*sr); sr += p.rss,dr += p.rds; }			
			_E_LOOP												
	return true;												
}

template<class T,class CL> inline BL _D__run(OpParam &p) { return _F__run<T,CL::run>(p); }
template<class T,class CL> inline BL d__run(OpParam &p) { return _d__run<T>(CL::run,p); }

/*! \brief skeleton for unary complex operations
*/
template<class T,V FUN(T &rv,T &iv,T ra,T ia)> BL _F__cun(OpParam &p)
{																
	register const S *sr = p.rsdt,*si = p.isdt;					
	register S *dr = p.rddt,*di = p.iddt;						
	register I i;												
	if(sr == dr && si == di)									
		if(_D_ALWAYS1 || (p.rds == 1 && p.ids == 1))			
			_D_LOOP(i,p.frames)									
			{ FUN(*dr,*di,*dr,*di); dr++,di++; }				
			_E_LOOP												
		else													
			_D_LOOP(i,p.frames)									
			{ FUN(*dr,*di,*dr,*di); dr += p.rds,di += p.ids; }	
			_E_LOOP												
	else														
		if(_D_ALWAYS1 || (p.rss == 1 && p.iss == 1 && p.rds == 1 && p.ids == 1)) 
			_D_LOOP(i,p.frames)									
			{ FUN(*dr,*di,*sr,*si); sr++,si++,dr++,di++; }	
			_E_LOOP												
		else													
			_D_LOOP(i,p.frames)									
			{ FUN(*dr,*di,*sr,*si);	sr += p.rss,si += p.iss,dr += p.rds,di += p.ids; } 
			_E_LOOP												
	return true;												
}

template<class T,class CL> inline BL _D__cun(OpParam &p) { return _F__cun<T,CL::cun>(p); }
template<class T,class CL> inline BL d__cun(OpParam &p) { return _d__cun<T>(CL::cun,p); }


/*! \brief skeleton for binary real operations
*/
template<class T,V FUN(T &v,T a,T b)> BL _F__rbin(OpParam &p)					
{																
	register const S *sr = p.rsdt;								
	register S *dr = p.rddt;									
	register I i;												
	if(p.HasArg() && p.arg[0].Is()) {							
		switch(p.arg[0].argtp) {								
		case OpParam::Arg::arg_v: {								
			register const S *ar = p.arg[0].v.rdt;				
			if(p.rsdt == p.rddt)								
				if(_D_ALWAYS1 || (p.rds == 1 && p.arg[0].v.rs == 1))				
					_D_LOOP(i,p.frames)									
                    { FUN(*dr,*dr,*ar); dr++,ar++; }				
					_E_LOOP												
				else												
					_D_LOOP(i,p.frames)									
					{ FUN(*dr,*dr,*ar);	dr += p.rds,ar += p.arg[0].v.rs; } 
					_E_LOOP												
			else													
				if(_D_ALWAYS1 || (p.rss == 1 && p.rds == 1 && p.arg[0].v.rs == 1))	
					_D_LOOP(i,p.frames)									
					{ FUN(*dr,*sr,*ar); sr++,dr++,ar++; }			
					_E_LOOP												
				else												
					_D_LOOP(i,p.frames)									
					{ FUN(*dr,*sr,*ar);	sr += p.rss,dr += p.rds,ar += p.arg[0].v.rs; } 
					_E_LOOP												
			break;												
		}														
		case OpParam::Arg::arg_env: {							
			Env::Iter it(*p.arg[0].e.env); it.Init(0);			
			if(p.rsdt == p.rddt)								
				if(_D_ALWAYS1 || p.rds == 1)										
					_D_LOOP(i,p.frames)									
					{ FUN(*dr,*dr,it.ValFwd(i)); dr++; }			
					_E_LOOP												
				else												
					_D_LOOP(i,p.frames)									
					{ FUN(*dr,*dr,it.ValFwd(i)); dr += p.rds; }		
					_E_LOOP												
			else													
				if(_D_ALWAYS1 || (p.rss == 1 && p.rds == 1))						
					_D_LOOP(i,p.frames)									
					{ FUN(*dr,*sr,it.ValFwd(i)); sr++,dr++; }		
					_E_LOOP												
				else												
					_D_LOOP(i,p.frames)								 	
					{ FUN(*dr,*sr,it.ValFwd(i)); sr += p.rss,dr += p.rds; }	
					_E_LOOP												
			break;												
		}														
		case OpParam::Arg::arg_x: {							
			const R v =  p.arg[0].x.r;						
			if(p.rsdt == p.rddt)							
				if(_D_ALWAYS1 || p.rds == 1)				
					_D_LOOP(i,p.frames)						
					{ FUN(*dr,*dr,v); dr++; }			
					_E_LOOP									
				else										
					_D_LOOP(i,p.frames)						
					{ FUN(*dr,*dr,v); dr += p.rds; }	
					_E_LOOP									
			else											
				if(_D_ALWAYS1 || (p.rss == 1 && p.rds == 1))
					_D_LOOP(i,p.frames)						
					{ FUN(*dr,*sr,v); sr++,dr++; }		
					_E_LOOP									
				else										
					_D_LOOP(i,p.frames)						
                    { FUN(*dr,*sr,v); sr += p.rss,dr += p.rds; }
					_E_LOOP											
			break;												
		}														
		}														
	}															
	else {														
		register const S v = p.rbin.arg;						
		if(p.rsdt == p.rddt)									
			if(_D_ALWAYS1 || p.rds == 1)						
				_D_LOOP(i,p.frames)									
				{ FUN(*dr,*dr,v); dr++; }						
				_E_LOOP												
			else												
				_D_LOOP(i,p.frames)									
				{ FUN(*dr,*dr,v); dr += p.rds; }				
				_E_LOOP												
		else													
			if(_D_ALWAYS1 || (p.rss == 1 && p.rds == 1))		
				_D_LOOP(i,p.frames)									
				{ FUN(*dr,*sr,v); sr++,dr++; }					
				_E_LOOP												
			else												
				_D_LOOP(i,p.frames)									
				{ FUN(*dr,*sr,v); sr += p.rss,dr += p.rds; }	
				_E_LOOP												
	}															
	return true;												
}

template<class T,class CL> inline BL _D__rbin(OpParam &p) { return _F__rbin<T,CL::rbin>(p); }
template<class T,class CL> inline BL d__rbin(OpParam &p) { return _d__rbin<T>(CL::rbin,p); }


/*! \brief skeleton for binary complex operations
*/
template<class T,V FUN(T &rv,T &iv,T ra,T ia,T rb,T ib)> BL _F__cbin(OpParam &p)							
{																
	register const S *sr = p.rsdt,*si = p.isdt;					
	register S *dr = p.rddt,*di = p.iddt;						
	register I i;												
	if(p.HasArg() && p.arg[0].Is()) {							
		switch(p.arg[0].argtp) {									
		case OpParam::Arg::arg_v: {									
			register const S *ar = p.arg[0].v.rdt,*ai = p.arg[0].v.idt;				
			if(ai)													
				if(sr == dr && si == di)							
					if(_D_ALWAYS1 || (p.rds == 1 && p.ids == 1 && p.arg[0].v.rs == 1 && p.arg[0].v.is == 1)) 
						_D_LOOP(i,p.frames)				 
						{ FUN(*dr,*di,*dr,*di,*ar,*ai);	dr++,di++,ar++,ai++; }		
						_E_LOOP												
					else											
						_D_LOOP(i,p.frames)				 
						{ FUN(*dr,*di,*dr,*di,*ar,*ai);	dr += p.rds,di += p.ids,ar += p.arg[0].v.rs,ai += p.arg[0].v.is; }		
						_E_LOOP												
				else												
					_D_LOOP(i,p.frames)				 
					{ FUN(*dr,*di,*sr,*si,*ar,*ai);	sr += p.rss,si += p.iss,dr += p.rds,di += p.ids,ar += p.arg[0].v.rs,ai += p.arg[0].v.is; }			
					_E_LOOP												
			else													
				if(sr == dr && si == di)							
					_D_LOOP(i,p.frames)				 
					{ FUN(*dr,*di,*dr,*di,*ar,0); dr += p.rds,di += p.ids,ar += p.arg[0].v.rs; }	
					_E_LOOP												
				else												
					_D_LOOP(i,p.frames)				 
					{ FUN(*dr,*di,*sr,*si,*ar,0); sr += p.rss,si += p.iss,dr += p.rds,di += p.ids,ar += p.arg[0].v.rs; } 
					_E_LOOP												
			break;												
		}														
		case OpParam::Arg::arg_env: {							
			Env::Iter it(*p.arg[0].e.env); it.Init(0);			
			if(sr == dr && si == di)							
				if(_D_ALWAYS1 || (p.rds == 1 && p.ids == 1)) 
					_D_LOOP(i,p.frames)				 
					{ FUN(*dr,*di,*dr,*di,it.ValFwd(i),0); dr++,di++; }			
					_E_LOOP												
				else											
					_D_LOOP(i,p.frames)				 
					{ FUN(*dr,*di,*dr,*di,it.ValFwd(i),0); dr += p.rds,di += p.ids; }			
					_E_LOOP												
			else												
				_D_LOOP(i,p.frames)				 
					{ FUN(*dr,*di,*sr,*si,it.ValFwd(i),0); sr += p.rss,si += p.iss,dr += p.rds,di += p.ids; }	
				_E_LOOP												
			break;												
		}														
		case OpParam::Arg::arg_x: {								
			register const R ar = p.arg[0].x.r,ai = p.arg[0].x.i;				
			if(sr == dr && si == di)							
				if(_D_ALWAYS1 || (p.rds == 1 && p.ids == 1)) 
					_D_LOOP(i,p.frames)				 
					{ FUN(*dr,*di,*dr,*di,ar,ai); dr++,di++; }			
					_E_LOOP												
				else											
					_D_LOOP(i,p.frames)				 
					{ FUN(*dr,*di,*dr,*di,ar,ai); dr += p.rds,di += p.ids; }			
					_E_LOOP												
			else												
				_D_LOOP(i,p.frames)				 
				{ FUN(*dr,*di,*sr,*si,ar,ai); sr += p.rss,si += p.iss,dr += p.rds,di += p.ids; }	
				_E_LOOP												
			break;												
		}														
		}														
	}															
	else {														
		register const S rv = p.cbin.rarg,iv = p.cbin.iarg;		
		if(sr == dr && si == di)								
			if(_D_ALWAYS1 || (p.rds == 1 && p.ids == 1))		
				_D_LOOP(i,p.frames)						
				{ FUN(*dr,*di,*dr,*di,rv,iv); dr++,di++; }					
				_E_LOOP												
			else												
				_D_LOOP(i,p.frames)				 
				{ FUN(*dr,*di,*dr,*di,rv,iv); dr += p.rds,di += p.ids; }					
				_E_LOOP												
		else													
			if(_D_ALWAYS1 || (p.rds == 1 && p.ids == 1 && p.rss == 1 && p.iss == 1)) 
				_D_LOOP(i,p.frames)				 
				{ FUN(*dr,*di,*sr,*si,rv,iv); sr++,si++,dr++,di++; }					
				_E_LOOP												
			else												
				_D_LOOP(i,p.frames)				 
				{ FUN(*dr,*di,*sr,*si,rv,iv); sr += p.rss,si += p.iss,dr += p.rds,di += p.ids; }					
				_E_LOOP												
	}															
	return true;												
}

template<class T,class CL> inline BL _D__cbin(OpParam &p) { return _F__cbin<T,CL::cbin>(p); }			
template<class T,class CL> inline BL d__cbin(OpParam &p) { return _d__cbin<T>(CL::cbin,p); }


/*! \brief skeleton for real operations with parameter block
*/
template<class T,V FUN(T &v,T r,OpParam &p)> BL _F__rop(OpParam &p)						
{																
	register const S *sr = p.rsdt;								
	register S *dr = p.rddt;									
	register I i;												
	if(sr == dr)												
		if(_D_ALWAYS1 || p.rds == 1)											
			_D_LOOP(i,p.frames)										
            { FUN(*dr,*dr,p); dr++; }							
			_E_LOOP												
		else													
			_D_LOOP(i,p.frames)									
			{ FUN(*dr,*dr,p); dr += p.rds; }					
			_E_LOOP												
	else														
		if(_D_ALWAYS1 || (p.rss == 1 && p.rds == 1))			
			_D_LOOP(i,p.frames)									
			{ FUN(*dr,*sr,p); sr++,dr++; }					
			_E_LOOP												
		else													
			_D_LOOP(i,p.frames)									
			{ FUN(*dr,*sr,p); sr += p.rss,dr += p.rds; }		
			_E_LOOP												
	return true;												
}

template<class T,class CL> inline BL _D__rop(OpParam &p) { return _F__rop<T,CL::rop>(p); }			
template<class T,class CL> inline BL d__rop(OpParam &p) { return _d__rop<T>(CL::rop,p); }


/*! \brief skeleton for complex operations with parameter block
*/
template<class T,V FUN(T &rv,T &iv,T ra,T ia,OpParam &p)> BL _F__cop(OpParam &p)						
{																
	register const S *sr = p.rsdt,*si = p.isdt;					
	register S *dr = p.rddt,*di = p.iddt;						
	register I i;												
	if(sr == dr && si == di)									
		if(_D_ALWAYS1 || (p.rds == 1 && p.ids == 1))			
			_D_LOOP(i,p.frames)									
			{ FUN(*dr,*di,*dr,*di,p); dr++,di++; }			
			_E_LOOP												
		else													
			_D_LOOP(i,p.frames)									
			{ FUN(*dr,*di,*dr,*di,p); dr += p.rds,di += p.ids; } 
			_E_LOOP												
	else														
		if(_D_ALWAYS1 || (p.rss == 1 && p.iss == 1 && p.rds == 1 && p.ids == 1)) 
			_D_LOOP(i,p.frames)				 
			{ FUN(*dr,*di,*sr,*si,p); sr++,si++,dr++,di++; }	
			_E_LOOP												
		else													
			_D_LOOP(i,p.frames)				 
			{ FUN(*dr,*di,*sr,*si,p); sr += p.rss,si += p.iss,dr += p.rds,di += p.ids; }	
			_E_LOOP												
	return true;												
}

template<class T,class CL> inline BL _D__cop(OpParam &p) { return _F__cop<T,CL::cop>(p); }
template<class T,class CL> inline BL d__cop(OpParam &p) { return _d__cop<T>(CL::cop,p); }

template<class T,V FUN(T &v,T a)> inline BL f__run(OpParam &p) { return _d__run(FUN,p); }
template<class T,V FUN(T &rv,T &iv,T ra,T ia)> inline BL f__cun(OpParam &p) { return _d__cun(FUN,p); }
template<class T,V FUN(T &v,T a,T b)> inline BL f__rbin(OpParam &p) { return _d__rbin(FUN,p); }
template<class T,V FUN(T &rv,T &iv,T ra,T ia,T rb,T ib)> inline BL f__cbin(OpParam &p) { return _d__cbin(FUN,p); }
template<class T,V FUN(T &v,T r,OpParam &p)> inline BL f__rop(OpParam &p) { return _d__rop(FUN,p); }
template<class T,V FUN(T &rv,T &iv,T ra,T ia,OpParam &p)> inline BL f__cop(OpParam &p) { return _d__cop(FUN,p); }


#ifdef VASP_COMPACT
template<class T,class CL> inline BL D__run(OpParam &p) { return _d__run<T>(CL::run,p); }
template<class T,class CL> inline BL D__cun(OpParam &p) { return _d__cun<T>(CL::cun,p); }
template<class T,class CL> inline BL D__rbin(OpParam &p) { return _d__rbin<T>(CL::rbin,p); }
template<class T,class CL> inline BL D__cbin(OpParam &p) { return _d__cbin<T>(CL::cbin,p); }
template<class T,class CL> inline BL D__rop(OpParam &p) { return _d__rop<T>(CL::rop,p); }
template<class T,class CL> inline BL D__cop(OpParam &p) { return _d__cop<T>(CL::cop,p); }
template<class T,V FUN(T &v,T a)> inline BL F__run(OpParam &p) { return _d__run(FUN,p); }
template<class T,V FUN(T &rv,T &iv,T ra,T ia)> inline BL F__cun(OpParam &p) { return _d__cun<T>(FUN,p); }
template<class T,V FUN(T &v,T a,T b)> inline BL F__rbin(OpParam &p) { return _d__rbin<T>(FUN,p); }
template<class T,V FUN(T &rv,T &iv,T ra,T ia,T rb,T ib)> inline BL F__cbin(OpParam &p) { return _d__cbin<T>(FUN,p); }
template<class T,V FUN(T &v,T r,OpParam &p)> inline BL F__rop(OpParam &p) { return _d__rop<T>(FUN,p); }
template<class T,V FUN(T &rv,T &iv,T ra,T ia,OpParam &p)> inline BL F__cop(OpParam &p) { return _d__cop<T>(FUN,p); }
#else
template<class T,class CL> inline BL D__run(OpParam &p) { return _D__run<T,CL>(p); }
template<class T,class CL> inline BL D__cun(OpParam &p) { return _D__cun<T,CL>(p); }
template<class T,class CL> inline BL D__rbin(OpParam &p) { return _D__rbin<T,CL>(p); }
template<class T,class CL> inline BL D__cbin(OpParam &p) { return _D__cbin<T,CL>(p); }
template<class T,class CL> inline BL D__rop(OpParam &p) { return _D__rop<T,CL>(p); }
template<class T,class CL> inline BL D__cop(OpParam &p) { return _D__cop<T,CL>(p); }
template<class T,V FUN(T &v,T a)> inline BL F__run(OpParam &p) { return _F__run<T,FUN>(p); }
template<class T,V FUN(T &rv,T &iv,T ra,T ia)> inline BL F__cun(OpParam &p) { return _F__cun<T,FUN>(p); }
template<class T,V FUN(T &v,T a,T b)> inline BL F__rbin(OpParam &p) { return _F__rbin<T,FUN>(p); }
template<class T,V FUN(T &rv,T &iv,T ra,T ia,T rb,T ib)> inline BL F__cbin(OpParam &p) { return _F__cbin<T,FUN>(p); }
template<class T,V FUN(T &v,T r,OpParam &p)> inline BL F__rop(OpParam &p) { return _F__rop<T,FUN>(p); }
template<class T,V FUN(T &rv,T &iv,T ra,T ia,OpParam &p)> inline BL F__cop(OpParam &p) { return _F__cop<T,FUN>(p); }
#endif

} // namespace VecOp

#endif
