#==============================================================================#
#
# Centralized build system for "externals".  
#
# see README for instructions  <hans@at.or.at>
#
#==============================================================================#

# TODO: maybe this should be moved to the root like all the others

CWD := $(shell pwd)

# these are setup to be overridden by the packages/Makefile
SRC_ROOT_DIR = $(CWD)/..
INSTALL_PREFIX = $(CWD)/build
BUILDLAYOUT_DIR = $(CWD)

# default target
all: objects

.SUFFIXES: .$(EXTENSION)


include $(BUILDLAYOUT_DIR)/Makefile.buildlayout


# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				SRC_ROOT_DIR=$(SRC_ROOT_DIR) \
				INSTALL_PREFIX=$(INSTALL_PREFIX)

#==============================================================================#
#
# COMPILE TARGETS
#
#==============================================================================#

objects: objects_$(OS_NAME)
	@echo "Compiling objects for $(OS_NAME) aka $(UNAME)"
# try it this way so that it'll recognize files that have already been built
	cd $(EXTERNALS_SRC)/build/$(OS_NAME) && make -k 

# these targets are for platform-specific needs
objects_darwin:

objects_linux:

objects_win:

objects_unknown:
# this target is for "everything else"

#==============================================================================#
#
# INSTALL TARGETS
#
#==============================================================================#

