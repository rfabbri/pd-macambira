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
OPT_CFLAGS = -O3 -funroll-loops -fomit-frame-pointer -fstrict-aliasing

# Generic x86 (tune for Pentium III, since that's most common these days)
OPT_CFLAGS += -march=pentium-mmx -mtune=pentium3 -mmmx

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

install:
	cd $(packages_src) && make $(DEST_PATHS) extended_install
	cd .. && make $(DEST_PATHS) doc_format
	install -p pd-settings.reg $(DESTDIR)$(prefix)
	@echo " "
	@echo "win32_inno install succeeded!"

#==============================================================================#
#
## CVS SOURCES
#
#==============================================================================#

# since I can't get Pd to compile, here are some hacks to assemble a package
# from binaries
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
