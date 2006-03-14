/* 

pool - hierarchical storage object for PD and Max/MSP

Copyright (c) 2002-2005 Thomas Grill
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include "pool.h"

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <fstream>

using namespace std;


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
			FLEXT_LOG("pool - atom comparison: type not handled");
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

    FLEXT_ASSERT(nxt == NULL);
}

poolval &poolval::Set(AtomList *d)
{
	if(data) delete data;
	data = d;
	return *this;
}

poolval *poolval::Dup() const
{
	return new poolval(key,data?new Atoms(*data):NULL); 
}


pooldir::pooldir(const A &d,pooldir *p,I vcnt,I dcnt):
	parent(p),nxt(NULL),vals(NULL),dirs(NULL),
	vbits(Int2Bits(vcnt)),dbits(Int2Bits(dcnt)),
	vsize(1<<vbits),dsize(1<<dbits)
{
	Reset();
	CopyAtom(&dir,&d);
}

pooldir::~pooldir()
{
	Reset(false);
		
    FLEXT_ASSERT(nxt == NULL);
}

V pooldir::Clear(BL rec,BL dironly)
{
	if(rec && dirs) { 
        for(I i = 0; i < dsize; ++i) {
            pooldir *d = dirs[i].d,*d1; 
            if(d) {
                do {
                    d1 = d->nxt;
                    d->nxt = NULL;
                    delete d;
                } while((d = d1) != NULL);
                dirs[i].d = NULL; 
                dirs[i].cnt = 0;
            }
        }
	}
	if(!dironly && vals) { 
        for(I i = 0; i < vsize; ++i) {
            poolval *v = vals[i].v,*v1;
            if(v) {
                do {
                    v1 = v->nxt;
                    v->nxt = NULL;
                    delete v;
                } while((v = v1) != NULL);
                vals[i].v = NULL; 
                vals[i].cnt = 0;
            }
        }
    }
}

V pooldir::Reset(BL realloc)
{
	Clear(true,false);

	if(dirs) delete[] dirs; 
	if(vals) delete[] vals;

	if(realloc) {
		dirs = new direntry[dsize];
		ZeroMem(dirs,dsize*sizeof *dirs);
		vals = new valentry[vsize];
		ZeroMem(vals,vsize*sizeof *vals);
	}
	else 
		dirs = NULL,vals = NULL;
}

pooldir *pooldir::AddDir(I argc,const A *argv,I vcnt,I dcnt)
{
	if(!argc) return this;

	I c = 1,dix = DIdx(argv[0]);
	pooldir *prv = NULL,*ix = dirs[dix].d;
	for(; ix; prv = ix,ix = ix->nxt) {
		c = compare(argv[0],ix->dir);
		if(c <= 0) break;
	}

	if(c || !ix) {
		pooldir *nd = new pooldir(argv[0],this,vcnt,dcnt);
		nd->nxt = ix;

		if(prv) prv->nxt = nd;
		else dirs[dix].d = nd;
		dirs[dix].cnt++;
		ix = nd;
	}

	return ix->AddDir(argc-1,argv+1);
}

pooldir *pooldir::GetDir(I argc,const A *argv,BL rmv)
{
	if(!argc) return this;

	I c = 1,dix = DIdx(argv[0]);
	pooldir *prv = NULL,*ix = dirs[dix].d;
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
			else dirs[dix].d = nd;
			dirs[dix].cnt--;
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
    I c = 1,vix = VIdx(key);
	poolval *prv = NULL,*ix = vals[vix].v;
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
			else vals[vix].v = nv;
			vals[vix].cnt++;
		}
	}
	else if(over) { 
		// data exists... only set if overwriting enabled
		
		if(data)
			ix->Set(data);
		else {
			// delete key
		
			poolval *nv = ix->nxt;
			if(prv) prv->nxt = nv;
			else vals[vix].v = nv;
			vals[vix].cnt--;
			ix->nxt = NULL;
			delete ix;
		}
	}
}

BL pooldir::SetVali(I rix,AtomList *data)
{
    poolval *prv = NULL,*ix = NULL;
    int vix;
	for(vix = 0; vix < vsize; ++vix) 
		if(rix > vals[vix].cnt) rix -= vals[vix].cnt;
		else {
			ix = vals[vix].v;
			for(; ix && rix; prv = ix,ix = ix->nxt) --rix;
			if(ix && !rix) break;
		}  

	if(ix) { 
		// data exists... overwrite it
		
		if(data)
			ix->Set(data);
		else {
			// delete key
		
			poolval *nv = ix->nxt;
			if(prv) prv->nxt = nv;
			else vals[vix].v = nv;
			vals[vix].cnt--;
			ix->nxt = NULL;
			delete ix;
		}
        return true;
	}
    else
        return false;
}

poolval *pooldir::RefVal(const A &key)
{
	I c = 1,vix = VIdx(key);
	poolval *ix = vals[vix].v;
	for(; ix; ix = ix->nxt) {
		c = compare(key,ix->key);
		if(c <= 0) break;
	}

	return c || !ix?NULL:ix;
}

poolval *pooldir::RefVali(I rix)
{
	for(I vix = 0; vix < vsize; ++vix) 
		if(rix > vals[vix].cnt) rix -= vals[vix].cnt;
		else {
			poolval *ix = vals[vix].v;
			for(; ix && rix; ix = ix->nxt) --rix;
			if(ix && !rix) return ix;
		}
	return NULL;
}

flext::AtomList *pooldir::PeekVal(const A &key)
{
	poolval *ix = RefVal(key);
	return ix?ix->data:NULL;
}

flext::AtomList *pooldir::GetVal(const A &key,BL cut)
{
	I c = 1,vix = VIdx(key);
	poolval *prv = NULL,*ix = vals[vix].v;
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
			else vals[vix].v = nv;
			vals[vix].cnt--;
			ix->nxt = NULL;
			ret = ix->data; ix->data = NULL;
			delete ix;
		}
		else
			ret = new Atoms(*ix->data);
		return ret;
	}
}

I pooldir::CntAll() const
{
	I cnt = 0;
	for(I vix = 0; vix < vsize; ++vix) cnt += vals[vix].cnt;
	return cnt;
}

I pooldir::PrintAll(char *buf,int len) const
{
    int offs = strlen(buf);

    I cnt = 0;
    for(I vix = 0; vix < vsize; ++vix) {
		poolval *ix = vals[vix].v;
        for(I i = 0; ix; ++i,ix = ix->nxt) {
			PrintAtom(ix->key,buf+offs,len-offs);
            strcat(buf+offs," , ");
            int l = strlen(buf+offs)+offs;
			ix->data->Print(buf+l,len-l);
            post(buf);
        }
        cnt += vals[vix].cnt;
    }
    
    buf[offs] = 0;

	return cnt;
}

I pooldir::GetKeys(AtomList &keys)
{
	I cnt = CntAll();
	keys(cnt);

	for(I vix = 0; vix < vsize; ++vix) {
		poolval *ix = vals[vix].v;
		for(I i = 0; ix; ++i,ix = ix->nxt) 
			SetAtom(keys[i],ix->key);
	}
	return cnt;
}

I pooldir::GetAll(A *&keys,Atoms *&lst,BL cut)
{
	I cnt = CntAll();
	keys = new A[cnt];
	lst = new Atoms[cnt];

	for(I i = 0,vix = 0; vix < vsize; ++vix) {
		poolval *ix = vals[vix].v;
		for(; ix; ++i) {
			SetAtom(keys[i],ix->key);
			lst[i] = *ix->data;

			if(cut) {
				poolval *t = ix;
				vals[vix].v = ix = ix->nxt;
				vals[vix].cnt--;				
				t->nxt = NULL; delete t;
			}
			else
				ix = ix->nxt;
		}
	}
	return cnt;
}


I pooldir::CntSub() const
{
	I cnt = 0;
	for(I dix = 0; dix < dsize; ++dix) cnt += dirs[dix].cnt;
	return cnt;
}


I pooldir::GetSub(const A **&lst)
{
	const I cnt = CntSub();
	lst = new const A *[cnt];
	for(I i = 0,dix = 0; i < cnt; ++dix) {
		pooldir *ix = dirs[dix].d;
		for(; ix; ix = ix->nxt) lst[i++] = &ix->dir;
	}
	return cnt;
}


BL pooldir::Paste(const pooldir *p,I depth,BL repl,BL mkdir)
{
	BL ok = true;

	for(I vi = 0; vi < p->vsize; ++vi) {
		for(poolval *ix = p->vals[vi].v; ix; ix = ix->nxt) {
			SetVal(ix->key,new Atoms(*ix->data),repl);
		}
	}

	if(ok && depth) {
		for(I di = 0; di < p->dsize; ++di) {
			for(pooldir *dix = p->dirs[di].d; ok && dix; dix = dix->nxt) {
				pooldir *ndir = mkdir?AddDir(1,&dix->dir):GetDir(1,&dix->dir);
				if(ndir) { 
					ok = ndir->Paste(dix,depth > 0?depth-1:depth,repl,mkdir);
				}
			}
		}
	}

	return ok;
}

BL pooldir::Copy(pooldir *p,I depth,BL cut)
{
	BL ok = true;

	if(cut) {
		for(I vi = 0; vi < vsize; ++vi) {
			for(poolval *ix = vals[vi].v; ix; ix = ix->nxt)
				p->SetVal(ix->key,ix->data);
			vals[vi].cnt = 0;
			vals[vi].v = NULL;
		}
	}
	else {
		for(I vi = 0; vi < vsize; ++vi) {
			for(poolval *ix = vals[vi].v; ix; ix = ix->nxt) {
				p->SetVal(ix->key,new Atoms(*ix->data));
			}
		}
	}

	if(ok && depth) {
		for(I di = 0; di < dsize; ++di) {
			for(pooldir *dix = dirs[di].d; ok && dix; dix = dix->nxt) {
				pooldir *ndir = p->AddDir(1,&dix->dir);
				if(ndir)
					ok = dix->Copy(ndir,depth > 0?depth-1:depth,cut);
				else
					ok = false;
			}
		}
	}

	return ok;
}


static const char *ReadAtom(const char *c,A &a)
{
	// skip leading whitespace
	while(*c && isspace(*c)) ++c;
	if(!*c) return NULL;

	char tmp[1024];
    char *m = tmp; // write position

    bool issymbol;
    if(*c == '"') {
        issymbol = true;
        ++c;
    }
    else
        issymbol = false;

    // go to next whitespace
    for(bool escaped = false;; ++c)
        if(*c == '\\') {
            if(escaped) {
                *m++ = *c;
                escaped = false;
            }
            else
                escaped = true;
        }
        else if(*c == '"' && issymbol && !escaped) {
            // end of string
            ++c;
            FLEXT_ASSERT(!*c || isspace(*c));
            *m = 0;
            break;
        }
        else if(!*c || (isspace(*c) && !escaped)) {
            *m = 0;
            break;
        }
        else {
            *m++ = *c;
            escaped = false;
        }

    // save character and set delimiter

    float fres;
    // first try float
#if 0
    if(!issymbol && sscanf(tmp,"%f",&fres) == 1) {
#else
    char *endp;
    // see if it's a float - thanks to Frank Barknecht
    fres = (float)strtod(tmp,&endp);   
    if(!issymbol && !*endp && endp != tmp) { 
#endif
        int ires = (int)fres; // try a cast
        if(fres == ires)
            flext::SetInt(a,ires);
        else
            flext::SetFloat(a,fres);
    }
    // no, it's a symbol
    else
        flext::SetString(a,tmp);

	return c;
}

static BL ParseAtoms(C *tmp,flext::AtomList &l)
{
    const int MAXATOMS = 1024;
    int cnt = 0;
    t_atom atoms[MAXATOMS];
    for(const char *t = tmp; *t && cnt < MAXATOMS; ++cnt) {
		t = ReadAtom(t,atoms[cnt]);
        if(!t) break;
    }
    l(cnt,atoms);
	return true;
}

static BL ParseAtoms(string &s,flext::AtomList &l) 
{ 
    return ParseAtoms((C *)s.c_str(),l); 
}

static BL ReadAtoms(istream &is,flext::AtomList &l,C del)
{
	C tmp[1024];
	is.getline(tmp,sizeof tmp,del); 
	if(is.eof() || !is.good()) 
        return false;
    else
        return ParseAtoms(tmp,l);
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
    case A_SYMBOL: {
        const char *c = flext::GetString(flext::GetSymbol(a));
        os << '"';
        for(; *c; ++c) {
            if(isspace(*c) || *c == '\\' || *c == ',' || *c == '"')
                os << '\\';
	        os << *c;
        }
        os << '"';
		break;
	}
    }
}

static V WriteAtoms(ostream &os,const flext::AtomList &l)
{
	for(I i = 0; i < l.Count(); ++i) {
		WriteAtom(os,l[i]);
		if(i < l.Count()-1) os << ' ';
	}
}

BL pooldir::LdDir(istream &is,I depth,BL mkdir)
{
	for(I i = 1; !is.eof(); ++i) {
		Atoms d,k,*v = new Atoms;
		BL r = 
            ReadAtoms(is,d,',') && 
            ReadAtoms(is,k,',') &&
            ReadAtoms(is,*v,'\n');

		if(r) {
			if(depth < 0 || d.Count() <= depth) {
				pooldir *nd = mkdir?AddDir(d):GetDir(d);
				if(nd) {
                    if(k.Count() == 1) {
	    				nd->SetVal(k[0],v); v = NULL;
                    }
                    else if(k.Count() > 1)
                        post("pool - file format invalid: key must be a single word");
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
    I cnt = 0;
	for(I vi = 0; vi < vsize; ++vi) {
		for(poolval *ix = vals[vi].v; ix; ix = ix->nxt) {
			WriteAtoms(os,dir);
			os << " , ";
			WriteAtom(os,ix->key);
			os << " , ";
			WriteAtoms(os,*ix->data);
			os << endl;
            ++cnt;
		}
	}
    if(!cnt) {
        // no key/value pairs present -> force empty directory
		WriteAtoms(os,dir);
		os << " , ," << endl;
    }
	if(depth) {
        // save sub-directories
		I nd = depth > 0?depth-1:-1;
		for(I di = 0; di < dsize; ++di) {
			for(pooldir *ix = dirs[di].d; ix; ix = ix->nxt) {
				ix->SvDir(os,nd,Atoms(dir).Append(ix->dir));
			}
		}
	}
	return true;
}

class xmltag {
public:
    string tag,attr;
    bool Ok() const { return tag.length() > 0; }
    bool operator ==(const C *t) const { return !tag.compare(t); }

    void Clear() 
    { 
#if defined(_MSC_VER) && (_MSC_VER < 0x1200)
        // incomplete STL implementation
        tag = ""; attr = ""; 
#else
        tag.clear(); attr.clear(); 
#endif
    }

    enum { t_start,t_end,t_empty } type;
};

static bool gettag(istream &is,xmltag &tag)
{
    static const char *commstt = "<!--",*commend = "-->";

    for(;;) {
        // eat whitespace
        while(isspace(is.peek())) is.get();

        // no tag begin -> break
        if(is.peek() != '<') break;
        is.get(); // swallow <

        char tmp[1024],*t = tmp;

        // parse for comment start
        const char *c = commstt;
        while(*++c) {
            if(*c != is.peek()) break;
            *(t++) = is.get();
        }

        if(!*c) { // is comment
            char cmp[2] = {0,0}; // set to some unusual initial value

            for(int ic = 0; ; ic = (++ic)%2) {
                char c = is.get();
                if(c == '>') {
                    // if third character is > then check also the former two
                    int i;
                    for(i = 0; i < 2 && cmp[(ic+i)%2] == commend[i]; ++i) {}
                    if(i == 2) break; // match: comment end found!
                }
                else
                    cmp[ic] = c;
            }
        }
        else {
            // parse until > with consideration of "s
            bool intx = false;
            for(;;) {
                *t = is.get();
                if(*t == '"') intx = !intx;
                else if(*t == '>' && !intx) {
                    *t = 0;
                    break;
                }
                t++;
            }

            // look for tag slashes

            char *tb = tmp,*te = t-1,*tf;

            for(; isspace(*tb); ++tb) {}
            if(*tb == '/') { 
                // slash at the beginning -> end tag
                tag.type = xmltag::t_end;
                for(++tb; isspace(*tb); ++tb) {}
            }
            else {
                for(; isspace(*te); --te) {}
                if(*te == '/') { 
                    // slash at the end -> empty tag
                    for(--te; isspace(*te); --te) {}
                    tag.type = xmltag::t_empty;
                }
                else 
                    // no slash -> begin tag
                    tag.type = xmltag::t_start;
            }

            // copy tag text without slashes
            for(tf = tb; tf <= te && *tf && !isspace(*tf); ++tf) {}
            tag.tag.assign(tb,tf-tb);
            while(isspace(*tf)) ++tf;
            tag.attr.assign(tf,te-tf+1);

            return true;
        }
    }

    tag.Clear();
    return false;
}

static void getvalue(istream &is,string &s)
{
    char tmp[1024],*t = tmp; 
    bool intx = false;
    for(;;) {
        char c = is.peek();
        if(c == '"') intx = !intx;
        else if(c == '<' && !intx) break;
        *(t++) = is.get();
    }
    *t = 0;
    s = tmp;
}

BL pooldir::LdDirXMLRec(istream &is,I depth,BL mkdir,AtomList &d)
{
    Atoms k,v;
    bool inval = false,inkey = false,indata = false;
    int cntval = 0;

	while(!is.eof()) {
        xmltag tag;
        gettag(is,tag);
        if(!tag.Ok()) {
            // look for value
            string s;
            getvalue(is,s);

            if(s.length() &&
                (
                    (!inval && inkey && d.Count()) ||  /* dir */
                    (inval && (inkey || indata)) /* value */
                )
            ) {
                BL ret = true;
                if(indata) {
                    if(v.Count())
                        post("pool - XML load: value data already given, ignoring new data");
                    else
                        ret = ParseAtoms(s,v);
                }
                else // inkey
                    if(inval) {
                        if(k.Count())
                            post("pool - XML load, value key already given, ignoring new key");
                        else
                            ret = ParseAtoms(s,k);
                    }
                    else {
                        t_atom &dkey = d[d.Count()-1];
                        FLEXT_ASSERT(IsSymbol(dkey));
                        const char *ds = GetString(dkey);
                        FLEXT_ASSERT(ds);
                        if(*ds) 
                            post("pool - XML load: dir key already given, ignoring new key");
                        else
                            ReadAtom(s.c_str(),dkey);

                        ret = true;
                    }
                if(!ret) post("pool - error interpreting XML value (%s)",s.c_str());
            }
            else
                post("pool - error reading XML data");
        }
        else if(tag == "dir") {
            if(tag.type == xmltag::t_start) {
                // warn if last directory key was not given
                if(d.Count() && GetSymbol(d[d.Count()-1]) == sym__)
                    post("pool - XML load: dir key must be given prior to subdirs, ignoring items");

                Atoms dnext(d.Count()+1);
                // copy existing dir
                dnext.Set(d.Count(),d.Atoms(),0,false);
                // initialize current dir key as empty
                SetSymbol(dnext[d.Count()],sym__);

                // read next level
                LdDirXMLRec(is,depth,mkdir,dnext); 
            }
            else if(tag.type == xmltag::t_end) {
                if(!cntval && mkdir) {
                    // no values have been found in dir -> make empty dir
                    AddDir(d);
                }

                // break tag loop
                break;
            }
        }
        else if(tag == "value") {
            if(tag.type == xmltag::t_start) {
                inval = true;
                ++cntval;
                k.Clear(); v.Clear();
            }
            else if(tag.type == xmltag::t_end) {
                // set value after tag closing, but only if level <= depth
        	    if(depth < 0 || d.Count() <= depth) {
                    int fnd;
                    for(fnd = d.Count()-1; fnd >= 0; --fnd)
                        if(GetSymbol(d[fnd]) == sym__) break;

                    // look if last dir key has been given
                    if(fnd >= 0) {
                        if(fnd == d.Count()-1)
                            post("pool - XML load: dir key must be given prior to values");

                        // else: one directoy level has been left unintialized, ignore items
                    }
                    else {
                        // only use first word of key
                        if(k.Count() == 1) {
		        		    pooldir *nd = mkdir?AddDir(d):GetDir(d);
        				    if(nd) 
                                nd->SetVal(k[0],new Atoms(v));
                            else
                                post("pool - XML load: value key must be exactly one word, value not stored");
				        }
                    }
                }
                inval = false;
            }
        }
        else if(tag == "key") {
            if(tag.type == xmltag::t_start) {
                inkey = true;
            }
            else if(tag.type == xmltag::t_end) {
                inkey = false;
            }
        }
        else if(tag == "data") {
            if(!inval) 
                post("pool - XML tag <data> not within <value>");

            if(tag.type == xmltag::t_start) {
                indata = true;
            }
            else if(tag.type == xmltag::t_end) {
                indata = false;
            }
        }
        else if(!d.Count() && tag == "pool" && tag.type == xmltag::t_end) {
            // break tag loop
            break;
        }
