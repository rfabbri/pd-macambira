#include "flgui.h"
#include "flguiobj.h"
#include "flinternal.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#if FLEXT_SYS == FLEXT_SYS_PD
t_class *flext_gui::px_class = NULL;
t_class *flext_gui::pxkey_class = NULL;

struct flext_gui::px_object  // no virtual table!
{ 
	t_object obj;			// MUST reside at memory offset 0
	t_canvas *canv;

	void init(t_canvas *c) { canv = c; }
};

struct flext_gui::pxkey_object  // no virtual table!
{ 
	t_object obj;			// MUST reside at memory offset 0
	flext_gui *th;

	void init(flext_gui *t) { th = t; }
};
#endif


flext_gui::flext_gui(int xs,int ys):
	objs(NULL),
#if FLEXT_SYS == FLEXT_SYS_PD
	xsize(xs),ysize(ys),
#endif
#if FLEXT_SYS == FLEXT_SYS_MAX
	curx(-1),cury(-1),curmod(-1),
	created(false),
#endif
	bindsym(NULL)
{
	canvas = new FCanvas(thisCanvas());
	objs = new GuiGroup(canvas);

#if FLEXT_SYS == FLEXT_SYS_PD
	AddCanvas();
#else
	t_box *b = (t_box *)gensym("#B")->s_thing;
	
	int x = b->b_rect.left,y = b->b_rect.top;
	t_pxbox *p = thisHdr();	
	box_new(&p->z_box, thisCanvas(), F_DRAWFIRSTIN | F_GROWBOTH | F_SAVVY,x,y,x+xs,y+ys);
	p->z_box.b_firstin = (void *)p;  /* it's not really an inlet */
	box_ready(&p->z_box);
#endif
}

flext_gui::~flext_gui()
{
#if FLEXT_SYS == FLEXT_SYS_PD
	RmvCanvas();
#endif

	delete objs;
	delete canvas;
}


