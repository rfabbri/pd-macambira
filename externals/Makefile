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
DESTDIR = $(CWD)/build/
BUILDLAYOUT_DIR = $(cvs_root_dir)/packages

# default target
default: all

.SUFFIXES: .$(EXTENSION)


include $(BUILDLAYOUT_DIR)/Makefile.buildlayout


# these are sent to all of the various Makefiles so that they all copy their
# output to the same directory tree
DEST_PATHS = BUILDLAYOUT_DIR=$(BUILDLAYOUT_DIR) \
				cvs_root_dir=$(cvs_root_dir) \
				DESTDIR=$(DESTDIR) \
				prefix=$(prefix)

#==============================================================================#
#
# COMPILE TARGETS
#
#==============================================================================#

CFLAGS = -DPD -DUNIX -Dunix $(OPTIM_FLAGS) \
    -Wall -W -Wno-unused -Wno-parentheses -Wno-switch -Wno-shadow 
INCLUDES = -I$(pd_src)/src
LDFLAGS =  
STRIP = strip --strip-unneeded -R .note -R .comment

#------------------------------------------------------------------------------#
# DARWIN
DARWIN_CFLAGS = $(CFLAGS)
DARWIN_INCLUDES = $(INCLUDES) -I/sw/include
DARWIN_LDFLAGS = $(LDFLAGS) -bundle -bundle_loader $(pd_src)/bin/pd -L/sw/lib
%.pd_darwin: %.c
	$(CC) $(DARWIN_CFLAGS) $(DARWIN_INCLUDES) -o "$*.o" -c "$*.c"
	$(CC) $(DARWIN_LDFLAGS) -o "$*.pd_darwin" "$*.o" -lc -lm
	chmod a-x "$*.pd_darwin"
	rm -f "$*.o" 


#------------------------------------------------------------------------------#
# LINUX
LINUX_CFLAGS = $(CFLAGS) -fPIC
LINUX_INCLUDES = $(INCLUDES)
LINUX_LDFLAGS = $(LDFLAGS) -Wl,-export_dynamic  -shared
%.pd_linux: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -o "$*.o" -c "$*.c"
	gcc  $(LINUX_LDFLAGS) -o "$*.pd_linux" "$*.o" -lc -lm 
	chmod a-x "$*.pd_linux"
	$(STRIP) $*.pd_linux
	rm -f "$*.o" 


#------------------------------------------------------------------------------#
# WIN (MinGW)
WIN_CFLAGS = -mms-bitfields $(CFLAGS)
WIN_INCLUDES = $-I. -I.. -I$(PD_PATH)/src -IC:/msys/1.0/include
WIN_LDFLAGS = $(LDFLAGS) -shared -LC:/msys/1.0/lib -L$(PD_PATH)/bin -lpd
# these are for compatibility
WIN_DEFINES = \
	-D'drand48()=((double)rand()/RAND_MAX)' \
	-D'srand48(n)=srand((n))' \
	-D'O_NONBLOCK=1' \
# These don't seem to be needed:
#	-D'bzero(p,n)=memset(p,0,n)' \
#	-D'PROT_READ=1' \
#	-D'MAP_PRIVATE=2' \
#	-D'O_NDELAY=O_NONBLOCK'
%.dll: %.c
	$(CC) $(WIN_CFLAGS) $(WIN_DEFINES) $(WIN_INCLUDES) \
		 -o "$*.o" -c "../src/$*.c"
	gcc $(LDFLAGS) -o "$*.dll" "$*.o" 
	chmod a-x "$*.pd_linux"
	$(STRIP) "$*.dll"
	rm "$*.o"




