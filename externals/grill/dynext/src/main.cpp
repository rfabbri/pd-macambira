/* 

dyn~ - dynamical object management for PD

Copyright (c)2003-2005 Thomas Grill (gr@grrrr.org)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  


-- this is all a bit hacky, but hey, it's PD! --

*/

#define FLEXT_ATTRIBUTES 1

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 500)
#error You need at least flext version 0.5.0
#endif

#define DYN_VERSION "0.1.1pre"


#if FLEXT_SYS != FLEXT_SYS_PD
#error Sorry, dyn~ works for PD only!
#endif


#include "flinternal.h"
#include <stdlib.h>

#ifdef _MSC_VER
#pragma warning(disable: 4091 4244)
#endif
#include "g_canvas.h"


class dyn:
	public flext_dsp
{
	FLEXT_HEADER_S(dyn,flext_dsp,setup)
public:
	dyn(int argc,const t_atom *argv);
	virtual ~dyn();

	void m_reset();
	void m_reload(); // refresh objects/abstractions
	void m_newobj(int argc,const t_atom *argv);
	void m_newmsg(int argc,const t_atom *argv);
	void m_newtext(int argc,const t_atom *argv);
	void m_del(const t_symbol *n);
	void m_connect(int argc,const t_atom *argv) { ConnDis(true,argc,argv); }
	void m_disconnect(int argc,const t_atom *argv) { ConnDis(false,argc,argv); }
	void m_send(int argc,const t_atom *argv);
    void ms_vis(bool vis) { canvas_vis(canvas,vis?1:0); }
    void mg_vis(bool &vis) const { vis = canvas && canvas->gl_editor; }

protected:

    virtual void m_click() { ms_vis(true); }

    static const t_symbol *k_obj,*k_msg,*k_text;

	class obj {
	public:
		obj(unsigned long i,t_gobj *o): id(i),object(o),nxt(NULL) {}

		void Add(obj *o);

		unsigned long id;
		t_gobj *object;
		obj *nxt;
	} *root;


	obj *Find(const t_symbol *n);
    t_glist *FindCanvas(const t_symbol *n);
	void Add(const t_symbol *n,t_gobj *o);

    t_gobj *New(const t_symbol *kind,int _argc_,const t_atom *_argv_,bool add = true);
	void Delete(t_gobj *o);

	void ConnDis(bool conn,int argc,const t_atom *argv);

    virtual bool m_method_(int n,const t_symbol *s,int argc,const t_atom *argv);
	virtual void m_dsp(int n,t_signalvec const *insigs,t_signalvec const *outsigs);
	virtual void m_signal(int n,t_sample *const *insigs,t_sample *const *outsigs);


	// proxy object
	class proxy
	{ 
	public:
		t_object obj;
		dyn *th;
		int n;
		t_sample *buf;
		t_sample defsig;

		void init(dyn *t);

        static void px_exit(proxy *px) { if(px->buf) FreeAligned(px->buf); }
	};

	// proxy for inbound messages
	class proxyin:
		public proxy
	{ 
	public:
		t_outlet *outlet;

		void Message(const t_symbol *s,int argc,const t_atom *argv) 
		{
			typedmess((t_pd *)&obj,(t_symbol *)s,argc,(t_atom *)argv);
		}

		void init(dyn *t,bool s);

		static void px_method(proxyin *obj,const t_symbol *s,int argc,const t_atom *argv)
		{
			outlet_anything(obj->outlet,(t_symbol *)s,argc,(t_atom *)argv);
		}

		static void dsp(proxyin *x, t_signal **sp);
	};


	// proxy for outbound messages
	class proxyout:
		public proxy
	{ 
	public:
		int outlet;

		void init(dyn *t,int o,bool s);

		static void px_method(proxyout *obj,const t_symbol *s,int argc,const t_atom *argv)
		{
			obj->th->ToOutAnything(obj->outlet,s,argc,argv);
		}

		static void dsp(proxyout *x, t_signal **sp);
	};

	static t_class *pxin_class,*pxout_class;
	static t_class *pxins_class,*pxouts_class;

	proxyin **pxin;
	proxyout **pxout;

    static t_object *pxin_new() { return (t_object *)pd_new(pxin_class); }
    static t_object *pxins_new() { return (t_object *)pd_new(pxins_class); }
    static t_object *pxout_new() { return (t_object *)pd_new(pxout_class); }
    static t_object *pxouts_new() { return (t_object *)pd_new(pxouts_class); }

    int m_inlets,s_inlets,m_outlets,s_outlets;
	t_canvas *canvas;
    bool stripext;

private:
	static void setup(t_classid c);

	FLEXT_CALLBACK(m_reset)
	FLEXT_CALLBACK(m_reload)
	FLEXT_CALLBACK_V(m_newobj)
	FLEXT_CALLBACK_V(m_newmsg)
	FLEXT_CALLBACK_V(m_newtext)
	FLEXT_CALLBACK_S(m_del)
	FLEXT_CALLBACK_V(m_connect)
	FLEXT_CALLBACK_V(m_disconnect)
	FLEXT_CALLBACK_V(m_send)
	FLEXT_CALLVAR_B(mg_vis,ms_vis)
//	FLEXT_CALLBACK(m_refresh)

	FLEXT_ATTRVAR_B(stripext)

    static const t_symbol *sym_dot,*sym_dynsin,*sym_dynsout,*sym_dynin,*sym_dynout,*sym_dyncanvas;
    static const t_symbol *sym_vis,*sym_loadbang,*sym_dsp;
};

