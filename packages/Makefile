#==============================================================================#
#
# Centralized cross-platform build system 
#
# see for instructions: http://puredata.org/docs/developer/build
#  <hans@at.or.at>
#
#==============================================================================#

CWD := $(shell pwd)

cvs_root_dir = $(CWD)/..
DESTDIR = $(CWD)/build
BUILDLAYOUT_DIR = $(CWD)


# default target
all: pd abstractions doc externals
#all: pd abstractions doc externals flext gem
	@echo "Complete build succeeded!"

include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

#==============================================================================#
#
# GET VERSIONS FOR RELEVANT SOFTWARE
#
#==============================================================================#


CYCLONE_MAJOR_VERSION := $(shell grep CYCLONE_VERSION \
	$(externals_src)/miXed/cyclone/build_counter | cut -d ' ' -f 3 | \
	cut -d '"' -f 2)
CYCLONE_RELEASE := $(shell grep CYCLONE_RELEASE \
	$(externals_src)/miXed/cyclone/build_counter | cut -d ' ' -f 3 | \
	cut -d '"' -f 2)
CYCLONE_BUILD := $(shell grep CYCLONE_BUILD \
	$(externals_src)/miXed/cyclone/build_counter | cut -d ' ' -f 3 | \
	cut -d '"' -f 2)
CYCLONE_VERSION := $(CYCLONE_MAJOR_VERSION) $(CYCLONE_RELEASE) $(CYCLONE_BUILD)


FLEXT_MAJOR_VERSION := $(shell grep FLEXTMAJOR \
	$(externals_src)/grill/flext/buildsys/version.inc | cut -d '=' -f2)
FLEXT_MINOR_VERSION := $(shell grep FLEXTMINOR \
	$(externals_src)/grill/flext/buildsys/version.inc | cut -d '=' -f2)
FLEXT_MICRO_VERSION := $(shell grep FLEXTMICRO \
	$(externals_src)/grill/flext/buildsys/version.inc | cut -d '=' -f2)
FLEXT_VERSION := $(FLEXT_MAJOR_VERSION).$(FLEXT_MINOR_VERSION).$(FLEXT_MICRO_VERSION)


GEM_VERSION := $(shell grep "GEM_VERSION" $(gem_src)/src/Base/GemVersion.h | \
	cut -d '"' -f 2)


MAXLIB_VERSION := $(shell grep "define VERSION" \
	$(externals_src)/maxlib/maxlib.c | cut -d '"' -f 2)


OSC_VERSION := $(shell grep "define VERSION" $(externals_src)/OSCx/src/OSC.c | \
	cut -d '"' -f 2)


PDP_VERSION := $(shell grep PDP_VERSION= $(externals_src)/pdp/configure | \
	cut -d '=' -f 2)


PMPD_VERSION := $(shell grep "define VERSION" \
	$(externals_src)/pmpd/src/pmpd.c | cut -d '"' -f 2)


TOXY_MAJOR_VERSION := $(shell grep TOXY_VERSION \
	$(externals_src)/miXed/toxy/build_counter | cut -d ' ' -f 3 | \
	cut -d '"' -f 2)
TOXY_RELEASE := $(shell grep TOXY_RELEASE \
	$(externals_src)/miXed/toxy/build_counter | cut -d ' ' -f 3 | \
	cut -d '"' -f 2)
TOXY_BUILD := $(shell grep TOXY_BUILD \
	$(externals_src)/miXed/toxy/build_counter | cut -d ' ' -f 3 | \
	cut -d '"' -f 2)
TOXY_VERSION := $(TOXY_MAJOR_VERSION) $(TOXY_RELEASE) $(TOXY_BUILD)


ZEXY_VERSION := $(shell grep VERSION $(externals_src)/zexy/src/zexy.h | \
	grep -v _VERSION | cut -d ' ' -f 3 | cut -d '"' -f 2)



OPT_FLAGS = -O6 -funroll-loops -fomit-frame-pointer

# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				cvs_root_dir=$(cvs_root_dir) \
				DESTDIR=$(DESTDIR) \
				prefix=$(prefix) \
				libpddir=$(libpddir) \
				OPT_CFLAGS="$(OPT_CFLAGS)" \
				UNAME=$(UNAME)


#==============================================================================#
#
# BUILD TARGETS
#
#==============================================================================#

#------------------------------------------------------------------------------
# abstractions
abstractions:


#------------------------------------------------------------------------------
# doc
doc:


#------------------------------------------------------------------------------
# externals
externals: 
	cd $(externals_src) && make $(DEST_PATHS)


