#include "flguiobj.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>


#define USETAGS

#define BUFSIZE 20000
#define ZONE 1000


#if FLEXT_SYS == FLEXT_SYS_PD
bool FCanvas::store = true;
bool FCanvas::debug = false;
#endif

FCanvas::FCanvas(t_canvas *c): 
	canvas(c)
#if FLEXT_SYS == FLEXT_SYS_PD
	,buffer(new char[BUFSIZE]),bufix(0),waiting(0) 
#endif
{}

FCanvas::~FCanvas() 
{ 
#if FLEXT_SYS == FLEXT_SYS_PD
	if(buffer) delete[] buffer; 
#endif
}

#if FLEXT_SYS == FLEXT_SYS_PD
void FCanvas::Send(const char *t)
{
	if(debug) post("GUI - %s",t);
	sys_gui((char *)t);
}

void FCanvas::SendBuf()
{
	if(bufix) {
		Send(buffer);
		bufix = 0;
	}
}

void FCanvas::ToBuf(const char *t)
{
	int len = strlen(t);
	if(!len) return;

	bool end = t[len-1] == '\n';
	if((store && waiting) || !end) {
		int nxt = bufix+len;
		if(nxt >= BUFSIZE || (end && nxt >= BUFSIZE-ZONE)) SendBuf();

		memcpy(buffer+bufix,t,len);
		buffer[bufix += len] = 0;
	}
	else {
		SendBuf();
		Send(t);
	}
}

FCanvas &FCanvas::TkC()
{
	char tmp[20];
	sprintf(tmp,".x%x.c ",canvas);
	ToBuf(tmp);
	return *this;
}

FCanvas &FCanvas::TkE()
{
	ToBuf("\n");
	return *this;
}

FCanvas &FCanvas::Tk(char *fmt,...)
{
 //   int result, i;
    char buf[2048];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    ToBuf(buf);
    va_end(ap);
	return *this;
}
#endif

bool FCanvas::Pre(int x,int y) 
{ 
	xpos = x,ypos = y;
#if FLEXT_SYS == FLEXT_SYS_PD
	++waiting;
	return true;
#else
	svgp = patcher_setport(canvas); 
	if(svgp != NULL) {
		GetForeColor(&svcol);
		GetPenState(&svpen);
		::PenMode(patCopy);
		return true;
	}
	else return false; 
#endif
}

void FCanvas::Post() 
{ 
#if FLEXT_SYS == FLEXT_SYS_PD
	if(!--waiting) SendBuf();
#else
	if(svgp) {
		RGBForeColor(&svcol);
		SetPenState(&svpen);
		SetPort(svgp); 
	}
#endif
}


// --------------------------------------------------------------------------


FRect &FRect::Add(const FPnt &p)
{
	if(p.x < lo.x) lo.x = p.x;
	if(p.y < lo.y) lo.y = p.y;
	if(p.x > hi.x) hi.x = p.x;
	if(p.y > hi.y) hi.y = p.y;
	return *this;
}

FRect &FRect::Add(const FRect &r)
{
	if(r.lo.x < lo.x) lo.x = r.lo.x;
	if(r.lo.y < lo.y) lo.y = r.lo.y;
	if(r.hi.x > hi.x) hi.x = r.hi.x;
	if(r.hi.y > hi.y) hi.y = r.hi.y;
	return *this;
}

bool FRect::In(const FPnt &p) const
{
	return p.x >= lo.x && p.x <= hi.x && p.y >= lo.y && p.y <= hi.y;
}

bool FRect::Inter(const FRect &r) const
{
	return true;
}


// --------------------------------------------------------------------------


GuiObj::GuiObj(FCanvas *c,GuiGroup *p): 
	canvas(c),idsym(NULL),
	parent(p)
//	,ori(0,0)
{}

GuiObj::~GuiObj() 
{ 
//	Delete();
}


// --------------------------------------------------------------------------

GuiSingle::Event::Event(int evmask,bool (*m)(flext_gui &g,GuiSingle &obj,const flext_gui::CBParams &p)):
	methfl(evmask),method(m),ext(false),nxt(NULL) 
{}

GuiSingle::Event::~Event() { if(nxt) delete nxt; }


GuiSingle::GuiSingle(FCanvas *c,GuiGroup *p,const t_symbol *s): 
	GuiObj(c,p),sym(s),active(false),event(NULL)
{
	char tmp[20];
#ifdef __MWERKS__
	std::
#endif
		sprintf(tmp,"GUI%x",this);
	idsym = MakeSymbol(tmp);
}

