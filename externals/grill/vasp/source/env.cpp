/* 

VASP modular - vector assembling signal processor / objects for Max/MSP and PD

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "env.h"
#include "classes.h"
#include "util.h"

Env::Env(I argc,const t_atom *argv)
{
	I ix = 0;
	t_symbol *v = ix < argc?flext::GetASymbol(argv[ix]):NULL;
	if(v && v == vasp_base::sym_env) ix++; // if it is "env" ignore it

	cnt = (argc-ix)/2;
	pos = new R[cnt];
	val = new R[cnt];

	R prev = -BIG;
	BL ok = true;
	for(I i = 0; i < cnt; ++i) {
		val[i] = flext::GetAFloat(argv[ix++]);
		pos[i] = flext::GetAFloat(argv[ix++]);
		if(pos[i] < prev) ok = false;
		prev = pos[i];
	}

	if(ix < argc) {
		post("vasp - env pos/value pairs incomplete, omitted dangling value");
	}

	if(!ok) Clear();
}

/*
Env::Env(const Env &s):
	cnt(s.cnt),pos(new R[s.cnt]),val(new R[s.cnt])
{
	for(I i = 0; i < cnt; ++i) pos[i] = s.pos[i],val[i] = s.val[i];
}
*/

Env::~Env() { Clear(); }


BL Env::ChkArgs(I argc,const t_atom *argv)
{
	I ix = 0;

	// vasp keyword
	t_symbol *v = ix < argc?flext::GetASymbol(argv[ix]):NULL;
	if(v && v == vasp_base::sym_env) ix++; // if it is "env" ignore it

	while(argc > ix) {
		// check for position
		if(flext::CanbeFloat(argv[ix])) ix++;
		else 
			return false;

		// check for value
		if(argc > ix)
			if(flext::CanbeFloat(argv[ix])) ix++;
			else 
				return false;
	}

	return true;
}



V Env::Clear()
{
	cnt = 0;
	if(pos) delete[] pos; pos = NULL;
	if(val) delete[] val; val = NULL;
}


Env::Iter::Iter(const Env &bpl): bp(bpl),ppt(-BIG),npt(BIG),pvl(0),k(0) {}

V Env::Iter::Init(R p) 
{
	I cnt = bp.Count();
	ASSERT(cnt > 0);

	if(p < bp.Pos(0)) {
		// position is before the head
		ix = -1;
		ppt = -BIG; pvl = bp.Val(0);
	}
	else if(p > bp.Pos(cnt-1)) { 
		// position is after the tail
		ix = cnt-1;
		ppt = bp.Pos(ix); pvl = bp.Val(ix);
	}
	else { 
		// somewhere in the list
		for(ix = 0; ix < cnt; ++ix)
			if(p >= bp.Pos(ix)) break;
		ppt = bp.Pos(ix); pvl = bp.Val(ix);

		ASSERT(ix < cnt);
	}

	if(ix >= cnt) {
		npt = BIG; nvl = pvl;
		k = 0;
	}
	else {
		npt = bp.Pos(ix+1); nvl = bp.Val(ix+1);
		k = (nvl-pvl)/(npt-ppt); 
	}
}

// \todo iteration first, then calculation of k
V Env::Iter::UpdateFwd(R p) 
{
	do {
		ppt = npt,pvl = nvl;
		if(++ix >= bp.Count()-1) npt = BIG,k = 0;
		else {
			k = ((nvl = bp.Val(ix+1))-pvl)/((npt = bp.Pos(ix+1))-ppt); 
		}
	} while(p > npt);
}

// \todo iteration first, then calculation of k
V Env::Iter::UpdateBwd(R p) 
{
	do {
		npt = ppt,nvl = pvl;
		if(--ix < 0) ppt = -BIG,k = 0;
		else {
			k = (nvl-(pvl = bp.Val(ix)))/(npt-(ppt = bp.Pos(ix))); 
		}
	} while(p < ppt);
}
