AC_PREREQ(2.59)
AC_INIT(hidio,0.1)
AC_CONFIG_SRCDIR(hidio.c)
AC_CANONICAL_HOST
AM_INIT_AUTOMAKE(1.9)

CFLAGS="-DPD $CFLAGS"
INCLUDES="-I../../../pd/src $INCLUDES"

case $host in
*darwin*)
		MACOSX=yes
		;;
*linux*)
		LINUX=yes
		;;
*mingw*)
		WINDOWS=yes
		MINGW=yes
		;;
*cygwin*)
		WINDOWS=yes
		CYGWIN=yes
		;;
esac
AM_CONDITIONAL(ANDROID, test x$ANDROID = xyes)
AM_CONDITIONAL(IPHONEOS, test x$IPHONEOS = xyes)
AM_CONDITIONAL(LINUX, test x$LINUX = xyes)
AM_CONDITIONAL(MACOSX, test x$MACOSX = xyes)
AM_CONDITIONAL(WINDOWS, test x$WINDOWS = xyes)
AM_CONDITIONAL(CYGWIN, test x$MINGW = xyes)
AM_CONDITIONAL(MINGW, test x$MINGW = xyes)

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_MAKE_SET

AC_SUBST(EXTERNTARGET)
AC_SUBST(INCLUDES)

AC_HEADER_STDC

AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug], [use debugging support])], 
		[debug=$enableval])
if test x$debug = xyes; then
    CFLAGS="$CFLAGS -g"
fi

AC_ARG_ENABLE([universal],
	[AS_HELP_STRING([--enable-universal],
		[build universal binary on Mac OS X])], 
    [universal=$enableval], [universal=no])
AM_CONDITIONAL(UNIVERSAL, test x$universal = xyes)
if test x$universal = xyes; then
   UNIVERSAL_FLAGS="-arch ppc -arch i386 -arch x86_64 -mmacosx-version-min=10.4"
   CFLAGS="$CFLAGS -fast -ffast-math $UNIVERSAL_FLAGS"
   LDFLAGS="$LDFLAGS $UNIVERSAL_FLAGS"
fi

# Checks for header files.
AC_FUNC_ALLOCA
AC_CHECK_HEADERS([fcntl.h limits.h malloc.h stddef.h stdlib.h string.h sys/ioctl.h sys/param.h sys/socket.h sys/time.h sys/timeb.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([getcwd memmove memset pow regcomp select socket strchr strrchr strstr strtol])

# Specify what files are to be created.
AC_CONFIG_FILES([Makefile
                HID_Utilities_Source/Makefile])

AC_OUTPUT

dnl output some messages at the end