FLEXT_NEW_DSP_V("dyn~",dyn)


t_class *dyn::pxin_class = NULL,*dyn::pxout_class = NULL;
t_class *dyn::pxins_class = NULL,*dyn::pxouts_class = NULL;

const t_symbol *dyn::k_obj = NULL;
const t_symbol *dyn::k_msg = NULL;
const t_symbol *dyn::k_text = NULL;

const t_symbol *dyn::sym_dot = NULL;
const t_symbol *dyn::sym_dynsin = NULL;
const t_symbol *dyn::sym_dynsout = NULL;
const t_symbol *dyn::sym_dynin = NULL;
const t_symbol *dyn::sym_dynout = NULL;
const t_symbol *dyn::sym_dyncanvas = NULL;

const t_symbol *dyn::sym_vis = NULL;
const t_symbol *dyn::sym_loadbang = NULL;
const t_symbol *dyn::sym_dsp = NULL;


void dyn::setup(t_classid c)
{
	post("");
	post("dyn~ %s - dynamic object management, (C)2003-2005 Thomas Grill",DYN_VERSION);
	post("");

    sym_dynsin = MakeSymbol("dyn_in~");
    sym_dynsout = MakeSymbol("dyn_out~");
    sym_dynin = MakeSymbol("dyn_in");
    sym_dynout = MakeSymbol("dyn_out");

    sym_dot = MakeSymbol(".");
    sym_dyncanvas = MakeSymbol(" dyn~-canvas ");

	// set up proxy class for inbound messages
    pxin_class = class_new(const_cast<t_symbol *>(sym_dynin),(t_newmethod)pxin_new,(t_method)proxy::px_exit,sizeof(proxyin),0, A_NULL);
	add_anything(pxin_class,proxyin::px_method); 

	// set up proxy class for inbound signals
	pxins_class = class_new(const_cast<t_symbol *>(sym_dynsin),(t_newmethod)pxins_new,(t_method)proxy::px_exit,sizeof(proxyin),0, A_NULL);
    add_dsp(pxins_class,proxyin::dsp);
    CLASS_MAINSIGNALIN(pxins_class, proxyin, defsig);

	// set up proxy class for outbound messages
	pxout_class = class_new(const_cast<t_symbol *>(sym_dynout),(t_newmethod)pxout_new,(t_method)proxy::px_exit,sizeof(proxyout),0, A_NULL);
	add_anything(pxout_class,proxyout::px_method); 

	// set up proxy class for outbound signals
	pxouts_class = class_new(const_cast<t_symbol *>(sym_dynsout),(t_newmethod)pxouts_new,(t_method)proxy::px_exit,sizeof(proxyout),0, A_NULL);
	add_dsp(pxouts_class,proxyout::dsp);
    CLASS_MAINSIGNALIN(pxouts_class, proxyout, defsig);

	// set up dyn~
	FLEXT_CADDMETHOD_(c,0,"reset",m_reset);
	FLEXT_CADDMETHOD_(c,0,"reload",m_reload);
	FLEXT_CADDMETHOD_(c,0,"newobj",m_newobj);
	FLEXT_CADDMETHOD_(c,0,"newmsg",m_newmsg);
	FLEXT_CADDMETHOD_(c,0,"newtext",m_newtext);
	FLEXT_CADDMETHOD_(c,0,"del",m_del);
	FLEXT_CADDMETHOD_(c,0,"conn",m_connect);
	FLEXT_CADDMETHOD_(c,0,"dis",m_disconnect);
	FLEXT_CADDMETHOD_(c,0,"send",m_send);
	FLEXT_CADDATTR_VAR(c,"vis",mg_vis,ms_vis);
    FLEXT_CADDATTR_VAR1(c,"stripext",stripext);

    // set up symbols
    k_obj = MakeSymbol("obj"); 
    k_msg = MakeSymbol("msg"); 
    k_text = MakeSymbol("text"); 

    sym_vis = MakeSymbol("vis");
    sym_loadbang = MakeSymbol("loadbang");
    sym_dsp = MakeSymbol("dsp");
}


