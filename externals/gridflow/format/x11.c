/*
	$Id: x11.c,v 1.2 2006-03-15 04:37:46 matju Exp $

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

	Note: some of the code was adapted from PDP's (the XVideo stuff).
*/
#include "../base/grid.h.fcs"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/StringDefs.h>
#ifdef HAVE_X11_SHARED_MEMORY
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif
#ifdef HAVE_X11_XVIDEO
#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>
#endif

#undef L
#define L gfpost("%s:%d in %s",__FILE__,__LINE__,__PRETTY_FUNCTION__);

/* X11 Error Handler type */
typedef int (*XEH)(Display *, XErrorEvent *);

\class FormatX11 < Format
struct FormatX11 : Format {
/* at the Display/Screen level */
	Display *display; /* connection to xserver */
	Visual *visual;   /* screen properties */
	Window root_window;
	Colormap colormap; /* for 256-color mode */
	short depth;
	int transfer;	   /* 0=plain 1=xshm 2=xvideo */
	bool use_stripes;  /* use alternate conversion in 256-color mode */
/* at the Window level */
	Window window;       /* X11 window number */
	Window parent;       /* X11 window number of the parent */
	GC imagegc;          /* X11 graphics context (like java.awt.Graphics) */
	XImage *ximage;      /* X11 image descriptor */
	Pt<uint8> image;     /* the real data (that XImage binds to) */
	bool is_owner;
	int32 pos[2];
	P<BitPacking> bit_packing;
	P<Dim> dim;
	bool lock_size;
	bool override_redirect;
#ifdef HAVE_X11_SHARED_MEMORY
	XShmSegmentInfo *shm_info; /* to share memory with X11/Unix */
#endif
#ifdef HAVE_X11_XVIDEO
    int xv_format;
    int xv_port;
    XvImage *xvi;
    unsigned char *data;
    int last_encoding;
#endif
	FormatX11 () : transfer(0), use_stripes(false), 
	window(0), ximage(0), image(Pt<uint8>()), is_owner(true),
	dim(0), lock_size(false), override_redirect(false)
#ifdef HAVE_X11_SHARED_MEMORY
		, shm_info(0)
#endif
	{}
	template <class T> void frame_by_type (T bogus);
	void show_section(int x, int y, int sx, int sy);
	void set_wm_hints ();
	void dealloc_image ();
	bool alloc_image (int sx, int sy);
	void resize_window (int sx, int sy);
	void open_display(const char *disp_string);
	void report_pointer(int y, int x, int state);
	void prepare_colormap();
	Window FormatX11::search_window_tree (Window xid, Atom key, const char *value, int level=0);
	\decl void initialize (...);
	\decl void frame ();
	\decl void close ();
	\decl void call ();
	\decl void _0_out_size (int sy, int sx);
	\decl void _0_setcursor (int shape);
	\decl void _0_hidecursor ();
	\decl void _0_set_geometry (int y, int x, int sy, int sx);
	\decl void _0_fall_thru (int flag);
	\decl void _0_transfer (Symbol s);
	\decl void _0_title (String s=Qnil);
	\grin 0 int
};

/* ---------------------------------------------------------------- */

static const char *xfers[3] = {"plain","xshm","xvideo"};

void FormatX11::show_section(int x, int y, int sx, int sy) {
	int zy=dim->get(0), zx=dim->get(1);
	if (y>zy||x>zx) return;
	if (y+sy>zy) sy=zy-y;
	if (x+sx>zx) sx=zx-x;
	switch (transfer) {
	case 0: XPutImage(display,window,imagegc,ximage,x,y,x,y,sx,sy);
		XFlush(display);
	break;
#ifdef HAVE_X11_SHARED_MEMORY
	case 1:	XSync(display,False);
		XShmPutImage(display,window,imagegc,ximage,x,y,x,y,sx,sy,False);
		XFlush(display);
		//XPutImage( display,window,imagegc,ximage,x,y,x,y,sx,sy);
		// should completion events be waited for? looks like a bug
		break;
#endif
#ifdef HAVE_X11_XVIDEO
	case 2:
	break;
#endif
	default: RAISE("transfer mode '%s' not available", xfers[transfer]);
	}
}

