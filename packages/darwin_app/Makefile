
current: darwin_app

# Pd needs autoconf 2.59, which is labeled differently on different machines
PD_AUTOCONF := ${shell ( test -x "`which autoconf-2.59`" && echo autoconf-2.59 ) || echo autoconf }

# needed to support weak linking of frameworks
MAKE=MACOSX_DEPLOYMENT_TARGET=10.2 make

CWD := $(shell pwd)
DESTDIR = $(CWD)/build
PD_APP_NAME = Pd
PD_APP_CONTENTS = $(DESTDIR)/$(PD_APP_NAME).app/Contents
INSTALL_PREFIX = $(PD_APP_CONTENTS)/Resources


# source for the Wish Shell.app used to build Pd
# download TclTkAquaStandalone-8.?.*.dmg from http://tcltkaqua.sourceforge.net/
# and mount it.  The files will be copied from the mounted .dmg image.
#WISH_SHELL = /Applications/Utilities/Wish\ Shell.app
TCLTK := $(shell /bin/ls -1r TclTkAquaStandalone-8.?.*.dmg|head -1|sed 's/\.dmg//')
WISH_SHELL = /Volumes/$(TCLTK)/Wish\ Shell.app
WISH_SHELL_CONTENTS = $(WISH_SHELL)/Contents


# locations of the various directories from CVS
SRC_ROOT_DIR = $(CWD)/../..

ABSTRACTIONS_SRC = $(SRC_ROOT_DIR)/abstractions
DOC_SRC = $(SRC_ROOT_DIR)/doc
EXTERNALS_SRC = $(SRC_ROOT_DIR)/externals
GEM_SRC = $(SRC_ROOT_DIR)/Gem
GEMLIBS_SRC = $(SRC_ROOT_DIR)/GemLibs
PD_SRC = $(SRC_ROOT_DIR)/pd
SCRIPTS_SRC = $(SRC_ROOT_DIR)/scripts


# final destinations for compiled files
ABSTRACTIONS_DST = $(INSTALL_PREFIX)/doc/abstractions
EXTERNALS_DST = $(INSTALL_PREFIX)/extra
DOC_DST = $(INSTALL_PREFIX)/doc


PD_MAJOR_VERSION := $(shell /usr/bin/grep 'Pd version' $(PD_SRC)/src/s_main.c | sed 's/char pd_version\[\] = "Pd version \([0-9]\)\.[0-9]*[. TES]*[0-9]*[0-9extndRC.-]*\\n";/\1/')
PD_MINOR_VERSION := $(shell /usr/bin/grep 'Pd version' $(PD_SRC)/src/s_main.c | sed 's/char pd_version\[\] = "Pd version [0-9]\.\([0-9]*\)[. TES]*\([0-9]*\)[0-9extndRC.-]*\\n";/\1/')
PD_BUGFIX_VERSION := $(shell /usr/bin/grep 'Pd version' $(PD_SRC)/src/s_main.c | sed 's/char pd_version\[\] = "Pd version [0-9]\.[0-9]*[. TES]*\([0-9]*\)[0-9extndRC.-]*\\n";/\1/')
PD_VERSION = $(PD_MAJOR_VERSION).$(PD_MINOR_VERSION).$(PD_BUGFIX_VERSION)


# release version for this distro
PACKAGE_VERSION = extended-RC2
PACKAGE_NAME = Pd-$(PD_VERSION)-$(PACKAGE_VERSION)
DMG_NAME = $(PACKAGE_NAME)


WELCOME_FILE = Welcome.html
README_FILE = ReadMe.html
LICENSE_FILE = License.html


