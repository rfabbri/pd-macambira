# File: ./tests/Makefile.am
# Package: gfsm
# Description:
#   + test-level automake file
#
# Process this file with Automake to create Makefile.in.
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Options & Subdirectories
#-----------------------------------------------------------------------

## --- recursion subdirectories
#SUBDIRS =

#-----------------------------------------------------------------------
# Rules: test (check)
#-----------------------------------------------------------------------

$(srcdir)/package.m4: $(top_srcdir)/configure.in
	{                                      \
	 echo '# Signature of the current package.'; \
	 echo 'm4_define([AT_PACKAGE_NAME],      [@PACKAGE_NAME@])'; \
	 echo 'm4_define([AT_PACKAGE_TARNAME],   [@PACKAGE_TARNAME@])'; \
	 echo 'm4_define([AT_PACKAGE_VERSION],   [@PACKAGE_VERSION@])'; \
	 echo 'm4_define([AT_PACKAGE_STRING],    [@PACKAGE_STRING@])'; \
	 echo 'm4_define([AT_PACKAGE_BUGREPORT], [@PACKAGE_BUGREPORT@])'; \
	} >$(srcdir)/package.m4

TESTSUITE = $(srcdir)/testsuite
check-local: atconfig atlocal $(TESTSUITE)
	sh $(TESTSUITE)

AUTOTEST = autom4te --language=autotest
$(TESTSUITE):   $(srcdir)/testsuite.at \
		$(srcdir)/01_basic.at \
		$(srcdir)/02_arith.at \
		$(srcdir)/03_algebra.at
	$(AUTOTEST) -I $(srcdir) $^ -o $@.tmp
	mv $@.tmp $@

#-----------------------------------------------------------------------
# Variables: cleanup
#-----------------------------------------------------------------------

## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean: built by 'make'
#CLEANFILES =

## --- distclean: built by 'configure'
DISTCLEANFILES = \
	autom4te.cache \
	config.log \
	config.status \
	config.cache \
	atconfig \
	testsuite.log

distclean-local:
	rm -rf testsuite.dir

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = *~ \
	Makefile Makefile.in \
	aclocal.m4 \
	configure

#-----------------------------------------------------------------------
# Variables: distribution
#-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	atconfig \
	atlocal.in \
	package.m4 \
	01_basic.at \
	02_arith.at \
	testsuite.at \
	testsuite

## --- recursion subdirectories for 'make dist'
DIST_SUBDIRS = $(SUBDIRS)

## --- dist-hook: when another 'Makefile.am' is overkill
DISTHOOK_DIRS = data
DISTHOOK_FILES = \
	data/basic1.inf \
	data/basic1.tfst \
	data/basic2.labs.inf \
	data/basic2.labs.tfst \
	data/closure-plus-in.tfst \
	data/closure-plus-want.tfst \
	data/closure-star-in.tfst \
	data/closure-star-want.tfst \
	data/complement-b-in.tfst \
	data/complement-b-want.tfst \
	data/complement-in.tfst \
	data/complement-want.tfst \
	data/compose-in-1.tfst \
	data/compose-in-2.tfst \
	data/compose-want.tfst \
	data/concat-in-1.tfst \
	data/concat-in-2.tfst \
	data/concat-want.tfst \
	data/connect-in.tfst \
	data/connect-want.tfst \
	data/determinize-in.tfst \
	data/determinize-want.tfst \
	data/difference-in-1.tfst \
	data/difference-in-2.tfst \
	data/difference-want.tfst \
	data/intersect-in-1.tfst \
	data/intersect-in-2.tfst \
	data/intersect-want.tfst \
	data/intersect2-in-1.tfst \
	data/intersect2-in-2.tfst \
	data/intersect2-want.tfst \
	data/invert-in.tfst \
	data/invert-want.tfst \
	data/lookup-123-want.tfst \
	data/lookup-223-want.tfst \
	data/lookup.tfst \
	data/n_closure-in.tfst \
	data/n_closure-want.tfst \
	data/null.inf \
	data/null.tfst \
	data/optional-in.tfst \
	data/optional-want.tfst \
	data/project-hi-in.tfst \
	data/project-hi-want.tfst \
	data/project-lo-in.tfst \
	data/project-lo-want.tfst \
	data/renumber-in.tfst \
	data/renumber-want.tfst \
	data/rmepsilon-in.tfst \
	data/rmepsilon-want.tfst \
	data/test.lab \
	data/union-in-1.tfst \
	data/union-in-2.tfst \
	data/union-want.tfst

dist-hook:
	for d in $(DISTHOOK_DIRS); do\
	  mkdir -p $(distdir)/$$d ;\
	done
	for f in $(DISTHOOK_FILES); do\
	  cp -p $(srcdir)/$$f $(distdir)/$$f ;\
	done

#-----------------------------------------------------------------------
# Rules: cleanup
#-----------------------------------------------------------------------
.PHONY: cvsclean cvsclean-hook

cvsclean: maintainer-clean ;

