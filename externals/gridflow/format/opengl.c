/*
	$Id: opengl.c 3650 2008-04-25 15:55:52Z matju $

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
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif
#include <setjmp.h>

static bool in_use = false;

\class FormatOpenGL : Format {
	int window;
	GLuint gltex;
	P<BitPacking> bit_packing;
	P<Dim> dim;
	uint8 *buf;
	t_clock *clock;
	void call ();
	\decl 0 resize_window (int sx, int sy);
	\grin 0
	\constructor (t_symbol *mode) {
		if (in_use) RAISE("only one #io:opengl object at a time; sorry");
		in_use=true;
		if (mode!=gensym("out")) RAISE("write-only, sorry");
		int dummy = 0;
		glutInit(&dummy,0);
		glutInitDisplayMode(GLUT_RGBA);
		resize_window(0,0,320,240);
		gltex = 0;
		glEnable(GL_TEXTURE_2D);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DITHER);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);
		window = 0xDeadBeef;
		uint32 mask[3] = {0xff000000,0x00ff0000,0x0000ff00};
		bit_packing = new BitPacking(4,4,3,mask);
		clock = clock_new(this,(t_method)FormatOpenGL_call);
		clock_delay(clock,0);
	}

	~FormatOpenGL () {
		clock_unset(clock);
		if (gltex) glDeleteTextures(1, (GLuint*)&gltex);
		if (buf) delete buf;
		in_use=false;
		if ((unsigned)window!=0xDeadBeef) {
			glutDestroyWindow(window);
			window=0xDeadBeef;
		}
	}
};

static jmp_buf hack;
static void my_idle () { longjmp(hack,1); }

\def void call () {
	int32 sy = dim->get(0);
	int32 sx = dim->get(1);
	glEnable(GL_TEXTURE_2D);
	if (!gltex) {
	}
	glTexSubImage2D(GL_TEXTURE_2D,0,0,0,sx,sy,GL_RGBA,GL_UNSIGNED_BYTE,buf);
	glBindTexture(GL_TEXTURE_2D, gltex);
	glBegin(GL_QUADS);
	glColor3f(1.f,1.f,1.f);
	glTexCoord2f(0.f,0.f); glVertex2f(0.f,0.f);
	glTexCoord2f(1.f,0.f); glVertex2f( sx,0.f);
	glTexCoord2f(1.f,1.f); glVertex2f( sx, sy);
	glTexCoord2f(0.f,1.f); glVertex2f(0.f, sy);
	glEnd();

	//Here comes some (un)fair amount of arm-twisting
	//This is for processing queued events and then "returning".
	glutIdleFunc(my_idle);
	if(!setjmp(hack)) glutMainLoop();
	//done

	clock_delay(clock,100);
}
void FormatOpenGL_call (FormatOpenGL *self) {self->call();}

\def 0 resize_window (int sx, int sy) {
	dim = new Dim(sy,sx,3);
	char foo[666];
	sprintf(foo,"GridFlow/GL (%d,%d,3)",sy,sx);
	if ((unsigned)window==0xDeadBeef) {
		glutInitWindowSize(sx,sy);
		window = glutCreateWindow(foo);
	} else {
		glutReshapeWindow(sx,sy);
	}
	if (buf) delete buf;
	buf = new uint8[sy*sx*4];
	glViewport(0,0,sx,sy);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0,sx,sy,0,-99999,99999);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (gltex) glDeleteTextures(1, (GLuint*)&gltex);
	glGenTextures(1, (GLuint*)&gltex);
	glBindTexture(GL_TEXTURE_2D,gltex);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,sx,sy,0,GL_RGBA,GL_UNSIGNED_BYTE,NULL);

}

GRID_INLET(FormatOpenGL,0) {
	if (in->dim->n != 3)
		RAISE("expecting 3 dimensions: rows,columns,channels");
	if (in->dim->get(2) != 3)
		RAISE("expecting 3 channels: red,green,blue (got %d)",in->dim->get(2));
	int sx = in->dim->get(1), osx = dim->get(1);
	int sy = in->dim->get(0), osy = dim->get(0);
	in->set_chunk(1);
	if (sx!=osx || sy!=osy) resize_window(0,0,sx,sy);
} GRID_FLOW {
	int sxc = in->dim->prod(1);
	int sx = in->dim->get(1);
	int bypl = 4*sx;
	int y = in->dex / sxc;
	for (; n>0; y++, data+=sxc, n-=sxc) bit_packing->pack(sx, data, buf+y*bypl);
	} GRID_FINISH {
} GRID_END

\classinfo {install_format("#io.opengl",2,"");}
\end class FormatOpenGL
void startup_opengl () {
	\startall
}
