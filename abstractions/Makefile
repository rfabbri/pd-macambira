# Created by Hans-Christoph Steiner <hans@eds.org> to generate MacOS X packages
# This Makefile does not generate a PDDP pkg, PDDP has its own Makefile

current: darwin_pkg

ABSTRACTIONS_VERSION = $(shell date +20%y.%m.%d)
PKG_PREFIX = pd-abstractions
PKG_NAME = $(PKG_PREFIX)-$(ABSTRACTIONS_VERSION)

darwin_pkg_clean:
	-sudo rm -Rf installroot/ $(PKG_PREFIX)*.pkg/
	-rm -f $(PKG_NAME).info 1 

# install into MSP's default: /usr/local/lib

darwin_pkg: 
# set up installroot dir
	-chmod a-x */*.pd */*/*.pd */*/*/*.pd
	test -d installroot/abstractions || mkdir -p installroot/abstractions
	cp -Rf footils timestretch vadsr~ installroot/abstractions/
	cp -f pd-abstractions.info $(PKG_NAME).info
# delete cruft
	-find installroot -name .DS_Store -delete
	-sudo rm -Rf installroot/*/*/CVS installroot/*/*/*/CVS  installroot/*/*/*/*/CVS
# set proper permissions
	sudo chown -R root:staff installroot/*
	package installroot $(PKG_NAME).info -d . -ignoreDSStore
# install pkg docs
	install -m 644 Welcome.html $(PKG_NAME).pkg/Contents/Resources
	sudo chown -R root:staff $(PKG_NAME).pkg/Contents/Resources

