/* 

dyn~ - dynamical object management for PD

Copyright (c) 2003 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  


-- this is all a bit hacky, but hey, it's PD! --

*/

#include <flext.h>

#if !defined(FLEXT_VERSION) || (FLEXT_VERSION < 402)
#error You need at least flext version 0.4.2
#endif

#define DYN_VERSION "0.0.3"


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
	void m_del(const t_symbol *n);
	void m_connect(int argc,const t_atom *argv) { conndis(true,argc,argv); }
	void m_disconnect(int argc,const t_atom *argv) { conndis(false,argc,argv); }
	void m_send(int argc,const t_atom *argv);
	void m_vis(bool vis);

protected:

	class obj {
	public:
		obj(const t_symbol *n,t_object *o): name(n),object(o),nxt(NULL) {}

		void Add(obj *o);

		const t_symbol *name;
		t_object *object;
		obj *nxt;
	} *root;

	obj *Find(const t_symbol *n);
	void Add(const t_symbol *n,t_object *o);

	void ToCanvas(t_object *o,t_binbuf *b,int x,int y);
	void FromCanvas(t_object *o);

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
		void exit();
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
	int m_inlets,s_inlets,m_outlets,s_outlets;
	t_canvas *canvas;

	void conndis(bool conn,int argc,const t_atom *argv);

private:
	static void setup(t_classid c);

	FLEXT_CALLBACK(m_reset)
	FLEXT_CALLBACK(m_reload)
	FLEXT_CALLBACK_V(m_newobj)
	FLEXT_CALLBACK_S(m_del)
	FLEXT_CALLBACK_V(m_connect)
	FLEXT_CALLBACK_V(m_disconnect)
	FLEXT_CALLBACK_V(m_send)
	FLEXT_CALLBACK_B(m_vis)
};

FLEXT_NEW_DSP_V("dyn~",dyn)


t_class *dyn::pxin_class = NULL,*dyn::pxout_class = NULL;
t_class *dyn::pxins_class = NULL,*dyn::pxouts_class = NULL;

void dyn::setup(t_classid c)
{
	post("");
	post("dyn~ %s - dynamic object management, (C)2003 Thomas Grill",DYN_VERSION);
	post("");

	// set up proxy class for inbound messages
	pxin_class = class_new(gensym("dyn proxy in"),NULL,NULL,sizeof(proxyin),0, A_NULL);
	add_anything(pxin_class,proxyin::px_method); 

	// set up proxy class for inbound signals
	pxins_class = class_new(gensym("dyn proxy in~"),NULL,NULL,sizeof(proxyin),0, A_NULL);
    add_dsp(pxins_class,proxyin::dsp);
    CLASS_MAINSIGNALIN(pxins_class, proxyin, defsig);

	// set up proxy class for outbound messages
	pxout_class = class_new(gensym("dyn proxy out"),NULL,NULL,sizeof(proxyout),0, A_NULL);
	add_anything(pxout_class,proxyout::px_method); 

	// set up proxy class for outbound signals
	pxouts_class = class_new(gensym("dyn proxy out~"),NULL,NULL,sizeof(proxyout),0, A_NULL);
	add_dsp(pxouts_class,proxyout::dsp);
    CLASS_MAINSIGNALIN(pxouts_class, proxyout, defsig);

	// set up dyn~
	FLEXT_CADDMETHOD_(c,0,"reset",m_reset);
	FLEXT_CADDMETHOD_(c,0,"reload",m_reload);
	FLEXT_CADDMETHOD_(c,0,"new",m_newobj);
	FLEXT_CADDMETHOD_(c,0,"del",m_del);
	FLEXT_CADDMETHOD_(c,0,"conn",m_connect);
	FLEXT_CADDMETHOD_(c,0,"dis",m_disconnect);
	FLEXT_CADDMETHOD_(c,0,"send",m_send);
	FLEXT_CADDMETHOD_(c,0,"vis",m_vis);
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
	pxin(NULL),pxout(NULL)
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

	// make a sub-canvas for dyn~
	t_atom arg[6];
	SetInt(arg[0],0);	// xpos
	SetInt(arg[1],0);	// ypos
	SetInt(arg[2],700);	// xwidth 
	SetInt(arg[3],520);	// xwidth 
	SetString(arg[4]," dyn~-canvas ");	// canvas name
	SetInt(arg[5],0);	// visible

	canvas = canvas_new(NULL, NULL, 6, arg);
	// must do that....
	canvas_unsetcurrent(canvas);

	pxin = new proxyin *[s_inlets+m_inlets];
	for(i = 0; i < s_inlets+m_inlets; ++i) {
		bool sig = i < s_inlets;
		pxin[i] = (proxyin *)object_new(sig?pxins_class:pxin_class);
		pxin[i]->init(this,sig);
		t_binbuf *b = binbuf_new();
		binbuf_text(b,sig?"dyn-in~":"dyn-in",7);
		ToCanvas(&pxin[i]->obj,b,i*100,10); // place them left-to-right 
	}

	pxout = new proxyout *[s_outlets+m_outlets];
	for(i = 0; i < s_outlets+m_outlets; ++i) {
		bool sig = i < s_outlets;
		pxout[i] = (proxyout *)object_new(sig?pxouts_class:pxout_class);
		pxout[i]->init(this,i,sig);
		t_binbuf *b = binbuf_new();
		binbuf_text(b,sig?"dyn-out~":"dyn-out",8);
		ToCanvas(&pxout[i]->obj,b,i*100,500); // place them left-to-right 
	}

	AddInSignal("Messages (new,del,conn,dis)");
	AddInSignal(s_inlets);
	AddInAnything(m_inlets);
	AddOutSignal(s_outlets);
	AddOutAnything(m_outlets);
}

