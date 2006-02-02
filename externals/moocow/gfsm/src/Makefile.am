# File: ./src/Makefile.am
# Package: pd-gfsm
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
pdexterns_PROGRAMS = gfsm

## --- patches
pdexterns_DATA = \
	gfsm_markov.pd

## --- documentation
pddoc_DATA = \
	gfsm-help.pd \
	gfsm_alphabet-help.pd \
	gfsm_automaton-help.pd \
	gfsm_state-help.pd \
	gfsm_markov-help.pd

#-----------------------------------------------------------------------
# sources
#-----------------------------------------------------------------------

gfsm_SOURCES = \
	config.h \
	atom_alphabet.h atom_alphabet.c \
	pd_alphabet.h  pd_alphabet.c \
	\
	pd_automaton.h pd_automaton.c \
	pd_algebra.c \
	\
	pd_state.h pd_state.c \
	\
	pd_io.h pd_io.c \
	\
	pd_gfsm.h pd_gfsm.c


#----------------------------------------------------------------------
# external compilation : flags
#-----------------------------------------------------------------------
DEFS    = @DEFS@
AFLAGS  = @AFLAGS@
DFLAGS  = @DFLAGS@
IFLAGS  = @IFLAGS@
LFLAGS  = @LFLAGS@
OFLAGS  = @OFLAGS@
WFLAGS  = -Wall

AM_CPPFLAGS = $(IFLAGS) $(DFLAGS)
AM_CFLAGS   = $(OFLAGS) $(WFLAGS) $(AFLAGS)
AM_CXXFLAGS = $(OFLAGS) $(WFLAGS) $(AFLAGS)

gfsm_LDFLAGS = $(LFLAGS)
#gfsm_LDADD   =

#-----------------------------------------------------------------------
# additional hacks
#-----------------------------------------------------------------------
#(none)

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