void flext_gui::setup(t_classid c)
{
#if FLEXT_SYS == FLEXT_SYS_PD
	SetWidget(c);

    pxkey_class = class_new(gensym("flext_gui key proxy"),NULL,NULL,sizeof(pxkey_object),CLASS_PD|CLASS_NOINLET, A_NULL);
	add_anything(pxkey_class,pxkey_method); 
    pxkey = (pxkey_object *)pd_new(pxkey_class);

	pd_bind(&pxkey_class,gensym("#keyname"));
//	pd_bind(&pxkey_class,gensym("#key"));
//	pd_bind(&pxkey_class,gensym("#keyup"));

	gcanv = NULL;

#ifdef DIRECT_TK
    px_class = class_new(gensym("flext_gui proxy"),NULL,NULL,sizeof(px_object),CLASS_PD|CLASS_NOINLET, A_NULL);
	add_anything(px_class,px_method); 

	gcm_motion = MakeSymbol("_tk_motion");
	gcm_mousekey = MakeSymbol("_tk_mousekey");
	gcm_mousewheel = MakeSymbol("_tk_mousewheel");
	gcm_key = MakeSymbol("_tk_key");
	gcm_destroy = MakeSymbol("_tk_destroy");
#endif

	// this is wrong if a modifier key is pressed during creation of the first object.....
	curmod = 0;



	sys_gui(
		"proc flgui_apply {id} {\n"
			// strip "." from the TK id to make a variable name suffix
			"set vid [string trimleft $id .]\n"

			// for each variable, make a local variable to hold its name...
			"set var_graph_width [concat graph_width_$vid]\n"
			"global $var_graph_width\n"
			"set var_graph_height [concat graph_height_$vid]\n"
			"global $var_graph_height\n"
			"set var_graph_draw [concat graph_draw_$vid]\n"
			"global $var_graph_draw\n"

			"set cmd [concat $id dialog [eval concat $$var_graph_width] [eval concat $$var_graph_height] [eval concat $$var_graph_draw] \\;]\n"
			// puts stderr $cmd
			"pd $cmd\n"
		"}\n"

		"proc flgui_cancel {id} {\n"
			"set cmd [concat $id cancel \\;]\n"
			// puts stderr $cmd
			"pd $cmd\n"
		"}\n"

		"proc flgui_ok {id} {\n"
			"flgui_apply $id\n"
			"flgui_cancel $id\n"
		"}\n"

		"proc pdtk_flgui_dialog {id width height draw} {\n"
				"set vid [string trimleft $id .]\n"

				"set var_graph_width [concat graph_width_$vid]\n"
				"global $var_graph_width\n"
				"set var_graph_height [concat graph_height_$vid]\n"
				"global $var_graph_height\n"
				"set var_graph_draw [concat graph_draw_$vid]\n"
				"global $var_graph_draw\n"

				"set $var_graph_width $width\n"
				"set $var_graph_height $height\n"
				"set $var_graph_draw $draw\n"

				"toplevel $id\n"
				"wm title $id {flext}\n"
				"wm protocol $id WM_DELETE_WINDOW [concat flgui_cancel $id]\n"

				"label $id.label -text {Attributes}\n"
				"pack $id.label -side top\n"

				"frame $id.buttonframe\n"
				"pack $id.buttonframe -side bottom -fill x -pady 2m\n"

				"button $id.buttonframe.cancel -text {Cancel} -command \"flgui_cancel $id\"\n"
				"button $id.buttonframe.apply -text {Apply} -command \"flgui_apply $id\"\n"
				"button $id.buttonframe.ok -text {OK} -command \"flgui_ok $id\"\n"

				"pack $id.buttonframe.cancel -side left -expand 1\n"
				"pack $id.buttonframe.apply -side left -expand 1\n"
				"pack $id.buttonframe.ok -side left -expand 1\n"

				"frame $id.1rangef\n"
				"pack $id.1rangef -side top\n"
				"label $id.1rangef.lwidth -text \"Width :\"\n"
				"entry $id.1rangef.width -textvariable $var_graph_width -width 7\n"
				"pack $id.1rangef.lwidth $id.1rangef.width -side left\n"

				"frame $id.2rangef\n"
				"pack $id.2rangef -side top\n"
				"label $id.2rangef.lheight -text \"Height :\"\n"
				"entry $id.2rangef.height -textvariable $var_graph_height -width 7\n"
				"pack $id.2rangef.lheight $id.2rangef.height -side left\n"

				"checkbutton $id.draw -text {Draw Sample} -variable $var_graph_draw -anchor w\n"
				"pack $id.draw -side top\n"

				"bind $id.1rangef.width <KeyPress-Return> [concat flgui_ok $id]\n"
				"bind $id.2rangef.height <KeyPress-Return> [concat flgui_ok $id]\n"
				"focus $id.1rangef.width\n"
		"}\n"
	);

#else
	addmess((method)sg_update, "update", A_CANT, A_NULL);
	addmess((method)sg_click, "click", A_CANT, A_NULL);
	addmess((method)sg_psave, "psave", A_CANT, A_NULL);
	addmess((method)sg_bfont, "bfont", A_CANT, A_NULL);
	addmess((method)sg_key, "key", A_CANT, A_NULL);
	addmess((method)sg_bidle, "bidle", A_CANT, A_NULL);
#endif
}

#if FLEXT_SYS == FLEXT_SYS_PD

// this event mask declares supported events
int flext_gui::evmask = evMotion|evMouseDown|evMouseDrag|evKeyDown|evKeyUp|evKeyRepeat;
int flext_gui::curmod = 0;
flext_gui::pxkey_object *flext_gui::pxkey = NULL;
flext_gui::guicanv *flext_gui::gcanv = NULL;

