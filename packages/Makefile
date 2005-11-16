#
#
#

# these are setup to be overridden by the packages/*/Makefiles
SRC_ROOT_DIR := $(shell pwd)/..
INSTALL_PREFIX = $(SRC_ROOT_DIR)/packages/build

# default target
all: abstractions

include Makefile.buildlayout

DST_PATHS = APPLICATIONS_DEST=$(APPLICATIONS_DEST) \
				DOCS_DEST=$(DOCS_DEST) \
				HELP_DEST=$(HELP_DEST) \
				MANUALS_DEST=$(MANUALS_DEST) \
			   OBJECTS_DEST=$(OBJECTS_DEST) 

#==============================================================================#
#
# BUILD TARGETS
#
#==============================================================================#

#------------------------------------------------------------------------------
# abstractions
abstractions:
	make -f $(ABSTRACTIONS_SRC)/Makefile $(DST_PATHS) install

#------------------------------------------------------------------------------
# doc
doc:


#------------------------------------------------------------------------------
# externals
externals:
# doc
	make -f $(EXTERNALS_SRC)/build/doc/makefile $(DST_PATHS) install


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
	make -f $(ABSTRACTIONS_SRC)/Makefile $(DST_PATHS) clean

clean: abstractions_clean
	-rm -f *~
	rm -rf $(MANUALS_DEST) $(HELP_DEST)
	rmdir $(DOCS_DEST) $(INSTALL_PREFIX)
