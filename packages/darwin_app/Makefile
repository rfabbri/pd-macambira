
current: darwin_app

# turn on weak linking and dlopen support
export MACOSX_DEPLOYMENT_TARGET = 10.3

# needed to support weak linking of frameworks
#MAKE = MACOSX_DEPLOYMENT_TARGET=10.3 make

CWD := $(shell pwd)
BUILD_BASE = $(CWD)/build
PD_APP_NAME = $(PACKAGE_NAME)
PD_APP_CONTENTS = $(BUILD_BASE)/$(PD_APP_NAME).app/Contents
DESTDIR = $(PD_APP_CONTENTS)/Resources

cvs_root_dir = $(CWD)/../..
BUILDLAYOUT_DIR = $(CWD)/..

# source for the Wish Shell.app used to build Pd
# download TclTkAquaStandalone-8.?.*.dmg from http://tcltkaqua.sourceforge.net/
# and mount it.  The files will be copied from the mounted .dmg image.
#WISH = /Applications/Utilities/Wish\ Shell.app
TCLTK := $(shell /bin/ls -1r TclTkAquaStandalone-8.?.*.dmg|head -1|sed 's/\.dmg//')
WISH_NAME = Wish\ Shell
WISH = /Volumes/$(TCLTK)/$(WISH_NAME).app
WISH_CONTENTS = $(WISH)/Contents


include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

# base level optimizations
OPT_CFLAGS = -O3 -funroll-loops -fomit-frame-pointer

# CPU-specific gleaned from http://developer.apple.com/releasenotes/DeveloperTools/GCC3.html
# check this too: http://hpc.sourceforge.net/
# Generic PowerPC
#OPT_CFLAGS += -mcpu=powerpc -mpowerpc
# PowerPC 750 (G3)
#OPT_CFLAGS += -mcpu=750 -mpowerpc-gfxopt
# PowerPC 7400 (G4 <= 700Mhz) 
#OPT_CFLAGS += -mcpu=7400 -faltivec -mpowerpc-gfxopt
# PowerPC 7450 (G4 >= 533MHz) 
#OPT_CFLAGS += -fast -fPIC -mcpu=7450 -mtune=7450 -faltivec -ffast-math -mpowerpc-gfxopt
# PowerPC G5 (gcc-3.3)
#OPT_CFLAGS += -fast -fPIC -faltivec -mpowerpc-gpopt -mpowerpc-gfxopt

# Package default, support older PowerPCs, but G4 (7450) is the most common
OPT_CFLAGS += -mcpu=powerpc -mtune=7450 -mpowerpc-gfxopt


# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				cvs_root_dir=$(cvs_root_dir) \
				prefix=$(prefix) \
				DESTDIR=$(DESTDIR) \
				OPT_CFLAGS="$(OPT_CFLAGS)" \
				UNAME=$(UNAME)

all:
	cd $(packages_src) && $(MAKE) $(DEST_PATHS)

install: darwin_app

#------------------------------------------------------------------------------#
darwin_app: darwin_app_core extended_install darwin_app_doc_format rez_install \
darwin_app_embed_libs
	@echo " "
	@echo "Pd.app build succeeded!"

extended_install:
	cd $(packages_src) && $(MAKE) $(DEST_PATHS) install

#------------------------------------------------------------------------------#
# this target is for when you already have a pre-built Pd.app and you
# just want to fill it with goodies from CVS.  To do so, place your 
# Pd.app in $(BUILD_BASE)/Pd.app
extended_app_install:
	cd $(packages_src) && $(MAKE) $(DEST_PATHS) abstractions_install
	cd $(packages_src) && $(MAKE) $(DEST_PATHS) doc_install
	cd $(packages_src) && $(MAKE) $(DEST_PATHS) extensions_install
	cd $(packages_src) && $(MAKE) $(DEST_PATHS) externals_install


