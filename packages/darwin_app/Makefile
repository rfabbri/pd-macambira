
current: darwin_app

# needed to support weak linking of frameworks
MAKE=MACOSX_DEPLOYMENT_TARGET=10.3 make

CWD := $(shell pwd)
BUILD_BASE = $(CWD)/build
PD_APP_NAME = Pd
PD_APP_CONTENTS = $(BUILD_BASE)/$(PD_APP_NAME).app/Contents
INSTALL_PREFIX = $(PD_APP_CONTENTS)/Resources

SRC_ROOT_DIR = $(CWD)/../..
BUILDLAYOUT_DIR = $(PACKAGES_SRC)

# source for the Wish Shell.app used to build Pd
# download TclTkAquaStandalone-8.?.*.dmg from http://tcltkaqua.sourceforge.net/
# and mount it.  The files will be copied from the mounted .dmg image.
#WISH = /Applications/Utilities/Wish\ Shell.app
TCLTK := $(shell /bin/ls -1r TclTkAquaStandalone-8.?.*.dmg|head -1|sed 's/\.dmg//')
WISH_NAME = Wish\ Shell
WISH = /Volumes/$(TCLTK)/$(WISH_NAME).app
WISH_CONTENTS = $(WISH)/Contents


# locations of the various directories from CVS
SRC_ROOT_DIR = $(CWD)/../..

include ../Makefile.buildlayout

# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				SRC_ROOT_DIR=$(SRC_ROOT_DIR) \
				INSTALL_PREFIX=$(INSTALL_PREFIX) \
				UNAME=$(UNAME)

install: darwin_app

#------------------------------------------------------------------------------#
darwin_app: darwin_app_core extended_app_install

#------------------------------------------------------------------------------#
# this target is for when you already have a pre-built Pd.app and you
# just want to fill it with goodies from CVS.  To do so, place your 
# Pd.app in $(BUILD_BASE)/Pd.app
extended_app_install:  extended_install darwin_app_noncvs darwin_app_doc_format


#------------------------------------------------------------------------------#
# check here for a reference on how to do this:
# http://cvs.sourceforge.net/viewcvs.py/tkcvs/tkcvs-proj/PackApp?rev=1.4
darwin_app_wrapper:
	test -d /Volumes/$(TCLTK) || hdiutil mount $(TCLTK).dmg
# copy Wish Shell.app from default install location
# note: use the "standalone" Wish shell to make a "standalone" pd app
	install -d "$(PD_APP_CONTENTS)/MacOS"
	install -m0755 -p \
		$(WISH_CONTENTS)/MacOS/$(WISH_NAME) \
		"$(PD_APP_CONTENTS)/MacOS"
	install -d "$(PD_APP_CONTENTS)/Resources"
	install -m0644 -p \
		$(WISH_CONTENTS)/Resources/Wish\ Shell.rsrc \
		"$(PD_APP_CONTENTS)/Resources"
	install -d "$(PD_APP_CONTENTS)/Frameworks"
	cp -Rp $(WISH_CONTENTS)/Frameworks "$(PD_APP_CONTENTS)"
# set up app wrapper
	install -d "$(PD_APP_CONTENTS)/Resources/Scripts"
	install -m0644 -p Info.plist "$(PD_APP_CONTENTS)"
	install -m0644 -p *.icns  "$(PD_APP_CONTENTS)/Resources"
# rename the Wish Shell executable to "pd", the all lowercase is important
# since it needs to match the pd executable in order for the Gem windows to
# get focus properly
	mv "$(PD_APP_CONTENTS)/MacOS/Wish Shell" \
		"$(PD_APP_CONTENTS)/MacOS/pd"
#	diskutil eject /Volumes/$(TCLTK)



#------------------------------------------------------------------------------#
darwin_app_core: darwin_app_wrapper
	cd $(PACKAGES_SRC) && \
		make $(DEST_PATHS) PD_CONFIGURE_FLAGS=--enable-jack pd_install 
