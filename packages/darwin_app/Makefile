
current: darwin_app

PD_APP_NAME = Pd

DESTDIR = installroot
PD_APP_CONTENTS = /Applications/$(PD_APP_NAME).app/Contents
INSTALL_PREFIX = $(PD_APP_CONTENTS)/Resources

WISH_SHELL_CONTENTS = /Applications/Utilities/Wish\ Shell.app/Contents

PD_FILE_ROOT = $(DESTDIR)$(INSTALL_PREFIX)/lib/pd
ABSTRACTIONS_DIR = $(PD_FILE_ROOT)/doc/abstractions
EXTERNALS_DIR = $(PD_FILE_ROOT)/extra
HELP_DIR = $(PD_FILE_ROOT)/doc

# needed for PackageMaker.app since it needs absolute paths
PWD = $(shell pwd)

PD_MAJOR_VERSION = $(shell /usr/bin/grep 'Pd version' ../../pd/src/s_main.c | /usr/bin/cut -d '\\' -f 1 | /usr/bin/cut -d ' ' -f 6 | /usr/bin/cut -d '.' -f 1)
PD_MINOR_VERSION = $(shell /usr/bin/grep 'Pd version' ../../pd/src/s_main.c | /usr/bin/cut -d '\\' -f 1 | /usr/bin/cut -d ' ' -f 6 | /usr/bin/cut -d '.' -f 2)
PD_BUGFIX_VERSION = $(shell /usr/bin/grep 'Pd version' ../../pd/src/s_main.c | /usr/bin/cut -d '\\' -f 1 | /usr/bin/cut -d ' ' -f 6 | /usr/bin/cut -d '.' -f 3)
PD_VERSION = $(PD_MAJOR_VERSION).$(PD_MINOR_VERSION).$(PD_BUGFIX_VERSION)

# release version for this distro
PACKAGE_VERSION = beta3
PACKAGE_NAME = pd-$(PD_VERSION)-$(PACKAGE_VERSION)
DMG_NAME = $(PACKAGE_NAME)


UNIX_PACKAGE_NAME = $(PD_APP_NAME)-UNIX-$(PD_VERSION)-$(PACKAGE_VERSION)
UNIX_prefix = /usr/local
UNIX_DESTDIR = pd-unix-root


WELCOME_FILE = Welcome.html
README_FILE = ReadMe.html
LICENSE_FILE = License.html


