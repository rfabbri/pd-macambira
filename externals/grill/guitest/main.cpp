#include "flgui.h"
#include "flguiobj.h"

#include <stdlib.h>
/*
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
*/

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 303)
#error You need at least flext version 0.3.3
#endif


#define V void
#define I int
#define C char
#define BL bool
#define L long
#define UL unsigned long

class guitest:
public flext_gui //,virtual public flext_base
{
	FLEXT_HEADER(guitest,flext_gui)
 
public:
	guitest(I argc,t_atom *argv);
	~guitest();

	void m_bang() 
	{ 
		post("%s - bang!",thisName()); 
	}

protected:

	virtual void g_Create();
//	virtual void g_Edit(bool selected);

	static bool g_Motion(flext_gui &g,GuiSingle &obj,const CBParams &p);
	static bool g_MouseKey(flext_gui &g,GuiSingle &obj,const CBParams &p);
	static bool g_Key(flext_gui &g,GuiSingle &obj,const CBParams &p);

//	virtual void g_Properties();
	virtual void g_Save(t_binbuf *b);

private:
	FLEXT_CALLBACK(m_bang);
};

FLEXT_NEW_V("guitest",guitest)

guitest::guitest(I argc,t_atom *argv):
	flext_gui(400,100)
{ 
	AddInAnything();  
	AddOutInt(2); 

	FLEXT_ADDBANG(0,m_bang);
}

guitest::~guitest()
{
}


void guitest::g_Create()
{
	GuiSingle *frame = Group().Add_Box(0,0,XSize(),YSize(),-1,0xE0E0E0);
	frame->Symbol("rect1");
//	GuiSingle *wave = Group().Add_Box(8,10,XSize()-16,YSize()-11,0,0x4040FF);
	Group().Add_Text(1,1,"Hula",-1,GuiText::left);
/*
	I n = XSize()-16;
	FPnt *p = new FPnt[n];
	for(int i = 0; i < n; ++i) {
		p[i](8+i,10+rand()%(YSize()-11));
	}
	Group().Add_Poly(n,p,-1,0xC0C0C0);
	delete[] p;

	if(!BindEvent(*frame,g_Motion,evMotion)) post("Motion not supported");
	if(!BindEvent(*frame,g_Motion,evMouseDrag)) post("MouseDrag not supported");
	if(!BindEvent(*wave,g_MouseKey,evMouseDown)) post("MouseDown not supported");
	if(!BindEvent(*wave,g_MouseKey,evKeyDown)) post("KeyDown not supported");
	if(!BindEvent(*wave,g_MouseKey,evKeyUp)) post("KeyUp not supported");
	if(!BindEvent(*wave,g_MouseKey,evKeyRepeat)) post("KeyRepeat not supported");
*/
}

/*
void guitest::g_Properties()
{
	post("properties");
}
*/

void guitest::g_Save(t_binbuf *b)
{
	post("save");
#if FLEXT_SYS == FLEXT_SYS_PD
	binbuf_addv(b, "ssiis;", gensym("#X"),gensym("obj"),
		(t_int)XLo(), (t_int)YLo(),MakeSymbol(thisName())
		// here the arguments
	);
#else
#endif
}

/*
void guitest::g_Edit(bool selected)
{
	post("select is=%d", selected);
	
	GuiSingle *obj = Group().Find(MakeSymbol("rect1"));
	if(obj) 
		obj->Outline(selected?0xFF0000:0x000000);
	else
		post("obj not found");
}
*/

bool guitest::g_Motion(flext_gui &g,GuiSingle &obj,const CBParams &p)
{
	if(p.kind == evMotion) {
		post("Motion %s x:%i y:%i mod:%i",GetString(obj.Id()),p.pMotion.x,p.pMotion.y,p.pMotion.mod);
	}
	else if(p.kind == evMouseDrag) {
		post("Drag %s x:%i y:%i dx:%i dy:%i b:%i mod:%i",GetString(obj.Id()),p.pMouseDrag.x,p.pMouseDrag.y,p.pMouseDrag.dx,p.pMouseDrag.dy,p.pMouseDrag.b,p.pMouseDrag.mod);
	}
	else
		post("Motion");
	return true;
}

bool guitest::g_MouseKey(flext_gui &g,GuiSingle &obj,const CBParams &p)
{
	if(p.kind == evMouseDown) {
		post("MouseDown %s x:%i y:%i b:%i mod:%i",GetString(obj.Id()),p.pMouseKey.x,p.pMouseKey.y,p.pMouseKey.b,p.pMouseKey.mod);
	}
	else if(p.kind == evKeyDown) {
		post("KeyDown %s asc:%i key:%i mod:%i",GetString(obj.Id()),p.pKey.a,p.pKey.k,p.pKey.mod);
	}
	else if(p.kind == evKeyUp) {
		post("KeyUp %s asc:%i key:%i mod:%i",GetString(obj.Id()),p.pKey.a,p.pKey.k,p.pKey.mod);
	}
	else if(p.kind == evKeyRepeat) {
		post("KeyRepeat %s asc:%i key:%i mod:%i",GetString(obj.Id()),p.pKey.a,p.pKey.k,p.pKey.mod);
	}
	return true;
}

bool guitest::g_Key(flext_gui &g,GuiSingle &obj,const CBParams &p)
{
	post("Key");
	return true;
}







