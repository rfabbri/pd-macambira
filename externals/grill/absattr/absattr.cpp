/* 

absattr - patcher attributes

Copyright (c) 2002-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#define VERSION "0.1.0"

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 500)
#error You need at least flext version 0.5.0
#endif

#include <map>

class absattr
    : public flext_base
{
	FLEXT_HEADER_S(absattr,flext_base,Setup)

public:
    absattr(int argc,const t_atom *argv)
//        : loadbang(lb)
    {
        AddInAnything("bang/get/set");
        AddInAnything("external attribute messages");
        AddOutAnything("arguments");
        AddOutAnything("attributes");
        AddOutAnything("external attribute outlet");

		// process default values (only attributes can have default values)
		Process(argc,argv);

		// process canvas arguments
        AtomListStatic<20> args;
        GetCanvasArgs(args);
        Process(args.Count(),args.Atoms());
    }

    //! dump parameters
    void m_bang()
    {
        ToOutList(0,args);
        for(AttrMap::const_iterator it = attrs.begin(); it != attrs.end(); ++it) {
            const AtomList &lst = it->second;
            ToOutAnything(1,it->first,lst.Count(),lst.Atoms());
        }
    }

    void m_dump() { DumpAttr(0); }

    void m_get(const t_symbol *s) { OutAttr(0,s); }

    void m_dumpx() { DumpAttr(1); }

    void m_getx(const t_symbol *s) { OutAttr(1,s); }

    void m_set(int argc,const t_atom *argv)
    {
        if(!argc && !IsSymbol(*argv))
            post("%s - attribute must be given as first argument",thisName());
        else {
            const t_symbol *attr = GetSymbol(*argv);
            --argc,++argv;
            SetAttr(attr,argc,argv);
        }
    }

protected:

    typedef std::map<const t_symbol *,AtomList> AttrMap;

//    bool loadbang;
    AtomList args;
    AttrMap attrs;  

//    virtual void CbLoadbang() { if(loadbang) m_bang(); }

    void DumpAttr(int ix)
    {
        int cnt = attrs.size();
        AtomListStatic<20> lst(cnt);

        AttrMap::const_iterator it = attrs.begin();
        for(int i = 0; it != attrs.end(); ++it,++i)
            SetSymbol(lst[i],it->first);

        ToOutAnything(1+ix,sym_attributes,lst.Count(),lst.Atoms());
    }

    void OutAttr(int ix,const t_symbol *s)
    {
        AttrMap::const_iterator it = attrs.find(s);
        if(it != attrs.end()) {
            const AtomList &lst = it->second;
            ToOutAnything(1+ix,s,lst.Count(),lst.Atoms());
        }
        else
            post("%s - attribute %s not found",thisName(),GetString(s));
    }

    void SetAttr(const t_symbol *attr,int argc,const t_atom *argv)
    {
        if(argc) 
            attrs[attr].Set(argc,argv,0,true);
        else
            attrs.erase(attr);
    }

    static IsAttr(const t_atom &at) { return IsSymbol(at) && *GetString(at) == '@'; }

    void Process(int argc,const t_atom *argv)
    {
        int cnt;
        for(cnt = 0; cnt < argc && !IsAttr(argv[cnt]); ++cnt) {}

        args.Set(cnt,argv,0,true);
        argc -= cnt,argv += cnt;

        while(argc) {
            FLEXT_ASSERT(IsAttr(*argv));
            const t_symbol *attr = MakeSymbol(GetString(*argv)+1);
            --argc,++argv;

            for(cnt = 0; cnt < argc && !IsAttr(argv[cnt]); ++cnt) {}

            if(cnt) {
                AtomList &lst = attrs[attr];
                lst.Set(cnt,argv,0,true);
                argc -= cnt,argv += cnt;
            }
            else
                attrs.erase(attr);
        }
    }

private:

    static const t_symbol *sym_attributes;

    FLEXT_CALLBACK(m_bang);
    FLEXT_CALLBACK_S(m_get);
    FLEXT_CALLBACK(m_dump);
    FLEXT_CALLBACK_S(m_getx);
    FLEXT_CALLBACK(m_dumpx);
    FLEXT_CALLBACK_V(m_set);

	static void Setup(t_classid cl)
    {
        sym_attributes = MakeSymbol("attributes");

        FLEXT_CADDBANG(cl,0,m_bang);
        FLEXT_CADDMETHOD_(cl,0,"get",m_get);
        FLEXT_CADDMETHOD_(cl,0,"getattributes",m_dump);
        FLEXT_CADDMETHOD_(cl,0,"set",m_set);
        FLEXT_CADDMETHOD_(cl,1,"get",m_getx);
        FLEXT_CADDMETHOD_(cl,1,"getattributes",m_dumpx);
        FLEXT_CADDMETHOD_(cl,1,"set",m_set);
    }
};

const t_symbol *absattr::sym_attributes;


FLEXT_NEW_V("absattr",absattr)