#ifdef DIRECT_TK
const t_symbol *flext_gui::gcm_motion = NULL;
const t_symbol *flext_gui::gcm_mousekey = NULL;
const t_symbol *flext_gui::gcm_mousewheel = NULL;
const t_symbol *flext_gui::gcm_key = NULL;
const t_symbol *flext_gui::gcm_destroy = NULL;

void flext_gui::px_method(px_object *obj,const t_symbol *s,int argc,t_atom *argv)
{
	guicanv *ix = gcanv;
	for(; ix && ix->canv != obj->canv; ix = ix->nxt);

	if(ix) {
		CBParams parms;

		if(s == gcm_motion) {
			parms.kind = evMotion;
			parms.pMotion.x = GetAInt(argv[0]);
			parms.pMotion.y = GetAInt(argv[1]);
			parms.pMotion.mod = GetAInt(argv[2]);
		}
		else if(s == gcm_mousekey) {
			parms.kind = GetAInt(argv[0])?evMouseDown:evMouseUp;
			parms.pMouseKey.x = GetAInt(argv[1]);
			parms.pMouseKey.y = GetAInt(argv[2]);
			parms.pMouseKey.b = GetAInt(argv[3]);
			parms.pMouseKey.mod = GetAInt(argv[4]);
		}
		else if(s == gcm_mousewheel) {
			parms.kind = evMouseWheel;
			parms.pMouseWheel.x = GetAInt(argv[0]);
			parms.pMouseWheel.y = GetAInt(argv[1]);
			parms.pMouseWheel.mod = GetAInt(argv[2]);
			parms.pMouseWheel.delta = GetAInt(argv[3]);
		}
		else if(s == gcm_key) {
			parms.kind = GetAInt(argv[0])?evKeyDown:evKeyUp;
			parms.pKey.k = GetAInt(argv[1]);
			parms.pKey.a = GetAInt(argv[2]);
//			parms.pKey.n = GetAInt(argv[3]);
			parms.pKey.mod = GetAInt(argv[4]);
		}
		else if(s == gcm_destroy) {
//			post("TK destroy");
			DelCanvas(ix->canv);
		}

		if(parms.kind != evNone) {
			for(canvobj *co = ix->head; co; co = co->nxt) 
				co->guiobj->m_Method(parms);
		}

	}
	else 
		error("flext_gui: canvas not found!");
}

#endif

static const char *extkeys[] = {
	"Escape","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12",
	"Prior","Next","Home","End","Delete","Insert",""
};

void flext_gui::pxkey_method(pxkey_object *obj,const t_symbol *s,int argc,t_atom *argv)
{
/*
	if(s == sym_float && argc == 1) {
		lastkey = GetInt(argv[0]);
	}
	else 
*/
	if(s == sym_list && argc == 2) {
		CBParams p;

		bool down = GetABool(argv[0]);
		const char *str = GetString(argv[1]);
		int code = str[0];
		int asc = code;
		int mod = mod_None;
		if(code && str[1] != 0) { 
			code = asc = 0; 
			if(GetSymbol(argv[1]) == MakeSymbol("Shift_L") || GetSymbol(argv[1]) == MakeSymbol("Shift_R")) {
				code = 2001;
				mod = mod_Shift;
			}
			else if(GetSymbol(argv[1]) == MakeSymbol("Control_L") || GetSymbol(argv[1]) == MakeSymbol("Control_R")) {
				code = 2002;
				mod = mod_Ctrl;
			}
			else if(GetSymbol(argv[1]) == MakeSymbol("Alt_L") || GetSymbol(argv[1]) == MakeSymbol("Alt_R")) {
				code = 2003;
				mod = mod_Alt;
			}
			else {
				for(int i = 0;; ++i) {
					const char *ci = extkeys[i];
					if(!*ci) break;
					if(GetSymbol(argv[1]) == MakeSymbol(ci)) {
						code = 1000+i;
						break;
					}
				}
			}
#if 0 //def FLEXT_DEBUG
			else
				post("unknown modifier %s",str);
#endif
		}

		if(down) curmod |= mod;
		else curmod &= ~mod;

//		post("Key down=%i c=%c mod=%i",down?1:0,code,curmod);

		if(code || mod) {
			// remember past keycodes for repetition detection
			static int lastcode = 0,lastasc = 0,lastmod = 0;

			// button is pressed
			if(down) {
				if(lastcode == code && lastmod == curmod) 
					p.kind = evKeyRepeat;
				else {
					p.kind = evKeyDown;
					lastcode = code;
					lastasc = asc;
					lastmod = curmod;
				}
			}
			else {
				p.kind = evKeyUp;
				lastcode = lastasc = 0;
			}

			p.ext = true;
			p.pKey.k = code; //lastkey;
			p.pKey.a = asc;
			p.pKey.mod = curmod;

			for(guicanv *ix = gcanv; ix; ix = ix->nxt)
				for(canvobj *ci = ix->head; ci; ci = ci->nxt)
					ci->guiobj->m_Method(p); 
		}
	}
	else
		post("flext_gui key proxy - unknown method");
}

