/* 

pool - hierarchical storage object for PD and Max/MSP

Copyright (c) 2002-2003 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pool.h"
#include <string>

#define POOL_VERSION "0.2.0pre"

#define VCNT 64
#define DCNT 16

class pool:
	public flext_base
{
	FLEXT_HEADER_S(pool,flext_base,setup)

public:
	pool(I argc,const A *argv);
	~pool();

	static V setup(t_classid);
	
	virtual BL Init();

	pooldata *Pool() { return pl; }

protected:

	// switch to other pool
	V ms_pool(const AtomList &l);
	V mg_pool(AtomList &l);

	// clear all data in pool
	V m_reset();

	// handle directories
	V m_getdir();

	V m_mkdir(I argc,const A *argv,BL abs = true,BL chg = false); // make (and change) to dir
	V m_mkchdir(I argc,const A *argv) { m_mkdir(argc,argv,true,true); } // make and change to dir
	V m_chdir(I argc,const A *argv,BL abs = true);		// change to dir
	V m_rmdir(I argc,const A *argv,BL abs = true);		// remove dir
	V m_updir(I argc,const A *argv);		// one or more levels up

	V m_mksub(I argc,const A *argv) { m_mkdir(argc,argv,false); }
	V m_mkchsub(I argc,const A *argv) { m_mkdir(argc,argv,false,true); }
	V m_chsub(I argc,const A *argv) { m_chdir(argc,argv,false); }
	V m_rmsub(I argc,const A *argv) { m_rmdir(argc,argv,false); }

	// handle data
	V m_set(I argc,const A *argv) { set(MakeSymbol("set"),argc,argv,true); }
	V m_add(I argc,const A *argv) { set(MakeSymbol("add"),argc,argv,false); }
	V m_clr(I argc,const A *argv);
	V m_clrall();	// only values
	V m_clrrec();	// also subdirectories
	V m_clrsub();	// only subdirectories
	V m_get(I argc,const A *argv);
	V m_geti(I ix); // get value at index
	V m_getall();	// only values
	V m_getrec(I argc,const A *argv);	// also subdirectories
	V m_getsub(I argc,const A *argv);	// only subdirectories
	V m_ogetall();	// only values (ordered)
	V m_ogetrec(I argc,const A *argv);	// also subdirectories (ordered)
	V m_ogetsub(I argc,const A *argv);	// only subdirectories (ordered)
	V m_cntall();	// only values
	V m_cntrec(I argc,const A *argv);	// also subdirectories
	V m_cntsub(I argc,const A *argv);	// only subdirectories

	// print directories
	V m_printall();   // print values in current dir
	V m_printrec(I argc,const A *argv,BL fromroot = false);   // print values recursively
    V m_printroot() { m_printrec(0,NULL,true); }   // print values recursively from root

	// cut/copy/paste
	V m_paste(I argc,const A *argv) { paste(MakeSymbol("paste"),argc,argv,true); } // paste contents of clipboard
	V m_pasteadd(I argc,const A *argv) { paste(MakeSymbol("pasteadd"),argc,argv,false); } // paste but don't replace
	V m_clrclip();  // clear clipboard
	V m_cut(I argc,const A *argv) { copy(MakeSymbol("cut"),argc,argv,true); } // cut value into clipboard
	V m_copy(I argc,const A *argv) { copy(MakeSymbol("copy"),argc,argv,false); } 	// copy value into clipboard
	V m_cutall() { copyall(MakeSymbol("cutall"),true,0); }   // cut all values in current directory into clipboard
	V m_copyall() { copyall(MakeSymbol("copyall"),false,0); }   // copy all values in current directory into clipboard
	V m_cutrec(I argc,const A *argv) { copyrec(MakeSymbol("cutrec"),argc,argv,true); }   // cut directory (and subdirs) into clipboard
	V m_copyrec(I argc,const A *argv) { copyrec(MakeSymbol("copyrec"),argc,argv,false); }   // cut directory (and subdirs) into clipboard

	// load/save from/to file
    V m_load(I argc,const A *argv) { load(argc,argv,false); }
	V m_save(I argc,const A *argv) { save(argc,argv,false); }
	V m_loadx(I argc,const A *argv) { load(argc,argv,true); } // XML
	V m_savex(I argc,const A *argv) { save(argc,argv,true); } // XML

	// load directories
	V m_lddir(I argc,const A *argv) { lddir(argc,argv,false); }   // load values into current dir
	V m_ldrec(I argc,const A *argv) { ldrec(argc,argv,false); }   // load values recursively
	V m_ldxdir(I argc,const A *argv) { lddir(argc,argv,true); }   // load values into current dir (XML)
	V m_ldxrec(I argc,const A *argv) { ldrec(argc,argv,true); }   // load values recursively (XML)

	// save directories
	V m_svdir(I argc,const A *argv) { svdir(argc,argv,false); }   // save values in current dir
	V m_svrec(I argc,const A *argv) { svrec(argc,argv,false); }   // save values recursively
	V m_svxdir(I argc,const A *argv) { svdir(argc,argv,true); }   // save values in current dir (XML)
	V m_svxrec(I argc,const A *argv) { svrec(argc,argv,true); }   // save values recursively (XML)

private:
	static BL KeyChk(const A &a);
	static BL ValChk(I argc,const A *argv);
	static BL ValChk(const AtomList &l) { return ValChk(l.Count(),l.Atoms()); }
	V ToOutAtom(I ix,const A &a);

    enum get_t { get_norm,get_cnt,get_print };

	V set(const S *tag,I argc,const A *argv,BL over);
	V getdir(const S *tag);
	I getrec(const S *tag,I level,BL order,get_t how = get_norm,const AtomList &rdir = AtomList());
	I getsub(const S *tag,I level,BL order,get_t how = get_norm,const AtomList &rdir = AtomList());

	V paste(const S *tag,I argc,const A *argv,BL repl);
	V copy(const S *tag,I argc,const A *argv,BL cut);
	V copyall(const S *tag,BL cut,I lvls);
	V copyrec(const S *tag,I argc,const A *argv,BL cut);

	V load(I argc,const A *argv,BL xml);
	V save(I argc,const A *argv,BL xml);
	V lddir(I argc,const A *argv,BL xml);   // load values into current dir
	V ldrec(I argc,const A *argv,BL xml);   // load values recursively
	V svdir(I argc,const A *argv,BL xml);   // save values in current dir
	V svrec(I argc,const A *argv,BL xml);   // save values recursively

	V echodir() { if(echo) getdir(MakeSymbol("echo")); }

	BL priv,absdir,echo;
	I vcnt,dcnt;
	pooldata *pl;
	AtomList curdir;
	pooldir *clip;
	const S *holdname;

	static pooldata *head,*tail;

	V SetPool(const S *s);
	V FreePool();

	static pooldata *GetPool(const S *s);
	static V RmvPool(pooldata *p);

	std::string MakeFilename(const C *fn) const;

	FLEXT_CALLVAR_V(mg_pool,ms_pool)
	FLEXT_ATTRVAR_B(absdir)
	FLEXT_ATTRVAR_B(echo)
	FLEXT_ATTRGET_B(priv)
//	FLEXT_ATTRGET_B(curdir)
	FLEXT_ATTRVAR_I(vcnt)
	FLEXT_ATTRVAR_I(dcnt)

	FLEXT_CALLBACK(m_reset)

	FLEXT_CALLBACK(m_getdir)
	FLEXT_CALLBACK_V(m_mkdir)
	FLEXT_CALLBACK_V(m_chdir)
	FLEXT_CALLBACK_V(m_mkchdir)
	FLEXT_CALLBACK_V(m_updir)
	FLEXT_CALLBACK_V(m_rmdir)
	FLEXT_CALLBACK_V(m_mksub)
	FLEXT_CALLBACK_V(m_chsub)
	FLEXT_CALLBACK_V(m_mkchsub)
	FLEXT_CALLBACK_V(m_rmsub)

	FLEXT_CALLBACK_V(m_set)
	FLEXT_CALLBACK_V(m_add)
	FLEXT_CALLBACK_V(m_clr)
	FLEXT_CALLBACK(m_clrall)
	FLEXT_CALLBACK(m_clrrec)
	FLEXT_CALLBACK(m_clrsub)
	FLEXT_CALLBACK_V(m_get)
	FLEXT_CALLBACK_I(m_geti)
	FLEXT_CALLBACK(m_getall)
	FLEXT_CALLBACK_V(m_getrec)
	FLEXT_CALLBACK_V(m_getsub)
	FLEXT_CALLBACK(m_ogetall)
	FLEXT_CALLBACK_V(m_ogetrec)
	FLEXT_CALLBACK_V(m_ogetsub)
	FLEXT_CALLBACK(m_cntall)
	FLEXT_CALLBACK_V(m_cntrec)
	FLEXT_CALLBACK_V(m_cntsub)
	FLEXT_CALLBACK(m_printall)
	FLEXT_CALLBACK_V(m_printrec)
	FLEXT_CALLBACK(m_printroot)

	FLEXT_CALLBACK_V(m_paste)
	FLEXT_CALLBACK_V(m_pasteadd)
	FLEXT_CALLBACK(m_clrclip)
	FLEXT_CALLBACK_V(m_copy)
	FLEXT_CALLBACK_V(m_cut)
	FLEXT_CALLBACK(m_copyall)
	FLEXT_CALLBACK(m_cutall)
	FLEXT_CALLBACK_V(m_copyrec)
	FLEXT_CALLBACK_V(m_cutrec)

	FLEXT_CALLBACK_V(m_load)
	FLEXT_CALLBACK_V(m_save)
	FLEXT_CALLBACK_V(m_lddir)
	FLEXT_CALLBACK_V(m_ldrec)
	FLEXT_CALLBACK_V(m_svdir)
	FLEXT_CALLBACK_V(m_svrec)
	FLEXT_CALLBACK_V(m_loadx)
	FLEXT_CALLBACK_V(m_savex)
	FLEXT_CALLBACK_V(m_ldxdir)
	FLEXT_CALLBACK_V(m_ldxrec)
	FLEXT_CALLBACK_V(m_svxdir)
	FLEXT_CALLBACK_V(m_svxrec)
};

FLEXT_NEW_V("pool",pool);


pooldata *pool::head,*pool::tail;	


V pool::setup(t_classid c)
{
	post("");
	post("pool %s - hierarchical storage object, (C)2002-2003 Thomas Grill",POOL_VERSION);
	post("");

	head = tail = NULL;

	FLEXT_CADDATTR_VAR(c,"pool",mg_pool,ms_pool);
	FLEXT_CADDATTR_VAR1(c,"absdir",absdir);
	FLEXT_CADDATTR_VAR1(c,"echodir",echo);
	FLEXT_CADDATTR_GET(c,"private",priv);
	FLEXT_CADDATTR_VAR1(c,"valcnt",vcnt);
	FLEXT_CADDATTR_VAR1(c,"dircnt",dcnt);

	FLEXT_CADDMETHOD_(c,0,"reset",m_reset);
	FLEXT_CADDMETHOD_(c,0,"set",m_set);
	FLEXT_CADDMETHOD_(c,0,"add",m_add);
	FLEXT_CADDMETHOD_(c,0,"getdir",m_getdir);
	FLEXT_CADDMETHOD_(c,0,"mkdir",m_mkdir);
	FLEXT_CADDMETHOD_(c,0,"chdir",m_chdir);
	FLEXT_CADDMETHOD_(c,0,"mkchdir",m_mkchdir);
	FLEXT_CADDMETHOD_(c,0,"rmdir",m_rmdir);
	FLEXT_CADDMETHOD_(c,0,"updir",m_updir);
	FLEXT_CADDMETHOD_(c,0,"mksub",m_mksub);
	FLEXT_CADDMETHOD_(c,0,"chsub",m_chsub);
	FLEXT_CADDMETHOD_(c,0,"mkchsub",m_mkchsub);
	FLEXT_CADDMETHOD_(c,0,"rmsub",m_rmsub);

	FLEXT_CADDMETHOD_(c,0,"set",m_set);
	FLEXT_CADDMETHOD_(c,0,"clr",m_clr);
	FLEXT_CADDMETHOD_(c,0,"clrall",m_clrall);
	FLEXT_CADDMETHOD_(c,0,"clrrec",m_clrrec);
	FLEXT_CADDMETHOD_(c,0,"clrsub",m_clrsub);
	FLEXT_CADDMETHOD_(c,0,"get",m_get);
	FLEXT_CADDMETHOD_(c,0,"geti",m_geti);
	FLEXT_CADDMETHOD_(c,0,"getall",m_getall);
	FLEXT_CADDMETHOD_(c,0,"getrec",m_getrec);
	FLEXT_CADDMETHOD_(c,0,"getsub",m_getsub);
	FLEXT_CADDMETHOD_(c,0,"ogetall",m_ogetall);
	FLEXT_CADDMETHOD_(c,0,"ogetrec",m_ogetrec);
	FLEXT_CADDMETHOD_(c,0,"ogetsub",m_ogetsub);
	FLEXT_CADDMETHOD_(c,0,"cntall",m_cntall);
	FLEXT_CADDMETHOD_(c,0,"cntrec",m_cntrec);
	FLEXT_CADDMETHOD_(c,0,"cntsub",m_cntsub);

	FLEXT_CADDMETHOD_(c,0,"printall",m_printall);
	FLEXT_CADDMETHOD_(c,0,"printrec",m_printrec);
	FLEXT_CADDMETHOD_(c,0,"printroot",m_printroot);

	FLEXT_CADDMETHOD_(c,0,"paste",m_paste);
	FLEXT_CADDMETHOD_(c,0,"pasteadd",m_pasteadd);
	FLEXT_CADDMETHOD_(c,0,"clrclip",m_clrclip);
	FLEXT_CADDMETHOD_(c,0,"cut",m_cut);
	FLEXT_CADDMETHOD_(c,0,"copy",m_copy);
	FLEXT_CADDMETHOD_(c,0,"cutall",m_cutall);
	FLEXT_CADDMETHOD_(c,0,"copyall",m_copyall);
	FLEXT_CADDMETHOD_(c,0,"cutrec",m_cutrec);
	FLEXT_CADDMETHOD_(c,0,"copyrec",m_copyrec);

	FLEXT_CADDMETHOD_(c,0,"load",m_load);
	FLEXT_CADDMETHOD_(c,0,"save",m_save);
	FLEXT_CADDMETHOD_(c,0,"lddir",m_lddir);
	FLEXT_CADDMETHOD_(c,0,"ldrec",m_ldrec);
	FLEXT_CADDMETHOD_(c,0,"svdir",m_svdir);
	FLEXT_CADDMETHOD_(c,0,"svrec",m_svrec);
	FLEXT_CADDMETHOD_(c,0,"loadx",m_loadx);
	FLEXT_CADDMETHOD_(c,0,"savex",m_savex);
	FLEXT_CADDMETHOD_(c,0,"ldxdir",m_ldxdir);
	FLEXT_CADDMETHOD_(c,0,"ldxrec",m_ldxrec);
	FLEXT_CADDMETHOD_(c,0,"svxdir",m_svxdir);
	FLEXT_CADDMETHOD_(c,0,"svxrec",m_svxrec);
}

pool::pool(I argc,const A *argv):
	absdir(true),echo(false),pl(NULL),
	clip(NULL),
	vcnt(VCNT),dcnt(DCNT)
{
	holdname = argc >= 1 && IsSymbol(argv[0])?GetSymbol(argv[0]):NULL;

	AddInAnything("Commands in");
	AddOutList();
	AddOutAnything();
	AddOutList();
	AddOutAnything();
}

pool::~pool()
{
	FreePool();
}

BL pool::Init()
{
	if(flext_base::Init()) {
		SetPool(holdname);	
		return true;
	}
	else return false;
}

V pool::SetPool(const S *s)
{
	if(pl) FreePool();

	if(s) {
		priv = false;
		pl = GetPool(s);
	}
	else {
		priv = true;
		pl = new pooldata(NULL,vcnt,dcnt);
	}
}

V pool::FreePool()
{
	curdir(); // reset current directory

	if(pl) {
		if(!priv) 
			RmvPool(pl);
		else
			delete pl;
		pl = NULL;
	}

	if(clip) { delete clip; clip = NULL; }
}

V pool::ms_pool(const AtomList &l) 
{
	const S *s = NULL;
	if(l.Count()) {
		if(l.Count() > 1) post("%s - pool: superfluous arguments ignored",thisName());
		s = GetASymbol(l[0]);
		if(!s) post("%s - pool: invalid pool name, pool set to private",thisName());
	}

	SetPool(s);
}

V pool::mg_pool(AtomList &l)
{
	if(priv) l();
	else { l(1); SetSymbol(l[0],pl->sym); }
}

V pool::m_reset() 
{
	pl->Reset();
}


V pool::getdir(const S *tag)
{
	ToOutAnything(3,tag,0,NULL);
	ToOutList(2,curdir);
}

V pool::m_getdir() { getdir(MakeSymbol("getdir")); }

V pool::m_mkdir(I argc,const A *argv,BL abs,BL chg)
{
    const char *nm = chg?"mkchdir":"mkdir";
	if(!ValChk(argc,argv))
		post("%s - %s: invalid directory name",thisName(),nm);
	else {
		AtomList ndir;
		if(abs) ndir(argc,argv);
		else (ndir = curdir).Append(argc,argv);
		if(!pl->MkDir(ndir,vcnt,dcnt)) {
			post("%s - %s: directory couldn't be created",thisName(),nm);
		}
        else if(chg) 
            // change to newly created directory
            curdir = ndir;
	}

	echodir();
}

V pool::m_chdir(I argc,const A *argv,BL abs)
{
	if(!ValChk(argc,argv)) 
		post("%s - chdir: invalid directory name",thisName());
	else {
		AtomList prv(curdir);
		if(abs) curdir(argc,argv);
		else curdir.Append(argc,argv);
		if(!pl->ChkDir(curdir)) {
			post("%s - chdir: directory couldn't be changed",thisName());
			curdir = prv;
		}
	}

	echodir();
}

V pool::m_updir(I argc,const A *argv)
{
	I lvls = 1;
	if(argc > 0) {
		if(CanbeInt(argv[0])) {
			if(argc > 1)
				post("%s - updir: superfluous arguments ignored",thisName());
			lvls = GetAInt(argv[0]);
			if(lvls < 0)
				post("%s - updir: invalid level specification - set to 1",thisName());
		}
		else
			post("%s - updir: invalid level specification - set to 1",thisName());
	}

	AtomList prv(curdir);

	if(lvls > curdir.Count()) {
		post("%s - updir: level exceeds directory depth - corrected",thisName());
		curdir();
	}
	else
		curdir.Part(0,curdir.Count()-lvls);

	if(!pl->ChkDir(curdir)) {
		post("%s - updir: directory couldn't be changed",thisName());
		curdir = prv;
	}

	echodir();
}

V pool::m_rmdir(I argc,const A *argv,BL abs)
{
	if(abs) curdir(argc,argv);
	else curdir.Append(argc,argv);

	if(!pl->RmDir(curdir)) 
		post("%s - rmdir: directory couldn't be removed",thisName());
	curdir();

	echodir();
}

V pool::set(const S *tag,I argc,const A *argv,BL over)
{
	if(!argc || !KeyChk(argv[0])) 
		post("%s - %s: invalid key",thisName(),GetString(tag));
	else if(!ValChk(argc-1,argv+1)) {
		post("%s - %s: invalid data values",thisName(),GetString(tag));
	}
	else 
		if(!pl->Set(curdir,argv[0],new AtomList(argc-1,argv+1),over))
			post("%s - %s: value couldn't be set",thisName(),GetString(tag));

	echodir();
}

V pool::m_clr(I argc,const A *argv)
{
	if(!argc || !KeyChk(argv[0]))
		post("%s - clr: invalid key",thisName());
	else {
		if(argc > 1) 
			post("%s - clr: superfluous arguments ignored",thisName());

		if(!pl->Clr(curdir,argv[0]))
			post("%s - clr: value couldn't be cleared",thisName());
	}

	echodir();
}

V pool::m_clrall()
{
	if(!pl->ClrAll(curdir,false))
		post("%s - clrall: values couldn't be cleared",thisName());

	echodir();
}

V pool::m_clrrec()
{
	if(!pl->ClrAll(curdir,true))
		post("%s - clrrec: values couldn't be cleared",thisName());

	echodir();
}

V pool::m_clrsub()
{
	if(!pl->ClrAll(curdir,true,true))
		post("%s - clrsub: directories couldn't be cleared",thisName());

	echodir();
}

V pool::m_get(I argc,const A *argv)
{
	if(!argc || !KeyChk(argv[0]))
		post("%s - get: invalid key",thisName());
	else {
		if(argc > 1) 
			post("%s - get: superfluous arguments ignored",thisName());

		poolval *r = pl->Ref(curdir,argv[0]);

		ToOutAnything(3,MakeSymbol("get"),0,NULL);
		if(absdir)
			ToOutList(2,curdir);
		else
			ToOutList(2,0,NULL);
		if(r) {
			ToOutAtom(1,r->key);
			ToOutList(0,*r->data);
		}
		else {
			ToOutBang(1);
			ToOutBang(0);
		}
	}

	echodir();
}

V pool::m_geti(I ix)
{
	if(ix < 0)
		post("%s - geti: invalid index",thisName());
	else {
		poolval *r = pl->Refi(curdir,ix);

		ToOutAnything(3,MakeSymbol("geti"),0,NULL);
		if(absdir)
			ToOutList(2,curdir);
		else
			ToOutList(2,0,NULL);
		if(r) {
			ToOutAtom(1,r->key);
			ToOutList(0,*r->data);
		}
		else {
			ToOutBang(1);
			ToOutBang(0);
		}
	}

	echodir();
}

I pool::getrec(const S *tag,I level,BL order,get_t how,const AtomList &rdir)
{
	AtomList gldir(curdir);
	gldir.Append(rdir);

	I ret = 0;

    switch(how) {
        case get_cnt: 
            ret = pl->CntAll(gldir);
            break;
        case get_print:
            ret = pl->PrintAll(gldir);
            break;
        case get_norm: {
		    A *k;
		    AtomList *r;
		    I cnt = pl->GetAll(gldir,k,r);
		    if(!k) 
			    post("%s - %s: error retrieving values",thisName(),GetString(tag));
		    else {
			    for(I i = 0; i < cnt; ++i) {
				    ToOutAnything(3,tag,0,NULL);
				    ToOutList(2,absdir?gldir:rdir);
				    ToOutAtom(1,k[i]);
				    ToOutList(0,r[i]);
			    }
			    delete[] k;
			    delete[] r;
		    }
		    ret = cnt;
        }
	}

	if(level != 0) {
		const A **r;
		I cnt = pl->GetSub(gldir,r);
		if(!r) 
			post("%s - %s: error retrieving directories",thisName(),GetString(tag));
		else {
			I lv = level > 0?level-1:-1;
			for(I i = 0; i < cnt; ++i) {
				ret += getrec(tag,lv,order,how,AtomList(rdir).Append(*r[i]));
			}
			delete[] r;
		}
	}
	
	return ret;
}

V pool::m_getall()
{
	getrec(MakeSymbol("getall"),0,false);
	ToOutBang(3);

	echodir();
}

V pool::m_ogetall()
{
	getrec(MakeSymbol("ogetall"),0,true);
	ToOutBang(3);

	echodir();
}

V pool::m_getrec(I argc,const A *argv)
{
	I lvls = -1;
	if(argc > 0) {
		if(CanbeInt(argv[0])) {
			if(argc > 1)
				post("%s - getrec: superfluous arguments ignored",thisName());
			lvls = GetAInt(argv[0]);
		}
		else 
			post("%s - getrec: invalid level specification - set to infinite",thisName());
	}
	getrec(MakeSymbol("getrec"),lvls,false);
	ToOutBang(3);

	echodir();
}


V pool::m_ogetrec(I argc,const A *argv)
{
	I lvls = -1;
	if(argc > 0) {
		if(CanbeInt(argv[0])) {
			if(argc > 1)
				post("%s - ogetrec: superfluous arguments ignored",thisName());
			lvls = GetAInt(argv[0]);
		}
		else 
			post("%s - ogetrec: invalid level specification - set to infinite",thisName());
	}
	getrec(MakeSymbol("ogetrec"),lvls,true);
	ToOutBang(3);

	echodir();
}


I pool::getsub(const S *tag,I level,BL order,get_t how,const AtomList &rdir)
{
	AtomList gldir(curdir);
	gldir.Append(rdir);
	
	I ret = 0;

	const A **r = NULL;
	// CntSub is not used here because it doesn't allow checking for valid directory
	I cnt = pl->GetSub(gldir,r);
	if(!r) 
		post("%s - %s: error retrieving directories",thisName(),GetString(tag));
	else {
		I lv = level > 0?level-1:-1;
		for(I i = 0; i < cnt; ++i) {
			AtomList ndir(absdir?gldir:rdir);
			ndir.Append(*r[i]);

			if(how == get_norm) {
				ToOutAnything(3,tag,0,NULL);
				ToOutList(2,curdir);
				ToOutList(1,ndir);
				ToOutBang(0);
			}

			if(level != 0)
				ret += getsub(tag,lv,order,how,AtomList(rdir).Append(*r[i]));
		}
		delete[] r;
	}
	
	return ret;
}

V pool::m_getsub(I argc,const A *argv)
{
	I lvls = 0;
	if(argc > 0) {
		if(CanbeInt(argv[0])) {
			if(argc > 1)
				post("%s - getsub: superfluous arguments ignored",thisName());
			lvls = GetAInt(argv[0]);
		}
		else 
			post("%s - getsub: invalid level specification - set to 0",thisName());
	}

	getsub(MakeSymbol("getsub"),lvls,false);
	ToOutBang(3);

	echodir();
}


V pool::m_ogetsub(I argc,const A *argv)
{
	I lvls = 0;
	if(argc > 0) {
		if(CanbeInt(argv[0])) {
			if(argc > 1)
				post("%s - ogetsub: superfluous arguments ignored",thisName());
			lvls = GetAInt(argv[0]);
		}
		else 
			post("%s - ogetsub: invalid level specification - set to 0",thisName());
	}

	getsub(MakeSymbol("ogetsub"),lvls,true); 
	ToOutBang(3);

	echodir();
}


V pool::m_cntall()
{
	const S *tag = MakeSymbol("cntall");
	I cnt = getrec(tag,0,false,get_cnt);
	ToOutSymbol(3,tag);
	ToOutBang(2);
	ToOutBang(1);
	ToOutInt(0,cnt);

	echodir();
}

V pool::m_cntrec(I argc,const A *argv)
{
	const S *tag = MakeSymbol("cntrec");

	I lvls = -1;
	if(argc > 0) {
		if(CanbeInt(argv[0])) {
			if(argc > 1)
				post("%s - %s: superfluous arguments ignored",thisName(),GetString(tag));
			lvls = GetAInt(argv[0]);
		}
		else 
			post("%s - %s: invalid level specification - set to infinite",thisName(),GetString(tag));
	}
	
	I cnt = getrec(tag,lvls,false,get_cnt);
	ToOutSymbol(3,tag);
	ToOutBang(2);
	ToOutBang(1);
	ToOutInt(0,cnt);

	echodir();
}


V pool::m_cntsub(I argc,const A *argv)
{
	const S *tag = MakeSymbol("cntsub");

	I lvls = 0;
	if(argc > 0) {
		if(CanbeInt(argv[0])) {
			if(argc > 1)
				post("%s - %s: superfluous arguments ignored",thisName(),GetString(tag));
			lvls = GetAInt(argv[0]);
		}
		else 
			post("%s - %s: invalid level specification - set to 0",thisName(),GetString(tag));
	}

	I cnt = getsub(tag,lvls,false,get_cnt);
	ToOutSymbol(3,tag);
	ToOutBang(2);
	ToOutBang(1);
	ToOutInt(0,cnt);

	echodir();
}

V pool::m_printall()
{
    const S *tag = MakeSymbol("printall");
	I cnt = getrec(tag,0,false,get_print);
    post("");
}

V pool::m_printrec(I argc,const A *argv,BL fromroot)
{
	const S *tag = MakeSymbol(fromroot?"printroot":"printrec");

	I lvls = -1;
	if(argc > 0) {
		if(CanbeInt(argv[0])) {
			if(argc > 1)
				post("%s - %s: superfluous arguments ignored",thisName(),GetString(tag));
			lvls = GetAInt(argv[0]);
		}
		else 
			post("%s - %s: invalid level specification - set to infinite",thisName(),GetString(tag));
	}

	AtomList svdir(curdir);
    if(fromroot) curdir.Clear();

	I cnt = getrec(tag,lvls,false,get_print);
    post("");

    curdir = svdir;
}


V pool::paste(const S *tag,I argc,const A *argv,BL repl)
{
	if(clip) {
		BL mkdir = true;
		I depth = -1;

		if(argc >= 1) {
			if(CanbeInt(argv[0])) depth = GetAInt(argv[1]);
			else
				post("%s - %s: invalid depth argument - set to -1",thisName(),GetString(tag));

			if(argc >= 2) {
				if(CanbeBool(argv[1])) mkdir = GetABool(argv[1]);
				else
					post("%s - %s: invalid mkdir argument - set to true",thisName(),GetString(tag));

				if(argc > 2) post("%s - %s: superfluous arguments ignored",thisName(),GetString(tag));
			}
		}
		
		pl->Paste(curdir,clip,depth,repl,mkdir);
	}
	else
		post("%s - %s: clipboard is empty",thisName(),GetString(tag));

	echodir();
}


V pool::m_clrclip()
{
	if(clip) { delete clip; clip = NULL; }
}


V pool::copy(const S *tag,I argc,const A *argv,BL cut)
{
	if(!argc || !KeyChk(argv[0]))
		post("%s - %s: invalid key",thisName(),GetString(tag));
	else {
		if(argc > 1) 
			post("%s - %s: superfluous arguments ignored",thisName(),GetString(tag));

		m_clrclip();
		clip = pl->Copy(curdir,argv[0],cut);

		if(!clip)
			post("%s - %s: Copying into clipboard failed",thisName(),GetString(tag));
	}

	echodir();
}


V pool::copyall(const S *tag,BL cut,I depth)
{
	m_clrclip();
	clip = pl->CopyAll(curdir,depth,cut);

	if(!clip)
		post("%s - %s: Copying into clipboard failed",thisName(),GetString(tag));

	echodir();
}


V pool::copyrec(const S *tag,I argc,const A *argv,BL cut) 
{
	I lvls = -1;
	if(argc > 0) {
		if(CanbeInt(argv[0])) {
			if(argc > 1)
				post("%s - %s: superfluous arguments ignored",thisName(),GetString(tag));
			lvls = GetAInt(argv[0]);
		}
		else 
			post("%s - %s: invalid level specification - set to infinite",thisName(),GetString(tag));
	}

	copyall(tag,cut,lvls);
}

V pool::load(I argc,const A *argv,BL xml)
{
    const C *sym = xml?"loadx":"load";
	const C *flnm = NULL;
	if(argc > 0) {
		if(argc > 1) post("%s - %s: superfluous arguments ignored",thisName(),sym);
		if(IsString(argv[0])) flnm = GetString(argv[0]);
	}

	if(!flnm) 
		post("%s - %s: no filename given",thisName(),sym);
	else {
		string fl(MakeFilename(flnm));
		if(!(xml?pl->LoadXML(fl.c_str()):pl->Load(fl.c_str())))
			post("%s - %s: error loading data",thisName(),sym);
	}

	echodir();
}

V pool::save(I argc,const A *argv,BL xml)
{
    const C *sym = xml?"savex":"save";
	const C *flnm = NULL;
	if(argc > 0) {
		if(argc > 1) post("%s - %s: superfluous arguments ignored",thisName(),sym);
		if(IsString(argv[0])) flnm = GetString(argv[0]);
	}

	if(!flnm) 
		post("%s - %s: no filename given",thisName(),sym);
	else {
		string fl(MakeFilename(flnm));
		if(!(xml?pl->SaveXML(fl.c_str()):pl->Save(fl.c_str())))
			post("%s - %s: error saving data",thisName(),sym);
	}

	echodir();
}

V pool::lddir(I argc,const A *argv,BL xml)
{
    const C *sym = xml?"ldxdir":"lddir";
	const C *flnm = NULL;
	if(argc > 0) {
		if(argc > 1) post("%s - %s: superfluous arguments ignored",thisName(),sym);
		if(IsString(argv[0])) flnm = GetString(argv[0]);
	}

	if(!flnm)
		post("%s - %s: invalid filename",thisName(),sym);
	else {
		string fl(MakeFilename(flnm));
		if(!(xml?pl->LdDirXML(curdir,fl.c_str(),0):pl->LdDir(curdir,fl.c_str(),0))) 
			post("%s - %s: directory couldn't be loaded",thisName(),sym);
	}

	echodir();
}

V pool::ldrec(I argc,const A *argv,BL xml)
{
    const C *sym = xml?"ldxrec":"ldrec";
	const C *flnm = NULL;
	I depth = -1;
	BL mkdir = true;
	if(argc >= 1) {
		if(IsString(argv[0])) flnm = GetString(argv[0]);

		if(argc >= 2) {
			if(CanbeInt(argv[1])) depth = GetAInt(argv[1]);
			else
				post("%s - %s: invalid depth argument - set to -1",thisName(),sym);

			if(argc >= 3) {
				if(CanbeBool(argv[2])) mkdir = GetABool(argv[2]);
				else
					post("%s - %s: invalid mkdir argument - set to true",thisName(),sym);

				if(argc > 3) post("%s - %s: superfluous arguments ignored",thisName(),sym);
			}
		}
	}

	if(!flnm)
		post("%s - %s: invalid filename",thisName(),sym);
	else {
		string fl(MakeFilename(flnm));
        if(!(xml?pl->LdDirXML(curdir,fl.c_str(),depth,mkdir):pl->LdDir(curdir,fl.c_str(),depth,mkdir))) 
		post("%s - %s: directory couldn't be saved",thisName(),sym);
	}

	echodir();
}

V pool::svdir(I argc,const A *argv,BL xml)
{
    const C *sym = xml?"svxdir":"svdir";
	const C *flnm = NULL;
	if(argc > 0) {
		if(argc > 1) post("%s - %s: superfluous arguments ignored",thisName(),sym);
		if(IsString(argv[0])) flnm = GetString(argv[0]);
	}

	if(!flnm)
		post("%s - %s: invalid filename",thisName(),sym);
	else {
		string fl(MakeFilename(flnm));
        if(!(xml?pl->SvDirXML(curdir,fl.c_str(),0,absdir):pl->SvDir(curdir,fl.c_str(),0,absdir))) 
		post("%s - %s: directory couldn't be saved",thisName(),sym);
	}

	echodir();
}

V pool::svrec(I argc,const A *argv,BL xml)
{
    const C *sym = xml?"svxrec":"svrec";
	const C *flnm = NULL;
	if(argc > 0) {
		if(argc > 1) post("%s - %s: superfluous arguments ignored",thisName(),sym);
		if(IsString(argv[0])) flnm = GetString(argv[0]);
	}

	if(!flnm)
		post("%s - %s: invalid filename",thisName(),sym);
	else {
		string fl(MakeFilename(flnm));
        if(!(xml?pl->SvDirXML(curdir,fl.c_str(),-1,absdir):pl->SvDir(curdir,fl.c_str(),-1,absdir))) 
		post("%s - %s: directory couldn't be saved",thisName(),sym);
	}

	echodir();
}



BL pool::KeyChk(const t_atom &a)
{
	return IsSymbol(a) || IsFloat(a) || IsInt(a);
}

BL pool::ValChk(I argc,const t_atom *argv)
{
	for(I i = 0; i < argc; ++i) {
		const t_atom &a = argv[i];
		if(!IsSymbol(a) && !IsFloat(a) && !IsInt(a)) return false;
	}
	return true;
}

V pool::ToOutAtom(I ix,const t_atom &a)
{
	if(IsSymbol(a))
		ToOutSymbol(ix,GetSymbol(a));
	else if(IsFloat(a))
		ToOutFloat(ix,GetFloat(a));
	else if(IsInt(a))
		ToOutInt(ix,GetInt(a));
	else
		post("%s - output atom: type not supported!",thisName());
}



pooldata *pool::GetPool(const S *s)
{
	pooldata *pi = head;
	for(; pi && pi->sym != s; pi = pi->nxt) (V)0;

	if(pi) {
		pi->Push();
		return pi;
	}
	else {
		pooldata *p = new pooldata(s);
		p->Push();

		// now add to chain
		if(head) head->nxt = p;
		else head = p;
		tail = p;
		return p;
	}
}

V pool::RmvPool(pooldata *p)
{
	pooldata *prv = NULL,*pi = head;
	for(; pi && pi != p; prv = pi,pi = pi->nxt) (V)0;

	if(pi && !pi->Pop()) {
		if(prv) prv->nxt = pi->nxt;
		else head = pi->nxt;
		if(!pi->nxt) tail = pi;

		delete pi;
	}
}

std::string pool::MakeFilename(const C *fn) const
{
#if FLEXT_SYS == FLEXT_SYS_PD
    // / and \ must not be mixed!
    // (char *) type casts for BorlandC++
	C *sl = strchr((C *)fn,'/');
	if(!sl) sl = strchr((C *)fn,'\\');
    if(!sl || (sl != fn 
#if FLEXT_OS == FLEXT_OS_WIN
        && sl[-1] != ':' // look for drive specification with ":/" or ":\\"
#endif
    )) {
        // prepend absolute canvas path if filename has no absolute path
		const C *p = GetString(canvas_getdir(thisCanvas()));
		return string(p)+'/'+fn;
	}
	else
		return fn;
#else
#pragma message("Relative file paths not implemented")
	return fn;
#endif
}