GuiSingle::~GuiSingle() 
{ 
	Delete(); 
	if(event) delete event;
}

void GuiSingle::Symbol(const t_symbol *s) 
{ 
	sym = s;
}

GuiSingle *GuiSingle::Find(const t_symbol *s)
{
	return sym == s?this:NULL;
}

GuiObj &GuiSingle::MoveTo(int x,int y)
{
#if FLEXT_SYS == FLEXT_SYS_PD
    if(active) { 
		canvas->TkC().Tk("coords %s %i %i\n",GetString(Id()), x, y);
	}
#endif
	rect.MoveTo(x,y);
	return *this;
}

GuiObj &GuiSingle::MoveRel(int dx,int dy)
{
#if FLEXT_SYS == FLEXT_SYS_PD
	if(active) {
		canvas->TkC().Tk("move %s %i %i\n",GetString(Id()), dx, dy);
	}
#endif
	rect.Move(dx,dy);
	return *this;
}

GuiObj &GuiSingle::Delete()
{
#if FLEXT_SYS == FLEXT_SYS_PD
	if(active) {
		canvas->TkC().Tk("delete -tags %s\n",GetString(Id()));
		active = false;
	}
#endif
	return *this;
}

GuiObj &GuiSingle::FillColor(unsigned long col)
{
#if FLEXT_SYS == FLEXT_SYS_PD
	if(active) {
		canvas->TkC().Tk("itemconfigure %s -fill #%06x\n",GetString(Id()),col);
	}
#endif
	return *this;
}

GuiObj &GuiSingle::Outline(unsigned long col)
{
#if FLEXT_SYS == FLEXT_SYS_PD
	if(active) {
		canvas->TkC().Tk("itemconfigure %s -outline #%06x\n",GetString(Id()),col);
	}
#endif
	return *this;
}

GuiObj &GuiSingle::Focus()
{
#if FLEXT_SYS == FLEXT_SYS_PD
	if(active) {
		canvas->TkC().Tk("focus %s\n",GetString(Id()));
	}
#endif
	return *this;
}

bool GuiSingle::Method(flext_gui &g,const flext_gui::CBParams &p)
{
	bool ret = true;
	for(Event *ei = event; ei && ret; ei = ei->nxt)
		ret = ret && ((ei->method && (ei->methfl&p.kind))?ei->method(g,*this,p):true);
	return ret;
}

void GuiSingle::AddEvent(int evmask,bool (*m)(flext_gui &g,GuiSingle &obj,const flext_gui::CBParams &p))
{
	Event *prv = NULL,*ix = event;
	for(; ix && ix->method != m; prv = ix,ix = ix->nxt) {}

	if(ix) 
		// previous handler found -> update event mask
		ix->methfl |= evmask;
	else {
		// no previous handler was found -> make new one

		Event *nev = new Event(evmask,m);
		if(prv) prv->nxt = nev;
		else event = nev;
	}
}

void GuiSingle::RmvEvent(int evmask,bool (*m)(flext_gui &g,GuiSingle &obj,const flext_gui::CBParams &p))
{
	Event *prv = NULL,*ix = event;
	for(; ix && ix->method != m; prv = ix,ix = ix->nxt) {}

	if(ix) {
		// handler found

		if(!(ix->methfl &= ~evmask)) {
			// mask has become zero -> remove handler

			if(prv) prv->nxt = ix->nxt;
			else event = ix->nxt;
			ix->nxt = NULL;
			delete ix;
		}
	}
}



// --------------------------------------------------------------------------


GuiGroup::GuiGroup(FCanvas *c,GuiGroup *p): 
	GuiObj(c,p),head(NULL),tail(NULL)
{
	char tmp[20];
#ifdef __MWERKS__
	std::
#endif
		sprintf(tmp,"GRP%x",this);
	idsym = MakeSymbol(tmp);
}

GuiGroup::~GuiGroup() { Clear(); }

void GuiGroup::Clear()
{
	for(Part *ix = head; ix; ) {
		Part *n = ix->nxt;
#if FLEXT_SYS == FLEXT_SYS_PD
		RemoveTag(ix->obj);
#endif
		if(ix->owner) delete ix->obj;
		delete ix;
		ix = n;
	}
	head = tail = NULL;
}

void GuiGroup::Add(GuiObj *o,bool owner)
{
	o->canvas = canvas;
#if FLEXT_SYS == FLEXT_SYS_PD
	AddTag(o); // valid only for GuiSingle!
#endif
	Part *n = new Part(o,owner);
	if(!head) head = n;
	else tail->nxt = n;
	tail = n;
	
	rect.Add(o->rect);
}

