/*
	$Id: puredata.c,v 1.1 2005-10-04 02:02:13 matju Exp $

	GridFlow
	Copyright (c) 2001,2002,2003,2004,2005 by Mathieu Bouchard

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	See file ../COPYING for further informations on licensing terms.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*
'bridge_puredata.c' becomes 'gridflow.pd_linux' which loads Ruby and tells
Ruby to load the other 95% of Gridflow. GridFlow creates the FObject class
and then subclasses it many times, each time calling FObject.install(),
which tells the bridge to register a class in puredata. Puredata
objects have proxies for each of the non-first inlets so that any inlet
may receive any distinguished message. All inlets are typed as 'anything',
and the 'anything' method translates the whole message to Ruby objects and
tries to call a Ruby method of the proper name.
*/

#define IS_BRIDGE
#include "../base/grid.h.fcs"
/* resolving conflict: T_OBJECT will be PD's, not Ruby's */
#undef T_OBJECT
#undef EXTERN
#include <m_pd.h>
#include <ctype.h>
#include <stdarg.h>
#include <unistd.h>
#include "g_canvas.h"

/* **************************************************************** */
struct BFObject;
struct FMessage {
	BFObject *self;
	int winlet;
	t_symbol *selector;
	int ac;
	const t_atom *at;
	bool is_init;
};
#include <sys/time.h>
#include <signal.h>
#include <setjmp.h>
#define rb_sym_name rb_sym_name_r4j
static const char *rb_sym_name(Ruby sym) {return rb_id2name(SYM2ID(sym));}

static BuiltinSymbols *syms;

void CObject_freeee (void *victim) {
	CObject *self = (CObject *)victim;
	self->check_magic();
	if (!self->rself) {
		fprintf(stderr,"attempt to free object that has no rself\n");
		abort();
	}
	self->rself = 0; /* paranoia */
	delete self;
}


/* can't even refer to the other mGridFlow because we don't link
   statically to the other gridflow.so */
static Ruby mGridFlow2=0;
static Ruby mPointer=0;

\class Pointer < CObject
struct Pointer : CObject {
	void *p;
	Pointer() { assert(!"DYING HORRIBLY"); }
	Pointer(void *_p) : p(_p) {}
	\decl Ruby ptr ();
};
\def Ruby ptr () { return LONG2NUM(((long)p)); }
\classinfo {
	IEVAL(rself,
"self.module_eval{"
"def inspect; p=('%08x'%ptr).gsub(/^\\.\\.f/,''); \"#<Pointer:#{p}>\" % ptr; end;"
"alias to_s inspect }"
);}
\end class Pointer
Ruby Pointer_s_new (void *ptr) {
	return Data_Wrap_Struct(EVAL("GridFlow::Pointer"), 0, 0, new Pointer(ptr));
}
void *Pointer_get (Ruby rself) {
	DGS(Pointer);
	return self->p;
}

static Ruby make_error_message () {
	char buf[1000];
	sprintf(buf,"%s: %s",rb_class2name(rb_obj_class(ruby_errinfo)),
		rb_str_ptr(rb_funcall(ruby_errinfo,SI(to_s),0)));
	Ruby ary = rb_ary_new2(2);
	Ruby backtrace = rb_funcall(ruby_errinfo,SI(backtrace),0);
	rb_ary_push(ary,rb_str_new2(buf));
	for (int i=0; i<2 && i<rb_ary_len(backtrace); i++)
		rb_ary_push(ary,rb_funcall(backtrace,SI([]),1,INT2NUM(i)));
//	rb_ary_push(ary,rb_funcall(rb_funcall(backtrace,SI(length),0),SI(to_s),0));
	return ary;
}

static int ninlets_of (Ruby qlass) {
	if (!rb_ivar_defined(qlass,SYM2ID(syms->iv_ninlets))) RAISE("no inlet count");
	return INT(rb_ivar_get(qlass,SYM2ID(syms->iv_ninlets)));
}

