/*

namedobjs - retrieve list of named objects in patcher (MaxMSP only!)

Copyright (c) 2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 203)
#error You need at least flext version 0.2.3
#endif

#ifndef MAXMSP
#error "This object is for MaxMSP only!"
#endif

#include <ctype.h>


#define I int
#define L long
#define V void
#define C char

class namedobjs:
	public flext_base
{
	FLEXT_HEADER(namedobjs,flext_base)
 
public:
	namedobjs(I argc,t_atom *argv);

protected:
	V m_bang();
	virtual V m_assist(L msg,L arg,C *s);
	
private:
	FLEXT_CALLBACK(m_bang);
};

FLEXT_NEW_G("namedobjs",namedobjs)


namedobjs::namedobjs(I argc,t_atom *argv)
{ 
	AddInAnything();
	AddOutList();
	AddOutBang();
	SetupInOut();
	
	FLEXT_ADDBANG(0,m_bang);
}

V namedobjs::m_bang()
{
	t_canvas *canv = thisCanvas();
//	t_object *self = (t_object *)&x_obj->obj;

	t_box *b;
	for(b = canv->p_box; b; b = b->b_next) {
		if(b->b_firstin) {
//			if(NOGOOD(b->b_firstin)) post("NOGOOD!");
	
			t_messlist *ms;
			
			ms = ((t_tinyobject *)b->b_firstin)->t_messlist;
			if(ms) {
		 		const t_class *c = (const t_class *)(ms-1);
				if(c) {
					t_symbol *nm;
			       	if (patcher_boxname(canv,b,&nm)) { 
			   			t_atom lst[4];
			   			SetString(lst[0],*(C **)c->c_sym->s_name);
			   			SetSymbol(lst[1],nm);
			   			SetInt(lst[2],b->b_rect.left);
			   			SetInt(lst[3],b->b_rect.top);
						ToOutList(0,4,lst);
					} 
				}

			}
		}
	}
	
	ToOutBang(1);
}

V namedobjs::m_assist(L msg,L arg,C *s)
{
	switch(msg) {
	case 1: //ASSIST_INLET:
		switch(arg) {
		case 0:
			sprintf(s,"Bang to retrieve list of named objects"); break;
		}
		break;
	case 2: //ASSIST_OUTLET:
		switch(arg) {
		case 0:
			sprintf(s,"Consecutive object type/name pairs"); break;
		case 1:
			sprintf(s,"Bang signals end of list"); break;
		}
		break;
	}
}


