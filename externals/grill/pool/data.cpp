/* 

pool - hierarchical storage object for PD and Max/MSP

Copyright (c) 2002-2003 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pool.h"

#include <string.h>
#include <fstream.h>
#include <ctype.h>
#include <stdlib.h>


pooldata::pooldata(const S *s,I vcnt,I dcnt):
	sym(s),nxt(NULL),refs(0),
	root(nullatom,NULL,vcnt,dcnt)
{
	FLEXT_LOG1("new pool %s",sym?flext_base::GetString(sym):"<private>");
}

pooldata::~pooldata()
{
	FLEXT_LOG1("free pool %s",sym?flext_base::GetString(sym):"<private>");
}


const A pooldata::nullatom = { A_NULL };


V pooldata::Reset()
{
	root.Reset();
}

BL pooldata::MkDir(const AtomList &d,I vcnt,I dcnt)
{
	root.AddDir(d,vcnt,dcnt);
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

flext::AtomList *pooldata::Peek(const AtomList &d,const A &key)
{
	pooldir *pd = root.GetDir(d);
	return pd?pd->PeekVal(key):NULL;
}

poolval *pooldata::Ref(const AtomList &d,const A &key)
{
	pooldir *pd = root.GetDir(d);
	return pd?pd->RefVal(key):NULL;
}

poolval *pooldata::Refi(const AtomList &d,I ix)
{
	pooldir *pd = root.GetDir(d);
	return pd?pd->RefVali(ix):NULL;
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

I pooldata::CntSub(const AtomList &d)
{
	pooldir *pd = root.GetDir(d);
	return pd?pd->CntSub():0;
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
			pooldir *ret = new pooldir(nullatom,NULL,pd->VSize(),pd->DSize());
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
		// What sizes should we choose here?
		pooldir *ret = new pooldir(nullatom,NULL,pd->VSize(),pd->DSize());
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
#if FLEXT_SYS == FLEXT_SYS_PD && FLEXT_OS == FLEXT_OS_WIN
	I i,cnt = strlen(src);
	if(cnt >= sz-1) return NULL;
	for(i = 0; i < cnt; ++i)
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
			AtomList tmp;
			if(absdir) tmp = d;
			return fl.good() && pd->SvDir(fl,depth,tmp);
		}
		else return false;
	}
	else
		return false;
}

BL pooldata::LdDirXML(const AtomList &d,const C *flnm,I depth,BL mkdir)
{
	pooldir *pd = root.GetDir(d);
	if(pd) {
		C tmp[1024];
		const C *t = CnvFlnm(tmp,flnm,sizeof tmp);
		if(t) {
			ifstream fl(t);
            BL ret = fl.good() != 0;
            if(ret) {
                fl.getline(tmp,sizeof tmp);
                ret = !strncmp(tmp,"<?xml",5);
            }
            if(ret) {
                fl.getline(tmp,sizeof tmp);
                // DOCTYPE need not be present / only external DOCTYPE is allowed!
                if(!strncmp(tmp,"<!DOCTYPE",9)) 
                    fl.getline(tmp,sizeof tmp);
                ret = !strncmp(tmp,"<pool>",6);
            }
            if(ret)
                ret = pd->LdDirXML(fl,depth,mkdir);
            return ret;
		}
	}

    return false;
}

BL pooldata::SvDirXML(const AtomList &d,const C *flnm,I depth,BL absdir)
{
	pooldir *pd = root.GetDir(d);
	if(pd) {
		C tmp[1024];
		const C *t = CnvFlnm(tmp,flnm,sizeof tmp);
		if(t) {
			ofstream fl(t);
			AtomList tmp;
			if(absdir) tmp = d;
            if(fl.good()) {
                fl << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>" << endl;
                fl << "<!DOCTYPE pool SYSTEM \"pool.dtd\">" << endl;
                fl << "<pool>" << endl;
                BL ret = pd->SvDirXML(fl,depth,tmp);
                fl << "</pool>" << endl;
                return ret;
            }
		}
	}

    return false;
}