void flext_gui::g_Properties()
{
   char buf[800];
   sprintf(buf, "pdtk_flgui_dialog %%s %d %d %d\n",0, 0, 0);
   gfxstub_new((t_pd *)thisHdr(), thisHdr(), buf);
}


flext_gui::guicanv::guicanv(t_canvas *c): 
	canv(c),nxt(NULL),ref(0),
	head(NULL),tail(NULL)
{
	char tmp[25];
	sprintf(tmp,"FLCANV%x",c);
	sym = MakeSymbol(tmp);

#ifdef DIRECT_TK
	// proxy for canvas messages
    (px = (px_object *)pd_new(px_class))->init(c);  
#endif
}

flext_gui::guicanv::~guicanv()
{
#ifdef DIRECT_TK
	if(px) pd_free(&px->obj.ob_pd);
#endif
}

void flext_gui::guicanv::Push(flext_gui *o) 
{ 
	canvobj *co = new canvobj(o);
	if(tail) tail->nxt = co;
	tail = co;
	if(!head) head = tail;

	++ref; 
}

void flext_gui::guicanv::Pop(flext_gui *o) 
{ 
	canvobj *prv = NULL,*ix = head;
	for(; ix && ix->guiobj != o; prv = ix,ix = ix->nxt);

	if(ix) {
		--ref; 
		if(prv) prv->nxt = ix->nxt;
		else head = ix->nxt;
		if(!ix->nxt) tail = prv;
	}
	else
		error("flext_gui: object not found in canvas!");
}


void flext_gui::AddCanvas()
{
	t_canvas *c = thisCanvas();
	guicanv *prv = NULL,*ix = gcanv;
	for(; ix && ix->canv != c; prv = ix,ix = ix->nxt);

	if(ix) {
		ix->Push(this);
	}
	else {
		guicanv *nc = new guicanv(c);
		if(prv) prv->nxt = nc;
		else gcanv = nc;

		nc->Push(this);

#ifdef DIRECT_TK
		pd_bind(&nc->px->obj.ob_pd,(t_symbol *)nc->sym); 

/*
		// initialize new canvas object
		sys_vgui("bind .x%x.c <Motion> {pd %s %s %%x %%y %%s \\;}\n",c,GetString(nc->sym),GetString(gcm_motion));

		sys_vgui("bind .x%x.c <ButtonPress> {pd %s %s 1 %%x %%y %%b %%s \\;}\n",c,GetString(nc->sym),GetString(gcm_mousekey));
		sys_vgui("bind .x%x.c <ButtonRelease> {pd %s %s 0 %%x %%y %%b %%s \\;}\n",c,GetString(nc->sym),GetString(gcm_mousekey));
		sys_vgui("bind .x%x.c <MouseWheel> {pd %s %s %%x %%y %%s %%D \\;}\n",c,GetString(nc->sym),GetString(gcm_mousewheel));
		sys_vgui("bind .x%x.c <KeyPress> {pd %s %s 1 %%k %%A %%N %%s \\;}\n",c,GetString(nc->sym),GetString(gcm_key));
		sys_vgui("bind .x%x.c <KeyRelease> {pd %s %s 0 %%k %%A %%N %%s \\;}\n",c,GetString(nc->sym),GetString(gcm_key));

		// what happend to objects in subpatchers?
		sys_vgui("bind .x%x.c <Destroy> {pd %s %s \\;}\n",c,GetString(nc->sym),GetString(gcm_destroy));

	//	sys_vgui("bind .x%x.c <Visibility> {pd %s %s %x %s \\;}\n",c,GetString(nc->sym),"_tk_visibility",this);
*/
#endif
	}
}

