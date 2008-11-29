## File: src/programs/Makefile.am
## Package: gfsm
## Description:
##   + src-level automake file
##
## Process this file with Automake to create Makefile.in.
##-----------------------------------------------------------------------

##-----------------------------------------------------------------------
## subdirectories
##-----------------------------------------------------------------------
#SUBDIRS =

##-----------------------------------------------------------------------
## primary targets
##-----------------------------------------------------------------------

## --- executable programs
#bin_PROGRAMS = ${prog_1} ... ${prog_N}
bin_PROGRAMS = \
	gfsmarcsort \
	gfsmarith \
	gfsmclosure \
	gfsmcompile \
	gfsmcomplement \
	gfsmcompose \
	gfsmcompre \
	gfsmconcat \
	gfsmconnect \
	gfsmconvert \
	gfsmdeterminize \
	gfsmdifference \
	gfsmdraw \
	gfsmheader \
	gfsminfo \
	gfsmintersect \
	gfsminvert \
	gfsmlabels \
	gfsmlookup \
	gfsmoptional \
	gfsmprint \
	gfsmproduct \
	gfsmproject \
	gfsmrenumber \
	gfsmreplace \
	gfsmreverse \
	gfsmrmepsilon \
	gfsmsigma \
	gfsmstrings \
	gfsmunion \
	gfsmviterbi

EXTRA_PROGRAMS = gfsmindex
bin_PROGRAMS += gfsmindex

EXTRA_DIST =

#bin_SCRIPTS = ${script1} ... ${scriptN}

## --- libtool libraries
#lib_LTLIBRARIES = ${lib_1}.la ... ${lib_N}.la

## --- shared data in @pkgdatadir@
#pkgdata_DATA = ${data_1} ... ${data_N}

## --- manpages -- section is auto-detected
#man_MANS = ${man_1} ... ${man_N}

##-----------------------------------------------------------------------
## sources
##-----------------------------------------------------------------------

SUFFIXES = \
	.gog _cmdparser.c _cmdparser.cc _cmdparser.h \
	.pod .txt .1 .html .tex


##-----------------------------------------------------------------------
## individual programs
##-----------------------------------------------------------------------

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
## --- Required sources
#${prog_i}_SOURCES =
##
## --- Extra (possible)sources
#EXTRA_${prog_i}_SOURCES = 
#EXTRA_${lib_i}_la_SOURCES = 
##
## --- leftovers
#mootconfig_SOURCES = mootconfig.cc
##
## --- Additional dependencies
#gfsmcompile_main.o: gfsmcompile_cmdparser.h
##
## --- Linker flags
#${prog_i}_LDFLAGS = -L. -static
#${prog_i}_LDADD   = ${non_src_file}.o -lsomelib
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#gfsmX_SOURCES = \
#	gfsmX_main.c \
#	gfsmX_cmdparser.c gfsmX_cmdparser.h
#
#gfsmX_main.o: gfsmX_cmdparser.h
#
#gfsmX_LDFLAGS = $(LDFLAGS_COMMON)
#gfsmX_LDADD = $(LDADD_COMMON)
#
#EXTRA_DIST += gfsmX.gog
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmarith_SOURCES = \
	gfsmarith_main.c \
	gfsmarith_cmdparser.c gfsmarith_cmdparser.h

gfsmarith_main.o: gfsmarith_cmdparser.h

gfsmarith_LDFLAGS = $(LDFLAGS_COMMON)
gfsmarith_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmarith.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmcompile_SOURCES = \
	gfsmcompile_main.c \
	gfsmcompile_cmdparser.c gfsmcompile_cmdparser.h

gfsmcompile_main.o: gfsmcompile_cmdparser.h

gfsmcompile_LDFLAGS = $(LDFLAGS_COMMON)
gfsmcompile_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmcompile.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmconvert_SOURCES = \
	gfsmconvert_main.c \
	gfsmconvert_cmdparser.c gfsmconvert_cmdparser.h

gfsmconvert_main.o: gfsmconvert_cmdparser.h

gfsmconvert_LDFLAGS = $(LDFLAGS_COMMON)
gfsmconvert_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmconvert.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmprint_SOURCES = \
	gfsmprint_main.c \
	gfsmprint_cmdparser.c gfsmprint_cmdparser.h