static int noutlets_of (Ruby qlass) {
	if (!rb_ivar_defined(qlass,SYM2ID(syms->iv_noutlets))) RAISE("no outlet count");
	return INT(rb_ivar_get(qlass,SYM2ID(syms->iv_noutlets)));
}

#ifndef STACK_GROW_DIRECTION
#define STACK_GROW_DIRECTION -1
#endif
extern "C" void Init_stack(VALUE *addr);
static VALUE *localize_sysstack () {
	long bp;
	sscanf(RUBY_STACK_END,"0x%08lx",&bp);
	//fprintf(stderr,"old RUBY_STACK_END = %08lx\n",bp);
	// HACK (2004.08.29: alx has a problem; i hope it doesn't get worse)
	// this rounds to the last word of a 4k block
	// cross fingers that no other OS does it too different
	// !@#$ doesn't use STACK_GROW_DIRECTION
	// bp=((bp+0xfff)&~0xfff)-sizeof(void*);
	// GAAAH
	bp=((bp+0xffff)&~0xffff)-sizeof(void*);
	//fprintf(stderr,"new RUBY_STACK_END = %08lx\n",bp);
	return (VALUE *)bp;
}

//****************************************************************
// BFObject

struct BFObject : t_object {
	int32 magic; // paranoia
	Ruby rself;
	int nin;  // per object settings (not class)
	int nout; // per object settings (not class)
	t_outlet **out;

	void check_magic () {
		if (magic != OBJECT_MAGIC) {
			fprintf(stderr,"Object memory corruption! (ask the debugger)\n");
			for (int i=-1; i<=1; i++) {
				fprintf(stderr,"this[0]=0x%08x\n",((int*)this)[i]);
			}
			raise(11);
		}
	}
};

static t_class *find_bfclass (t_symbol *sym) {
	t_atom a[1];
	SETSYMBOL(a,sym);
	char buf[4096];
	if (sym==&s_list) strcpy(buf,"list"); else atom_string(a,buf,sizeof(buf));
	Ruby v = rb_hash_aref(rb_ivar_get(mGridFlow2, SI(@fclasses)), rb_str_new2(buf));
	if (v==Qnil) {
		post("GF: class not found: '%s'",buf);
		return 0;
	}
	if (Qnil==rb_ivar_get(v,SI(@bfclass))) {
		post("@bfclass missing for '%s'",buf);
		return 0;
	}
	return FIX2PTR(t_class,rb_ivar_get(v,SI(@bfclass)));
}

static t_class *BFProxy_class;

struct BFProxy : t_object {
	BFObject *parent;
	int inlet;
};

static void Bridge_export_value(Ruby arg, t_atom *at) {
	if (INTEGER_P(arg)) {
		SETFLOAT(at,NUM2INT(arg));
	} else if (SYMBOL_P(arg)) {
		const char *name = rb_sym_name(arg);
		SETSYMBOL(at,gensym((char *)name));
	} else if (FLOAT_P(arg)) {
		SETFLOAT(at,RFLOAT(arg)->value);
	} else if (rb_obj_class(arg)==mPointer) {
		SETPOINTER(at,(t_gpointer*)Pointer_get(arg));
	} else {
		RAISE("cannot convert argument of class '%s'",
			rb_str_ptr(rb_funcall(rb_funcall(arg,SI(class),0),SI(inspect),0)));
	}
}

static Ruby Bridge_import_value(const t_atom *at) {
	t_atomtype t = at->a_type;
	if (t==A_SYMBOL) {
		return ID2SYM(rb_intern(at->a_w.w_symbol->s_name));
	} else if (t==A_FLOAT) {
		return rb_float_new(at->a_w.w_float);
	} else if (t==A_POINTER) {
		return Pointer_s_new(at->a_w.w_gpointer);
		} else {
		return Qnil; /* unknown */
	}
}

static Ruby BFObject_method_missing_1 (FMessage *fm) {
	Ruby argv[fm->ac+2];
	argv[0] = INT2NUM(fm->winlet);
	argv[1] = ID2SYM(rb_intern(fm->selector->s_name));
	for (int i=0; i<fm->ac; i++) argv[2+i] = Bridge_import_value(fm->at+i);
	fm->self->check_magic();
	rb_funcall2(fm->self->rself,SI(send_in),fm->ac+2,argv);
	return Qnil;
}

