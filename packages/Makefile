#
#
#

# these are setup to be overridden by the packages/*/Makefiles
SRC_ROOT_DIR := $(shell pwd)/..
INSTALL_PREFIX = $(SRC_ROOT_DIR)/packages/build

# default target
all: abstractions

include Makefile.dirs

DST_PATHS = APPLICATIONS_DEST=$(APPLICATIONS_DEST) \
			   OBJECTS_DEST=$(OBJECTS_DEST) \
				DOCS_DEST=$(DOCS_DEST) \
				MANUALS_DEST=$(MANUALS_DEST)

#------------------------------------------------------------------------------#
# BUILD TARGETS
#------------------------------------------------------------------------------#
abstractions:
	make -f $(ABSTRACTIONS_SRC)/Makefile $(DST_PATHS)

#------------------------------------------------------------------------------#
# CLEAN TARGETS
#------------------------------------------------------------------------------#
abstractions_clean:
	make -f $(ABSTRACTIONS_SRC)/Makefile $(DST_PATHS) clean

clean: abstractions_clean
	-rm -f *~
	rm -rf $(MANUALS_DEST)
	rmdir $(DOCS_BASE) $(INSTALL_PREFIX)
