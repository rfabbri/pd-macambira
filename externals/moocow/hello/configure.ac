dnl Process this file with autoconf to produce a configure script.
dnl -- for a clean build, run: aclocal && autoheader && automake -a && autoconf
AC_PREREQ(2.5)

##-- Some handy macros
define([THE_PACKAGE_NAME],    [hello])
define([THE_PACKAGE_VERSION], [0.01])
define([THE_PACKAGE_AUTHOR],  [moocow@ling.uni-potsdam.de])

AC_INIT(THE_PACKAGE_NAME, THE_PACKAGE_VERSION, THE_PACKAGE_AUTHOR)

##-- Additional m4 macros
AC_CONFIG_MACRO_DIR(m4)

##-- source & aux
dnl AC_CONFIG_AUX_DIR(config)
AC_CONFIG_AUX_DIR(.)

##-- save user's CFLAGS,CPPFLAGS
UCPPFLAGS="$CPPFLAGS"
UCFLAGS="$CFLAGS"
ULDFLAGS="$LDFLAGS"

##-- use automake, autoheader
AM_INIT_AUTOMAKE(THE_PACKAGE_NAME, THE_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)

##-- Default prefix
AC_PREFIX_DEFAULT(/usr/local)

dnl ----- maintainer mode
dnl  + enables "maintainer mode" only with ./configure --enable-maintainer-mode
dnl    - causes make __never__ to invoke 'config/missing' (i.e. any autotools)
dnl    - basically a hack to avoid version mismatches in autoconf, automake, etc.
dnl      for autobuilds from SVN
dnl  + maintainer should call ./configure --enable-maintainer-mode, and must keep
dnl    SVN sources consistent
AM_MAINTAINER_MODE
dnl -----/maintainer mode

##-- other flags (?)
dnl AC_ISC_POSIX

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Pd external checks
AX_PD_EXTERNAL
## /pd external common
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Local: headers
AC_MSG_NOTICE([performing local configuration actions])
## /local: headers
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##-- local: restore user's CFLAGS (now done by AX_PD_EXTERNAL)
#CFLAGS="$UCFLAGS"
#CPPFLAGS="$UCPPFLAGS"
#LDFLAGS="$ULDFLAGS"

##-- local: output
AC_OUTPUT(Makefile)