/*
There must be a separate canvas for the dynamically created objects as some mechanisms in PD
(like copy/cut/paste) get confused otherwise.
On the other hand it seems to be possible to create objects without a canvas. 
They won't receive DSP processing, though, hence it's only possible for message objects.
Problems arise when an object is not yet loaded... the canvas environment is then needed to
load it.. if there is no canvas, PD currently crashes.


How to create the canvas:
1) via direct call to canvas_new()
2) a message to pd_canvasmaker
	con: does not return a pointer for the created canvas

There are two possibilities for the canvas
1) make a sub canvas to the one where dyn~ resides:
	pro: no problems with environment (abstractions are found and loaded correctly)
2) make a root canvas:
	pro: it will be in the canvas list per default, hence DSP is processed
	con: canvas environment must be created manually 
			(is normally done by pd_canvasmaker if there is a directory set, which is again done somewhere else)

Enabling DSP on the subcanvas
1) send it a "dsp" message (see rabin~ by K.Czaja)... but, which signal vector should be taken?
	-> answer: NONE!  (just send NULL)
2) add it to the list of _root_ canvases (these will be DSP-processed per default)
	(for this the canvas_addtolist and canvas_takefromlist functions are used)
	however, it's not clear if this can lead to problems since it is no root-canvas!

In all cases the 1)s have been chosen as the cleaner solution
*/

dyn::dyn(int argc,const t_atom *argv):
	root(NULL),
	canvas(NULL),
	pxin(NULL),pxout(NULL),
    stripext(false)
{
	if(argc < 4) { 
		post("%s - Syntax: dyn~ sig-ins msg-ins sig-outs msg-outs",thisName());
		InitProblem(); 
		return; 
	}

	s_inlets = GetAInt(argv[0]);
	m_inlets = GetAInt(argv[1]);
	s_outlets = GetAInt(argv[2]);
	m_outlets = GetAInt(argv[3]);

	int i;

	// --- make a sub-canvas for dyn~ ------

	t_atom arg[6];
	SetInt(arg[0],0);	// xpos
	SetInt(arg[1],0);	// ypos
	SetInt(arg[2],700);	// xwidth 
	SetInt(arg[3],520);	// xwidth 
	SetSymbol(arg[4],sym_dyncanvas);	// canvas name
	SetInt(arg[5],0);	// visible

	canvas = canvas_new(NULL, NULL, 6, arg);
	// must do that....
	canvas_unsetcurrent(canvas);

	// --- create inlet proxies ------

	pxin = new proxyin *[s_inlets+m_inlets];
	for(i = 0; i < s_inlets+m_inlets; ++i) {
		bool sig = i < s_inlets;

        t_atom lst[5];
        SetInt(lst[0],i*100);
        SetInt(lst[1],10);
        SetSymbol(lst[2],sym_dot);
        SetSymbol(lst[3],sym__);
        SetSymbol(lst[4],sig?sym_dynsin:sym_dynin);

        try {
            pxin[i] = (proxyin *)New(k_obj,5,lst,false);
		    if(pxin[i]) pxin[i]->init(this,sig);
        }
        catch(...) {
            error("%s - Error creating inlet proxy",thisName());
        }
    }

	// --- create outlet proxies ------

	pxout = new proxyout *[s_outlets+m_outlets];
	for(i = 0; i < s_outlets+m_outlets; ++i) {
		bool sig = i < s_outlets;

        t_atom lst[5];
        SetInt(lst[0],i*100);
        SetInt(lst[1],500);
        SetSymbol(lst[2],sym_dot);
        SetSymbol(lst[3],sym__);
        SetSymbol(lst[4],sig?sym_dynsout:sym_dynout);

        try {
            pxout[i] = (proxyout *)New(k_obj,5,lst,false);
            if(pxout[i]) pxout[i]->init(this,i,sig);
        }
        catch(...) {
            error("%s - Error creating outlet proxy",thisName());
        }
    }

	AddInSignal("Messages (newobj,newmsg,newtext,del,conn,dis)");
	AddInSignal(s_inlets);
	AddInAnything(m_inlets);
	AddOutSignal(s_outlets);
	AddOutAnything(m_outlets);
}

