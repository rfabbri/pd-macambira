dnl created with this version of autoconf
AC_PREREQ(2.53)

dnl Process this file with autoconf to produce a configure script.
AC_INIT([rawHID(e)],[0.0],[hans@eds.org],[rawhid])

dnl the obligatory license
AC_COPYRIGHT([2003 Hans-Christoph Steiner <hans@eds.org> GNU GPL 2.0 or higher])

dnl get cvs revision
AC_REVISION($Revision: 1.2 $)

dnl check and make sure the source files are actually there
AC_CONFIG_SRCDIR([rawjoystick.c])

dnl variables to substitute in Makefile.in
AC_SUBST(CC)
AC_SUBST(CFLAGS)
AC_SUBST(DEFS)
AC_SUBST(EXT)
AC_SUBST(INCLUDE)
AC_SUBST(LD)
AC_SUBST(LFLAGS)
AC_SUBST(STRIPFLAGS)

dnl Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

dnl check headers
AC_HEADER_STDC
AC_CHECK_HEADERS(SDL/SDL.h)
AC_CHECK_HEADERS(m_imp.h)
AC_CHECK_HEADERS(stddef.h)

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE

#LD=ld

dnl
dnl  OK, checks for machines are here now
dnl
if test `uname -s` = Linux; 
then
  CFLAGS="-O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes -Werror \
    -Wno-unused -Wno-parentheses -Wno-switch -g"
  INCLUDE="-I../../../pd/src -I. -I/usr/local/include"
  LFLAGS="-export_dynamic -shared"
  EXT=pd_linux	
  DEFS="-DUNIX -DLINUX"
  STRIPFLAGS="--strip-unneeded"
fi

dnl This should use '-bundle_loader /path/to/pd/bin/pd' instead of'-undefined suppress'
dnl then strip might do something
if test `uname -s` = Darwin; 
then
  CFLAGS="-O2"
  DEFS="-DUNIX -DMACOSX"
  EXT=pd_darwin
  INCLUDE="-I../../../pd/src -I. -I/sw/include"
  LD=cc
  LFLAGS="-bundle -bundle_loader ../../../pd/bin/pd -flat_namespace"
  STRIPFLAGS=
fi

if test `uname -s` = CYGWIN_NT-5.0;
then
  CFLAGS="-O2"
  DEFS=
  EXT=dll
  INCLUDE="-I/cygdrive/c/pd/src -I/usr/local/include"
  LFLAGS="-export_dynamic -shared"
  STRIPFLAGS=
fi

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