gfsmprint_main.o: gfsmprint_cmdparser.h

gfsmprint_LDFLAGS = $(LDFLAGS_COMMON)
gfsmprint_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmprint.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmindex_SOURCES = \
	gfsmindex_main.c \
	gfsmindex_cmdparser.c gfsmindex_cmdparser.h

gfsmindex_main.o: gfsmindex_cmdparser.h

gfsmindex_LDFLAGS = $(LDFLAGS_COMMON)
gfsmindex_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmindex.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsminfo_SOURCES = \
	gfsminfo_main.c \
	gfsminfo_cmdparser.c gfsminfo_cmdparser.h

gfsminfo_main.o: gfsminfo_cmdparser.h

gfsminfo_LDFLAGS = $(LDFLAGS_COMMON)
gfsminfo_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsminfo.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmarcsort_SOURCES = \
	gfsmarcsort_main.c \
	gfsmarcsort_cmdparser.c gfsmarcsort_cmdparser.h

gfsmarcsort_main.o: gfsmarcsort_cmdparser.h

gfsmarcsort_LDFLAGS = $(LDFLAGS_COMMON)
gfsmarcsort_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmarcsort.gog
##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmcomplement_SOURCES = \
	gfsmcomplement_main.c \
	gfsmcomplement_cmdparser.c gfsmcomplement_cmdparser.h

gfsmcomplement_main.o: gfsmcomplement_cmdparser.h

gfsmcomplement_LDFLAGS = $(LDFLAGS_COMMON)
gfsmcomplement_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmcomplement.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmcompose_SOURCES = \
	gfsmcompose_main.c \
	gfsmcompose_cmdparser.c gfsmcompose_cmdparser.h

gfsmcompose_main.o: gfsmcompose_cmdparser.h

gfsmcompose_LDFLAGS = $(LDFLAGS_COMMON)
gfsmcompose_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmcompose.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmcompre_SOURCES = \
	gfsmcompre_main.c \
	gfsmcompre_cmdparser.c gfsmcompre_cmdparser.h

gfsmcompre_main.o: gfsmcompre_cmdparser.h

gfsmcompre_LDFLAGS = $(LDFLAGS_COMMON)
gfsmcompre_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmcompre.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmconcat_SOURCES = \
	gfsmconcat_main.c \
	gfsmconcat_cmdparser.c gfsmconcat_cmdparser.h

gfsmconcat_main.o: gfsmconcat_cmdparser.h

gfsmconcat_LDFLAGS = $(LDFLAGS_COMMON)
gfsmconcat_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmconcat.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmclosure_SOURCES = \
	gfsmclosure_main.c \
	gfsmclosure_cmdparser.c gfsmclosure_cmdparser.h

gfsmclosure_main.o: gfsmclosure_cmdparser.h

gfsmclosure_LDFLAGS = $(LDFLAGS_COMMON)
gfsmclosure_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmclosure.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmconnect_SOURCES = \
	gfsmconnect_main.c \
	gfsmconnect_cmdparser.c gfsmconnect_cmdparser.h

gfsmconnect_main.o: gfsmconnect_cmdparser.h

gfsmconnect_LDFLAGS = $(LDFLAGS_COMMON)
gfsmconnect_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmconnect.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmdeterminize_SOURCES = \
	gfsmdeterminize_main.c \
	gfsmdeterminize_cmdparser.c gfsmdeterminize_cmdparser.h

gfsmdeterminize_main.o: gfsmdeterminize_cmdparser.h

gfsmdeterminize_LDFLAGS = $(LDFLAGS_COMMON)
gfsmdeterminize_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmdeterminize.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmdifference_SOURCES = \
	gfsmdifference_main.c \
	gfsmdifference_cmdparser.c gfsmdifference_cmdparser.h

gfsmdifference_main.o: gfsmdifference_cmdparser.h

gfsmdifference_LDFLAGS = $(LDFLAGS_COMMON)
gfsmdifference_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmdifference.gog


##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmdraw_SOURCES = \
	gfsmdraw_main.c \
	gfsmdraw_cmdparser.c gfsmdraw_cmdparser.h

gfsmdraw_main.o: gfsmdraw_cmdparser.h