# tigital's Gem window focus black magic:
	install -p $(CWD)/mac.r $(BIN_DEST)
	cd $(BIN_DEST) && /Developer/tools/Rez -t APPL mac.r -o pd
	cd "$(PD_APP_CONTENTS)/Resources/Scripts" && \
		ln -s ../bin/pd.tk AppMain.tcl
# support for Info Panel Plugins mgmt
	cd "$(PD_APP_CONTENTS)" && ln -s Resources/extra Plugins
# Headers
	install -d $(INSTALL_PREFIX)/include
	install $(PD_SRC)/src/*.h $(INSTALL_PREFIX)/include
# run script to move help-*.pd files to *-help.pd according to the standard
	cd $(DOCS_DEST) && \
		$(SCRIPTS_SRC)/convert-help-to-standard.sh


darwin_app_doc_format:
# set the font to 10pt on all help patches
# the BSD/Darwin version of sed must create a backup file when doing 
# in-place replacement, so delete the unneeded backup files
	cd $(DOCS_DEST) && \
		sed -i.bak 's/^\(\#N canvas [0-9]* [0-9]* [0-9]* [0-9]*\) 12/\1 10/' \
			*/*.pd */*/*.pd */*/*/*.pd

#==============================================================================#
#
## CVS SOURCES
#
#==============================================================================#

extended_install:
	cd $(PACKAGES_SRC) && make $(DEST_PATHS) install


#------------------------------------------------------------------------------#
darwin_app_externals: darwin_app_externals_standard darwin_app_externals_c++ darwin_app_externals_flext
#darwin_app_externals: darwin_app_externals_standard darwin_app_externals_c++ darwin_app_externals_gem


#------------------------------------------------------------------------------#
# GEM
darwin_app_externals_gem:
	cd $(GEM_SRC)/Gem && xcodebuild
	install -p -m0644 $(GEM_SRC)/Gem/help/*.* $(HELP_DEST)
	install -d $(DOCS_DEST)/Gem/doc
	install -p -m0644 $(GEM_SRC)/Gem/doc/*.* $(DOCS_DEST)/gem
	install -d $(DOCS_DEST)/Gem/00.manual
	install -p -m0644 $(GEM_SRC)/Gem/manual/*.* $(DOCS_DEST)/gem/00.manual
	cp -Rfp $(GEM_SRC)/Gem/examples/*.* $(GEM_SRC)/Gem/examples/data $(DOCS_DEST)/gem


#------------------------------------------------------------------------------#
darwin_app_externals_flext:
#----------------------------------------------------------------------------
# FLEXT
# temp hack -HCS
	test -f $(EXTERNALS_SRC)/grill/flext/buildsys/mac/pd/config-gcc.def || \
		cp $(EXTERNALS_SRC)/grill/flext/buildsys/mac/pd/config-gcc.def \
			$(EXTERNALS_SRC)/grill/flext/buildsys/config-mac-pd-gcc.txt
# end temp hack
# tigital 8/30/2005 added
	cd $(EXTERNALS_SRC)/grill/flext && \
		( \
        test -f buildsys/config-mac-pd-gcc.txt || \
          ( \
            cp buildsys/mac/pd/config-gcc.def buildsys/config-mac-pd-gcc.txt && \
            echo Copied template sys config file \
          ) \
      ) && \
		( \
			cat buildsys/config-mac-pd-gcc.txt | \
				sed "s/^PDPATH=.*/PDPATH=..\/..\/..\/pd/" > buildsys/config-mac-pd-gcc.txt \
		) && \
		( \
			cat package.txt | sed "s/^PRECOMPILE=.*/PRECOMPILE=/" > package.txt \
		) && \
		( \
			cat buildsys/mac/pd/gnumake-gcc-ext.inc | \
				sed "s/^LDFLAG\S +=.*/LDFLAG\S += -bundle -bundle_loader ..\/..\/..\/pd\/bin\/pd/" > buildsys/mac/pd/gnumake-gcc-ext.inc \
		) && \
			( \
			cat buildsys/mac/pd/gnumake-gcc-flext.inc | \
				sed "s/^LDFLAG\S +=.*/LDFLAG\S += -dynamiclib -undefined dynamic_lookup /" > buildsys/mac/pd/gnumake-gcc-flext.inc \
		) && \
		( \
			test -f config.txt || \
			( \
				cp -f build/config-mac.def config.txt && \
				echo Copied template flext config file \
			) \
		) && \
		MACOSX_DEPLOYMENT_TARGET=10.3 bash build.sh pd gcc build-release-shared FLEXTLIB=@executable_path/../extra FLEXTINC=../flext/source FLEXTSYS=$(OBJECTS_DEST)
	ln -sf $(EXTERNALS_SRC)/grill/flext/pd-darwin/release-shared/libflext-pd.$(FLEXT_VERSION).dylib $(EXTERNALS_SRC)/grill/flext/pd-darwin/release-shared/libflext-pd.dylib
	install -p -m0444 $(EXTERNALS_SRC)/grill/flext/pd-darwin/release-shared/libflext-pd.$(FLEXT_VERSION).dylib $(OBJECTS_DEST)
	ln -sf libflext-pd.$(FLEXT_VERSION).dylib $(OBJECTS_DEST)/libflext-pd.dylib
	install -d $(DOCS_DEST)/tutorials
	cp -rp $(EXTERNALS_SRC)/grill/flext/tutorial $(DOCS_DEST)/tutorials/flext
	install -d $(DOCS_DEST)/flext
	install -p -m0644 $(EXTERNALS_SRC)/grill/flext/gpl.txt  \
		$(EXTERNALS_SRC)/grill/flext/readme.txt   \
		$(EXTERNALS_SRC)/grill/flext/license.txt  \
		$(DOCS_DEST)/flext