static Ruby BFObject_rescue (FMessage *fm) {
	Ruby error_array = make_error_message();
//	for (int i=0; i<rb_ary_len(error_array); i++)
//		post("%s\n",rb_str_ptr(rb_ary_ptr(error_array)[i]));
	if (fm->self) pd_error(fm->self,"%s",rb_str_ptr(
		rb_funcall(error_array,SI(join),1,rb_str_new2("\n"))));
	if (fm->self && fm->is_init) fm->self = 0;
	return Qnil;
}

static void BFObject_method_missing (BFObject *bself,
int winlet, t_symbol *selector, int ac, t_atom *at) {
	FMessage fm = { bself, winlet, selector, ac, at, false };
	if (!bself->rself) {
		pd_error(bself,"message to a dead object. (supposed to be impossible)");
		return;
	}
	rb_rescue2(
		(RMethod)BFObject_method_missing_1,(Ruby)&fm,
		(RMethod)BFObject_rescue,(Ruby)&fm,
		rb_eException,0);
}

static void BFObject_method_missing0 (BFObject *self,
t_symbol *s, int argc, t_atom *argv) {
	BFObject_method_missing(self,0,s,argc,argv);
}

static void BFProxy_method_missing(BFProxy *self,
t_symbol *s, int argc, t_atom *argv) {
	BFObject_method_missing(self->parent,self->inlet,s,argc,argv);
}
	
static Ruby BFObject_init_1 (FMessage *fm) {
	Ruby argv[fm->ac+1];
	for (int i=0; i<fm->ac; i++) argv[i+1] = Bridge_import_value(fm->at+i);

	if (fm->selector==&s_list) {
		argv[0] = rb_str_new2("list"); // pd is slightly broken here
	} else {
		argv[0] = rb_str_new2(fm->selector->s_name);
	}

	Ruby rself = rb_funcall2(rb_const_get(mGridFlow2,SI(FObject)),SI([]),fm->ac+1,argv);
	DGS(FObject);
	self->bself = fm->self;
	self->bself->rself = rself;

	int ninlets  = self->bself->nin = ninlets_of(rb_funcall(rself,SI(class),0));
	int noutlets = self->bself->nout = noutlets_of(rb_funcall(rself,SI(class),0));

	for (int i=1; i<ninlets; i++) {
		BFProxy *p = (BFProxy *)pd_new(BFProxy_class);
		p->parent = self->bself;
		p->inlet = i;
		inlet_new(self->bself, &p->ob_pd, 0,0);
	}
	self->bself->out = new t_outlet*[noutlets];
	for (int i=0; i<noutlets; i++) {
		self->bself->out[i] = outlet_new(self->bself,&s_anything);
	}
	rb_funcall(rself,SI(initialize2),0);
	return rself;
}

static void *BFObject_init (t_symbol *classsym, int ac, t_atom *at) {
	t_class *qlass = find_bfclass(classsym);
	if (!qlass) return 0;
	BFObject *bself = (BFObject *)pd_new(qlass);
	bself->magic = OBJECT_MAGIC;
	bself->check_magic();
	FMessage fm = { self: bself, winlet:-1, selector: classsym,
		ac: ac, at: at, is_init: true };
	long r = rb_rescue2(
		(RMethod)BFObject_init_1,(Ruby)&fm,
		(RMethod)BFObject_rescue,(Ruby)&fm,
		rb_eException,0);
	return r==Qnil ? 0 : (void *)bself; // return NULL if broken object
}

static void BFObject_delete_1 (FMessage *fm) {
	fm->self->check_magic();
	if (fm->self->rself) {
		rb_funcall(fm->self->rself,SI(delete),0);
	} else {
		post("BFObject_delete is NOT handling BROKEN object at %08x",(int)fm);
	}
}