#------------------------------------------------------------------------------
# flext
flext:
# nusmuk
	-cd $(externals_src)/nusmuk/msd && $(externals_src)/grill/flext/build.sh \
		pd gcc
	-cd $(externals_src)/nusmuk/msd2D && $(externals_src)/grill/flext/build.sh \
		pd gcc
	-cd $(externals_src)/nusmuk/msd3D && $(externals_src)/grill/flext/build.sh \
		pd gcc

#------------------------------------------------------------------------------
# Gem
gem:
#	cd $(gem_src)/Gem && xcodebuild


#------------------------------------------------------------------------------
# pd

ifeq ($(OS_NAME),windows)
  PD_CONFIGURE_FLAGS = 
else
  PD_CONFIGURE_FLAGS = --enable-jack
endif

PD_BUILD_FLAGS = 

# Pd needs autoconf 2.5x, which is labeled differently on different machines
#PD_AUTOCONF := ${shell ( test -x "`which autoconf-2.59`" && echo autoconf-2.59 ) || echo autoconf }
PD_AUTOCONF = autoconf

$(pd_src)/src/configure: $(pd_src)/src/configure.in
	cd $(pd_src)/src/ && $(PD_AUTOCONF)

# this is not used yet because MinGW doesn't use ./configure && make yet
#$(pd_src)/src/makefile: $(pd_src)/src/makefile.in
#	cd $(pd_src)/src/ && ./configure $(PD_CONFIGURE_FLAGS)

pd: $(pd_src)/src/configure
	echo $$MACOSX_DEPLOYMENT_TARGET
	-cd $(pd_src)/src && ./configure $(PD_CONFIGURE_FLAGS) && \
		make $(DEST_PATHS) $(PD_BUILD_FLAGS)


#==============================================================================#
#
# INSTALL TARGETS
#
#==============================================================================#

# this is used for installing into a pre-build Pd binary
extended_install: abstractions_install doc_install extensions_install \
externals_install flext_install gem_install readme_install welcome_install \
license_install noncvs_install


#------------------------------------------------------------------------------
# install
install: pd_install extended_install
	@echo "Complete install succeeded!"

#------------------------------------------------------------------------------
# abstractions_install
abstractions_install:
	cd $(abstractions_src) && make $(DEST_PATHS) install

#------------------------------------------------------------------------------
# doc_install
doc_install: 
	cd $(doc_src) && make $(DEST_PATHS) install


#------------------------------------------------------------------------------
# extensions_install
extensions_install: 
	cd $(extensions_src) && make $(DEST_PATHS) install


#------------------------------------------------------------------------------
# externals_install
externals_install: 
	cd $(externals_src) && make $(DEST_PATHS) install



#------------------------------------------------------------------------------
# flext_install
flext_install:
# FLEXT
# temp hack -HCS
#	test -f $(externals_src)/grill/flext/buildsys/mac/pd/config-gcc.def || \
#		cp $(externals_src)/grill/flext/buildsys/mac/pd/config-gcc.def \
#			$(externals_src)/grill/flext/buildsys/config-mac-pd-gcc.txt
# end temp hack
# tigital 8/30/2005 added
#	cd $(externals_src)/grill/flext && \
#		( \
#        test -f buildsys/config-mac-pd-gcc.txt || \
#          ( \
#            cp buildsys/mac/pd/config-gcc.def buildsys/config-mac-pd-gcc.txt && \
#            echo Copied template sys config file \
#          ) \
#      ) && \
#		( \
#			cat buildsys/config-mac-pd-gcc.txt | \
#				sed "s/^PDPATH=.*/PDPATH=..\/..\/..\/pd/" > buildsys/config-mac-pd-gcc.txt \
#		) && \
#		( \
#			cat package.txt | sed "s/^PRECOMPILE=.*/PRECOMPILE=/" > package.txt \
#		) && \
#		( \
#			cat buildsys/mac/pd/gnumake-gcc-ext.inc | \
#				sed "s/^LDFLAG\S +=.*/LDFLAG\S += -bundle -bundle_loader ..\/..\/..\/pd\/bin\/pd/" > buildsys/mac/pd/gnumake-gcc-ext.inc \
#		) && \
#			( \
#			cat buildsys/mac/pd/gnumake-gcc-flext.inc | \
#				sed "s/^LDFLAG\S +=.*/LDFLAG\S += -dynamiclib -undefined dynamic_lookup /" > buildsys/mac/pd/gnumake-gcc-flext.inc \
#		) && \
#		( \
#			test -f config.txt || \
#			( \
#				cp -f build/config-mac.def config.txt && \
#				echo Copied template flext config file \
#			) \
#		) && \
#		MACOSX_DEPLOYMENT_TARGET=10.3 bash build.sh pd gcc build-release-shared FLEXTLIB=@executable_path/../extra FLEXTINC=../flext/source FLEXTSYS=$(objectsdir)
#	ln -sf $(externals_src)/grill/flext/pd-darwin/release-shared/libflext-pd.$(FLEXT_VERSION).dylib $(externals_src)/grill/flext/pd-darwin/release-shared/libflext-pd.dylib
#	install -p $(externals_src)/grill/flext/pd-darwin/release-shared/libflext-pd.$(FLEXT_VERSION).dylib $(objectsdir)
#	ln -sf libflext-pd.$(FLEXT_VERSION).dylib $(objectsdir)/libflext-pd.dylib
	install -d $(manualsdir)/flext
	cp -rp $(externals_src)/grill/flext/tutorial $(manualsdir)/flext
	install -p $(externals_src)/grill/flext/gpl.txt  \
		$(externals_src)/grill/flext/readme.txt   \
		$(externals_src)/grill/flext/license.txt  \
		$(manualsdir)/flext