/* window manager hints, defines the window as non-resizable */
void FormatX11::set_wm_hints () {
	Ruby title = rb_ivar_get(rself,SI(@title));
	if (!is_owner) return;
	XWMHints wmh;
	char buf[256],*bufp=buf;
	if (title==Qnil) {
		sprintf(buf,"GridFlow (%d,%d,%d)",dim->get(0),dim->get(1),dim->get(2));
	} else {
		sprintf(buf,"%.255s",rb_str_ptr(title));
	}
	XTextProperty wtitle; XStringListToTextProperty((char **)&bufp, 1, &wtitle);
	XSizeHints sh;
	sh.flags=PSize|PMaxSize|PMinSize;
	sh.min_width  = sh.max_width  = sh.width  = dim->get(1);
	sh.min_height = sh.max_height = sh.height = dim->get(0);
	wmh.input = True;
	wmh.flags = InputHint;
	XSetWMProperties(display,window,&wtitle,&wtitle,0,0,&sh,&wmh,0);
}

void FormatX11::report_pointer(int y, int x, int state) {
	Ruby argv[5] = {
		INT2NUM(0), SYM(position),
		INT2NUM(y), INT2NUM(x), INT2NUM(state) };
	send_out(COUNT(argv),argv);
}

\def void call() {
	XEvent e;
	for (;;) {
		int xpending = XEventsQueued(display, QueuedAfterFlush);
		if (!xpending) break;
		XNextEvent(display,&e);
		switch (e.type) {
		case Expose:{
			XExposeEvent *ex = (XExposeEvent *)&e;
			if (rb_ivar_get(rself,SI(@mode)) == SYM(out)) {
				show_section(ex->x,ex->y,ex->width,ex->height);
			}
		}break;
		case ButtonPress:{
			XButtonEvent *eb = (XButtonEvent *)&e;
			eb->state |= 128<<eb->button;
			report_pointer(eb->y,eb->x,eb->state);
		}break;
		case ButtonRelease:{
			XButtonEvent *eb = (XButtonEvent *)&e;
			eb->state &= ~(128<<eb->button);
			report_pointer(eb->y,eb->x,eb->state);
		}break;
		case KeyPress:
		case KeyRelease:{
			XKeyEvent *ek = (XKeyEvent *)&e;
			//XLookupString(ek, buf, 63, 0, 0);
			char *kss = XKeysymToString(XLookupKeysym(ek, 0));
			char buf[64];
			if (!kss) return; /* unknown keys ignored */
			if (isdigit(*kss)) sprintf(buf,"D%s",kss); else strcpy(buf,kss);
			Ruby argv[6] = {
				INT2NUM(0), e.type==KeyPress ? SYM(keypress) : SYM(keyrelease),
				INT2NUM(ek->y), INT2NUM(ek->x), INT2NUM(ek->state),
				rb_funcall(rb_str_new2(buf),SI(intern),0) };
			send_out(COUNT(argv),argv);
			//XFree(kss);
		}break;
		case MotionNotify:{
			XMotionEvent *em = (XMotionEvent *)&e;
			report_pointer(em->y,em->x,em->state);
		}break;
		case DestroyNotify:{
			gfpost("This window is being closed, so this handler will close too!");
			rb_funcall(rself,SI(close),0);
			return;
		}break;
		case ConfigureNotify:break; // as if we cared
		}
	}
	IEVAL(rself,"@clock.delay 20");
}

\def void frame () {
	XGetSubImage(display, window, 0, 0, dim->get(1), dim->get(0),
		(unsigned)-1, ZPixmap, ximage, 0, 0);
	GridOutlet out(this,0,dim,NumberTypeE_find(rb_ivar_get(rself,SI(@cast))));
	int sy=dim->get(0), sx=dim->get(1), bs=dim->prod(1);
	STACK_ARRAY(uint8,b2,bs);
	for(int y=0; y<sy; y++) {
		Pt<uint8> b1 = Pt<uint8>(image,ximage->bytes_per_line*dim->get(0))
			+ ximage->bytes_per_line * y;
		bit_packing->unpack(sx,b1,b2);
		out.send(bs,b2);
	}
}