dyn::~dyn()
{
	m_reset();

	if(canvas) pd_free((t_pd *)canvas);

	if(pxin) delete[] pxin;
	if(pxout) delete[] pxout;
}


void dyn::obj::Add(obj *o) {	if(nxt) nxt->Add(o); else nxt = o; }

dyn::obj *dyn::Find(const t_symbol *n)
{
    unsigned long id = *(unsigned long *)n;
	obj *o;
	for(o = root; o && o->id != id; o = o->nxt) {}
	return o;
}

t_glist *dyn::FindCanvas(const t_symbol *n)
{
    if(n == sym_dot) 
        return canvas;
    else {
        obj *o = Find(n);
        if(o && pd_class(&o->object->g_pd) == canvas_class) 
            return (t_glist *)o->object;
        else 
            return NULL;
    }
}

void dyn::Add(const t_symbol *n,t_gobj *ob)
{
    unsigned long id = *(unsigned long *)n;
	obj *o = new obj(id,ob);
	if(root) root->Add(o); else root = o;
}

void dyn::Delete(t_gobj *o)
{
	glist_delete(canvas,o);
}

static t_gobj *GetLast(t_glist *gl)
{
    t_gobj *go = gl->gl_list;
    if(go)
        while(go->g_next) 
            go = go->g_next;
    return go;
}

t_gobj *dyn::New(const t_symbol *kind,int _argc_,const t_atom *_argv_,bool add)
{
    t_gobj *newest = NULL;
    const char *err = NULL;
	int argc = 0;
	t_atom *argv = NULL;
    const t_symbol *name = NULL,*canv = NULL;
    t_glist *glist = NULL;

	if(_argc_ >= 4 && CanbeInt(_argv_[0]) && CanbeInt(_argv_[1]) && IsSymbol(_argv_[2]) && IsSymbol(_argv_[3])) {
        canv = GetSymbol(_argv_[2]);
        name = GetSymbol(_argv_[3]);

        argc = _argc_-2;
        argv = new t_atom[argc];
		SetInt(argv[0],GetAInt(_argv_[0]));
		SetInt(argv[1],GetAInt(_argv_[1]));
        for(int i = 0; i < argc-2; ++i) SetAtom(argv[i+2],_argv_[i+4]);
	}
	else if(_argc_ >= 3 && IsSymbol(_argv_[0]) && IsSymbol(_argv_[1])) {
        canv = GetSymbol(_argv_[0]);
        name = GetSymbol(_argv_[1]);

        argc = _argc_;
        argv = new t_atom[argc];
		// random position if not given
		SetInt(argv[0],rand()%600);
		SetInt(argv[1],50+rand()%400);
        for(int i = 0; i < argc-2; ++i) SetAtom(argv[i+2],_argv_[i+2]);
	}

	if(argv) {
		if(add && (!name || name == sym_dot || Find(name))) 
			err = "Object name is already present";
        else if(!canv || !(glist = FindCanvas(canv)))
			err = "Canvas could not be found";
        else {
            // convert abstraction filenames
            if(stripext && kind == k_obj && argc >= 3 && IsSymbol(argv[2])) {
                const char *c = GetString(argv[2]);
                int l = strlen(c);
                // check end of string for .pd file extension
                if(l >= 4 && !memcmp(c+l-3,".pd",4)) {
                    // found -> get rid of it
                    char tmp[64],*t = tmp;
                    if(l > sizeof tmp-1) t = new char[l+1];
                    memcpy(tmp,c,l-3); tmp[l-3] = 0;
                    SetString(argv[2],tmp);
                    if(tmp != t) delete[] t;
                }
            }

            // set selected canvas as current
			canvas_setcurrent(glist); 

            t_gobj *last = GetLast(glist);
            pd_typedmess((t_pd *)glist,(t_symbol *)kind,argc,argv);
            newest = GetLast(glist);

            if(kind == k_obj) {
                t_object *o = (t_object *)pd_newest();

                if(!o) {
                    // PD creates a text object when the intended object could not be created
                    t_gobj *trash = GetLast(glist);

                    // Test for newly created object....
                    if(trash && last != trash) {
                        // Delete it!
                        glist_delete(glist,trash);
                    }
                    newest = NULL;
                }
                else
                    newest = &o->te_g;
            }

			// look for latest created object
			if(newest) {
				// add to database
				if(add) Add(name,newest);

				// send loadbang (if it is an abstraction)
				if(pd_class(&newest->g_pd) == canvas_class) {
					// hide the sub-canvas
					pd_vmess((t_pd *)newest,const_cast<t_symbol *>(sym_vis),"i",0);

                    // loadbang the abstraction
					pd_vmess((t_pd *)newest,const_cast<t_symbol *>(sym_loadbang),"");
                }

				// restart dsp - that's necessary because ToCanvas is called manually
				canvas_update_dsp();
			}
			else
				if(!err) err = "Could not create object";

			// pop the current canvas 
			canvas_unsetcurrent(glist); 
		}

        delete[] argv;
	}
	else 
        if(!err) err = "new name object [args]";

    if(err) throw err;

    return newest;
}