GuiSingle *GuiGroup::Find(const t_symbol *s)
{
	GuiSingle *r = NULL;
	for(Part *ix = head; ix && !r; ix = ix->nxt) {
		r = ix->obj->Find(s);
	}
	return r;
}

GuiSingle *GuiGroup::Detach(const t_symbol *s)
{
	if(head) {
		Part *p = NULL,*ix = head;
		while(ix) {
			if(ix->obj->Symbol() == s) {
				// found
				if(p) p->nxt = ix->nxt;
				GuiSingle *ret = (GuiSingle *)(ix->obj);
				
#if FLEXT_SYS == FLEXT_SYS_PD
				RemoveTag(ret);
#endif
				if(head == ix) head = ix->nxt;
				if(tail == ix) tail = p;
				
				delete ix;
				
				return ret;
			}
			else p = ix,ix = ix->nxt;
		}
	}
	return NULL;
}

GuiObj &GuiGroup::MoveRel(int dx,int dy)
{
#if FLEXT_SYS == FLEXT_SYS_PD
	canvas->TkC().Tk("move %s %i %i\n",GetString(Id()), dx, dy);
#endif
	for(Part *ix = head; ix; ix = ix->nxt) {
		ix->obj->rect.Move(dx,dy);
	}
	return *this;
}

GuiObj &GuiGroup::Delete()
{	
#if FLEXT_SYS == FLEXT_SYS_PD
	canvas->TkC().Tk("delete -tags %s\n",GetString(Id()));

	for(Part *ix = head; ix; ix = ix->nxt) ix->obj->Inactive();
#endif
	return *this;
}


GuiObj &GuiGroup::Draw()
{	
	for(Part *ix = head; ix; ix = ix->nxt) ix->obj->Draw();
	return *this;
}


#if FLEXT_SYS == FLEXT_SYS_PD

void GuiGroup::AddTag(GuiObj *o)
{
	canvas->TkC().Tk("addtag %s withtag %s\n",GetString(Id()),GetString(o->Id()));
}

void GuiGroup::RemoveTag(GuiObj *o)
{
	canvas->TkC().Tk("dtag %s %s\n",GetString(o->Id()),GetString(Id()));
}

#endif

GuiGroup *GuiGroup::Add_Group()
{
	GuiGroup *obj = new GuiGroup(canvas,this);
	Add(obj);
	return obj;
}


GuiSingle *GuiGroup::Add_Point(int x,int y,long fill)
{
	GuiPoint *obj = new GuiPoint(canvas,this);
	obj->Set(x,y,fill);
	Add(obj);
	return obj;
}

GuiSingle *GuiGroup::Add_Cloud(int n,const FPnt *p,long fill)
{
	GuiCloud *obj = new GuiCloud(canvas,this);
	obj->Set(n,p,fill);
	Add(obj);
	return obj;
}

GuiSingle *GuiGroup::Add_Box(int x,int y,int xsz,int ysz,int width,long fill,long outl)
{
	GuiBox *obj = new GuiBox(canvas,this);
	obj->Set(x,y,xsz,ysz,width,fill,outl);
	Add(obj);
	return obj;
}

GuiSingle *GuiGroup::Add_Rect(int x,int y,int xsz,int ysz,int width,long outl)
{
	GuiRect *obj = new GuiRect(canvas,this);
	obj->Set(x,y,xsz,ysz,width,outl);
	Add(obj);
	return obj;
}

GuiSingle *GuiGroup::Add_Line(int x1,int y1,int x2,int y2,int width,long fill)
{
	GuiLine *obj = new GuiLine(canvas,this);
	obj->Set(x1,y1,x2,y2,width,fill);
	Add(obj);
	return obj;
}

GuiSingle *GuiGroup::Add_Poly(int n,const FPnt *p,int width,long fill)
{
	GuiPoly *obj = new GuiPoly(canvas,this);
	obj->Set(n,p,width,fill);
	Add(obj);
	return obj;
}

GuiSingle *GuiGroup::Add_Text(int x,int y,const char *txt,long fill,GuiText::just_t just)
{
	GuiText *obj = new GuiText(canvas,this);
	obj->Set(x,y,txt,fill,just);
	Add(obj);
	return obj;
}

