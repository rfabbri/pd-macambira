/* 

delsplit - split a delimited list-in-a-symbol

Copyright (c) 2002-2003 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error You need at least flext version 0.4.1 
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#define I int
#define L long
#define F float
#define D double
#define V void
#define C char
#define BL bool

#define VERSION "0.1.2"

#ifdef __MWERKS__
#define STD std
#else
#define STD 
#endif


class delsplit:
	public flext_base
{
	FLEXT_HEADER_S(delsplit,flext_base,Setup)

public:
	delsplit(I argc,const t_atom *argv);

protected:
	V m_list(const t_symbol *s);
	V m_del(const t_symbol *s);
	
	const t_symbol *delim;
	
	virtual void m_help();
		
	static V SetAtom(t_atom &l,const C *s);
private:
	static V Setup(t_classid c);

	FLEXT_CALLBACK_S(m_list)
	FLEXT_CALLBACK_S(m_del)
};

FLEXT_NEW_V("delsplit",delsplit)


V delsplit::Setup(t_classid c)
{
	FLEXT_CADDMETHOD(c,0,m_list);
	FLEXT_CADDMETHOD(c,1,m_del);
}

delsplit::delsplit(I argc,const t_atom *argv):
	delim(NULL)
{ 
	AddInAnything("Symbol in, representing the delimited list");
	AddInSymbol("Set the Delimiter");
	AddOutList("The split list");

	if(argc && IsSymbol(argv[0])) delim = GetSymbol(argv[0]);
}

V delsplit::m_help()
{
	post("%s version " VERSION " (using flext " FLEXT_VERSTR "), (C) 2002 Thomas Grill",thisName());
}

/** \brief check whether string represents a number
	\ret 0..integer, 1..float, -1..no number
*/
static I chknum(const C *s)
{
	int num = 0,pts = 0;
	for(const char *si = s; *s; ++s) {
		if(*s == '.') ++pts;
		else if(isdigit(*s)) ++num;
		else { num = 0; break; }
	}
	return (num > 0 && pts <= 1)?pts:-1;
}

V delsplit::SetAtom(t_atom &l,const C *s)
{
	I n = chknum(s);

	if(n < 0)
		SetString(l,s);
	else if(n == 0)
		SetInt(l,atoi(s));
	else
		SetFloat(l,(F)atof(s));
}

V delsplit::m_list(const t_symbol *sym)
{
	if(delim) {
		t_atom lst[256];
		int cnt = 0;
		const C *sdel = GetString(delim);
		I ldel = strlen(sdel);
		C str[1024];
		strcpy(str,GetString(sym));
		
		for(const char *s = str; *s; ) {
			C *e = strstr(s,sdel);
			if(!e) {
				SetAtom(lst[cnt++],s);
				break;
			}
			else {
				*e = 0;
				SetAtom(lst[cnt++],s);
				s = e+ldel;
			}
		}
		
		ToOutList(0,cnt,lst);
	}
	else
		post("%s - No delimiter defined",thisName());
}

V delsplit::m_del(const t_symbol *s)
{
	delim = s;
}

