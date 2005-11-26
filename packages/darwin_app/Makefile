
current: darwin_app

# needed to support weak linking of frameworks
MAKE=MACOSX_DEPLOYMENT_TARGET=10.3 make

CWD := $(shell pwd)
BUILD_BASE = $(CWD)/build
PD_APP_NAME = $(PACKAGE_NAME)
PD_APP_CONTENTS = $(BUILD_BASE)/$(PD_APP_NAME).app/Contents
prefix = $(PD_APP_CONTENTS)/Resources

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

# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				cvs_root_dir=$(cvs_root_dir) \
				prefix=$(prefix) \
				UNAME=$(UNAME)

install: darwin_app

#------------------------------------------------------------------------------#
darwin_app: darwin_app_core extended_app_install darwin_app_doc_format rez_install
	@echo " "
	@echo "Pd.app build succeeded!"

#------------------------------------------------------------------------------#
# this target is for when you already have a pre-built Pd.app and you
# just want to fill it with goodies from CVS.  To do so, place your 
# Pd.app in $(BUILD_BASE)/Pd.app
extended_app_install:  extended_install


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
		$(WISH_CONTENTS)/Resources/Wish\ Shell.rsrc \
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


#------------------------------------------------------------------------------#
darwin_app_core: darwin_app_wrapper
	cd $(packages_src) && \
		make $(DEST_PATHS) PD_CONFIGURE_FLAGS=--enable-jack pd_install 
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
	find $(prefix) -name '*.pd.bak' -delete

#==============================================================================#
#
## CVS SOURCES
#
#==============================================================================#

extended_install:
	cd $(packages_src) && make $(DEST_PATHS) install


#------------------------------------------------------------------------------#
darwin_app_externals: darwin_app_externals_standard darwin_app_externals_c++ darwin_app_externals_flext
#darwin_app_externals: darwin_app_externals_standard darwin_app_externals_c++


#------------------------------------------------------------------------------#
darwin_app_externals_flext:
#----------------------------------------------------------------------------
# FLEXT
# temp hack -HCS
	test -f $(externals_src)/grill/flext/buildsys/mac/pd/config-gcc.def || \
		cp $(externals_src)/grill/flext/buildsys/mac/pd/config-gcc.def \
			$(externals_src)/grill/flext/buildsys/config-mac-pd-gcc.txt
# end temp hack
# tigital 8/30/2005 added
	cd $(externals_src)/grill/flext && \
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
		MACOSX_DEPLOYMENT_TARGET=10.3 bash build.sh pd gcc build-release-shared FLEXTLIB=@executable_path/../extra FLEXTINC=../flext/source FLEXTSYS=$(objectsdir)
	ln -sf $(externals_src)/grill/flext/pd-darwin/release-shared/libflext-pd.$(FLEXT_VERSION).dylib $(externals_src)/grill/flext/pd-darwin/release-shared/libflext-pd.dylib
	install -p -m0444 $(externals_src)/grill/flext/pd-darwin/release-shared/libflext-pd.$(FLEXT_VERSION).dylib $(objectsdir)
	ln -sf libflext-pd.$(FLEXT_VERSION).dylib $(objectsdir)/libflext-pd.dylib
	install -d $(pddocdir)/tutorials
	cp -rp $(externals_src)/grill/flext/tutorial $(pddocdir)/tutorials/flext
	install -d $(pddocdir)/flext
	install -p -m0644 $(externals_src)/grill/flext/gpl.txt  \
		$(externals_src)/grill/flext/readme.txt   \
		$(externals_src)/grill/flext/license.txt  \
		$(pddocdir)/flext
