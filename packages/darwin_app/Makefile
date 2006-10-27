current: darwin_app

# turn on weak linking and dlopen support
export MACOSX_DEPLOYMENT_TARGET = 10.3

CWD := $(shell pwd)
BUILD_BASE = $(CWD)/build
PD_APP_NAME = $(PACKAGE_NAME)
PD_APP_CONTENTS = $(BUILD_BASE)/$(PD_APP_NAME).app/Contents
DESTDIR = $(PD_APP_CONTENTS)/Resources

cvs_root_dir := $(shell cd $(CWD)/../.. && pwd)
BUILDLAYOUT_DIR = $(cvs_root_dir)/packages

include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

# source for the Wish Shell.app used to build Pd
# download TclTkAquaStandalone-8.?.*.dmg from http://tcltkaqua.sourceforge.net/
# and mount it.  The files will be copied from the mounted .dmg image.
#WISH = /Applications/Utilities/Wish\ Shell.app
TCLTK := $(shell find $(packages_src)/darwin_app -name 'TclTkAquaStandalone-8.?.*.dmg'|head -n 1|sed 's|.*/\(.*\)\.dmg|\1|')
TCLTK_BUGFIX_VERSION := $(shell echo $(TCLTK) | \
	sed 's|.*TclTkAquaStandalone-[0-9]\.[0-9]\.\([0-9]*\)|\1|')

WISH_SOURCE = $(shell $(scripts_src)/find-wish.app.sh "$(TCLTK)")
# Wish Shell.app changed to Wish.app in TclTk 8.4.10
WISH_NAME := $(shell ( test -d $(WISH_SOURCE)/Wish.app && echo Wish) \
	|| echo Wish Shell)
WISH = "$(WISH_SOURCE)/$(WISH_NAME).app"
WISH_CONTENTS = "$(WISH)/Contents"
#WISH_CONTENTS = $(WISH)/Contents

# base level optimizations
OPT_CFLAGS = -Os -funroll-loops -fomit-frame-pointer

# which CPU to compile for
TARGET_PLATFORM := $(shell uname -p)

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

# auto-vectorization would be nice:
# http://gcc.gnu.org/projects/tree-ssa/vectorization.html

ifeq ($(TARGET_PLATFORM),powerpc)
  # Package default, support older PowerPCs, but G4 (7450) is the most common
  OPT_CFLAGS += -mcpu=powerpc -mtune=7450 -mpowerpc-gfxopt
endif


# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				cvs_root_dir=$(cvs_root_dir) \
				prefix=$(prefix) \
				DESTDIR=$(DESTDIR) \
				OPT_CFLAGS="$(OPT_CFLAGS)" \
				UNAME=$(UNAME)

all:
	$(MAKE) -C $(packages_src) $(DEST_PATHS)

install: darwin_app
devel_install: darwin_app_devel

#------------------------------------------------------------------------------#
darwin_app: darwin_app_core extended_install darwin_app_doc_format rez_install \
darwin_app_embed_libs
	@echo " "
	@echo "Pd.app build succeeded!"

extended_install:
	$(MAKE) -C $(packages_src) $(DEST_PATHS) extended_install

#------------------------------------------------------------------------------#
darwin_app_devel: darwin_app_devel_core rez_install
	@echo " "
	@echo "Pd_devel.app build succeeded!"

#------------------------------------------------------------------------------#
# this target is for when you already have a pre-built Pd.app and you
# just want to fill it with goodies from CVS.  To do so, place your 
# Pd.app in $(BUILD_BASE)/Pd.app
extended_app_install:
	$(MAKE) -C $(packages_src) $(DEST_PATHS) abstractions_install
	$(MAKE) -C $(packages_src) $(DEST_PATHS) doc_install
	$(MAKE) -C $(packages_src) $(DEST_PATHS) extensions_install
	$(MAKE) -C $(packages_src) $(DEST_PATHS) externals_install


#------------------------------------------------------------------------------#
# check here for a reference on how to do this:
# http://cvs.sourceforge.net/viewcvs.py/tkcvs/tkcvs-proj/PackApp?rev=1.4
darwin_app_wrapper: Info.plist
# copy Wish Shell.app from default install location
# note: use the "standalone" Wish shell to make a "standalone" pd app
	install -d "$(PD_APP_CONTENTS)/MacOS"
	install -m0755 -p \
		"$(WISH_CONTENTS)/MacOS/$(WISH_NAME)" \
		"$(PD_APP_CONTENTS)/MacOS"
	install -d "$(PD_APP_CONTENTS)/Resources"
	install -m0644 -p \
		"$(WISH_CONTENTS)/Resources/$(WISH_NAME).rsrc" \
		"$(PD_APP_CONTENTS)/Resources"
