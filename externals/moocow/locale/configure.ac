dnl Process this file with autoconf to produce a configure script.
dnl -- for a clean build, run: aclocal && autoheader && automake -a && autoconf
AC_PREREQ(2.5)

dnl Some handy macros
define([THE_PACKAGE_NAME],    [locale])
define([THE_PACKAGE_VERSION], [0.01])
define([THE_PACKAGE_AUTHOR],  [moocow@ling.uni-potsdam.de])

AC_INIT(THE_PACKAGE_NAME, THE_PACKAGE_VERSION, THE_PACKAGE_AUTHOR)

dnl
dnl source & aux
dnl
AC_CONFIG_AUX_DIR(config)

dnl
dnl save user's CFLAGS,CPPFLAGS
dnl
UCPPFLAGS="$CPPFLAGS"
UCFLAGS="$CFLAGS"

dnl
dnl use automake
dnl
AM_INIT_AUTOMAKE(THE_PACKAGE_NAME, THE_PACKAGE_VERSION)

dnl
dnl use autoheader
dnl
AM_CONFIG_HEADER(src/config.h)


dnl
dnl Programs, prefix
dnl
AC_PROG_CC
AC_PROG_INSTALL
AC_PREFIX_DEFAULT(/usr/local)

dnl
dnl Substitutions
dnl 
AC_SUBST(DEFS)
AC_SUBST(AFLAGS)
AC_SUBST(DFLAGS)
AC_SUBST(IFLAGS)
AC_SUBST(LFLAGS)
AC_SUBST(OFLAGS)
AC_SUBST(WFLAGS)
AC_SUBST(LD)

AC_SUBST(PDEXT)

dnl version stuff (automatically exported?)
AC_SUBST(PACKAGE_VERSION)
AC_SUBST(PACKAGE_NAME)
AC_SUBST(BUGREPORT)

dnl ----- maintainer mode
dnl  + enables "maintainer mode" only with ./configure --enable-maintainer-mode
dnl    - causes make __never__ to invoke 'config/missing', e.g. any autotools
dnl    - basically a hack to avoid version mismatches in autoconf, automake, etc.
dnl      for autobuilds from SVN
dnl  + maintainer should call ./configure --enable-maintainer-mode, and must keep
dnl    SVN sources consistent
AM_MAINTAINER_MODE
dnl -----/maintainer mode


dnl other flags
dnl AC_ISC_POSIX

dnl Checks for header files.
dnl AC_HEADER_STDC
AC_CHECK_HEADERS([string.h wchar.h locale.h],
	[],
	AC_MSG_WARN([-----------------------------------------------------------------])
	AC_MSG_WARN([could not find standard C headers -- things may get ugly])
	AC_MSG_WARN([-----------------------------------------------------------------]),
	[/* nonempty includes: compile only */])