dyn::~dyn()
{
	m_reset();

	if(pxin) {
		for(int i = 0; i < s_inlets+m_inlets; ++i)
			if(pxin[i])	{ pxin[i]->exit(); FromCanvas(&pxin[i]->obj); }
		delete[] pxin;
	}
	if(pxout) {
		for(int i = 0; i < s_outlets+m_outlets; ++i)
			if(pxout[i]) { pxout[i]->exit(); FromCanvas(&pxout[i]->obj); }
		delete[] pxout;
	}

	if(canvas) {
		pd_free((t_pd *)canvas);
	}
}


void dyn::obj::Add(obj *o) {	if(nxt) nxt->Add(o); else nxt = o; }

dyn::obj *dyn::Find(const t_symbol *n)
{
	for(obj *o = root; o && o->name != n; o = o->nxt) {}
	return o;
}

void dyn::Add(const t_symbol *n,t_object *ob)
{
	obj *o = new obj(n,ob);
	if(root) root->Add(o); else root = o;
}

void dyn::ToCanvas(t_object *o,t_binbuf *b,int x,int y)
{
	// add object to the glist.... this is needed for graphical representation
	// which is needed to have all connections be properly deleted
	
	o->te_binbuf = b; 
	o->te_xpix = x,o->te_ypix = y;
	o->te_width = 0;
	o->te_type = T_OBJECT;

	glist_add(canvas, &o->te_g);
}

void dyn::FromCanvas(t_object *o)
{
	glist_delete(canvas,&o->te_g);
}

void dyn::m_reset()
{
	obj *o = root;
	while(o) {
		FromCanvas(o->object);
		obj *n = o->nxt;
		delete o; 
		o = n;
	}
	root = NULL; 
}

void dyn::m_vis(bool vis)
{
	canvas_vis(canvas,vis?1:0);
}

void dyn::m_reload()
{
	post("%s - reload: not implemented yet",thisName());
}