# FLEXTERNALS
#########
# dyn~
	cd $(EXTERNALS_SRC)/grill/dynext && bash ../flext/build.sh pd gcc build-release-shared FLEXTLIB=../flext/pd-darwin/release-shared FLEXTINC=../flext/source FLEXTSYS=$(OBJECTS_DEST)
	install -p $(EXTERNALS_SRC)/grill/dynext/pd-darwin/release-shared/*.pd_darwin $(OBJECTS_DEST)
	install -p $(EXTERNALS_SRC)/grill/dynext/pd/*.pd $(DOCS_DEST)
	install -d $(DOCS_DEST)/dynext
	install -p $(EXTERNALS_SRC)/grill/dynext/gpl.txt  \
		$(EXTERNALS_SRC)/grill/dynext/readme.txt   \
		$(EXTERNALS_SRC)/grill/dynext/license.txt  \
		$(DOCS_DEST)/dynext
#########
# pool
	cd $(EXTERNALS_SRC)/grill/pool && bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(OBJECTS_DEST) FLEXTINC=../flext/source
	install -p $(EXTERNALS_SRC)/grill/pool/pd-darwin/release-shared/*.pd_darwin $(OBJECTS_DEST)
	install -p $(EXTERNALS_SRC)/grill/pool/help-pool.pd $(DOCS_DEST)
	install -d $(DOCS_DEST)/pool
	install -p $(EXTERNALS_SRC)/grill/pool/gpl.txt \
		$(EXTERNALS_SRC)/grill/pool/readme.txt   \
		$(EXTERNALS_SRC)/grill/pool/license.txt \
		$(DOCS_DEST)/pool
#########
# py/pyext
#	cd $(EXTERNALS_SRC)/grill/py && \
#		( \
#			cp -f build/config-mac.def config.txt \
#		) && \
#		bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(OBJECTS_DEST) FLEXTINC=../flext/source
#	install -p -m0444 $(EXTERNALS_SRC)/grill/py/pd-darwin/release-shared/*.pd_darwin $(OBJECTS_DEST)
# this wasn't building for me -HCS 2005-06-09
	install -d $(DOCS_DEST)/py/examples
	install -d $(DOCS_DEST)/py/scripts
	install -p $(EXTERNALS_SRC)/grill/py/pd/*.pd $(DOCS_DEST)/py/examples
	install -p $(EXTERNALS_SRC)/grill/py/scripts/*.py $(DOCS_DEST)/py/scripts
	install -p $(EXTERNALS_SRC)/grill/py/gpl.txt  \
		$(EXTERNALS_SRC)/grill/py/readme.txt $(EXTERNALS_SRC)/grill/py/license.txt \
		$(DOCS_DEST)/py
#	install -d $(INSTALL_PREFIX)/Library/Frameworks
#	cp -R /Library/Frameworks/Python.framework $(INSTALL_PREFIX)/Library/Frameworks
#########
# vasp
#  fails without the dir
#	test -d ../../vasp/pd-darwin || mkdir -p ../../vasp/pd-darwin 
#	cd $(EXTERNALS_SRC)/grill/vasp/ && bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(OBJECTS_DEST) FLEXTINC=../flext/source
#	install -p -m0444 $(EXTERNALS_SRC)/grill/vasp/pd-darwin/release-shared/*.pd_darwin $(OBJECTS_DEST)
# this wasn't building for me -HCS 2005-06-09
	install -p -m0644 $(EXTERNALS_SRC)/grill/vasp/pd-help/*.pd $(DOCS_DEST)
	install -d $(DOCS_DEST)/vasp
	install -p -m0644 $(EXTERNALS_SRC)/grill/vasp/gpl.txt  \
		$(EXTERNALS_SRC)/grill/vasp/readme.txt   \
		$(EXTERNALS_SRC)/grill/vasp/license.txt  \
		$(EXTERNALS_SRC)/grill/vasp/changes.txt  \
		$(EXTERNALS_SRC)/grill/vasp/todo.txt     \
		$(DOCS_DEST)/vasp
	cp -rp $(EXTERNALS_SRC)/grill/vasp/pd-ex $(DOCS_DEST)/vasp
#########
# xsample
	cd $(EXTERNALS_SRC)/grill/xsample && bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(OBJECTS_DEST) FLEXTINC=../flext/source
	install -p -m0444 $(EXTERNALS_SRC)/grill/xsample/pd-darwin/release-shared/*.pd_darwin $(OBJECTS_DEST)
	install -p -m0644 $(EXTERNALS_SRC)/grill/xsample/pd/*.pd $(DOCS_DEST)
	install -d $(DOCS_DEST)/xsample
	install -p -m0644 $(EXTERNALS_SRC)/grill/xsample/gpl.txt \
		$(EXTERNALS_SRC)/grill/xsample/readme.txt \
		$(EXTERNALS_SRC)/grill/xsample/license.txt \
		$(DOCS_DEST)/xsample
########## FOOTILS #########
# syncgrain
#	cd $(EXTERNALS_SRC)/footils/syncgrain/ && $(MAKE) -f makefile.pd-darwin
#	install -p -m0444 $(EXTERNALS_SRC)/footils/syncgrain/pd-darwin/*.pd_darwin \
#		$(OBJECTS_DEST)
	install -p -m0644 $(EXTERNALS_SRC)/footils/syncgrain/pd/*.* $(DOCS_DEST)




darwin_app_externals_c++:
#----------------------------------------------------------------------------
# CREB
#	cd $(EXTERNALS_SRC)/creb && autoconf && ./configure && cd modules++ && $(MAKE)
#	install -p -m0644 $(EXTERNALS_SRC)/creb/modules++/*.pd_darwin $(OBJECTS_DEST)
#----------------------------------------------------------------------------
# gem2pdp
#	cd $(EXTERNALS_SRC)/gem2pdp && $(MAKE) -f Makefile.darwin
#	install -p -m0444 $(EXTERNALS_SRC)/gem2pdp/*.pd_darwin $(OBJECTS_DEST)
#----------------------------------------------------------------------------
# GridFlow
#	cd ../../gridflow && ./configure && $(MAKE)


darwin_app_noncvs:
	test -d noncvs/doc/5.reference && \
		install -p noncvs/doc/5.reference/*.* $(HELP_DEST)
	test -d noncvs/extra && install -p noncvs/extra/*.* $(OBJECTS_DEST)
	test -d noncvs/doc/serendipd && cp -Rp noncvs/doc/serendipd \
		$(APPLICATIONS_DEST)

darwin_app_perms:
	chmod a-x $(DOCS_DEST)/*/*.pd $(DOCS_DEST)/*/*/*.pd $(DOCS_DEST)/*/*/*/*.pd
	chmod -R a-w $(DOCS_DEST)
# serendiPd shared patch
	test -d $(APPLICATIONS_DEST)/serendipd && \
		chmod a+w $(APPLICATIONS_DEST)/serendipd/*shared_patch.pd

dmg: darwin_app_perms
	install -d "$(CWD)/$(DMG_NAME)"
	cd $(BUILD_BASE) && mv $(PD_APP_NAME).app "$(CWD)/$(DMG_NAME)/$(DMG_NAME).app"
	install -p -m0444 $(INSTALL_PREFIX)/*.txt $(INSTALL_PREFIX)/*.html  \
		"$(CWD)/$(DMG_NAME)"
	cd $(CWD) && /bin/sh mkdmg "$(DMG_NAME)"


clean: darwin_app_clean darwin_app_externals_standard_clean darwin_pd_clean
clean: darwin_app_externals_gem_clean darwin_app_externals_flext_clean

darwin_pd_clean:
	cd $(PD_SRC)/src/ && make clean

darwin_app_clean:
	-sudo rm -Rf -- $(BUILD_BASE) "$(DMG_NAME)" 
	-rm -f -- *~ 1 $(README_FILE) $(WELCOME_FILE) $(LICENSE_FILE) \
		$(UNIX_PACKAGE_NAME).pkg  "$(DMG_NAME).dmg"

darwin_app_externals_standard_clean:
	cd $(EXTERNALS_SRC)/build/darwin && make clean
	cd $(EXTERNALS_SRC)/hcs/hid && make clean

darwin_app_externals_gem_clean:
	cd $(GEM_SRC)/Gem && xcodebuild clean

darwin_app_externals_flext_clean:
	cd $(EXTERNALS_SRC)/footils/syncgrain && make -f makefile.pd-darwin clean
	-cd $(EXTERNALS_SRC)/grill && \
		rm -- flext/buildsys/config-mac-pd-gcc.txt config.txt flext/config.txt \
			flext/source/libflext.a */pd-darwin/*.o */pd-darwin/*/*.o \
		 	*/pd-darwin/*/*.pd_darwin */pd-darwin/*.pd_darwin \
			*/pd-darwin/release-shared/*.dylib */pd-darwin/release-shared/*.opp



#==============================================================================#
#
## DEPRECATED TARGETS
#
#==============================================================================#

darwin_app_externals:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(EXTERNALS_SRC) && make install"

darwin_app_externals_standard:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(EXTERNALS_SRC) && make install"

darwin_patch_pd:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd .. && make patch_pd"

darwin_unpatch_pd:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd .. && make unpatch_pd"

darwin_app_docs:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(DOC_SRC) && make install"

darwin_app_license:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd .. && make license"

darwin_app_welcome:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd .. && make welcome"

darwin_app_readme:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd .. && make readme"

darwin_prebuilt_app:
	@echo " "
	@echo "Deprecated!!" 
	@echo "Use make extended_app_install"