gfsmdraw_LDFLAGS = $(LDFLAGS_COMMON)
gfsmdraw_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmdraw.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmheader_SOURCES = \
	gfsmheader_main.c \
	gfsmheader_cmdparser.c gfsmheader_cmdparser.h

gfsmheader_main.o: gfsmheader_cmdparser.h

gfsmheader_LDFLAGS = $(LDFLAGS_COMMON)
gfsmheader_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmheader.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmintersect_SOURCES = \
	gfsmintersect_main.c \
	gfsmintersect_cmdparser.c gfsmintersect_cmdparser.h

gfsmintersect_main.o: gfsmintersect_cmdparser.h

gfsmintersect_LDFLAGS = $(LDFLAGS_COMMON)
gfsmintersect_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmintersect.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsminvert_SOURCES = \
	gfsminvert_main.c \
	gfsminvert_cmdparser.c gfsminvert_cmdparser.h

gfsminvert_main.o: gfsminvert_cmdparser.h

gfsminvert_LDFLAGS = $(LDFLAGS_COMMON)
gfsminvert_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsminvert.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmlabels_SOURCES = \
	gfsmlabels_main.c \
	gfsmlabels_cmdparser.c gfsmlabels_cmdparser.h

gfsmlabels_main.o: gfsmlabels_cmdparser.h

gfsmlabels_LDFLAGS = $(LDFLAGS_COMMON)
gfsmlabels_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmlabels.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmlookup_SOURCES = \
	gfsmlookup_main.c \
	gfsmlookup_cmdparser.c gfsmlookup_cmdparser.h

gfsmlookup_main.o: gfsmlookup_cmdparser.h

gfsmlookup_LDFLAGS = $(LDFLAGS_COMMON)
gfsmlookup_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmlookup.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmoptional_SOURCES = \
	gfsmoptional_main.c \
	gfsmoptional_cmdparser.c gfsmoptional_cmdparser.h

gfsmoptional_main.o: gfsmoptional_cmdparser.h

gfsmoptional_LDFLAGS = $(LDFLAGS_COMMON)
gfsmoptional_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmoptional.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmproduct_SOURCES = \
	gfsmproduct_main.c \
	gfsmproduct_cmdparser.c gfsmproduct_cmdparser.h

gfsmproduct_main.o: gfsmproduct_cmdparser.h

gfsmproduct_LDFLAGS = $(LDFLAGS_COMMON)
gfsmproduct_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmproduct.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmproject_SOURCES = \
	gfsmproject_main.c \
	gfsmproject_cmdparser.c gfsmproject_cmdparser.h

gfsmproject_main.o: gfsmproject_cmdparser.h

gfsmproject_LDFLAGS = $(LDFLAGS_COMMON)
gfsmproject_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmproject.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmrenumber_SOURCES = \
	gfsmrenumber_main.c \
	gfsmrenumber_cmdparser.c gfsmrenumber_cmdparser.h

gfsmrenumber_main.o: gfsmrenumber_cmdparser.h

gfsmrenumber_LDFLAGS = $(LDFLAGS_COMMON)
gfsmrenumber_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmrenumber.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmreplace_SOURCES = \
	gfsmreplace_main.c \
	gfsmreplace_cmdparser.c gfsmreplace_cmdparser.h

gfsmreplace_main.o: gfsmreplace_cmdparser.h

gfsmreplace_LDFLAGS = $(LDFLAGS_COMMON)
gfsmreplace_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmreplace.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmreverse_SOURCES = \
	gfsmreverse_main.c \
	gfsmreverse_cmdparser.c gfsmreverse_cmdparser.h

gfsmreverse_main.o: gfsmreverse_cmdparser.h

gfsmreverse_LDFLAGS = $(LDFLAGS_COMMON)
gfsmreverse_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmreverse.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmrmepsilon_SOURCES = \
	gfsmrmepsilon_main.c \
	gfsmrmepsilon_cmdparser.c gfsmrmepsilon_cmdparser.h

gfsmrmepsilon_main.o: gfsmrmepsilon_cmdparser.h

gfsmrmepsilon_LDFLAGS = $(LDFLAGS_COMMON)
gfsmrmepsilon_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmrmepsilon.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmsigma_SOURCES = \
	gfsmsigma_main.c \
	gfsmsigma_cmdparser.c gfsmsigma_cmdparser.h

