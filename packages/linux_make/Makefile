#
# for making a tar.bz2 that installs using a Makefile
#

CWD := $(shell pwd)

DESTDIR = $(CWD)/build/
cvs_root_dir := $(shell cd $(CWD)/../.. && pwd)
BUILDLAYOUT_DIR = $(cvs_root_dir)/packages


# for command-line UNIX versions, you need to install Pd into the same
# directory as $(prefix) otherwise Pd won't be able to find extra, doc, etc.
prefix = /usr/local


include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

TARGET_PLATFORM := $(shell uname -m)
ifeq ($(TARGET_PLATFORM),i686)
# Generic x86 (tune for Pentium III, since that's more common these days)
OPT_CFLAGS += -mcpu=i686 -march=i386
#OPT_CFLAGS += -march=pentium3 -msse -mfpmath=sse
#OPT_CFLAGS += -march=pentium4 -msse -msse2 -mfpmath=sse
endif
ifeq ($(TARGET_PLATFORM),ppc)
    OPT_CFLAGS += -pipe -fsigned-char -mpowerpc-gfxopt
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
	@echo " "
	@echo "linux_make install succeeded!"

#==============================================================================#
#
## installer package
#
#==============================================================================#

package: tarbz2

PACKAGE_NAME = Pd-$(PD_VERSION)
tarbz2: installer_makefile installer_readme
	install -p .pdsettings $(DESTDIR)$(prefix)
	mv $(DESTDIR)$(prefix) $(DESTDIR)$(PACKAGE_NAME)
	cd $(DESTDIR) && \
		tar cjf $(PACKAGE_NAME).tar.bz2  $(PACKAGE_NAME)
	mv $(DESTDIR)$(PACKAGE_NAME) $(DESTDIR)$(prefix)


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
	autoconf --version
	@echo --------------------------------------------------
	make --version
	@echo --------------------------------------------------
	gcc --version