static void BFObject_delete (BFObject *bself) {
	bself->check_magic();
	FMessage fm = { self: bself, winlet:-1, selector: gensym("delete"),
		ac: 0, at: 0, is_init: false };
	rb_rescue2(
		(RMethod)BFObject_delete_1,(Ruby)&fm,
		(RMethod)BFObject_rescue,(Ruby)&fm,
		rb_eException,0);
	bself->magic = 0xDeadBeef;
}

/* **************************************************************** */

struct RMessage {
	VALUE rself;
	ID sel;
	int argc;
	VALUE *argv;
};

// this was called rb_funcall_rescue[...] but recently (ruby 1.8.2)
// got a conflict with a new function in ruby.

VALUE rb_funcall_myrescue_1(RMessage *rm) {
	return rb_funcall2(rm->rself,rm->sel,rm->argc,rm->argv);
}

static Ruby rb_funcall_myrescue_2 (RMessage *rm) {
	Ruby error_array = make_error_message();
//	for (int i=0; i<rb_ary_len(error_array); i++)
//		post("%s\n",rb_str_ptr(rb_ary_ptr(error_array)[i]));
	post("%s",rb_str_ptr(rb_funcall(error_array,SI(join),1,rb_str_new2("\n"))));
	return Qnil;
}

VALUE rb_funcall_myrescue(VALUE rself, ID sel, int argc, ...) {
	va_list foo;
	va_start(foo,argc);
	VALUE argv[argc];
	for (int i=0; i<argc; i++) argv[i] = va_arg(foo,VALUE);
	RMessage rm = { rself, sel, argc, argv };
	va_end(foo);
	return rb_rescue2(
		(RMethod)rb_funcall_myrescue_1,(Ruby)&rm,
		(RMethod)rb_funcall_myrescue_2,(Ruby)&rm,
		rb_eException,0);
}

/* Call this to get a gobj's bounding rectangle in pixels */
void bf_getrectfn(t_gobj *x, struct _glist *glist,
int *x1, int *y1, int *x2, int *y2) {
	BFObject *bself = (BFObject*)x;
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	Ruby a = rb_funcall_myrescue(bself->rself,SI(pd_getrect),1,can);
	if (TYPE(a)!=T_ARRAY || rb_ary_len(a)<4) {
		post("bf_getrectfn: return value should be 4-element array");
		*x1=*y1=*x2=*y2=0;
		return;
	}
	*x1 = INT(rb_ary_ptr(a)[0]);
	*y1 = INT(rb_ary_ptr(a)[1]);
	*x2 = INT(rb_ary_ptr(a)[2]);
	*y2 = INT(rb_ary_ptr(a)[3]);
}

/* and this to displace a gobj: */
void bf_displacefn(t_gobj *x, struct _glist *glist, int dx, int dy) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	BFObject *bself = (BFObject *)x;
	bself->te_xpix+=dx;
	bself->te_ypix+=dy;
	rb_funcall_myrescue(bself->rself,SI(pd_displace),3,can,INT2NUM(dx),INT2NUM(dy));
	canvas_fixlinesfor(glist, (t_text *)x);
}

/* change color to show selection: */
void bf_selectfn(t_gobj *x, struct _glist *glist, int state) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_select),2,can,INT2NUM(state));
}

/* change appearance to show activation/deactivation: */
void bf_activatefn(t_gobj *x, struct _glist *glist, int state) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_activate),2,can,INT2NUM(state));
}

/* warn a gobj it's about to be deleted */
void bf_deletefn(t_gobj *x, struct _glist *glist) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_delete),1,can);
	canvas_deletelinesfor(glist, (t_text *)x);
}

/*  making visible or invisible */
void bf_visfn(t_gobj *x, struct _glist *glist, int flag) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	Ruby rself = ((BFObject*)x)->rself;
	DGS(FObject);
	self->check_magic();
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_vis),2,can,INT2NUM(flag));
}

