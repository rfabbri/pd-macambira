dnl Process this file with autoconf to produce a configure script.

AC_INIT
GEM2PDP_VERSION=0.6

if test -f /usr/local/pd; then
	PD_DIR=/usr/local/pd
elif test -f ../../pd; then
	PD_DIR=../../pd
elif
   echo "Pd sources not found"
fi

if test -f /usr/local/pd/pdp; then
	PDP_DIR=/usr/local/pd/pdp
elif test -f ../pdp; then
	PDP_DIR=../pdp
elif
   echo "PdP sources not found"
fi

if test -f /usr/local/pd/gem; then
   GEM_DIR=/usr/local/pd/gem
elif test -f ../../Gem; then
	GEM_DIR=../../Gem
elif  test -f ../gem/Gem; then
	GEM_DIR=../gem/Gem
elif
   echo "Gem sources not found"
fi

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_SANITY_CHECK

AM_MAINTAINER_MODE
AM_WITH_DMALLOC

AC_SUBST(GEM2PDP_VERSION)

echo -n "looking for gem sources (required) ... "
if test -f $GEM_DIR/src/Base/GemMan.h
then
  AC_SUBST(GEM_DIR)
  echo "ok." 
else
  echo "gem source tree not found... install it, fix the path in configure.ac and run autoconf"
  exit -1
fi

AC_SUBST(PD_DIR)

echo -n "looking for pdp sources (required) ... "
if test -f $PDP_DIR/include/pdp.h
then
  AC_SUBST(PDP_DIR)
  echo "ok." 
else
  echo "pdp source tree not found... install it, fix the path in configure.ac and run autoconf"
  exit -1
fi

LIBS="$LIBS -lm"
CFLAGS="$CFLAGS -Wall"

AC_CONFIG_FILES([
Makefile
])
AC_OUTPUT
