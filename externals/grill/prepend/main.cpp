/* 

prepend - just like in MaxMSP

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 401)
#error You need at least flext version 0.4.1
#endif

#define PREPEND_VERSION "0.0.3"

class prepend: 
	public flext_base
{
	FLEXT_HEADER_S(prepend, flext_base, Setup)

public:
	prepend(int argc,t_atom *argv);

protected:
	void m_bang();
	void m_set(int argc,t_atom *argv) { reg[0].Store(NULL,argc,argv); }
	void m_any0(const t_symbol *s,int argc,t_atom *argv) { reg[1].Store(s,argc,argv); m_bang(); }
	void m_any1(const t_symbol *s,int argc,t_atom *argv) { reg[0].Store(s,argc,argv); }

	virtual void m_help();

private:	

	static void Setup(t_class *c);

	class reg_t 
	{ 
	public:
		reg_t(): cnt(0),lst(NULL) {}
		~reg_t() { if(lst) delete[] lst; }

		bool Is() const { return cnt && lst; }
		bool IsSimple() const { return !IsSymbol(lst[0]); }
		bool IsList() const { return IsSymbol(lst[0]) && GetSymbol(lst[0]) == &s_list; }

		void Store(const t_symbol *s,int argc,t_atom *argv);
		int cnt; t_atom *lst; 
	} reg[2];

	FLEXT_CALLBACK(m_bang)
	FLEXT_CALLBACK_V(m_set)
	FLEXT_CALLBACK_A(m_any0)
	FLEXT_CALLBACK_A(m_any1)
};

FLEXT_NEW_V("prepend",prepend)



prepend::prepend(int argc,t_atom *argv)
{
	AddInAnything(2);  
	AddOutAnything();   

	m_set(argc,argv);
}

void prepend::Setup(t_class *c)
{
	FLEXT_CADDMETHOD_(c,0,"bang",m_bang);
//	FLEXT_CADDMETHOD_(c,0,"set",m_set);
	FLEXT_CADDMETHOD(c,0,m_any0);
	FLEXT_CADDMETHOD(c,1,m_any1);
}

void prepend::reg_t::Store(const t_symbol *s,int argc,t_atom *argv)
{
	if(lst) delete[] lst; cnt = 0;
	lst = new t_atom[argc+1];

	if(s && s != &s_float) SetSymbol(lst[cnt++],s);
	else if(argc > 0 && !IsSymbol(argv[0])) SetSymbol(lst[cnt++],&s_list);

	for(int i = 0; i < argc; ++i,++cnt) lst[cnt] = argv[i];
}

void prepend::m_bang()
{
	t_atom *ret = new t_atom[reg[0].cnt+reg[1].cnt+1];
	int i,rcnt = 0;

	if(reg[0].Is()) {
		if(reg[0].IsSimple()) SetSymbol(ret[rcnt++],&s_list);
		for(i = reg[0].IsList()?1:0; i < reg[0].cnt; ++i) ret[rcnt++] = reg[0].lst[i];
	}
	else
		SetSymbol(ret[rcnt++],&s_list);

	if(reg[1].Is()) {
		for(i = reg[1].IsList()?1:0; i < reg[1].cnt; ++i) ret[rcnt++] = reg[1].lst[i];
	}

	if(IsSymbol(ret[0]))
		ToOutAnything(0,GetSymbol(ret[0]),rcnt-1,ret+1);
	else
		ToOutList(0,rcnt,ret);
	delete[] ret;
}

void prepend::m_help()
{
	post("%s - just like in Max/MSP, version " PREPEND_VERSION,thisName());
#ifdef _DEBUG
	post("compiled on " __DATE__ " " __TIME__);
#endif
	post("(C) Thomas Grill, 2002");
	post("");
	post("Arguments: %s [atoms to prepend]",thisName());
	post("Inlets: 1:triggering atoms");
	post("Inlets: 2:atoms to prepend");
	post("Outlets: 1:sum of prepend");	
	post("Methods:");
	post("\thelp: shows this help");
//	post("\tset [atoms]: set atoms to prepend");
	post("");
}





