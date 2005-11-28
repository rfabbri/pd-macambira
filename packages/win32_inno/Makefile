#
# This is a grand unifying Makefile for compiling Pd-extended under MinGW
#
all: install

CWD := $(shell pwd)

DESTDIR = $(CWD)/build
cvs_root_dir = $(CWD)/../..
BUILDLAYOUT_DIR = $(CWD)/..


include $(BUILDLAYOUT_DIR)/Makefile.buildlayout


# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				cvs_root_dir=$(cvs_root_dir) \
				DESTDIR=$(DESTDIR) \
				prefix=$(prefix) \
				UNAME=$(UNAME)

PD_INNO_SETUP = pd-inno.iss


install: $(PD_INNO_SETUP)
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

clean: 	
	rm $(PD_INNO_SETUP)
	cd $(packages_src) && make $(DEST_PATHS) clean


