/* 

pool - hierarchical storage object for PD and Max/MSP

Copyright (c) 2002-2006 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pool.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <fstream>

using namespace std;

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

I pooldata::GetAll(const AtomList &d,A *&keys,Atoms *&lst)
{
	pooldir *pd = root.GetDir(d);
	if(pd)
		return pd->GetAll(keys,lst);
	else {
		keys = NULL; lst = NULL;
		return 0;
	}
}

I pooldata::PrintAll(const AtomList &d)
{
    char tmp[1024];
    d.Print(tmp,sizeof tmp);
    pooldir *pd = root.GetDir(d);
    strcat(tmp," , ");
	int cnt = pd?pd->PrintAll(tmp,sizeof tmp):0;
    if(!cnt) post(tmp);
    return cnt;
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
	return pd && pd->Paste(clip,depth,repl,mkdir);
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


static const char *CnvFlnm(char *dst,const char *src,int sz)
{
#if FLEXT_SYS == FLEXT_SYS_PD && FLEXT_OS == FLEXT_OS_WIN
	int i,cnt = strlen(src);
	if(cnt >= sz-1) return NULL;
	for(i = 0; i < cnt; ++i)
		dst[i] = src[i] != '/'?src[i]:'\\';
	dst[i] = 0;
	return dst;
#else
	return src;
#endif
}

bool pooldata::LdDir(const AtomList &d,const char *flnm,int depth,bool mkdir)
{
	pooldir *pd = root.GetDir(d);
	if(pd) {
		char tmp[1024];
		const char *t = CnvFlnm(tmp,flnm,sizeof tmp);
		if(t) {
			ifstream file(t);
			return file.good() && pd->LdDir(file,depth,mkdir);
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
			ofstream file(t);
			Atoms tmp;
			if(absdir) tmp = d;
			return file.good() && pd->SvDir(file,depth,tmp);
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
			ifstream file(t);
            BL ret = file.good() != 0;
            if(ret) {
                file.getline(tmp,sizeof tmp);
                ret = !strncmp(tmp,"<?xml",5);
            }
/*
            if(ret) {
                fl.getline(tmp,sizeof tmp);
                // DOCTYPE need not be present / only external DOCTYPE is allowed!
                ret = !strncmp(tmp,"<!DOCTYPE",9);
            }
*/
            if(ret)
                ret = pd->LdDirXML(file,depth,mkdir);
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
			ofstream file(t);
			Atoms tmp;
			if(absdir) tmp = d;
            if(file.good()) {
                file << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>" << endl;
                file << "<!DOCTYPE pool SYSTEM \"http://grrrr.org/ext/pool/pool-0.2.dtd\">" << endl;
                file << "<pool>" << endl;
                BL ret = pd->SvDirXML(file,depth,tmp);
                file << "</pool>" << endl;
                return ret;
            }
		}
	}

    return false;
}
