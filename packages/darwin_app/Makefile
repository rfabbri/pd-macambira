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

WISH_FRAMEWORKS_SOURCE := $(shell (test -e "/Library/Frameworks/Tk.framework/Tk" && echo "/Library/Frameworks") || echo /System/Library/Frameworks)
WISH_SOURCE = $(WISH_FRAMEWORKS_SOURCE)/Tk.framework/Resources
# Wish Shell.app changed to Wish.app in TclTk 8.4.10
WISH_NAME := $(shell ( test -d $(WISH_SOURCE)/Wish.app && echo Wish) \
	|| echo Wish Shell)
WISH = "$(WISH_SOURCE)/$(WISH_NAME).app"
WISH_CONTENTS = "$(WISH)/Contents"

# which CPU to compile for
TARGET_PLATFORM := $(shell uname -p)

# 10.3 uses gcc 3.3, and 10.3 only runs on PowerPC
GCC_VERSION := $(shell gcc -dumpversion)
ifeq ($(GCC_VERSION),3.3)
OPT_CFLAGS += -O3 -funroll-loops -fomit-frame-pointer -mcpu=powerpc -mtune=G3
else
# auto-vectorization in GCC 4.x for CPUs with SIMD
OPT_CFLAGS = -ftree-vectorize -ftree-vectorizer-verbose=2 \
	-fast -fPIC -mcpu=7450 -mtune=7450
endif

# Mac/Intels only run 10.4, so always gcc 4.x
ifeq ($(TARGET_PLATFORM),i386)
OPT_CFLAGS = -march=pentium4 -msse2 -mfpmath=sse \
	-ftree-vectorize -ftree-vectorizer-verbose=2
endif

ifeq ($(TARGET_PLATFORM),powerpc)
# Package default, support older PowerPCs, but G3 is the most common
#OPT_CFLAGS += -mcpu=powerpc -mtune=G3

# CPU-specific gleaned from:
# http://developer.apple.com/releasenotes/DeveloperTools/RN-GCC4/
# http://hpc.sourceforge.net/
# Generic PowerPC
#OPT_CFLAGS += -mcpu=powerpc -mpowerpc
# PowerPC 750 (G3)
#OPT_CFLAGS += -mcpu=750 -mpowerpc-gfxopt
# PowerPC 7400 (G4 <= 700Mhz) 
#OPT_CFLAGS += -fast -fPIC -mcpu=G4 -mtune=G4
# PowerPC 7450 (G4 >= 533MHz) 
#OPT_CFLAGS += -fast -fPIC -mcpu=7450 -mtune=7450
# PowerPC G5 (gcc-4.x)
#OPT_CFLAGS += -fast -fPIC
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
darwin_app: darwin_app_core extended_install rez_install darwin_app_embed_libs
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
# build a standalone Wish.app from the installed Tcl/Tk Frameworks
darwin_app_wrapper: Info.plist
# Wish
	install -d "$(PD_APP_CONTENTS)/MacOS"
	install -m0755 -p \
		"$(WISH_CONTENTS)/MacOS/$(WISH_NAME)" "$(PD_APP_CONTENTS)/MacOS"
	install -d "$(PD_APP_CONTENTS)/Resources"
	install -m0644 -p \
		"$(WISH_CONTENTS)/Resources/$(WISH_NAME).rsrc" \
		"$(PD_APP_CONTENTS)/Resources"
# only embed the Tcl/Tk Frameworks if not building against the built-in ones
ifneq (/System,$(findstring /System,$(WISH_FRAMEWORKS_SOURCE)))
	install -d $(PD_APP_CONTENTS)/Frameworks \
		$(PD_APP_CONTENTS)/Frameworks/Tcl.framework \
		$(PD_APP_CONTENTS)/Frameworks/Tk.framework
	ditto $(WISH_FRAMEWORKS_SOURCE)/Tcl.framework \
		$(PD_APP_CONTENTS)/Frameworks/Tcl.framework/
	ditto $(WISH_FRAMEWORKS_SOURCE)/Tk.framework \
		$(PD_APP_CONTENTS)/Frameworks/Tk.framework/
	rm -rf $(PD_APP_CONTENTS)/Frameworks/Tcl.framework/{,/Versions/8.4}/{Headers,PrivateHeaders,*_debug,lib*.a,*Config.sh} && \
	rm -rf $(PD_APP_CONTENTS)/Frameworks/Tk.framework/{,/Versions/8.4}/{Headers,PrivateHeaders,*_debug,lib*.a,*Config.sh}
	chmod -R u+w $(PD_APP_CONTENTS)/Frameworks
# from http://tktoolkit.cvs.sourceforge.net/tktoolkit/tk/macosx/GNUmakefile
	fix_install_id ( ) { \
	    chmod -R a+w "$$1"; \
	    install_name_tool -id $$(otool -L "$$1" | awk "/$$2\.framework.*[^:]\$$/ {sub(\"^.*/Frameworks\",\"@executable_path/../Frameworks\",\$$1); print \$$1}") "$$1"; \
	    chmod -R a-w "$$1"; \
	} && \
	fix_install_name ( ) { \
	    chmod -R a+w "$$1"; \
	    install_name_tool -change $$(otool -L "$$1" | awk "/$$2\.framework.*[^:]\$$/ {print \$$1; sub(\"^.*/Frameworks\",\"@executable_path/../Frameworks\",\$$1); print \$$1}") "$$1"; \
	    chmod -R a-w "$$1"; \
	} && \
	fix_install_id $(PD_APP_CONTENTS)/Frameworks/Tcl.framework/Tcl Tcl && \
	fix_install_id $(PD_APP_CONTENTS)/Frameworks/Tk.framework/Tk Tk && \
	fix_install_name "$(PD_APP_CONTENTS)/MacOS/$(WISH_NAME)" Tcl && \
	fix_install_name "$(PD_APP_CONTENTS)/MacOS/$(WISH_NAME)" Tk