/* loathe Xlib's error handlers */
static FormatX11 *current_x11;
static int FormatX11_error_handler (Display *d, XErrorEvent *xee) {
	gfpost("XErrorEvent: type=0x%08x display=0x%08x xid=0x%08x",
		xee->type, xee->display, xee->resourceid);
	gfpost("... serial=0x%08x error=0x%08x request=0x%08lx minor=0x%08x",
		xee->serial, xee->error_code, xee->request_code, xee->minor_code);
	if (current_x11->transfer==1) {
		gfpost("(note: turning shm off)");
		current_x11->transfer = 0;
	}
	return 42; /* it seems that the return value is ignored. */
}

bool FormatX11::alloc_image (int sx, int sy) {
	dim = new Dim(sy,sx,3);
	dealloc_image();
	if (sx==0 || sy==0) return false;
	current_x11 = this;
	switch (transfer) {
	case 0: {
		ximage = XCreateImage(display,visual,depth,ZPixmap,0,0,sx,sy,8,0);
		int size = ximage->bytes_per_line*ximage->height;
		if (!ximage) RAISE("can't create image"); 
		image = ARRAY_NEW(uint8,size);
		ximage->data = (int8 *)image;
	} break;
#ifdef HAVE_X11_SHARED_MEMORY
	case 1: {
		shm_info = new XShmSegmentInfo;
		ximage = XShmCreateImage(display,visual,depth,ZPixmap,0,shm_info,sx,sy);
                if (!ximage) {gfpost("shm got disabled, retrying..."); transfer=0;}
		XSync(display,0);
		if (transfer==0) return alloc_image(sx,sy);
		int size = ximage->bytes_per_line*ximage->height;
		gfpost("size = %d",size);
		shm_info->shmid = shmget(IPC_PRIVATE,size,IPC_CREAT|0777);
		if(shm_info->shmid < 0) RAISE("shmget() failed: %s",strerror(errno));
		ximage->data = shm_info->shmaddr = (char *)shmat(shm_info->shmid,0,0);
		if ((long)(shm_info->shmaddr) == -1) RAISE("shmat() failed: %s",strerror(errno));
		gfpost("shmaddr=%p",shm_info->shmaddr);
		image = Pt<uint8>((uint8 *)ximage->data,size);
		shm_info->readOnly = False;
		if (!XShmAttach(display, shm_info)) RAISE("ERROR: XShmAttach: big problem");
		XSync(display,0); // make sure the server picks it up
		// yes, this can be done now. should cause auto-cleanup.
		shmctl(shm_info->shmid,IPC_RMID,0);
		if (transfer==0) return alloc_image(sx,sy);
	} break;
#endif
#ifdef HAVE_X11_XVIDEO
	case 2: {
	unsigned int ver, rel, req, ev, err, i, j, adaptors, formats;
	XvAdaptorInfo *ai;
	if (Success != XvQueryExtension(display,&ver,&rel,&req,&ev,&err)) RAISE("XvQueryExtension problem");
	/* find + lock port */
	if (Success != XvQueryAdaptors(display,DefaultRootWindow(display),&adaptors,&ai)) RAISE("XvQueryAdaptors problem");
	for (i = 0; i < adaptors; i++) {
		if (ai[i].type&XvInputMask && ai[i].type&XvImageMask) {
			for (j=0; j<ai[i].num_ports; j++) {
				if (Success != XvGrabPort(display,ai[i].base_id+j,CurrentTime)) RAISE("XvGrabPort problem");
				xv_port = ai[i].base_id + j;
				goto breakout;
			}
		}
	}
	breakout:
	XFree(ai);
	if (!xv_port) RAISE("no xv_port");
/*	unsigned int encn;
	XvEncodingInfo *enc;
	XvQueryEncodings(display,xv_port,&encn,&enc);
	for (i=0; i<encn; i++) gfpost("XvEncodingInfo: name='%s' encoding_id=0x%08x",enc[i].name,enc[i].encoding_id);*/
	gfpost("pdp_xvideo: grabbed port %d on adaptor %d",xv_port,i);
	size_t size = sx*sy*4;
	data = new uint8[size];
	for (i=0; i<size; i++) data[i]=0;
	xvi = XvCreateImage(display,xv_port,0x51525762,(char *)data,sx,sy);
	last_encoding=-1;
	if (!xvi) RAISE("XvCreateImage problem");
	} break;
#endif
	default: RAISE("transfer mode '%s' not available", xfers[transfer]);
	}
	int status = XInitImage(ximage);
	if (status!=1) gfpost("XInitImage returned: %d", status);
	return true;
retry:
	gfpost("shm got disabled, retrying...");
	return alloc_image(sx,sy);
}

