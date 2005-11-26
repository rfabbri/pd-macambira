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
default: all

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

#------------------------------------------------------------------------------#
# BUILD
all: pre_all_$(OS_NAME) creb cyclone iemabs iemlib iemmatrix pddp pdp pmpd \
		toxy vbap zexy
	@echo "Compiling objects for $(OS_NAME) aka $(UNAME)"
# try it this way so that it'll recognize files that have already been built
	cd $(EXTERNALS_SRC)/build/$(OS_NAME) && make -k 

# these targets are for platform-specific needs that run before objects:
pre_all_darwin:

pre_all_linux:

pre_all_win:

pre_all_unknown:
# this target is for "everything else"


#------------------------------------------------------------------------------#
# INSTALL
install: all $(OBJECTS_DEST) help_install objects_install unfiltered_install \
 creb_install cyclone_install iemabs_install iemabs_install iemmatrix_install \
 pddp_install pdp_install pmpd_install toxy_install vbap_install zexy_install
	@echo " "
	@echo "externals install succeeded!"

objects_install: 
	cd $(EXTERNALS_SRC)/build/$(OS_NAME)/ && \
		install -p *.$(EXTENSION) $(OBJECTS_DEST)


#==============================================================================#
#
# INSTALL TARGETS
#
#==============================================================================#


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
			$(EXTERNALS_SRC)/iem/comport/*/*-help.pd \
			$(EXTERNALS_SRC)/markex/*.pd \
			$(EXTERNALS_SRC)/motex/*.pd \
			$(EXTERNALS_SRC)/nusmuk/line3/*.pd\
				$(HELP_DEST)
	install -p \
			$(EXTERNALS_SRC)/pdogg/*/*.pd \
			$(EXTERNALS_SRC)/plugin~/*.pd \
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
#  mjlib [prob] conflicts with cyclone's [prob] so filter it
	install -p $(shell ls -1 $(EXTERNALS_SRC)/mjlib/doc/*.* | \
		grep -v 'prob') $(HELP_DEST)
#------------------------------------------------------------------------------#
#  maxlib [split] conflicts with cyclone so filter it
	install -p $(shell ls -1 $(EXTERNALS_SRC)/maxlib/help/*.* | \
		grep -v 'split') $(HELP_DEST)



#==============================================================================#
#
# PROJECT TARGETS
#
#==============================================================================#


#------------------------------------------------------------------------------#
# TEMPLATE
TEMPLATE_NAME=template
template:


template_install: template
	install -d $(OBJECTS_DEST)/$(TEMPLATE_NAME)
	install -d $(HELP_DEST)/$(TEMPLATE_NAME)
	install -d $(MANUALS_DEST)/$(TEMPLATE_NAME)
	install -d $(APPLICATIONS_DEST)/$(TEMPLATE_NAME)


#------------------------------------------------------------------------------#
# CREB
CREB_NAME=creb
creb:


creb_install: creb
	install -d $(OBJECTS_DEST)/$(CREB_NAME)
	install -p $(EXTERNALS_SRC)/creb/abs/*.pd $(OBJECTS_DEST)/$(CREB_NAME)
	install -d $(HELP_DEST)/$(CREB_NAME)
	install -p -m0644	$(EXTERNALS_SRC)/creb/doc/examples/*.* \
		$(HELP_DEST)/$(CREB_NAME)
#	install -d $(MANUALS_DEST)/$(CREB_NAME)
#	install -d $(APPLICATIONS_DEST)/$(CREB_NAME)



#------------------------------------------------------------------------------#
# CYCLONE
CYCLONE_NAME=cyclone
# cyclone is compiled straight into $(OUT_DIR)
CYCLONE_OUT_DIR=$(OBJECTS_DEST)/$(CYCLONE_NAME)
cyclone:


cyclone_install: cyclone
	install -d $(OBJECTS_DEST)/$(CYCLONE_NAME)
	$(SCRIPTS_SRC)/generate-libdir-metafile.sh $(OBJECTS_DEST) $(CYCLONE_NAME) \
		--author "Kzrysztof Czaja" \
		--license "BSD" \
		--description "a library for porting and running Max/MSP patches in Pd"
	cd $(EXTERNALS_SRC)/miXed/cyclone && $(MAKE) OUT_DIR=$(CYCLONE_OUT_DIR)
# install "cyclist" command line app with pd
	install -d $(BIN_DEST)
# this doesn't build in the right place yet
#	install -p -m0755 $(CYCLONE_OUT_DIR)/cyclist $(BIN_DEST) && \
#		rm $(CYCLONE_OUT_DIR)/cyclist
# rename cyclone lib since it will pre-empt libdir functionality
	mv $(CYCLONE_OUT_DIR)/cyclone.$(EXTENSION) \
		$(CYCLONE_OUT_DIR)/cyclonelib.$(EXTENSION)
	install -d $(HELP_DEST)/$(CYCLONE_NAME)
	install -p $(EXTERNALS_SRC)/miXed/doc/help/cyclone/*.*  \
		$(HELP_DEST)/$(CYCLONE_NAME)
#	install -d $(MANUALS_DEST)/$(CYCLONE_NAME)
	install -d $(APPLICATIONS_DEST)/$(CYCLONE_NAME)
	install -p $(EXTERNALS_SRC)/miXed/test/cyclone/*.*  \
		$(APPLICATIONS_DEST)/$(CYCLONE_NAME)





#------------------------------------------------------------------------------#
# IEMABS
IEMABS_NAME=iemabs
iemabs:


iemabs_install: iemabs
#  iemabs still has the help patches in the old help-*.pd style
	install -d $(OBJECTS_DEST)/$(IEMABS_NAME)
	$(SCRIPTS_SRC)/generate-libdir-metafile.sh $(OBJECTS_DEST) $(IEMABS_NAME) \
		--description "a collection of objects written at IEM/KUG"
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



#------------------------------------------------------------------------------#
# IEMLIB
IEMLIB_NAME=iemlib
iemlib:


iemlib_install: iemlib
#	install -d $(OBJECTS_DEST)/$(IEMLIB_NAME)
#	install -d $(HELP_DEST)/$(IEMLIB_NAME)
	install -d $(HELP_DEST)/iemhelp
	install -p $(EXTERNALS_SRC)/iemlib/iemhelp/*.* $(HELP_DEST)/iemhelp
#	install -d $(MANUALS_DEST)/$(IEMLIB_NAME)
	install -d $(APPLICATIONS_DEST)/$(IEMLIB_NAME)
	install -p $(EXTERNALS_SRC)/iemlib/examples/*.* \
		$(APPLICATIONS_DEST)/$(IEMLIB_NAME)



#------------------------------------------------------------------------------#
# IEMMATRIX
IEMMATRIX_NAME=iemmatrix
iemmatrix:


iemmatrix_install: iemmatrix
	install -d $(OBJECTS_DEST)/$(IEMMATRIX_NAME)
	$(SCRIPTS_SRC)/generate-libdir-metafile.sh $(OBJECTS_DEST) $(IEMMATRIX_NAME) \
		--description "objects for matrix operations and math"
	mv $(OBJECTS_DEST)/mtx*.$(EXTENSION) $(OBJECTS_DEST)/$(IEMMATRIX_NAME)
	install -p $(shell ls -1 $(EXTERNALS_SRC)/iem/iemmatrix/abs/*.* | \
		grep -v '\-help.pd') $(OBJECTS_DEST)/$(IEMMATRIX_NAME)
	install -d $(HELP_DEST)/$(IEMMATRIX_NAME)
	install -p $(EXTERNALS_SRC)/iem/iemmatrix/doc/*.pd \
		$(HELP_DEST)/$(IEMMATRIX_NAME)



#------------------------------------------------------------------------------#
# PDDP
PDDP_NAME=pddp
# pddp is compiled straight into $(OUT_DIR)
PDDP_OUT_DIR=$(OBJECTS_DEST)/$(PDDP_NAME)
pddp:


pddp_install: pddp
	install -d $(OBJECTS_DEST)/$(PDDP_NAME)
	$(SCRIPTS_SRC)/generate-libdir-metafile.sh $(OBJECTS_DEST) $(PDDP_NAME) \
		--author "PDDP" \
		--license "BSD" \
		--description "objects for working with Tcl and Pd's Tk GUI"
	cd $(EXTERNALS_SRC)/miXed/pddp && $(MAKE) OUT_DIR=$(PDDP_OUT_DIR)
#	install -d $(HELP_DEST)/$(PDDP_NAME)
#	install -d $(MANUALS_DEST)/$(PDDP_NAME)
	install -d $(APPLICATIONS_DEST)/$(PDDP_NAME)
	install -p $(EXTERNALS_SRC)/miXed/test/pddp/*.*  \
		$(APPLICATIONS_DEST)/$(PDDP_NAME)



#------------------------------------------------------------------------------#
# PDP
PDP_NAME=pdp
pdp:
#	cd $(EXTERNALS_SRC)/pdp && ./configure && $(MAKE)
#TODO: compile pidip here
#TODO: compile gem2pdp here

pdp_install: pdp
#	install -d $(OBJECTS_DEST)/$(PDP_NAME)
#	install -p $(EXTERNALS_SRC)/pdp/*.$(EXTENSION) $(OBJECTS_DEST)
#	install -p $(EXTERNALS_SRC)/pidip/*.$(EXTENSION) $(OBJECTS_DEST)
#	install -p $(EXTERNALS_SRC)/gem2pdp/*.$(EXTENSION) $(OBJECTS_DEST)
	install -p $(EXTERNALS_SRC)/pdp/abstractions/*.pd $(OBJECTS_DEST)
#	install -d $(HELP_DEST)/$(PDP_NAME)
	install -p $(EXTERNALS_SRC)/pdp/doc/objects/*.* $(HELP_DEST)
	install -d $(MANUALS_DEST)/$(PDP_NAME)
	install -p $(EXTERNALS_SRC)/pdp/doc/reference.txt $(MANUALS_DEST)/$(PDP_NAME)
	install -p $(EXTERNALS_SRC)/pdp/doc/introduction/*.* \
		$(MANUALS_DEST)/$(PDP_NAME)
	install -d $(APPLICATIONS_DEST)/$(PDP_NAME)
	install -p $(EXTERNALS_SRC)/pdp/doc/examples/*.* \
		$(APPLICATIONS_DEST)/$(PDP_NAME)



#------------------------------------------------------------------------------#
# PMPD
PMPD_NAME=pmpd
pmpd:


pmpd_install: pmpd
#	install -d $(OBJECTS_DEST)/$(PMPD_NAME)
	install -d $(HELP_DEST)/$(PMPD_NAME)
	install -p $(EXTERNALS_SRC)/pmpd/help/*.pd \
		 $(HELP_DEST)/$(PMPD_NAME)
	install -d $(MANUALS_DEST)/$(PMPD_NAME)
	install -d $(MANUALS_DEST)/$(PMPD_NAME)
	install -p $(EXTERNALS_SRC)/pmpd/doc/pmpd.pdf/pmpd.pdf \
		$(MANUALS_DEST)/$(PMPD_NAME)
	install -d $(APPLICATIONS_DEST)/$(PMPD_NAME)
	install -p $(EXTERNALS_SRC)/pmpd/exemples/*.pd \
		$(APPLICATIONS_DEST)/$(PMPD_NAME)



#------------------------------------------------------------------------------#
# TOXY
TOXY_NAME=toxy
# toxy is compiled straight into $(OUT_DIR)
TOXY_OUT_DIR=$(OBJECTS_DEST)/$(TOXY_NAME)
toxy:


toxy_install: toxy
	install -d $(OBJECTS_DEST)/$(TOXY_NAME)
	$(SCRIPTS_SRC)/generate-libdir-metafile.sh $(OBJECTS_DEST) $(TOXY_NAME) \
		--author "Kzrysztof Czaja" \
		--license "BSD" \
		--description "objects for working with Tcl and Pd's Tk GUI"
	cd $(EXTERNALS_SRC)/miXed/toxy && $(MAKE) OUT_DIR=$(TOXY_OUT_DIR)
	install -d $(HELP_DEST)/$(TOXY_NAME)
	install -p $(EXTERNALS_SRC)/miXed/doc/help/toxy/*.*  \
		$(HELP_DEST)/$(TOXY_NAME)
#	install -d $(MANUALS_DEST)/$(TOXY_NAME)
	install -d $(APPLICATIONS_DEST)/$(TOXY_NAME)
	install -p $(EXTERNALS_SRC)/miXed/test/toxy/*.*  \
		$(APPLICATIONS_DEST)/$(TOXY_NAME)



#------------------------------------------------------------------------------#
# VBAP
VBAP_NAME=vbap
vbap:


vbap_install: vbap
	install -d $(OBJECTS_DEST)/$(VBAP_NAME)
#	install -d $(HELP_DEST)/$(VBAP_NAME)
#	install -d $(MANUALS_DEST)/$(VBAP_NAME)
#	install -d $(APPLICATIONS_DEST)/$(VBAP_NAME)
	install -p $(EXTERNALS_SRC)/vbap/graph-to-aziele.pd \
		$(OBJECTS_DEST)/$(VBAP_NAME)



#------------------------------------------------------------------------------#
# Zexy
ZEXY_NAME = zexy
zexy:
#  zexy's new build system doesn't work with CVS layout, and most of zexy is
#  included in the extrnals build system now anyway.  But the abstractions
#  should still be included
#	cd $(EXTERNALS_SRC)/zexy/src && autoconf && ./configure && $(MAKE)


zexy_install: zexy
	install -d $(OBJECTS_DEST)/$(ZEXY_NAME)
#	install -p $(EXTERNALS_SRC)/zexy/src/*.$(EXTENSION) $(OBJECTS_DEST)/$(ZEXY_NAME)
	install -p $(shell ls -1 $(EXTERNALS_SRC)/zexy/abs/*.pd | \
		grep -v '-help.pd' | sed 's/\([&<>|~]\)/\\\1/g') \
		$(OBJECTS_DEST)/$(ZEXY_NAME)
	install -d $(HELP_DEST)/$(ZEXY_NAME)
	install -p $(EXTERNALS_SRC)/zexy/abs/*-help.pd $(HELP_DEST)/$(ZEXY_NAME)
	install -p $(EXTERNALS_SRC)/zexy/zexy.pd  $(HELP_DEST)
	install -p $(EXTERNALS_SRC)/zexy/examples/*.* $(HELP_DEST)/$(ZEXY_NAME)



#==============================================================================#
#
# UNFILTERED PROJECT TARGETS
#
#==============================================================================#
# this is not yet organized into libdirs, but just leftover <hans@at.or.at>

unfiltered_install:
#----------------------------------------------------------------------------
# hcs
	install -p $(shell ls -1 $(EXTERNALS_SRC)/hcs/*.pd | \
			 grep -v '\-help.pd') $(OBJECTS_DEST)
# hid 
# these don't work on Windows yet
	cd $(EXTERNALS_SRC)/hcs/hid && $(MAKE) 
	install -p $(EXTERNALS_SRC)/hcs/hid/hid.$(EXTENSION) $(OBJECTS_DEST)
	install -p $(shell ls -1 $(EXTERNALS_SRC)/hcs/hid/*.pd | \
			 grep -v '\-help.pd'| grep -v '\-list.pd') $(OBJECTS_DEST)
# general
	install -p $(EXTERNALS_SRC)/hcs/general/*.pd $(OBJECTS_DEST)
# pan
	install -p $(shell ls -1 $(EXTERNALS_SRC)/hcs/pan/*.pd | \
			 grep -v '\-help.pd') $(OBJECTS_DEST)
#----------------------------------------------------------------------------
# OSC
	test -e cd $(EXTERNALS_SRC)/OSCx/Makefile || \
		(cd $(EXTERNALS_SRC)/OSCx && ./configure)
# these don't work on Windows yet
	cd $(EXTERNALS_SRC)/OSCx && $(MAKE) CC=gcc
	install -p $(EXTERNALS_SRC)/OSCx/doc/*.* $(HELP_DEST)
	install -p $(EXTERNALS_SRC)/OSCx/src/*.$(EXTENSION) $(OBJECTS_DEST)
#----------------------------------------------------------------------------
# unauthorized
# these don't work on Windows yet
	cd $(EXTERNALS_SRC)/unauthorized && $(MAKE) 
	install -p $(EXTERNALS_SRC)/unauthorized/*/*.$(EXTENSION) $(OBJECTS_DEST)
	install -p $(EXTERNALS_SRC)/unauthorized/*/*.pd $(HELP_DEST)
	install -p $(EXTERNALS_SRC)/unauthorized/*/*.txt $(HELP_DEST)
	install -p $(EXTERNALS_SRC)/unauthorized/*/*.pls $(HELP_DEST)
	cp -Rfp $(EXTERNALS_SRC)/unauthorized/blinkenlights/blm  $(HELP_DEST)


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
	cd $(EXTERNALS_SRC)/build/$(OS_NAME) && make $(DEST_PATHS) clean
	cd $(EXTERNALS_SRC)/hcs/hid && make $(DEST_PATHS) clean
	cd $(EXTERNALS_SRC)/OSCx && make $(DEST_PATHS) clean
	cd $(EXTERNALS_SRC)/mixed/cyclone && make $(DEST_PATHS) clean
	cd $(EXTERNALS_SRC)/mixed/pddp && make $(DEST_PATHS) clean
	cd $(EXTERNALS_SRC)/mixed/toxy && make $(DEST_PATHS) clean
	cd $(EXTERNALS_SRC)/unauthorized && make $(DEST_PATHS) clean

distclean: clean cruft_clean
	rm $(EXTERNALS_SRC)/OSCx Makefile
