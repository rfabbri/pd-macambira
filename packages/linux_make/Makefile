#
# for making a tar.bz2 that installs using a Makefile
#
all: install

CWD := $(shell pwd)

DESTDIR = $(CWD)/build/
cvs_root_dir = $(CWD)/../..
BUILDLAYOUT_DIR = $(CWD)/..


# for command-line UNIX versions, you need to install Pd into the same
# directory as $(prefix) otherwise Pd won't be able to find extra, doc, etc.
prefix = /usr/local


include $(BUILDLAYOUT_DIR)/Makefile.buildlayout


# base level optimizations
OPT_CFLAGS = -O3 -funroll-loops -fomit-frame-pointer

# Generic x86 (tune for Pentium III, since that's more common these days)
#OPT_CFLAGS += -mcpu=i386 -mtune=pentium3
OPT_CFLAGS += -mcpu=pentium

# INTEL
#
# Pentium MMX
#OPT_CFLAGS += -mcpu=pentium-mmx -mmmx
# Pentium Pro
#OPT_CFLAGS += -mcpu=pentiumpro -mmmx
# Pentium II/Celeron
#OPT_CFLAGS += -mfpmath=sse -mmmx -msse -mcpu=pentium2
# Pentium III/Celeron2
#OPT_CFLAGS += -mfpmath=sse -mmmx -msse -mcpu=pentium3
# Pentium 4
#OPT_CFLAGS += -mfpmath=sse -mmmx -msse -msse2 -mcpu=pentium4

# AMD
#
# Athlon XP K7
#OPT_CFLAGS = -O3 -mcpu=athlon-xp -m3dnow -msse -mfpmath=sse

# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				cvs_root_dir=$(cvs_root_dir) \
				DESTDIR=$(DESTDIR) \
				prefix=$(prefix) \
				OPT_CFLAGS="$(OPT_CFLAGS)" \
				UNAME=$(UNAME)


install: 
	make -C $(packages_src) $(DEST_PATHS) install
	make -C $(packages_src) $(DEST_PATHS) doc_format
	@echo " "
	@echo "linux_make install succeeded!"

#==============================================================================#
#
## installer package
#
#==============================================================================#

package: tarbz2

PACKAGE_NAME = Pd-$(PD_VERSION)-$(PACKAGE_VERSION)-$(OS_NAME)-$(shell uname -m)
tarbz2: installer_makefile installer_readme
	mv $(DESTDIR)$(prefix) $(DESTDIR)$(PACKAGE_NAME)
	cd $(DESTDIR) && \
		tar cjf $(PACKAGE_NAME).tar.bz2  $(PACKAGE_NAME)
	mv $(DESTDIR)$(PACKAGE_NAME) $(DESTDIR)$(prefix)


INSTALLER_README_FILE = $(DESTDIR)$(prefix)/README.txt
installer_readme:
	install -d $(DESTDIR)$(prefix)
	-rm $(INSTALLER_README_FILE)
	touch $(INSTALLER_README_FILE)
	echo -e "$(PACKAGE_NAME)" >> $(INSTALLER_README_FILE)
	echo -e "\n\n" >> $(INSTALLER_README_FILE)
	echo -e "To install:\n" >> $(INSTALLER_README_FILE)
	echo -e "\tmake install\n" >> $(INSTALLER_README_FILE)
	echo -e "To uninstall:\n" >> $(INSTALLER_README_FILE)
	echo -e "\tmake uninstall\n" >> $(INSTALLER_README_FILE)
	echo -e "\n\n" >> $(INSTALLER_README_FILE)
	echo -e "You can install this package elsewhere, but its been compiled to run in $(prefix).  If you install it somewhere other than $(prefix), you will only be able to run Pd like this:\n" >> $(INSTALLER_README_FILE)
	echo -e "\tcd /path/to/bin/pd && ./pd\n" >> $(INSTALLER_README_FILE)
	echo -e "Here's how to install it elsewhere:" >> $(INSTALLER_README_FILE)
	echo -e "To install:\n" >> $(INSTALLER_README_FILE)
	echo -e "\tmake install prefix=/path/to\n" >> $(INSTALLER_README_FILE)
	echo -e "To uninstall:\n" >> $(INSTALLER_README_FILE)
	echo -e "\tmake uninstall prefix=/path/to\n" >> $(INSTALLER_README_FILE)


installer_makefile:
# it is very hard to escape the $ in shell scripts so delete files
	-rm -f -- $(helpdir)/iem*/*\$$*.pd $(objectsdir)/iem*/*\$$*.pd
# don't put the Makefile into the package yet, otherwise it'll get installed
	./generate_install_makefile.bash  $(DESTDIR)$(prefix) > tarbz2Makefile
	mv -i tarbz2Makefile $(DESTDIR)$(prefix)/Makefile

installer_clean:
	chmod -R +w $(packages_src)/linux_make/build
	rm -rf $(packages_src)/linux_make/build


#==============================================================================#
#
## CVS SOURCES
#
#==============================================================================#

clean: installer_clean
	cd $(packages_src) && make $(DEST_PATHS) clean


test_package:
	cd /tmp && tar xjf $(DESTDIR)/$(PACKAGE_NAME).tar.bz2
	make -C /tmp/$(PACKAGE_NAME) prefix=/tmp/usr/local install
	make -C /tmp/$(PACKAGE_NAME) prefix=/tmp/usr/local uninstall

test_locations:
	@echo "PD_VERSION: $(PD_VERSION)"
	@echo "PACKAGE_VERSION: $(PACKAGE_VERSION)"
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