# FLEXTERNALS
#########
# dyn~
	cd $(externals_src)/grill/dynext && bash ../flext/build.sh pd gcc build-release-shared FLEXTLIB=../flext/pd-darwin/release-shared FLEXTINC=../flext/source FLEXTSYS=$(objectsdir)
	install -p $(externals_src)/grill/dynext/pd-darwin/release-shared/*.pd_darwin $(objectsdir)
	install -p $(externals_src)/grill/dynext/pd/*.pd $(pddocdir)
	install -d $(pddocdir)/dynext
	install -p $(externals_src)/grill/dynext/gpl.txt  \
		$(externals_src)/grill/dynext/readme.txt   \
		$(externals_src)/grill/dynext/license.txt  \
		$(pddocdir)/dynext
#########
# pool
	cd $(externals_src)/grill/pool && bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(objectsdir) FLEXTINC=../flext/source
	install -p $(externals_src)/grill/pool/pd-darwin/release-shared/*.pd_darwin $(objectsdir)
	install -p $(externals_src)/grill/pool/help-pool.pd $(pddocdir)
	install -d $(pddocdir)/pool
	install -p $(externals_src)/grill/pool/gpl.txt \
		$(externals_src)/grill/pool/readme.txt   \
		$(externals_src)/grill/pool/license.txt \
		$(pddocdir)/pool
#########
# py/pyext
#	cd $(externals_src)/grill/py && \
#		( \
#			cp -f build/config-mac.def config.txt \
#		) && \
#		bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(objectsdir) FLEXTINC=../flext/source
#	install -p -m0444 $(externals_src)/grill/py/pd-darwin/release-shared/*.pd_darwin $(objectsdir)
# this wasn't building for me -HCS 2005-06-09
	install -d $(pddocdir)/py/examples
	install -d $(pddocdir)/py/scripts
	install -p $(externals_src)/grill/py/pd/*.pd $(pddocdir)/py/examples
	install -p $(externals_src)/grill/py/scripts/*.py $(pddocdir)/py/scripts
	install -p $(externals_src)/grill/py/gpl.txt  \
		$(externals_src)/grill/py/readme.txt $(externals_src)/grill/py/license.txt \
		$(pddocdir)/py
#	install -d $(prefix)/Library/Frameworks
#	cp -R /Library/Frameworks/Python.framework $(prefix)/Library/Frameworks
#########
# vasp
#  fails without the dir
#	test -d ../../vasp/pd-darwin || mkdir -p ../../vasp/pd-darwin 
#	cd $(externals_src)/grill/vasp/ && bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(objectsdir) FLEXTINC=../flext/source
#	install -p -m0444 $(externals_src)/grill/vasp/pd-darwin/release-shared/*.pd_darwin $(objectsdir)
# this wasn't building for me -HCS 2005-06-09
	install -p -m0644 $(externals_src)/grill/vasp/pd-help/*.pd $(pddocdir)
	install -d $(pddocdir)/vasp
	install -p -m0644 $(externals_src)/grill/vasp/gpl.txt  \
		$(externals_src)/grill/vasp/readme.txt   \
		$(externals_src)/grill/vasp/license.txt  \
		$(externals_src)/grill/vasp/changes.txt  \
		$(externals_src)/grill/vasp/todo.txt     \
		$(pddocdir)/vasp
	cp -rp $(externals_src)/grill/vasp/pd-ex $(pddocdir)/vasp
#########
# xsample
	cd $(externals_src)/grill/xsample && bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(objectsdir) FLEXTINC=../flext/source
	install -p -m0444 $(externals_src)/grill/xsample/pd-darwin/release-shared/*.pd_darwin $(objectsdir)
	install -p -m0644 $(externals_src)/grill/xsample/pd/*.pd $(pddocdir)
	install -d $(pddocdir)/xsample
	install -p -m0644 $(externals_src)/grill/xsample/gpl.txt \
		$(externals_src)/grill/xsample/readme.txt \
		$(externals_src)/grill/xsample/license.txt \
		$(pddocdir)/xsample
########## FOOTILS #########
# syncgrain
#	cd $(externals_src)/footils/syncgrain/ && $(MAKE) -f makefile.pd-darwin
#	install -p -m0444 $(externals_src)/footils/syncgrain/pd-darwin/*.pd_darwin \
#		$(objectsdir)
	install -p -m0644 $(externals_src)/footils/syncgrain/pd/*.* $(pddocdir)




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

DMG_NAME = $(PACKAGE_NAME)
dmg: darwin_app_perms
	install -d "$(CWD)/$(DMG_NAME)"
	cd $(BUILD_BASE) && mv $(PD_APP_NAME).app "$(CWD)/$(DMG_NAME)/"
#	install -p -m0444 $(manualsdir)/$(PD_NAME)/PD\ LICENSE.txt \
#		$(manualsdir)/*.html "$(CWD)/$(DMG_NAME)"
	install -p -m0444 org.puredata.pd.plist "$(CWD)/$(DMG_NAME)"
	cd $(CWD) && /bin/sh mkdmg "$(DMG_NAME)"


clean: darwin_app_clean	darwin_app_externals_flext_clean \
	cd $(packages_src) && make clean
	-rm Info.plist


darwin_app_clean:
	-sudo rm -Rf -- $(BUILD_BASE) "$(DMG_NAME)" 
	-rm -f -- *~ 1 $(README_FILE) $(WELCOME_FILE) $(LICENSE_FILE) \
		$(UNIX_PACKAGE_NAME).pkg  "$(DMG_NAME).dmg"




	cd $(gem_src)/Gem && xcodebuild clean

darwin_app_externals_flext_clean:
	cd $(externals_src)/footils/syncgrain && make -f makefile.pd-darwin clean
	-cd $(externals_src)/grill && \
		rm -- flext/buildsys/config-mac-pd-gcc.txt config.txt flext/config.txt \
			flext/source/libflext.a */pd-darwin/*.o */pd-darwin/*/*.o \
		 	*/pd-darwin/*/*.pd_darwin */pd-darwin/*.pd_darwin \
			*/pd-darwin/release-shared/*.dylib */pd-darwin/release-shared/*.opp



#==============================================================================#
#
## DEPRECATED TARGETS
#
#==============================================================================#

darwin_app_noncvs:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(packages_src) && make noncvs_install"
	cd $(packages_src) && make $(DEST_PATHS) noncvs_install

darwin_app_externals_gem_clean:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(packages_src) && make gem_clean"

darwin_app_externals_gem:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(packages_src) && make gem_install"

darwin_app_externals_standard_clean:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(packages_src) && make externals_clean"

darwin_pd_clean:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(packages_src) && make pd_clean"

darwin_app_externals:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(externals_src) && make install"

darwin_app_externals_standard:
	@echo " "
	@echo "Deprecated!!" 
	@echo "cd $(externals_src) && make install"

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
	@echo "cd $(doc_src) && make install"

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

