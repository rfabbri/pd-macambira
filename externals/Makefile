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
cvs_root_dir = $(CWD)/..
prefix = $(CWD)/build
BUILDLAYOUT_DIR = $(CWD)

# default target
default: all

.SUFFIXES: .$(EXTENSION)


include $(BUILDLAYOUT_DIR)/Makefile.buildlayout


# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				cvs_root_dir=$(cvs_root_dir) \
				prefix=$(prefix)

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
	-cd $(externals_src)/build/$(OS_NAME) && make -k 

# these targets are for platform-specific needs that run before objects:
pre_all_darwin:

pre_all_linux:

pre_all_win:

pre_all_unknown:
# this target is for "everything else"


#------------------------------------------------------------------------------#
# INSTALL
install: all $(objectsdir) help_install objects_install unfiltered_install \
 creb_install cyclone_install iemabs_install iemabs_install iemmatrix_install \
 pddp_install pdp_install pmpd_install toxy_install vbap_install zexy_install
	@echo " "
	@echo "externals install succeeded!"

objects_install: 
	cd $(externals_src)/build/$(OS_NAME)/ && \
		install -p *.$(EXTENSION) $(objectsdir)


#==============================================================================#
#
# INSTALL TARGETS
#
#==============================================================================#


help_install: $(helpdir)
#------------------------------------------------------------------------------#
# all standard objs' help files
#    it had to be broken up because the list is soo long
	install -p \
			$(externals_src)/OSCx/doc/*.pd \
			$(externals_src)/aenv~/*.pd \
			$(externals_src)/arraysize/*.pd \
			$(externals_src)/beatpipe/*.pd \
			$(externals_src)/control/*/*.pd \
			$(externals_src)/bbogart/chaos/tools/*.pd \
			$(externals_src)/bbogart/*/*.pd \
			$(externals_src)/creb/doc/*.pd \
				$(helpdir)
	install -p \
			$(externals_src)/creb/doc/examples/*.pd \
			$(externals_src)/cxc/reference/*.pd \
			$(externals_src)/dfx/*/*.pd \
			$(externals_src)/ext13/doc/*.pd \
			$(externals_src)/ggee/*/*-help.pd \
			$(externals_src)/gem2pdp/*.pd \
			$(externals_src)/ff/*.pd \
			$(externals_src)/freeverb~/*.pd \
			$(externals_src)/hcs/*-help.pd \
				$(helpdir)
	install -p \
			$(externals_src)/hcs/*/doc/*.pd \
			$(externals_src)/hcs/hid/examples/*.pd \
			$(externals_src)/hcs/pan/*-help.pd \
			$(externals_src)/iem/comport/*/*-help.pd \
			$(externals_src)/markex/*.pd \
			$(externals_src)/motex/*.pd \
			$(externals_src)/nusmuk/line3/*.pd\
				$(helpdir)
	install -p \
			$(externals_src)/pdogg/*/*.pd \
			$(externals_src)/plugin~/*.pd \
			$(externals_src)/rhythm_estimator/*.p? \
			$(externals_src)/signal/*/*.pd \
			$(externals_src)/sprinkler/sprinkler-help.pd \
			$(externals_src)/susloop~/*.pd \
			$(externals_src)/svf~/*.pd \
			$(externals_src)/vbap/*.pd \
			$(externals_src)/vst/*.pd \
			$(externals_src)/zhzxh~/*.pd \
				$(helpdir)
#------------------------------------------------------------------------------#
#  mjlib [prob] conflicts with cyclone's [prob] so filter it
	install -p $(shell ls -1 $(externals_src)/mjlib/doc/*.* | \
		grep -v 'prob') $(helpdir)
#------------------------------------------------------------------------------#
#  maxlib [split] conflicts with cyclone so filter it
	install -p $(shell ls -1 $(externals_src)/maxlib/help/*.* | \
		grep -v 'split') $(helpdir)



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
	install -d $(objectsdir)/$(TEMPLATE_NAME)
	install -d $(helpdir)/$(TEMPLATE_NAME)
	install -d $(manualsdir)/$(TEMPLATE_NAME)
	install -d $(examplesdir)/$(TEMPLATE_NAME)


#------------------------------------------------------------------------------#
# CREB
CREB_NAME=creb
creb:


creb_install: creb
	install -d $(objectsdir)/$(CREB_NAME)
	install -p $(externals_src)/creb/abs/*.pd $(objectsdir)/$(CREB_NAME)
	install -d $(helpdir)/$(CREB_NAME)
	install -p -m0644	$(externals_src)/creb/doc/examples/*.* \
		$(helpdir)/$(CREB_NAME)
#	install -d $(manualsdir)/$(CREB_NAME)
#	install -d $(examplesdir)/$(CREB_NAME)



#------------------------------------------------------------------------------#
# CYCLONE
CYCLONE_NAME=cyclone
# cyclone is compiled straight into $(OUT_DIR)
CYCLONE_OUT_DIR=$(objectsdir)/$(CYCLONE_NAME)
cyclone:


cyclone_install: cyclone
	install -d $(objectsdir)/$(CYCLONE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(CYCLONE_NAME) \
		--author "Kzrysztof Czaja" \
		--license "BSD" \
		--description "a library for porting and running Max/MSP patches in Pd"
	cd $(externals_src)/miXed/cyclone && $(MAKE) OUT_DIR=$(CYCLONE_OUT_DIR)
# install "cyclist" command line app with pd
	install -d $(bindir)
# this doesn't build in the right place yet
#	install -p -m0755 $(CYCLONE_OUT_DIR)/cyclist $(bindir) && \
#		rm $(CYCLONE_OUT_DIR)/cyclist
# rename cyclone lib since it will pre-empt libdir functionality
#	mv $(CYCLONE_OUT_DIR)/cyclone.$(EXTENSION) \
#		$(CYCLONE_OUT_DIR)/cyclonelib.$(EXTENSION)
	install -d $(helpdir)/$(CYCLONE_NAME)
	cd $(externals_src)/miXed/doc/help/cyclone && \
		install -p *.* $(helpdir)/$(CYCLONE_NAME)
#	install -d $(manualsdir)/$(CYCLONE_NAME)
	install -d $(examplesdir)/$(CYCLONE_NAME)
	install -p $(externals_src)/miXed/test/cyclone/*.*  \
		$(examplesdir)/$(CYCLONE_NAME)





#------------------------------------------------------------------------------#
# IEMABS
IEMABS_NAME=iemabs
iemabs:


iemabs_install: iemabs
#  iemabs still has the help patches in the old help-*.pd style
	install -d $(objectsdir)/$(IEMABS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEMABS_NAME) \
		--description "a collection of objects written at IEM/KUG"
	install -p $(shell ls -1 $(externals_src)/iemlib/iemabs/*.pd | grep -v 'help') \
		$(objectsdir)/$(IEMABS_NAME)
	install -d $(helpdir)/$(IEMABS_NAME)
	install -p $(externals_src)/iemlib/iemabs/*help*.pd \
		$(helpdir)/$(IEMABS_NAME)
	install -p $(externals_src)/iemlib/iemabs/*.mp3 $(helpdir)/$(IEMABS_NAME)
	install -p $(externals_src)/iemlib/iemabs/*.wav $(helpdir)/$(IEMABS_NAME)
	install -d $(manualsdir)/$(IEMABS_NAME)
	install -p $(externals_src)/iemlib/iemabs/*.txt \
		$(manualsdir)/$(IEMABS_NAME)



#------------------------------------------------------------------------------#
# IEMLIB
IEMLIB_NAME=iemlib
iemlib:


iemlib_install: iemlib
#	install -d $(objectsdir)/$(IEMLIB_NAME)
#	install -d $(helpdir)/$(IEMLIB_NAME)
	install -d $(helpdir)/iemhelp
	install -p $(externals_src)/iemlib/iemhelp/*.* $(helpdir)/iemhelp
#	install -d $(manualsdir)/$(IEMLIB_NAME)
	install -d $(examplesdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/examples/*.* \
		$(examplesdir)/$(IEMLIB_NAME)



#------------------------------------------------------------------------------#
# IEMMATRIX
IEMMATRIX_NAME=iemmatrix
iemmatrix:


iemmatrix_install: iemmatrix
	install -d $(objectsdir)/$(IEMMATRIX_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEMMATRIX_NAME) \
		--description "objects for matrix operations and math"
	mv $(objectsdir)/mtx*.$(EXTENSION) $(objectsdir)/$(IEMMATRIX_NAME)
	install -p $(shell ls -1 $(externals_src)/iem/iemmatrix/abs/*.* | \
		grep -v '\-help.pd') $(objectsdir)/$(IEMMATRIX_NAME)
	install -d $(helpdir)/$(IEMMATRIX_NAME)
	install -p $(externals_src)/iem/iemmatrix/doc/*.pd \
		$(helpdir)/$(IEMMATRIX_NAME)



#------------------------------------------------------------------------------#
# PDDP
PDDP_NAME=pddp
# pddp is compiled straight into $(OUT_DIR)
PDDP_OUT_DIR=$(objectsdir)/$(PDDP_NAME)
pddp:


pddp_install: pddp
	install -d $(objectsdir)/$(PDDP_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(PDDP_NAME) \
		--author "PDDP" \
		--license "BSD" \
		--description "objects for working with Tcl and Pd's Tk GUI"
	cd $(externals_src)/miXed/pddp && $(MAKE) OUT_DIR=$(PDDP_OUT_DIR)
#	install -d $(helpdir)/$(PDDP_NAME)
#	install -d $(manualsdir)/$(PDDP_NAME)
	install -d $(examplesdir)/$(PDDP_NAME)
	install -p $(externals_src)/miXed/test/pddp/*.*  \
		$(examplesdir)/$(PDDP_NAME)



#------------------------------------------------------------------------------#
# PDP
PDP_NAME=pdp
pdp:
#	cd $(externals_src)/pdp && ./configure && $(MAKE)
#TODO: compile pidip here
#TODO: compile gem2pdp here

pdp_install: pdp
#	install -d $(objectsdir)/$(PDP_NAME)
#	install -p $(externals_src)/pdp/*.$(EXTENSION) $(objectsdir)
#	install -p $(externals_src)/pidip/*.$(EXTENSION) $(objectsdir)
#	install -p $(externals_src)/gem2pdp/*.$(EXTENSION) $(objectsdir)
	install -p $(externals_src)/pdp/abstractions/*.pd $(objectsdir)
#	install -d $(helpdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/doc/objects/*.* $(helpdir)
	install -d $(manualsdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/doc/reference.txt $(manualsdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/doc/introduction/*.* \
		$(manualsdir)/$(PDP_NAME)
	install -d $(examplesdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/doc/examples/*.* \
		$(examplesdir)/$(PDP_NAME)



#------------------------------------------------------------------------------#
# PMPD
PMPD_NAME=pmpd
pmpd:


pmpd_install: pmpd
#	install -d $(objectsdir)/$(PMPD_NAME)
	install -d $(helpdir)/$(PMPD_NAME)
	install -p $(externals_src)/pmpd/help/*.pd \
		 $(helpdir)/$(PMPD_NAME)
	install -d $(manualsdir)/$(PMPD_NAME)
	install -d $(manualsdir)/$(PMPD_NAME)
	install -p $(externals_src)/pmpd/doc/pmpd.pdf/pmpd.pdf \
		$(manualsdir)/$(PMPD_NAME)
	install -d $(examplesdir)/$(PMPD_NAME)
	install -p $(externals_src)/pmpd/exemples/*.pd \
		$(examplesdir)/$(PMPD_NAME)



#------------------------------------------------------------------------------#
# TOXY
TOXY_NAME=toxy
# toxy is compiled straight into $(OUT_DIR)
TOXY_OUT_DIR=$(objectsdir)/$(TOXY_NAME)
toxy:


toxy_install: toxy
	install -d $(objectsdir)/$(TOXY_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(TOXY_NAME) \
		--author "Kzrysztof Czaja" \
		--license "BSD" \
		--description "objects for working with Tcl and Pd's Tk GUI"
# [tow] is currently broken, it doesn't compile
	-cd $(externals_src)/miXed/toxy && $(MAKE) OUT_DIR=$(TOXY_OUT_DIR)
	install -d $(helpdir)/$(TOXY_NAME)
	install -p $(externals_src)/miXed/doc/help/toxy/*.*  \
		$(helpdir)/$(TOXY_NAME)
#	install -d $(manualsdir)/$(TOXY_NAME)
	install -d $(examplesdir)/$(TOXY_NAME)
	install -p $(externals_src)/miXed/test/toxy/*.*  \
		$(examplesdir)/$(TOXY_NAME)



#------------------------------------------------------------------------------#
# VBAP
VBAP_NAME=vbap
vbap:


vbap_install: vbap
	install -d $(objectsdir)/$(VBAP_NAME)
#	install -d $(helpdir)/$(VBAP_NAME)
#	install -d $(manualsdir)/$(VBAP_NAME)
#	install -d $(examplesdir)/$(VBAP_NAME)
	install -p $(externals_src)/vbap/graph-to-aziele.pd \
		$(objectsdir)/$(VBAP_NAME)



#------------------------------------------------------------------------------#
# Zexy
ZEXY_NAME = zexy
zexy:
#  zexy's new build system doesn't work with CVS layout, and most of zexy is
#  included in the extrnals build system now anyway.  But the abstractions
#  should still be included
#	cd $(externals_src)/zexy/src && autoconf && ./configure && $(MAKE)


zexy_install: zexy
	install -d $(objectsdir)/$(ZEXY_NAME)
#	install -p $(externals_src)/zexy/src/*.$(EXTENSION) $(objectsdir)/$(ZEXY_NAME)
	install -p $(shell ls -1 $(externals_src)/zexy/abs/*.pd | \
		grep -v '-help.pd' | sed 's/\([&<>|~]\)/\\\1/g') \
		$(objectsdir)/$(ZEXY_NAME)
	install -d $(helpdir)/$(ZEXY_NAME)
	install -p $(externals_src)/zexy/abs/*-help.pd $(helpdir)/$(ZEXY_NAME)
	install -p $(externals_src)/zexy/zexy.pd  $(helpdir)
	install -p $(externals_src)/zexy/examples/*.* $(helpdir)/$(ZEXY_NAME)



#==============================================================================#
#
# UNFILTERED PROJECT TARGETS
#
#==============================================================================#
# this is not yet organized into libdirs, but just leftover <hans@at.or.at>

unfiltered_install:
#----------------------------------------------------------------------------
# hcs
	install -p $(shell ls -1 $(externals_src)/hcs/*.pd | \
			 grep -v '\-help.pd') $(objectsdir)
# hid 
# these don't work on Windows yet
	-cd $(externals_src)/hcs/hid && $(MAKE) 
	-install -p $(externals_src)/hcs/hid/hid.$(EXTENSION) $(objectsdir)
	install -p $(shell ls -1 $(externals_src)/hcs/hid/*.pd | \
			 grep -v '\-help.pd'| grep -v '\-list.pd') $(objectsdir)
# general
	install -p $(externals_src)/hcs/general/*.pd $(objectsdir)
# pan
	install -p $(shell ls -1 $(externals_src)/hcs/pan/*.pd | \
			 grep -v '\-help.pd') $(objectsdir)
#----------------------------------------------------------------------------
# OSC
	test -e $(externals_src)/OSCx/Makefile || \
		(cd $(externals_src)/OSCx && ./configure)
# these don't work on Windows yet
	-cd $(externals_src)/OSCx && $(MAKE) CC=gcc
	-install -p $(externals_src)/OSCx/src/*.$(EXTENSION) $(objectsdir)
	install -p $(externals_src)/OSCx/doc/*.* $(helpdir)
#----------------------------------------------------------------------------
# unauthorized
# these don't work on Windows yet
	-cd $(externals_src)/unauthorized && $(MAKE) 
	-install -p $(externals_src)/unauthorized/*/*.$(EXTENSION) $(objectsdir)
	install -p $(externals_src)/unauthorized/*/*.pd $(helpdir)
	install -p $(externals_src)/unauthorized/*/*.txt $(helpdir)
	install -p $(externals_src)/unauthorized/*/*.pls $(helpdir)
	cp -Rfp $(externals_src)/unauthorized/blinkenlights/blm  $(helpdir)


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
	cd $(externals_src)/build/$(OS_NAME) && make $(DEST_PATHS) clean
	cd $(externals_src)/hcs/hid && make $(DEST_PATHS) clean
	cd $(externals_src)/OSCx && make $(DEST_PATHS) clean
	cd $(externals_src)/mixed/cyclone && make $(DEST_PATHS) clean
	cd $(externals_src)/mixed/pddp && make $(DEST_PATHS) clean
	cd $(externals_src)/mixed/toxy && make $(DEST_PATHS) clean
	cd $(externals_src)/unauthorized && make $(DEST_PATHS) clean

distclean: clean cruft_clean
	rm $(externals_src)/OSCx Makefile
