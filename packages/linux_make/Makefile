#
# for making a tar.bz2 that installs using a Makefile
#

CWD := $(shell pwd)

# DESTDIR needs the trailing slash
DESTDIR = $(CWD)/build/
cvs_root_dir := $(shell cd $(CWD)/../.. && pwd)
BUILDLAYOUT_DIR = $(cvs_root_dir)/packages

include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

# for command-line UNIX versions, you need to install Pd into the same
# directory as $(prefix) otherwise Pd won't be able to find extra, doc, etc.
ifeq ($(PACKAGE_TYPE),deb)
  prefix = /usr
  # generates the dependencies for all externals and pd itself
  PACKAGE_DEPENDS := $(shell find $(DESTDIR) \( -name "*.pd_linux" -or -name pd-gui -or -wholename \*bin/pd \) -print0 | xargs -0 dpkg-shlibdeps -O | sed 's|shlibs:Depends=|, |' )
else
  prefix = /usr/local
endif

TARGET_PLATFORM := $(shell uname -m)
ifeq ($(TARGET_PLATFORM),i686)
# Generic x86 (tune for Pentium III, since that's more common these days)
OPT_CFLAGS += -mtune=i686 -march=i386
#OPT_CFLAGS += -march=pentium3 -msse -mfpmath=sse
#OPT_CFLAGS += -march=pentium4 -msse -msse2 -mfpmath=sse
endif
ifeq ($(TARGET_PLATFORM),ppc)
    OPT_CFLAGS += -pipe -fsigned-char -mpowerpc
endif

GCC_VERSION := $(shell gcc -dumpversion)
ifeq ($(GCC_VERSION),4.1.2)
  OPT_CFLAGS += -ftree-vectorize -ftree-vectorizer-verbose=3
endif

# lots more here: http://www.gentoo.se/docs/doc-cflags.php

# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				cvs_root_dir=$(cvs_root_dir) \
				DESTDIR=$(DESTDIR) \
				prefix=$(prefix) \
				OPT_CFLAGS="$(OPT_CFLAGS)" \
				UNAME=$(UNAME)

all: 	
	$(MAKE) -C $(packages_src) $(DEST_PATHS)


install: 
	$(MAKE) -C $(packages_src) $(DEST_PATHS) install
	$(MAKE) -C $(packages_src) $(DEST_PATHS) doc_format
	find $(DESTDIR) -name \*.pd -print0 | xargs -0 chmod a-x
	@echo " "
	@echo "linux_make install succeeded!"

#==============================================================================#
#
## installer package
#
#==============================================================================#

PACKAGE_NAME = Pd-$(PD_VERSION)

package: $(PACKAGE_TYPE)


tarbz2: installer_makefile installer_readme
	install -p $(packages_src)/linux_make/default.pdextended $(DESTDIR)$(libpddir)
	mv $(DESTDIR)$(prefix) $(DESTDIR)$(PACKAGE_NAME)
	cd $(DESTDIR) && \
		tar cjf $(PACKAGE_NAME).tar.bz2  $(PACKAGE_NAME)
	mv $(DESTDIR)$(PACKAGE_NAME) $(DESTDIR)$(prefix)


deb: DEB_BUILD_ARCH := $(shell dpkg-architecture -qDEB_BUILD_ARCH)
deb: DEB_PD_VERSION := $(shell echo $(PD_VERSION) | sed 's|\(.*\)-extended-\(.*\)|\1~cvs\2-1|')
deb: $(bindir)
# free desktop menu, mime, and icon support
# perhaps the icons should go in /usr/share/pixmaps like 'scribus'
	install -d $(DESTDIR)/usr/share/icons/hicolor/128x128/apps
	install -p -m0644 $(packages_src)/linux_make/pd.png \
		$(DESTDIR)/usr/share/icons/hicolor/128x128/apps/
	install -d $(DESTDIR)/usr/share/icons/hicolor/48x48/apps
	install -p -m0644 $(packages_src)/linux_make/pd-48x48.png \
		$(DESTDIR)/usr/share/icons/hicolor/48x48/apps/pd.png
	install -d $(DESTDIR)/usr/share/icons/hicolor/128x128/mimetypes
	install -p -m0644 $(packages_src)/linux_make/text-x-puredata.png \
		$(DESTDIR)/usr/share/icons/hicolor/128x128/mimetypes
	install -d $(DESTDIR)/usr/share/applications/ 
	install -p -m0644 $(packages_src)/linux_make/pd-extended.desktop \
		$(DESTDIR)/usr/share/applications/
# add mime type to .pd files
	install -d $(DESTDIR)/usr/share/mime/packages/
	install -p -m0644 $(packages_src)/linux_make/pd-extended.xml \
		$(DESTDIR)/usr/share/mime/packages/
# files for /etc
	install -d $(DESTDIR)/etc/bash_completion.d/
	install -p -m0644 $(scripts_src)/bash_completion/pd \
		$(DESTDIR)/etc/bash_completion.d
	install -d $(DESTDIR)/etc/default
	install -p -m0644 $(scripts_src)/debian/default/pd-extended \
		$(DESTDIR)/etc/default
	install -d $(DESTDIR)/etc/init.d
	install -p -m0644 $(scripts_src)/debian/init.d/pd-extended \
		$(DESTDIR)/etc/init.d/
