AC_INIT(system/pdp.c)
AC_CONFIG_HEADER(include/pdp_config.h)
AC_PROG_CC
AC_HEADER_STDC

dnl TAG CVS WHEN VERSION CHANGES !!!
PDP_VERSION=0.9
AC_SUBST(PDP_VERSION)

AC_ARG_ENABLE(pwc,    
	[  --enable-pwc            enable additional Philips WebCam support],
	AC_DEFINE(HAVE_PWCV4L, 1,enable pwc v4l support))

AC_ARG_ENABLE(mmx,    
	[  --enable-mmx            enable MMX support], MMX=yes, MMX=no)


if test $prefix == NONE;
then
	prefix=/usr/local
fi


dnl try to locate the pd header in case the setup is nonstandard
dnl check in  $prefix/pd/src then ../pd/src 
dnl if this fails we trust it is in the standard include path
PWD=`pwd`
if test -f $prefix/pd/src/m_pd.h;
then
	PD_CPPFLAGS="-I$prefix/pd/src"

elif test -f $prefix/src/pd/src/m_pd.h;
then
	PD_CPPFLAGS="-I$prefix/src/pd/src"
elif test -f $PWD/../pd/src/m_pd.h;
then
	PD_CPPFLAGS="-I$PWD/../pd/src"
elif test -f $PWD/../src/m_pd.h;
then
	PD_CPPFLAGS="-I$PWD/../src"
fi


CPPFLAGS="$CPPFLAGS $PD_CPPFLAGS"
AC_CHECK_HEADER(m_pd.h,,
	echo "WARNING: m_pd.h not found. Is PD installed?"
	echo "WARNING: if you have changed PD_CPPFLAGS in Makefile.config.in you can ignore this warning." )


AC_CHECK_LIB(m,sin)


ARCH=`uname -s`
if test $ARCH == Linux;
then
	PDP_LIBRARY_NAME=pdp.pd_linux
	if test $MMX == yes;
	then
		PDP_TARGET=linux_mmx
	else
		PDP_TARGET=linux
	fi
elif test $ARCH == Darwin;
then
	PDP_LIBRARY_NAME=pdp.pd_darwin
	PDP_TARGET=darwin
else
	echo WARNING: Architecture `uname -s` not supported.
	exit
fi



dnl Darwin specific stuff: this is still pretty experimental
dnl How to test if frameworks are present ????
if test $ARCH == Darwin
then
	PDP_OPTMOD="$PDP_OPTMOD pdp_sdl.o"
	LIBS="$LIBS -framework SDL"
	PDP_EXTRA_CPPFLAGS="$PDP_EXTRA_INCLUDE -I/Library/Frameworks/SDL.framework/Headers"
	PD_EXECUTABLE=ihavenocluewherethepdbinaryislocated
	AC_DEFINE(HAVE_PDP_SDL, 1, build pdp_sdl)

dnl These are checks for libraries. 
dnl Objects that depend on a lib only get included when the lib is found.
else

	AC_CHECK_LIB(quicktime, lqt_decode_video,
		PDP_OPTMOD="$PDP_OPTMOD pdp_qt.o"
		LIBS="$LIBS -lquicktime"
		AC_DEFINE(HAVE_PDP_QT, 1, build pdp_qt),
		echo "   libquicktime not found: not building pdp_qt")

	AC_CHECK_LIB(SDL, SDL_Init, 
		PDP_OPTMOD="$PDP_OPTMOD pdp_sdl.o"
		LIBS="$LIBS -lSDL"
		AC_DEFINE(HAVE_PDP_SDL, 1, build pdp_sdl),
		echo "   libSDL not found: not building pdp_sdl")

	AC_CHECK_LIB(Xv, XvPutImage,
		PDP_OPTMOD="$PDP_OPTMOD pdp_xv.o"
		LIBS="$LIBS -L/usr/X11R6/lib -lX11 -lXv -lXext"
		AC_DEFINE(HAVE_PDP_XV, 1, build pdp_xv),
		echo "   libXv not found: not building pdp_xv",
		-L/usr/X11R6/lib -lX11 -lXext)

	AC_CHECK_HEADER(linux/videodev.h,
		PDP_OPTMOD="$PDP_OPTMOD pdp_v4l.o"
		AC_DEFINE(HAVE_PDP_V4L, 1, build pdp_v4l),
		echo "   linux/videodev.h not found: not building pdp_v4l")
fi


echo target is $PDP_TARGET


AC_SUBST(PD_CPPFLAGS)
AC_SUBST(PD_EXECUTABLE)
AC_SUBST(PDP_EXTRA_CPPFLAGS)
AC_SUBST(PDP_LIBRARY_NAME)
AC_SUBST(PDP_TARGET)
AC_SUBST(PDP_OPTMOD)
AC_CONFIG_FILES(Makefile.config)
AC_CONFIG_FILES(bin/pdp-config)
AC_OUTPUT
