#ifndef __FLGUIOBJ_H
#define __FLGUIOBJ_H

#define FLEXT_VIRT
#include <flext.h>
#include "flgui.h"


class Canvas
{
public:
	Canvas(t_canvas *c);
	~Canvas();

#if FLEXT_SYS == FLEXT_SYS_PD
	Canvas &Tk(char *fmt,...);
	Canvas &TkC();
	Canvas &TkE();

	void ToBuf(const char *t);
#endif

	bool Pre(int x,int y);
	void Post();

	int X() const { return xpos; }
	int Y() const { return ypos; }

protected:

	t_canvas *canvas;
	int xpos,ypos;

#if FLEXT_SYS == FLEXT_SYS_PD
	void Send(const char *t);
	void SendBuf();

	static bool debug,store;

	char *buffer;
	int bufix;
	int waiting;
#else
	PenState svpen;
	RGBColor svcol;
	GrafPtr svgp;
#endif
};


class Pnt 
{
public:
	Pnt() {}
	Pnt(const Pnt &p): x(p.x),y(p.y) {}
	Pnt(int px,int py): x(px),y(py) {}

	Pnt &operator =(const Pnt &p) { x = p.x,y = p.y; return *this; }
	Pnt &operator ()(int px,int py) { x = px,y = py; return *this; }

	Pnt &Move(int dx,int dy) { x += dx,y += dy; return *this; }

	int X() const { return x; }
	int Y() const { return y; }

//protected:
	int x,y;
};

class Rect
{
public:
	Rect() {}
	Rect(const Rect &r):  lo(r.lo),hi(r.hi) {}
	Rect(int xlo,int ylo,int xhi,int yhi): lo(xlo,ylo),hi(xhi,yhi) {}
	
	Rect &operator =(const Rect &r) { lo = r.lo; hi = r.hi; return *this; }
	Rect &operator ()(const Pnt &l,const Pnt &h) { lo = l; hi = h; return *this; }
	Rect &operator ()(int xlo,int ylo,int xhi,int yhi) { lo(xlo,ylo); hi(xhi,yhi); return *this; }

	Rect &Move(int dx,int dy) { lo.Move(dx,dy); hi.Move(dx,dy); return *this; }
	Rect &MoveTo(int x,int y) { hi(x+hi.X()-lo.X(),y+hi.Y()-lo.Y()); lo(x,y); return *this; }

	Pnt &Lo() { return lo; }
	Pnt &Hi() { return hi; }
	int SizeX() const { return hi.X()-lo.X()+1; }
	int SizeY() const { return hi.Y()-lo.Y()+1; }
	
	Rect &Add(const Pnt &p);
	Rect &Add(const Rect &r);
	bool In(const Pnt &p) const;
	bool Inter(const Rect &r) const;
	
protected:
	Pnt lo,hi;
};

class GuiObj:
	public flext
{
	friend class GuiGroup;
public:
	GuiObj(Canvas *c = NULL,GuiGroup *p = NULL);
	virtual ~GuiObj();

	const t_symbol *Id() const { return idsym; }
	virtual const t_symbol *Symbol() const { return NULL; }

	virtual void Active() {}
	virtual void Inactive() {}

/*
	void Origin(int x,int y) { ori(x,y); }
	void Origin(const Pnt &p) { ori = p; }
	const Pnt &Origin() const { return ori; }
	void OriMove(int dx,int dy) { ori.Move(dx,dy); }
	int OriX() const { return ori.X(); }
	int OriY() const { return ori.Y(); }
*/

	virtual GuiSingle *Find(const t_symbol *s) { return NULL; }
	inline GuiSingle *Find(const char *s) { return Find(MakeSymbol(s)); }

	virtual GuiObj &MoveRel(int dx,int dy) = 0;
	virtual GuiObj &Focus() { return *this; }

	virtual GuiObj &Draw() = 0;
	
	Canvas &Canv() { return *canvas; }

protected:
	virtual GuiObj &Delete() = 0;

	GuiGroup *parent;
	Canvas *canvas;
	const t_symbol *idsym;

	virtual bool Method(flext_gui &g,const flext_gui::CBParams &p) = 0;

	Rect rect;
};


class GuiSingle:
	public GuiObj
{
	friend class flext_gui;
public:
	GuiSingle(Canvas *c = NULL,GuiGroup *p = NULL,const t_symbol *s = NULL);
	~GuiSingle();

	virtual const t_symbol *Symbol() const { return sym; }
	virtual void Symbol(const t_symbol *s);
	void Symbol(const char *s) { Symbol(MakeSymbol(s)); }

	virtual void Active() { active = true; }
	virtual void Inactive() { active = false; }

	virtual bool In(const Pnt &p) const { return false; }

	virtual GuiSingle *Find(const t_symbol *s);
	virtual GuiObj &MoveTo(int x,int y);
	virtual GuiObj &MoveRel(int dx,int dy);
	virtual GuiObj &FillColor(unsigned long col);
	virtual GuiObj &Outline(unsigned long col);

	virtual GuiObj &Focus();

	GuiGroup &Parent() { return *parent; }

protected:
	virtual GuiObj &Delete();

	const t_symbol *sym;
	bool active;

	class Event {
	public:
		Event(int evmask,bool (*m)(flext_gui &g,GuiSingle &obj,const flext_gui::CBParams &p));
		~Event();

		int methfl;
		bool (*method)(flext_gui &g,GuiSingle &obj,const flext_gui::CBParams &p);
		bool ext;
		Event *nxt;
	} *event;

	void AddEvent(int evmask,bool (*m)(flext_gui &g,GuiSingle &obj,const flext_gui::CBParams &p));
	void RmvEvent(int evmask,bool (*m)(flext_gui &g,GuiSingle &obj,const flext_gui::CBParams &p));

	virtual bool Method(flext_gui &g,const flext_gui::CBParams &p);
};