IEMABS_NAME = iemabs
IEMLIB_NAME = iemlib
PMPD_NAME = pmpd
applications_install: $(APPLICATIONS_DEST)
# iemlib
	install -d $(APPLICATIONS_DEST)/$(IEMLIB_NAME)
	install -p $(EXTERNALS_SRC)/iemlib/examples/*.* \
		$(APPLICATIONS_DEST)/$(IEMLIB_NAME)
# PMDP examples
	install -d $(APPLICATIONS_DEST)/$(PMPD_NAME)
	install -p $(EXTERNALS_SRC)/pmpd/exemples/*.pd \
		$(APPLICATIONS_DEST)/$(PMPD_NAME)


help_install: $(HELP_DEST)
#------------------------------------------------------------------------------#
# all standard objs' help files
#    it had to be broken up because the list is soo long
	install -p \
			$(EXTERNALS_SRC)/OSCx/doc/*.pd \
			$(EXTERNALS_SRC)/aenv~/*.pd \
			$(EXTERNALS_SRC)/arraysize/*.pd \
			$(EXTERNALS_SRC)/beatpipe/*.pd \
			$(EXTERNALS_SRC)/control/*/*.pd \
			$(EXTERNALS_SRC)/bbogart/chaos/tools/*.pd \
			$(EXTERNALS_SRC)/bbogart/*/*.pd \
			$(EXTERNALS_SRC)/creb/doc/*.pd \
				$(HELP_DEST)
	install -p \
			$(EXTERNALS_SRC)/creb/doc/examples/*.pd \
			$(EXTERNALS_SRC)/cxc/reference/*.pd \
			$(EXTERNALS_SRC)/dfx/*/*.pd \
			$(EXTERNALS_SRC)/ext13/doc/*.pd \
			$(EXTERNALS_SRC)/ggee/*/*-help.pd \
			$(EXTERNALS_SRC)/gem2pdp/*.pd \
			$(EXTERNALS_SRC)/ff/*.pd \
			$(EXTERNALS_SRC)/freeverb~/*.pd \
			$(EXTERNALS_SRC)/hcs/*-help.pd \
				$(HELP_DEST)
	install -p \
			$(EXTERNALS_SRC)/hcs/*/doc/*.pd \
			$(EXTERNALS_SRC)/hcs/hid/examples/*.pd \
			$(EXTERNALS_SRC)/hcs/pan/*-help.pd \
			$(EXTERNALS_SRC)/iem/*/*/*-help.pd \
			$(EXTERNALS_SRC)/markex/*.pd \
			$(EXTERNALS_SRC)/motex/*.pd \
			$(EXTERNALS_SRC)/nusmuk/line3/*.pd\
				$(HELP_DEST)
	install -p \
			$(EXTERNALS_SRC)/pdogg/*/*.pd \
			$(EXTERNALS_SRC)/plugin~/*.pd \
			$(EXTERNALS_SRC)/pmpd/help/*.pd\
			$(EXTERNALS_SRC)/rhythm_estimator/*.p? \
			$(EXTERNALS_SRC)/signal/*/*.pd \
			$(EXTERNALS_SRC)/sprinkler/sprinkler-help.pd \
			$(EXTERNALS_SRC)/susloop~/*.pd \
			$(EXTERNALS_SRC)/svf~/*.pd \
			$(EXTERNALS_SRC)/vbap/*.pd \
			$(EXTERNALS_SRC)/vst/*.pd \
			$(EXTERNALS_SRC)/zhzxh~/*.pd \
				$(HELP_DEST)
#------------------------------------------------------------------------------#
# IEMlib uses its own dir and since its not maintained in CVS
# its best not to change the objects
	install -d $(HELP_DEST)/iemhelp
	install -p $(EXTERNALS_SRC)/iemlib/iemhelp/*.* $(HELP_DEST)/iemhelp
#------------------------------------------------------------------------------#
# zexy uses its own dir
	install -d $(HELP_DEST)/zexy
	install -p $(EXTERNALS_SRC)/zexy/zexy.pd  $(HELP_DEST)
	install -p $(EXTERNALS_SRC)/zexy/examples/*.* $(HELP_DEST)/zexy
#------------------------------------------------------------------------------#
# iemmatrix uses its own dir
	install -d $(HELP_DEST)/iemmatrix
	install -p $(EXTERNALS_SRC)/iem/iemmatrix/doc/*.pd  $(HELP_DEST)/iemmatrix
#------------------------------------------------------------------------------#
#  mjlib [prob] conflicts with cyclone's [prob] so filter it
	install -p $(shell ls -1 $(EXTERNALS_SRC)/mjlib/doc/*.* | \
		grep -v 'prob') $(HELP_DEST)
#------------------------------------------------------------------------------#
#  maxlib [split] conflicts with cyclone so filter it
	install -p $(shell ls -1 $(EXTERNALS_SRC)/maxlib/help/*.* | \
		grep -v 'split') $(HELP_DEST)


manuals_install: $(MANUALS_DEST)
# pmpd
	install -d $(MANUALS_DEST)/$(PMPD_NAME)
	install -p $(EXTERNALS_SRC)/pmpd/doc/pmpd.pdf/pmpd.pdf \
		$(MANUALS_DEST)/$(PMPD_NAME)


objects_install: objects
	install -p $(EXTERNALS_SRC)/build/$(OS_NAME)/*.$(EXTENSION) $(OBJECTS_DEST)
#------------------------------------------------------------------------------#
# abstractions (i.e. objects written in Pd)
# iemmatrix
	install -p $(shell ls -1 $(EXTERNALS_SRC)/iem/iemmatrix/abs/*.* | \
		grep -v '\-help.pd') $(OBJECTS_DEST)
# IEMlib
#  iemabs still has the help patches in the old help-*.pd style
	install -d $(OBJECTS_DEST)/$(IEMABS_NAME)
	install -p $(shell ls -1 $(EXTERNALS_SRC)/iemlib/iemabs/*.pd | \
		grep -v 'help') $(OBJECTS_DEST)/$(IEMABS_NAME)
	install -d $(HELP_DEST)/$(IEMABS_NAME)
	install -p $(EXTERNALS_SRC)/iemlib/iemabs/*help*.pd \
		$(HELP_DEST)/$(IEMABS_NAME)
	install -p $(EXTERNALS_SRC)/iemlib/iemabs/*.mp3 $(HELP_DEST)/$(IEMABS_NAME)
	install -p $(EXTERNALS_SRC)/iemlib/iemabs/*.wav $(HELP_DEST)/$(IEMABS_NAME)
	install -d $(MANUALS_DEST)/$(IEMABS_NAME)
	install -p $(EXTERNALS_SRC)/iemlib/iemabs/*.txt \
		$(MANUALS_DEST)/$(IEMABS_NAME)
# vbap
	install -p $(EXTERNALS_SRC)/vbap/graph-to-aziele.pd $(OBJECTS_DEST)



install: $(OBJECTS_DEST) help_install applications_install objects_install
install: manuals_install
	@echo "externals install succeeded!"

#==============================================================================#
#
# DEVELOPER'S TARGETS
#
#==============================================================================#

# make the symlinks necessary to simulate the installed environment
devsymlinks: 


# FYI: clean targets are in Makefile.buildlayout
clean: install_clean cruft_clean
