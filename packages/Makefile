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
all: abstractions doc externals gem pd
	@echo "Complete build succeeded!"

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

#------------------------------------------------------------------------------
# which OS to compile for
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
  OS_NAME = linux
else 
  ifeq ($(UNAME),Darwin)
    OS_NAME = darwin
  else
    ifeq (MINGW,$(findstring MINGW,$(UNAME)))
      OS_NAME = win
    else
      OS_NAME = unknown
      $(warning WARNING: unknown environment "$(UNAME)".)
    endif
  endif
endif

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
externals: externals_$(OS_NAME)
# doc
	make -f $(EXTERNALS_SRC)/build/doc/makefile $(DEST_PATHS) install
	@echo "Making externals for $(OS_NAME) aka $(UNAME)"
	make -f $(EXTERNALS_SRC)/build/$(OS_NAME)/makefile $(DEST_PATHS) install

# these targets are for platform-specific needs
externals_darwin:

externals_linux:

externals_win:

externals_unknown:
# this target is for "everything else"

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


externals_clean:
	make -f $(EXTERNALS_SRC)/build/$(OS_NAME)/makefile $(DEST_PATHS) clean



clean: abstractions_clean 
	-rm -f *~
	rm -rf $(MANUALS_DEST) $(HELP_DEST)
	rmdir $(DOCS_DEST) $(INSTALL_PREFIX)
