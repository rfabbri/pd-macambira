# File: ./src/Makefile.am
# Package: weightmap
# Description:
#   + src-level automake file
#
# Process this file with Automake to create Makefile.in.
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Options & Subdirectories
#-----------------------------------------------------------------------

## --- recursion subdirectories
#SUBDIRS = 

## --- pseudo-deps for '.SUFFIXES'
SUFFIXES = .@PDEXT@

#-----------------------------------------------------------------------
# Flags and variables
#-----------------------------------------------------------------------
PDEXT    = @PDEXT@
EXEEXT   = .@PDEXT@

#-----------------------------------------------------------------------
# pd externals (hacked _PROGRAMS target)
#-----------------------------------------------------------------------

## --- externals
pdexterns_PROGRAMS = @PD_OBJECT_EXTERNALS@

## --- possible externals
EXTRA_PROGRAMS = \
	weightmap

## --- patches
pdexterns_DATA =

## --- documentation
pddoc_DATA = weightmap-help.pd


#-----------------------------------------------------------------------
# sources
#-----------------------------------------------------------------------

weightmap_SOURCES = \
	weightmap.c

#-----------------------------------------------------------------------
# external compilation : flags
#-----------------------------------------------------------------------
DEFS    = @DEFS@
AFLAGS  = @AFLAGS@
DFLAGS  = @DFLAGS@
IFLAGS  = @IFLAGS@
LFLAGS  = @LFLAGS@
OFLAGS  = @OFLAGS@
WFLAGS  = -Wall -Winline

#GLIB_IFLAGS = @GLIB_IFLAGS@
#GLIB_LFLAGS = @GLIB_LFLAGS@

AM_CPPFLAGS = $(IFLAGS) $(GLIB_IFLAGS) $(DFLAGS)
AM_CFLAGS   = $(OFLAGS) $(WFLAGS) $(AFLAGS)

weightmap_LDFLAGS = $(LFLAGS)
weightmap_LDADD   = $(GLIB_LFLAGS)

#-----------------------------------------------------------------------
# Variables: cleanup
#-----------------------------------------------------------------------
## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean: built by 'make'
CLEANFILES = *$(EXEEXT)

## --- distclean: built by 'configure'
DISTCLEANFILES = \
	config.log	\
	config.cache	\
	config.status

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = *~ \
	$(PODS:.pod=.txt) \
	Makefile Makefile.in \
	aclocal.m4 \
	configure \
	install-sh \
	stamp-h.in \
	config.h.in

maintainer-clean-local:
	rm -rf autom4te.cache

#CVSCLEAN_SUBDIRS = $(SUBDIRS)

#CVSCLEANFILES = Makefile.in Makefile


#-----------------------------------------------------------------------
# Variables: distribution
#-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	$(pddoc_DATA) \
	$(pdexterns_DATA)

## --- recursion subdirectories for 'make dist'
DIST_SUBDIRS = $(SUBDIRS)

## --- dist-hook: when another 'Makefile.am' is overkill
#DISTHOOK_DIRS = foo
#DISTHOOK_FILES = foo/bar.txt foo/baz.txt
#dist-hook:
#	for d in $(DISTHOOK_DIRS); do\
#	  mkdir -p $(distdir)/$$d ;\
#	done
#	for f in $(DISTHOOK_FILES); do\
#	  cp -p $(srcdir)/$$f $(distdir)/$$f ;\
#	done

#dist-bz2: dist-bzip2 ;


#-----------------------------------------------------------------------
# Rules: cleanup
#-----------------------------------------------------------------------
.PHONY: cvsclean cvsclean-hook

cvsclean: maintainer-clean ;

