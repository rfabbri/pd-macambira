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
prefix = $(cvs_root_dir)/packages/build
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



# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				cvs_root_dir=$(cvs_root_dir) \
				prefix=$(prefix) \
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
	cd $(externals_src)/nusmuk/msd && $(externals_src)/grill/flext/build.sh \
		pd gcc
	cd $(externals_src)/nusmuk/msd2D && $(externals_src)/grill/flext/build.sh \
		pd gcc
	cd $(externals_src)/nusmuk/msd3D && $(externals_src)/grill/flext/build.sh \
		pd gcc

#------------------------------------------------------------------------------
# Gem
gem:
#	cd $(gem_src)/Gem && xcodebuild


#------------------------------------------------------------------------------
# pd
# pd's makefile has some of its own variables that need to be remapped
# this relies on patches/build_vars_4_makefile.in.patch  <hans@at.or.at>
DESTDIR = ""
ifeq ($(OS_NAME),darwin)
  libpddir = $(prefix)
  libpdbindir = $(bindir)
else
  ifeq ($(OS_NAME),win)
    libpddir = $(prefix)
    libpdbindir = $(bindir)
  else
    libpddir = $(prefix)/lib/pd
    libpdbindir = $(libpddir)/bin
  endif
endif

PD_DEST_PATHS = $(DEST_PATHS) DESTDIR=$(DESTDIR) \
	 libpddir=$(libpddir) \
	 libpdbindir=$(libpdbindir) 

PD_CONFIGURE_FLAGS = 
PD_EXTRA_FLAGS = 

# Pd sometimes needs a specific autoconf, which is labeled differently on different
# machines
#PD_AUTOCONF := ${shell ( test -x "`which autoconf-2.59`" && echo autoconf-2.59 ) || echo autoconf }
PD_AUTOCONF = autoconf


pd:
	echo "OS_NAME $(OS_NAME)"
	echo "DESTDIR $(DESTDIR)  libpdbindir $(libpdbindir)"
	echo "DEST_PATHS $(DEST_PATHS)"
	echo "PD_DEST_PATHS $(PD_DEST_PATHS)"
	echo "PD_EXTRA_FLAGS $(PD_EXTRA_FLAGS)"
	cd $(pd_src)/src/ && $(PD_AUTOCONF)
	cd $(pd_src)/src && ./configure $(PD_CONFIGURE_FLAGS) && \
		make $(PD_DEST_PATHS) $(PD_EXTRA_FLAGS)


#==============================================================================#
#
# INSTALL TARGETS
#
#==============================================================================#

#------------------------------------------------------------------------------
# install
install: license_install readme_install welcome_install gem_install
install: pd_install abstractions_install doc_install externals_install
install: noncvs_install
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
# externals_install
externals_install: 
	cd $(externals_src) && make $(DEST_PATHS) install


#------------------------------------------------------------------------------
# flext_install
flext_install:
# nusmuk
	cd $(externals_src)/nusmuk/msd && $(externals_src)/grill/flext/build.sh \
		pd gcc install
	cd $(externals_src)/nusmuk/msd2D && $(externals_src)/grill/flext/build.sh \
		pd gcc install
	cd $(externals_src)/nusmuk/msd3D && $(externals_src)/grill/flext/build.sh \
		pd gcc install
#TODO: need to add nusmuk/editor


