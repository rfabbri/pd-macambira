# Created by Hans-Christoph Steiner <hans@eds.org> to generate MacOS X packages
# This Makefile does not generate a PDDP pkg, PDDP has its own Makefile
#
# For info on making file types and icons, see:
# http://www.macosxhints.com/article.php?story=20030728055235121&query=add+icon+file
# 

current: darwin_pkg

PACKAGE_VERSION = $(shell date +20%y.%m.%d)
PACKAGE_PREFIX = pd-double-clickers
PACKAGE_NAME = $(PACKAGE_PREFIX)-$(PACKAGE_VERSION)

darwin_pkg_clean:
	-sudo rm -Rf installroot/ $(PACKAGE_PREFIX)*.pkg/
	-rm -f $(PACKAGE_PREFIX)-*.info 1 License.html Welcome.???*

# install into MSP's default: /usr/local/lib

darwin_pkg: darwin_pkg_clean
# set up installroot dir
	install -d "installroot/Applications/Pure Data.app/Contents/MacOS"
	install -m555 Pd.* "installroot/Applications/Pure Data.app/Contents/MacOS"
	install -m555 "Pure Data" "installroot/Applications/Pure Data.app/Contents/MacOS"
	install -m444 Info.plist "installroot/Applications/Pure Data.app/Contents"
	install -d "installroot/Applications/Pure Data.app/Contents/Resources"
	install -m444 *.icns "installroot/Applications/Pure Data.app/Contents/Resources"
	cp -f pd-double-clickers.info $(PACKAGE_NAME).info
# delete cruft
	-find installroot -name .DS_Store -delete
	-find installroot -name CVS -delete
	-rm -f 1
# set proper permissions
	sudo chown -R root:staff installroot
	package installroot $(PACKAGE_NAME).info -d . -ignoreDSStore
# install pkg docs
#	install -m 644 License.html $(PACKAGE_NAME).pkg/Contents/Resources
	sudo chown -R root:staff $(PACKAGE_NAME).pkg/Contents/Resources

