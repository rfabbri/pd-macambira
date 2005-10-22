/* 

pool - hierarchical storage object for PD and Max/MSP

Copyright (c) 2002-2005 Thomas Grill
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __POOL_H
#define __POOL_H

#define FLEXT_ATTRIBUTES 1

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 500)
#error You need at least flext version 0.5.0
#endif

#include <iostream>

using namespace std;

typedef void V;
typedef int I;
typedef unsigned long UL;
typedef float F;
typedef char C;
typedef bool BL;
typedef t_atom A;
typedef t_symbol S;

typedef flext::AtomListStatic<8> Atoms;

class poolval:
	public flext
{
public:
	poolval(const A &key,AtomList *data);
	~poolval();

	poolval &Set(AtomList *data);
	poolval *Dup() const;

	A key;
	AtomList *data;
	poolval *nxt;
};

class pooldir:
	public flext
{
public:
	pooldir(const A &dir,pooldir *parent,I vcnt,I dcnt);
	~pooldir();

	V Clear(BL rec,BL dironly = false);
	V Reset(BL realloc = true);

	BL Empty() const { return !dirs && !vals; }
	BL HasDirs() const { return dirs != NULL; }
	BL HasVals() const { return vals != NULL; }

	pooldir *GetDir(I argc,const A *argv,BL cut = false);
	pooldir *GetDir(const AtomList &d,BL cut = false) { return GetDir(d.Count(),d.Atoms(),cut); }
	BL DelDir(I argc,const A *argv);
	BL DelDir(const AtomList &d) { return DelDir(d.Count(),d.Atoms()); }
	pooldir *AddDir(I argc,const A *argv,I vcnt = 0,I dcnt = 0);
	pooldir *AddDir(const AtomList &d,I vcnt = 0,I dcnt = 0) { return AddDir(d.Count(),d.Atoms(),vcnt,dcnt); }

	V SetVal(const A &key,AtomList *data,BL over = true);
	BL SetVali(I ix,AtomList *data);
	V ClrVal(const A &key) { SetVal(key,NULL); }
    BL ClrVali(I ix) { return SetVali(ix,NULL); }
	AtomList *PeekVal(const A &key);
	AtomList *GetVal(const A &key,BL cut = false);
	I CntAll() const;
	I GetAll(A *&keys,Atoms *&lst,BL cut = false);
	I PrintAll(char *buf,int len) const;
	I GetKeys(AtomList &keys);
	I CntSub() const;
	I GetSub(const A **&dirs);

	poolval *RefVal(const A &key);
	poolval *RefVali(I ix);
	
	BL Paste(const pooldir *p,I depth,BL repl,BL mkdir);
	BL Copy(pooldir *p,I depth,BL cur);

	BL LdDir(istream &is,I depth,BL mkdir);
	BL LdDirXML(istream &is,I depth,BL mkdir);
	BL SvDir(ostream &os,I depth,const AtomList &dir = AtomList());
	BL SvDirXML(ostream &os,I depth,const AtomList &dir = AtomList(),I ind = 0);

	int VSize() const { return vsize; }
	int DSize() const { return dsize; }

protected:
	int VIdx(const A &v) const { return FoldBits(AtomHash(v),vbits); }
	int DIdx(const A &d) const { return FoldBits(AtomHash(d),dbits); }

	A dir;
	pooldir *nxt;

	pooldir *parent;
	const I vbits,dbits,vsize,dsize;
	
	static unsigned int FoldBits(unsigned long h,int bits);
	static int Int2Bits(unsigned long n);

	struct valentry { int cnt; poolval *v; };
	struct direntry { int cnt; pooldir *d; };
	
	valentry *vals;
	direntry *dirs;

private:
  	BL LdDirXMLRec(istream &is,I depth,BL mkdir,AtomList &d);
};


class pooldata:
	public flext
{
public:
	pooldata(const S *s = NULL,I vcnt = 0,I dcnt = 0);
	~pooldata();

	V Push() { ++refs; }
	BL Pop() { return --refs > 0; }

    V Reset() { root.Reset(); }

    BL MkDir(const AtomList &d,I vcnt = 0,I dcnt = 0) 
    { 
        root.AddDir(d,vcnt,dcnt); 
        return true; 
    }

    BL ChkDir(const AtomList &d) 
    { 
        return root.GetDir(d) != NULL; 
    }

    BL RmDir(const AtomList &d) 
    { 
        return root.DelDir(d); 
    }

    BL Set(const AtomList &d,const A &key,AtomList *data,BL over = true)
    {
	    pooldir *pd = root.GetDir(d);
	    if(!pd) return false;
	    pd->SetVal(key,data,over);
	    return true;
    }

    BL Seti(const AtomList &d,I ix,AtomList *data)
    {
	    pooldir *pd = root.GetDir(d);
	    if(!pd) return false;
	    pd->SetVali(ix,data);
	    return true;
    }

	BL Clr(const AtomList &d,const A &key)
    {
	    pooldir *pd = root.GetDir(d);
	    if(!pd) return false;
	    pd->ClrVal(key);
	    return true;
    }

	BL Clri(const AtomList &d,I ix)
    {
	    pooldir *pd = root.GetDir(d);
	    if(!pd) return false;
	    pd->ClrVali(ix);
	    return true;
    }

	BL ClrAll(const AtomList &d,BL rec,BL dironly = false)
    {
	    pooldir *pd = root.GetDir(d);
	    if(!pd) return false;
	    pd->Clear(rec,dironly);
	    return true;
    }

	AtomList *Peek(const AtomList &d,const A &key)
    {
	    pooldir *pd = root.GetDir(d);
	    return pd?pd->PeekVal(key):NULL;
    }

	AtomList *Get(const AtomList &d,const A &key)
    {
	    pooldir *pd = root.GetDir(d);
	    return pd?pd->GetVal(key):NULL;
    }

	poolval *Ref(const AtomList &d,const A &key)
    {
	    pooldir *pd = root.GetDir(d);
	    return pd?pd->RefVal(key):NULL;
    }

	poolval *Refi(const AtomList &d,I ix)
    {
	    pooldir *pd = root.GetDir(d);
	    return pd?pd->RefVali(ix):NULL;
    }

	I CntAll(const AtomList &d)
    {
	    pooldir *pd = root.GetDir(d);
	    return pd?pd->CntAll():0;
    }

	I PrintAll(const AtomList &d);
	I GetAll(const AtomList &d,A *&keys,Atoms *&lst);

    I CntSub(const AtomList &d)
    {
	    pooldir *pd = root.GetDir(d);
	    return pd?pd->CntSub():0;
    }

	I GetSub(const AtomList &d,const t_atom **&dirs);

	BL Paste(const AtomList &d,const pooldir *clip,I depth = -1,BL repl = true,BL mkdir = true);
	pooldir *Copy(const AtomList &d,const A &key,BL cut);
	pooldir *CopyAll(const AtomList &d,I depth,BL cut);

	BL LdDir(const AtomList &d,const C *flnm,I depth,BL mkdir = true);
	BL SvDir(const AtomList &d,const C *flnm,I depth,BL absdir);
	BL Load(const C *flnm) { AtomList l; return LdDir(l,flnm,-1); }
	BL Save(const C *flnm) { AtomList l; return SvDir(l,flnm,-1,true); }
	BL LdDirXML(const AtomList &d,const C *flnm,I depth,BL mkdir = true);
	BL SvDirXML(const AtomList &d,const C *flnm,I depth,BL absdir);
	BL LoadXML(const C *flnm) { AtomList l; return LdDirXML(l,flnm,-1); }
	BL SaveXML(const C *flnm) { AtomList l; return SvDirXML(l,flnm,-1,true); }

	I refs;
	const S *sym;
	pooldata *nxt;

	pooldir root;

private:
	static const A nullatom;
};

#endif