class GuiPoint:
	public GuiSingle
{
	friend class GuiGroup;

	GuiPoint(Canvas *c = NULL,GuiGroup *p = NULL,const t_symbol *s = NULL): GuiSingle(c,p,s) {}
	GuiObj &Set(int x,int y,long fill = -1);
	virtual GuiObj &Draw();
	
	long fill;
public:
};


class GuiCloud:
	public GuiSingle
{
	friend class GuiGroup;

	GuiCloud(Canvas *c = NULL,GuiGroup *p = NULL,const t_symbol *s = NULL): GuiSingle(c,p,s),pnt(NULL) {}
	GuiObj &Set(int n,const Pnt *p,long fill = -1);
	virtual GuiObj &Draw();
	virtual GuiObj &Delete();
	
	long fill;
	int pnts;
	Pnt *pnt;
public:
};


class GuiBox:
	public GuiSingle
{
	friend class GuiGroup;

	GuiBox(Canvas *c = NULL,GuiGroup *p = NULL,const t_symbol *s = NULL): GuiSingle(c,p,s) {}
	GuiObj &Set(int x,int y,int xsz,int ysz,int width = -1,long fill = -1,long outl = -1);
	virtual GuiObj &Draw();
	
	virtual bool In(const Pnt &p) const { return rect.In(p); }

	int width;
	long fill,outln;
public:
};


class GuiRect:
	public GuiSingle
{
	friend class GuiGroup;

	GuiRect(Canvas *c = NULL,GuiGroup *p = NULL,const t_symbol *s = NULL): GuiSingle(c,p,s) {}
	GuiObj &Set(int x,int y,int xsz,int ysz,int width = -1,long outl = -1);
	virtual GuiObj &Draw();
	
	virtual bool In(const Pnt &p) const { return rect.In(p); }

	int width;
	long outln;
public:
};


class GuiLine:
	public GuiSingle
{
	friend class GuiGroup;

	GuiLine(Canvas *c = NULL,GuiGroup *p = NULL,const t_symbol *s = NULL): GuiSingle(c,p,s) {}
	GuiObj &Set(int x1,int y1,int x2,int y2,int width = -1,long fill = -1);
	virtual GuiObj &Draw();
	
	int width;
	long fill;
	Pnt p1,p2;
public:
};


class GuiPoly:
	public GuiSingle
{
	friend class GuiGroup;

	GuiPoly(Canvas *c = NULL,GuiGroup *p = NULL,const t_symbol *s = NULL): GuiSingle(c,p,s),pnt(NULL) {}
	GuiObj &Set(int n,const Pnt *p,int width = -1,long fill = -1);
	virtual GuiObj &Draw();
	virtual GuiObj &Delete();
	
	int width;
	long fill;
	int pnts;
	Pnt *pnt;
public:
};


class GuiText:
	public GuiSingle
{
	friend class GuiGroup;
public:
	enum just_t { none = -1,left = 0,right,center };
protected:
	GuiText(Canvas *c = NULL,GuiGroup *p = NULL,const t_symbol *s = NULL): GuiSingle(c,p,s) {}
	GuiObj &Set(int x,int y,const char *txt = NULL,long fill = -1,just_t just = none);
	virtual GuiObj &Draw();
	
	just_t just;
	long fill;
};


class GuiGroup:
	public GuiObj
{
	friend class flext_gui;
public:
	GuiGroup(Canvas *c,GuiGroup *p = NULL);
	~GuiGroup();

	virtual GuiSingle *Find(const t_symbol *s);
	virtual GuiObj &MoveRel(int dx,int dy);

	void Clear();
	void Add(GuiObj *o,bool own = true);
	GuiSingle *Detach(const t_symbol *s);

	virtual GuiObj &Draw();

	GuiGroup *Add_Group();
	GuiSingle *Add_Point(int x,int y,long fill = -1);
	inline GuiSingle *Add_Point(const Pnt &p,long fill = -1) { return Add_Point(p.X(),p.Y(),fill); }
	GuiSingle *Add_Cloud(int n,const Pnt *p,long fill = -1);
	GuiSingle *Add_Box(int x,int y,int xsz,int ysz,int width = -1,long fill = -1,long outl = -1);
	GuiSingle *Add_Rect(int x,int y,int xsz,int ysz,int width = -1,long outl = -1);
	GuiSingle *Add_Line(int x1,int y1,int x2,int y2,int width = -1,long fill = -1);
	inline GuiSingle *Add_Line(const Pnt &p1,const Pnt &p2,int width = -1,long fill = -1) { return Add_Line(p1.X(),p1.Y(),p2.X(),p2.Y(),width,fill); }
	GuiSingle *Add_Poly(int n,const Pnt *p,int width = -1,long fill = -1);
	GuiSingle *Add_Text(int x,int y,const char *txt,long fill = -1,GuiText::just_t just = GuiText::none);
	inline GuiSingle *Add_Text(const Pnt &p,const char *txt,long fill = -1,GuiText::just_t just = GuiText::none) { return Add_Text(p.X(),p.Y(),txt,fill,just); }

	GuiSingle *Remove(GuiSingle *obj);

protected:
#if FLEXT_SYS == FLEXT_SYS_PD
	void AddTag(GuiObj *o);
	void RemoveTag(GuiObj *o);
#endif
	
	virtual GuiObj &Delete();

	class Part
	{
	public:
		Part(GuiObj *o,bool own = true): obj(o),owner(own),nxt(NULL) {}

		GuiObj *obj;
		bool owner;
		Part *nxt;
	} *head,*tail;

	virtual bool Method(flext_gui &g,const flext_gui::CBParams &p);
};

#endif