/* field a mouse click (when not in "edit" mode) */
int bf_clickfn(t_gobj *x, struct _glist *glist,
int xpix, int ypix, int shift, int alt, int dbl, int doit) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	Ruby ret = rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_click),7,can,
		INT2NUM(xpix),INT2NUM(ypix),
		INT2NUM(shift),INT2NUM(alt),
		INT2NUM(dbl),INT2NUM(doit));
	if (TYPE(ret) == T_FIXNUM) return INT(ret);
	post("bf_clickfn: expected Fixnum");
	return 0;
}

/* save to a binbuf */
void bf_savefn(t_gobj *x, t_binbuf *b) {
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_save),1,Qnil);
}

/* open properties dialog */
void bf_propertiesfn(t_gobj *x, struct _glist *glist) {
	Ruby can = PTR2FIX(glist_getcanvas(glist));
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_properties),1,can);
}

/* get keypresses during focus */
void bf_keyfn(void *x, t_floatarg fkey) {
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_key),1,INT2NUM((int)fkey));
}

/* get motion diff during focus */
void bf_motionfn(void *x, t_floatarg dx, t_floatarg dy) {
	rb_funcall_myrescue(((BFObject*)x)->rself,SI(pd_motion),2,
		INT2NUM((int)dx), INT2NUM((int)dy));
}

/* **************************************************************** */

static void BFObject_class_init_1 (t_class *qlass) {
	class_addanything(qlass,(t_method)BFObject_method_missing0);
}

\class FObject

static Ruby FObject_s_install2(Ruby rself, Ruby name) {
	if (TYPE(name)!=T_STRING) RAISE("name must be String");
	t_class *qlass = class_new(gensym(rb_str_ptr(name)),
		(t_newmethod)BFObject_init, (t_method)BFObject_delete,
		sizeof(BFObject), CLASS_DEFAULT, A_GIMME,0);
	rb_ivar_set(rself, SI(@bfclass), PTR2FIX(qlass));
	FMessage fm = {0, -1, 0, 0, 0, false};
	rb_rescue2(
		(RMethod)BFObject_class_init_1,(Ruby)qlass,
		(RMethod)BFObject_rescue,(Ruby)&fm,
		rb_eException,0);
	return Qnil;
}

static Ruby FObject_send_out2(int argc, Ruby *argv, Ruby rself) {
//\def Ruby send_out2(...) {
	DGS(FObject);
	BFObject *bself = self->bself;
	if (!bself) {
		//post("FObject#send_out2 : bself is NULL, rself=%08x",rself);
		return Qnil;
	}
	bself->check_magic();
	Ruby qlass = rb_funcall(rself,SI(class),0);
	int outlet = INT(argv[0]);
	Ruby sym = argv[1];
	argc-=2;
	argv+=2;
	t_atom sel, at[argc];
	Bridge_export_value(sym,&sel);
	for (int i=0; i<argc; i++) Bridge_export_value(argv[i],at+i);
	t_outlet *out = bself->te_outlet;
	outlet_anything(bself->out[outlet],atom_getsymbol(&sel),argc,at);
	return Qnil;
}

static Ruby FObject_s_set_help (Ruby rself, Ruby path) {
	path = rb_funcall(path,SI(to_s),0);
	Ruby qlassid = rb_ivar_get(rself,SI(@bfclass));
	if (qlassid==Qnil) RAISE("@bfclass==nil ???");
	t_class *qlass = FIX2PTR(t_class,qlassid);
	class_sethelpsymbol(qlass,gensym(rb_str_ptr(path)));
	return Qnil;
}

static Ruby GridFlow_s_gui (int argc, Ruby *argv, Ruby rself) {
	if (argc!=1) RAISE("bad args");
	Ruby command = rb_funcall(argv[0],SI(to_s),0);
	sys_gui(rb_str_ptr(command));
	return Qnil;
}