GuiSingle *GuiGroup::Remove(GuiSingle *go)
{
	for(Part *prv = NULL,*ix = head; ix; prv = ix,ix = ix->nxt) 
		if(ix->obj == go) {
			if(prv) prv->nxt = ix->nxt;
			else head = ix->nxt;
			if(!head) tail = NULL;

			GuiObj *ret = ix->obj;
			delete ix;

			if(head) {
				rect = head->obj->rect;
				for(ix = head->nxt; ix; ix = ix->nxt) rect.Add(ix->obj->rect);
			}

			return (GuiSingle *)ret;
		}

	return NULL;
}

bool GuiGroup::Method(flext_gui &g,const flext_gui::CBParams &p)
{
	bool go = true;
	for(Part *ix = head; go && ix; ix = ix->nxt) go = go && ix->obj->Method(g,p);
	return go;
}


// --------------------------------------------------------------------------


GuiObj &GuiPoint::Set(int x,int y,long fl)
{
	Delete();
	fill = fl;
	rect(x,y,x,y);

#if FLEXT_SYS == FLEXT_SYS_PD
    canvas->TkC();
	canvas->Tk("create line %i %i %i %i -tags %s",x,y,x+1,y,GetString(Id()));
	if(fill >= 0) canvas->Tk(" -fill #%06x",fill);
	canvas->TkE();

	active = true;
#endif
	return *this;
}

GuiObj &GuiPoint::Draw()
{
#if FLEXT_SYS == FLEXT_SYS_MAX
#endif
	return *this;
}


GuiObj &GuiCloud::Set(int n,const FPnt *p,long fl)
{
	int i;
	Delete();

	fill = fl;
	pnt = new FPnt[pnts = n];
	rect(pnt[0] = p[0],p[0]);
	for(i = 1; i < n; ++i) rect.Add(pnt[i] = p[i]);

#if FLEXT_SYS == FLEXT_SYS_PD
    canvas->TkC().Tk("create line");
	for(i = 0; i < n; ++i)
		canvas->Tk(" %i %i",p[i].X(),p[i].Y());
	canvas->Tk(" -tags %s",GetString(Id()));
	if(fill >= 0) canvas->Tk(" -fill #%06x",fill);
	canvas->TkE();

	active = true;
#endif
	return *this;
}

GuiObj &GuiCloud::Draw()
{
#if FLEXT_SYS == FLEXT_SYS_MAX
#endif
	return *this;
}

GuiObj &GuiCloud::Delete()
{
	if(pnt) { delete[] pnt; pnt = NULL; }
	return *this;
}


GuiObj &GuiBox::Set(int x,int y,int xsz,int ysz,int wd,long fl,long outl)
{
	Delete();
	rect(x,y,x+xsz-1,y+ysz-1);
	width = wd,fill = fl,outln = outl;

#if FLEXT_SYS == FLEXT_SYS_PD
    canvas->TkC().Tk("create rectangle %i %i %i %i -tags %s",x,y,x+xsz,y+ysz,GetString(Id()));
	if(wd >= 0) canvas->Tk(" -width %i",wd);
	if(fl >= 0) canvas->Tk(" -fill #%06x",fl);
	if(outl >= 0) canvas->Tk(" -outline #%06x",outl);
	canvas->TkE();

	active = true;
#endif
	return *this;
}

GuiObj &GuiBox::Draw()
{
#if FLEXT_SYS == FLEXT_SYS_MAX
	::Rect r;
	::RGBColor col;
	int w = width; 
	if(!w) w = 1;

	r.left = Canv().X()+rect.Lo().X();
	r.top = Canv().Y()+rect.Lo().Y();
	r.right = Canv().X()+rect.Hi().X();
	r.bottom = Canv().Y()+rect.Hi().Y();
	
	if(width >= 0) {
		col.red = (outln>>8)&0xff00;
		col.green = outln&0xff00;
		col.blue = (outln&0xff)<<8;
		::RGBForeColor(&col);
		::PenSize(w,w);
		::FrameRect(&r);
	}
	col.red = (fill>>8)&0xff00;
	col.green = fill&0xff00;
	col.blue = (fill&0xff)<<8;
	::RGBForeColor(&col);
	::PaintRect(&r);
#endif
	return *this;
}