# only install Frameworks if using a standalone Wish.app
	(test -d "$(WISH_CONTENTS)/Frameworks" && \
		(install -d "$(PD_APP_CONTENTS)/Frameworks"; \
		cp -Rp "$(WISH_CONTENTS)/Frameworks" "$(PD_APP_CONTENTS)" ) ) || \
		echo "Skipping Frameworks copy."
# set up app wrapper
	install -d "$(PD_APP_CONTENTS)/Resources/Scripts"
	install -m0644 -p Info.plist "$(PD_APP_CONTENTS)"
	install -m0644 -p *.icns  "$(PD_APP_CONTENTS)/Resources"
	mv "$(PD_APP_CONTENTS)/MacOS/${WISH_NAME}" \
		"$(PD_APP_CONTENTS)/MacOS/${PD_APP_NAME}"

#------------------------------------------------------------------------------#
# 
darwin_app_devel_wrapper: Info.plist
#	test -d /Volumes/$(TCLTK) || hdiutil mount $(TCLTK).dmg
# copy Wish Shell.app from default install location
# note: use the "standalone" Wish shell to make a "standalone" pd app
	install -d "$(PD_APP_CONTENTS)/MacOS"
	install -m0755 -p \
		"$(WISH_CONTENTS)/MacOS/$(WISH_NAME)" \
		"$(PD_APP_CONTENTS)/MacOS"
	install -d "$(PD_APP_CONTENTS)/Resources"
	install -m0644 -p \
		"$(WISH_CONTENTS)/Resources/$(WISH_NAME).rsrc" \
		"$(PD_APP_CONTENTS)/Resources"
	install -d "$(PD_APP_CONTENTS)/Frameworks"
	cp -Rp "$(WISH_CONTENTS)/Frameworks" "$(PD_APP_CONTENTS)"
# set up app wrapper
	install -d "$(PD_APP_CONTENTS)/Resources/Scripts"
	install -d "$(PD_APP_CONTENTS)/Resources/bin"
	install -m0644 -p Info.plist "$(PD_APP_CONTENTS)"
	install -m0644 -p *.icns  "$(PD_APP_CONTENTS)/Resources"
	mv "$(PD_APP_CONTENTS)/MacOS/${WISH_NAME}" \
		"$(PD_APP_CONTENTS)/MacOS/${PD_APP_NAME}"
#	diskutil eject /Volumes/$(TCLTK)

#------------------------------------------------------------------------------#
# generate Info.plist using PD_APP_NAME and PD_VERSION
Info.plist: Info.plist.in 
	cat Info.plist.in | \
		sed 's/PD_APP_NAME/$(PD_APP_NAME)/' | \
		sed 's/PD_VERSION/$(PD_VERSION)/' > \
		Info.plist

# tigital's Gem window focus black magic
rez_install:
	install -pv "$(CWD)/mac.r" "$(bindir)"
	cd $(bindir) && /Developer/tools/Rez -t APPL mac.r -o pd


darwin_app_embed_libs:
	./embed-MacOSX-dependencies.sh $(PD_APP_CONTENTS)

# shortcut to install the readme on non-Extended builds
readme_install:
	make -C $(packages_src) $(DEST_PATHS) readme_install


#------------------------------------------------------------------------------#
darwin_app_core: darwin_app_wrapper
	$(MAKE) -C $(packages_src) $(DEST_PATHS) PD_CONFIGURE_FLAGS=--enable-jack \
		pd_install 
	cd "$(PD_APP_CONTENTS)/Resources/Scripts" && \
		ln -s ../bin/pd.tk AppMain.tcl
# support for Info Panel Plugins mgmt
	cd "$(PD_APP_CONTENTS)" && ln -s Resources/extra Plugins
# run script to move help-*.pd files to *-help.pd according to the standard
	cd $(pddocdir) && \
		$(scripts_src)/convert-help-to-standard.sh

#------------------------------------------------------------------------------#
darwin_app_devel_core: darwin_app_devel_wrapper
	cd $(packages_src) && \
		$(MAKE) $(DEST_PATHS) PD_CONFIGURE_FLAGS=--enable-jack devel_install 
	cd "$(PD_APP_CONTENTS)/Resources/Scripts" && \
		ln -s ../bin/pd.tk AppMain.tcl
# support for Info Panel Plugins mgmt
	cd "$(PD_APP_CONTENTS)" && ln -s Resources/extra Plugins
# run script to move help-*.pd files to *-help.pd according to the standard
	cd $(pddocdir) && \
		$(scripts_src)/convert-help-to-standard.sh

# set the font to 10pt on all help patches
# the BSD/Darwin version of sed must create a backup file when doing 
# in-place replacement, so delete the unneeded backup files.
darwin_app_doc_format:
	find $(pddocdir) -name '*.pd' -exec \
		sed -i.b 's/^\(\#N canvas [0-9]* [0-9]* [0-9]* [0-9]*\) 12/\1 10/' '{}' \;
	find $(DESTDIR) -name '*.pd.b' -delete


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