static Ruby GridFlow_s_bind (Ruby rself, Ruby argv0, Ruby argv1) {
	if (TYPE(argv0)==T_STRING) {
#if PD_VERSION_INT < 37
	RAISE("requires Pd 0.37");
#else
		Ruby name = rb_funcall(argv0,SI(to_s),0);
		Ruby qlassid = rb_ivar_get(rb_hash_aref(rb_ivar_get(mGridFlow2,SI(@fclasses)),name),SI(@bfclass));
		if (qlassid==Qnil) RAISE("no such class: %s",rb_str_ptr(name));
		pd_typedmess(&pd_objectmaker,gensym(rb_str_ptr(name)),0,0);
		t_pd *o = pd_newest();
		pd_bind(o,gensym(rb_str_ptr(argv1)));
#endif
	} else {
		Ruby rself = argv0;
		DGS(FObject);
		t_symbol *s = gensym(rb_str_ptr(argv1));
		t_pd *o = (t_pd *)(self->bself);
		//fprintf(stderr,"binding %08x to: \"%s\" (%08x %s)\n",o,rb_str_ptr(argv[1]),s,s->s_name);
		pd_bind(o,s);
	}
	return Qnil;
}

static Ruby FObject_s_gui_enable (Ruby rself) {
	Ruby qlassid = rb_ivar_get(rself,SI(@bfclass));
	if (qlassid==Qnil) RAISE("no class id ?");
	t_widgetbehavior *wb = new t_widgetbehavior;
	wb->w_getrectfn    = bf_getrectfn;
	wb->w_displacefn   = bf_displacefn;
	wb->w_selectfn     = bf_selectfn;
	wb->w_activatefn   = bf_activatefn;
	wb->w_deletefn     = bf_deletefn;
	wb->w_visfn        = bf_visfn;
	wb->w_clickfn      = bf_clickfn;
	//wb->w_savefn       = bf_savefn;
	//wb->w_propertiesfn = bf_propertiesfn;
	class_setwidget(FIX2PTR(t_class,qlassid),wb);
	return Qnil;
}

static Ruby FObject_focus (Ruby rself, Ruby canvas_, Ruby x_, Ruby y_) {
	DGS(FObject);
	t_glist *canvas = FIX2PTR(t_glist,canvas_);
	t_gobj  *bself = (t_gobj *)(self->bself);
	int x = INT(x_);
	int y = INT(y_);
	glist_grab(canvas,bself, bf_motionfn, bf_keyfn, x,y);
	return Qnil;
}

// doesn't use rself but still is aside FObject_focus for symmetry reasons.
static Ruby FObject_unfocus (Ruby rself, Ruby canvas_) {
	DGS(FObject);
	t_glist *canvas = FIX2PTR(t_glist,canvas_);
	glist_grab(canvas,0,0,0,0,0);
	return Qnil;
}

static Ruby FObject_add_inlets (Ruby rself, Ruby n_) {
	DGS(FObject);
	if (!self->bself) RAISE("there is no bself");
	int n = INT(n_);
	for (int i=self->bself->nin; i<self->bself->nin+n; i++) {
		BFProxy *p = (BFProxy *)pd_new(BFProxy_class);
		p->parent = self->bself;
		p->inlet = i;
		inlet_new(self->bself, &p->ob_pd, 0,0);
	}
	self->bself->nin+=n;
	return Qnil;
}

static Ruby FObject_add_outlets (Ruby rself, Ruby n_) {
	DGS(FObject);
	if (!self->bself) RAISE("there is no bself");
	int n = INT(n_);
	t_outlet **oldouts = self->bself->out;
	self->bself->out = new t_outlet*[self->bself->nout+n];
	memcpy(self->bself->out,oldouts,self->bself->nout*sizeof(t_outlet*));
	for (int i=self->bself->nout; i<self->bself->nout+n; i++) {
		self->bself->out[i] = outlet_new(self->bself,&s_anything);
	}
	self->bself->nout+=n;
	return Qnil;
}

static Ruby bridge_add_to_menu (int argc, Ruby *argv, Ruby rself) {
	if (argc!=1) RAISE("bad args");
	Ruby name = rb_funcall(argv[0],SI(to_s),0);
	Ruby qlassid = rb_ivar_get(rb_hash_aref(rb_ivar_get(mGridFlow2,SI(@fclasses)),name),SI(@bfclass));
	if (qlassid==Qnil) RAISE("no such class: %s",rb_str_ptr(name));
	//!@#$
	return Qnil;
}