void dyn::m_reset()
{
	obj *o = root;
	while(o) {
		Delete(o->object);
		obj *n = o->nxt;
		delete o; 
		o = n;
	}
	root = NULL; 
}

void dyn::m_reload()
{
	post("%s - reload: not implemented yet",thisName());
}

void dyn::m_newobj(int argc,const t_atom *argv)
{
    try { New(k_obj,argc,argv); }
    catch(const char *err) {
		post("%s - %s",thisName(),err);
    }
    catch(...) {
		post("%s - unknown error",thisName());
    }
}

void dyn::m_newmsg(int argc,const t_atom *argv)
{
    try { New(k_msg,argc,argv); }
    catch(const char *err) {
		post("%s - %s",thisName(),err);
    }
    catch(...) {
		post("%s - unknown error",thisName());
    }
}

void dyn::m_newtext(int argc,const t_atom *argv)
{
    try { New(k_text,argc,argv); }
    catch(const char *err) {
		post("%s - %s",thisName(),err);
    }
    catch(...) {
		post("%s - unknown error",thisName());
    }
}

void dyn::m_del(const t_symbol *n)
{
	unsigned long id = *(unsigned long *)n;

    obj *p = NULL,*o = root;
	for(; o && o->id != id; p = o,o = o->nxt) {}

	if(o) {
		if(p) p->nxt = o->nxt; else root = o->nxt;

		Delete(o->object);
		delete o;
	}
	else
		post("%s - del: object not found",thisName());
}

void dyn::ConnDis(bool conn,int argc,const t_atom *argv)
{
	const t_symbol *s_n = NULL,*d_n = NULL;
	int s_x,d_x;

	if(argc == 4 && IsSymbol(argv[0]) && CanbeInt(argv[1]) && IsSymbol(argv[2]) && CanbeInt(argv[3])) {
		s_n = GetSymbol(argv[0]);
		s_x = GetAInt(argv[1]);
		d_n = GetSymbol(argv[2]);
		d_x = GetAInt(argv[3]);
	}
	else if(argc == 3 && CanbeInt(argv[0]) && IsSymbol(argv[1]) && CanbeInt(argv[2])) {
		s_n = NULL;
		s_x = GetAInt(argv[0]);
		d_n = GetSymbol(argv[1]);
		d_x = GetAInt(argv[2]);
	}
	else if(argc == 3 && IsSymbol(argv[0]) && CanbeInt(argv[1]) && CanbeInt(argv[2])) {
		s_n = GetSymbol(argv[0]);
		s_x = GetAInt(argv[1]);
		d_n = NULL;
		d_x = GetAInt(argv[2]);
	}
	else if(argc == 2 && CanbeInt(argv[0]) && CanbeInt(argv[1])) {
		// direct connection from proxy-in to proxy-out (for testing above all....)
		s_n = NULL;
		s_x = GetAInt(argv[0]);
		d_n = NULL;
		d_x = GetAInt(argv[1]);
	}
	else {
		post("%s - connect: [src] srcslot [dst] dstslot",thisName());
		return;
	}

	t_text *s_obj,*d_obj;
	if(s_n) {
		obj *s_o = Find(s_n);
		if(!s_o) { 
			post("%s - connect: source \"%s\" not found",thisName(),GetString(s_n));
			return;
		}
		s_obj = (t_text *)s_o->object;
	}
	else if(s_x < 0 && s_x >= s_inlets+m_inlets) {
		post("%s - connect: inlet %i out of range (0..%i)",thisName(),s_x,s_inlets+m_inlets-1);
		return;
	}
	else {
		s_obj = &pxin[s_x]->obj;
		s_x = 0; // always 0 for proxy
	}

	if(d_n) {
		obj *d_o = Find(d_n);
		if(!d_o) { 
			post("%s - connect: destination \"%s\" not found",thisName(),GetString(d_n));
			return;
		}
		d_obj = (t_text *)d_o->object;
	}
	else if(d_x < 0 && d_x >= s_outlets+m_outlets) {
		post("%s - connect: outlet %i out of range (0..%i)",thisName(),d_x,s_outlets+m_outlets-1);
		return;
	}
	else  {
		d_obj = &pxout[d_x]->obj;
		d_x = 0; // always 0 for proxy
	}

#ifndef NO_VIS
	int s_oix = canvas_getindex(canvas,&s_obj->te_g);
	int d_oix = canvas_getindex(canvas,&d_obj->te_g);
#endif

    if(conn) {
		if(!canvas_isconnected(canvas,(t_text *)s_obj,s_x,(t_text *)d_obj,d_x)) {
#ifdef NO_VIS
			if(!obj_connect(s_obj, s_x, d_obj, d_x))
				post("%s - connect: connection could not be made",thisName());
#else
			canvas_connect(canvas,s_oix,s_x,d_oix,d_x);
#endif
		}
	}
	else {
#ifdef NO_VIS
		obj_disconnect(s_obj, s_x, d_obj, d_x);
#else
		canvas_disconnect(canvas,s_oix,s_x,d_oix,d_x);
#endif
	}
}


