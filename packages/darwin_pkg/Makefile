
current: darwin_mpkg

#
# Fetch the versions of the various included packages

PD_MAJOR_VERSION = $(shell grep 'Pd version' s_main.c | cut -d '\\' -f 1 | cut -d ' ' -f 6 | cut -d '.' -f 1)
PD_MINOR_VERSION = $(shell grep 'Pd version' s_main.c | cut -d '\\' -f 1 | cut -d ' ' -f 6 | cut -d '.' -f 2)
PD_BUGFIX_VERSION = $(shell grep 'Pd version' s_main.c | cut -d '\\' -f 1 | cut -d ' ' -f 6 | cut -d '.' -f 3)
PD_VERSION = $(PD_MAJOR_VERSION).$(PD_MINOR_VERSION).$(PD_BUGFIX_VERSION)

CYCLONE_MAJOR_VERSION = $(shell grep CYCLONE_VERSION ../../externals/miXed/cyclone/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
CYCLONE_RELEASE = $(shell grep CYCLONE_RELEASE ../../externals/miXed/cyclone/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
CYCLONE_BUILD = $(shell grep CYCLONE_BUILD ../../externals/miXed/cyclone/build_counter | cut -d ' ' -f 3 | cut -d '"' -f 2)
CYCLONE_VERSION = $(CYCLONE_MAJOR_VERSION) $(CYCLONE_RELEASE) $(CYCLONE_BUILD)

FLEXT_VERSION = $(shell grep PROJECT_NUMBER ../../externals/grill/flext/flext.doxy | cut -d '"' -f2 | cut -d ' ' -f2)

MAXLIB_VERSION = $(shell grep "#define VERSION" ../../externals/maxlib/maxlib.c | cut -d '"' -f 2)

ZEXY_VERSION = $(shell grep VERSION ../../externals/zexy/src/zexy.h | cut -d ' ' -f 3 | cut -d '"' -f 2)

# various files


DOUBLECLICKER_VERSION = $(shell date +20%y.%m.%d)
DOUBLECLICKER_PREFIX = pd-double-clicker
DOUBLECLICKER_NAME =  $(DOUBLECLICKER_PREFIX)-$(DOUBLECLICKER_VERSION)

PACKAGE_MAJOR_VERSION = 0
PACKAGE_MINOR_VERSION = 2
PACKAGE_BUGFIX_VERSION = 5
PACKAGE_VERSION = $(PACKAGE_MAJOR_VERSION).$(PACKAGE_MINOR_VERSION).$(PACKAGE_BUGFIX_VERSION)
PACKAGE_PREFIX = Pure Data
PACKAGE_NAME = $(PACKAGE_PREFIX)

# needs to have no spaces for SourceForce
DMG_PREFIX = PureData
DMG_NAME = $(DMG_PREFIX)-$(PD_VERSION)_MacOSX-($(PACKAGE_VERSION))

WELCOME_FILE = Welcome.html
README_FILE = ReadMe.html
INFO_FILE = $(PACKAGE_PREFIX).info

clean: darwin_dmg_clean darwin_mpkg_clean 
	-cd ../../pd/src && make clean
	cd ../../externals/miXed/cyclone && make clean
	cd ../../externals/build/darwin && make clean
	cd ../../externals/grill/build/darwin && make clean
	cd ../../externals/unauthorized && make clean
	cd ../../externals/zexy/src && make -f makefile.darwin clean
	cd ../../doc/pddp && make clean
	-cd ../../externals/sprinkler && make distclean
	cd ../../nqpoly && make clean

darwin_dmg_clean:
	-sudo rm -Rf installroot
	-sudo rm -Rf "$(DMG_PREFIX)"*
	-rm -f "$(README_FILE)" *.dmg *~

darwin_mpkg_clean:
	-cd ../../pd/src && make darwin_pkg_clean
	cd ../../externals/miXed/cyclone && make darwin_pkg_clean
	cd ../../externals/build/darwin && make darwin_pkg_clean
	cd ../../externals/grill/build/darwin && make darwin_pkg_clean
	cd ../../externals/unauthorized && make darwin_pkg_clean
	cd ../../externals/zexy/src && make -f makefile.darwin darwin_pkg_clean
	cd ../../doc/  && make darwin_pkg_clean
	cd ../../doc/pddp  && make darwin_pkg_clean
	cd ../../abstractions && make darwin_pkg_clean
# don't fail on these since they aren't in CVS
	-cd noncvs && make darwin_pkg_clean
	-cd ../../nqpoly && make darwin_pkg_clean

# this gets the files you need and puts them in the 
# right places for this Makefile to find them.  You 
# might want to use a different version of Pd.
checkout:
	cd ../.. && cvs co pd
	cd ../.. && cvs co pd
	cd ../.. && cvs co externals
	cd ../.. && cvs co doc
	cd ../.. && cvs co abstractions

darwin_mpkg_welcome:
	echo "<HTML><BODY><P><P>" > $(WELCOME_FILE)
	echo "<CENTER><IMG SRC=\"logo.jpg\">" >> $(WELCOME_FILE)
	echo "<H2>Version $(PD_VERSION)</H2>" >> $(WELCOME_FILE)
	echo "<P>written by Miller S. Puckette</P></CENTER>" >> $(WELCOME_FILE)
	echo "<FONT SIZE=\"-1\">" >> $(WELCOME_FILE)
	echo "<P>`grep -A9 ACKNOWLEDG ../../pd/README.txt`</P>" >> $(WELCOME_FILE)
	echo "</FONT>" >> $(WELCOME_FILE)
	echo "</BODY></HTML>" >> $(WELCOME_FILE)

darwin_mpkg_license:
  # generate HTML version of License
	echo "<HTML><BODY>" > License.html
	echo "<H3>(Parts of this package can be used under the Pd/BSD license)</H3>" >> License.html
	echo "<FONT SIZE=\"-1\">" >> License.html
	cat ../../externals/creb/COPYING | sed -e 's/^$$/\<P\>/g' >> License.html	
	echo "</FONT></BODY></HTML>" >> License.html

darwin_mpkg_readme:
	echo $(CYCLONE_RELEASE)
	-rm $(README_FILE)
	echo "<HTML>" > $(README_FILE)
	echo "<HEAD>" >> $(README_FILE)
	echo "<meta http-equiv=\"Content-Type\" content=\"text/html;charset=iso-8859-1\" />" >> $(README_FILE)
	echo "</HEAD>" >> $(README_FILE)
	echo "<BODY>" >> $(README_FILE)
	echo "<H2><IMG SRC=\"pd-16.png\">&nbsp;Pure Data Installer $(PD_VERSION)-$(PACKAGE_VERSION)</H2>" 	>> $(README_FILE)
	echo "<P>Pd is a free real-time computer music software package resembling Max.  It provides a patchable environment for audio analysis, synthesis, and processing, with a rich set of multimedia capabilities.  You can get Pd for Linux, Windows, MacOS X, BSD, or IRIX.</P>" >> $(README_FILE)
	echo "<P>For more information, go to: http://www.pure-data.org</P>" >> $(README_FILE)
	echo "<H3>!!Warning!!</H3>" >> $(README_FILE)	
	echo "<P>Installing this package anywhere <b>but</B> the default location will break things. Only do it if you are willing to clean up afterwards.</P>" >> $(README_FILE)	
	echo "<H3>Pure Data CVS Developers</H3>" >> $(README_FILE)	
# this may seem whack, but it generates the list of developers from the SourceForge site:
	wget -q -O - 'http://sourceforge.net/project/memberlist.php?group_id=55736' | grep -e '<td>' | sed -e :a -e 's/<[^>]*>//g;/</N;//ba' | grep -v '^$$' | sed -e 's/\t*/ - /' >> $(README_FILE)
	echo "</P>" >> $(README_FILE)
	echo "<P>" >> $(README_FILE)
	echo "Many others not listed have contributed their time and effort, this is just a list of the current developers in the SourceForge project.  But really, every Pd user is a developer and is encouraged to contribute to the CVS repository." >> $(README_FILE)
	echo "</P>" >> $(README_FILE)
	echo "<H3>Included Versions</H3>" >> $(README_FILE)	
#	echo "<P>The various versions of the included packages:</P>" >> $(README_FILE)
	echo "<UL>" >> $(README_FILE)
	echo "<LI>pure data: $(PD_VERSION) " >> $(README_FILE)
	echo "<LI>cyclone: $(CYCLONE_VERSION)" >> $(README_FILE)
	echo "<LI>flext: $(FLEXT_VERSION)" >> $(README_FILE)
	echo "<LI>maxlib: $(MAXLIB_VERSION)" >> $(README_FILE)
	echo "<LI>pd-abstractions: `date +20%y.%m.%d`" >> $(README_FILE)
	echo "<LI>pd-doc: `date +20%y.%m.%d`" >> $(README_FILE)
	echo "<LI>pd-externals: `date +20%y.%m.%d`" >> $(README_FILE)
	echo "<LI>pddp: `date +20%y.%m.%d`" >> $(README_FILE)
	echo "<LI>unauthorized: `date +20%y.%m.%d`" >> $(README_FILE)
	echo "<LI>zexy: $(ZEXY_VERSION)" >> $(README_FILE)
	echo "</UL>" >> $(README_FILE)
	echo "(this package was built on `date`) <BR>" >> $(README_FILE)
	echo "</BODY></HTML>" >> $(README_FILE)

darwin_mpkg: darwin_mpkg_readme darwin_mpkg_license darwin_mpkg_welcome
	test -d installroot/Packages || mkdir -p installroot/Packages
	test -d "installroot/$(PACKAGE_NAME).mpkg/Contents/Resources" \
		|| mkdir -p "installroot/$(PACKAGE_NAME).mpkg/Contents/Resources"
##############################
# MAKE PACKAGES
#
# pd
# maybe? a--enable-portaudio
	cd ../../pd/src && ./configure && make darwin_pkg 
	sudo cp -R ../../pd/pd-*.pkg installroot/Packages
# miXed (cyclone,toxy)
	cd ../../externals/miXed/cyclone && make darwin_pkg
	sudo cp -R ../../externals/miXed/cyclone/pd-*.pkg installroot/Packages
# 'externals'
	cd ../../externals/build/darwin && make darwin_pkg
	sudo cp -R ../../externals/build/darwin/pd-externals*.pkg installroot/Packages
# flext
	cd ../../externals/grill/build/darwin && make darwin_pkg
	sudo cp -R ../../externals/grill/build/darwin/pd-*.pkg installroot/Packages
# unauthorized
	cd ../../externals/unauthorized && make darwin_pkg
	sudo cp -R ../../externals/unauthorized/pd-unauthorized*.pkg installroot/Packages
# zexy
	cd ../../externals/zexy/src && make -f makefile.darwin darwin_pkg
	sudo cp -R ../../externals/zexy/src/pd-zexy*.pkg installroot/Packages
# pd-abstractions
	cd ../../abstractions/  && make darwin_pkg
	sudo cp -R ../../abstractions/pd-abstractions*.pkg installroot/Packages
# pd-doc
	cd ../../doc/  && make darwin_pkg
	sudo cp -R ../../doc/pd-doc*.pkg installroot/Packages
# pddp
	cd ../../doc/pddp  && make darwin_pkg
	sudo cp -R ../../doc/pddp/pd-pddp*.pkg installroot/Packages
# double-clicker
	cd double-clickers && make darwin_pkg
	sudo cp -R double-clickers/pd-*.pkg installroot/Packages
# don't fail on these since they aren't in CVS
# pd-noncvs
	-cd noncvs && make darwin_pkg
	-sudo cp -R noncvs/pd-noncvs*.pkg installroot/Packages
# nqpoly
	-cd ../../nqpoly && make darwin_pkg
	-sudo cp -R ../../nqpoly/pd-*.pkg installroot/Packages
##############################
# MAKE MPKG
#
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
	install -m644 ../../pd/LICENSE.txt "installroot/PD LICENSE.txt"
	install -m644 ../../externals/creb/COPYING "installroot/GNU GPL.txt"
	install -m644 $(README_FILE) $(WELCOME_FILE) License.html \
		logo.jpg pd-32.png pd-16.png \
		background.tiff \
		"installroot/$(PACKAGE_NAME).mpkg/Contents/Resources/"
	sudo chmod -R u+w installroot
	sudo chmod -R a+r installroot
	sudo chmod -R go-w installroot
	sudo chgrp -R staff installroot
	echo " "
	echo " "
	echo " DO NOT FORGET TO ADD Tcl/Tk!!"
	echo " "
	echo " "

dmg: 
	mv installroot "$(DMG_NAME)"
	/bin/sh mkdmg "$(DMG_NAME)"