CYCLONE_MAJOR_VERSION := $(shell grep CYCLONE_VERSION $(EXTERNALS_SRC)/miXed/cyclone/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
CYCLONE_RELEASE := $(shell grep CYCLONE_RELEASE $(EXTERNALS_SRC)/miXed/cyclone/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
CYCLONE_BUILD := $(shell grep CYCLONE_BUILD $(EXTERNALS_SRC)/miXed/cyclone/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
CYCLONE_VERSION := $(CYCLONE_MAJOR_VERSION) $(CYCLONE_RELEASE) $(CYCLONE_BUILD)

FLEXT_VERSION := $(shell grep FLEXTMAJOR $(EXTERNALS_SRC)/grill/flext/buildsys/version.inc | cut -d '=' -f2).$(shell grep FLEXTMINOR $(EXTERNALS_SRC)/grill/flext/buildsys/version.inc | cut -d '=' -f2).$(shell grep FLEXTMICRO $(EXTERNALS_SRC)/grill/flext/buildsys/version.inc | cut -d '=' -f2)

GEM_VERSION := $(shell grep "GEM_VERSION" ../../Gem/src/Base/GemVersion.h | cut -d '"' -f 2)

MAXLIB_VERSION := $(shell grep "define VERSION" $(EXTERNALS_SRC)/maxlib/maxlib.c | cut -d '"' -f 2)

OSC_VERSION := $(shell grep "define VERSION" $(EXTERNALS_SRC)/OSCx/src/OSC.c | cut -d '"' -f 2)

PDP_VERSION := $(shell grep PDP_VERSION= $(EXTERNALS_SRC)/pdp/configure | cut -d '=' -f 2)

PMPD_VERSION := $(shell grep "define VERSION" $(EXTERNALS_SRC)/pmpd/src/pmpd.c | cut -d '"' -f 2)

TOXY_MAJOR_VERSION := $(shell grep TOXY_VERSION $(EXTERNALS_SRC)/miXed/toxy/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
TOXY_RELEASE := $(shell grep TOXY_RELEASE $(EXTERNALS_SRC)/miXed/toxy/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
TOXY_BUILD := $(shell grep TOXY_BUILD $(EXTERNALS_SRC)/miXed/toxy/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
TOXY_VERSION := $(TOXY_MAJOR_VERSION) $(TOXY_RELEASE) $(TOXY_BUILD)

ZEXY_VERSION := $(shell grep VERSION $(EXTERNALS_SRC)/zexy/src/zexy.h | cut -d ' ' -f 3 | cut -d '"' -f 2)


darwin_app: darwin_app_core darwin_prebuilt_app

# this target is for when you already have a pre-built Pd.app and you
# just want to fill it with goodies from CVS.  To do so, place your 
# Pd.app in $(DESTDIR)/Pd.app
darwin_prebuilt_app: darwin_app_externals darwin_app_docs \
	darwin_app_abstractions darwin_app_noncvs darwin_app_doc_format

darwin_app_license:
  # generate HTML version of License
	echo "<html><body>" > $(LICENSE_FILE)
	echo "<h3>(Parts of this package can be used under the Pd/BSD license)</h3>" >> $(LICENSE_FILE)
	echo "<font size=\"-1\">" >> $(LICENSE_FILE)
	cat $(EXTERNALS_SRC)/creb/COPYING | sed -e 's/^$$/\<P\>/g' >> $(LICENSE_FILE)	
	echo "</font></body></html>" >> $(LICENSE_FILE)


darwin_app_readme: 
	echo $(CYCLONE_RELEASE)
	-rm $(README_FILE)
	echo "<html>" > $(README_FILE)
	echo "<head>" >> $(README_FILE)
	echo "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=iso-8859-1\" />" >> $(README_FILE)
	echo "</head>" >> $(README_FILE)
	echo "<body>" >> $(README_FILE)
	echo "<h2>Pure Data $(PD_VERSION)-$(PACKAGE_VERSION)</h2>" 	>> $(README_FILE)
	echo "<p>Pd is a free real-time computer music software package resembling Max.  It provides a patchable environment for audio analysis, synthesis, and processing, with a rich set of multimedia capabilities.  You can get Pd for Linux, Windows, MacOS X, BSD, or IRIX.</p>" >> $(README_FILE)
	echo "<p>For more information, go to: <a href=\"http://puredata.org\" target=\"pd\">http://puredata.org</a></p>" >> $(README_FILE)
	echo "<h3>Installation</h3>" >> $(README_FILE)	
	echo "<p>To install Pd, drag the Pd.app to anywhere in your hard disk.</p>" >> $(README_FILE)	
	echo "<p>To install Gem and pmpd support, copy <code>org.puredata.pd.plist</code> to <code>~/Library/Preferences</code> (<code>~</code> means your home folder).</p>" >> $(README_FILE)	
	echo "<h3>Pure Data CVS Developers</h3>" >> $(README_FILE)	
# this may seem whack, but it generates the list of developers from the SourceForge site:
	curl 'http://sourceforge.net/project/memberlist.php?group_id=55736' | grep -e '<td>' | sed -e :a -e 's/<[^>]*>//g;/</N;//ba' | grep -v '^$$' | sed -e 's/\t*/ - /' >> $(README_FILE)
	echo "</p>" >> $(README_FILE)
	echo "<p>" >> $(README_FILE)
	echo "Many others not listed have contributed their time and effort, this is just a list of the current developers in the SourceForge project.  But really, every Pd user is a developer and is encouraged to contribute to the CVS repository." >> $(README_FILE)
	echo "</p>" >> $(README_FILE)
	echo "<h3>License</h3>" >> $(README_FILE)	
	echo "<p>" >> $(README_FILE)
	echo "The Pd core is licensed under a BSD license, almost every other part of this package is available under the GNU GPL.  A couple packages have BSD-style licenses too." >> $(README_FILE)
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


# check here for a reference on how to do this:
# http://cvs.sourceforge.net/viewcvs.py/tkcvs/tkcvs-proj/PackApp?rev=1.4
darwin_app_wrapper:
	test -d /Volumes/$(TCLTK) || hdiutil mount $(TCLTK).dmg
# copy Wish Shell.app from default install location
# note: use the "standalone" Wish shell to make a "standalone" pd app
	install -d "$(PD_APP_CONTENTS)/MacOS"
	install -m0755 -p \
		$(WISH_SHELL_CONTENTS)/MacOS/Wish\ Shell \
		"$(PD_APP_CONTENTS)/MacOS"
	install -d "$(PD_APP_CONTENTS)/Resources"
	install -m0644 -p \
		$(WISH_SHELL_CONTENTS)/Resources/Wish\ Shell.rsrc \
		"$(PD_APP_CONTENTS)/Resources"
	install -d "$(PD_APP_CONTENTS)/Frameworks"
	cp -Rp $(WISH_SHELL_CONTENTS)/Frameworks "$(PD_APP_CONTENTS)"
# set up app wrapper
	install -d "$(PD_APP_CONTENTS)/Resources/Scripts"
	install -m0644 -p Info.plist "$(PD_APP_CONTENTS)"
	install -m0644 -p *.icns  "$(PD_APP_CONTENTS)/Resources"
# rename the Wish Shell executable to "Pd"
	mv "$(PD_APP_CONTENTS)/MacOS/Wish Shell" \
		"$(PD_APP_CONTENTS)/MacOS/Pd"
#	diskutil eject /Volumes/$(TCLTK)

darwin_patch_pd:
	sed -i.bak 's/\(pd_version\[\] = "Pd version \)[0-9extndRC.-]*/\1$(PD_VERSION)-$(PACKAGE_VERSION)/'  \
		$(PD_SRC)/src/s_main.c
	cd $(PD_SRC)/src/ && patch -p0 < \
		../../packages/darwin_app/patches/darwin_linking_fixes.patch
	cd $(PD_SRC)/src/ && patch -p0 < \
		../../packages/darwin_app/patches/extended-help-menu.patch
	cd $(PD_SRC)/src/ && patch -p0 < \
		../../packages/darwin_app/patches/display_tweaks.patch
	-rm -f -- $(PD_SRC)/src/configure  $(PD_SRC)/src/makefile

darwin_app_core: darwin_app_wrapper
# pd core
#	cd $(PD_SRC)/src/ && $(PD_AUTOCONF) && ./configure && $(MAKE)
# or pd core with jack
	cd $(PD_SRC)/src/ && $(PD_AUTOCONF) && ./configure --enable-jack && $(MAKE)
	cp -Rp -- $(PD_SRC)/bin $(PD_SRC)/doc $(PD_SRC)/extra $(PD_SRC)/man  \
		$(INSTALL_PREFIX)
# AppMain.tcl
	cd "$(PD_APP_CONTENTS)/Resources/Scripts" && \
		ln -s ../bin/pd.tk AppMain.tcl
# Headers
	install -d -m0755 $(INSTALL_PREFIX)/include
	install -m0444 $(PD_SRC)/src/*.h $(INSTALL_PREFIX)/include
# run script to move help-*.pd files to *-help.pd according to the standard
	cd $(DOC_DST)/5.reference && \
		$(SCRIPTS_SRC)/convert-help-to-standard.sh


darwin_app_doc_format:
#----------------------------------------------------------------------------
# clean out cruft files
	-find $(PD_APP_CONTENTS) -name .DS_Store -delete
	-rm -Rf `find $(INSTALL_PREFIX) -name CVS`
# set the font to 10pt on all help patches
# the BSD/Darwin version of sed must create a backup file when doing 
# in-place replacement, so delete the unneeded backup files
	cd $(DOC_DST) && \
		sed -i.bak 's/^\(\#N canvas [0-9]* [0-9]* [0-9]* [0-9]*\) 12/\1 10/' \
			*/*.pd */*/*.pd && \
		rm -f -- */*.pd.bak	
# run script to move help-*.pd files to *-help.pd according to the standard
	cd $(DOC_DST)/5.reference && \
		$(SCRIPTS_SRC)/convert-help-to-standard.sh
#----------------------------------------------------------------------------
# remove write perms to prevent people form editing the helpfiles by mistake
	cd $(DOC_DST) && chmod a-wx */*.pd */*/*.pd
	cd $(DOC_DST)/5.reference && chmod a-w ????*/*.* *.txt


darwin_app_docs:
#----------------------------------------------------------------------------
# externals help patches
	install -d $(DOC_DST)/5.reference
	cd $(EXTERNALS_SRC)/build/doc && $(MAKE)
	cp -Rp $(EXTERNALS_SRC)/build/doc/* $(DOC_DST)/5.reference
#----------------------------------------------------------------------------
# DOC
	install -d $(DOC_DST)/7.stuff
	cp -Rp $(DOC_SRC)/tutorials $(DOC_DST)
	cp -Rp $(DOC_SRC)/additional/pd-msg $(DOC_DST)/7.stuff/
#----------------------------------------------------------------------------
# PDDP
	install -m644 --group=staff $(DOC_SRC)/pddp/*.pd $(DOC_DST)/5.reference
#----------------------------------------------------------------------------
# DOC menus
	install -d $(DOC_DST)/menus
	cp -p ./doc-menu.pd $(DOC_DST)/menus
	./mkdocs $(DOC_DST)/2.control.examples >! $(DOC_DST)/menus/2.control.examples.pd
	./mkdocs $(DOC_DST)/3.audio.examples >! $(DOC_DST)/menus/3.audio.examples.pd
	./mkdocs $(DOC_DST)/4.fft.examples >! $(DOC_DST)/menus/4.fft.examples.pd
	./mkdocs $(DOC_DST)/5.reference >! $(DOC_DST)/menus/5.reference.pd


darwin_app_abstractions:
#----------------------------------------------------------------------------
# ABSTRACTIONS
	install -d $(ABSTRACTIONS_DST)
# audionerd
	cp -Rp $(ABSTRACTIONS_SRC)/audionerd $(ABSTRACTIONS_DST)
# footils
	cp -Rp $(ABSTRACTIONS_SRC)/footils $(ABSTRACTIONS_DST)
# keyboardkeys
	cp -Rp $(ABSTRACTIONS_SRC)/keyboardkeys $(ABSTRACTIONS_DST)
#----------------------------------------------------------------------------
# EXTERNALS written in Pd
# RRADical
	cp -Rp $(ABSTRACTIONS_SRC)/rradical $(EXTERNALS_DST)
	cp -Rp $(ABSTRACTIONS_SRC)/rradical/doc $(DOC_DST)/rradical
# footils
	install -p $(ABSTRACTIONS_SRC)/footils/softtakeover/sto-help.pd \
		$(DOC_DST)/5.reference
	install -p $(ABSTRACTIONS_SRC)/footils/softtakeover/sto.pd $(EXTERNALS_DST)
# fft_convolution
	install -p $(ABSTRACTIONS_SRC)/fft_convolution/*-help.pd \
		$(DOC_DST)/5.reference
	install -p -m0644 $(shell ls -1 $(ABSTRACTIONS_SRC)/fft_convolution/*.pd | \
		grep -v '\-help.pd') $(EXTERNALS_DST)
# La-kitchen
	install -d $(DOC_DST)/5.reference/La-kitchen
	install -p $(ABSTRACTIONS_SRC)/La-kitchen/*-help.pd \
		$(DOC_DST)/5.reference/La-kitchen
	install -d $(EXTERNALS_DST)/La-kitchen
	install -p -m0644 $(shell ls -1 $(ABSTRACTIONS_SRC)/La-kitchen/*.pd | \
		grep -v '\-help.pd') $(EXTERNALS_DST)/La-kitchen
# nusmuk
	install -d $(DOC_DST)/5.reference/nusmuk
	install -p $(ABSTRACTIONS_SRC)/nusmuk/*-help.pd \
		$(DOC_DST)/5.reference/nusmuk
	install -d $(EXTERNALS_DST)/nusmuk
	install -p -m0644 $(shell ls -1 $(ABSTRACTIONS_SRC)/nusmuk/*.pd | \
		grep -v '\-help.pd') $(EXTERNALS_DST)/nusmuk
# timestretch~
	install -p $(ABSTRACTIONS_SRC)/timestretch/*-help.pd \
		$(DOC_DST)/5.reference
	install -p -m0644 $(shell ls -1 $(ABSTRACTIONS_SRC)/timestretch/*.pd | \
		grep -v '\-help.pd') $(EXTERNALS_DST)
# vadsr~
	install -p $(ABSTRACTIONS_SRC)/vadsr~/vadsr-test.pd \
		$(DOC_DST)/5.reference/vadsr~-help.pd
	install -p $(ABSTRACTIONS_SRC)/vadsr~/vadsr~.pd $(EXTERNALS_DST)


darwin_app_externals: darwin_app_externals_standard darwin_app_externals_c++ darwin_app_externals_flext
#darwin_app_externals: darwin_app_externals_standard darwin_app_externals_c++ darwin_app_externals_gem

darwin_app_externals_gem:
	cd ../../Gem && pbxbuild

darwin_app_externals_flext:
#----------------------------------------------------------------------------
# FLEXT
# temp hack -HCS
	test -f $(EXTERNALS_SRC)/grill/flext/buildsys/mac/pd/config-gcc.def || \
		cp $(EXTERNALS_SRC)/grill/flext/buildsys/mac/pd/config-gcc.def \
			$(EXTERNALS_SRC)/grill/flext/buildsys/config-mac-pd-gcc.txt
# end temp hack
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
				sed "s/^PDPATH=.*/PDPATH=..\/..\/..\/pd/" | \
				sed "s/^FLEXTPREFIX=.*/FLEXTPREFIX=@executable_path\/..\/extra/" > buildsys/config-mac-pd-gcc.txt \
		) && \
		( \
			test -f config.txt || \
			( \
				cp -f build/config-mac.def config.txt && \
				echo Copied template flext config file \
			) \
		) && \
		bash build.sh pd gcc build-release-shared
	install -p -m0444 $(EXTERNALS_SRC)/grill/flext/pd-darwin/release-shared/libflext-pd.$(FLEXT_VERSION).dylib $(EXTERNALS_DST)
	ln -sf libflext-pd.$(FLEXT_VERSION).dylib $(EXTERNALS_DST)/libflext-pd.dylib
	install -d $(DOC_DST)/tutorials
	cp -rp $(EXTERNALS_SRC)/grill/flext/tutorial $(DOC_DST)/tutorials/flext
	install -d $(DOC_DST)/flext
	install -p -m0644 $(EXTERNALS_SRC)/grill/flext/gpl.txt  \
		$(EXTERNALS_SRC)/grill/flext/readme.txt   \
		$(EXTERNALS_SRC)/grill/flext/license.txt  \
		$(DOC_DST)/flext
# FLEXTERNALS
#########
# dyn~
	cd $(EXTERNALS_SRC)/grill/dynext && bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(EXTERNALS_DST) FLEXTINC=../flext/source
	install -p -m0444 $(EXTERNALS_SRC)/grill/dynext/pd-darwin/release-shared/*.pd_darwin $(EXTERNALS_DST)
	install -p -m0644 $(EXTERNALS_SRC)/grill/dynext/pd/*.pd $(DOC_DST)/5.reference
	install -d $(DOC_DST)/dynext
	install -p -m0644 $(EXTERNALS_SRC)/grill/dynext/gpl.txt  \
		$(EXTERNALS_SRC)/grill/dynext/readme.txt   \
		$(EXTERNALS_SRC)/grill/dynext/license.txt  \
		$(DOC_DST)/dynext
#########
# pool
	cd $(EXTERNALS_SRC)/grill/pool && bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(EXTERNALS_DST) FLEXTINC=../flext/source
	install -p -m0444 $(EXTERNALS_SRC)/grill/pool/pd-darwin/release-shared/*.pd_darwin $(EXTERNALS_DST)
	install -p -m0644 $(EXTERNALS_SRC)/grill/pool/help-pool.pd $(DOC_DST)/5.reference
	install -d $(DOC_DST)/pool
	install -p -m0644 $(EXTERNALS_SRC)/grill/pool/gpl.txt \
		$(EXTERNALS_SRC)/grill/pool/readme.txt   \
		$(EXTERNALS_SRC)/grill/pool/license.txt \
		$(DOC_DST)/pool
#########
# py/pyext
#	cd $(EXTERNALS_SRC)/grill/py && \
#		( \
#			cp -f build/config-mac.def config.txt \
#		) && \
#		bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(EXTERNALS_DST) FLEXTINC=../flext/source
#	install -p -m0444 $(EXTERNALS_SRC)/grill/py/pd-darwin/release-shared/*.pd_darwin $(EXTERNALS_DST)
# this wasn't building for me -HCS 2005-06-09
	install -d $(DOC_DST)/py/examples
	install -d $(DOC_DST)/py/scripts
	install -p -m0644 $(EXTERNALS_SRC)/grill/py/pd/*.pd $(DOC_DST)/py/examples
	install -p -m0644 $(EXTERNALS_SRC)/grill/py/scripts/*.py $(DOC_DST)/py/scripts
	install -p -m0644 $(EXTERNALS_SRC)/grill/py/gpl.txt  \
		$(EXTERNALS_SRC)/grill/py/readme.txt $(EXTERNALS_SRC)/grill/py/license.txt \
		$(DOC_DST)/py
#	install -d $(INSTALL_PREFIX)/Library/Frameworks
#	cp -R /Library/Frameworks/Python.framework $(INSTALL_PREFIX)/Library/Frameworks
#########
# vasp
#  fails without the dir
#	test -d ../../vasp/pd-darwin || mkdir -p ../../vasp/pd-darwin 
#	cd $(EXTERNALS_SRC)/grill/vasp/ && bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(EXTERNALS_DST) FLEXTINC=../flext/source
#	install -p -m0444 $(EXTERNALS_SRC)/grill/vasp/pd-darwin/release-shared/*.pd_darwin $(EXTERNALS_DST)
# this wasn't building for me -HCS 2005-06-09
	install -p -m0644 $(EXTERNALS_SRC)/grill/vasp/pd-help/*.pd $(DOC_DST)/5.reference
	install -d $(DOC_DST)/vasp
	install -p -m0644 $(EXTERNALS_SRC)/grill/vasp/gpl.txt  \
		$(EXTERNALS_SRC)/grill/vasp/readme.txt   \
		$(EXTERNALS_SRC)/grill/vasp/license.txt  \
		$(EXTERNALS_SRC)/grill/vasp/changes.txt  \
		$(EXTERNALS_SRC)/grill/vasp/todo.txt     \
		$(DOC_DST)/vasp
	cp -rp $(EXTERNALS_SRC)/grill/vasp/pd-ex $(DOC_DST)/vasp
#########
# xsample
	cd $(EXTERNALS_SRC)/grill/xsample && bash ../flext/build.sh pd gcc build-release-shared FLEXTPREFIX=. FLEXTLIB=$(EXTERNALS_DST) FLEXTINC=../flext/source
	install -p -m0444 $(EXTERNALS_SRC)/grill/xsample/pd-darwin/release-shared/*.pd_darwin $(EXTERNALS_DST)
	install -p -m0644 $(EXTERNALS_SRC)/grill/xsample/pd/*.pd $(DOC_DST)/5.reference
	install -d $(DOC_DST)/xsample
	install -p -m0644 $(EXTERNALS_SRC)/grill/xsample/gpl.txt \
		$(EXTERNALS_SRC)/grill/xsample/readme.txt \
		$(EXTERNALS_SRC)/grill/xsample/license.txt \
		$(DOC_DST)/xsample
########## FOOTILS #########
# syncgrain
#	cd $(EXTERNALS_SRC)/footils/syncgrain/ && $(MAKE) -f makefile.pd-darwin
#	install -p -m0444 $(EXTERNALS_SRC)/footils/syncgrain/pd-darwin/*.pd_darwin \
#		$(EXTERNALS_DST)
	install -p -m0644 $(EXTERNALS_SRC)/footils/syncgrain/pd/*.* $(DOC_DST)/5.reference


darwin_app_externals_standard:
#----------------------------------------------------------------------------
# PD-EXTERNALS
	-cd $(EXTERNALS_SRC)/build/darwin && $(MAKE) -k
	install -d $(EXTERNALS_DST)
	install -p -m0644 $(EXTERNALS_SRC)/build/darwin/*.pd_darwin $(EXTERNALS_DST)
# creb
	install -d $(EXTERNALS_DST)/creb
	install -p -m0644	$(EXTERNALS_SRC)/creb/abs/*.pd $(EXTERNALS_DST)/creb
	install -d $(DOC_DST)/5.reference/creb
	install -p -m0644	$(EXTERNALS_SRC)/creb/doc/examples/*.* \
		$(DOC_DST)/5.reference/creb
#----------------------------------------------------------------------------
# GEM
	install -p -m0644 ../../Gem/help/*.* $(DOC_DST)/5.reference
	install -d $(DOC_DST)/Gem/doc
	install -p -m0644 ../../Gem/doc/*.* $(DOC_DST)/gem
	install -d $(DOC_DST)/Gem/00.manual
	install -p -m0644 ../../Gem/manual/*.* $(DOC_DST)/gem/00.manual
	cp -Rfp ../../Gem/examples/*.* ../../Gem/examples/data $(DOC_DST)/gem
#----------------------------------------------------------------------------
# hcs
	install -p -m0644 $(shell ls -1 $(EXTERNALS_SRC)/hcs/*.pd | \
			 grep -v '\-help.pd') $(EXTERNALS_DST)
# hid
	cd $(EXTERNALS_SRC)/hcs/hid && $(MAKE) 
	install -p -m0644 $(EXTERNALS_SRC)/hcs/hid/hid.pd_darwin $(EXTERNALS_DST)
	install -p -m0644 $(shell ls -1 $(EXTERNALS_SRC)/hcs/hid/*.pd | \
			 grep -v '\-help.pd'| grep -v '\-list.pd') $(EXTERNALS_DST)
# general
	install -p -m0644 $(EXTERNALS_SRC)/hcs/general/*.pd $(EXTERNALS_DST)
# pan
	install -p -m0644 $(shell ls -1 $(EXTERNALS_SRC)/hcs/pan/*.pd | \
			 grep -v '\-help.pd') $(EXTERNALS_DST)
#----------------------------------------------------------------------------
# IEMlib
	cp -Rp $(EXTERNALS_SRC)/iemlib/iemabs $(EXTERNALS_DST)
	install -d -m0755 $(DOC_DST)/5.reference/iemabs
	mv  $(EXTERNALS_DST)/iemabs/help-*.pd $(DOC_DST)/5.reference/iemabs
#----------------------------------------------------------------------------
# miXed
	cd $(EXTERNALS_SRC)/miXed/toxy && $(MAKE)
	cd $(EXTERNALS_SRC)/miXed/cyclone && $(MAKE)
	cd $(EXTERNALS_SRC)/miXed/pddp && $(MAKE)
	install -d -m755 $(EXTERNALS_DST)
	install -p -m444 $(EXTERNALS_SRC)/miXed/bin/*.pd_darwin $(EXTERNALS_DST)
	install -p -m555 $(EXTERNALS_SRC)/miXed/bin/cyclist $(INSTALL_PREFIX)/bin
	install -d -m755 $(DOC_DST)/5.reference/cyclone
	install -p -m0644 $(EXTERNALS_SRC)/miXed/test/*/*.*    \
		$(DOC_DST)/5.reference/cyclone
	install -p -m0644 $(EXTERNALS_SRC)/miXed/doc/*/*/*.*  \
		$(DOC_DST)/5.reference
#----------------------------------------------------------------------------
# OSC
	cd $(EXTERNALS_SRC)/OSCx && ./configure && $(MAKE)
	install -p -m0644 $(EXTERNALS_SRC)/OSCx/doc/*.* $(DOC_DST)/5.reference
	install -p -m0444 $(EXTERNALS_SRC)/OSCx/src/*.pd_darwin $(EXTERNALS_DST)
#----------------------------------------------------------------------------
# PdP
#	cd $(EXTERNALS_SRC)/pdp && ./configure && $(MAKE)
#	install -p -m0444 $(EXTERNALS_SRC)/pdp/*.pd_darwin $(EXTERNALS_DST)
	install -p -m0644 $(EXTERNALS_SRC)/pdp/doc/objects/*.* $(DOC_DST)/5.reference
	install -d $(DOC_DST)/pdp/introduction
	install -p -m0644 $(EXTERNALS_SRC)/pdp/doc/introduction/*.* $(DOC_DST)/pdp/introduction
	install -d $(DOC_DST)/pdp/examples
	install -p -m0644 $(EXTERNALS_SRC)/pdp/doc/examples/*.* $(DOC_DST)/pdp/examples
	install -p -m0644 $(EXTERNALS_SRC)/pdp/doc/reference.txt $(DOC_DST)/pdp
# libs for PdP
#----------------------------------------------------------------------------
# unauthorized
	cd $(EXTERNALS_SRC)/unauthorized && $(MAKE) 
	install -p -m0444 $(EXTERNALS_SRC)/unauthorized/*/*.pd_darwin $(EXTERNALS_DST)
	install -p -m0644 $(EXTERNALS_SRC)/unauthorized/*/*.pd $(DOC_DST)/5.reference
	install -p -m0644 $(EXTERNALS_SRC)/unauthorized/*/*.txt $(DOC_DST)/5.reference
	install -p -m0644 $(EXTERNALS_SRC)/unauthorized/*/*.pls $(DOC_DST)/5.reference
	cp -Rfp $(EXTERNALS_SRC)/unauthorized/blinkenlights/blm  $(DOC_DST)/5.reference
#----------------------------------------------------------------------------
# zexy 
# new build system doesn't work with CVS layout, and most of zexy is
# included in the extrnals build system now anyway
#	cd $(EXTERNALS_SRC)/zexy/src && autoconf && ./configure && $(MAKE)
	-install -p -m0444 $(EXTERNALS_SRC)/zexy/src/*.pd_darwin $(EXTERNALS_DST)
	install -p -m0444 \
		$(shell ls -1 $(EXTERNALS_SRC)/zexy/abs/*.pd | \
			grep -v '-help.pd' | sed 's/\([&<>|~]\)/\\\1/g') \
		$(EXTERNALS_DST)
	install -p -m0644	$(EXTERNALS_SRC)/zexy/abs/*-help.pd $(DOC_DST)/5.reference


darwin_app_externals_c++:
#----------------------------------------------------------------------------
# CREB
#	cd $(EXTERNALS_SRC)/creb && autoconf && ./configure && cd modules++ && $(MAKE)
#	install -p -m0644 $(EXTERNALS_SRC)/creb/modules++/*.pd_darwin $(EXTERNALS_DST)
#----------------------------------------------------------------------------
# gem2pdp
#	cd $(EXTERNALS_SRC)/gem2pdp && $(MAKE) -f Makefile.darwin
#	install -p -m0444 $(EXTERNALS_SRC)/gem2pdp/*.pd_darwin $(EXTERNALS_DST)
#----------------------------------------------------------------------------
# GridFlow
#	cd ../../gridflow && ./configure && $(MAKE)


darwin_app_welcome:
	echo "<html><body><p><p>" > $(WELCOME_FILE)
	echo "<CENTER><IMG SRC=\"logo.jpg\">" >> $(WELCOME_FILE)
	echo "<h2>Version $(PD_VERSION)</h2>" >> $(WELCOME_FILE)
	echo "<p>written by Miller S. Puckette</p></CENTER>" >> $(WELCOME_FILE)
	echo "<FONT SIZE=\"-1\">" >> $(WELCOME_FILE)
	echo "<p>`grep -A9 ACKNOWLEDG $(PD_SRC)/README.txt`</p>" >> $(WELCOME_FILE)
	echo "</FONT>" >> $(WELCOME_FILE)
	echo "</body></html>" >> $(WELCOME_FILE)

darwin_app_noncvs:
	test -d noncvs/doc/5.reference && \
		install -p -m0644 noncvs/doc/5.reference/*.* $(DOC_DST)/5.reference
	test -d noncvs/extra && \
		install -p -m0644 noncvs/extra/*.* $(EXTERNALS_DST)
	cp -Rp noncvs/doc/serendipd $(DOC_DST)

darwin_app_perms:
	chmod a-x $(DOC_DST)/*/*.pd $(DOC_DST)/*/*/*.pd $(DOC_DST)/*/*/*/*.pd
	chmod -R a-w $(DOC_DST)
# serendiPd shared patch
	chmod a+w $(DOC_DST)/serendipd/*shared_patch.pd

dmg: darwin_app_readme darwin_app_welcome darwin_app_license darwin_app_perms
	install -d "$(CWD)/$(DMG_NAME)"
	cd $(DESTDIR) && mv $(PD_APP_NAME).app "$(CWD)/$(DMG_NAME)/$(DMG_NAME).app"
	install -p -m0444 $(PD_SRC)/LICENSE.txt "$(CWD)/$(DMG_NAME)/PD LICENSE.txt"
	install -p -m0444 $(LICENSE_FILE) $(README_FILE) \
		$(WELCOME_FILE) $(LICENSE_FILE)   "$(CWD)/$(DMG_NAME)"
	cd $(CWD) && /bin/sh mkdmg "$(DMG_NAME)"


clean: darwin_app_clean darwin_app_externals_standard_clean darwin_pd_clean
clean: darwin_app_externals_gem_clean darwin_app_externals_flext_clean

darwin_pd_clean:
	cd $(PD_SRC)/src/ && make clean

darwin_app_clean:
	-sudo rm -Rf -- $(DESTDIR) "$(DMG_NAME)" 
	-rm -f -- *~ 1 $(README_FILE) $(WELCOME_FILE) $(LICENSE_FILE) \
		$(UNIX_PACKAGE_NAME).pkg  "$(DMG_NAME).dmg"

darwin_unpatch_pd:
# this sed pattern won't work with TEST versions
	cd $(PD_SRC)/src && \
		sed -i.bak 's/\(pd_version\[\] = "Pd version [0-9.]*\)[0-9extndRC.-]*/\1/' \
			s_main.c && \
		rm -- *.bak
	cd $(PD_SRC)/src/ && patch -R -p0 < \
		../../packages/darwin_app/patches/darwin_linking_fixes.patch
	cd $(PD_SRC)/src/ && patch -R -p0 < \
		../../packages/darwin_app/patches/extended-help-menu.patch
	cd $(PD_SRC)/src/ && patch -R -p0 < \
		../../packages/darwin_app/patches/display_tweaks.patch

darwin_app_externals_standard_clean:
	cd $(EXTERNALS_SRC)/build/darwin && make clean
	cd $(EXTERNALS_SRC)/build/doc && make clean
	cd $(EXTERNALS_SRC)/hcs/hid && make clean

darwin_app_externals_gem_clean:
	cd ../../Gem && pbxbuild clean

darwin_app_externals_flext_clean:
	cd $(EXTERNALS_SRC)/footils/syncgrain && make -f makefile.pd-darwin clean
	cd $(EXTERNALS_SRC)/grill && \
		rm -- flext/buildsys/config-mac-pd-gcc.txt config.txt flext/config.txt \
			flext/source/libflext.a */pd-darwin/*.o */pd-darwin/*.pd_darwin \
			*/pd-darwin/*/*.pd_darwin