static Ruby GridFlow_s_add_creator_2 (Ruby rself, Ruby name_) {
	t_symbol *name = gensym(rb_str_ptr(rb_funcall(name_,SI(to_s),0)));
	class_addcreator((t_newmethod)BFObject_init,name,A_GIMME,0);
	return Qnil;
}

static Ruby FObject_get_position (Ruby rself, Ruby canvas) {
	DGS(FObject);
	t_text *bself = (t_text *)(self->bself);
	t_glist *c = FIX2PTR(t_glist,canvas);
	float x0,y0;
	if (c->gl_havewindow || !c->gl_isgraph) {
		x0=bself->te_xpix;
		y0=bself->te_ypix;
	}
	else { // graph-on-parent: have to zoom
		float zx = float(c->gl_x2 - c->gl_x1) / (c->gl_screenx2 - c->gl_screenx1);
		float zy = float(c->gl_y2 - c->gl_y1) / (c->gl_screeny2 - c->gl_screeny1);
		x0 = glist_xtopixels(c, c->gl_x1 + bself->te_xpix*zx);
		y0 = glist_ytopixels(c, c->gl_y1 + bself->te_ypix*zy);
	}
	Ruby a = rb_ary_new();
	rb_ary_push(a,INT2NUM((int)x0));
	rb_ary_push(a,INT2NUM((int)y0));
	return a;
}

\classinfo {}
\end class FObject

//****************************************************************

\class Clock < CObject
struct Clock : CObject {
	t_clock *serf;
	Ruby owner; /* copy of ptr that serf already has, for marking */
	\decl void set  (double   systime);
	\decl void delay(double delaytime);
	\decl void unset();
};

void Clock_fn (Ruby rself) { rb_funcall_myrescue(rself,SI(call),0); }
void Clock_mark (Clock *self) { rb_gc_mark(self->owner); }
void Clock_free (Clock *self) { clock_free(self->serf); CObject_freeee(self); }

Ruby Clock_s_new (Ruby qlass, Ruby owner) {
	Clock *self = new Clock();
	self->rself = Data_Wrap_Struct(qlass, Clock_mark, Clock_free, self);
	self->serf = clock_new((void*)owner,(t_method)Clock_fn);
	self->owner = owner;
	return self->rself;
}

\def void set  (double   systime) {   clock_set(serf,  systime); }
\def void delay(double delaytime) { clock_delay(serf,delaytime); }
\def void unset() { clock_unset(serf); }

\classinfo {}
\end class Clock

//****************************************************************

Ruby GridFlow_s_post_string (Ruby rself, Ruby string) {
	if (TYPE(string)!=T_STRING) RAISE("not a string!");
	post("%s",rb_str_ptr(string));
	return Qnil;
}

