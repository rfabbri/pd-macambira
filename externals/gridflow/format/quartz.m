/*
	$Id: quartz.m,v 1.2 2006-03-15 04:37:46 matju Exp $

	GridFlow
	Copyright (c) 2001,2002,2003 by Mathieu Bouchard

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
	This is written in Objective C++, which is the union of C++ and Objective C;
	Their intersection is C or almost. They add quite different sets of features.
	I need Objective C here because the Cocoa API is for Objective C and Java only,
	and the Objective C one was the easiest to integrate in GridFlow.

	The next best possibility may be using RubyCocoa, a port of the Cocoa API to Ruby;
	However I haven't checked whether Quartz is wrapped, and how easy it is to
	process images.
*/

#include <stdio.h>
#include <objc/Object.h>

/* wrapping name conflict */
#define T_DATA T_COCOA_DATA
#include <Cocoa/Cocoa.h>
#undef T_DATA

#include "../base/grid.h.fcs"

@interface GFView: NSView {
	Pt<uint8> imdata;
	int imwidth;
	int imheight;
}
- (id) drawRect: (NSRect)rect;
- (id) imageHeight: (int)w width: (int)h;
- (uint8 *) imageData;
- (int) imageDataSize;
@end

@implementation GFView

- (uint8 *) imageData { return imdata; }
- (int) imageDataSize { return imwidth*imheight*4; }

- (id) imageHeight: (int)h width: (int)w {
	if (imheight==h && imwidth==w) return self;
	gfpost("new size: y=%d x=%d",h,w);
	imheight=h;
	imwidth=w;
	if (imdata) delete imdata.p;
	int size = [self imageDataSize];
	imdata = ARRAY_NEW(uint8,size);
	uint8 *p = imdata;
	CLEAR(imdata,size);
	NSSize s = {w,h};
	[[self window] setContentSize: s];
	return self;
}

- (id) initWithFrame: (NSRect)r {
	[super initWithFrame: r];
	imdata=Pt<uint8>(); imwidth=-1; imheight=-1;
	[self imageHeight: 240 width: 320];
	return self;
}	

- (id) drawRect: (NSRect)rect {
	[super drawRect: rect];
	if (![self lockFocusIfCanDraw]) return self;
	CGContextRef g = (CGContextRef)
		[[NSGraphicsContext graphicsContextWithWindow: [self window]]
			graphicsPort];
	CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();
	CGDataProviderRef dp = CGDataProviderCreateWithData(
		NULL, imdata, imheight*imwidth*4, NULL);
	CGImageRef image = CGImageCreate(imwidth, imheight, 8, 32, imwidth*4, 
		cs, kCGImageAlphaFirst, dp, NULL, 0, kCGRenderingIntentDefault);
	CGDataProviderRelease(dp);
	CGColorSpaceRelease(cs);
	CGRect rectangle = CGRectMake(0,0,imwidth,imheight);
	CGContextDrawImage(g,rectangle,image);
	CGImageRelease(image);
	[self unlockFocus];
	return self;
}
@end

/* workaround: bus error in gcc */
Pt<uint8> GFView_imageData(GFView *self) {
	return Pt<uint8>([self imageData], [self imageDataSize]);
}
void GFView_imageHeight_width(GFView *self, int height, int width) {
	[self imageHeight: height width: width];
}

void GFView_display(GFView *self) {
	NSRect r = {{0,0},{self->imheight,self->imwidth}};
	[self displayRect: r];
	[self setNeedsDisplay: YES];
	[self display];
}

\class FormatQuartz < Format
struct FormatQuartz : Format {
	NSWindow *window;
	NSWindowController *wc;
	GFView *widget; /* GridFlow's Cocoa widget */
	NSDate *distantFuture;
	\decl void initialize (Symbol mode);
	\decl void delete_m ();
	\decl void close ();
	\decl void call ();
	\grin 0
};

static NSDate *distantFuture, *distantPast;

\def void call() {
	NSEvent *e = [NSApp nextEventMatchingMask: NSAnyEventMask
		// untilDate: distantFuture // blocking
		untilDate: distantPast // nonblocking
		inMode: NSDefaultRunLoopMode
		dequeue: YES];
	if (e) {
		NSLog(@"%@", e);
		[NSApp sendEvent: e];
	}
	[NSApp updateWindows];
	[this->window flushWindowIfNeeded];
	IEVAL(rself,"@clock.delay 20");
}

template <class T, class S>
static void convert_number_type(int n, Pt<T> out, Pt<S> in) {
	for (int i=0; i<n; i++) out[i]=(T)in[i];
}

GRID_INLET(FormatQuartz,0) {
	if (in->dim->n!=3) RAISE("expecting 3 dims, not %d", in->dim->n);
	int c=in->dim->get(2);
	if (c!=3&&c!=4) RAISE("expecting 3 or 4 channels, not %d", in->dim->get(2));
//	[widget imageHeight: in->dim->get(0) width: in->dim->get(1) ];
	GFView_imageHeight_width(widget,in->dim->get(0),in->dim->get(1));
	in->set_factor(in->dim->prod(1));
} GRID_FLOW {
	int off = in->dex/in->dim->prod(2);
	int c=in->dim->get(2);
	NSView *w = widget;
	Pt<uint8> data2 = GFView_imageData(w)+off*4;
//	convert_number_type(n,data2,data);
	if (c==3) {
		while(n) {
			data2[0]=255;
			data2[1]=data[0];
			data2[2]=data[1];
			data2[3]=data[2];
			data+=3; data2+=4; n-=3;
		}
	} else {
		while(n) {
			data2[0]=255;
			data2[1]=data[0];
			data2[2]=data[1];
			data2[3]=data[2];
			data+=4; data2+=4; n-=4;
		}
	}
} GRID_FINISH {
	GFView_display(widget);
} GRID_END

\def void initialize (Symbol mode) {
	rb_call_super(argc,argv);
	NSRect r = {{0,0}, {320,240}};
	window = [[NSWindow alloc]
		initWithContentRect: r
		styleMask: NSTitledWindowMask | NSMiniaturizableWindowMask | NSClosableWindowMask
		backing: NSBackingStoreBuffered
		defer: YES
		];
	widget = [[GFView alloc] initWithFrame: r];
	[window setContentView: widget];
	[window setTitle: @"GridFlow"];
	[window makeKeyAndOrderFront: NSApp];
	[window orderFrontRegardless];
	wc = [[NSWindowController alloc]
		initWithWindow: window];
	IEVAL(rself,"@clock = Clock.new self");
	[window makeFirstResponder: widget];
	gfpost("mainWindow = %08lx",(long)[NSApp mainWindow]);
	gfpost(" keyWindow = %08lx",(long)[NSApp keyWindow]);
	NSColor *color = [NSColor clearColor];
	[window setBackgroundColor: color];
}

\def void delete_m () {
	[window autorelease];
}

\def void close () {
	IEVAL(rself,"@clock.unset");
	rb_call_super(argc,argv);
	[window autorelease];
	[window setReleasedWhenClosed: YES];
	[window close];
}

\classinfo {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	distantFuture = [NSDate distantFuture];
	distantPast = [NSDate distantPast];
	[NSApplication sharedApplication];
	IEVAL(rself,
\ruby
	install '#io:quartz',1,1
	@comment = "Apple Quartz/Cocoa"
	@flags = 2
\end ruby
);}

\end class FormatQuartz
void startup_quartz () {
        \startall
}