endif # NOT /System
# set up app wrapper
	install -d "$(PD_APP_CONTENTS)/Resources/Scripts"
	install -m0644 -p Info.plist "$(PD_APP_CONTENTS)"
	install -m0644 -p *.icns  "$(PD_APP_CONTENTS)/Resources"
	mv "$(PD_APP_CONTENTS)/MacOS/${WISH_NAME}" \
		"$(PD_APP_CONTENTS)/MacOS/${PD_APP_NAME}"

#------------------------------------------------------------------------------#
# 
darwin_app_devel_wrapper: Info.plist
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
	cd $(bindir) && /Developer/Tools/Rez -t APPL mac.r -o pd


darwin_app_embed_libs:
# embed libquicktime plugins
	install -d $(PD_APP_CONTENTS)/lib/libquicktime
	install -p /sw/lib/libquicktime/*.so $(PD_APP_CONTENTS)/lib/libquicktime
	./embed-MacOSX-dependencies.sh $(PD_APP_CONTENTS)

# shortcut to install the readme on non-Extended builds
readme_install:
	make -C $(packages_src) $(DEST_PATHS) readme_install


#------------------------------------------------------------------------------#
darwin_app_core: darwin_app_wrapper
	$(MAKE) -C $(packages_src) $(DEST_PATHS) \
		PD_CONFIGURE_FLAGS="--enable-jack --disable-fat" \
		pd_install 
	cd "$(PD_APP_CONTENTS)/Resources/Scripts" && \
		ln -s ../bin/pd.tk AppMain.tcl
# support for Info Panel Plugins mgmt
	cd "$(PD_APP_CONTENTS)" && ln -s Resources/extra Plugins
	install -p -m0666 org.puredata.pd.plist "$(PD_APP_CONTENTS)/"


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

VOLUME_NAME = Pd-extended
dmg: 
	hdiutil create -format UDRW -fs HFS+ -srcfolder "$(BUILD_BASE)" \
		-volname $(VOLUME_NAME) build.dmg
# detach one with the same name first
	-hdiutil detach "/Volumes/$(VOLUME_NAME)"
	hdiutil attach build.dmg
	install -p $(manualsdir)/Pd/ReadMe.html "/Volumes/$(VOLUME_NAME)"
	install -d "/Volumes/$(VOLUME_NAME)/.imagefolder"
# add link to /Applications for easy install
	ln -s /Applications /Volumes/$(VOLUME_NAME)/Applications
# background image will only show when the disk image is called "$(VOLUME_NAME)
# the background image needs to be added to the .DS_Store to work
	install -p background.png "/Volumes/$(VOLUME_NAME)/.imagefolder/"
	install -p DS_Store "/Volumes/$(VOLUME_NAME)/.DS_Store"
	install -p VolumeIcon.icns "/Volumes/$(VOLUME_NAME)/.VolumeIcon.icns"
	chmod -R a-w "/Volumes/$(VOLUME_NAME)/$(PD_APP_NAME).app/Contents/Resources/doc"
	hdiutil detach `mount | grep $(VOLUME_NAME) | cut -d ' ' -f 1`
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


distclean: package_clean
	$(MAKE) -C $(packages_src) $(DEST_PATHS) distclean


clean: package_clean
	$(MAKE) -C $(packages_src) $(DEST_PATHS) clean


package_clean: darwin_app_clean
	-chmod -R u+w -- $(BUILD_BASE)
	-rm -rf -- $(BUILD_BASE)
	-rm Info.plist
	-rm -f -- "$(CWD)/$(PACKAGE_NAME)/ReadMe.html"

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
	@echo --------------------------------------------------
	@echo "OPT_CFLAGS: $(OPT_CFLAGS)"
	@echo "PD_VERSION: $(PD_VERSION)"
	@echo "PACKAGE_NAME: $(PACKAGE_NAME)"
	@echo "PD-EXTENDED_VERSION: $(PD-EXTENDED_VERSION)"
	@echo "WISH: $(WISH)"
	@echo "WISH_FRAMEWORKS_SOURCE: $(WISH_FRAMEWORKS_SOURCE)"
	@echo "Tcl/Tk Version: $(shell sed -n 's|.*>\(8\..*\)<.*|\1|p' $(WISH_FRAMEWORKS_SOURCE)/Tk.framework/Resources/Info.plist | head -1)"
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
	@echo --------------------------------------------------
	sw_vers
	@echo --------------------------------------------------
	autoconf --version
	@echo --------------------------------------------------
	make --version
	@echo --------------------------------------------------
	gcc --version
	@echo --------------------------------------------------
	xcodebuild -version || pbxbuild -version
