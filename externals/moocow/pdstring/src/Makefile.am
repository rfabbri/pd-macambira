# File: ./src/Makefile.am
# Package: pdstring
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
pdexterns_PROGRAMS = pdstring
if WANT_OBJECT_EXTERNALS
 pdexterns_PROGRAMS += \
	any2bytes \
	bytes2any \
	bytes2wchars \
	wchars2bytes \
	printbytes \
	printwchars
endif

## --- possible externals
EXTRA_PROGRAMS = \
	pdstring \
	any2bytes \
	bytes2any \
	bytes2wchars \
	wchars2bytes \
	printbytes \
	printwchars

## --- patches
pdexterns_DATA = \
	any2wchars.pd \
	wchars2any.pd

## --- documentation
pddoc_DATA = \
	pdstring-help.pd \
	any2bytes-help.pd \
	bytes2any-help.pd \
	bytes2wchars-help.pd \
	wchars2bytes-help.pd \
	any2wchars-help.pd \
	wchars2any-help.pd

	printbytes-help.pd 
	printwchars-help.pd


#-----------------------------------------------------------------------
# sources
#-----------------------------------------------------------------------

any2bytes_SOURCES = any2bytes.c mooPdUtils.h pdstringUtils.h pdstringUtils.c

bytes2any_SOURCES = bytes2any.c mooPdUtils.h pdstringUtils.h pdstringUtils.c

bytes2wchars_SOURCES = bytes2wchars.c mooPdUtils.h pdstringUtils.h pdstringUtils.c

wchars2bytes_SOURCES = wchars2bytes.c mooPdUtils.h pdstringUtils.h pdstringUtils.c

printbytes_SOURCES = printbytes.c mooPdUtils.h pdstringUtils.h pdstringUtils.c

printwchars_SOURCES = printwchars.c mooPdUtils.h pdstringUtils.h pdstringUtils.c

pdstring_SOURCES = pdstring.c mooPdUtils.h pdstringUtils.h pdstringUtils.c

#-----------------------------------------------------------------------
# aliases via install hook
#-----------------------------------------------------------------------

if WANT_OBJECT_EXTERNALS
all-local:
	rm -f any2string.$(PDEXT)
	rm -f string2any.$(PDEXT)
	$(LN_S) any2bytes.$(PDEXT) any2string.$(PDEXT)
	$(LN_S) bytes2any.$(PDEXT) string2any.$(PDEXT)
endif

install-data-hook:
	rm -f $(pdexternsdir)/any2string.$(PDEXT)
	rm -f $(pdexternsdir)/string2any.$(PDEXT)
if WANT_OBJECT_EXTERNALS
	$(LN_S) any2bytes.$(PDEXT) $(pdexternsdir)/any2string.$(PDEXT)
	$(LN_S) bytes2any.$(PDEXT) $(pdexternsdir)/string2any.$(PDEXT)
endif

uninstall-local:
	rm -f $(pdexternsdir)/any2string.$(PDEXT)
	rm -f $(pdexternsdir)/string2any.$(PDEXT)


#-----------------------------------------------------------------------
# external compilation : flags
#-----------------------------------------------------------------------
DEFS    = @DEFS@
AFLAGS  = @AFLAGS@
DFLAGS  = @DFLAGS@
IFLAGS  = @IFLAGS@
LFLAGS  = @LFLAGS@
OFLAGS  = @OFLAGS@
WFLAGS  = @WFLAGS@

#GLIB_IFLAGS = @GLIB_IFLAGS@
#GLIB_LFLAGS = @GLIB_LFLAGS@

AM_CPPFLAGS = $(IFLAGS) $(GLIB_IFLAGS) $(DFLAGS)
AM_CFLAGS   = $(OFLAGS) $(WFLAGS) $(AFLAGS)

AM_LDFLAGS ?= $(LFLAGS)

#pdstring_LDFLAGS = $(LFLAGS)
#bytes2any_LDFLAGS = $(LFLAGS)
##string2any_LDFLAGS = $(bytes2any_LDFLAGS)

#-----------------------------------------------------------------------
# Variables: cleanup
#-----------------------------------------------------------------------
## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean: built by 'make'
CLEANFILES = *.$(PDEXT)

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

