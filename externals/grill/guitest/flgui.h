#ifndef __FLEXT_GUI
#define __FLEXT_GUI

//#define FLEXT_VIRT
#include <flext.h>

#if FLEXT_SYS == FLEXT_SYS_PD
#pragma warning( disable : 4091 ) 
#include <g_canvas.h>
#endif

class FCanvas;
class GuiObj;
class GuiGroup;
class GuiSingle;

class flext_gui:
	public flext_base
{
	FLEXT_HEADER_S(flext_gui,flext_dsp,setup)
 
public:
	flext_gui(int xs,int ys);
	~flext_gui();

	enum CBEvs { 
		evNone = 0,
		evMotion = 0x01,
		evMouseDown = 0x02,
		evMouseUp = 0x04,
		evMouseWheel = 0x08,
		evMouseDrag = 0x10,
		evKeyDown = 0x20, 
		evKeyUp = 0x40,
		evKeyRepeat = 0x80
	};

	class CBParams {
	public:
		CBParams(CBEvs k = evNone): kind(k),ext(false) {}

		CBEvs kind;
		union {
			struct { int x,y,mod; } pMotion;
			struct { int x,y,b,mod; } pMouseKey;
			struct { int x,y,mod,delta; } pMouseWheel;
			struct { int x,y,dx,dy,b,mod; } pMouseDrag;
			struct { int k,a,mod; } pKey;
		};
		bool ext;
	};

	static int EventMask() { return evmask; }

	bool BindEvent(GuiSingle &o,bool (*cb)(flext_gui &o,GuiSingle &obj,const CBParams &p),int evs);
	void UnbindEvent(GuiSingle &o,bool (*cb)(flext_gui &o,GuiSingle &obj,const CBParams &p),int evs);

protected:

	virtual void g_Create() {}
	virtual void g_Delete();
//	virtual void g_GetRect(int &xp1,int &yp1,int &xp2,int &yp2);
	virtual void g_Edit(bool selected) {}
	virtual void g_Displace(int dx, int dy);
//	virtual void g_Activate(bool state) {}
//	virtual int g_Click(int xpix, int ypix, int shift, int alt, int dbl, int doit) { return 0; }
	virtual void g_Properties();
	virtual void g_Save(t_binbuf *b) {}
/*
	virtual bool g_Motion(GuiObj &obj,int x,int y,int mod) { return false; }
	virtual bool g_MouseKey(GuiObj &obj,bool down,int x,int y,int b,int mod) { return false; }
	virtual bool g_MouseWheel(GuiObj &obj,int x,int y,int d,int mod) { return false; }
	virtual bool g_Key(GuiObj &obj,bool down,int k,int a,int n,int mod) { return false; }
	virtual bool g_Region(GuiObj &obj,bool on,int mod) { return false; }
	virtual bool g_Focus(GuiObj &obj,bool on,int mod) { return false; }
*/	

#if FLEXT_SYS == FLEXT_SYS_PD
	bool Selected() const { return selected; }

	void FixLines() { canvas_fixlinesfor( thisCanvas(), thisHdr() ); }
    void DelLines() { canvas_deletelinesfor( glist_getcanvas(thisCanvas()), (t_text *)thisHdr()); }

 	int XLo() const { return thisHdr()->te_xpix; }
	int YLo() const { return thisHdr()->te_ypix; }
 	int XHi() const { return XLo()+XSize()-1; }
	int YHi() const { return YLo()+YSize()-1; }
	void XLo(int x) { thisHdr()->te_xpix = x; }
	void YLo(int y) { thisHdr()->te_ypix = y; }

 	int XSize() const { return xsize; }
	int YSize() const { return ysize; }
#else // MAXMSP
	bool Selected() const { return box_ownerlocked((t_box *)(&thisHdr()->z_box)) == 0; }

	void FixLines() {}
    void DelLines() {}

	int XLo() const { return thisHdr()->z_box.b_rect.left; } 
	int YLo() const { return thisHdr()->z_box.b_rect.top; } 
	int XHi() const { return thisHdr()->z_box.b_rect.right; } 
	int YHi() const { return thisHdr()->z_box.b_rect.bottom; } 
	void XLo(int x) { thisHdr()->z_box.b_rect.left = x; }
	void YLo(int y) { thisHdr()->z_box.b_rect.top = y; }

 	int XSize() const { return XHi()-XLo()+1; }
	int YSize() const { return YHi()-YLo()+1; }
#endif

	const t_symbol *Id() const { return bindsym; }

	GuiGroup &Group() { return *objs; }


//	static void Setup(t_class *c);


	enum Modifier {
		mod_None = 0,
		mod_Ctrl = 0x0001,
		mod_Shift = 0x0002,
		mod_Alt = 0x0004,
		mod_Meta = 0x0008,
		mod_Mod1 = 0x0010,
		mod_Mod2 = 0x0020,
		mod_Mod3 = 0x0040,
		mod_Caps = 0x0080,
		mod_Double = 0x0100,
//		mod_Triple = 0x0200,
		mod_Button1 = 0x1000,
		mod_Button2 = 0x2000,
		mod_Button3 = 0x4000,
		mod_Button4 = 0x8000
	};

private:
	bool visible;
	FCanvas *canvas;
	GuiGroup *objs;

	const t_symbol *bindsym;

	static int evmask;
	
	static void setup(t_class *);
	
	virtual void m_Method(const CBParams &p);

#if FLEXT_SYS == FLEXT_SYS_PD
	bool selected;
	int xsize,ysize;
	int xdrag,ydrag,dxdrag,dydrag;

	static void sg_getrect(t_gobj *c, t_glist *,int *xp1, int *yp1, int *xp2, int *yp2);
	static void sg_displace(t_gobj *c, t_glist *, int dx, int dy);
	static void sg_select(t_gobj *c, t_glist *, int selected);
//	static void sg_activate(t_gobj *c, t_glist *, int state) { thisObject(c)->g_Activate(state != 0); }
	static void sg_delete(t_gobj *c, t_glist *);
	static void sg_vis(t_gobj *c, t_glist *, int vis); 
	static int sg_click(t_gobj *c, t_glist *,int xpix, int ypix, int shift, int alt, int dbl, int doit);
	static void sg_drag(t_gobj *x, t_floatarg dx, t_floatarg dy);
	static void sg_properties(t_gobj *c, t_glist *);
	static void sg_save(t_gobj *c, t_binbuf *b);
//	static void sg_motion(void *c, float dx,float dy) { thisObject((t_gobj *)c)->g_Motion(dx,dy); }

	static bool sg_Key(flext_base *c,int argc,t_atom *argv);
	static bool sg_KeyNum(flext_base *c,int &keynum);
	static bool sg_KeyUp(flext_base *c,int &keynum);

	static t_widgetbehavior widgetbehavior;
	static void SetWidget(t_class *c);

	struct px_object;
	struct pxkey_object;
	static int curmod; //,lastkey;
	static pxkey_object *pxkey;

	class canvobj {
	public:
		canvobj(flext_gui *go): guiobj(go),nxt(NULL) {}

		flext_gui *guiobj;
		canvobj *nxt;
	};

	class guicanv {
	public:
		guicanv(t_canvas *c);
		~guicanv();

		int Refs() const { return ref; }
		void Push(flext_gui *o);
		void Pop(flext_gui *o);

#ifdef DIRECT_TK
		px_object *px;
#endif
		const t_symbol *sym;
		t_canvas *canv;
		guicanv *nxt;
		canvobj *head,*tail;
		int ref;
	};

	static guicanv *gcanv;
	static const t_symbol *gcm_motion,*gcm_mousekey,*gcm_mousewheel,*gcm_key,*gcm_destroy;
	static void px_method(px_object *c,const t_symbol *s,int argc,t_atom *argv);
	static void pxkey_method(pxkey_object *c,const t_symbol *s,int argc,t_atom *argv);

	void AddCanvas();
	void RmvCanvas();
	static void DelCanvas(t_canvas *c);

public:
	static t_class *px_class,*pxkey_class;
	static void sg_tk(t_canvas *c,const t_symbol *s,int argc,t_atom *argv);

#else // MAXMSP
	int curx,cury,curmod;
	bool created;
	static t_clock clock;
	static t_qelem qelem;
	
	void Update();

	static void sg_click(t_object *x, Point pt, short modifiers);
	static void sg_update(t_object *x);
	static void sg_psave (t_object *x, t_binbuf *dest);
	static void sg_bfont (t_object *x, short size, short font);
	static void sg_key (t_object *x,short keyvalue);
	static void sg_enter (t_object *x);
	static void sg_clipregion (t_object *x, RgnHandle *rgn, short *result);
	static void sg_bidle(t_object *x);
#endif
};

#endif