#------------------------------------------------------------------------------
# gem_install
GEM_NAME = gem
gem_install: gem $(helpdir)
	install -p $(gem_src)/help/*.* $(helpdir)
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



#------------------------------------------------------------------------------
# pd_install
PD_NAME=Pd
pd_install: pd
	cd $(pd_src)/src && \
		make $(PD_DEST_PATHS) $(PD_EXTRA_FLAGS) install
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
	touch $(LICENSE_FILE)
	echo "<html><body>" >> "$(LICENSE_FILE)"
	echo "<h3>(Parts of this package can be used under " >> "$(LICENSE_FILE)"
	echo "<a href="PD%20LICENSE.txt">Pd&quot;s BSD license</a>)</h3>" >> "$(LICENSE_FILE)"
	echo "<font size=\"-1\">" >> "$(LICENSE_FILE)"
	cat "$(externals_src)/creb/COPYING" | sed -e 's/^$$/\<P\>/g' >> "$(LICENSE_FILE)"
	echo "</font></body></html>" >> $(LICENSE_FILE)
# Pd's license file
	install -p "$(pd_src)/LICENSE.txt" "$(manualsdir)/$(PD_NAME)/PD LICENSE.txt"



WELCOME_FILE = $(manualsdir)/$(PD_NAME)/Welcome.html
welcome_install:
	install -d $(manualsdir)/$(PD_NAME)
	touch $(WELCOME_FILE)
	echo "<html><body><p><p>" >> $(WELCOME_FILE)
	echo "<CENTER><IMG SRC=\"logo.jpg\">" >> $(WELCOME_FILE)
	echo "<h2>Version $(PD_VERSION)</h2>" >> $(WELCOME_FILE)
	echo "<p>written by Miller S. Puckette</p></CENTER>" >> $(WELCOME_FILE)
	echo "<FONT SIZE=\"-1\">" >> $(WELCOME_FILE)
	echo "<p>`grep -A9 ACKNOWLEDG $(pd_src)/README.txt`</p>" >> $(WELCOME_FILE)
	echo "</FONT>" >> $(WELCOME_FILE)
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
	echo "</head>" >> $(README_FILE)
	echo "<body>" >> $(README_FILE)
	echo "<h2>Pure Data $(PD_VERSION)-$(PACKAGE_VERSION)</h2>" 	>> $(README_FILE)
	echo "<p>Pd is a free real-time computer music software package resembling Max.  It provides a patchable environment for audio analysis, synthesis, and processing, with a rich set of multimedia capabilities.  You can get Pd for Linux, Windows, MacOS X, BSD, or IRIX.</p>" >> $(README_FILE)
	echo "<p>For more information, go to: <a href=\"http://puredata.org\" target=\"pd\">http://puredata.org</a></p>" >> $(README_FILE)
	echo "<h3>Installation</h3>" >> $(README_FILE)	
	echo "<p>To install Pd, drag the Pd.app to anywhere in your hard disk.</p>" >> $(README_FILE)	
	echo "<p>To install Gem, pmpd, xsample, dyn~, and vasp support, copy <code>org.puredata.pd.plist</code> to <code>~/Library/Preferences</code> (<code>~</code> means your home folder).  WARNING: this will overwrite any existing Pd preferences!</p>" >> $(README_FILE)	
	echo "<h3>Pure Data CVS Developers</h3>" >> $(README_FILE)	
# this may seem whack, but it generates the list of developers from the SourceForge site:
	curl 'http://sourceforge.net/project/memberlist.php?group_id=55736' | grep -A2 -e '<td>' | sed 's|\(href="\)|target="w" \1http://sourceforge.net|' >> $(README_FILE)
	echo "</p>" >> $(README_FILE)
	echo "<p>" >> $(README_FILE)
	echo "Many others not listed have contributed their time and effort, this is just a list of the current developers in the SourceForge project.  But really, every Pd user is a developer and is encouraged to contribute to the CVS repository." >> $(README_FILE)
	echo "</p>" >> $(README_FILE)
	echo "<h3>License</h3>" >> $(README_FILE)	
	echo "<p>" >> $(README_FILE)
	echo "The Pd core is licensed under a <a href="PD%20LICENSE.txt">BSD license</a>, almost every other part of this package is available under the <a href="http://www.gnu.org/copyleft/gpl.html" target="gpl">GNU GPL</a>.  A couple packages have BSD-style licenses too." >> $(README_FILE)
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
	-find $(prefix) -name .DS_Store -delete
	-find $(prefix) -name '*.*.bak' -delete
# run script to move help-*.pd files to *-help.pd according to the standard
	cd $(pddocdir) && \
		$(scripts_src)/convert-help-to-standard.sh
# remove write perms to prevent people form editing the helpfiles by mistake
	cd $(pddocdir) && chmod a-wx */*.pd */*/*.pd
	find $(pddocdir) -name '*.pd' -print0 | xargs -0 chmod a-wx
	find $(pddocdir) -name '*.txt' -print0 | xargs -0 chmod a-wx



#==============================================================================#
#
# DEVELOPER TARGETS
#
#==============================================================================#

devsymlinks:



patch_pd:
	@echo pd_src $(pd_src)
# change Pd's version number to reflect the extended build
	cd $(pd_src)/src/ && \
		sed 's/\(pd_version\[\] = "Pd version \)[0-9extndRC.-]*/\1$(PD_VERSION)-$(PACKAGE_VERSION)/'  s_main.c > s_main.c.tmp && \
		mv s_main.c.tmp s_main.c
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
	@echo " "
	@echo "patching succeeded!"


unpatch_pd:
# this sed pattern won't work with TEST versions
	cd $(pd_src)/src && \
		sed 's/\(pd_version\[\] = "Pd version [0-9]\.[0-9]*[.-][0-9]*\)[0-9extndRC.-]*/\1/' \
			s_main.c > s_main.c.tmp && \
		mv s_main.c.tmp s_main.c
# apply all platform-neutral patches
	for patch in $(wildcard $(CWD)/patches/*.patch); do \
		echo "Applying $$patch"; \
		cd $(pd_src)/src/ && patch -p0 -R < $$patch; \
	done
# apply all platform-specific patches
	for patch in $(wildcard $(CWD)/patches/$(OS_NAME)/*.patch); do \
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


gem_clean:


pd_clean:
	-cd $(pd_src) && make $(PD_DEST_PATHS) clean


# these targets are all from Makefile.buildlayout: install_clean cruft_clean
clean: abstractions_clean doc_clean externals_clean flext_clean 
clean: gem_clean pd_clean 
	echo "Complete clean finished."


distclean: clean cruft_clean
