
current: darwin_pkg

DESTDIR = installroot

PKG_VERSION = $(shell date +20%y.%m.%d)
PKG_PREFIX = pd-noncvs
PKG_NAME = $(PKG_PREFIX)-$(PKG_VERSION)

WELCOME_FILE = Welcome.html
INFO_FILE = $(PKG_NAME).info

clean: darwin_pkg_clean

darwin_pkg_clean:
	-sudo rm -Rf installroot $(PKG_NAME)* *.pkg
	-rm -f *~ 1

# install into MSP's default: /usr/local/lib

darwin_pkg: darwin_pkg_clean
# set up installroot dir
	test -d $(DESTDIR)/pd/extra || mkdir -p $(DESTDIR)/pd/extra
	install -m444 extra/*.pd_darwin $(DESTDIR)/pd/extra
	cp -rf doc $(DESTDIR)/pd/
	cp -f pd-noncvs.info $(PKG_NAME).info
# delete cruft
	-find $(DESTDIR) -name .DS_Store -delete
	-rm -f 1
# set proper permissions
	sudo chown -R root:staff $(DESTDIR)
	package $(DESTDIR) $(PKG_NAME).info -d . -ignoreDSStore
# install pkg docs
	install -m444 $(WELCOME_FILE) $(PKG_NAME).pkg/Contents/Resources
	sudo chown -R root:staff $(PKG_NAME).pkg/Contents/Resources