void flext_gui::RmvCanvas()
{
	guicanv *ix = gcanv;
	for(; ix && ix->canv != thisCanvas(); ix = ix->nxt);

	if(ix) {
		ix->Pop(this);
		if(!ix->Refs()) DelCanvas(thisCanvas());
	}
	else {
		error("flext_gui: Canvas not found!");
	}
}

void flext_gui::DelCanvas(t_canvas *c)
{
	guicanv *prv = NULL,*ix = gcanv;
	for(; ix && ix->canv != c; prv = ix,ix = ix->nxt);

	if(ix) {
#ifdef DIRECT_TK
		pd_unbind(&ix->px->obj.ob_pd,(t_symbol *)ix->sym); 
#endif

		if(prv) prv->nxt = ix->nxt;
		else gcanv = ix->nxt;
	}
	else {
		error("flext_gui: Canvas not found!");
	}
}

static GuiObj *GetGuiObj(const t_atom &a)
{
	GuiObj *th = NULL;
	sscanf(flext::GetString(a),"%x",&th);
	return th;
}

void flext_gui::g_Displace(int dx, int dy)
{
//	post("Displace");

	XLo(XLo()+dx);
	YLo(YLo()+dy);

	Group().MoveRel(dx,dy);
	FixLines();
}

void flext_gui::g_Delete()
{
	objs->Clear();
	DelLines();
}



t_widgetbehavior flext_gui::widgetbehavior; 

void flext_gui::SetWidget(t_class *c)
{
	// widgetbehavior struct MUST be resident... (static is just ok here)

    widgetbehavior.w_getrectfn =    sg_getrect;
    widgetbehavior.w_displacefn =   sg_displace;
    widgetbehavior.w_selectfn =     sg_select;
    widgetbehavior.w_activatefn =   NULL; //sg_activate;
    widgetbehavior.w_deletefn =     sg_delete;
    widgetbehavior.w_visfn =        sg_vis;
    widgetbehavior.w_clickfn =      sg_click;
    widgetbehavior.w_propertiesfn = sg_properties;
    widgetbehavior.w_savefn =       sg_save;
    class_setwidget(c, &widgetbehavior);
}

void flext_gui::sg_getrect(t_gobj *c, t_glist *,int *xp1, int *yp1, int *xp2, int *yp2) 
{ 
	flext_gui *th = thisObject(c); 
	/*th->g_GetRect(*xp1,*yp1,*xp2,*yp2);*/ 
	*xp1 = th->XLo(),*yp1 = th->YLo(),*xp2 = th->XHi(),*yp2 = th->YHi(); 
}

void flext_gui::sg_displace(t_gobj *c, t_glist *, int dx, int dy) 
{ 
	thisObject(c)->g_Displace(dx,dy); 
}

void flext_gui::sg_select(t_gobj *c, t_glist *, int selected) 
{ 
//	post("Select");

	flext_gui *th = thisObject(c); 
	th->g_Edit(th->selected = (selected != 0)); 
}

