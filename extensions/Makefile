#==============================================================================#
#
# Centralized build system for "extensions".  
#
# see for instructions: http://puredata.org/docs/developer/build
#  <hans@at.or.at>
#
#==============================================================================#

CWD := $(shell pwd)

# these are designed to be overridden by the packages/Makefile
cvs_root_dir = $(CWD)/..
DESTDIR = $(CWD)/build/
BUILDLAYOUT_DIR = $(cvs_root_dir)/packages

# default target
all: gripd

include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

#==============================================================================#
#
# OVERARCHING BUILD TARGETS
#
#==============================================================================#

# clean up after everything is installed
final_setup:
	chmod -R ugo-w $(pddocdir)

install: gripd_install ix_install
	@echo " "
	@echo "extensions install succeeded!"

#==============================================================================#
#
# PROJECT-SPECIFIC TARGETS
#
#==============================================================================#

#------------------------------------------------------------------------------#
# GRIPD
GRIPD_NAME = gripd
gripd:


gripd_install: 
	install -d $(helpdir)
	install -p $(extensions_src)/gripd/examples/gripd.pd \
		$(helpdir)/gripd-help.pd
	install -d $(manualsdir)/$(GRIPD_NAME)
	install -p $(extensions_src)/gripd/README.txt \
		$(manualsdir)/$(GRIPD_NAME) 
	install -d $(examplesdir)/$(GRIPD_NAME)
	install -p $(extensions_src)/gripd/examples/*.* \
		$(examplesdir)/$(GRIPD_NAME) 



#------------------------------------------------------------------------------#
# IX
IX_NAME = ix
ix_install:
	install -d $(objectsdir)/$(IX_NAME)
	install -p $(extensions_src)/gui/ix/*.wid $(objectsdir)/$(IX_NAME) 
	install -d $(manualsdir)/$(IX_NAME)
	install -p $(extensions_src)/gui/ix/README \
		$(manualsdir)/$(IX_NAME)/README.txt
	install -d $(examplesdir)/$(IX_NAME)
	install -p $(extensions_src)/gui/ix/*.pd $(examplesdir)/$(IX_NAME) 



#==============================================================================#
#
# DEVELOPER'S TARGETS
#
#==============================================================================#

# make the symlinks necessary to simulate the installed environment
devsymlinks:

#==============================================================================#
#
# CLEAN TARGETS
#
#==============================================================================#

# the destination-specific clean targets are in Makefile.buildlayout
clean: install_clean

distclean: clean cruft_clean