void FormatX11::dealloc_image () {
	if (!ximage) return;
	switch (transfer) {
	case 0: XFree(ximage); ximage=0; image=Pt<uint8>(); break;
#ifdef HAVE_X11_SHARED_MEMORY
	case 1:
		shmdt(ximage->data);
		XShmDetach(display,shm_info);
		if (shm_info) {delete shm_info; shm_info=0;}
		XFree(ximage);
		ximage = 0;
		image = Pt<uint8>();
	break;
#endif
#ifdef HAVE_X11_XVIDEO
	case 2: {
		if (data) delete[] data;
		if (xvi) XFree(xvi);
		xvi=0;
		data=0;
	}
	break;
#endif
	default: RAISE("transfer mode '%s' not available",xfers[transfer]);
	}
}

void FormatX11::resize_window (int sx, int sy) {
	if (sy<16) sy=16; if (sy>4096) RAISE("height too big");
	if (sx<16) sx=16; if (sx>4096) RAISE("width too big");
	alloc_image(sx,sy);
	if (window) {
		if (is_owner && !lock_size) {
			set_wm_hints();
			XResizeWindow(display,window,sx,sy);
		}
	} else {
		XSetWindowAttributes xswa;
		xswa.do_not_propagate_mask = 0; //?
		xswa.override_redirect = override_redirect; //#!@#$
		window = XCreateWindow(display,
			parent, pos[1], pos[0], sx, sy, 0,
			CopyFromParent, InputOutput, CopyFromParent,
			CWOverrideRedirect|CWDontPropagate, &xswa);
		if(!window) RAISE("can't create window");
		set_wm_hints();
		_0_fall_thru(0,0,is_owner);
		if (is_owner) XMapRaised(display, window);
		imagegc = XCreateGC(display, window, 0, NULL);
		if (visual->c_class == PseudoColor) prepare_colormap(); 
	}
	XSync(display,0);
}

GRID_INLET(FormatX11,0) {
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2)!=3 && in->dim->get(2)!=4)
		RAISE("expecting 3 or 4 channels: red,green,blue,ignored (got %d)",in->dim->get(2));
	int sxc = in->dim->prod(1);
	int sx = in->dim->get(1), osx = dim->get(1);
	int sy = in->dim->get(0), osy = dim->get(0);
	in->set_factor(sxc);
	if (sx!=osx || sy!=osy) resize_window(sx,sy);
	if (in->dim->get(2)!=bit_packing->size) {
		bit_packing->mask[3]=0;
		bit_packing = new BitPacking(bit_packing->endian,
		  bit_packing->bytes, in->dim->get(2), bit_packing->mask);
	}
} GRID_FLOW {
	int bypl = ximage->bytes_per_line;
	int sxc = in->dim->prod(1);
	int sx = in->dim->get(1);
	int y = in->dex/sxc;
	int oy = y;
	for (; n>0; y++, data+=sxc, n-=sxc) {
		// convert line
		if (use_stripes) {
			int o=y*bypl;
			for (int x=0, i=0, k=y%3; x<sx; x++, i+=3, k=(k+1)%3) {
				image[o+x] = (k<<6) | data[i+k]>>2;
			}
		} else {
			bit_packing->pack(sx, data, image+y*bypl);
		}
	}
} GRID_FINISH {
	show_section(0,0,in->dim->get(1),in->dim->get(0));
} GRID_END

\def void close () {
	if (!this) RAISE("stupid error: trying to close display NULL. =)");
	bit_packing=0;
	IEVAL(rself,"@clock.unset");
	if (is_owner) XDestroyWindow(display,window);
	XSync(display,0);
	dealloc_image();
	XCloseDisplay(display);
	display=0;
	rb_call_super(argc,argv);
}

\def void _0_out_size (int sy, int sx) { resize_window(sx,sy); }

\def void _0_setcursor (int shape) {
	shape = 2*(shape&63);
	Cursor c = XCreateFontCursor(display,shape);
	XDefineCursor(display,window,c);
	XFlush(display);
}

\def void _0_hidecursor () {
	Font font = XLoadFont(display,"fixed");
	XColor color; /* bogus */
	Cursor c = XCreateGlyphCursor(display,font,font,' ',' ',&color,&color);
	XDefineCursor(display,window,c);
	XFlush(display);
}

