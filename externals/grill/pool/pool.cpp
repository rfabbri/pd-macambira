/* 

pool - hierarchical storage object for PD and Max/MSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pool.h"

#include <string.h>
#include <fstream.h>
#include <ctype.h>
#include <stdlib.h>


inline I compare(I a,I b) { return a == b?0:(a < b?-1:1); }
inline I compare(F a,F b) { return a == b?0:(a < b?-1:1); }

static I compare(const S *a,const S *b) 
{
	if(a == b)
		return 0;
	else
		return strcmp(flext::GetString(a),flext::GetString(b));
}

static I compare(const A &a,const A &b) 
{
	if(flext::GetType(a) == flext::GetType(b)) {
		switch(flext::GetType(a)) {
		case A_FLOAT:
			return compare(flext::GetFloat(a),flext::GetFloat(b));
#if FLEXT_SYS == FLEXT_SYS_MAX
		case A_LONG:
			return compare(flext::GetInt(a),flext::GetInt(b));
#endif
		case A_SYMBOL:
			return compare(flext::GetSymbol(a),flext::GetSymbol(b));
#if FLEXT_SYS == FLEXT_SYS_PD
		case A_POINTER:
			return flext::GetPointer(a) == flext::GetPointer(b)?0:(flext::GetPointer(a) < flext::GetPointer(b)?-1:1);
#endif
		default:
			LOG("pool - atom comparison: type not handled");
			return -1;
		}
	}
	else
		return flext::GetType(a) < flext::GetType(b)?-1:1;
}


poolval::poolval(const A &k,AtomList *d):
	data(d),nxt(NULL)
{
	SetAtom(key,k);
}

poolval::~poolval()
{
	if(data) delete data;
	if(nxt) delete nxt;
}

poolval &poolval::Set(AtomList *d)
{
	if(data) delete data;
	data = d;
	return *this;
}

poolval *poolval::Dup() const
{
	return new poolval(key,data?new AtomList(*data):NULL); 
}


pooldir::pooldir(const A &d,pooldir *p):
	parent(p),dirs(NULL),vals(NULL),nxt(NULL)
{
	CopyAtom(&dir,&d);
}

pooldir::~pooldir()
{
	Clear(true);
	if(nxt) delete nxt;
}

V pooldir::Clear(BL rec,BL dironly)
{
	if(rec && dirs) { delete dirs; dirs = NULL; }
	if(!dironly && vals) { delete vals; vals = NULL; }
}

pooldir *pooldir::AddDir(I argc,const A *argv)
{
	if(!argc) return this;

	I c = 1;
	pooldir *prv = NULL,*ix = dirs;
	for(; ix; prv = ix,ix = ix->nxt) {
		c = compare(argv[0],ix->dir);
		if(c <= 0) break;
	}

	if(c || !ix) {
		pooldir *nd = new pooldir(argv[0],this);
		nd->nxt = ix;

		if(prv) prv->nxt = nd;
		else dirs = nd;
		ix = nd;
	}

	return ix->AddDir(argc-1,argv+1);
}

pooldir *pooldir::GetDir(I argc,const A *argv,BL rmv)
{
	if(!argc) return this;

	I c = 1;
	pooldir *prv = NULL,*ix = dirs;
	for(; ix; prv = ix,ix = ix->nxt) {
		c = compare(argv[0],ix->dir);
		if(c <= 0) break;
	}

	if(c || !ix) 
		return NULL;
	else {
		if(argc > 1)
			return ix->GetDir(argc-1,argv+1,rmv);
		else if(rmv) {
			pooldir *nd = ix->nxt;
			if(prv) prv->nxt = nd;
			else dirs = nd;
			ix->nxt = NULL;
			return ix;
		}
		else 
			return ix;
	}
}

BL pooldir::DelDir(I argc,const A *argv)
{
	pooldir *pd = GetDir(argc,argv,true);
	if(pd && pd != this) {
		delete pd;
		return true;
	}
	else 
		return false;
}

V pooldir::SetVal(const A &key,AtomList *data,BL over)
{
	I c = 1;
	poolval *prv = NULL,*ix = vals;
	for(; ix; prv = ix,ix = ix->nxt) {
		c = compare(key,ix->key);
		if(c <= 0) break;
	}

	if(c || !ix) {
		// no existing data found
	
		if(data) {
			poolval *nv = new poolval(key,data);
			nv->nxt = ix;

			if(prv) prv->nxt = nv;
			else vals = nv;
		}
	}
	else if(over) { 
		// data exists... only set if overwriting enabled
		
		if(data)
			ix->Set(data);
		else {
			poolval *nv = ix->nxt;
			if(prv) prv->nxt = nv;
			else vals = nv;
			ix->nxt = NULL;
			delete ix;
		}
	}
}

poolval *pooldir::RefVal(const A &key)
{
	I c = 1;
	poolval *ix = vals;
	for(; ix; ix = ix->nxt) {
		c = compare(key,ix->key);
		if(c <= 0) break;
	}

	return c || !ix?NULL:ix;
}

poolval *pooldir::RefVali(I rix)
{
	I c = 0;
	poolval *ix = vals;
	for(; ix && c < rix; ix = ix->nxt,++c) {}

	return c == rix?ix:NULL;
}

flext::AtomList *pooldir::PeekVal(const A &key)
{
	poolval *ix = RefVal(key);
	return ix?ix->data:NULL;
}

flext::AtomList *pooldir::GetVal(const A &key,BL cut)
{
	I c = 1;
	poolval *prv = NULL,*ix = vals;
	for(; ix; prv = ix,ix = ix->nxt) {
		c = compare(key,ix->key);
		if(c <= 0) break;
	}

	if(c || !ix) 
		return NULL;
	else {
		AtomList *ret;
		if(cut) {
			poolval *nv = ix->nxt;
			if(prv) prv->nxt = nv;
			else vals = nv;
			ix->nxt = NULL;
			ret = ix->data; ix->data = NULL;
			delete ix;
		}
		else
			ret = new AtomList(*ix->data);
		return ret;
	}
}

I pooldir::CntAll()
{
	I cnt = 0;
	poolval *ix = vals;
	for(; ix; ix = ix->nxt,++cnt) {}
	return cnt;
}

I pooldir::GetKeys(AtomList &keys)
{
	I cnt = CntAll();
	keys(cnt);

	poolval *ix = vals;
	for(I i = 0; ix; ++i,ix = ix->nxt) 
		SetAtom(keys[i],ix->key);

	return cnt;
}

I pooldir::GetAll(A *&keys,AtomList *&lst,BL cut)
{
	I cnt = CntAll();
	keys = new A[cnt];
	lst = new AtomList[cnt];

	poolval *ix = vals;
	for(I i = 0; ix; ++i) {
		SetAtom(keys[i],ix->key);
		lst[i] = *ix->data;

		if(cut) {
			poolval *t = ix;
			vals = ix = ix->nxt;
			t->nxt = NULL; delete t;
		}
		else
			ix = ix->nxt;
	}

	return cnt;
}

I pooldir::GetSub(const A **&lst)
{
	I cnt = 0;
	pooldir *ix = dirs;
	for(; ix; ix = ix->nxt,++cnt) {}
	lst = new const A *[cnt];

	ix = dirs;
	for(I i = 0; ix; ix = ix->nxt,++i) {
		lst[i] = &ix->dir;
	}

	return cnt;
}


BL pooldir::Paste(const pooldir *p,I depth,BL repl,BL mkdir)
{
	BL ok = true;

	for(poolval *ix = p->vals; ix; ix = ix->nxt) {
		SetVal(ix->key,new AtomList(*ix->data),repl);
	}

	if(ok && depth) {
		for(pooldir *dix = p->dirs; ok && dix; dix = dix->nxt) {
			pooldir *ndir = mkdir?AddDir(1,&dix->dir):GetDir(1,&dix->dir);
			if(ndir) { 
				ok = ndir->Paste(dix,depth > 0?depth-1:depth,repl,mkdir);
			}
		}
	}

	return ok;
}

BL pooldir::Copy(pooldir *p,I depth,BL cut)
{
	BL ok = true;

	if(cut) {
		if(p->vals) 
			ok = false;
		else
			p->vals = vals, vals = NULL;
	}
	else {
		// inefficient!! p->SetVal has to search through list unnecessarily!!
		for(poolval *ix = vals; ix; ix = ix->nxt) {
			p->SetVal(ix->key,new AtomList(*ix->data));
		}
	}

	if(ok && depth) {
		// also quite inefficient for cut 
		for(pooldir *dix = dirs; ok && dix; dix = dix->nxt) {
			pooldir *ndir = p->AddDir(1,&dix->dir);
			if(ndir)
				ok = ndir->Copy(dix,depth > 0?depth-1:depth,cut);
			else
				ok = false;
		}
	}

	return ok;
}


static C *ReadAtom(C *c,A *a)
{
	// skip whitespace
	while(*c && isspace(*c)) ++c;
	if(!*c) return NULL;

	const C *m = c; // remember position

	// check for word type (s = 0,1,2 ... int,float,symbol)
	I s = 0;
	for(; *c && !isspace(*c); ++c) {
		if(!isdigit(*c)) 
			s = (*c != '.' || s == 1)?2:1;
	}

	if(a) {
		switch(s) {
		case 0: // integer
#if FLEXT_SYS == FLEXT_SYS_MAX
			flext::SetInt(*a,atoi(m));
			break;
#endif
		case 1: // float
			flext::SetFloat(*a,(F)atof(m));
			break;
		default: { // anything else is a symbol
			C t = *c; *c = 0;
			flext::SetString(*a,m);
			*c = t;
			break;
		}
		}
	}

	return c;
}

static BL ReadAtoms(istream &is,flext::AtomList &l,C del)
{
	C tmp[1024];
	is.getline(tmp,sizeof tmp,del); 
	if(is.eof() || !is.good()) return false;

	I i,cnt;
	C *t = tmp;
	for(cnt = 0; ; ++cnt) {
		t = ReadAtom(t,NULL);
		if(!t) break;
	}

	l(cnt);
	if(cnt) {
		for(i = 0,t = tmp; i < cnt; ++i)
			t = ReadAtom(t,&l[i]);
	}
	return true;
}

static V WriteAtom(ostream &os,const A &a)
{
	switch(a.a_type) {
	case A_FLOAT:
		os << a.a_w.w_float;
		break;
#if FLEXT_SYS == FLEXT_SYS_MAX
	case A_LONG:
		os << a.a_w.w_long;
		break;
#endif
	case A_SYMBOL:
		os << flext::GetString(flext::GetSymbol(a));
		break;
	}
}

static V WriteAtoms(ostream &os,const flext::AtomList &l)
{
	for(I i = 0; i < l.Count(); ++i) {
		WriteAtom(os,l[i]);
		os << ' ';
	}
}

BL pooldir::LdDir(istream &is,I depth,BL mkdir)
{
	BL r;
	for(I i = 1; !is.eof(); ++i) {
		AtomList d,k,*v = new AtomList;
		r = ReadAtoms(is,d,',');
		r = r && ReadAtoms(is,k,',') && k.Count() == 1;
		r = r && ReadAtoms(is,*v,'\n') && v->Count();

		if(r) {
			if(depth < 0 || d.Count() <= depth) {
				pooldir *nd = mkdir?AddDir(d):GetDir(d);
				if(nd) {
					nd->SetVal(k[0],v); v = NULL;
				}
	#ifdef FLEXT_DEBUG
				else
					post("pool - directory was not found",i);
	#endif
			}
		}
		else if(!is.eof())
			post("pool - format mismatch encountered, skipped line %i",i);

		if(v) delete v;
	}
	return true;
}

BL pooldir::SvDir(ostream &os,I depth,const AtomList &dir)
{
	{
		for(poolval *ix = vals; ix; ix = ix->nxt) {
			WriteAtoms(os,dir);
			os << ", ";
			WriteAtom(os,ix->key);
			os << " , ";
			WriteAtoms(os,*ix->data);
			os << endl;
		}
	}
	if(depth) {
		I nd = depth > 0?depth-1:-1;
		for(pooldir *ix = dirs; ix; ix = ix->nxt) {
			ix->SvDir(os,nd,AtomList(dir).Append(ix->dir));
		}
	}
	return true;
}




