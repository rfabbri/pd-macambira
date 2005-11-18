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


# default target
all: abstractions doc externals flext gem pd
	@echo "Complete build succeeded!"

include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				SRC_ROOT_DIR=$(SRC_ROOT_DIR) \
				INSTALL_PREFIX=$(INSTALL_PREFIX) \
				UNAME=$(UNAME)

#==============================================================================#
#
# BUILD TARGETS
#
#==============================================================================#

#------------------------------------------------------------------------------
# abstractions
abstractions:


#------------------------------------------------------------------------------
# doc
doc:


#------------------------------------------------------------------------------
# externals
externals: 
	cd $(EXTERNALS_SRC) && make $(DEST_PATHS)


#------------------------------------------------------------------------------
# flext
flext:


#------------------------------------------------------------------------------
# Gem
gem:


#------------------------------------------------------------------------------
# pd
pd:

#==============================================================================#
#
# INSTALL TARGETS
#
#==============================================================================#

#------------------------------------------------------------------------------
# abstractions
abstractions_install:
	cd $(ABSTRACTIONS_SRC) && make $(DEST_PATHS) install

#------------------------------------------------------------------------------
# doc
doc_install:


#------------------------------------------------------------------------------
# externals
externals_install: 
	cd $(EXTERNALS_SRC) && make $(DEST_PATHS) install


#------------------------------------------------------------------------------
# flext_install
flext_install:


#------------------------------------------------------------------------------
# Gem
gem_install:


#------------------------------------------------------------------------------
# pd
pd_install:


#------------------------------------------------------------------------------
# install
install: abstractions_install doc_install externals_install flext_install 
install: gem_install pd_install
	@echo "Complete install succeeded!"

#==============================================================================#
# CLEAN TARGETS
#==============================================================================#
abstractions_clean:
	-cd $(ABSTRACTIONS_SRC) && make $(DEST_PATHS) clean


doc_clean:


externals_clean:
	-cd $(EXTERNALS_SRC) && make $(DEST_PATHS) clean


flext_clean:


gem_clean:


pd_clean:

# these targets are all from Makefile.buildlayout:
#   install_clean cruft_clean
clean: abstractions_clean doc_clean externals_clean flext_clean 
clean: gem_clean pd_clean 
	echo "Complete clean finished."