void FormatX11::prepare_colormap() {
	Colormap colormap = XCreateColormap(display,window,visual,AllocAll);
	XColor colors[256];
	if (use_stripes) {
		for (int i=0; i<192; i++) {
			int k=(i&63)*0xffff/63;
			colors[i].pixel = i;
			colors[i].red   = (i>>6)==0 ? k : 0;
			colors[i].green = (i>>6)==1 ? k : 0;
			colors[i].blue  = (i>>6)==2 ? k : 0;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
		XStoreColors(display,colormap,colors,192);
	} else {	
		for (int i=0; i<256; i++) {
			colors[i].pixel = i;
			colors[i].red   = ((i>>0)&7)*0xffff/7;
			colors[i].green = ((i>>3)&7)*0xffff/7;
			colors[i].blue  = ((i>>6)&3)*0xffff/3;
			colors[i].flags = DoRed | DoGreen | DoBlue;
		}
		XStoreColors(display,colormap,colors,256);
	}
	XSetWindowColormap(display,window,colormap);
}

void FormatX11::open_display(const char *disp_string) {
	display = XOpenDisplay(disp_string);
	if(!display) RAISE("ERROR: opening X11 display: %s",strerror(errno));
	// btw don't expect too much from Xlib error handling.
	// Xlib, you are so free of the ravages of intelligence...
	XSetErrorHandler(FormatX11_error_handler);
	Screen *screen = DefaultScreenOfDisplay(display);
	int screen_num = DefaultScreen(display);
	visual   = DefaultVisual(display, screen_num);
	root_window = DefaultRootWindow(display);
	depth    = DefaultDepthOfScreen(screen);
	colormap = 0;

	switch(visual->c_class) {
	// without colormap
	case TrueColor: case DirectColor: break;
	// with colormap
	case PseudoColor: if (depth!=8) RAISE("ERROR: with colormap, only supported depth is 8 (got %d)", depth); break;
	default: RAISE("ERROR: visual type not supported (got %d)", visual->c_class);
	}

#if defined(HAVE_X11_XVIDEO)
	transfer = 2;
#elif defined(HAVE_X11_SHARED_MEMORY)
	transfer = !! XShmQueryExtension(display);
#else
	transfer = 0;
#endif
}

Window FormatX11::search_window_tree (Window xid, Atom key, const char *value, int level) {
	if (level>2) return 0xDeadBeef;
	Window root_r, parent_r;
	Window *children_r;
	unsigned int nchildren_r;
	XQueryTree(display,xid,&root_r,&parent_r,&children_r,&nchildren_r);
	Window target = 0xDeadBeef;
	for (int i=0; i<(int)nchildren_r; i++) {
		Atom actual_type_r;
		int actual_format_r;
		unsigned long nitems_r, bytes_after_r;
		unsigned char *prop_r;
		XGetWindowProperty(display,children_r[i],key,0,666,0,AnyPropertyType,
		&actual_type_r,&actual_format_r,&nitems_r,&bytes_after_r,&prop_r);
		uint32 value_l = strlen(value);
		bool match = prop_r && nitems_r>=value_l &&
			strncmp((char *)prop_r+nitems_r-value_l,value,value_l)==0;
		XFree(prop_r);
		if (match) {target=children_r[i]; break;}
		target = search_window_tree(children_r[i],key,value,level+1);
		if (target != 0xDeadBeef) break;
	}
	if (children_r) XFree(children_r);
	return target;
}

\def void _0_set_geometry (int y, int x, int sy, int sx) {
	pos[0]=y; pos[1]=x;
	XMoveWindow(display,window,x,y);
	resize_window(sx,sy);
	XFlush(display);
}

\def void _0_fall_thru (int flag) {
	int mask = ExposureMask | StructureNotifyMask;
	if (flag) mask |= ExposureMask|StructureNotifyMask|PointerMotionMask|
		ButtonPressMask|ButtonReleaseMask|ButtonMotionMask|
		KeyPressMask|KeyReleaseMask;
	XSelectInput(display, window, mask);
	XFlush(display);
}

\def void _0_transfer (Symbol s) {
	if (s==SYM(plain))       transfer=0;
	else if (s==SYM(xshm))   transfer=1;
	else if (s==SYM(xvideo)) transfer=2;
	else RAISE("unknown transfer mode (possible: plain xshm xvideo)");
}

\def void _0_title (String s=Qnil) {
	rb_ivar_set(rself,SI(@title),s);
	set_wm_hints();
}

\def void initialize (...) {
	int sy=240, sx=320; // defaults
	rb_call_super(argc,argv);
	rb_ivar_set(rself,SI(@title),Qnil);
	argv++, argc--;
	VALUE domain = argc<1 ? SYM(here) : argv[0];
	int i;
	char host[256];
	if (domain==SYM(here)) {
		open_display(0);
		i=1;
	} else if (domain==SYM(local)) {
		if (argc<2) RAISE("open x11 local: not enough args");
		sprintf(host,":%ld",NUM2LONG(argv[1]));
		open_display(host);
		i=2;
	} else if (domain==SYM(remote)) {
		if (argc<3) RAISE("open x11 remote: not enough args");
		sprintf(host,"%s:%ld",rb_sym_name(argv[1]),NUM2LONG(argv[2]));
		open_display(host);
		i=3;
	} else if (domain==SYM(display)) {
		if (argc<2) RAISE("open x11 display: not enough args");
		strcpy(host,rb_sym_name(argv[1]));
		for (int k=0; host[k]; k++) if (host[k]=='%') host[k]==':';
		gfpost("mode `display', DISPLAY=`%s'",host);
		open_display(host);
		i=2;
	} else {
		RAISE("x11 destination syntax error");
	}

	for(;i<argc;i++) {
		Ruby a=argv[i];
		if (a==SYM(override_redirect)) override_redirect = true;
		else if (a==SYM(use_stripes))  use_stripes = true;
		else RAISE("argument '%s' not recognized",rb_sym_name(argv[i]));
	}

	pos[1]=pos[0]=0;
	parent = root_window;
	if (i>=argc) {
	} else {
		VALUE winspec = argv[i];
		if (winspec==SYM(root)) {
			window = root_window;
			is_owner = false;
		} else if (winspec==SYM(embed)) {
			Ruby title_s = rb_funcall(argv[i+1],SI(to_s),0);
			char *title = strdup(rb_str_ptr(title_s));
			sy = sx = pos[0] = pos[1] = 0;
			parent = search_window_tree(root_window,XInternAtom(display,"WM_NAME",0),title);
			free(title);
			if (parent == 0xDeadBeef) RAISE("Window not found.");
		} else if (winspec==SYM(embed_by_id)) {
			const char *winspec2 = rb_sym_name(argv[i+1]);
			if (strncmp(winspec2,"0x",2)==0) {
				parent = strtol(winspec2+2,0,16);
			} else {
				parent = atoi(winspec2);
			}
		} else {
			if (TYPE(winspec)==T_SYMBOL) {
				const char *winspec2 = rb_sym_name(winspec);
				if (strncmp(winspec2,"0x",2)==0) {
					window = strtol(winspec2+2,0,16);
				} else {
					window = atoi(winspec2); // huh?
				}
			} else {
				window = INT(winspec);
			}
			is_owner = false;
			sy = sx = pos[0] = pos[1] = 0;
		}
	}

	// "resize" also takes care of creation
	resize_window(sx,sy);

	if (is_owner) {
		Atom wmDeleteAtom = XInternAtom(display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(display,window,&wmDeleteAtom,1);
	}
	
	Visual *v = visual;
	int disp_is_le = !ImageByteOrder(display);
	int bpp = ximage->bits_per_pixel;
	switch(visual->c_class) {
	case TrueColor: case DirectColor: {
		uint32 masks[3] = { v->red_mask, v->green_mask, v->blue_mask };
		bit_packing = new BitPacking(disp_is_le, bpp/8, 3, masks);
	} break;
	case PseudoColor: {
		uint32 masks[3] = { 0x07, 0x38, 0xC0 };
		bit_packing = new BitPacking(disp_is_le, bpp/8, 3, masks);
	} break;
	default: { RAISE("huh?"); }
	}
	IEVAL(rself,"@clock = Clock.new self; @clock.delay 0");
	show_section(0,0,sx,sy);
}

\classinfo {
	IEVAL(rself,"install '#io:x11',1,1;@mode=6;@comment='X Window System Version 11.x'");
}
\end class FormatX11
void startup_x11 () {
	\startall
}