# FLEXTERNALS
#------------------------------------------------------------------------------#
# dyn~
#	-cd $(externals_src)/grill/dynext && \
#		bash ../flext/build.sh pd gcc build-release-shared \
#			FLEXTLIB=../flext/pd-darwin/release-shared \
#			FLEXTINC=../flext/source FLEXTSYS=$(objectsdir)
	-install -p $(externals_src)/grill/dynext/pd-darwin/release-shared/*.$(EXTENSION) $(objectsdir)
	install -p $(externals_src)/grill/dynext/pd/*.pd $(helpdir)
	install -d $(manualsdir)/dynext
	install -p $(externals_src)/grill/dynext/gpl.txt  \
		$(externals_src)/grill/dynext/readme.txt   \
		$(externals_src)/grill/dynext/license.txt  \
		$(manualsdir)/dynext
#########
# pool
#	-cd $(externals_src)/grill/pool && bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(objectsdir) FLEXTINC=../flext/source
	-install -p $(externals_src)/grill/pool/pd-darwin/release-shared/*.$(EXTENSION) $(objectsdir)
	install -p $(externals_src)/grill/pool/pool-help.pd $(helpdir)
	install -d $(manualsdir)/pool
	install -p $(externals_src)/grill/pool/gpl.txt \
		$(externals_src)/grill/pool/readme.txt   \
		$(externals_src)/grill/pool/license.txt \
		$(manualsdir)/pool
#########
# py/pyext
#	cd $(externals_src)/grill/py && \
#		( \
#			cp -f build/config-mac.def config.txt \
#		) && \
#		bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(objectsdir) FLEXTINC=../flext/source
#	install -p $(externals_src)/grill/py/pd-darwin/release-shared/*.$(EXTENSION) $(objectsdir)
# this wasn't building for me -HCS 2005-06-09
	install -d $(examplesdir)/py/examples
	install -d $(examplesdir)/py/scripts
	install -p $(externals_src)/grill/py/pd/*.pd $(examplesdir)/py/examples
	install -p $(externals_src)/grill/py/scripts/*.py $(examplesdir)/py/scripts
	install -d $(manualsdir)/py/scripts
	install -p $(externals_src)/grill/py/gpl.txt  \
		$(externals_src)/grill/py/readme.txt $(externals_src)/grill/py/license.txt \
		$(manualsdir)/py
#	install -d $(prefix)/Library/Frameworks
#	cp -R /Library/Frameworks/Python.framework $(prefix)/Library/Frameworks
#########
# vasp
#  fails without the dir
#	test -d ../../vasp/pd-darwin || mkdir -p ../../vasp/pd-darwin 
#	cd $(externals_src)/grill/vasp/ && bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(objectsdir) FLEXTINC=../flext/source
#	install -p $(externals_src)/grill/vasp/pd-darwin/release-shared/*.$(EXTENSION) $(objectsdir)
# this wasn't building for me -HCS 2005-06-09
	install -d $(helpdir)/vasp
	install -p $(externals_src)/grill/vasp/pd-help/*.pd $(helpdir)/vasp
	install -d $(manualsdir)/vasp
	install -p $(externals_src)/grill/vasp/gpl.txt  \
		$(externals_src)/grill/vasp/readme.txt   \
		$(externals_src)/grill/vasp/license.txt  \
		$(externals_src)/grill/vasp/changes.txt  \
		$(externals_src)/grill/vasp/todo.txt     \
		$(manualsdir)/vasp
	install -d $(examplesdir)/vasp
	cp -rp $(externals_src)/grill/vasp/pd-ex $(examplesdir)/vasp
#########
# xsample
#	cd $(externals_src)/grill/xsample && bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(objectsdir) FLEXTINC=../flext/source
#	install -p $(externals_src)/grill/xsample/pd-darwin/release-shared/*.$(EXTENSION) $(objectsdir)
	install -p $(externals_src)/grill/xsample/pd/*.pd $(helpdir)
	install -d $(manualsdir)/xsample
	install -p $(externals_src)/grill/xsample/gpl.txt \
		$(externals_src)/grill/xsample/readme.txt \
		$(externals_src)/grill/xsample/license.txt \
		$(manualsdir)/xsample
########## FOOTILS #########
# syncgrain
#	cd $(externals_src)/footils/syncgrain/ && $(MAKE) -f makefile.pd-darwin
#	install -p $(externals_src)/footils/syncgrain/pd-darwin/*.$(EXTENSION) \
#		$(objectsdir)
	install -p $(externals_src)/footils/syncgrain/pd/*.* $(helpdir)
########## NUSMUK ##########
# nusmuk
#	cd $(externals_src)/nusmuk/msd && $(externals_src)/grill/flext/build.sh \
#		pd gcc install
#	cd $(externals_src)/nusmuk/msd2D && $(externals_src)/grill/flext/build.sh \
#		pd gcc install
#	cd $(externals_src)/nusmuk/msd3D && $(externals_src)/grill/flext/build.sh \
#		pd gcc install
#TODO: need to add nusmuk/editor


#------------------------------------------------------------------------------
# gem_install
GEM_NAME = Gem
gem_install: gem $(helpdir)
	install -d $(helpdir)/$(GEM_NAME)
	install -p $(gem_src)/help/*.* $(helpdir)/$(GEM_NAME)
	install -p $(gem_src)/abstractions/*-help.pd $(helpdir)
	install -d $(objectsdir)/$(GEM_NAME)
	install -p $(shell ls -1 $(gem_src)/abstractions/*.* | \
		grep -v '\-help.pd')  $(objectsdir)
	install -d $(manualsdir)/$(GEM_NAME)
	install -p $(gem_src)/doc/*.* $(manualsdir)/$(GEM_NAME)
	install -d $(manualsdir)/$(GEM_NAME)/manual
	install -p $(gem_src)/manual/*.* $(manualsdir)/$(GEM_NAME)/manual
	for dir in $(shell ls -1 $(gem_src)/examples | grep -v CVS); do \
		echo "installing $$dir"; \
		install -d $(examplesdir)/$(GEM_NAME)/$$dir ; \
		install -p $(gem_src)/examples/$$dir/*.* $(examplesdir)/$(GEM_NAME)/$$dir ; \
	done


#------------------------------------------------------------------------------
# noncvs_install
# this is for including pre-compiled binaries in a build
noncvs_install:
		-install -p $(packages_src)/noncvs/$(OS_NAME)/bin/*.* $(bindir)
		-install -p $(packages_src)/noncvs/$(OS_NAME)/doc/5.reference/*.* \
			$(helpdir)
		-install -p $(packages_src)/noncvs/$(OS_NAME)/extra/*.* $(objectsdir)
		-install -d $(DESTDIR)$(prefix)/gripd
		-install -p $(packages_src)/noncvs/$(OS_NAME)/gripd/*.* \
			 $(DESTDIR)$(prefix)/gripd


#------------------------------------------------------------------------------
# pd_install
PD_NAME=Pd
pd_install: pd
	cd $(pd_src)/src && \
		make $(DEST_PATHS) $(PD_BUILD_FLAGS) install
	install -d $(manualsdir)/$(PD_NAME)
	install -p $(pd_src)/src/notes.txt $(manualsdir)/$(PD_NAME)



#==============================================================================#
#
# GENERATE TEXT FILES FOR PACKAGE
#
#==============================================================================#

LICENSE_FILE = $(manualsdir)/$(PD_NAME)/License.html
license_install:
  # generate HTML version of License
	install -d $(manualsdir)/$(PD_NAME)
	-rm $(LICENSE_FILE)
	touch $(LICENSE_FILE)
	echo "<html><body>" >> "$(LICENSE_FILE)"
	echo "<h3>(Parts of this package can be used under " >> "$(LICENSE_FILE)"
	echo "<a href="Pd-LICENSE.txt">Pd&quot;s BSD license</a>)</h3>" >> "$(LICENSE_FILE)"
	echo "<font size=\"-1\">" >> "$(LICENSE_FILE)"
	cat "$(externals_src)/creb/COPYING" | sed -e 's/^$$/\<P\>/g' >> "$(LICENSE_FILE)"
	echo "</font></body></html>" >> $(LICENSE_FILE)
# Pd's license file
	install -p "$(pd_src)/LICENSE.txt" "$(manualsdir)/$(PD_NAME)/Pd-LICENSE.txt"



WELCOME_FILE = $(manualsdir)/$(PD_NAME)/Welcome.html
welcome_install:
	install -d $(manualsdir)/$(PD_NAME)
	-rm $(WELCOME_FILE)
	touch $(WELCOME_FILE)
	echo "<html><head>" >> $(WELCOME_FILE)
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"http://puredata.org/ploneCustom.css\" />" >> $(README_FILE)
	echo "</head>" >> $(README_FILE)
	echo "<body><p><p>" >> $(WELCOME_FILE)
	echo "<center><img src=\"\">" >> $(WELCOME_FILE)
	echo "<h2>Version $(PD_VERSION)</h2>" >> $(WELCOME_FILE)
	echo "<p>written by Miller S. Puckette</p></center>" >> $(WELCOME_FILE)
	echo "<font size=\"-1\">" >> $(WELCOME_FILE)
	echo "<p>`grep -A9 ACKNOWLEDG $(pd_src)/README.txt`</p>" >> $(WELCOME_FILE)
	echo "</font>" >> $(WELCOME_FILE)
	echo "</body></html>" >> $(WELCOME_FILE)



README_FILE = $(manualsdir)/$(PD_NAME)/ReadMe.html
readme_install: 
	install -d $(manualsdir)/$(PD_NAME)
	echo $(CYCLONE_RELEASE)
	-rm $(README_FILE)
	touch $(README_FILE)
	echo "<html>" >> $(README_FILE)
	echo "<head>" >> $(README_FILE)
	echo "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=utf-8\" />" >> $(README_FILE)
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"http://puredata.org/plone.css\" />" >> $(README_FILE)
	echo "</head>" >> $(README_FILE)
	echo "<body>" >> $(README_FILE)
	echo "<h2>Pure Data $(PD_VERSION)-$(PACKAGE_VERSION)</h2>" 	>> $(README_FILE)
	echo "<p>Pd is a free real-time computer music software package resembling Max.  It provides a patchable environment for audio analysis, synthesis, and processing, with a rich set of multimedia capabilities.  You can get Pd for Linux, Windows, MacOS X, BSD, or IRIX.</p>" >> $(README_FILE)
	echo "<p>For more information, go to: <a href=\"http://puredata.org\" target=\"pd\">http://puredata.org</a></p>" >> $(README_FILE)
	echo "<h3>Installation</h3>" >> $(README_FILE)	
	echo "<dl><dt>GNU/Linux</dt><dd>" >> $(README_FILE)
	echo "<p></p>" >> $(README_FILE)
	echo "<p></p>" >> $(README_FILE)
	echo "</dd><dt>Mac OS X</dt>" >> $(README_FILE)
	echo "<dd>" >> $(README_FILE)
	echo "<p>To install Pd, drag the Pd.app to anywhere in your hard disk.</p>" >> $(README_FILE)	
	echo "<p>To install Gem, pmpd, xsample, dyn~, and vasp support, copy <code>org.puredata.pd.plist</code> to <code>~/Library/Preferences</code> (<code>~</code> means your home folder).  WARNING: this will overwrite any existing Pd preferences!</p>" >> $(README_FILE)	
	echo "</dd>" >> $(README_FILE)
	echo "<dt>Windows</dt><dd>" >> $(README_FILE)
	echo "<p>To install, run the installer.</p>" >> $(README_FILE)
	echo "<p>To make sure that all of the libraries are loaded when Pd runs, " >> $(README_FILE)
	echo "double-click <code>C:\Program Files\pd\pd-settings.reg</code> to import the settings to the registry.</p>" >> $(README_FILE)
	echo "</dd></dl>" >> $(README_FILE)
	echo "<h3>Pure Data CVS Developers</h3>" >> $(README_FILE)	
# this may seem whack, but it generates the list of developers from the SourceForge site:
	curl 'http://sourceforge.net/project/memberlist.php?group_id=55736' | grep -A2 -e '<td>' | sed 's|\(href="\)|target="w" \1http://sourceforge.net|' >> $(README_FILE)
	echo "</p>" >> $(README_FILE)
	echo "<p>" >> $(README_FILE)
	echo "Many others not listed have contributed their time and effort, this is just a list of the current developers in the SourceForge project.  But really, every Pd user is a developer and is encouraged to contribute to the CVS repository." >> $(README_FILE)
	echo "</p>" >> $(README_FILE)
	echo "<h3>License</h3>" >> $(README_FILE)	
	echo "<p>" >> $(README_FILE)
	echo "The Pd core is licensed under a <a href="Pd-LICENSE.txt">BSD license</a>, almost every other part of this package is available under the <a href="http://www.gnu.org/copyleft/gpl.html" target="gpl">GNU GPL</a>.  A couple packages have BSD-style licenses too." >> $(README_FILE)
	echo "</p>" >> $(README_FILE)
	echo "<h3>Included Versions</h3>" >> $(README_FILE)	
	echo "<p>These externals are all included from the Pd CVS repository:</p>" >> $(README_FILE)
	echo "<ul>" >> $(README_FILE)
	echo "<li>pure data: $(PD_VERSION) " >> $(README_FILE)
	echo "<li>cyclone: $(CYCLONE_VERSION)" >> $(README_FILE)
	echo "<li>flext: $(FLEXT_VERSION)" >> $(README_FILE)
	echo "<li>gem: $(GEM_VERSION)" >> $(README_FILE)
	echo "<li>iemlib: `date +20%y.%m.%d`" >> $(README_FILE)
#
# these cause this error:
# Makefile:308: *** unterminated call to function `shell': missing `)'.  Stop.
#
	echo "<li>maxlib: $(MAXLIB_VERSION)" >> $(README_FILE)
	echo "<li>osc: $(OSC_VERSION)" >> $(README_FILE)
	echo "<li>pmpd: $(PMPD_VERSION)" >> $(README_FILE)
	echo "<li>pd-abstractions: `date +20%y.%m.%d`" >> $(README_FILE)
	echo "<li>pd-doc: `date +20%y.%m.%d`" >> $(README_FILE)
	echo "<li>pd-externals: `date +20%y.%m.%d`" >> $(README_FILE)
	echo "<li>pddp: `date +20%y.%m.%d`" >> $(README_FILE)
	echo "<li>pdp: $(PDP_VERSION)" >> $(README_FILE)
	echo "<li>toxy: $(TOXY_VERSION)" >> $(README_FILE)
	echo "<li>unauthorized: `date +20%y.%m.%d`" >> $(README_FILE)
	echo "<li>zexy: $(ZEXY_VERSION)" >> $(README_FILE)
	echo "</ul>" >> $(README_FILE)
	echo "(this package was built on `date`) <BR>" >> $(README_FILE)
	echo "</body></html>" >> $(README_FILE)



#==============================================================================#
#
## FINAL ASSEMBLY
#
#==============================================================================#


#----------------------------------------------------------------------------
# DOC_FORMAT
doc_format:
# clean out cruft files
	-find $(DESTDIR) -name .DS_Store -delete
	-find $(DESTDIR) -name '*.*.bak' -delete
# run script to move help-*.pd files to *-help.pd according to the standard
	cd $(helpdir) && \
		$(scripts_src)/convert-help-to-standard.sh
	cd $(helpdir)/iemabs && \
		$(scripts_src)/convert-help-to-standard.sh
# remove write perms to prevent people form editing the helpfiles by mistake
	find $(pddocdir) -name '*.pd' -print0 | xargs -0 chmod a-wx
	find $(pddocdir) -name '*.txt' -print0 | xargs -0 chmod a-wx



#==============================================================================#
#
# DEVELOPER TARGETS
#
#==============================================================================#

devsymlinks: devsymlinks_$(OS_NAME)



devsymlinks_windows:


devsymlinks_linux:


TCLTK_VERSION=8.4
devsymlinks_darwin: devsymlinks_clean
# this makes /usr/local symlinks to the Tcl/Tk frameworks so that ./configure
# can find them.
# headers
	test -e  /Library/Frameworks/Tcl.framework/Versions/$(TCLTK_VERSION)/Headers/tcl.h && \
		sudo ln -s \
			/Library/Frameworks/Tcl.framework/Versions/$(TCLTK_VERSION)/Headers/tcl.h \
			/usr/local/include/tcl.h
	test -e  /Library/Frameworks/Tcl.framework/Versions/$(TCLTK_VERSION)/Headers/tclDecls.h && \
		sudo ln -s \
			/Library/Frameworks/Tcl.framework/Versions/$(TCLTK_VERSION)/Headers/tclDecls.h \
			/usr/local/include/tclDecls.h
	test -e  /Library/Frameworks/Tcl.framework/Versions/$(TCLTK_VERSION)/Headers/tclPlatDecls.h && \
		sudo ln -s \
			/Library/Frameworks/Tcl.framework/Versions/$(TCLTK_VERSION)/Headers/tclPlatDecls.h \
			/usr/local/include/tclPlatDecls.h
# libs
	test -e  /Library/Frameworks/Tcl.framework/tclConfig.sh && \
		sudo ln -s /Library/Frameworks/Tcl.framework/tclConfig.sh /usr/local/lib
	test -e  /Library/Frameworks/Tcl.framework/Versions/$(TCLTK_VERSION)/Tcl && \
		sudo ln -s \
			/Library/Frameworks/Tcl.framework/Versions/$(TCLTK_VERSION)/Tcl \
			/usr/local/lib/libtcl$(TCLTK_VERSION).dylib
	test -e  /Library/Frameworks/Tcl.framework/Versions/$(TCLTK_VERSION)/libtclstub$(TCLTK_VERSION).a && \
		sudo ln -s \
			/Library/Frameworks/Tcl.framework/Versions/$(TCLTK_VERSION)/libtclstub$(TCLTK_VERSION).a \
			/usr/local/lib/libtclstub$(TCLTK_VERSION).a
	test -e  /usr/local/lib/libtcl$(TCLTK_VERSION).dylib && \
		sudo ln -s /usr/local/lib/libtcl$(TCLTK_VERSION).dylib /usr/local/lib/libtcl.dylib
# headers
	test -e  /Library/Frameworks/Tk.framework/Versions/$(TCLTK_VERSION)/Headers/tk.h && \
		sudo ln -s \
			/Library/Frameworks/Tk.framework/Versions/$(TCLTK_VERSION)/Headers/tk.h \
			/usr/local/include/tk.h
	test -e  /Library/Frameworks/Tk.framework/Versions/$(TCLTK_VERSION)/Headers/tkDecls.h && \
		sudo ln -s \
			/Library/Frameworks/Tk.framework/Versions/$(TCLTK_VERSION)/Headers/tkDecls.h \
			/usr/local/include/tkDecls.h
	test -e  /Library/Frameworks/Tk.framework/Versions/$(TCLTK_VERSION)/Headers/tkIntXlibDecls.h && \
		sudo ln -s \
			/Library/Frameworks/Tk.framework/Versions/$(TCLTK_VERSION)/Headers/tkIntXlibDecls.h \
			/usr/local/include/tkIntXlibDecls.h
	test -e  /Library/Frameworks/Tk.framework/Versions/$(TCLTK_VERSION)/Headers/tkMacOSX.h && \
		sudo ln -s \
			/Library/Frameworks/Tk.framework/Versions/$(TCLTK_VERSION)/Headers/tkMacOSX.h \
			/usr/local/include/tkMacOSX.h
	test -e  /Library/Frameworks/Tk.framework/Versions/$(TCLTK_VERSION)/Headers/tkPlatDecls.h && \
		sudo ln -s \
			/Library/Frameworks/Tk.framework/Versions/$(TCLTK_VERSION)/Headers/tkPlatDecls.h \
			/usr/local/include/tkPlatDecls.h
# libs
	test -e  /Library/Frameworks/Tk.framework/tkConfig.sh && \
		sudo ln -s /Library/Frameworks/Tk.framework/tkConfig.sh /usr/local/lib
	test -e  /Library/Frameworks/Tk.framework/Versions/$(TCLTK_VERSION)/Tk && \
		sudo ln -s /Library/Frameworks/Tk.framework/Versions/$(TCLTK_VERSION)/Tk \
			/usr/local/lib/libtk$(TCLTK_VERSION).dylib
	test -e /Library/Frameworks/Tk.framework/Versions/$(TCLTK_VERSION)/libtkstub$(TCLTK_VERSION).a && \
		sudo ln -s \
			/Library/Frameworks/Tk.framework/Versions/$(TCLTK_VERSION)/libtkstub$(TCLTK_VERSION).a \
			/usr/local/lib/libtkstub$(TCLTK_VERSION).a
	test -e  /usr/local/lib/libtk$(TCLTK_VERSION).dylib && \
		sudo ln -s /usr/local/lib/libtk$(TCLTK_VERSION).dylib /usr/local/lib/libtk.dylib



devsymlinks_clean:
	-sudo rm -f /usr/local/include/tcl*.h /usr/local/include/tk*.h
	-sudo rm -f /usr/local/lib/tclConfig.sh /usr/local/lib/tkConfig.sh
	-sudo rm -f /usr/local/lib/libtcl*.dylib /usr/local/lib/libtk*.dylib
	-sudo rm -f /usr/local/lib/libtclstub*.a /usr/local/lib/libtkstub*.a



patch_pd:
	@echo pd_src $(pd_src)
# apply all platform-neutral patches
	for patch in $(wildcard $(CWD)/patches/*.patch); do \
		echo "Applying $$patch"; \
		cd $(pd_src)/src/ && patch -p0 < $$patch; \
	done
# apply all platform-specific patches
	for patch in $(wildcard $(CWD)/patches/$(OS_NAME)/*.patch); do \
		echo "Applying $$patch"; \
		cd $(pd_src)/src/ && patch -p0 < $$patch; \
	done
	-rm -f -- $(pd_src)/src/configure  $(pd_src)/src/makefile
# change Pd's version number to reflect the extended build
	cd $(pd_src)/src/ && \
		sed 's/\(pd_version\[\] = "Pd version \)[0-9extndRC.-]*/\1$(PD_VERSION)-$(PACKAGE_VERSION)/'  s_main.c > s_main.c.tmp && \
		mv s_main.c.tmp s_main.c
	for file in $(wildcard $(pd_src)/src/*.[ch]); do \
		sed 's/MACOSX/__APPLE__/g' $${file} > $${file}.tmp && \
			mv -f $${file}.tmp $$file;\
	done
	@echo " "
	@echo "patching succeeded!"


unpatch_pd:
	for file in $(wildcard $(pd_src)/src/*.[ch]); do \
		sed 's/__APPLE__/MACOSX/g' $${file} > $${file}.tmp && \
			mv -f $${file}.tmp $$file;\
	done
# this sed pattern won't work with TEST versions
	cd $(pd_src)/src && \
		sed 's/\(pd_version\[\] = "Pd version [0-9]\.[0-9]*[.-][0-9]*\)[0-9extndRC.-]*/\1/' \
			s_main.c > s_main.c.tmp && \
		mv s_main.c.tmp s_main.c
# apply all platform-specific patches
	for patch in $(wildcard $(CWD)/patches/$(OS_NAME)/*.patch); do \
		echo "Applying $$patch"; \
		cd $(pd_src)/src/ && patch -p0 -R < $$patch; \
	done
# apply all platform-neutral patches
	for patch in $(wildcard $(CWD)/patches/*.patch); do \
		echo "Applying $$patch"; \
		cd $(pd_src)/src/ && patch -p0 -R < $$patch; \
	done
	-rm -f -- $(pd_src)/src/configure  $(pd_src)/src/makefile
	@echo " "
	@echo "unpatching succeeded!"


#==============================================================================#
#
# CLEAN TARGETS
#
#==============================================================================#
abstractions_clean:
	-cd $(abstractions_src) && make $(DEST_PATHS) clean


doc_clean:


externals_clean:
	-cd $(externals_src) && make $(DEST_PATHS) clean


flext_clean:
	-cd $(externals_src)/footils/syncgrain && make -f makefile.pd-darwin clean
	-cd $(externals_src)/grill && \
		rm -- flext/buildsys/config-mac-pd-gcc.txt config.txt flext/config.txt \
			flext/source/libflext.a */pd-darwin/*.o */pd-darwin/*/*.o \
		 	*/pd-darwin/*/*.pd_darwin */pd-darwin/*.pd_darwin \
			*/pd-darwin/release-shared/*.dylib */pd-darwin/release-shared/*.opp


gem_clean:


pd_clean:
	-cd $(pd_src)/src && make $(DEST_PATHS) clean


# these targets are all from Makefile.buildlayout: install_clean cruft_clean
clean: abstractions_clean doc_clean externals_clean flext_clean gem_clean \
pd_clean 
	echo "Complete clean finished."


distclean: clean cruft_clean
	cd $(abstractions_src) && make distclean
	cd $(doc_src) && make distclean
	cd $(extensions_src) && make distclean
	cd $(externals_src) && make distclean
	-cd $(pd_src) && make distclean

test_locations:
	@echo "PD_VERSION: $(PD_VERSION)"
	@echo "PACKAGE_VERSION: $(PACKAGE_VERSION)"
	@echo "CWD $(CWD)"
	@echo "DESTDIR $(DESTDIR)"
	@echo "PREFIX $(prefix)"
	@echo "BINDIR  $(bindir)"
	@echo "LIBDIR  $(libdir)"
	@echo "OBJECTSDIR  $(objectsdir)"
	@echo "PDDOCDIR  $(pddocdir)"
	@echo "LIBPDDIR  $(libpddir)"
	@echo "LIBPDBINDIR  $(libpdbindir)"
	@echo "HELPDIR  $(helpdir)"
	@echo "MANUALSDIR  $(manualsdir)"
	@echo "EXAMPLESDIR  $(examplesdir)"