gfsmsigma_main.o: gfsmsigma_cmdparser.h

gfsmsigma_LDFLAGS = $(LDFLAGS_COMMON)
gfsmsigma_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmsigma.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmstrings_SOURCES = \
	gfsmstrings_main.c \
	gfsmstrings_cmdparser.c gfsmstrings_cmdparser.h

gfsmstrings_main.o: gfsmstrings_cmdparser.h

gfsmstrings_LDFLAGS = $(LDFLAGS_COMMON)
gfsmstrings_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmstrings.gog


##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmunion_SOURCES = \
	gfsmunion_main.c \
	gfsmunion_cmdparser.c gfsmunion_cmdparser.h

gfsmunion_main.o: gfsmunion_cmdparser.h

gfsmunion_LDFLAGS = $(LDFLAGS_COMMON)
gfsmunion_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmunion.gog

##~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
gfsmviterbi_SOURCES = \
	gfsmviterbi_main.c \
	gfsmviterbi_cmdparser.c gfsmviterbi_cmdparser.h

gfsmviterbi_main.o: gfsmviterbi_cmdparser.h

gfsmviterbi_LDFLAGS = $(LDFLAGS_COMMON)
gfsmviterbi_LDADD = $(LDADD_COMMON)

EXTRA_DIST += gfsmviterbi.gog

##-----------------------------------------------------------------------
## headers
##-----------------------------------------------------------------------

## --- installable headers: @includedir@/@pkgname@
#pkginclude_HEADERS = ${hfile_1} ... ${hfile_N}

## --- no-install headers
noinst_HEADERS = config.h

##-----------------------------------------------------------------------
## pre-compile rules: gengetopt
##-----------------------------------------------------------------------

##-----------------------------------------------------------------------
## pre-compile rules: Getopt::Gen
##-----------------------------------------------------------------------
.SECONDARY:

OPTGENFLAGS = -u -l --no-handle-rcfile

if HAVE_OPTGEN

.gog_cmdparser.h:
	optgen.perl $(OPTGENFLAGS) --nopod -F $*_cmdparser $<

.gog_cmdparser.c:
	optgen.perl $(OPTGENFLAGS) --nopod -F $*_cmdparser $<

.gog.pod:
	optgen.perl $(OPTGENFLAGS) --nocfile --nohfile -F $* $<

endif

##-----------------------------------------------------------------------
## compile flags
##-----------------------------------------------------------------------

## --- preprocessor flags
AM_CPPFLAGS = -I@srcdir@/../libgfsm -I../libgfsm

## --- compiler flags (c++)
AM_CFLAGS = $(gfsm_WFLAGS) $(gfsm_OFLAGS)

##-----------------------------------------------------------------------
## linker flags
##   + library special handling of configure.in vars:
##-----------------------------------------------------------------------
#${prog_i}_LDFLAGS = -L. -static
#${prog_i}_LDADD   = ${non_src_file}.o -lsomelib

LDFLAGS_COMMON = -L../libgfsm
LDADD_COMMON   = ../libgfsm/libgfsm.la @gfsm_LIBS@

#${lib_i}_la_LDFLAGS = -L. -version-info ${lib_i_current}:${lib_i_rev}:${lib_i_age}
#${lib_i}_LIBADD     = -lotherlib

##-----------------------------------------------------------------------
## cleanup: fst
##-----------------------------------------------------------------------

fsmclean:
	rm -f *.afsa *.afst *.gfsa *.gfst *.thfsa *.thfst

##-----------------------------------------------------------------------
## Variables: cleanup
##-----------------------------------------------------------------------

## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean:  built by 'make'
#CLEANFILES =

## --- distclean: built by 'configure'
#DISTCLEANFILES = 

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = \
	*~ .*~ Makefile Makefile.in \
	*.man *.1 *.html *.txt *.tex \
	*.afsa *.afst *.gfsa *.gfst *.thfst \
	*.dot *.ps *.vcg

#	*_cmdparser.c *_cmdparser.h

##-----------------------------------------------------------------------
## Variables: distribution
##-----------------------------------------------------------------------

## --- extra distribution files
#EXTRA_DIST =

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

##-----------------------------------------------------------------------
## Rules: cleanup
##-----------------------------------------------------------------------
.PHONY: cvsclean

cvsclean: maintainer-clean ;