# default prefs file
	install -d $(DESTDIR)/etc/pd-extended
	ln -s $(libpddir)/default.pdextended \
		$(DESTDIR)/etc/pd-extended/default.pdextended
	install -p $(packages_src)/linux_make/default.pdextended $(DESTDIR)$(libpddir)
# emacs mode for .pd files
	install -d $(DESTDIR)/usr/share/emacs/site-lisp/
	install -p -m0644 $(scripts_src)/pd-mode.el \
		$(DESTDIR)/usr/share/emacs/site-lisp/
# Pd-related scripts
	install -p $(scripts_src)/pd-diff $(DESTDIR)$(bindir)
	install -p $(scripts_src)/config-switcher.sh $(DESTDIR)$(bindir)
# delete these since they are provided by the 'puredata' package
	rm -f $(DESTDIR)$(bindir)/pdsend
	rm -f $(DESTDIR)$(bindir)/pdreceive
	rm -f $(DESTDIR)/usr/share/man/man1/pdsend.1.gz
	rm -f $(DESTDIR)/usr/share/man/man1/pdreceive.1.gz
# debian packaging
	test -d $(DESTDIR)/DEBIAN || mkdir -p $(DESTDIR)/DEBIAN
	cp $(packages_src)/linux_make/debian/control $(DESTDIR)/DEBIAN
	cp $(packages_src)/linux_make/debian/pd-extended.postrm $(DESTDIR)/DEBIAN
	cp $(packages_src)/linux_make/debian/pd-extended.postinst $(DESTDIR)/DEBIAN
# set build architecture and version for the package
	sed -i 's|^Version:.*|Version: $(DEB_PD_VERSION)|' \
		$(DESTDIR)/DEBIAN/control
	sed -i 's|^Architecture:.*|Architecture: $(DEB_BUILD_ARCH)|' \
		$(DESTDIR)/DEBIAN/control
# add auto-generated dependencies
	sed -i 's!^\(Depends:.*\)!\1 $(PACKAGE_DEPENDS)!' $(DESTDIR)/DEBIAN/control
	fakeroot dpkg-deb --build $(DESTDIR) $(packages_src)/linux_make/$(PACKAGE_NAME).deb


INSTALLER_README_FILE = $(DESTDIR)$(prefix)/README.txt
installer_readme:
	install -d $(DESTDIR)$(prefix)
	-rm $(INSTALLER_README_FILE)
	sed 's|PACKAGE_NAME|$(PACKAGE_NAME)|' $(packages_src)/linux_make/README > \
		$(INSTALLER_README_FILE)


installer_makefile:
# it is very hard to escape the $ in shell scripts so delete files
	-rm -f -- $(helpdir)/iem*/*\$$*.pd $(objectsdir)/iem*/*\$$*.pd
# don't put the Makefile into the package yet, otherwise it'll get installed
	./generate_install_makefile.bash  $(DESTDIR)$(prefix) > tarbz2Makefile
	mv -i tarbz2Makefile $(DESTDIR)$(prefix)/Makefile

installer_clean:
	-chmod -R +w $(packages_src)/linux_make/build
	-rm -rf $(packages_src)/linux_make/build


#==============================================================================#
#
## CLEAN TARGETS
#
#==============================================================================#

distclean: installer_clean 
	$(MAKE) -C $(packages_src) $(DEST_PATHS) distclean


clean: installer_clean
	$(MAKE) -C $(packages_src) $(DEST_PATHS) clean

package_clean: installer_clean


#==============================================================================#
#
## TEST TARGETS
#
#==============================================================================#

#==============================================================================#
#
## TEST TARGETS
#
#==============================================================================#

test_package:
	cd /tmp && tar xjf $(DESTDIR)/$(PACKAGE_NAME).tar.bz2
	$(MAKE) -C /tmp/$(PACKAGE_NAME) prefix=/tmp/usr/local install
	$(MAKE) -C /tmp/$(PACKAGE_NAME) prefix=/tmp/usr/local uninstall
	-rmdir /tmp/usr/local
	-rmdir /tmp/usr
	-rm -rf -- /tmp/$(PACKAGE_NAME)

test_locations:
	@echo --------------------------------------------------
	@echo "PD_VERSION: $(PD_VERSION)"
	@echo "PD-EXTENDED_VERSION: $(PD-EXTENDED_VERSION)"
	@echo "CWD $(CWD)"
	@echo "DESTDIR $(DESTDIR)"
	@echo "PACKAGE_TYPE $(PACKAGE_TYPE)"
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
#	@echo "PACKAGE_DEPENDS 	$(PACKAGE_DEPENDS)"
#	@echo --------------------------------------------------
	autoconf --version
	@echo --------------------------------------------------
	make --version
	@echo --------------------------------------------------
	gcc --version