#------------------------------------------------------------------------------#
# ALL
all: pre_all_$(OS_NAME) creb cxc cyclone ext13 iemabs iemlib iemmatrix pddp \
pdp pmpd smlib toxy vbap zexy
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
creb_install cxc_install cyclone_install ext13_install iemabs_install \
iemabs_install iemlib_install iemmatrix_install pddp_install pdp_install \
pmpd_install toxy_install unauthorized_install vbap_install zexy_install
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
			$(externals_src)/dfx/*/*.pd \
			$(externals_src)/ggee/*/*-help.pd \
			$(externals_src)/ggee/*/*.gif \
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
TEMPLATE_OBJECTS = $(wildcard $(externals_src)/template/*.c)
template: $(TEMPLATE_OBJECTS:.c=.$(EXTENSION))

template_install: template
	install -d $(objectsdir)/$(TEMPLATE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(TEMPLATE_NAME) \
		--author "" \
		--description "" \
		--license "" \
		--version ""
	install -p $(TEMPLATE_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(TEMPLATE_NAME)
	install -d $(helpdir)/$(TEMPLATE_NAME)
	install -p $(wildcard $(externals_src)/template/help/*.pd) \
		$(examplesdir)/$(TEMPLATE_NAME)
	install -d $(manualsdir)/$(TEMPLATE_NAME)
	install -p $(externals_src)/template/README.txt $(manualsdir)/$(TEMPLATE_NAME)
	install -d $(examplesdir)/$(TEMPLATE_NAME)
	install -p $(wildcard $(externals_src)/template/examples/*.pd) \
		$(examplesdir)/$(TEMPLATE_NAME)

template_clean:
	-rm -f -- $(externals_src)/template/*.$(EXTENSION)
	-rm -f -- $(externals_src)/template/*.bak


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

creb_clean:
	-rm -f -- $(externals_src)/creb/*.$(EXTENSION)
	-rm -f -- $(externals_src)/creb/*.bak


#------------------------------------------------------------------------------#
# CXC
CXC_NAME=cxc
CXC_OBJECTS = $(wildcard $(externals_src)/cxc/???*.c)
cxc: $(CXC_OBJECTS:.c=.$(EXTENSION))

cxc_install: cxc
	install -d $(objectsdir)/$(CXC_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(CXC_NAME) \
		--author "jdl@xdv.org" \
		--description "" \
		--license "" \
		--version ""
	install -p $(CXC_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(CXC_NAME)
	install -d $(helpdir)/$(CXC_NAME)
	install -p $(wildcard $(externals_src)/cxc/reference/*.pd) \
		$(helpdir)/$(CXC_NAME)
	install -d $(manualsdir)/$(CXC_NAME)
	install -p $(externals_src)/cxc/README \
		 $(manualsdir)/$(CXC_NAME)/README.txt

cxc_clean:
	-rm -f -- $(SMLIB_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(externals_src)/cxc/*.$(EXTENSION)
	-rm -f -- $(externals_src)/cxc/*.bak

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
	cd $(externals_src)/miXed/test/cyclone && \
		install -p *.* $(examplesdir)/$(CYCLONE_NAME)



#------------------------------------------------------------------------------#
# EXT13
EXT13_NAME=ext13
# sfread.c doesn't compile, so exclude it
EXT13_OBJECTS = $(wildcard $(externals_src)/ext13/???????*.c) \
	$(externals_src)/ext13/ftos.c $(externals_src)/ext13/send13.c 
ext13: $(EXT13_OBJECTS:.c=.$(EXTENSION))

ext13_test:
	@echo "objects: $(EXT13_OBJECTS)"


ext13_install: ext13
	install -d $(objectsdir)/$(EXT13_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(EXT13_NAME) \
		--author "d13@klingt.org" \
		--version "0.17"
	install -p $(EXT13_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(EXT13_NAME)
	install -d $(helpdir)/$(EXT13_NAME)
	install -p $(wildcard $(externals_src)/ext13/doc/*.pd) \
		$(helpdir)/$(EXT13_NAME)
	install -d $(manualsdir)/$(EXT13_NAME)
	install -p $(externals_src)/ext13/README \
		$(manualsdir)/$(EXT13_NAME)/README.txt

ext13_clean:
	-rm -f -- $(externals_src)/ext13/*.$(EXTENSION)
	-rm -f -- $(externals_src)/ext13/*.bak
	-rm -f -- $(externals_src)/ext13/*~



#------------------------------------------------------------------------------#
# IEMABS
IEMABS_NAME=iemabs
iemabs:


iemabs_install: iemabs
#  iemabs still has the help patches in the old help-*.pd style
	install -d $(objectsdir)/$(IEMABS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEMABS_NAME) \
		--description "a collection of objects written at IEM/KUG"
	cd $(externals_src)/iemlib/iemabs/ && \
		install -p $(shell cd $(externals_src)/iemlib/iemabs/ && ls -1 *.pd | \
			grep -v 'help') $(objectsdir)/$(IEMABS_NAME)
	install -d $(helpdir)/$(IEMABS_NAME)
	cd $(externals_src)/iemlib/iemabs/ && \
		install -p *help*.pd $(helpdir)/$(IEMABS_NAME)
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
	install -d $(objectsdir)/$(PDP_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(PDP_NAME) \
		--author "Tom Schouten" \
		--description "Pure Data Packet" \
		--license "GNU GPL"
	-install -p $(externals_src)/pdp/*.$(EXTENSION) $(objectsdir)/$(PDP_NAME)
	-install -p $(externals_src)/gem2pdp/*.$(EXTENSION) $(objectsdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/abstractions/*.pd $(objectsdir)/$(PDP_NAME)
	install -d $(helpdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/doc/objects/*.* $(helpdir)/$(PDP_NAME)
	install -d $(manualsdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/doc/reference.txt $(manualsdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/doc/introduction/*.* \
		$(manualsdir)/$(PDP_NAME)
	install -d $(examplesdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/doc/examples/*.* \
		$(examplesdir)/$(PDP_NAME)


#------------------------------------------------------------------------------#
# PIDIP
PIDIP_NAME=pidip
pidip:
#	cd $(externals_src)/pidip && ./configure && $(MAKE)

pidip_install: pidip
	install -d $(objectsdir)/$(PIDIP_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(PIDIP_NAME) \
		--author "Yves Degoyon" \
		--description "PiDiP is Definitely in Pieces"
	-install -p $(externals_src)/pidip/*.$(EXTENSION) $(objectsdir)/$(PIDIP_NAME)
	install -d $(helpdir)/$(PIDIP_NAME)
	install -p $(externals_src)/pidip/doc/*.pd $(helpdir)/$(PIDIP_NAME)
	install -d $(examplesdir)/$(PIDIP_NAME)
	install -p $(externals_src)/pidip/patches/*.* $(examplesdir)/$(PIDIP_NAME)
	install -d $(examplesdir)/$(PIDIP_NAME)/images
	install -p $(externals_src)/pidip/patches/images/*.* \
		$(examplesdir)/$(PIDIP_NAME)/images
	install -d $(examplesdir)/$(PIDIP_NAME)/morphology
	install -p $(externals_src)/pidip/patches/morphology/*.* \
		$(examplesdir)/$(PIDIP_NAME)/morphology


#------------------------------------------------------------------------------#
# PMPD
PMPD_NAME=pmpd
pmpd:


pmpd_install: pmpd
#	install -d $(objectsdir)/$(PMPD_NAME)
	install -d $(helpdir)/$(PMPD_NAME)
	install -p $(externals_src)/pmpd/help/*.pd $(helpdir)
	install -d $(manualsdir)/$(PMPD_NAME)
	install -d $(manualsdir)/$(PMPD_NAME)
	install -p $(externals_src)/pmpd/doc/pmpd.pdf/pmpd.pdf \
		$(manualsdir)/$(PMPD_NAME)
	install -d $(examplesdir)/$(PMPD_NAME)
	install -p $(externals_src)/pmpd/exemples/*.pd \
		$(examplesdir)/$(PMPD_NAME)


#------------------------------------------------------------------------------#
# SMLIB
SMLIB_NAME=smlib
# exclude SMlib.c since its just for the compiled library
SMLIB_OBJECTS = $(wildcard $(externals_src)/smlib/source/[a-z]*.c)
smlib: $(SMLIB_OBJECTS:.c=.$(EXTENSION))

smlib_install: smlib
	install -d $(objectsdir)/$(SMLIB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(SMLIB_NAME) \
		--author "Johannes Taelman <johannes.taelman@rug.ac.be>" \
		--license "GNU GPL" \
		--description "vector processing, vector analysis, vector synthesis, number stream analysis, number stream filters"
	install -p $(SMLIB_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(SMLIB_NAME)
	install -d $(helpdir)/$(SMLIB_NAME)
	install -p $(wildcard $(externals_src)/smlib/help/*.pd )\
		$(helpdir)/$(SMLIB_NAME)
	install -d $(manualsdir)/$(SMLIB_NAME)
	install -p $(externals_src)/smlib/readme.txt $(manualsdir)/$(SMLIB_NAME)
	install -d $(examplesdir)/$(SMLIB_NAME)
	install -p $(wildcard $(externals_src)/smlib/examples/*.pd) \
		$(examplesdir)/$(SMLIB_NAME)

smlib_clean:
	-rm -f -- $(SMLIB_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(externals_src)/smlib/*.bak
	-rm -f -- $(externals_src)/smlib/*~


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
	cd $(externals_src)/miXed/toxy && $(MAKE) OUT_DIR=$(TOXY_OUT_DIR)
	install -d $(helpdir)/$(TOXY_NAME)
	install -p $(externals_src)/miXed/doc/help/toxy/*.*  \
		$(helpdir)/$(TOXY_NAME)
#	install -d $(manualsdir)/$(TOXY_NAME)
	install -d $(examplesdir)/$(TOXY_NAME)
	install -p $(externals_src)/miXed/test/toxy/*.*  \
		$(examplesdir)/$(TOXY_NAME)


#------------------------------------------------------------------------------#
# UNAUTHORIZED
UNAUTHORIZED_NAME=unauthorized
# unauthorized is compiled straight into $(OUT_DIR)
UNAUTHORIZED_OUT_DIR=$(objectsdir)/$(UNAUTHORIZED_NAME)
unauthorized:


unauthorized_install: unauthorized
# this needs to go here since it produces errors
	-cd $(externals_src)/unauthorized && $(MAKE) -k
	install -d $(objectsdir)/$(UNAUTHORIZED_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(UNAUTHORIZED_NAME) \
		--author "Yves Degoyon" \
		--license "GNU GPL" \
		--description "GUI and streaming objects"
	install -p $(externals_src)/unauthorized/*/*.$(EXTENSION) \
		$(objectsdir)/$(UNAUTHORIZED_NAME)
	install -d $(helpdir)/$(UNAUTHORIZED_NAME)
	install -p $(externals_src)/unauthorized/*/*.pd \
		$(helpdir)/$(UNAUTHORIZED_NAME)
	install -p $(externals_src)/unauthorized/*/*.txt \
		$(helpdir)/$(UNAUTHORIZED_NAM)
	install -p $(externals_src)/unauthorized/*/*.pls \
		$(helpdir)/$(UNAUTHORIZED_NAME)
	install -d $(helpdir)/$(UNAUTHORIZED_NAME)/blm
	install -p $(externals_src)/unauthorized/blinkenlights/blm/*.* \
		$(helpdir)/$(UNAUTHORIZED_NAME)/blm


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
	install -p $(shell ls -1 $(externals_src)/zexy/abs/*.pd | grep -v '-help.pd' | sed 's|\(.*\)|"\1"|g') \
		$(objectsdir)/$(ZEXY_NAME)
#		grep -v '-help.pd' | sed 's/\([&<>|~]\)/\\\1/g') \

	install -d $(helpdir)/$(ZEXY_NAME)
	install -p $(shell ls -1 $(externals_src)/zexy/abs/*-help.pd | sed 's|\(.*\)|"\1"|g') \
		$(helpdir)/$(ZEXY_NAME)
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
clean: creb_clean cxc_clean ext13_clean smlib_clean install_clean
	cd $(externals_src)/build/$(OS_NAME) && make $(DEST_PATHS) clean
	cd $(externals_src)/hcs/hid && make $(DEST_PATHS) clean
	cd $(externals_src)/OSCx && make $(DEST_PATHS) clean
	cd $(externals_src)/mixed/cyclone && make $(DEST_PATHS) clean
	cd $(externals_src)/mixed/pddp && make $(DEST_PATHS) clean
	cd $(externals_src)/mixed/toxy && make $(DEST_PATHS) clean
	cd $(externals_src)/unauthorized && make clean

distclean: clean cruft_clean
	rm $(externals_src)/OSCx Makefile
