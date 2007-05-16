#
# This is a grand unifying Makefile for compiling Pd-extended under MinGW
#
all: install

CWD := $(shell pwd)

DESTDIR = $(CWD)/build
cvs_root_dir := $(shell cd $(CWD)/../.. && pwd)
BUILDLAYOUT_DIR = $(cvs_root_dir)/packages


include $(BUILDLAYOUT_DIR)/Makefile.buildlayout


# Generic x86 (tune for Pentium III, since that's most common these days)
OPT_CFLAGS += -mcpu=i586 -mtune=pentium3


# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				cvs_root_dir=$(cvs_root_dir) \
				DESTDIR=$(DESTDIR) \
				prefix=$(prefix) \
				OPT_CFLAGS="$(OPT_CFLAGS)" \
				UNAME=$(UNAME)

PD_INNO_SETUP = pd-inno.iss

package:  $(PD_INNO_SETUP)
## this pops up the GUI app
#	start $(PD_INNO_SETUP)
## this compiles the package on the command line
	/c/Program\ Files/Inno\ Setup\ 5/ISCC.exe $(PD_INNO_SETUP)
	@echo " "
	@echo "win32_inno install succeeded!"

# makefile.mingw is available from here:
# http://sourceforge.net/tracker/index.php?func=detail&aid=1374659&group_id=55736&atid=478072
build_pd:
	$(MAKE) -C $(pd_src)/src -f makefile.mingw

pd_install: build_pd
# the autoconf/MinGW setup doesn't compile the extras yet
#	$(MAKE) -C $(pd_src)/src $(DEST_PATHS) bin
#	-$(MAKE) -C $(pd_src)/src $(DEST_PATHS) install
	$(MAKE) -C $(pd_src)/src -f makefile.mingw $(DEST_PATHS) install

install: pd_install prebuilt_install lib_install exe_install
	@echo " "
	@echo "win32_inno install succeeded!"


prebuilt_install:
	$(MAKE) -C $(packages_src) $(DEST_PATHS) extended_install
	$(MAKE) -C $(packages_src) $(DEST_PATHS) doc_format
	install -p pd-settings.reg $(DESTDIR)$(prefix)

#==============================================================================#
#
## setup Pd binaries
#
#==============================================================================#

bin_src = /usr/local/bin
system32 = /c/WINDOWS/system32
# ultimately, the DLLs should be installed in %SystemRoot%\system32 by InnoSetup
dlldir =  $(DESTDIR)$(prefix)/
#dlldir = $(bindir)
lib_install:
# these get installed into %SystemRoot%\system32 by the installer
	install -d $(dlldir)
	install -p $(bin_src)/libogg-0.dll  $(dlldir)
	install -p $(bin_src)/libsndfile-1.dll  $(dlldir)
	install -p $(bin_src)/libvorbis-0.dll  $(dlldir)
	install -p $(bin_src)/libvorbisenc-2.dll  $(dlldir)
	install -p $(bin_src)/libvorbisfile-3.dll  $(dlldir)
	install -p $(bin_src)/pthreadGC2.dll  $(dlldir)
# these stay with pd.exe
	install -p $(bin_src)/tcl84.dll  $(bindir)
	install -p $(bin_src)/tclpip84.dll  $(bindir)
	install -p $(bin_src)/tk84.dll  $(bindir)
# if these are there, install them
	-install -p $(system32)/msvcp71.dll $(dlldir)
	-install -p $(system32)/msvcr71.dll $(dlldir)
	-install -p $(system32)/pthreadVC.dll $(dlldir)
# these go into the Pd package's lib dir
	install -d $(libdir)/tcl8.4
	cp -r /usr/local/lib/tcl8.4/* $(libdir)/tcl8.4
	install -d $(libdir)/tk8.4
	cp -r /usr/local/lib/tk8.4/* $(libdir)/tk8.4


exe_install:
	install -d $(bindir)
	install -p $(bin_src)/wish84.exe  $(bindir)
	install -p $(bin_src)/tclsh84.exe  $(bindir)


# here are some hacks to assemble a package from binaries
no_compile_hacks:
	cp -a /c/Program\ Files/pd-0.38-4 $(DESTDIR)$(prefix)/

$(PD_INNO_SETUP): $(PD_INNO_SETUP).in
	@echo "Making Inno Setup file: $(PD_INNO_SETUP)"
	cat "$(PD_INNO_SETUP).in" | \
		sed 's/PACKAGE_NAME/$(PACKAGE_NAME)/g' | \
		sed 's/PD_VERSION/$(PD_VERSION)/g' | \
		sed 's/PD-EXTENDED_VERSION/$(PD-EXTENDED_VERSION)/g' > \
		$(PD_INNO_SETUP)
#	start $(PD_INNO_SETUP)


#==============================================================================#
#
## CLEAN TARGETS
#
#==============================================================================#

distclean: package_clean
	$(MAKE) -C $(packages_src) $(DEST_PATHS) distclean


package_clean: 	
	-rm $(PD_INNO_SETUP)
	-rm -rf $(DESTDIR)
	-rm -rf Output


clean: package_clean
	$(MAKE) -C $(packages_src) $(DEST_PATHS) clean

#==============================================================================#
#
## TEST TARGETS
#
#==============================================================================#

test_package:
	echo "No tests here yet"


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


.PHONY: all install package clean distclean test_locations test_package exe_install