void dyn::m_newobj(int _argc_,const t_atom *_argv_)
{
	int argc = 0;
	const t_atom *argv = NULL;
	int posx,posy;

	if(_argc_ >= 4 && CanbeInt(_argv_[0]) && CanbeInt(_argv_[1]) && IsSymbol(_argv_[2]) && IsSymbol(_argv_[3])) {
		posx = GetAInt(_argv_[0]);
		posy = GetAInt(_argv_[1]);
		argc = _argc_-2;
		argv = _argv_+2;
	}
	else if(_argc_ >= 2 && IsSymbol(_argv_[0]) && IsSymbol(_argv_[1])) {
		// random position if not given
		posx = rand()%600;
		posy = 50+rand()%400;
		argc = _argc_;
		argv = _argv_;
	}

	if(argv) {
		const t_symbol *name = GetSymbol(argv[0]);

		if(Find(name)) 
			post("%s - new: object \"%s\" is already present",thisName(),GetString(name));
		else {
			// now set canvas 
			canvas_setcurrent(canvas); 

			t_binbuf *b = binbuf_new();
			// make arg list
			binbuf_add(b,argc-1,(t_atom *)argv+1);
			// send message to object maker
			binbuf_eval(b,&pd_objectmaker,0,NULL);

			// look for latest created object
			t_object *x = (t_object *)pd_newest();
			if(x) {
				// place it
				ToCanvas(x,b,posx,posy);
				// add to database
				Add(name,x);

				// send loadbang (if it is an abstraction)
				if(pd_class(&x->te_g.g_pd) == canvas_class) {
					// loadbang the abstraction
					pd_vmess((t_pd *)x,gensym("loadbang"),"");			

					// for an abstraction dsp must be restarted 
					// that's necessary because ToCanvas is called manually
					// (may also be necessary for normal objects in a later PD version)
					canvas_update_dsp();
				}
			}
			else {
				post("%s - new: Could not create object",thisName());
				binbuf_free(b);
			}

			// pop the dyn~ canvas 
			canvas_unsetcurrent(canvas); 
		}
	}
	else
		post("%s - new name object [args]",thisName());
}

void dyn::m_del(const t_symbol *n)
{
	obj *p = NULL,*o = root;
	for(; o && o->name != n; p = o,o = o->nxt) {}

	if(o) {
		if(p) p->nxt = o->nxt; else root = o->nxt;

		FromCanvas(o->object);
		delete o;
	}
	else
		post("%s - del: object not found",thisName());
}

void dyn::conndis(bool conn,int argc,const t_atom *argv)
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

	t_object *s_obj,*d_obj;
	if(s_n) {
		obj *s_o = Find(s_n);
		if(!s_o) { 
			post("%s - connect: source \"%s\" not found",thisName(),GetString(s_n));
			return;
		}
		s_obj = s_o->object;
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
		d_obj = d_o->object;
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
	int s_oix = canvas_getindex(canvas,&s_obj->ob_g);
	int d_oix = canvas_getindex(canvas,&d_obj->ob_g);
#endif

    if(conn) {
		if(!canvas_isconnected(canvas,s_obj,s_x,d_obj,d_x)) {
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

void dyn::proxy::exit() { if(buf) delete[] buf; }

	
void dyn::proxyin::dsp(proxyin *x,t_signal **sp)
{
	int n = sp[0]->s_n;
	if(n != x->n) {
		// if vector size has changed make new buffer
		if(x->buf) delete[] x->buf;
		x->buf = new t_sample[x->n = n];
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
		if(x->buf) delete[] x->buf;
		x->buf = new t_sample[x->n = n];
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
    mess1((t_pd *)canvas, gensym("dsp"),NULL);

	flext_dsp::m_dsp(n,insigs,outsigs);
}
    
void dyn::m_signal(int n,t_sample *const *insigs,t_sample *const *outsigs)
{
	int i;
	for(i = 0; i < s_inlets; ++i)
		CopySamples(pxin[i]->buf,insigs[i+1],n);
	for(i = 0; i < s_outlets; ++i)
		CopySamples(outsigs[i],pxout[i]->buf,n);
}

