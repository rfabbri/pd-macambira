/* 

deljoin - join a list with delimiter

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 400)
#error You need at least flext version 0.4.0
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define I int
#define L long
#define F float
#define D double
#define V void
#define C char
#define BL bool

#define VERSION "0.1.1"

#ifdef __MWERKS__
#define STD std
#else
#define STD 
#endif


class deljoin:
	public flext_base
{
	FLEXT_HEADER(deljoin,flext_base)

public:
	deljoin(I argc,const t_atom *argv);

protected:
	V m_list(const t_symbol *s,int argc,const t_atom *argv);
	V m_del(const t_symbol *s);
	
	const t_symbol *delim;
	
	virtual void m_help();
	virtual void m_assist(long /*msg*/,long /*arg*/,char * /*s*/);
		
private:
	FLEXT_CALLBACK_A(m_list)
	FLEXT_CALLBACK_S(m_del)
};

FLEXT_NEW_V("deljoin",deljoin)


deljoin::deljoin(I argc,const t_atom *argv):
	delim(NULL)
{ 
	AddInAnything();
	AddInSymbol();
	AddOutSymbol();

	FLEXT_ADDMETHOD_A(0,m_list);
	FLEXT_ADDMETHOD(1,m_del);

	if(argc && IsSymbol(argv[0])) delim = GetSymbol(argv[0]);
}

V deljoin::m_help()
{
	post("%s version " VERSION " (using flext " FLEXT_VERSTR "), (C) 2002 Thomas Grill",thisName());
}

V deljoin::m_assist(long msg,long arg,char *s)
{
	switch(msg) {
	case 1: //ASSIST_INLET:
		switch(arg) {
		case 0:
			STD::sprintf(s,"Anything in - triggers output"); break;
		case 1:
			STD::sprintf(s,"Set the Delimiter"); break;
		}
		break;
	case 2: //ASSIST_OUTLET:
		switch(arg) {
		case 0:
			STD::sprintf(s,"A symbol representing the joined list"); break;
		}
		break;
	}
}
	
/** \brief convert incoming list to a concatenated string
	
	Handles symbols, integers and floats
*/
V deljoin::m_list(const t_symbol *s,int argc,const t_atom *argv)
{
	C tmp[1024],*t = tmp;
	const C *sdel = GetString(delim);
	I ldel = strlen(sdel);
	
	if(s && s != sym_list && s != sym_float && s != sym_int) {
		strcpy(t,GetString(s));		
		t += strlen(t);
	}
	
	for(int i = 0; i < argc; ++i) {
		if(t != tmp) {
			strcpy(t,sdel);		
			t += ldel;
		}
	
		const t_atom &a = argv[i];
		if(IsSymbol(a))
			strcpy(t,GetString(a));
		else if(IsInt(a)) {
			STD::sprintf(t,"%i",GetInt(a),10);
		}
		else if(IsFloat(a)) {
			STD::sprintf(t,"%f",GetFloat(a),10);
		}
//		else do nothing
			
		t += strlen(t);
	}
	
	ToOutString(0,tmp);
}

V deljoin::m_del(const t_symbol *s)
{
	delim = s;
}

