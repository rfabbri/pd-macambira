
current: dmg

#
# Fetch the versions of the various included packages

PD_MAJOR_VERSION = $(shell grep PD_MAJOR_VERSION ../pd/src/m_pd.h | cut -d ' ' -f 3)
PD_MINOR_VERSION = $(shell grep PD_MINOR_VERSION ../pd/src/m_pd.h | cut -d ' ' -f 3)
PD_VERSION = $(PD_MAJOR_VERSION).$(PD_MINOR_VERSION)

CYCLONE_MAJOR_VERSION = $(shell grep CYCLONE_VERSION ../externals/miXed/cyclone/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
CYCLONE_RELEASE = $(shell grep CYCLONE_RELEASE ../externals/miXed/cyclone/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
CYCLONE_BUILD = $(shell grep CYCLONE_BUILD ../externals/miXed/cyclone/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
CYCLONE_VERSION = $(CYCLONE_MAJOR_VERSION) $(CYCLONE_RELEASE) $(CYCLONE_BUILD)

ZEXY_VERSION = $(shell grep VERSION ../externals/zexy/src/zexy.h | cut -d ' ' -f 3 | cut -d '"' -f 2)

# various files

PACKAGE_VERSION = $(shell date +20%y.%m.%d)
PACKAGE_PREFIX = Pure Data
PACKAGE_NAME = $(PACKAGE_PREFIX)

DMG_PREFIX = Pure Data Installer
DMG_NAME = $(DMG_PREFIX) $(PD_VERSION)-$(PACKAGE_VERSION)

WELCOME_FILE = Welcome.html
README_FILE = ReadMe.html
INFO_FILE = $(PACKAGE_PREFIX).info

clean: darwin_mpkg_clean
	-cd ../pd/src && make clean
	cd ../externals/build/darwin && make clean
	cd ../externals/zexy/src && make -f makefile.darwin clean
	cd ../externals/miXed/cyclone && make clean
	cd ../doc/pddp && make clean
	cd ../externals/unauthorized && make clean
	-cd ../externals/sprinkler && make distclean

darwin_mpkg_clean:
	-sudo rm -Rf installroot
	-sudo rm -Rf "$(DMG_PREFIX)"*
	-rm -f "$(README_FILE)" *.dmg *~
	-cd ../pd/src && make darwin_pkg_clean
	cd ../externals/build/darwin && make darwin_pkg_clean
	cd ../externals/zexy/src && make -f makefile.darwin darwin_pkg_clean
	cd ../externals/miXed/cyclone && make darwin_pkg_clean
	cd ../doc/  && make darwin_pkg_clean
	cd ../doc/pddp  && make darwin_pkg_clean
	cd ../externals/unauthorized && make darwin_pkg_clean
	cd ../abstractions && make darwin_pkg_clean
	cd noncvs && make darwin_pkg_clean

darwin_mpkg_welcome:
	echo "<HTML><BODY><P><P>" > $(WELCOME_FILE)
	echo "<CENTER><IMG SRC=\"logo.jpg\">" >> $(WELCOME_FILE)
	echo "<H2>Version $(PD_VERSION)</H2>" >> $(WELCOME_FILE)
	echo "<P>written by Miller S. Puckette</P></CENTER>" >> $(WELCOME_FILE)
	echo "<FONT SIZE=\"-1\">" >> $(WELCOME_FILE)
	echo "<P>`grep -A9 ACKNOWLEDG ../pd/README.txt`</P>" >> $(WELCOME_FILE)
	echo "</FONT>" >> $(WELCOME_FILE)
	echo "</BODY></HTML>" >> $(WELCOME_FILE)

darwin_mpkg_readme:
	echo $(CYCLONE_RELEASE)
	-rm $(README_FILE)
	echo "<HTML><BODY>" > $(README_FILE)
	echo "<H2><IMG SRC=\"pd-16.png\">&nbsp;Pure Data Installer $(PD_VERSION)-$(PACKAGE_VERSION)</H2>" >> $(README_FILE)
	echo "<P>Pd is a free real-time computer music software package resembling Max.  It provides a patchable environment for audio analysis, synthesis, and processing, with a rich set of multimedia capabilities.  You can get Pd for Linux, Windows, MacOS X, BSD, or IRIX.</P>" >> $(README_FILE)
	echo "<P>For more information, go to: http://www.pure-data.org</P>" >> $(README_FILE)
	echo "<H3>Included Versions</H3>" >> $(README_FILE)	
#	echo "<P>The various versions of the included packages:</P>" >> $(README_FILE)
	echo "<UL>" >> $(README_FILE)
	echo "<LI>pure data: $(PD_VERSION) " >> $(README_FILE)
	echo "<LI>cyclone: $(CYCLONE_VERSION)" >> $(README_FILE)
	echo "<LI>pd-abstractions: `date +20%y.%m.%d`" >> $(README_FILE)
	echo "<LI>pd-doc: `date +20%y.%m.%d`" >> $(README_FILE)
	echo "<LI>pd-externals: `date +20%y.%m.%d`" >> $(README_FILE)
	echo "<LI>pddp: `date +20%y.%m.%d`" >> $(README_FILE)
	echo "<LI>unauthorized: `date +20%y.%m.%d`" >> $(README_FILE)
	echo "<LI>zexy: $(ZEXY_VERSION)" >> $(README_FILE)
	echo "</UL>" >> $(README_FILE)
	echo "(this package was built on `date`) <BR>" >> $(README_FILE)
	echo "</BODY></HTML>" >> $(README_FILE)


darwin_mpkg: darwin_mpkg_readme darwin_mpkg_welcome
	test -d installroot/Packages || mkdir -p installroot/Packages
	test -d "installroot/$(PACKAGE_NAME).mpkg/Contents/Resources" \
		|| mkdir -p "installroot/$(PACKAGE_NAME).mpkg/Contents/Resources"
# pd
	cd ../pd/src && ./configure && make darwin_pkg
	sudo cp -R ../pd/pd-*.pkg installroot/Packages
# pd-externals
	cd ../externals/build/darwin && make darwin_pkg
	sudo cp -R ../externals/build/darwin/pd-externals*.pkg installroot/Packages
# zexy
	cd ../externals/zexy/src && make -f makefile.darwin darwin_pkg
	sudo cp -R ../externals/zexy/src/pd-zexy*.pkg installroot/Packages
# cyclone
	cd ../externals/miXed/cyclone && make darwin_pkg
	sudo cp -R ../externals/miXed/cyclone/pd-cyclone*.pkg installroot/Packages
# pd-abstractions
	cd ../abstractions/  && make darwin_pkg
	sudo cp -R ../abstractions/pd-abstractions*.pkg installroot/Packages
# pd-doc
	cd ../doc/  && make darwin_pkg
	sudo cp -R ../doc/pd-doc*.pkg installroot/Packages
# pd-noncvs
	cd noncvs && make darwin_pkg
	sudo cp -R noncvs/pd-noncvs*.pkg installroot/Packages
# pddp
	cd ../doc/pddp  && make darwin_pkg
	sudo cp -R ../doc/pddp/pd-pddp*.pkg installroot/Packages
# unauthorized
	cd ../externals/unauthorized && make darwin_pkg
	sudo cp -R ../externals/unauthorized/pd-unauthorized*.pkg installroot/Packages
# generate pd.list
	cd installroot/Packages && /bin/ls -1d *.pkg \
		| sed -e 's/\(.*\)/\1\:Selected/' \
		> "../$(PACKAGE_NAME).mpkg/Contents/Resources/$(PACKAGE_NAME).list"
# generate .info file
	sed -e 's/PACKAGE_PREFIX/$(PACKAGE_PREFIX)/' pd.info \
		| sed -e 's/PACKAGE_VERSION/$(PACKAGE_VERSION)/' \
		| sed -e 's/PD_VERSION/$(PD_VERSION)/' \
		> "installroot/$(PACKAGE_NAME).mpkg/Contents/Resources/$(INFO_FILE)"
# generate Description.plist
	sed -e 's/PACKAGE_PREFIX/$(PACKAGE_PREFIX)/' Description.plist \
		| sed -e 's/PACKAGE_VERSION/$(PACKAGE_VERSION)/' \
		| sed -e 's/PD_VERSION/$(PD_VERSION)/' \
		> "installroot/$(PACKAGE_NAME).mpkg/Contents/Resources/Description.plist"
# generate Info.plist
	sed -e 's/PACKAGE_PREFIX/$(PACKAGE_PREFIX)/' Info.plist \
		| sed -e 's/PACKAGE_VERSION/$(PACKAGE_VERSION)/' \
		| sed -e 's/PD_VERSION/$(PD_VERSION)/' \
		> "installroot/$(PACKAGE_NAME).mpkg/Contents/Info.plist"
# install files
	install -m644 ../pd/LICENSE.txt "installroot/PD LICENSE.txt"
	install -m644 ../externals/creb/COPYING "installroot/GNU GPL.txt"
	install -m644 $(README_FILE) $(WELCOME_FILE) \
		logo.jpg pd-32.png pd-16.png \
		background.tiff \
		"installroot/$(PACKAGE_NAME).mpkg/Contents/Resources/"
	sudo chmod -R u+w installroot
	sudo chmod -R a+r installroot
	sudo chmod -R go-w installroot
	sudo chgrp -R staff installroot

dmg: darwin_mpkg_clean darwin_mpkg
	mv installroot "$(DMG_NAME)"
	/bin/sh mkdmg "$(DMG_NAME)"