void flext_gui::sg_vis(t_gobj *c, t_glist *, int vis) 
{ 
	post("Visible %i",vis);
	
	if(vis) { 
		flext_gui *g = thisObject(c); 
		g->g_Create(); 
		g->Group().MoveRel(g->XLo(),g->YLo());
		g->FixLines(); 
	}
}

int flext_gui::sg_click(t_gobj *c, t_glist *gl,int xpix, int ypix, int shift, int alt, int dbl, int doit)
{
	flext_gui *g = thisObject(c);
	CBParams p;
	int x = xpix-g->XLo();
	int y = ypix-g->YLo();

	// PD bug: shift isn't reported for idle mousing
//	int mod = (alt?mod_Alt:0)+(shift?mod_Shift:0)+(dbl?mod_Double:0);

	if(doit) {
		// button is pressed
		p.kind = evMouseDown;
		p.pMouseKey.x = g->xdrag = x;
		p.pMouseKey.y = g->ydrag = y;
		g->dxdrag = g->dydrag = 0;
		p.pMouseKey.b = 1;
		p.pMouseKey.mod = curmod; //mod;

		glist_grab(gl,c,(t_glistmotionfn)sg_drag,0,xpix,ypix);
	}
	else {
		// only mouse position change
		p.kind = evMotion;
		p.pMotion.x = x;
		p.pMotion.y = y;
		p.pMotion.mod = curmod; //mod;
	}
	g->m_Method(p); 
	return 1;
}

void flext_gui::sg_drag(t_gobj *c,t_floatarg dx,t_floatarg dy)
{
	flext_gui *g = thisObject(c);
	CBParams p;
	p.kind = evMouseDrag;
	p.pMouseDrag.dx = (g->dxdrag += (int)dx);
	p.pMouseDrag.dy = (g->dydrag += (int)dy);
	p.pMouseDrag.x = g->xdrag+g->dxdrag;
	p.pMouseDrag.y = g->ydrag+g->dydrag;;
	p.pMouseDrag.b = 1;
	p.pMouseDrag.mod = curmod; //mod;
	g->m_Method(p); 
}

void flext_gui::sg_delete(t_gobj *c, t_glist *) 
{ 
	thisObject(c)->g_Delete(); 
}

void flext_gui::sg_properties(t_gobj *c, t_glist *) 
{ 
	thisObject(c)->g_Properties(); 
}

void flext_gui::sg_save(t_gobj *c, t_binbuf *b) 
{ 
	thisObject(c)->g_Save(b); 
}

/*
bool flext_gui::sg_Key(flext_base *c,int argc,t_atom *argv)
{
	return true;
}

bool flext_gui::sg_KeyNum(flext_base *c,int &keynum)
{
	flext_gui *g = dynamic_cast<flext_gui *>(c);
	post("KeyNum %i",keynum);
	return true;
}

bool flext_gui::sg_KeyUp(flext_base *c,int &keynum)
{
	flext_gui *g = dynamic_cast<flext_gui *>(c);
	post("KeyUp %i",keynum);
	return true;
}
*/

#else // MAXMSP

// this declared supported events
int flext_gui::evmask = evMotion|evMouseDown|evKeyDown;

static void dragfun()
{
}

static void tmfun()
{
}

void flext_gui::sg_click(t_object *x, Point pt, short m) 
{ 
	flext_gui *g = thisObject(x);
	CBParams p(evMouseDown);
	p.pMouseKey.x = pt.h-g->XLo();
	p.pMouseKey.y = pt.v-g->YLo();
	p.pMouseKey.b = 0;
	p.pMouseKey.mod = (m&256?mod_Meta:0)+(m&512?mod_Shift:0)+(m&1024?mod_Caps:0)+(m&2048?mod_Alt:0)+(m&4096?mod_Ctrl:0);
	g->m_Method(p); 
}

