dnl Process this file with autoconf to produce a configure script.

AC_INIT
VIDEOGRID_VERSION=0.6

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AM_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_CPP
AC_ISC_POSIX
AC_HEADER_STDC
AC_PROG_MAKE_SET
AM_SANITY_CHECK

AM_MAINTAINER_MODE
AM_WITH_DMALLOC

AC_SUBST(VIDEOGRID_VERSION)

# Checks for Pd and Gem source trees

if test -d ../../../pd; 
then
	PD_DIR=../../../pd
else
   echo "Pd sources not found"
fi


AC_ARG_WITH(pddir,       [  --with-pddir=<dir>      specify an alternate pd source tree])

if test "$with_pddir" != ""
then
 PD_DIR=$with_pddir
fi

echo -n "looking for pd sources (required) ... "
if test -f $PD_DIR/src/m_imp.h
then
  AC_SUBST(PD_DIR)
  echo "ok." 
else
  echo "pd source tree not found... use ./configure --with-pddir= option"
  exit -1
fi

# Checks for libraries.

pkg_modules="libavcodec >= 0.5, libavformat >= 0.5, libavutil >= 0.5, libswscale >= 0.5"
PKG_CHECK_MODULES(FF_PACKAGE,[$pkg_modules])
AC_SUBST(FF_PACKAGE_CFLAGS)
AC_SUBST(FF_PACKAGE_LIBS)


# FIXME: Replace `main' with a function in `-lc':
AC_CHECK_LIB([c], [main])
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h malloc.h stdlib.h string.h sys/ioctl.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([munmap pow select strstr])



LIBS="$LIBS -lm"
CFLAGS="$CFLAGS -Wall"

AC_CONFIG_FILES([
Makefile
])
AC_OUTPUT