#==============================================================================#
#
## PACKAGING
#
#==============================================================================#

package: dmg

dmg: 
	hdiutil create -format UDRW -fs HFS+ -srcfolder "$(BUILD_BASE)" \
		-volname $(PACKAGE_NAME) build.dmg
# detach one with the same name first
	-hdiutil detach "/Volumes/$(PACKAGE_NAME)"
	hdiutil attach build.dmg
	install -p $(manualsdir)/Pd/ReadMe.html "/Volumes/$(PACKAGE_NAME)"
	install -p -m0644 org.puredata.pd.plist "/Volumes/$(PACKAGE_NAME)"
	install -d "/Volumes/$(PACKAGE_NAME)/.imagefolder"
# background image will only show when the disk image is called "Pd-extended
# the background image needs to be added to the .DS_Store to work
#	install -p background.png "/Volumes/$(PACKAGE_NAME)/.imagefolder/"
	install -p DS_Store "/Volumes/$(PACKAGE_NAME)/.DS_Store"
	chmod -R a-w "/Volumes/$(PACKAGE_NAME)/$(PD_APP_NAME).app/Contents/Resources/doc"
	hdiutil detach `mount | grep $(PACKAGE_NAME) | cut -d ' ' -f 1`
	hdiutil convert -format UDZO -o "$(PACKAGE_NAME).dmg" build.dmg
	rm -f build.dmg
# install license
	hdiutil unflatten "$(PACKAGE_NAME).dmg"
	/Developer/Tools/Rez /Developer/Headers/FlatCarbon/*.r SLA.r -a \
		-o "$(PACKAGE_NAME).dmg"
	hdiutil flatten "$(PACKAGE_NAME).dmg"



#==============================================================================#
#
## CLEAN TARGETS
#
#==============================================================================#


distclean: clean
	$(MAKE) -C $(packages_src) $(DEST_PATHS) distclean


clean: package_clean
	$(MAKE) -C $(packages_src) $(DEST_PATHS) clean


package_clean: darwin_app_clean
	-chmod -R u+w -- $(BUILD_BASE)
	-rm -rf -- $(BUILD_BASE)
	-rm Info.plist
	-rm -f -- "$(CWD)/$(PACKAGE_NAME)/ReadMe.html"
	-rm -f -- "$(CWD)/$(PACKAGE_NAME)/org.puredata.pd.plist"

devel_clean: darwin_app_devel_clean
	$(MAKE) -C $(packages_src) $(DEST_PATHS) clean
	-rm Info.plist

darwin_app_clean:
	-chmod -R u+w $(PD_APP_CONTENTS)/Frameworks
	-rm -rf -- $(PD_APP_CONTENTS)/Frameworks
	-rm -f -- $(PD_APP_CONTENTS)/Plugins $(PD_APP_CONTENTS)/Info.plist
	-rm -f -- "$(PD_APP_CONTENTS)/MacOS/$(PD_APP_NAME)"
	-rmdir  $(PD_APP_CONTENTS)/MacOS
	-rm -f -- $(PD_APP_CONTENTS)/Resources/Scripts/AppMain.tcl
	-rmdir $(PD_APP_CONTENTS)/Resources/Scripts
	-rm -f -- "$(PD_APP_CONTENTS)/Resources/$(WISH_NAME).rsrc" \
		"$(PD_APP_CONTENTS)/Resources/pd*.icns"
	-rmdir  $(PD_APP_CONTENTS)/Resources
	-rmdir  $(BUILD_BASE)/$(PD_APP_NAME).app/Contents
	-rmdir  $(BUILD_BASE)/$(PD_APP_NAME).app
	-rmdir  $(BUILD_BASE)
	-rm -f -- "$(PACKAGE_NAME).dmg"
	-rm -f -- build.dmg


#==============================================================================#
#
## TEST TARGETS
#
#==============================================================================#

test_package:
	echo "Bypassing test..."
#	echo "Mounting $(PACKAGE_NAME)"
# this triggers the license prompt... hmm how to work around that...
#	cd $(CWD) && hdiutil mount "$(PACKAGE_NAME).dmg"
# some better tests should go in here
#	echo "Ejecting $(PACKAGE_NAME)"
#	cd $(CWD) && hdiutil eject `mount | grep "$(PACKAGE_NAME)" | cut -d ' ' -f 1`



test_locations:
	@echo "PD_VERSION: $(PD_VERSION)"
	@echo "PACKAGE_NAME: $(PACKAGE_NAME)"
	@echo "PD-EXTENDED_VERSION: $(PD-EXTENDED_VERSION)"
	@echo "WISH: $(WISH)"
	@echo "TCLTK: $(TCLTK)"
	@echo "TCLTK_BUGFIX_VERSION: $(TCLTK_BUGFIX_VERSION)"
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

