dnl Process this file with autoconf to produce a configure script.
AC_INIT(chaos~, 0.01, TimBlechmann@gmx.de, chaos~)
AM_INIT_AUTOMAKE

dnl Checks for programs.
AC_PROG_MAKE_SET
AC_PROG_CC
AC_PROG_CXX

dnl Checks for libraries.
AC_CHECK_LIB(m, sin)

dnl Checks for header files.
AC_HEADER_STDC

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME


dnl Checks for command line arguments
AC_ARG_WITH(flextdir,
	[  --with-flextdir         path to flext headers],
    [
		flextdir=$withval
		INCLUDEDIR="-I$withval $INCLUDEDIR"
	],
	[echo "path to flext headers required" && exit 1])

AC_CHECK_FILE("$flextdir/flext.h",,
	[echo "$flextdir/flext.h not found" && exit 1])


AC_ARG_ENABLE(system,
	[  --enable-system         system (default: pd)],
	[
	if test "enableval" == "max"; then
		AC_DEFINE(FLEXT_SYS,1)
	else
		AC_DEFINE(FLEXT_SYS,2)
	fi
	system = $enabeval
	],
	AC_DEFINE(FLEXT_SYS,2)
	)


AC_ARG_WITH(sysdir,
	[  --with-sysdir           path pd / max installation],
	[
		SYSDIR=$withval
		INCLUDEDIR="-I$withval $INCLUDEDIR"
	],	
	[echo "path to pd / max installation required" && exit 1])


AC_ARG_ENABLE(static,
	[  --enable-static         static linking with libflext],
			LDFLAGS="-Bstatic $LDFLAGS"
		,
	    [
			AC_DEFINE(FLEXT_SHARED)
			AC_DEFINE(FLEXT_THREADS)
			LDFLAGS="-Bdynamic $LDFLAGS"
		]
		)

AC_ARG_ENABLE(debug,
	[  --enable-debug          link with libflext_d library for debugging],
   	[
		LDFLAGS="-lflext_d $LDFLAGS"
		CXXFLAGS="-g"
	],
	[
		LDFLAGS="-lflext $LDFLAGS"
		CXXFLAGS="-O3"
	])

dnl mtune will break gcc 3.3.x
AC_ARG_ENABLE(optimize, [  --enable-optimize       enables optimized builds for: pentium4, pentium3, G4, G5],
    [
	case $enableval in
	pentium3 | pentium3m)
		OPT_FLAGS = "-mtune=$enableval -march=$enableval -mmmx -msse -mfpmath=sse";;
	pentium2 | athlon | pentium-mmx)
		OPT_FLAGS="-mtune=$enableval -march=$enableval -mmmx";;
	pentium)
	 	OPT_FLAGS="-mtune=$enableval -march=$enableval";;
	pentium4 | pentium4m | pentium-m | prescott | nocona | athlon-xp | athlon-mp | athlon64 | opteron) 
		OPT_FLAGS="-mtune=$enableval -march=$enableval -mmmx -msse -msse2 -mfpmath=sse";;
	G5 | G4)
		OPT_FLAGS="-mtune=$enableval -march=$enableval -maltivec -faltivec -malign-natural";;
	G3)
		OPT_FLAGS="-mtune=$enableval -march=$enableval -malign-natural";;
	*)
		;;
	esac
	])

dnl select build
if test `uname -s` == "Linux"; then 
	dnl no max/msp for linux
	OBJEXT=o
	EXTENSION=pd_linux
fi

if test `uname -s` == "Darwin"; then 
	dnl just a guess
	if test "$system" = "max"; then
		echo "build system doesn't support max, yet" && exit 1;
	else
		LDFLAGS="-bundle -bundle_loader $LDFLAGS"
		FRAMEWORKS=
		EXTENSION=pd_darwin
		LIBS="gcc $LIBS"
	fi
	OBJEXT=o
fi

if test `uname -s` == "Cygwin"; then 
	dnl just a guess
	if test "$system" = "max"; then
		echo "build system doesn't support max, yet" && exit 1;
	else
		EXTENSION=dll
		LIBS="gcc $LIBS bin/pd bin/pthreadVC.dll"
		LDFLAGS="-L$SYSDIR"
	fi
fi	


AC_SUBST(EXTENSION)
AC_SUBST(INCLUDEDIR)
AC_SUBST(CXXFLAGS)
AC_SUBST(OPT_FLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(DEFS)
AC_SUBST(LIBS)
AC_SUBST(OBJEXT)
AC_SUBST(FRAMEWORKS)
AC_SUBST(SYSDIR)
AC_OUTPUT([Makefile
		  src/Makefile])