#------------------------------------------------------------------------------#
# check here for a reference on how to do this:
# http://cvs.sourceforge.net/viewcvs.py/tkcvs/tkcvs-proj/PackApp?rev=1.4
darwin_app_wrapper: Info.plist
	test -d /Volumes/$(TCLTK) || hdiutil mount $(TCLTK).dmg
# copy Wish Shell.app from default install location
# note: use the "standalone" Wish shell to make a "standalone" pd app
	install -d "$(PD_APP_CONTENTS)/MacOS"
	install -m0755 -p \
		$(WISH_CONTENTS)/MacOS/$(WISH_NAME) \
		"$(PD_APP_CONTENTS)/MacOS"
	install -d "$(PD_APP_CONTENTS)/Resources"
	install -m0644 -p \
		$(WISH_CONTENTS)/Resources/$(WISH_NAME).rsrc \
		"$(PD_APP_CONTENTS)/Resources"
	install -d "$(PD_APP_CONTENTS)/Frameworks"
	cp -Rp $(WISH_CONTENTS)/Frameworks "$(PD_APP_CONTENTS)"
# set up app wrapper
	install -d "$(PD_APP_CONTENTS)/Resources/Scripts"
	install -m0644 -p Info.plist "$(PD_APP_CONTENTS)"
	install -m0644 -p *.icns  "$(PD_APP_CONTENTS)/Resources"
	mv "$(PD_APP_CONTENTS)/MacOS/Wish Shell" \
		"$(PD_APP_CONTENTS)/MacOS/Pd"
#	diskutil eject /Volumes/$(TCLTK)


#------------------------------------------------------------------------------#
# generate Info.plist using PD_APP_NAME and PD_VERSION
Info.plist: Info.plist.in 
	cat Info.plist.in | \
		sed 's/PD_APP_NAME/$(PD_APP_NAME)/' | \
		sed 's/PD_VERSION/$(PD_VERSION)-$(PACKAGE_VERSION)/' > \
		Info.plist

# tigital's Gem window focus black magic
rez_install:
	install -p $(CWD)/mac.r $(bindir)
	cd $(bindir) && /Developer/tools/Rez -t APPL mac.r -o pd


darwin_app_embed_libs:
	./embed-MacOSX-dependencies.sh $(PD_APP_CONTENTS)


#------------------------------------------------------------------------------#
darwin_app_core: darwin_app_wrapper
	cd $(packages_src) && \
		$(MAKE) $(DEST_PATHS) PD_CONFIGURE_FLAGS=--enable-jack pd_install 
	cd "$(PD_APP_CONTENTS)/Resources/Scripts" && \
		ln -s ../bin/pd.tk AppMain.tcl
# support for Info Panel Plugins mgmt
	cd "$(PD_APP_CONTENTS)" && ln -s Resources/extra Plugins
# run script to move help-*.pd files to *-help.pd according to the standard
	cd $(pddocdir) && \
		$(scripts_src)/convert-help-to-standard.sh