CYCLONE_MAJOR_VERSION = $(shell grep CYCLONE_VERSION ../../externals/miXed/cyclone/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
CYCLONE_RELEASE = $(shell grep CYCLONE_RELEASE ../../externals/miXed/cyclone/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
CYCLONE_BUILD = $(shell grep CYCLONE_BUILD ../../externals/miXed/cyclone/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
CYCLONE_VERSION = $(CYCLONE_MAJOR_VERSION) $(CYCLONE_RELEASE) $(CYCLONE_BUILD)

FLEXT_VERSION = $(shell grep PROJECT_NUMBER ../../externals/grill/flext/flext.doxy | cut -d '"' -f2 | cut -d ' ' -f2)

GEM_VERSION = $(shell grep "GEM_VERSION" ../../Gem/src/Base/GemVersion.h | cut -d '"' -f 2)

MAXLIB_VERSION = $(shell grep "#define VERSION" ../../externals/maxlib/maxlib.c | cut -d '"' -f 2)

OSC_VERSION = $(shell grep "#define VERSION" ../../externals/OSCx/OSC.c | cut -d '"' -f 2)

PDP_VERSION = $(shell grep PDP_VERSION= ../../externals/pdp/configure | cut -d '=' -f 2)

PMPD_VERSION = $(shell grep "#define VERSION" ../../externals/pmpd/src/pmpd.c | cut -d '"' -f 2)

TOXY_MAJOR_VERSION = $(shell grep TOXY_VERSION ../../externals/miXed/toxy/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
TOXY_RELEASE = $(shell grep TOXY_RELEASE ../../externals/miXed/toxy/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
TOXY_BUILD = $(shell grep TOXY_BUILD ../../externals/miXed/toxy/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
TOXY_VERSION = $(TOXY_MAJOR_VERSION) $(TOXY_RELEASE) $(TOXY_BUILD)

ZEXY_VERSION = $(shell grep VERSION ../../externals/zexy/src/zexy.h | cut -d ' ' -f 3 | cut -d '"' -f 2)


darwin_app: darwin_app_wrapper darwin_app_core darwin_unix_pkg darwin_app_externals \
		darwin_app_docs darwin_app_abstractions


darwin_app_license:
  # generate HTML version of License
	echo "<HTML><BODY>" > $(LICENSE_FILE)
	echo "<H3>(Parts of this package can be used under the Pd/BSD license)</H3>" >> $(LICENSE_FILE)
	echo "<FONT SIZE=\"-1\">" >> $(LICENSE_FILE)
	cat ../../externals/creb/COPYING | sed -e 's/^$$/\<P\>/g' >> $(LICENSE_FILE)	
	echo "</FONT></BODY></HTML>" >> $(LICENSE_FILE)


darwin_app_readme:
	echo $(CYCLONE_RELEASE)
	-rm $(README_FILE)
	echo "<html>" > $(README_FILE)
	echo "<head>" >> $(README_FILE)
	echo "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=iso-8859-1\" />" >> $(README_FILE)
	echo "</head>" >> $(README_FILE)
	echo "<body>" >> $(README_FILE)
	echo "<h2><IMG SRC=\"pd-16.png\">&nbsp;Pure Data $(PD_VERSION)-$(PACKAGE_VERSION)</h2>" 	>> $(README_FILE)
	echo "<p>Pd is a free real-time computer music software package resembling Max.  It provides a patchable environment for audio analysis, synthesis, and processing, with a rich set of multimedia capabilities.  You can get Pd for Linux, Windows, MacOS X, BSD, or IRIX.</p>" >> $(README_FILE)
	echo "<p>For more information, go to: http://www.puredata.org</p>" >> $(README_FILE)
	echo "<h3>Installation</h3>" >> $(README_FILE)	
	echo "<p>To install Pd, drag the Pd.app to anywhere in your hard disk.</p>" >> $(README_FILE)	
	echo "<h3>Pure Data CVS Developers</h3>" >> $(README_FILE)	
# this may seem whack, but it generates the list of developers from the SourceForge site:
	wget -q -O - 'http://sourceforge.net/project/memberlist.php?group_id=55736' | grep -e '<td>' | sed -e :a -e 's/<[^>]*>//g;/</N;//ba' | grep -v '^$$' | sed -e 's/\t*/ - /' >> $(README_FILE)
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
#	echo "<li>maxlib: $(MAXLIB_VERSION)" >> $(README_FILE)
#	echo "<li>osc: $(OSC_VERSION)" >> $(README_FILE)
#	echo "<li>pmpd: $(PMPD_VERSION)" >> $(README_FILE)
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


darwin_app_wrapper:
# copy Wish Shell.app from default install location
# note: use the "standalone" Wish shell to make a "standalone" pd app
	install -d "$(DESTDIR)$(PD_APP_CONTENTS)/MacOS"
	install -m0755 -p \
		/Applications/Utilities/Wish\ Shell.app/Contents/MacOS/Wish\ Shell \
		"$(DESTDIR)$(PD_APP_CONTENTS)/MacOS"
	install -d "$(DESTDIR)$(PD_APP_CONTENTS)/Resources"
	install -m0644 -p \
		/Applications/Utilities/Wish\ Shell.app/Contents/Resources/Wish\ Shell.rsrc \
		"$(DESTDIR)$(PD_APP_CONTENTS)/Resources"
#	install -d "$(DESTDIR)$(PD_APP_CONTENTS)/Frameworks"
	cp -Rp \
		/Applications/Utilities/Wish\ Shell.app/Contents/Frameworks \
		"$(DESTDIR)$(PD_APP_CONTENTS)"
	cp -Rp \
                /Applications/Utilities/Wish\ Shell.app/Contents/Frameworks \
                "$(DESTDIR)$(PD_APP_CONTENTS)/Resources"

#	install -d "$(DESTDIR)$(PD_APP_CONTENTS)/Frameworks"  \
#	"$(DESTDIR)$(PD_APP_CONTENTS)/Resources"
# set up app wrapper
	install -d "$(DESTDIR)$(PD_APP_CONTENTS)/Resources/Scripts"
	install -m0644 -p Info.plist "$(DESTDIR)$(PD_APP_CONTENTS)"
	#install -m0644 -p Pd.term "$(DESTDIR)$(PD_APP_CONTENTS)/Resources"
	install -m0644 -p *.icns  "$(DESTDIR)$(PD_APP_CONTENTS)/Resources"
# using the shell script hack until AppMain.tcl works
	install -m0644 -p AppMain.tcl "$(DESTDIR)$(PD_APP_CONTENTS)/Resources/Scripts"
	#  install -m0755 -p Pd "$(DESTDIR)$(PD_APP_CONTENTS)/MacOS"
# rename the Wish Shell executable to "Pd"
	mv "$(DESTDIR)$(PD_APP_CONTENTS)/MacOS/Wish Shell" "$(DESTDIR)$(PD_APP_CONTENTS)/MacOS/Pd"


# INSTALL_PREFIX=$(INSTALL_PREFIX) is for pd-MSP 0.37.1
# prefix=$(INSTALL_PREFIX) is for pd-CVS 0.37
darwin_app_core:
# apply some patches 
	cd ../../pd/src/ && patch -p0<../../packages/darwin_app/patches/socket.patch
	cd ../../pd/src/ && patch -p0<../../packages/darwin_app/patches/makefile.in.patch
	cd ../../pd/src/ && patch -p0<../../packages/darwin_app/patches/u_main.tk.patch
	cd ../../pd/src/ && patch -p0<../../packages/darwin_app/patches/configure.jackosx.patch
	cd ../../pd/src/ && patch -p0<../../packages/darwin_app/patches/s_audio_jack.patch
# pd core
	cd ../../pd/src/ && ./configure --enable-jack &&\
		make install \
			DESTDIR=../../packages/darwin_app/$(DESTDIR) \
			INSTALL_PREFIX=$(INSTALL_PREFIX)

darwin_app_docs:
#----------------------------------------------------------------------------
# DOC
	install -d $(HELP_DIR)/5.reference
	install -d $(HELP_DIR)/7.stuff
	cp -Rp ../../doc/tutorials $(HELP_DIR)
	cp -Rp ../../doc/additional/pd-msg $(HELP_DIR)/7.stuff/
#----------------------------------------------------------------------------
# PDDP
	install -m644 --group=staff ../../doc/pddp/*.pd $(HELP_DIR)/5.reference

# DOC menus
	install -d $(HELP_DIR)/menus
	cp -p ./doc-menu.pd $(HELP_DIR)/menus
	./mkdocs $(HELP_DIR)/2.control.examples>$(HELP_DIR)/menus/2.control.examples.pd
	./mkdocs $(HELP_DIR)/3.audio.examples>$(HELP_DIR)/menus/3.audio.examples.pd
	./mkdocs $(HELP_DIR)/4.fft.examples>$(HELP_DIR)/menus/4.fft.examples.pd
	./mkdocs $(HELP_DIR)/5.reference>$(HELP_DIR)/menus/5.reference.pd

darwin_app_abstractions:
#----------------------------------------------------------------------------
# ABSTRACTIONS
	install -d $(ABSTRACTIONS_DIR)
	install -p -m0644	../../externals/creb/abs/*.pd $(ABSTRACTIONS_DIR)
	cd $(ABSTRACTIONS_DIR) && rm count.pd
	install -d $(ABSTRACTIONS_DIR)/creb
	install -p -m0444	../../externals/creb/doc/examples/*.* $(ABSTRACTIONS_DIR)/creb
#	cp -Rf footils timestretch vadsr~  $(ABSTRACTIONS_DIR)

darwin_app_externals: darwin_app_externals_flext darwin_app_externals_standard darwin_app_externals_c++

darwin_app_externals_flext:
#----------------------------------------------------------------------------
# FLEXT
	
	cd ../../externals/grill/build/darwin && make 
	install -d $(HELP_DIR)/tutorials
	cp -rp ../../externals/grill/flext/tutorial $(HELP_DIR)/tutorials/flext
	install -d $(HELP_DIR)/flext
	install -p -m0444 ../../externals/grill/flext/gpl.txt  \
		../../externals/grill/flext/readme.txt   \
		../../externals/grill/flext/license.txt  \
		$(HELP_DIR)/flext
# FLEXTERNALS
# dyn
#	install -p -m0444 ../../externals/grill/dyn/pd-darwin/*.pd_darwin $(EXTERNALS_DIR)
	install -p -m0444 ../../externals/grill/dyn/pd/*.pd $(HELP_DIR)/5.reference
	install -d $(HELP_DIR)/dyn
	install -p -m0444 ../../externals/grill/dyn/gpl.txt  \
		../../externals/grill/dyn/readme.txt   \
		../../externals/grill/dyn/license.txt  \
		$(HELP_DIR)/dyn
# fftease
#	install -p -m0444 ../../externals/grill/fftease/pd-darwin/*.pd_darwin $(EXTERNALS_DIR)
	install -p -m0444 ../../externals/grill/fftease/pd/*.pd $(HELP_DIR)/5.reference
	install -d $(HELP_DIR)/fftease
	install -p -m0444 ../../externals/grill/fftease/gpl.txt  \
		../../externals/grill/fftease/readme.txt \
		../../externals/grill/fftease/license.txt $(HELP_DIR)/fftease
	cp -rp ../../externals/grill/fftease/media $(HELP_DIR)/fftease
# idelay
#	install -p -m0444 ../../externals/grill/idelay/pd-darwin/*.pd_darwin $(EXTERNALS_DIR)
	install -p -m0444 ../../externals/grill/idelay/pd/*.pd $(HELP_DIR)/5.reference
	install -d $(HELP_DIR)/idelay
	install -p -m0444 ../../externals/grill/idelay/gpl.txt \
		../../externals/grill/idelay/license.txt \
		$(HELP_DIR)/idelay
# pool
	install -p -m0444 ../../externals/grill/pool/pd-darwin/*.pd_darwin $(EXTERNALS_DIR)
	install -p -m0444 ../../externals/grill/pool/help-pool.pd $(HELP_DIR)/5.reference
	install -d $(HELP_DIR)/pool
	install -p -m0444 ../../externals/grill/pool/gpl.txt \
		../../externals/grill/pool/readme.txt   \
		../../externals/grill/pool/license.txt \
		$(HELP_DIR)/pool
# py/pyext
#	install -p -m0444 ../../externals/grill/py/pd-darwin/*.pd_darwin $(EXTERNALS_DIR)
	install -d $(HELP_DIR)/py/examples
	install -d $(HELP_DIR)/py/scripts
	install -p -m0444 ../../externals/grill/py/pd/*.pd $(HELP_DIR)/py/examples
	install -p -m0444 ../../externals/grill/py/scripts/*.py $(HELP_DIR)/py/scripts
	install -p -m0444 ../../externals/grill/py/gpl.txt  \
		../../externals/grill/py/readme.txt ../../externals/grill/py/license.txt \
		$(HELP_DIR)/py
#	install -d $(INSTALL_PREFIX)/Library/Frameworks
#	cp -R /Library/Frameworks/Python.framework $(INSTALL_PREFIX)/Library/Frameworks
# vasp
#  fails without the dir
#	test -d ../../vasp/pd-darwin || mkdir -p ../../vasp/pd-darwin 
#	install -p -m0444 ../../externals/grill/vasp/pd-darwin/*.pd_darwin $(EXTERNALS_DIR)
	install -p -m0444 ../../externals/grill/vasp/pd-help/*.pd $(HELP_DIR)/5.reference
	install -d $(HELP_DIR)/vasp
	install -p -m0444 ../../externals/grill/vasp/gpl.txt  \
		../../externals/grill/vasp/readme.txt   \
		../../externals/grill/vasp/license.txt  \
		../../externals/grill/vasp/changes.txt  \
		../../externals/grill/vasp/todo.txt     \
		$(HELP_DIR)/vasp
	cp -rp ../../externals/grill/vasp/pd-ex $(HELP_DIR)/vasp
# vst
#	install -p -m0444 ../../externals/grill/vst/pd-darwin/*.pd_darwin $(EXTERNALS_DIR)
	install -p -m0444 ../../externals/grill/vst/pd/*.pd $(HELP_DIR)/5.reference
	install -d $(HELP_DIR)/vst
	install -p -m0444 ../../externals/grill/vst/gpl.txt  \
		../../externals/grill/vst/readme.txt \
		../../externals/grill/vst/license.txt                \
		$(HELP_DIR)/vst
# xsample
	install -p -m0444 ../../externals/grill/xsample/pd-darwin/*.pd_darwin $(EXTERNALS_DIR)
	install -p -m0444 ../../externals/grill/xsample/pd/*.pd $(HELP_DIR)/5.reference
	install -d $(HELP_DIR)/xsample
	install -p -m0444 ../../externals/grill/xsample/gpl.txt \
		../../externals/grill/xsample/readme.txt \
		../../externals/grill/xsample/license.txt \
		$(HELP_DIR)/xsample

darwin_app_externals_standard:
#----------------------------------------------------------------------------
# PD-EXTERNALS
	cd ../../externals/build/darwin && make
	install -d $(EXTERNALS_DIR)
	install -p -m0644 ../../externals/build/darwin/*.pd_darwin $(EXTERNALS_DIR)
#----------------------------------------------------------------------------
# GEM
	install -p -m0444 ../../Gem/help/*.* $(HELP_DIR)/5.reference
	install -d $(HELP_DIR)/Gem/doc
	install -p -m0444 ../../Gem/doc/*.* $(HELP_DIR)/gem
	install -d $(HELP_DIR)/Gem/00.manual
	install -p -m0444 ../../Gem/manual/*.* $(HELP_DIR)/gem/00.manual
	cp -Rfp ../../Gem/examples/*.* ../../Gem/examples/data $(HELP_DIR)/gem
#----------------------------------------------------------------------------
# miXed
	cd ../../externals/miXed/toxy && make
	cd ../../externals/miXed/cyclone && make
	install -p -m444 ../../externals/miXed/bin/*.pd_darwin $(EXTERNALS_DIR)
	sudo install -p -m555 ../../externals/miXed/bin/cyclist $(UNIX_DESTDIR)$(UNIX_prefix)/bin
	install -p -m0444 ../../externals/miXed/test/*/*.*    \
		../../externals/miXed/doc/*/*/*.*   \
		$(HELP_DIR)/5.reference
#----------------------------------------------------------------------------
# OSC
	cd ../../externals/OSCx && ./configure && make
	install -d  $(UNIX_DESTDIR)/$(UNIX_prefix)/bin
	sudo install -p -m0755 ../../externals/OSCx/send+dump/????OSC $(UNIX_DESTDIR)$(UNIX_prefix)/bin
	install -p -m0444 ../../externals/OSCx/doc/*.* $(HELP_DIR)/5.reference
	install -p -m0444 ../../externals/OSCx/src/*.pd_darwin $(EXTERNALS_DIR)
#----------------------------------------------------------------------------
# PDP
	cd ../../externals/pdp && ./configure && make
	install -p -m0444 ../../externals/pdp/*.pd_darwin $(EXTERNALS_DIR)
	install -p -m0444 ../../externals/pdp/doc/objects/*.* $(HELP_DIR)/5.reference
	install -d $(HELP_DIR)/pdp/introduction
	install -p -m0444 ../../externals/pdp/doc/introduction/*.* $(HELP_DIR)/pdp/introduction
	install -d $(HELP_DIR)/pdp/examples
	install -p -m0444 ../../externals/pdp/doc/examples/*.* $(HELP_DIR)/pdp/examples
	install -p -m0444 ../../externals/pdp/doc/reference.txt $(HELP_DIR)/pdp
#----------------------------------------------------------------------------
# PMPD
	cd ../../externals/pmpd/src && make pd_darwin
	install -p -m0444 ../../externals/pmpd/src/*.pd_darwin $(EXTERNALS_DIR)
	install -p -m0444 ../../externals/pmpd/help/*.pd $(HELP_DIR)/5.reference
	install -d $(HELP_DIR)/pmpd
	install -p -m0444 ../../externals/pmpd/exemples/*.pd $(HELP_DIR)/pmpd
#----------------------------------------------------------------------------
# unauthorized
	cd ../../externals/unauthorized && make 
	install -p -m0444 ../../externals/unauthorized/*/*.pd_darwin $(EXTERNALS_DIR)
	install -p -m0444 ../../externals/unauthorized/*/*.pd $(HELP_DIR)/5.reference
	install -p -m0444 ../../externals/unauthorized/*/*.txt $(HELP_DIR)/5.reference
	install -p -m0444 ../../externals/unauthorized/*/*.pls $(HELP_DIR)/5.reference
	cp -Rfp ../../externals/unauthorized/blinkenlights/blm  $(HELP_DIR)/5.reference
#----------------------------------------------------------------------------
# zexy
	cd ../../externals/zexy/src && make -f makefile.darwin
	install -p -m0444 ../../externals/zexy/src/*.pd_darwin $(EXTERNALS_DIR)
	install -p -m0444 ../../externals/zexy/zexy.pd  $(HELP_DIR)/5.reference
	install -d $(HELP_DIR)/help-zexy
	install -p -m0444 ../../externals/zexy/examples/*.* $(HELP_DIR)/help-zexy


darwin_app_externals_c++:
#----------------------------------------------------------------------------
# CREB
#	cd ../../externals/creb && autoconf && ./configure && cd modules++ && make
#	install -p -m0644 ../../externals/creb/modules++/*.pd_darwin $(EXTERNALS_DIR)
#----------------------------------------------------------------------------
# gem2pdp
#	cd ../../externals/gem2pdp && make -f Makefile.darwin
#	install -p -m0444 ../../externals/gem2pdp/*.pd_darwin $(EXTERNALS_DIR)
	install -p -m0444 ../../externals/gem2pdp/*.pd $(HELP_DIR)/5.reference
#----------------------------------------------------------------------------
# GridFlow
#	cd ../../gridflow && ./configure && make


#darwin_unix_pkg: darwin_app_readme darwin_unix_pkg_welcome darwin_app_license
darwin_unix_pkg:
# mk the unix dir
	install -d $(UNIX_DESTDIR)$(UNIX_prefix)
# copy stuff to UNIX dirs
	sudo cp -Rp $(DESTDIR)$(INSTALL_PREFIX)/bin \
		$(DESTDIR)$(INSTALL_PREFIX)/include \
		$(DESTDIR)$(INSTALL_PREFIX)/man \
		$(UNIX_DESTDIR)$(UNIX_prefix)
#	cd .. && install -p darwin_app/pd-unix.info $(UNIX_PACKAGE_NAME).info
# cp text for pkg
	cp $(WELCOME_FILE) $(README_FILE) $(LICENSE_FILE) pkg
# generate Description.plist
	sed -e 's/PACKAGE_PREFIX/$(PACKAGE_PREFIX)/' pkg/Description.plist.template \
		| sed -e 's/PACKAGE_VERSION/$(PACKAGE_VERSION)/' \
		| sed -e 's/PD_VERSION/$(PD_VERSION)/' \
		> pkg/Description.plist
# generate Info.plist
	sed -e 's/PACKAGE_PREFIX/$(PACKAGE_PREFIX)/' pkg/Info.plist.template \
		| sed -e 's/PACKAGE_VERSION/$(PACKAGE_VERSION)/' \
		| sed -e 's/PD_VERSION/$(PD_VERSION)/' \
		> pkg/Info.plist
# delete cruft
	-find $(UNIX_DESTDIR) -name .DS_Store -delete
	-rm -f 1
# set proper permissions
	sudo chown -R root:staff $(UNIX_DESTDIR)/$(UNIX_prefix)
# build package
	/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker \
		-build \
		-p $(PWD)/$(UNIX_PACKAGE_NAME).pkg \
		-f $(PWD)/$(UNIX_DESTDIR) \
		-r $(PWD)/pkg \
		-i $(PWD)/pkg/Info.plist		\
		-d $(PWD)/Description.plist
# generate .info file
#	 sed -e 's/PACKAGE_PREFIX/$(PACKAGE_PREFIX)/' pkg/pd.info \
#		| sed -e 's/PACKAGE_VERSION/$(PACKAGE_VERSION)/' \
#		| sed -e 's/PD_VERSION/$(PD_VERSION)/' \
#		> $(UNIX_PACKAGE_NAME).pkg/Contents/Resources
# install pkg docs
#	sudo install -m 644 $(LICENSE_FILE) $(UNIX_PACKAGE_NAME).pkg/Contents/Resources
#	sudo chown -R root:staff $(UNIX_PACKAGE_NAME).pkg/Contents/Resources


darwin_unix_pkg_welcome:
	echo "<html><body><p><p>" > $(WELCOME_FILE)
	echo "<CENTER><IMG SRC=\"logo.jpg\">" >> $(WELCOME_FILE)
	echo "<h2>Version $(PD_VERSION)</h2>" >> $(WELCOME_FILE)
	echo "<p>written by Miller S. Puckette</p></CENTER>" >> $(WELCOME_FILE)
	echo "<FONT SIZE=\"-1\">" >> $(WELCOME_FILE)
	echo "<p>`grep -A9 ACKNOWLEDG ../../pd/README.txt`</p>" >> $(WELCOME_FILE)
	echo "</FONT>" >> $(WELCOME_FILE)
	echo "</body></html>" >> $(WELCOME_FILE)


dmg: 
	install -d "$(DMG_NAME)"
#	mv $(UNIX_PACKAGE_NAME).pkg "$(DMG_NAME)"
	cd $(DESTDIR)/Applications && mv $(PD_APP_NAME).app "../../$(DMG_NAME)"
	install -p -m0444 ../../pd/LICENSE.txt "$(DMG_NAME)/PD LICENSE.txt"
	install -p -m0444  pd-16.png $(LICENSE_FILE) $(README_FILE) \
		$(WELCOME_FILE) $(LICENSE_FILE) \
		"$(DMG_NAME)"
	/bin/sh mkdmg "$(DMG_NAME)"


clean: darwin_app_clean


darwin_app_clean:
	-sudo rm -Rf $(DESTDIR) $(UNIX_DESTDIR) "$(DMG_NAME)" $(UNIX_PACKAGE_NAME) 
	-rm -f *~ 1 $(README_FILE) $(WELCOME_FILE) $(LICENSE_FILE) \
		$(UNIX_PACKAGE_NAME).pkg  "$(DMG_NAME).dmg"
	cd patches && ./unpatch
