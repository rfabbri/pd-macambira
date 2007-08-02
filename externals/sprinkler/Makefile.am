# File: ./Makefile.am
# Package: sprinkler
# Description:
#   + top-level automake file
#
# Process this file with Automake to create Makefile.in.
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Options & Subdirectories
#-----------------------------------------------------------------------

## --- automake options
#AUTOMAKE_OPTIONS = foreign dist-bzip2 dist-zip
AUTOMAKE_OPTIONS = foreign

## --- recursion subdirectories
#SUBDIRS = config src

## --- pseudo-deps for '.SUFFIXES'
SUFFIXES = .pod .txt .@PDEXT@

#-----------------------------------------------------------------------
# Variables: cleanup
#-----------------------------------------------------------------------
## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean: built by 'make'
#CLEANFILES =

## --- distclean: built by 'configure'
DISTCLEANFILES = \
	config.log	\
	config.cache	\
	config.status

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = \
	*$(EXEEXT) \
	.*~ *~ \
	Makefile Makefile.in \
	aclocal.m4 \
	configure \
	install-sh \
	stamp-h.in \
	config.h.in \
	compile \
	config.guess \
	config.sub \
	depcomp \
	install-sh \
	ltmain.sh \
	missing \
	mkinstalldirs \
	texinfo.tex \
	ylwrap

#	$(PODS:.pod=.txt)

maintainer-clean-local:
	rm -rf autom4te.cache

#CVSCLEAN_SUBDIRS = $(SUBDIRS)

#CVSCLEANFILES = Makefile.in Makefile


#-----------------------------------------------------------------------
# Additional Variables & Rules: PODS
#-----------------------------------------------------------------------
PODS = README.pod

.pod.txt:
	pod2text $< $@

all-local: $(PODS:.pod=.txt)

#-----------------------------------------------------------------------
# Variables: distribution
#-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	$(PODS:.pod=.txt) autogen.sh configure \
	README.cvs GnuGPL.txt \
	Makefile.in

## --- recursion subdirectories for 'make dist'
#DIST_SUBDIRS = $(SUBDIRS)

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

##======================================================================
## sources etc
##======================================================================

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
	sprinkler

## --- patches
pdexterns_DATA =

## --- documentation
pddoc_DATA = sprinkler-help.pd


#-----------------------------------------------------------------------
# sources
#-----------------------------------------------------------------------

sprinkler_SOURCES = \
	sprinkler.c

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

AM_CPPFLAGS = $(IFLAGS) $(DFLAGS)
AM_CFLAGS   = $(OFLAGS) $(WFLAGS) $(AFLAGS)

sprinkler_LDFLAGS = $(LFLAGS)
#sprinkler_LDADD   =

#-----------------------------------------------------------------------
# Variables: distribution
#-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST += \
	$(pddoc_DATA) \
	$(pdexterns_DATA)