#define SDEF(_name1_,_name2_,_argc_) \
	rb_define_singleton_method(mGridFlow2,_name1_,(RMethod)GridFlow_s_##_name2_,_argc_)

Ruby gf_bridge_init (Ruby rself) {
	Ruby ver = EVAL("GridFlow::GF_VERSION");
	if (strcmp(rb_str_ptr(ver), GF_VERSION) != 0) {
		RAISE("GridFlow version mismatch: "
			"main library is '%s'; bridge is '%s'",
			rb_str_ptr(ver), GF_VERSION);
	}
	syms = FIX2PTR(BuiltinSymbols,rb_ivar_get(mGridFlow2,SI(@bsym)));
	Ruby fo = EVAL("GridFlow::FObject");
	rb_define_singleton_method(fo,"install2",(RMethod)FObject_s_install2,1);
	rb_define_singleton_method(fo,"gui_enable", (RMethod)FObject_s_gui_enable, 0);
	rb_define_singleton_method(fo,"set_help", (RMethod)FObject_s_set_help, 1);
	rb_define_method(fo,"get_position",(RMethod)FObject_get_position,1);
	rb_define_method(fo,"send_out2",   (RMethod)FObject_send_out2,-1);
	rb_define_method(fo,"send_out2",   (RMethod)FObject_send_out2,-1);
	rb_define_method(fo,"add_inlets",  (RMethod)FObject_add_inlets,  1);
	rb_define_method(fo,"add_outlets", (RMethod)FObject_add_outlets, 1);
	rb_define_method(fo,"unfocus",     (RMethod)FObject_unfocus, 1);
	rb_define_method(fo,  "focus",     (RMethod)FObject_focus,   3);

	SDEF("post_string",post_string,1);
	SDEF("add_creator_2",add_creator_2,1);
	SDEF("gui",gui,-1);
	SDEF("bind",bind,2);
	// SDEF("add_to_menu",add_to_menu,-1);

	\startall
	rb_define_singleton_method(EVAL("GridFlow::Clock"  ),"new", (RMethod)Clock_s_new, 1);
	rb_define_singleton_method(EVAL("GridFlow::Pointer"),"new", (RMethod)Pointer_s_new, 1);
	mPointer = EVAL("GridFlow::Pointer");
	EVAL("class<<GridFlow;attr_accessor :config; end");
	EVAL("GridFlow.config = {'PUREDATA_PATH' => %{"PUREDATA_PATH"}}");
	return Qnil;
}

struct BFGridFlow : t_object {};

t_class *bindpatcher;
static void *bindpatcher_init (t_symbol *classsym, int ac, t_atom *at) {
	t_pd *bself = pd_new(bindpatcher);
	if (ac!=1 || at->a_type != A_SYMBOL) {
		post("bindpatcher: oops");
	} else {
		t_symbol *s = atom_getsymbol(at);
		post("binding patcher to: %s",s->s_name);
		pd_bind((t_pd *)canvas_getcurrent(),s);
	}
	return bself;
}

extern "C" void gridflow_setup () {
	char *foo[] = {"Ruby-for-PureData","-w","-e",";"};
	post("setting up Ruby-for-PureData...");
/*
	post("pd_getfilename() = %s", pd_getfilename()->s_name);
	post("pd_getdirname() = %s", pd_getdirname()->s_name);
	post("canvas_getcurrentdir() = %p", canvas_getcurrentdir());
	char *dirresult = new char[242];
	char *nameresult;
	int fd = open_via_path("","gridflow",".so",dirresult,&nameresult,242,1);
	post("open_via_path: fd=%d dirresult=\"%s\" nameresult=\"%s\"",fd,dirresult,nameresult);
	delete[] dirresult;
*/
	ruby_init();
	Init_stack(localize_sysstack());
	ruby_options(COUNT(foo),foo);
	post("we are using Ruby version %s",rb_str_ptr(EVAL("RUBY_VERSION")));
	Ruby cData = rb_const_get(rb_cObject,SI(Data));
	BFProxy_class = class_new(gensym("ruby_proxy"),
		NULL,NULL,sizeof(BFProxy),CLASS_PD|CLASS_NOINLET, A_NULL);
	class_addanything(BFProxy_class,BFProxy_method_missing);
	rb_define_singleton_method(cData,"gf_bridge_init",
		(RMethod)gf_bridge_init,0);

	mGridFlow2 = EVAL(
		"module GridFlow; class<<self; attr_reader :bridge_name; end; "
		"@bridge_name = 'puredata'; self end");
	post("(done)");
	if (!
	EVAL("begin require 'gridflow'; true; rescue Exception => e;\
		STDERR.puts \"[#{e.class}] [#{e.message}]:\n#{e.backtrace.join'\n'}\"; false; end"))
	{
		post("ERROR: Cannot load GridFlow-for-Ruby (gridflow.so)\n");
		return;
	}
	bindpatcher = class_new(gensym("bindpatcher"),
		(t_newmethod)bindpatcher_init, 0, sizeof(t_object),CLASS_DEFAULT,A_GIMME,0);
}


