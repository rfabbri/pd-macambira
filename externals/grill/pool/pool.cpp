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
	if(a.a_type == b.a_type) {
		switch(a.a_type) {
		case A_FLOAT:
			return compare(a.a_w.w_float,b.a_w.w_float);
#ifdef MAXMSP
		case A_LONG:
			return compare((I)a.a_w.w_long,(I)b.a_w.w_long);
#endif
		case A_SYMBOL:
			return compare(a.a_w.w_symbol,b.a_w.w_symbol);
#ifdef PD
		case A_POINTER:
			return a.a_w.w_gpointer == b.a_w.w_gpointer?0:(a.a_w.w_gpointer < b.a_w.w_gpointer?-1:1);
#endif
		default:
			LOG("pool - atom comparison: type not handled");
			return -1;
		}
	}
	else
		return a.a_type < b.a_type?-1:1;
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


pooldir::pooldir(const A &d):
	dirs(NULL),vals(NULL),nxt(NULL)
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
		pooldir *nd = new pooldir(argv[0]);
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

BL pooldir::DelDir(const AtomList &d)
{
	pooldir *pd = GetDir(d,true);
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
#ifdef MAXMSP
			a->a_type = A_LONG;
			a->a_w.w_long = atol(m);
			break;
#endif
		case 1: // float
			a->a_type = A_FLOAT;
			a->a_w.w_float = (F)atof(m);
			break;
		default: { // anything else is a symbol
			C t = *c; *c = 0;
			a->a_type = A_SYMBOL;
			a->a_w.w_symbol = (S *)flext::MakeSymbol(m);
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
#ifdef MAXMSP
	case A_LONG:
		os << a.a_w.w_long;
		break;
#endif
	case A_SYMBOL:
		os << flext::GetString(a.a_w.w_symbol);
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
	#ifdef _DEBUG
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




pooldata::pooldata(const S *s):
	sym(s),nxt(NULL),refs(0),
	root(nullatom)
{
	LOG1("new pool %s",sym?flext_base::GetString(sym):"<private>");
}

pooldata::~pooldata()
{
	LOG1("free pool %s",sym?flext_base::GetString(sym):"<private>");
}

t_atom pooldata::nullatom = { A_NULL };


V pooldata::Reset()
{
	root.Clear(true);
}

BL pooldata::MkDir(const AtomList &d)
{
	root.AddDir(d);
	return true;
}

BL pooldata::ChkDir(const AtomList &d)
{
	return root.GetDir(d) != NULL;
}

BL pooldata::RmDir(const AtomList &d)
{
	return root.DelDir(d);
}

BL pooldata::Set(const AtomList &d,const A &key,AtomList *data,BL over)
{
	pooldir *pd = root.GetDir(d);
	if(!pd) return false;
	pd->SetVal(key,data,over);
	return true;
}

BL pooldata::Clr(const AtomList &d,const A &key)
{
	pooldir *pd = root.GetDir(d);
	if(!pd) return false;
	pd->ClrVal(key);
	return true;
}

BL pooldata::ClrAll(const AtomList &d,BL rec,BL dironly)
{
	pooldir *pd = root.GetDir(d);
	if(!pd) return false;
	pd->Clear(rec,dironly);
	return true;
}

flext::AtomList *pooldata::Get(const AtomList &d,const A &key)
{
	pooldir *pd = root.GetDir(d);
	return pd?pd->GetVal(key):NULL;
}

I pooldata::CntAll(const AtomList &d)
{
	pooldir *pd = root.GetDir(d);
	return pd?pd->CntAll():0;
}

I pooldata::GetAll(const AtomList &d,A *&keys,AtomList *&lst)
{
	pooldir *pd = root.GetDir(d);
	if(pd)
		return pd->GetAll(keys,lst);
	else {
		keys = NULL; lst = NULL;
		return 0;
	}
}

I pooldata::GetSub(const AtomList &d,const t_atom **&dirs)
{
	pooldir *pd = root.GetDir(d);
	if(pd)
		return pd->GetSub(dirs);
	else {
		dirs = NULL;
		return 0;
	}
}


BL pooldata::Paste(const AtomList &d,const pooldir *clip,I depth,BL repl,BL mkdir)
{
	pooldir *pd = root.GetDir(d);
	if(pd)
		return pd->Paste(clip,depth,repl,mkdir);
	else
		return false;
}

pooldir *pooldata::Copy(const AtomList &d,const A &key,BL cut)
{
	pooldir *pd = root.GetDir(d);
	if(pd) {
		AtomList *val = pd->GetVal(key,cut);
		if(val) {
			pooldir *ret = new pooldir(nullatom);
			ret->SetVal(key,val);
			return ret;
		}
		else
			return NULL;
	}
	else
		return NULL;
}

pooldir *pooldata::CopyAll(const AtomList &d,I depth,BL cut)
{
	pooldir *pd = root.GetDir(d);
	if(pd) {
		pooldir *ret = new pooldir(nullatom);
		if(pd->Copy(ret,depth,cut))
			return ret;
		else {
			delete ret;
			return NULL;
		}
	}
	else
		return NULL;
}


static const C *CnvFlnm(C *dst,const C *src,I sz)
{
#if defined(PD) && defined(NT)
	I cnt = strlen(src);
	if(cnt >= sz-1) return NULL;
	for(I i = 0; i < cnt; ++i)
		dst[i] = src[i] != '/'?src[i]:'\\';
	dst[i] = 0;
	return dst;
#else
	return src;
#endif
}

BL pooldata::LdDir(const AtomList &d,const C *flnm,I depth,BL mkdir)
{
	pooldir *pd = root.GetDir(d);
	if(pd) {
		C tmp[1024];
		const C *t = CnvFlnm(tmp,flnm,sizeof tmp);
		if(t) {
			ifstream fl(t);
			return fl.good() && pd->LdDir(fl,depth,mkdir);
		}
		else return false;
	}
	else
		return false;
}

BL pooldata::SvDir(const AtomList &d,const C *flnm,I depth,BL absdir)
{
	pooldir *pd = root.GetDir(d);
	if(pd) {
		C tmp[1024];
		const C *t = CnvFlnm(tmp,flnm,sizeof tmp);
		if(t) {
			ofstream fl(t);
			return fl.good() && pd->SvDir(fl,depth,absdir?d:AtomList());
		}
		else return false;
	}
	else
		return false;
}


