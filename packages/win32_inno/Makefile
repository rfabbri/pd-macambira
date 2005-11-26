#
# This is a grand unifying Makefile for compiling Pd-extended under MinGW
#
all: install

CWD := $(shell pwd)

prefix = $(CWD)/build
cvs_root_dir = $(CWD)/../..
BUILDLAYOUT_DIR = $(CWD)/..


include $(BUILDLAYOUT_DIR)/Makefile.buildlayout


# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				cvs_root_dir=$(cvs_root_dir) \
				prefix=$(prefix) \
				UNAME=$(UNAME)


install: extended_install


#==============================================================================#
#
## CVS SOURCES
#
#==============================================================================#

PD_ISS_NAME = pd-inno.iss
$(PD_ISS_NAME): $(PD_ISS_NAME).in
	@echo "Making Inno Setup file: $(PD_ISS_NAME)"
	cat "$(PD_ISS_NAME).in" | \
		sed 's/PACKAGE_NAME/$(PACKAGE_NAME)/g' | \
		sed 's/PD_VERSION/$(PD_VERSION)-$(PACKAGE_VERSION)/g' > \
		$(PD_ISS_NAME)
	-./generate_inno_files.sh >> $(PD_ISS_NAME)
#	start $(PD_ISS_NAME)

#==============================================================================#
#
## CVS SOURCES
#
#==============================================================================#

extended_install:
	cd $(packages_src) && make $(DEST_PATHS) install

