/* 

pool - hierarchical storage object for PD and Max/MSP

Copyright (c) 2002-2003 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#ifndef __POOL_H
#define __POOL_H

#define FLEXT_ATTRIBUTES 1

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error You need at least flext version 0.4.1
#endif

#include <iostream.h>

typedef void V;
typedef int I;
typedef float F;
typedef char C;
typedef bool BL;
typedef t_atom A;
typedef t_symbol S;

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
	pooldir(const A &dir,pooldir *parent);
	~pooldir();

	V Clear(BL rec,BL dironly = false);
	BL Empty() const { return !dirs && !vals; }
	BL HasDirs() const { return dirs != NULL; }
	BL HasVals() const { return vals != NULL; }

	pooldir *GetDir(I argc,const A *argv,BL cut = false);
	pooldir *GetDir(const AtomList &d,BL cut = false) { return GetDir(d.Count(),d.Atoms(),cut); }
	BL DelDir(I argc,const A *argv);
	BL DelDir(const AtomList &d) { return DelDir(d.Count(),d.Atoms()); }
	pooldir *AddDir(I argc,const A *argv);
	pooldir *AddDir(const AtomList &d) { return AddDir(d.Count(),d.Atoms()); }

	V SetVal(const A &key,AtomList *data,BL over = true);
	V ClrVal(const A &key) { SetVal(key,NULL); }
	AtomList *PeekVal(const A &key);
	AtomList *GetVal(const A &key,BL cut = false);
	I CntAll();
	I GetAll(A *&keys,AtomList *&lst,BL cut = false);
	I GetKeys(AtomList &keys);
	I GetSub(const A **&dirs);

	poolval *RefVal(const A &key);
	poolval *RefVali(I ix);
	
	BL Paste(const pooldir *p,I depth,BL repl,BL mkdir);
	BL Copy(pooldir *p,I depth,BL cur);

	BL LdDir(istream &is,I depth,BL mkdir);
	BL SvDir(ostream &os,I depth,const AtomList &dir = AtomList());

	A dir;
	pooldir *nxt;

	pooldir *parent,*dirs;
	poolval *vals;
};

class pooldata:
	public flext
{
public:
	pooldata(const S *s = NULL);
	~pooldata();

	V Push() { ++refs; }
	BL Pop() { return --refs > 0; }

	V Reset();
	BL MkDir(const AtomList &d); 
	BL ChkDir(const AtomList &d);
	BL RmDir(const AtomList &d);

	BL Set(const AtomList &d,const A &key,AtomList *data,BL over = true);
	BL Clr(const AtomList &d,const A &key);
	BL ClrAll(const AtomList &d,BL rec,BL dironly = false);
	AtomList *Peek(const AtomList &d,const A &key);
	AtomList *Get(const AtomList &d,const A &key);
	poolval *Ref(const AtomList &d,const A &key);
	poolval *Refi(const AtomList &d,I ix);
	I CntAll(const AtomList &d);
	I GetAll(const AtomList &d,A *&keys,AtomList *&lst);
	I GetSub(const AtomList &d,const t_atom **&dirs);

	BL Paste(const AtomList &d,const pooldir *clip,I depth = -1,BL repl = true,BL mkdir = true);
	pooldir *Copy(const AtomList &d,const A &key,BL cut);
	pooldir *CopyAll(const AtomList &d,I depth,BL cut);

	BL LdDir(const AtomList &d,const C *flnm,I depth,BL mkdir = true);
	BL SvDir(const AtomList &d,const C *flnm,I depth,BL absdir);
	BL Load(const C *flnm) { return LdDir(AtomList(),flnm,-1); }
	BL Save(const C *flnm) { return SvDir(AtomList(),flnm,-1,true); }

	I refs;
	const S *sym;
	pooldata *nxt;

	pooldir root;

private:
	static const A nullatom;
};

#endif
