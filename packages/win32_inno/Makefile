#
# This is a grand unifying Makefile for compiling Pd-extended under MinGW
#
all: install

CWD := $(shell pwd)

DESTDIR = $(CWD)/build
cvs_root_dir = $(CWD)/../..
BUILDLAYOUT_DIR = $(CWD)/..


include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

# base level optimizations
OPT_CFLAGS = -O3 -funroll-loops -fomit-frame-pointer

# Generic x86 (tune for Pentium III, since that's most common these days)
OPT_CFLAGS += -mcpu=i586 -mtune=pentium3

# INTEL
#
# Pentium MMX
#OPT_CFLAGS += -march=pentium-mmx -mmmx
# Pentium Pro
#OPT_CFLAGS += -march=pentiumpro -mmmx
# Pentium II/Celeron
#OPT_CFLAGS += -mfpmath=sse -mmmx -msse -march=pentium2
# Pentium III/Celeron2
#OPT_CFLAGS += -mfpmath=sse -mmmx -msse -march=pentium3
# Pentium 4
#OPT_CFLAGS += -mfpmath=sse -mmmx -msse -msse2 -march=pentium4

# AMD
#
# Athlon XP K7
#OPT_CFLAGS = -O3 -march=athlon-xp -m3dnow -msse -mfpmath=sse


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
	start $(PD_INNO_SETUP)
	@echo " "
	@echo "win32_inno install succeeded!"

# makefile.mingw is available from here:
# http://sourceforge.net/tracker/index.php?func=detail&aid=1374659&group_id=55736&atid=478072
build_pd:
	make -C $(pd_src)/src -f makefile.mingw

pd_install: build_pd
# the autoconf/MinGW setup doesn't compile the extras yet
#	make -C $(pd_src)/src $(DEST_PATHS) bin
#	-make -C $(pd_src)/src $(DEST_PATHS) install
	make -C $(pd_src)/src -f makefile.mingw $(DEST_PATHS) install

install: pd_install prebuilt_install lib_install exe_install
	@echo " "
	@echo "win32_inno install succeeded!"


prebuilt_install:
	make -C $(packages_src) $(DEST_PATHS) extended_install
	make -C $(packages_src) $(DEST_PATHS) doc_format
	install -p pd-settings.reg $(DESTDIR)$(prefix)

#==============================================================================#
#
## setup Pd binaries
#
#==============================================================================#

bin_src = /usr/local/bin
# ultimately, the DLLs should be installed in %SystemRoot%\system32 by InnoSetup
#dlldir =  $(DESTDIR)$(prefix)/
dlldir = $(bindir)
lib_install:
# these get installed into %SystemRoot%\system32 by the installer
	install -d $(dlldir)
	install -p $(bin_src)/libogg-0.dll  $(dlldir)/libogg-0.dll
	install -p $(bin_src)/libsndfile-1.dll  $(dlldir)/libsndfile-1.dll
	install -p $(bin_src)/libvorbis-0.dll  $(dlldir)/libvorbis-0.dll
	install -p $(bin_src)/libvorbisenc-2.dll  $(dlldir)/libvorbisenc-2.dll
	install -p $(bin_src)/libvorbisfile-3.dll  $(dlldir)/libvorbisfile-3.dll
	install -p $(bin_src)/pthreadGC2.dll  $(dlldir)/pthreadGC2.dll
	install -p $(bin_src)/tcl84.dll  $(dlldir)/tcl84.dll
	install -p $(bin_src)/tclpip84.dll  $(dlldir)/tclpip84.dll
	install -p $(bin_src)/tk84.dll  $(dlldir)/tk84.dll
# this is a quick hack, since the gripd.dll still looks for pthreadVC.dll
	(test -e $(DESTDIR)$(prefix)/gripd/pthreadVC.dll && \
		install -p $(DESTDIR)$(prefix)/gripd/pthreadVC.dll $(dlldir)/pthreadVC.dll) || \
		test -e $(SYSTEMROOT)/system32/pthreadVC.dll || \
			install -p $(SYSTEMROOT)/pthreadVC.dll $(dlldir)/pthreadVC.dll
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
		sed 's/PACKAGE_VERSION/$(PACKAGE_VERSION)/g' > \
		$(PD_INNO_SETUP)
#	start $(PD_INNO_SETUP)


#==============================================================================#
#
## CVS SOURCES
#
#==============================================================================#

distclean: clean
	cd $(packages_src) && make $(DEST_PATHS) distclean


clean: 	
	-rm $(PD_INNO_SETUP)
	-cd $(packages_src) && make $(DEST_PATHS) clean





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


.PHONY: all install package clean distclean test_locations dll_install exe_install
