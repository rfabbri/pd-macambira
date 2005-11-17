#==============================================================================#
#
# Centralized cross-platform build system 
#
# see README for instructions  <hans@at.or.at>
#
#==============================================================================#

CWD := $(shell pwd)

SRC_ROOT_DIR = $(CWD)/..
INSTALL_PREFIX = $(SRC_ROOT_DIR)/packages/build
BUILDLAYOUT_DIR = $(CWD)

# get the platform
UNAME := $(shell uname)

# default target
all: abstractions doc externals gem pd
	echo "Complete build succeeded!"

include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				SRC_ROOT_DIR=$(SRC_ROOT_DIR) \
				INSTALL_PREFIX=$(INSTALL_PREFIX)
#				APPLICATIONS_DEST=$(APPLICATIONS_DEST) \
#				DOCS_DEST=$(DOCS_DEST) \
#				HELP_DEST=$(HELP_DEST) \
#				MANUALS_DEST=$(MANUALS_DEST) \
#			   OBJECTS_DEST=$(OBJECTS_DEST) 

#==============================================================================#
#
# BUILD TARGETS
#
#==============================================================================#

#------------------------------------------------------------------------------
# abstractions
abstractions:
	make -f $(ABSTRACTIONS_SRC)/Makefile $(DEST_PATHS) install

#------------------------------------------------------------------------------
# doc
doc:


#------------------------------------------------------------------------------
# externals
externals:
# doc
	make -f $(EXTERNALS_SRC)/build/doc/makefile $(DEST_PATHS) install


#------------------------------------------------------------------------------
# Gem
gem:


#------------------------------------------------------------------------------
# pd
pd:


#==============================================================================#
# CLEAN TARGETS
#==============================================================================#
abstractions_clean:
	make -f $(ABSTRACTIONS_SRC)/Makefile $(DEST_PATHS) clean

clean: abstractions_clean
	-rm -f *~
	rm -rf $(MANUALS_DEST) $(HELP_DEST)
	rmdir $(DOCS_DEST) $(INSTALL_PREFIX)