GuiObj &GuiRect::Set(int x,int y,int xsz,int ysz,int wd,long outl)
{
	Delete();
	rect(x,y,x+xsz-1,y+ysz-1);
	width = wd,outln = outl;

#if FLEXT_SYS == FLEXT_SYS_PD
    canvas->TkC().Tk("create line %i %i %i %i %i %i %i %i %i %i -tags %s",x,y,x+xsz,y,x+xsz,y+ysz,x,y+ysz,x,y,GetString(Id()));
	if(width >= 0) canvas->Tk(" -width %i",width);
	if(outl >= 0) canvas->Tk(" -fill #%06x",outl);
	canvas->TkE();

	active = true;
#endif
	return *this;
}

GuiObj &GuiRect::Draw()
{
#if FLEXT_SYS == FLEXT_SYS_MAX
#endif
	return *this;
}


GuiObj &GuiLine::Set(int x1,int y1,int x2,int y2,int wd,long fl)
{
	Delete();
	rect(p1(x1,y1),p2(x2,y2));
	width = wd,fill = fl;
	
#if FLEXT_SYS == FLEXT_SYS_PD
	canvas->TkC().Tk("create line %i %i %i %i -tags %s",x1,y1,x2,y2,GetString(Id()));
	if(width >= 0) canvas->Tk(" -width %i",width);
	if(fill >= 0) canvas->Tk(" -fill #%06x",fill);
	canvas->TkE();

	active = true;
#endif
	return *this;
}

GuiObj &GuiLine::Draw()
{
#if FLEXT_SYS == FLEXT_SYS_MAX
	::Point p;
	::RGBColor col;
	int w = width; 
	if(!w) w = 1;
	
	col.red = (fill>>8)&0xff00;
	col.green = fill&0xff00;
	col.blue = (fill&0xff)<<8;
	::RGBForeColor(&col);
	::PenSize(w,w);
	p.h = Canv().X()+p1.X();
	p.v = Canv().Y()+p1.Y();
	::MoveTo(p.h,p.v);
	p.h = Canv().X()+p2.X();
	p.v = Canv().Y()+p2.Y();
	::LineTo(p.h,p.v);
#endif
	return *this;
}


GuiObj &GuiPoly::Set(int n,const FPnt *p,int wd,long fl)
{
	int i;

	Delete();

	width = wd,fill = fl;
	pnt = new FPnt[pnts = n];
	rect(pnt[0] = p[0],p[0]);
	for(i = 1; i < n; ++i) rect.Add(pnt[i] = p[i]);

#if FLEXT_SYS == FLEXT_SYS_PD
	canvas->TkC().Tk("create line");
	for(i = 0; i < n; ++i)
		canvas->Tk(" %i %i",p[i].X(),p[i].Y());
	canvas->Tk(" -tags %s",GetString(Id()));
	if(width >= 0) canvas->Tk(" -width %i",width);
	if(fill >= 0) canvas->Tk(" -fill #%06x",fill);
	canvas->TkE();

	active = true;
#endif
	return *this;
}

GuiObj &GuiPoly::Draw()
{
#if FLEXT_SYS == FLEXT_SYS_MAX
	::Point p;
	::RGBColor col;
	int ox = Canv().X(),oy = Canv().Y();
	int w = width; 
	if(!w) w = 1;
	
	col.red = (fill>>8)&0xff00;
	col.green = fill&0xff00;
	col.blue = (fill&0xff)<<8;
	::RGBForeColor(&col);
	::PenSize(w,w);
	p.h = ox+pnt[0].X();
	p.v = oy+pnt[0].Y();
	::MoveTo(p.h,p.v);
	for(int i = 1; i < pnts; ++i) {
		p.h = ox+pnt[i].X();
		p.v = oy+pnt[i].Y();
		::LineTo(p.h,p.v);
	}
#endif
	return *this;
}

GuiObj &GuiPoly::Delete()
{
	if(pnt) { delete[] pnt; pnt = NULL; }
	return *this;
}


GuiObj &GuiText::Set(int x,int y,const char *txt,long fl,just_t jt)
{
	Delete();
	rect(x,y,x,y);
	fill = fl,just = jt;

#if FLEXT_SYS == FLEXT_SYS_PD
	canvas->TkC().Tk("create text %i %i -tags %s",x,y,GetString(Id()));
	if(txt) canvas->Tk(" -text %s",txt);
	if(fill >= 0) canvas->Tk(" -fill #%06x",fill);
	if(just != none) {
		static const char justtxt[][7] = {"left","right","center"};
		canvas->Tk(" -justify %s",justtxt[(int)just]);
	}
	canvas->TkE();

	active = true;
#endif
	return *this;
}

GuiObj &GuiText::Draw()
{
#if FLEXT_SYS == FLEXT_SYS_MAX
#endif
	return *this;
}