bool dyn::m_method_(int n,const t_symbol *s,int argc,const t_atom *argv)
{
	if(n == 0) 
		// messages into inlet 0 are for dyn~
		return flext_base::m_method_(n,s,argc,argv);
	else {
		// all other messages are forwarded to proxies (and connected objects)
		pxin[n-1]->Message(s,argc,argv);
		return true;
	}
}


void dyn::m_send(int argc,const t_atom *argv)
{
	if(argc < 2 || !IsSymbol(argv[0])) 
		post("%s - Syntax: send name message [args]",thisName());
	else {
		obj *o = Find(GetSymbol(argv[0]));
		if(!o)
			post("%s - send: object \"%s\" not found",thisName(),GetString(argv[0]));
		else
			pd_forwardmess((t_pd *)o->object,argc-1,(t_atom *)argv+1);
	}
}


void dyn::proxy::init(dyn *t) 
{ 
	th = t; 
	n = 0,buf = NULL;
	defsig = 0;
}

void dyn::proxyin::dsp(proxyin *x,t_signal **sp)
{
	int n = sp[0]->s_n;
	if(n != x->n) {
		// if vector size has changed make new buffer
		if(x->buf) FreeAligned(x->buf);
		x->buf = (t_sample *)NewAligned(sizeof(t_sample)*(x->n = n));
	}
	dsp_add_copy(x->buf,sp[0]->s_vec,n);
}

void dyn::proxyin::init(dyn *t,bool s) 
{ 
	proxy::init(t);
	outlet = outlet_new(&obj,s?&s_signal:&s_anything); 
}



	
void dyn::proxyout::dsp(proxyout *x,t_signal **sp)
{
	int n = sp[0]->s_n;
	if(n != x->n) {
		// if vector size has changed make new buffer
		if(x->buf) FreeAligned(x->buf);
		x->buf = (t_sample *)NewAligned(sizeof(t_sample)*(x->n = n));
	}
	dsp_add_copy(sp[0]->s_vec,x->buf,n);
}

void dyn::proxyout::init(dyn *t,int o,bool s) 
{ 
	proxy::init(t);
	outlet = o;
	if(s) outlet_new(&obj,&s_signal); 
}


void dyn::m_dsp(int n,t_signalvec const *insigs,t_signalvec const *outsigs)
{
	// add sub canvas to dsp list (no signal vector to borrow from .. set it to NULL)
    mess1((t_pd *)canvas,const_cast<t_symbol *>(sym_dsp),NULL);

	flext_dsp::m_dsp(n,insigs,outsigs);
}
    
void dyn::m_signal(int n,t_sample *const *insigs,t_sample *const *outsigs)
{
	int i;
	for(i = 0; i < s_inlets; ++i)
		if(pxin[i]->buf)
		CopySamples(pxin[i]->buf,insigs[i+1],n);
	for(i = 0; i < s_outlets; ++i)
		if(pxout[i]->buf)
		CopySamples(outsigs[i],pxout[i]->buf,n);
}

