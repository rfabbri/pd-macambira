#==============================================================================#
#
# Centralized build system for "doc".  
#
# see for instructions: http://puredata.org/docs/developer/build
#  <hans@at.or.at>
#
#==============================================================================#

CWD := $(shell pwd)

# these are designed to be overridden by the packages/Makefile
SRC_ROOT_DIR = $(CWD)/..
INSTALL_PREFIX = build
BUILDLAYOUT_DIR = $(CWD)

# default target
all:
	@echo "this currently does nothing"

include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

#==============================================================================#
#
# OVERARCHING BUILD TARGETS
#
#==============================================================================#

# clean up after everything is installed
final_setup:
	chmod -R ugo-w $(DOCS_DEST)

install: pd-msg_install pddp_install tutorials_install
	@echo " "
	@echo "doc install succeeded!"

#==============================================================================#
#
# PROJECT-SPECIFIC TARGETS
#
#==============================================================================#

#------------------------------------------------------------------------------#
# PD-MSG
PD-MSG_NAME = pd-msg
pd-msg_install: $(MANUALS_DEST)
	install -d $(MANUALS_DEST)/$(PD-MSG_NAME)
	install -p $(DOC_SRC)/additional/pd-msg/*.txt \
		$(MANUALS_DEST)/$(PD-MSG_NAME) 
	for dir in $(shell cd  $(DOC_SRC)/additional/pd-msg && ls -d [1-5].*); do \
		echo "Including $$dir in $(PD-MSG_NAME)"; \
		install -d $(MANUALS_DEST)/$(PD-MSG_NAME)/$$dir; \
		install -p $(DOC_SRC)/additional/pd-msg/$$dir/*.* \
			$(MANUALS_DEST)/$(PD-MSG_NAME)/$$dir; \
	done


#------------------------------------------------------------------------------#
# PDDP
pddp_install: $(HELP_DEST)
	install -p $(DOC_SRC)/pddp/*.pd $(HELP_DEST)
# this file is used in key-help.pd
	install -p $(DOC_SRC)/pddp/keyboard_fun.txt $(HELP_DEST)


#------------------------------------------------------------------------------#
# TUTORIALS
tutorials_install: $(MANUALS_DEST)
# pddrums
	install -d $(MANUALS_DEST)/PdDrums
	install -p $(DOC_SRC)/tutorials/footils/pddrums/*.* $(MANUALS_DEST)/PdDrums
# playnow
	install -d $(MANUALS_DEST)/PlayNow
	install -p $(DOC_SRC)/tutorials/playnow/*.pd $(MANUALS_DEST)/PlayNow
# these aren't needed yet <hans@at.or.at>
#	install -p $(DOC_SRC)/tutorials/playnow/*.mid $(MANUALS_DEST)/PlayNow


#==============================================================================#
#
# DEVELOPER'S TARGETS
#
#==============================================================================#

# make the symlinks necessary to simulate the installed environment
devsymlinks:
	ln -s $(PD_SRC)/doc/2.control.examples $(DOC_SRC)/2.control.examples
	ln -s $(PD_SRC)/doc/3.audio.examples $(DOC_SRC)/3.audio.examples
# pd <= 0.38-4 has 4.fft.examples
	ln -s $(PD_SRC)/doc/4.fft.examples $(DOC_SRC)/4.fft.examples
# pd > 0.38-4 has 4.data.structures
	ln -s $(PD_SRC)/doc/4.data.structures $(DOC_SRC)/4.data.structures
	ln -s $(PD_SRC)/doc/7.stuff $(DOC_SRC)/7.stuff
	ln -s $(PD_SRC)/doc/sound $(DOC_SRC)/sound
# put the pddp support objects in the path
	ln -s $(ABSTRACTIONS_SRC)/pddp $(DOC_SRC)/pddp/pddp

#==============================================================================#
#
# CLEAN TARGETS
#
#==============================================================================#

# the destination-specific clean targets are in Makefile.buildlayout
clean: install_clean

distclean: clean cruft_clean
