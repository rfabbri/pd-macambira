/*
	$Id: sdl.c 3809 2008-06-05 17:17:54Z matju $

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

#include "../gridflow.h.fcs"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <SDL/SDL.h>

struct FormatSDL;
void FormatSDL_call(FormatSDL *self);
static bool in_use = false;
static bool full_screen = false;
static int mousex,mousey,mousem;
SDL_Surface *screen;
FObject *instance;

static t_symbol *keyboard[SDLK_LAST];

static void KEYS_ARE (int i, const char *s__) {
	char *s_ = strdup(s__);
	char *s = s_;
	while (*s) {
		char *t = strchr(s,' ');
		if (t) *t=0;
		keyboard[i] = gensym(s);
		if (!t) break;
		s=t+1; i++;
	}
	free(s_);
}

static void build_keyboard () {
	KEYS_ARE(8,"BackSpace Tab");
	KEYS_ARE(13,"Return");
	KEYS_ARE(27,"Escape");
	KEYS_ARE(32,"space exclam quotedbl numbersign dollar percent ampersand apostrophe");
	KEYS_ARE(40,"parenleft parenright asterisk plus comma minus period slash");
	KEYS_ARE(48,"D0 D1 D2 D3 D4 D5 D6 D7 D8 D9");
	KEYS_ARE(58,"colon semicolon less equal greater question at");
	//KEYS_ARE(65,"A B C D E F G H I J K L M N O P Q R S T U V W X Y Z");
	KEYS_ARE(91,"bracketleft backslash bracketright asciicircum underscore grave quoteleft");
	KEYS_ARE(97,"a b c d e f g h i j k l m n o p q r s t u v w x y z");
	//SDLK_DELETE = 127
	KEYS_ARE(256,"KP_0 KP_1 KP_2 KP_3 KP_4 KP_5 KP_6 KP_7 KP_8 KP_9");
	KEYS_ARE(266,"KP_Decimal KP_Divide KP_Multiply KP_Subtract KP_Add KP_Enter KP_Equal");
	KEYS_ARE(273,"KP_Up KP_Down KP_Right KP_Left KP_Insert KP_Home KP_End KP_Prior KP_Next");
	KEYS_ARE(282,"F1 F2 F3 F4 F5 F6 F7 F8 F9 F10 F11 F12 F13 F14 F15");
	KEYS_ARE(300,"Num_Lock Caps_Lock Scroll_Lock");
	KEYS_ARE(303,"Shift_R Shift_L Control_R Control_L Alt_R Alt_L Meta_L Meta_R");
	KEYS_ARE(311,"Super_L Super_R Mode_switch Multi_key");
}

static void report_pointer () {
	t_atom a[3];
	SETFLOAT(a+0,mousey);
	SETFLOAT(a+1,mousex);
	SETFLOAT(a+2,mousem);
	outlet_anything(instance->bself->outlets[0],gensym("position"),COUNT(a),a);
}

static void HandleEvent () {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		    case SDL_KEYDOWN: case SDL_KEYUP: {
			int key = event.key.keysym.sym;
			int mod = event.key.keysym.mod;
			if (event.type==SDL_KEYDOWN && (key==SDLK_F11 || key==SDLK_ESCAPE || key=='f')) {
				full_screen = !full_screen;
				SDL_WM_ToggleFullScreen(screen);
				break;
			}
			t_symbol *sel = gensym(const_cast<char *>(event.type==SDL_KEYDOWN ? "keypress" : "keyrelease"));
			t_atom at[4];
			mousem &= ~0xFF;
			mousem |= mod;
			SETFLOAT(at+0,mousey);
			SETFLOAT(at+1,mousex);
			SETFLOAT(at+2,mousem);
			SETSYMBOL(at+3,keyboard[event.key.keysym.sym]);
			outlet_anything(instance->bself->outlets[0],sel,4,at);
		    } break;
		    case SDL_MOUSEBUTTONDOWN: SDL_MOUSEBUTTONUP: {
			if (SDL_MOUSEBUTTONDOWN) mousem |=  (128<<event.button.button);
			else                     mousem &= ~(128<<event.button.button);
			//post("mousem=%d",mousem);
			report_pointer();
		    } break;
		    case SDL_MOUSEMOTION: {
			mousey = event.motion.y;
			mousex = event.motion.x;
			report_pointer();
		    } break;
		    case SDL_VIDEORESIZE: {
		    } break;
		}
	}
}

\class FormatSDL : Format {
	P<BitPacking> bit_packing;
	P<Dim> dim;
	t_clock *clock;
	void resize_window (int sx, int sy);
	void call ();
	\decl 0 setcursor (int shape);
	\decl 0 hidecursor ();
	\decl 0 title (string title);
	\constructor (t_symbol *mode) {
		dim=0;screen=0;
		if (in_use) RAISE("only one FormatSDL object at a time; sorry");
		in_use=true;
		if (SDL_Init(SDL_INIT_VIDEO)<0) RAISE("SDL_Init() error: %s",SDL_GetError());
		atexit(SDL_Quit);
		resize_window(320,240);
		SDL_PixelFormat *f = screen->format;
		uint32 mask[3] = {f->Rmask,f->Gmask,f->Bmask};
		switch (f->BytesPerPixel) {
		case 1: RAISE("8 bpp not supported"); break;
		case 2: case 3: case 4:
			bit_packing = new BitPacking(is_le(),f->BytesPerPixel,3,mask);
			break;
		default: RAISE("%d bytes/pixel: how do I deal with that?",f->BytesPerPixel); break;
		}
		instance=this;
		clock = clock_new(this,(t_method)FormatSDL_call);
		clock_delay(clock,0);
		_0_title(0,0,string("GridFlow SDL"));
	}
	\grin 0 int
	~FormatSDL () {
		clock_unset(clock);
		clock_free(clock);
		SDL_Quit();
		instance=0;
		in_use=false;
	}
};

\def 0 title (string title) {
	SDL_WM_SetCaption(title.data(),title.data());
}

void FormatSDL::call() {HandleEvent(); clock_delay(clock,20);}
void FormatSDL_call(FormatSDL *self) {self->call();}

void FormatSDL::resize_window (int sx, int sy) {
	dim = new Dim(sy,sx,3);
	screen = SDL_SetVideoMode(sx,sy,0,SDL_SWSURFACE);
	if (!screen)
		RAISE("Can't switch to (%d,%d,%dbpp): %s", sy,sx,24, SDL_GetError());
}

GRID_INLET(FormatSDL,0) {
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 3)
		RAISE("expecting 3 channels: red,green,blue (got %d)",in->dim->get(2));
	int sx = in->dim->get(1), osx = dim->get(1);
	int sy = in->dim->get(0), osy = dim->get(0);
	in->set_chunk(1);
	if (sx!=osx || sy!=osy) resize_window(sx,sy);
} GRID_FLOW {
	int bypl = screen->pitch;
	int sxc = in->dim->prod(1);
	int sx = in->dim->get(1);
	int y = in->dex / sxc;
	if (SDL_MUSTLOCK(screen)) if (SDL_LockSurface(screen) < 0) return; //???
	for (; n>0; y++, data+=sxc, n-=sxc) {
		/* convert line */
		bit_packing->pack(sx, data, (uint8 *)screen->pixels+y*bypl);
	}
	if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
} GRID_FINISH {
	SDL_UpdateRect(screen,0,0,in->dim->get(1),in->dim->get(0));
} GRID_END

\def 0 setcursor  (int shape) {SDL_ShowCursor(SDL_ENABLE);}
\def 0 hidecursor ()          {SDL_ShowCursor(SDL_DISABLE);}

\end class FormatSDL {install_format("#io.sdl",2,"");}
void startup_sdl () {
	\startall
	build_keyboard();
}
