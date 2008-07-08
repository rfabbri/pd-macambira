/*
	$Id: quartz.m 3709 2008-05-21 18:53:54Z matju $

	GridFlow
	Copyright (c) 2001-2008 by Mathieu Bouchard

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
#include <Cocoa/Cocoa.h>

#include "../gridflow.h.fcs"

@interface GFView: NSView {
	uint8 *imdata;
	int imwidth;
	int imheight;
}
- (id) drawRect: (NSRect)rect;
- (id) imageHeight: (int)w width: (int)h;
- (int) imageHeight;
- (int) imageWidth;
- (uint8 *) imageData;
- (int) imageDataSize;
@end

@implementation GFView

- (uint8 *) imageData {return imdata;}
- (int) imageDataSize {return imwidth*imheight*4;}
- (int) imageHeight {return imheight;}
- (int) imageWidth {return imwidth;}

- (id) imageHeight: (int)h width: (int)w {
	if (imheight==h && imwidth==w) return self;
	post("new size: y=%d x=%d",h,w);
	imheight=h;
	imwidth=w;
	if (imdata) delete imdata;
	int size = [self imageDataSize];
	imdata = new uint8[size];
	CLEAR(imdata,size);
	NSSize s = {w,h};
	[[self window] setContentSize: s];
	return self;
}

- (id) initWithFrame: (NSRect)r {
	[super initWithFrame: r];
	imdata=0; imwidth=-1; imheight=-1;
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
uint8 *GFView_imageData(GFView *self) {return (uint8 *)[self imageData];}

void GFView_imageHeight_width(GFView *self, int height, int width) {
	[self imageHeight: height width: width];
}

void GFView_display(GFView *self) {
	NSRect r = {{0,0},{[self imageHeight],[self imageWidth]}};
	[self displayRect: r];
	[self setNeedsDisplay: YES];
	[self display];
}

struct FormatQuartz;
void FormatQuartz_call(FormatQuartz *self);

\class FormatQuartz : Format {
	NSWindow *window;
	NSWindowController *wc;
	GFView *widget; /* GridFlow's Cocoa widget */
	t_clock *clock;
	\constructor (t_symbol *mode) {
		NSRect r = {{0,0}, {320,240}};
		window = [[NSWindow alloc]
			initWithContentRect: r
			styleMask: NSTitledWindowMask | NSMiniaturizableWindowMask | NSClosableWindowMask
			backing: NSBackingStoreBuffered
			defer: YES];
		widget = [[GFView alloc] initWithFrame: r];
		[window setContentView: widget];
		[window setTitle: @"GridFlow"];
		[window makeKeyAndOrderFront: NSApp];
		[window orderFrontRegardless];
		wc = [[NSWindowController alloc] initWithWindow: window];
		clock = clock_new(this,(t_method)FormatQuartz_call);
		[window makeFirstResponder: widget];
		post("mainWindow = %08lx",(long)[NSApp mainWindow]);
		post(" keyWindow = %08lx",(long)[NSApp keyWindow]);
		NSColor *color = [NSColor clearColor];
		[window setBackgroundColor: color];
	}
	~FormatQuartz () {
		clock_unset(clock);
		clock_free(clock);
		clock = 0;
		[window autorelease];
		[window setReleasedWhenClosed: YES];
		[window close];
	}
	void call ();
	\grin 0
};

static NSDate *distantFuture, *distantPast;

void FormatQuartz::call() {
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
	clock_delay(clock,20);
}
void FormatQuartz_call(FormatQuartz *self) {self->call();}

template <class T, class S>
static void convert_number_type(int n, T *out, S *in) {
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
	uint8 *data2 = GFView_imageData(w)+off*4;
//	convert_number_type(n,data2,data);
	if (c==3) {
		while(n) {
			data2[0]=255;
			data2[1]=(uint8)data[0];
			data2[2]=(uint8)data[1];
			data2[3]=(uint8)data[2];
			data+=3; data2+=4; n-=3;
		}
	} else {
		while(n) {
			data2[0]=255;
			data2[1]=(uint8)data[0];
			data2[2]=(uint8)data[1];
			data2[3]=(uint8)data[2];
			data+=4; data2+=4; n-=4;
		}
	}
} GRID_FINISH {
	GFView_display(widget);
} GRID_END

\end class FormatQuartz {
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	distantFuture = [NSDate distantFuture];
	distantPast = [NSDate distantPast];
	[NSApplication sharedApplication];
	install_format("#io.quartz",2,"");
}
void startup_quartz () {
        \startall
}

