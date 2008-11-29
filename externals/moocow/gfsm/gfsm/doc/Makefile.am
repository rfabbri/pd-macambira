## File: doc/Makefile.am
## Package: gfsm
## Description:
##   + automake file for doc/ subdir
##
## Process this file with Automake to create Makefile.in.
##-----------------------------------------------------------------------

##-----------------------------------------------------------------------
## Primaries: headers
##-----------------------------------------------------------------------

## --- recursion subdirectories
SUBDIRS = libgfsm programs

#-----------------------------------------------------------------------
# Variables: cleanup
#-----------------------------------------------------------------------

## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean:  built by 'make'
#CLEANFILES =

## --- distclean: built by 'configure'
#DISTCLEANFILES =

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = *~ .*~ Makefile Makefile.in

#-----------------------------------------------------------------------
# Variables: (un)install
#-----------------------------------------------------------------------
pkgdoc_UNINSTALL_FILES = libgfsm programs README.txt
uninstall-local:
	for f in $(pkgdoc_UNINSTALL_FILES) ; do \
	   rm -rf $(DESTDIR)$(pkgdocdir)/$$f ;\
	done

#-----------------------------------------------------------------------
# Variables: distribution
#-----------------------------------------------------------------------

## --- extra distribution files
#EXTRA_DIST = $(man_MANS) $(pkgdata_DATA)

## --- recursion subdirectories for 'make dist'
#DIST_SUBDIRS =

#-----------------------------------------------------------------------
# Rules: cleanup
#-----------------------------------------------------------------------
.PHONY: cvsclean

cvsclean: maintainer-clean ;