#ifdef FLEXT_DEBUG
        else {
            post("pool - unknown XML tag '%s'",tag.tag.c_str());
        }
#endif
    }
    return true;
}

BL pooldir::LdDirXML(istream &is,I depth,BL mkdir)
{
	while(!is.eof()) {
        xmltag tag;
        if(!gettag(is,tag)) break;

        if(tag == "pool") {
            if(tag.type == xmltag::t_start) {
                Atoms empty; // must be a separate definition for gcc
                LdDirXMLRec(is,depth,mkdir,empty);
            }
            else
                post("pool - pool not initialized yet");
        }
        else if(tag == "!DOCTYPE") {
            // ignore
        }
#ifdef FLEXT_DEBUG
        else {
            post("pool - unknown XML tag '%s'",tag.tag.c_str());
        }
#endif
    }
    return true;
}

static void indent(ostream &s,I cnt) 
{
    for(I i = 0; i < cnt; ++i) s << '\t';
}

BL pooldir::SvDirXML(ostream &os,I depth,const AtomList &dir,I ind)
{
	int i,lvls = ind?(dir.Count()?1:0):dir.Count();

	for(i = 0; i < lvls; ++i) {
		indent(os,ind+i);
		os << "<dir>" << endl;
		indent(os,ind+i+1);
		os << "<key>";
		WriteAtom(os,dir[ind+i]);
		os << "</key>" << endl;
	}

	for(I vi = 0; vi < vsize; ++vi) {
		for(poolval *ix = vals[vi].v; ix; ix = ix->nxt) {
            indent(os,ind+lvls);
            os << "<value><key>";
			WriteAtom(os,ix->key);
            os << "</key><data>";
			WriteAtoms(os,*ix->data);
			os << "</data></value>" << endl;
		}
	}

	if(depth) {
		I nd = depth > 0?depth-1:-1;
		for(I di = 0; di < dsize; ++di) {
			for(pooldir *ix = dirs[di].d; ix; ix = ix->nxt) {
				ix->SvDirXML(os,nd,Atoms(dir).Append(ix->dir),ind+lvls);
			}
		}
	}

	for(i = lvls-1; i >= 0; --i) {
		indent(os,ind+i);
		os << "</dir>" << endl;
	}
	return true;
}

unsigned int pooldir::FoldBits(unsigned long h,int bits)
{
	if(!bits) return 0;
	const int hmax = (1<<bits)-1;
	unsigned int ret = 0;
	for(unsigned int i = 0; i < sizeof(h)*8; i += bits)
		ret ^= (h>>i)&hmax;
	return ret;
}

int pooldir::Int2Bits(unsigned long n)
{
	int b;
	for(b = 0; n; ++b) n >>= 1;
	return b;
}