void flext_gui::sg_update(t_object *x) 
{ 
	flext_gui *g = thisObject(x);
	if(!g->created) { g->g_Create(); g->created = true; }

	// draw elements
	g->Update();
}

void flext_gui::sg_psave (t_object *x, t_binbuf *dest) { thisObject(x)->g_Save(dest); }

void flext_gui::sg_bfont (t_object *x, short size, short font) {}

void flext_gui::sg_key (t_object *x, short keyvalue) 
{ 
	flext_gui *g = thisObject(x);

	CBParams p(evKeyDown);
	p.pKey.k = keyvalue;
	p.pKey.a = 0;
	p.pKey.mod = 0;
	g->m_Method(p); 
}

void flext_gui::sg_enter (t_object *x) {}

void flext_gui::sg_clipregion (t_object *x, RgnHandle *rgn, short *result) {}

void flext_gui::sg_bidle (t_object *x) 
{ 
	flext_gui *g = thisObject(x);
	Point pnt; GetMouse(&pnt);

	CBParams p(evMotion);
	p.pMotion.x = pnt.h-g->XLo();
	p.pMotion.y = pnt.v-g->YLo();
	p.pMotion.mod = 0;
	
	if(p.pMotion.x != g->curx || p.pMotion.y != g->cury || p.pMotion.mod != g->curmod) {
		g->m_Method(p); 
		g->curx = p.pMotion.x;
		g->cury = p.pMotion.y;
		g->curmod = p.pMotion.mod;
	}
}

void flext_gui::g_Displace(int dx, int dy)
{
}

void flext_gui::g_Delete()
{
	objs->Clear();
}

void flext_gui::Update()
{
	box_ready(&thisHdr()->z_box);

	if(Group().Canv().Pre(XLo(),YLo())) 
		Group().Draw();
	Group().Canv().Post();
}

#endif // PD / MAXMSP

void flext_gui::m_Method(const CBParams &p)
{
/*
	switch(p.kind) {
		case evMotion: {
		//	if(!g->Selected() || mod) 
			post("Motion: x=%i y=%i m=%i",p.pMotion.x,p.pMotion.y,p.pMotion.mod); 
			break;
		}
		case evMouseDown: {
			post("MouseDown: x=%i y=%i b=%i m=%i",p.pMouseKey.x,p.pMouseKey.y,p.pMouseKey.b,p.pMouseKey.mod);
			break;
		}
		case evMouseUp: {
			post("MouseUp: x=%i y=%i b=%i m=%i",p.pMouseKey.x,p.pMouseKey.y,p.pMouseKey.b,p.pMouseKey.mod);
			break;
		}
		case evMouseWheel: {
			post("Mousewheel: x=%i y=%i m=%i d=%i",p.pMouseWheel.x,p.pMouseWheel.y,p.pMouseWheel.mod,p.pMouseWheel.delta);
			break;
		}
		case evKeyDown: {
			post("KeyDown: k=%i a=%i m=%i",p.pKey.k,p.pKey.a,p.pKey.mod);
			break;
		}
		case evKeyUp: {
			post("KeyUp: k=%i a=%i m=%i",p.pKey.k,p.pKey.a,p.pKey.mod);
			break;
		}
	}
*/
	if(!Selected() || p.kind != evMotion || p.kind != evMouseDown || p.kind != evMouseUp)
		Group().Method(*this,p);
  }

bool flext_gui::BindEvent(GuiSingle &o,bool (*cb)(flext_gui &o,GuiSingle &obj,const CBParams &p),int evs)
{
	if((evs&EventMask()) == evs) {
		o.AddEvent(evs,cb);
		return true;
	}
	else 
		// not all requested events supported
		return false;
}

void flext_gui::UnbindEvent(GuiSingle &o,bool (*cb)(flext_gui &o,GuiSingle &obj,const CBParams &p),int evs)
{
	o.RmvEvent(evs,cb);
}