##-- check for LC_* declarations required by ISO-C99 (ISO/IEC 9899:TC2)
AC_CHECK_DECLS([LC_ALL],
		[],
		[AC_MSG_ERROR([LC_ALL not declared - bailing out!])],
		[#include <locale.h>])
AC_CHECK_DECLS([LC_COLLATE,LC_CTYPE,LC_MONETARY,LC_NUMERIC,LC_TIME],[],[],[#include <locale.h>])

##-- check for additional LC_* declarations
AC_CHECK_DECLS([LC_MESSAGES,LC_PAPER,LC_NAME,LC_ADDRESS,LC_TELEPHONE,LC_MEASUREMENT,LC_IDENTIFICATION],[],[],[#include <locale.h>])

dnl PD externals
AC_MSG_CHECKING([how to hack automake EXEEXT conventions])
case "${am__api_version}" in
  1.[[0-4]]*)
    AC_MSG_RESULT([automake v${am__api_version}: on install])
    buildext=""
    ;;
  *)
    AC_MSG_RESULT([automake v${am__api_version}: on build])
    buildext="\$(EXEEXT)"
    ;;
esac

PD_OBJECT_EXTERNALS="locale${buildext}"
AC_SUBST(PD_OBJECT_EXTERNALS)


dnl
dnl pd-directory/ies
dnl 
AC_ARG_WITH(pd-dir,
	AC_HELP_STRING([--with-pd-dir=DIR], [PD base directory (default=PREFIX/pd)]),
	[pddir="$withval"],
	[pddir="\${prefix}/pd"])
pddocdir="${pddir}/doc/5.reference"
AC_SUBST(pddir)
AC_SUBST(pddocdir)



AC_ARG_WITH(pd-include,
	AC_HELP_STRING([--with-pd-include=DIR], [PD include directory (default=NONE)]),
	[pdincludedir="$withval"],
	[pdincludedir=""])
if test -n "$pdincludedir" ; then
  IFLAGS="$IFLAGS -I$pdincludedir"
fi
AC_SUBST(pdincludedir)

AC_ARG_WITH(pd-extdir,
	AC_HELP_STRING([--with-pd-extdir=DIR], [Directory for PD externals (default=PDDIR/externs)]),
	[pdexternsdir="$withval"],
	[pdexternsdir="$pddir/externs"])
AC_SUBST(pdexternsdir)

dnl
dnl Check for m_pd.h
dnl
CPPFLAGS="$CPPFLAGS $IFLAGS"
AC_CHECK_HEADER(m_pd.h,[],
	AC_MSG_WARN([-----------------------------------------------------------------])
	AC_MSG_WARN([could not find PD header file 'm_pd.h' -- things might get ugly.])
	AC_MSG_WARN([-----------------------------------------------------------------]),
	[/* nonempty includes: compile only */])


dnl
dnl debug version?
dnl
AC_MSG_CHECKING([whether we are building a debug version])
AC_ARG_ENABLE([debug],
	AC_HELP_STRING([--enable-debug],[build debug version (default=no)]))
if test "$enable_debug" = "yes" ; then
  AC_MSG_RESULT(yes)
  DEBUG="yes"
  AC_DEFINE(RAWFILE_DEBUG,1,
	[Define this to include debugging code for the 'rawfile' external.])
else
  AC_MSG_RESULT(no)
  DEBUG="no"
fi
AC_SUBST(DEBUG)


##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## compiled
AC_DEFINE_UNQUOTED(PDMOOEXT_DATE, "`date`", [Date this external was configured])
AC_DEFINE_UNQUOTED(PDMOOEXT_USER, "$USER",  [User who configured this external])
## /compiled
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## BEGIN machine-dependent variables

LD=ld

##-- Linux
if test `uname -s` = Linux; 
then
  LFLAGS="$LFFLAGS -Wl,-export-dynamic -shared"
  if test "$DEBUG" = "no" -a -z "$UCFLAGS"; then
    ##-- only set OFLAGS if user CFLAGS are empty
    OFLAGS="-O2 -pipe" 
  else
    OFLAGS="-g"
  fi
  OFLAGS="$OFLAGS -fPIC"
  DFLAGS="$DFLAGS -DPIC"
  PDEXT=pd_linux
fi

##-- MacOSX (darwin)
if test `uname -s` = Darwin;
then
  LD=cc
  LFLAGS="$LFLAGS -bundle -undefined suppress  -flat_namespace"
  DFLAGS="$DFLAGS -DMACOSX"
  if test "$DEBUG" = "no" -a -z "$UCFLAGS"; then
    ##-- only set OFLAGS if user CFLAGS are empty
    OFLAGS="-O2"
  else
    OFLAGS="-g"
  fi
  PDEXT=pd_darwin
fi

##-- alpha
if test `uname -m` = alpha; 
then 
 AFLAGS="-mieee  -mcpu=ev56"; 
 OFLAGS="$CFLAGS"
fi

##-- irix64
if test `uname -s` = IRIX64; 
then
  LFLAGS="$LFLAGS -n32 -DUNIX -DIRIX -DN32 -woff 1080,1064,1185 \
	-OPT:roundoff=3 -OPT:IEEE_arithmetic=3 -OPT:cray_ivdep=true \
	-shared -rdata_shared"
  OFLAGS="$CFLAGS"
  PDEXT=pd_irix6
fi

##-- irix32
if test `uname -s` = IRIX32; 
then
  LFLAGS="$LFLAGS -o32 -DUNIX -DIRIX -O2 -shared -rdata_shared"
  OFLAGS="$CFLAGS"
  PDEXT=pd_irix5
fi

EXT=$PDEXT

## END machine-dependent variables
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

dnl
dnl restore user's CFLAGS
dnl
CFLAGS="$UCFLAGS"
CPPFLAGS="$UCPPFLAGS"

AC_OUTPUT(config/Makefile src/Makefile Makefile)