darwin_app_doc_format:
# set the font to 10pt on all help patches
# the BSD/Darwin version of sed must create a backup file when doing 
# in-place replacement, so delete the unneeded backup files
	cd $(pddocdir) && \
		sed -i.bak 's/^\(\#N canvas [0-9]* [0-9]* [0-9]* [0-9]*\) 12/\1 10/' \
			*/*.pd */*/*.pd */*/*/*.pd
	find $(DESTDIR) -name '*.pd.bak' -delete

#==============================================================================#
#
## CVS SOURCES
#
#==============================================================================#


#------------------------------------------------------------------------------#
darwin_app_externals: darwin_app_externals_standard darwin_app_externals_c++ darwin_app_externals_flext
#darwin_app_externals: darwin_app_externals_standard darwin_app_externals_c++




darwin_app_externals_c++:
#----------------------------------------------------------------------------
# CREB
#	cd $(externals_src)/creb && autoconf && ./configure && cd modules++ && $(MAKE)
#	install -p -m0644 $(externals_src)/creb/modules++/*.pd_darwin $(objectsdir)
#----------------------------------------------------------------------------
# gem2pdp
#	cd $(externals_src)/gem2pdp && $(MAKE) -f Makefile.darwin
#	install -p -m0444 $(externals_src)/gem2pdp/*.pd_darwin $(objectsdir)


darwin_app_perms:
	chmod -R a-w $(pddocdir)

package: dmg

DMG_NAME = $(PACKAGE_NAME)
dmg: darwin_app_perms
	install -d "$(CWD)/$(DMG_NAME)"
	install -p -m0444 $(manualsdir)/Pd/Pd-LICENSE.txt \
		$(manualsdir)/Pd/License.html \
		$(manualsdir)/Pd/Welcome.html \
		$(manualsdir)/Pd/ReadMe.html \
		"$(CWD)/$(DMG_NAME)"
	cd $(BUILD_BASE) && mv $(PD_APP_NAME).app "$(CWD)/$(DMG_NAME)/"
	install -p -m0444 org.puredata.pd.plist "$(CWD)/$(DMG_NAME)"
	cd $(CWD) && /bin/sh mkdmg "$(DMG_NAME)"



distclean: clean
	cd $(packages_src) && $(MAKE) $(DEST_PATHS) distclean


clean: darwin_app_clean
	cd $(packages_src) && $(MAKE) $(DEST_PATHS) clean
	-rm Info.plist


darwin_app_clean:
	-chmod -R u+w $(PD_APP_CONTENTS)/Frameworks
	-rm -rf -- $(PD_APP_CONTENTS)/Frameworks
	-rm -f -- $(PD_APP_CONTENTS)/Plugins $(PD_APP_CONTENTS)/Info.plist
	-rm -f -- $(PD_APP_CONTENTS)/MacOS/Pd
	-rmdir  $(PD_APP_CONTENTS)/MacOS
	-rm -f -- $(PD_APP_CONTENTS)/Resources/Scripts/AppMain.tcl
	-rmdir $(PD_APP_CONTENTS)/Resources/Scripts
	-rm -f -- $(WISH_CONTENTS)/Resources/$(WISH_NAME).rsrc \
		$(WISH_CONTENTS)/Resources/pd*.icns
	-rmdir $(PD_APP_CONTENTS)/Resources
	-rm -f -- $(DESTDIR)
	-rm -f -- *~ 1 $(README_FILE) $(WELCOME_FILE) $(LICENSE_FILE) \
		$(UNIX_PACKAGE_NAME).pkg  "$(DMG_NAME).dmg"








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



#==============================================================================#
#
## DEPRECATED TARGETS
#
#==============================================================================#

darwin_app_externals_flext_clean:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(packages_src) && $(MAKE) flext_clean"


darwin_app_externals_flext:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(packages_src) && $(MAKE) flext_install"
	cd $(packages_src) && $(MAKE) $(DEST_PATHS) flext_install


darwin_app_noncvs:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(packages_src) && $(MAKE) noncvs_install"
	cd $(packages_src) && $(MAKE) $(DEST_PATHS) noncvs_install

darwin_app_externals_gem_clean:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(packages_src) && $(MAKE) gem_clean"

darwin_app_externals_gem:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(packages_src) && $(MAKE) gem_install"

darwin_app_externals_standard_clean:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(packages_src) && $(MAKE) externals_clean"

darwin_pd_clean:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(packages_src) && $(MAKE) pd_clean"

darwin_app_externals:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(externals_src) && $(MAKE) install"

darwin_app_externals_standard:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(externals_src) && $(MAKE) install"

darwin_patch_pd:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd .. && $(MAKE) patch_pd"

darwin_unpatch_pd:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd .. && $(MAKE) unpatch_pd"

darwin_app_docs:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(doc_src) && $(MAKE) install"

darwin_app_license:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd .. && $(MAKE) license"

darwin_app_welcome:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd .. && $(MAKE) welcome"

darwin_app_readme:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd .. && $(MAKE) readme"

darwin_prebuilt_app:
	@echo " "
	@echo "Deprecated!!" 
	@echo "Use make extended_app_install"

