
#==============================================================================#
#
# Centralized build system for "externals".  
#
# see README for instructions  <hans@at.or.at>
# http://puredata.org/docs/developer/build
#
#==============================================================================#

CWD := $(shell pwd)

# these are setup to be overridden by the packages/Makefile
cvs_root_dir := $(shell cd $(CWD)/.. && pwd)
DESTDIR = $(CWD)/build/
BUILDLAYOUT_DIR = $(cvs_root_dir)/packages

# turn on weak linking and dlopen support
export MACOSX_DEPLOYMENT_TARGET = 10.3

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

# this variable is to support old "win" directories, rather than "windows"
BUILDSRC_OS_NAME = $(OS_NAME)

CFLAGS = -DPD $(OPT_CFLAGS) -I$(pd_src)/src -Wall -W $(DEBUG_CFLAGS)
LDFLAGS =  
LIBS = -lm

ifeq ($(OS_NAME),darwin)
  CFLAGS += -I/sw/include -DMACOSX -DUNIX -Dunix
  LDFLAGS += -bundle -bundle_loader $(pd_src)/bin/pd -L/sw/lib
  LIBS += -lc 
  DYLIB_LDFLAGS = -dynamiclib -read_only_relocs warning -L/sw/lib
  DYLIB_EXTENSION = dylib
  STRIP = strip -x
endif
ifeq ($(OS_NAME),linux)
  CFLAGS +=  -DUNIX -Dunix -fPIC
  LDFLAGS += -Wl,-export_dynamic  -shared
  LIBS += -lc
  DYLIB_LDFLAGS = $(LDFLAGS)
  DYLIB_EXTENSION = so
  STRIP = strip --strip-unneeded -R .note -R .comment
endif
ifeq ($(OS_NAME),windows)
  BUILDSRC_OS_NAME = win
  WINDOWS_HACKS = -D'O_NONBLOCK=1' -D'srand48(n)=srand((n))' \
    -D'drand48()=((double)rand()/RAND_MAX)' -D'bzero(p,n)=memset(p,0,n)'
# These don't seem to be needed:
#	-D'PROT_READ=1' \
#	-D'MAP_PRIVATE=2' \
#	-D'O_NDELAY=O_NONBLOCK'
  CFLAGS += -mms-bitfields -DMSW -DNT $(WINDOWS_HACKS)
  LDFLAGS += -shared
# all of these included libs are part of libc in UNIX platforms.  All except
# libregex are in DLLs, so they get stripped from the external's .dll binary
  LIBS += -L$(pd_src)/bin  -L$(pd_src)/obj  -lpd \
    -lwsock32 -lpthreadGC2 -lkernel32 -luser32 -lgdi32 -lregex
  DYLIB_LDFLAGS = -shared
  DYLIB_EXTENSION = dll
  STRIP = strip --strip-unneeded -R .note -R .comment
endif

CXXFLAGS = $(CFLAGS)

### C++ files
%.$(EXTENSION): %.cpp
%.$(EXTENSION): %.cc
	$(CXX) $(CXXFLAGS) -o "$*.o" -c "$<"
	$(CXX) $(LDFLAGS) -o "$*.$(EXTENSION)" "$*.o" $(LIBS)
	chmod a-x "$*.$(EXTENSION)"
#	$(STRIP) $*.$(EXTENSION)
#	rm -f -- $*.o

%.o: %.cpp
%.o: %.cc
	$(CXX) $(CXXFLAGS) -o "$*.o" -c "$<"


### C files
%.o: %.c
	$(CC) $(CFLAGS) -o "$*.o" -c "$*.c"

%.$(EXTENSION): %.o
	$(CC) $(LDFLAGS) -o "$*.$(EXTENSION)" "$*.o"  $(LIBS) \
		`test -f $*.libs && cat $*.libs`	\
		`test -f $(dir $*)../$(BUILDSRC_OS_NAME)/$(notdir $*).libs && \
			cat $(dir $*)../$(BUILDSRC_OS_NAME)/$(notdir $*).libs`
	chmod a-x "$*.$(EXTENSION)"
	$(STRIP) $*.$(EXTENSION)
	rm -f -- $*.o




#------------------------------------------------------------------------------#
# ALL

# if your library isn't included in LIB_TARGETS, it won't be built with
# Pd-extended.  For libraries that build on all platforms, add them directly
# below, otherwise add to the correct platforms below.

#
# WARNING!  this MUST be all on one line because the automatic package
# building scripts rely on it being that way.
LIB_TARGETS = adaptive boids bsaylor creb cxc cyclone deprecated earplug ekext ext13 flashserver flatspace flib freeverb ggee hardware hcs iem_ambi iem_bin_ambi iemlib iemgui iem_adaptfilt iemmatrix iem_matrix iemxmlrpc iem_delay iem_roomsim iem_spec2 iem_tab jasch_lib loaders mapping markex maxlib mjlib moonlib motex mrpeach msd oscx pan pdcontainer pddp pdogg pmpd sigpack smlib toxy unauthorized vbap zexy

# this is for libraries that don't compile (yet) on all platforms
ifeq ($(OS_NAME),windows)
  LIB_TARGETS += 
else
  ifeq ($(OS_NAME),darwin)
    LIB_TARGETS += hid hidio pidip wiiremote gem2pdp iem16
  else
    # GNU/Linux, BSD, IRIX, etc.
    LIB_TARGETS += hid hidio pdp pidip hdspm_mixer gem2pdp iem16
  endif
endif


all: pre_all_$(OS_NAME) $(LIB_TARGETS)
	@echo " "
	@echo "Compiled external for $(OS_NAME) aka $(UNAME)"

# these targets are for platform-specific needs that run before objects:
pre_all_darwin:

pre_all_linux:

pre_all_windows:

pre_all_unknown:
# this target is for "everything else"


#------------------------------------------------------------------------------#
# INSTALL
install: $(examplesdir) $(helpdir) $(manualsdir) $(objectsdir) $(readmesdir) \
all $(patsubst %, %_install,$(LIB_TARGETS))
	@echo " "
	@echo "externals install succeeded!"


#==============================================================================#
#
# OLD EXTERNALS BUILD SYSTEM TARGETS
#
# This is all stuff related to the externals/build/src/*.c links.  The idea is
# that they make for a flat namespace, here they are included as a libdir
#
#==============================================================================#

FLATSPACE_NAME=flatspace
FLATSPACE_OBJECTS := $(wildcard $(externals_src)/build/src/*.c)

flatspace: $(FLATSPACE_OBJECTS:.c=.$(EXTENSION))

flatspace_install: flatspace $(helpdir) $(objectsdir)
	install -d $(objectsdir)/$(FLATSPACE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(FLATSPACE_NAME) \
		--author "Numerous" \
		--description "This is a collection of externals in a flat namespace" \
		--license "GNU GPL"
	install -p $(FLATSPACE_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(FLATSPACE_NAME)
	install -d $(helpdir)/$(FLATSPACE_NAME)
# all standard objs' help files
#    it had to be broken up because the list is soo long
	install -p \
			$(externals_src)/arraysize/*.pd \
			$(externals_src)/beatpipe/*.pd \
			$(externals_src)/bsaylor/help/*.pd \
			$(externals_src)/control/*/*.pd \
			$(externals_src)/bbogart/chaos/tools/*.pd \
			$(externals_src)/bbogart/*/*.pd \
			$(externals_src)/creb/doc/*.pd \
				$(helpdir)/$(FLATSPACE_NAME)
	install -p \
			$(externals_src)/dfx/*/*.pd \
			$(externals_src)/ggee/*/*-help.pd \
			$(externals_src)/ggee/*/*.gif \
			$(externals_src)/ff/*.pd \
			$(externals_src)/hcs/*-help.pd \
				$(helpdir)/$(FLATSPACE_NAME)
	install -p \
			$(externals_src)/hcs/*/doc/*.pd \
			$(externals_src)/iem/comport/*/*-help.pd \
			$(externals_src)/nusmuk/line3/*.pd \
				$(helpdir)/$(FLATSPACE_NAME)
	install -p \
			$(externals_src)/pdogg/*/*.pd \
			$(externals_src)/plugin~/*.pd \
			$(externals_src)/rhythm_estimator/*.p? \
			$(externals_src)/signal/*/*.pd \
			$(externals_src)/sprinkler/sprinkler-help.pd \
			$(externals_src)/vst/*.pd \
				$(helpdir)/$(FLATSPACE_NAME)


flatspace_clean:
	-rm -f -- $(FLATSPACE_OBJECTS:.c=.o) 
	-rm -f -- $(FLATSPACE_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(externals_src)/build/src/*.*~
	-rm -f -- $(externals_src)/build/src/*.c.bak



#==============================================================================#
#
# PROJECT TARGETS
#
#==============================================================================#

# this is the template for creating new entries:

#------------------------------------------------------------------------------#
# TEMPLATE
TEMPLATE_NAME=template
TEMPLATE_OBJECTS := $(wildcard $(externals_src)/template/*.c)
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
	install -p $(externals_src)/template/help/*.pd \
		$(helpdir)/$(TEMPLATE_NAME)
#	install -d $(manualsdir)/$(TEMPLATE_NAME)
#	install -p $(externals_src)/template/manual.txt \
#		$(manualsdir)/$(TEMPLATE_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/template/README \
		$(readmesdir)/$(TEMPLATE_NAME).txt
	install -d $(examplesdir)/$(TEMPLATE_NAME)
	install -p $(externals_src)/template/examples/*.pd \
		$(examplesdir)/$(TEMPLATE_NAME)

template_clean:
	-rm -f -- $(TEMPLATE_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(TEMPLATE_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/template/*.bak
	-rm -f -- $(externals_src)/template/*.*~


#------------------------------------------------------------------------------#
# BOIDS
BOIDS_NAME=boids
BOIDS_OBJECTS := $(wildcard $(externals_src)/boids/boids*/*.c)
boids: $(BOIDS_OBJECTS:.c=.$(EXTENSION))

boids_install: boids
	install -d $(objectsdir)/$(BOIDS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(BOIDS_NAME) \
		--author "Eric Singer, A. Sier, and Jasch" \
		--description "2D and 3D boids flocking algorithm" \
		--license "GNU GPL 2" \
		--version ""
	install -p $(BOIDS_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(BOIDS_NAME)
	install -d $(helpdir)/$(BOIDS_NAME)
	install -p $(externals_src)/boids/boids*/*-help.pd \
		$(helpdir)/$(BOIDS_NAME)
#	install -d $(manualsdir)/$(BOIDS_NAME)
#	install -p $(externals_src)/boids/manual.txt \
#		$(manualsdir)/$(BOIDS_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/boids/boids.readme.txt \
		$(readmesdir)/$(BOIDS_NAME).txt
#	install -d $(examplesdir)/$(BOIDS_NAME)
#	install -p $(externals_src)/boids/examples/*.pd \
#		$(examplesdir)/$(BOIDS_NAME)

boids_clean:
	-rm -f -- $(BOIDS_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(BOIDS_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/boids/*/*.bak
	-rm -f -- $(externals_src)/boids/*/*.*~


#------------------------------------------------------------------------------#
# BSAYLOR
BSAYLOR_NAME=bsaylor
# partconv~.c requires fftw3, which is not part of the build system yet
BSAYLOR_OBJECTS := $(wildcard $(externals_src)/bsaylor/[a-oq-z]*~.c)
bsaylor: $(BSAYLOR_OBJECTS:.c=.$(EXTENSION))

bsaylor_install: bsaylor
	install -d $(objectsdir)/$(BSAYLOR_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(BSAYLOR_NAME) \
		--author "Benjamin R. Saylor <bensaylor@fastmail.fm>" \
		--description "signal objects" \
		--license "GNU GPL 2" \
		--version ""
	install -p $(BSAYLOR_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(BSAYLOR_NAME)
	install -d $(helpdir)/$(BSAYLOR_NAME)
	install -p $(externals_src)/bsaylor/help/*.pd \
		$(helpdir)/$(BSAYLOR_NAME)
#	install -d $(manualsdir)/$(BSAYLOR_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/bsaylor/README.txt \
		$(readmesdir)/$(BSAYLOR_NAME).txt
#	install -d $(examplesdir)/$(BSAYLOR_NAME)
#	install -p $(externals_src)/bsaylor/examples/*.pd \
#		$(examplesdir)/$(BSAYLOR_NAME)

bsaylor_clean:
	-rm -f -- $(externals_src)/bsaylor/*.$(EXTENSION)
	-rm -f -- $(externals_src)/bsaylor/*.o
	-rm -f -- $(externals_src)/bsaylor/*.bak
	-rm -f -- $(externals_src)/bsaylor/*.*~



#------------------------------------------------------------------------------#
# CORELIBS
CORELIBS_NAME=corelibs
CORELIBS_OBJECTS := $(wildcard $(externals_src)/corelibs/*.c)
corelibs: $(CORELIBS_OBJECTS:.c=.$(EXTENSION))

corelibs_install: corelibs
	install -d $(objectsdir)/$(CORELIBS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(CORELIBS_NAME) \
		--author "Miller Puckette and others" \
		--description "core libraries stripped out of Pd" \
		--license "BSD" \
		--version ""
	install -p $(CORELIBS_OBJECTS:.c=.$(EXTENSION)) \
		$(objectsdir)/$(CORELIBS_NAME)
#	install -d $(helpdir)/$(CORELIBS_NAME)
#	install -p $(externals_src)/corelibs/help/*.pd \
#		$(helpdir)/$(CORELIBS_NAME)
#	install -d $(manualsdir)/$(CORELIBS_NAME)
#	install -p $(externals_src)/corelibs/manual.txt \
#		$(manualsdir)/$(CORELIBS_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/corelibs/README \
		$(readmesdir)/$(CORELIBS_NAME).txt
#	install -d $(examplesdir)/$(CORELIBS_NAME)
#	install -p $(externals_src)/corelibs/examples/*.pd \
#		$(examplesdir)/$(CORELIBS_NAME)

corelibs_clean:
	-rm -f -- $(CORELIBS_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(CORELIBS_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/corelibs/*.bak
	-rm -f -- $(externals_src)/corelibs/*.*~



#------------------------------------------------------------------------------#
# CREB
CREB_NAME=creb
# ead* are compiled separately
CREB_OBJECTS := $(wildcard $(externals_src)/creb/modules/*.c)
# loopsampler.cc doesn't compile, so omit
CREB_CXXOBJECTS := $(wildcard $(externals_src)/creb/modules++/[a-km-z]*.cc)

creb: $(CREB_OBJECTS:.c=.$(EXTENSION)) $(CREB_CXXOBJECTS:.cc=.$(EXTENSION))

creb_install: creb
	install -d $(objectsdir)/$(CREB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(CREB_NAME) \
		--author "Tom Schouten <creb@zzz.kotnet.org>" \
		--description "This is a collection of pd externals. My bag of tricks." \
		--license "GNU GPL 2" \
		--version "0.9.2"
	install -p $(externals_src)/creb/abs/*.pd $(objectsdir)/$(CREB_NAME)
	install -p $(CREB_OBJECTS:.c=.$(EXTENSION)) \
		$(CREB_CXXOBJECTS:.cc=.$(EXTENSION)) 		$(objectsdir)/$(CREB_NAME)
	install -d $(helpdir)/$(CREB_NAME)
	install -p $(externals_src)/creb/doc/*.* $(helpdir)/$(CREB_NAME)
	install -d $(manualsdir)/$(CREB_NAME)
	install -p $(externals_src)/creb/doc/reference.txt $(manualsdir)/$(CREB_NAME)
	install -d $(examplesdir)/$(CREB_NAME)
	install -p $(externals_src)/creb/doc/examples/*.* $(examplesdir)/$(CREB_NAME)

creb_clean:
	-rm -f -- $(CREB_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(CREB_ENVELOPE_UTIL:.c=.$(EXTENSION))
	-rm -f -- $(CREB_EA_OBJECTS:.c=.$(EXTENSION)) 
	-rm -f -- $(CREB_OBJECTS:.c=.o)
	-rm -f -- $(CREB_ENVELOPE_UTIL:.c=.o) $(CREB_EA_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/creb/*/*.bak
	-rm -f -- $(externals_src)/creb/*/*.*~


#------------------------------------------------------------------------------#
# CXC
CXC_NAME=cxc
CXC_OBJECTS := $(wildcard $(externals_src)/cxc/???*.c)
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
#	install -d $(manualsdir)/$(CXC_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/cxc/README \
		 $(readmesdir)/$(CXC_NAME).txt

cxc_clean:
	-rm -f -- $(CXC_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(externals_src)/cxc/*.o
	-rm -f -- $(externals_src)/cxc/*.bak
	-rm -f -- $(externals_src)/cxc/*.*~


#------------------------------------------------------------------------------#
# CYCLONE
CYCLONE_NAME=cyclone
# cyclone is compiled straight into $(OUT_DIR)
CYCLONE_OUT_DIR=$(objectsdir)/$(CYCLONE_NAME)
cyclone: $(bindir)


cyclone_install: cyclone
	install -d $(objectsdir)/$(CYCLONE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(CYCLONE_NAME) \
		--author "Kzrysztof Czaja" \
		--license "BSD" \
		--description "a library for porting and running Max/MSP patches in Pd"
	cd $(externals_src)/miXed/cyclone && $(MAKE) OUT_DIR=$(CYCLONE_OUT_DIR)
# install "cyclist" command line app with pd
# this doesn't build in the right place yet
#	install -p -m0755 $(CYCLONE_OUT_DIR)/cyclist $(bindir) && \
#		rm $(CYCLONE_OUT_DIR)/cyclist
	-install -p -m0755 $(externals_src)/miXed/bin/cyclist $(bindir)
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


cyclone_clean:
	-$(MAKE) -C $(externals_src)/miXed/cyclone $(DEST_PATHS) clean




#------------------------------------------------------------------------------#
# DEPRECATED
DEPRECATED_NAME=deprecated
DEPRECATED_OBJECTS := $(wildcard $(externals_src)/deprecated/*.c)
deprecated: $(DEPRECATED_OBJECTS:.c=.$(EXTENSION))

deprecated_install: deprecated
	install -d $(objectsdir)/$(DEPRECATED_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(DEPRECATED_NAME) \
		--author "numerous" \
		--description "a collection of deprecated objects that still have some use" \
		--license "" \
		--version ""
	install -p $(DEPRECATED_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(DEPRECATED_NAME)
	install -d $(helpdir)/$(DEPRECATED_NAME)
	install -p $(externals_src)/deprecated/help/*.pd \
		$(helpdir)/$(DEPRECATED_NAME)
	install -d $(manualsdir)/$(DEPRECATED_NAME)
	install -p $(externals_src)/deprecated/doc/*.* \
		$(manualsdir)/$(DEPRECATED_NAME)
#	install -d $(readmesdir)
#	install -p $(externals_src)/deprecated/README \
#		$(readmesdir)/$(DEPRECATED_NAME).txt
	install -d $(examplesdir)/$(DEPRECATED_NAME)
	install -p $(externals_src)/deprecated/examples/*.pd \
		$(examplesdir)/$(DEPRECATED_NAME)

deprecated_clean:
	-rm -f -- $(DEPRECATED_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(DEPRECATED_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/deprecated/*.bak
	-rm -f -- $(externals_src)/deprecated/*.*~



#------------------------------------------------------------------------------#
# DEVELLIBS
DEVELLIBS_NAME=devellibs
DEVELLIBS_OBJECTS := $(wildcard $(externals_src)/devellibs/*.c)
devellibs: $(DEVELLIBS_OBJECTS:.c=.$(EXTENSION))

devellibs_install: devellibs
	install -d $(objectsdir)/$(DEVELLIBS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(DEVELLIBS_NAME) \
		--author "Miller Puckette and others" \
		--description "core libraries stripped out of Pd-devel" \
		--license "BSD" \
		--version ""
	install -p $(DEVELLIBS_OBJECTS:.c=.$(EXTENSION)) 
		$(objectsdir)/$(DEVELLIBS_NAME)
	install -d $(helpdir)/$(DEVELLIBS_NAME)
	install -p $(externals_src)/devellibs/help/*.pd \
		$(helpdir)/$(DEVELLIBS_NAME)
#	install -d $(manualsdir)/$(DEVELLIBS_NAME)
#	install -p $(externals_src)/devellibs/manual.txt \
#		$(manualsdir)/$(DEVELLIBS_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/devellibs/README \
		$(readmesdir)/$(DEVELLIBS_NAME).txt
	install -d $(examplesdir)/$(DEVELLIBS_NAME)
	install -p $(externals_src)/devellibs/examples/*.pd \
		$(examplesdir)/$(DEVELLIBS_NAME)

devellibs_clean:
	-rm -f -- $(DEVELLIBS_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(DEVELLIBS_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/devellibs/*.bak
	-rm -f -- $(externals_src)/devellibs/*.*~





#------------------------------------------------------------------------------#
# EKEXT
EKEXT_NAME=ekext
EKEXT_OBJECTS := $(wildcard $(externals_src)/ekext/*/*.c) \
$(wildcard $(externals_src)/ekext/*/*/*.c)
ekext: $(EKEXT_OBJECTS:.c=.$(EXTENSION))

ekext_install: ekext
	install -d $(objectsdir)/$(EKEXT_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(EKEXT_NAME) \
		--author "Ed Kelly <morph_2016@yahoo.co.uk>" \
		--description "" \
		--license "BSD" \
		--version ""
	install -p $(EKEXT_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(EKEXT_NAME)
	install -d $(helpdir)/$(EKEXT_NAME)
	install -p $(externals_src)/ekext/*/*help*.pd \
		$(helpdir)/$(EKEXT_NAME)
#	install -d $(manualsdir)/$(EKEXT_NAME)
#	install -p $(externals_src)/ekext/manual.txt \
#		$(manualsdir)/$(EKEXT_NAME)
#	install -p $(externals_src)/ekext/README \
#		$(readmesdir)/$(EKEXT_NAME).txt
	install -d $(examplesdir)/$(EKEXT_NAME)
	install -p $(externals_src)/ekext/peakit~/peakit-listmoses.pd \
		$(examplesdir)/$(EKEXT_NAME)

ekext_clean:
	-rm -f -- $(EKEXT_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(EKEXT_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/ekext/*/*.bak
	-rm -f -- $(externals_src)/ekext/*/*.*~




#------------------------------------------------------------------------------#
# EXT13
EXT13_NAME=ext13
# openpatch and sfread.c doesn't compile, so exclude them
EXT13_FILES = catch13~.c kalashnikov.c ossmixer.c receive13~.c sfwrite13~.c \
wavinfo.c cdplayer.c mandelbrot.c piperead~.c scramble~.c streamin13~.c \
mandelbrot~.c pipewrite~.c send13.c streamout13~.c filesize.c messages.c \
promiscous~.c send13~.c strippath.c ftos.c receive13.c throw13~.c
EXT13_OBJECTS = $(patsubst %,$(externals_src)/ext13/%,$(EXT13_FILES))
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
#	install -d $(manualsdir)/$(EXT13_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/ext13/README \
		$(readmesdir)/$(EXT13_NAME).txt

ext13_clean:
	-rm -f -- $(externals_src)/ext13/*.$(EXTENSION)
	-rm -f -- $(externals_src)/ext13/*.o
	-rm -f -- $(externals_src)/ext13/*.bak
	-rm -f -- $(externals_src)/ext13/*.*~



#------------------------------------------------------------------------------#
# FFTease
FFTEASE_NAME=fftease
FFTEASE_LIB_OBJECTS := $(wildcard $(externals_src)/fftease/lib/*.c)
FFTEASE_OBJECTS := $(wildcard $(externals_src)/fftease/*.c)
fftease_lib: $(FFTEASE_LIB_OBJECTS:.c=.o)
	$(CC) $(DYLIB_LDFLAGS) \
		-o $(externals_src)/fftease/libfftease.$(DYLIB_EXTENSION) \
		 $(FFTEASE_LIB_OBJECTS:.c=.o)

fftease: fftease_lib $(FFTEASE_OBJECTS:.c=.$(EXTENSION))

fftease_install: fftease
	install -d $(objectsdir)/$(FFTEASE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(FFTEASE_NAME) \
		--author "Eric Lyon and Christopher Penrose" \
		--description "FFTease is a collection of objects implementing various forms of spectral sound processing. These include an additive-synthesis phase vocoder, noise reduction, cross synthesis, and more unusual forms of spectral processing." \
		--license "MIT License" \
		--version "2.0"
	install -p $(FFTEASE_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(FFTEASE_NAME)
	install -d $(helpdir)/$(FFTEASE_NAME)
	install -p $(externals_src)/fftease/help/*.pd \
		$(helpdir)/$(FFTEASE_NAME)
#	install -d $(manualsdir)/$(FFTEASE_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/fftease/README \
		$(readmesdir)/$(FFTEASE_NAME).txt
	install -d $(examplesdir)/$(FFTEASE_NAME)
	install -p $(externals_src)/fftease/examples/*.pd \
		$(examplesdir)/$(FFTEASE_NAME)

fftease_clean:
	-rm -f -- $(externals_src)/fftease/*.$(EXTENSION)
	-rm -f -- $(externals_src)/fftease/*.o
	-rm -f -- $(externals_src)/fftease/*.bak
	-rm -f -- $(externals_src)/fftease/*.*~



#------------------------------------------------------------------------------#
# FILE
FILE_NAME=file
FILE_OBJECTS := $(wildcard $(externals_src)/file/*.c)
file: $(FILE_OBJECTS:.c=.$(EXTENSION))

file_install: file
	install -d $(objectsdir)/$(FILE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(FILE_NAME) \
		--author "" \
		--description "" \
		--license "" \
		--version ""
	install -p $(FILE_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(FILE_NAME)
	install -d $(helpdir)/$(FILE_NAME)
	install -p $(externals_src)/file/help/*.pd \
		$(helpdir)/$(FILE_NAME)
#	install -d $(manualsdir)/$(FILE_NAME)
#	install -p $(externals_src)/file/manual.txt \
#		$(manualsdir)/$(FILE_NAME)
#	install -p $(externals_src)/file/README \
#		$(readmesdir)/$(FILE_NAME).txt
#	install -d $(examplesdir)/$(FILE_NAME)
#	install -p $(externals_src)/file/examples/*.pd \
#		$(examplesdir)/$(FILE_NAME)

file_clean:
	-rm -f -- $(FILE_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(FILE_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/file/*.bak
	-rm -f -- $(externals_src)/file/*.*~




#------------------------------------------------------------------------------#
# FLIB
FLIB_NAME=flib
# exclude the flib.c file for the single-file library
# plus ha~.c doesn't compile at the moment.
FLIB_OBJECTS := $(wildcard $(externals_src)/postlude/flib/src/[a-ei-z]*.c)
flib: $(FLIB_OBJECTS:.c=.$(EXTENSION))

flib_install: flib
	install -d $(objectsdir)/$(FLIB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(FLIB_NAME) \
		--author "Jamie Bullock" \
		--description "library for feature extraction" \
		--license "GNU GPL"
	install -p $(FLIB_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(FLIB_NAME)
	install -d $(helpdir)/$(FLIB_NAME)
	install -p $(externals_src)/postlude/flib/doc/*.pd \
		$(helpdir)/$(FLIB_NAME)
#	install -d $(manualsdir)/$(FLIB_NAME)
#	install -p $(externals_src)/postlude/flib/manual.txt \
#		$(manualsdir)/$(FLIB_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/postlude/flib/README \
		$(readmesdir)/$(FLIB_NAME).txt
#	install -d $(examplesdir)/$(FLIB_NAME)
#	install -p $(externals_src)/postlude/flib/examples/*.pd \
#		$(examplesdir)/$(FLIB_NAME)

flib_clean:
	-rm -f -- $(FLIB_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(FLIB_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/postlude/flib/*/*.bak
	-rm -f -- $(externals_src)/postlude/flib/*/*.*~



#------------------------------------------------------------------------------#
# FRANKENSTEIN
FRANKENSTEIN_NAME=frankenstein
# exclude test.c since it is not used
FRANKENSTEIN_OBJECTS := $(wildcard $(externals_src)/frankenstein/*.c)
frankenstein: $(FRANKENSTEIN_OBJECTS:.c=.$(EXTENSION))

frankenstein_install: frankenstein
	install -d $(objectsdir)/$(FRANKENSTEIN_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(FRANKENSTEIN_NAME) \
		--author "" \
		--description "" \
		--license "" \
		--version ""
	install -p $(FRANKENSTEIN_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(FRANKENSTEIN_NAME)
#	install -d $(helpdir)/$(FRANKENSTEIN_NAME)
#	install -p $(externals_src)/frankenstein/help/*.pd \
#		$(helpdir)/$(FRANKENSTEIN_NAME)
	install -d $(manualsdir)/$(FRANKENSTEIN_NAME)
	install -p $(externals_src)/frankenstein/doc/*.* \
		$(manualsdir)/$(FRANKENSTEIN_NAME)
	install -d $(examplesdir)/$(FRANKENSTEIN_NAME)
	install -p $(externals_src)/frankenstein/patches/*.* \
		$(examplesdir)/$(FRANKENSTEIN_NAME)

frankenstein_clean:
	-rm -f -- $(externals_src)/frankenstein/*.$(EXTENSION)
	-rm -f -- $(externals_src)/frankenstein/*.o
	-rm -f -- $(externals_src)/frankenstein/*.bak
	-rm -f -- $(externals_src)/frankenstein/*.*~



#------------------------------------------------------------------------------#
# FREEVERB
FREEVERB_NAME=freeverb
FREEVERB_OBJECTS := $(wildcard $(externals_src)/freeverb~/*.c)
freeverb: $(FREEVERB_OBJECTS:.c=.$(EXTENSION))

freeverb_install: freeverb
	install -d $(objectsdir)/$(FREEVERB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(FREEVERB_NAME) \
		--author "Olaf Matthes and Jezar Wakefield" \
		--description "Freeverb is a simple implementation of the standard Schroeder/Moorer reverb model" \
		--license "GNU GPL" \
		--version "1.2"
	install -p $(FREEVERB_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(FREEVERB_NAME)
	install -d $(helpdir)/$(FREEVERB_NAME)
	install -p $(wildcard $(externals_src)/freeverb~/*-help.pd) \
		$(helpdir)/$(FREEVERB_NAME)
#	install -d $(manualsdir)/$(FREEVERB_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/freeverb~/README \
		$(readmesdir)/$(FREEVERB_NAME).txt


freeverb_clean:
	-rm -f -- $(externals_src)/freeverb~/*.$(EXTENSION)
	-rm -f -- $(externals_src)/freeverb~/*.o
	-rm -f -- $(externals_src)/freeverb~/*.bak
	-rm -f -- $(externals_src)/freeverb~/*.*~



#------------------------------------------------------------------------------#
# GEM2PDP
GEM2PDP_NAME=gem2pdp
GEM2PDP_OBJECTS := $(wildcard $(externals_src)/gem2pdp/*.cpp)
$(externals_src)/gem2pdp/configure: $(externals_src)/gem2pdp/configure.ac
	cd $(externals_src)/gem2pdp && autoconf

$(externals_src)/gem2pdp/Makefile: $(externals_src)/gem2pdp/Makefile.in
	cd $(externals_src)/gem2pdp && ./configure --with-pddir=$(pd_src) \
		--with-gemdir=$(gem_src)  --with-pdpdir=$(externals_src)/pdp

gem2pdp: $(externals_src)/gem2pdp/configure $(externals_src)/gem2pdp/Makefile
	$(MAKE) -C $(externals_src)/gem2pdp

gem2pdp_install: gem2pdp
	install -d $(objectsdir)
#	install -d $(objectsdir)/$(GEM2PDP_NAME)
#	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(GEM2PDP_NAME) \
#		--author "Yves Degoyon, Jamie Tittle, Georg Holzmann" \
#		--description "Bridges between Gem and PDP" \
#		--version "0.6"
	install -p $(GEM2PDP_OBJECTS:.cpp=.$(EXTENSION)) $(objectsdir)
#	install -d $(helpdir)/$(GEM2PDP_NAME)
	install -d $(helpdir)
	install -p $(externals_src)/gem2pdp/*.pd  $(helpdir)
#	install -d $(manualsdir)/$(GEM2PDP_NAME)
#	install -p $(externals_src)/gem2pdp/manual.txt \
#		$(manualsdir)/$(GEM2PDP_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/gem2pdp/README \
		$(readmesdir)/$(GEM2PDP_NAME).txt
#	install -d $(examplesdir)/$(GEM2PDP_NAME)
#	install -p $(externals_src)/gem2pdp/examples/*.pd \
#		$(examplesdir)/$(GEM2PDP_NAME)

gem2pdp_clean:
	-rm -rf -- $(externals_src)/gem2pdp/autom4te.cache
	-rm -f -- $(externals_src)/gem2pdp/config.status
	-rm -f -- $(externals_src)/gem2pdp/config.log
	-rm -f -- $(externals_src)/gem2pdp/configure
	-rm -f -- $(externals_src)/gem2pdp/Makefile
	-rm -f -- $(GEM2PDP_OBJECTS:.cpp=.$(EXTENSION))
	-rm -f -- $(externals_src)/gem2pdp/*.o
	-rm -f -- $(externals_src)/gem2pdp/*.bak
	-rm -f -- $(externals_src)/gem2pdp/*.*~


#------------------------------------------------------------------------------#
# GGEE
GGEE_NAME=ggee
# experimental needs C++ and tools is command line, so exclude
GGEE_OBJECTS := $(wildcard $(externals_src)/ggee/[a-df-su-z]*/*.c)
ggee: $(GGEE_OBJECTS:.c=.$(EXTENSION))

ggee_install: ggee
	install -d $(objectsdir)/$(GGEE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(GGEE_NAME) \
		--author "Guenter Geiger" \
		--description "" \
		--license "BSD" \
		--version "0.25"
	install -p $(GGEE_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(GGEE_NAME)
	install -p $(shell ls -1 $(externals_src)/vbap/*.pd | grep -v '-help.pd') \
		 $(objectsdir)/$(GGEE_NAME)
	install -d $(helpdir)/$(GGEE_NAME)
	install -p $(externals_src)/ggee/*/*-help.pd \
		$(externals_src)/ggee/*/*.gif $(helpdir)/$(GGEE_NAME)
#	install -d $(manualsdir)/$(GGEE_NAME)
#	install -p $(externals_src)/ggee/manual.txt \
#		$(manualsdir)/$(GGEE_NAME)
#	install -p $(externals_src)/ggee/README \
#		$(readmesdir)/$(GGEE_NAME).txt
#	install -d $(examplesdir)/$(GGEE_NAME)
#	install -p $(externals_src)/ggee/examples/*.pd \
#		$(examplesdir)/$(GGEE_NAME)

ggee_clean:
	-rm -f -- $(GGEE_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(GGEE_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/ggee/*.bak
	-rm -f -- $(externals_src)/ggee/*.*~


#------------------------------------------------------------------------------#
# HARDWARE
HARDWARE_NAME=hardware
# memPIO has some special deps, so don't include it yet.
# multio.c requires libusb, so don't include it yet...  <hans@at.or.at>
#HARDWARE_OBJECTS := $(wildcard $(externals_src)/hardware/*/*.cpp) \
#$(wildcard $(externals_src)/hardware/*/*.c)
#hardware: $(HARDWARE_OBJECTS:.c=.$(EXTENSION)) $(HARDWARE_OBJECTS:.cpp=.$(EXTENSION))
hardware:

hardware_install: hardware
	install -d $(objectsdir)/$(HARDWARE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(HARDWARE_NAME) \
		--description "objects for working with hardware sensor boxes" \
		--license "GNU GPL"
# nothing built from source yet
#	install -p $(HARDWARE_OBJECTS:.c=.$(EXTENSION)) \
#		$(objectsdir)/$(HARDWARE_NAME)
	install -p $(externals_src)/hardware/lanbox/lanbox.pd \
		$(externals_src)/hardware/arduino/arduino.pd \
		$(objectsdir)/$(HARDWARE_NAME)
	install -d $(helpdir)/$(HARDWARE_NAME)
	install -p $(externals_src)/hardware/*/*-help.pd \
		$(helpdir)/$(HARDWARE_NAME)
	install -d $(manualsdir)/$(HARDWARE_NAME)
	install -p $(externals_src)/hardware/memPIO/README.txt \
		$(manualsdir)/$(HARDWARE_NAME)/memPIO.txt
	install -d $(readmesdir)
	install -p $(externals_src)/hardware/README.txt \
		$(readmesdir)/$(HARDWARE_NAME).txt
	install -d $(examplesdir)/$(HARDWARE_NAME)
	install -p $(externals_src)/hardware/*/examples/*.pd \
		$(externals_src)/hardware/arduino/arduino-test.pd \
		$(examplesdir)/$(HARDWARE_NAME)

hardware_clean:
	-rm -f -- $(externals_src)/hardware/*/*.$(EXTENSION)
	-rm -f -- $(externals_src)/hardware/*/*.o
	-rm -f -- $(externals_src)/hardware/*/*.bak
	-rm -f -- $(externals_src)/hardware/*/*.*~


#------------------------------------------------------------------------------#
# HCS
HCS_NAME=hcs
HCS_OBJECTS := $(wildcard $(externals_src)/hcs/*.c)
hcs: $(HCS_OBJECTS:.c=.$(EXTENSION))

hcs_install: hcs
	install -d $(objectsdir)/$(HCS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(HCS_NAME) \
		--author "Hans-Christoph Steiner <hans@at.or.at>" \
		--description "random grab bag of object prototypes" \
		--license "GNU GPL" \
		--version ""
	install -p $(HCS_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(HCS_NAME)
# temp kludge: install [classpath] and [import] into global namespace since
# they are used for loading libraries
	install -p $(externals_src)/hcs/classpath.$(EXTENSION) \
		$(externals_src)/hcs/import.$(EXTENSION) $(objectsdir)
	install -p $(shell ls -1 $(externals_src)/hcs/*.pd | \
			 grep -v '\-help.pd') $(objectsdir)/$(HCS_NAME)
# hmm, these don't have a home yet...
	install -p $(externals_src)/hcs/general/*.pd $(objectsdir)/$(HCS_NAME)
	install -d $(helpdir)/$(HCS_NAME)
	install -p $(externals_src)/hcs/*-help.pd \
		$(helpdir)/$(HCS_NAME)
	install -d $(manualsdir)/$(HCS_NAME)
	install -p $(externals_src)/hcs/README* $(externals_src)/hcs/TODO \
		$(manualsdir)/$(HCS_NAME)
#	install -d $(readmesdir)
#	install -p $(externals_src)/hcs/README \
#		$(readmesdir)/$(HCS_NAME).txt
#	install -d $(examplesdir)/$(HCS_NAME)
#	install -p $(externals_src)/hcs/examples/*.pd \
#g		$(examplesdir)/$(HCS_NAME)

hcs_clean:
	-rm -f -- $(HCS_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(HCS_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/hcs/*.bak
	-rm -f -- $(externals_src)/hcs/*.*~



#------------------------------------------------------------------------------#
# HID
HID_NAME = hid
ifeq ($(OS_NAME),windows)
  HID_CFLAGS = $(CFLAGS) 
  HID_LIBS = $(LIBS) -lhid -lsetupapi
else
  ifeq ($(OS_NAME),darwin)
    FRAMEWORKS = Carbon IOKit ForceFeedback
    HID_CFLAGS = $(CFLAGS) -I$(externals_src)/hcs/hid/HID\ Utilities\ Source
    HID_UTILITIES_SOURCE = $(externals_src)/hcs/hid/HID\ Utilities\ Source
    HID_LIBS = $(LIBS) -L$(HID_UTILITIES_SOURCE)/build \
		-L$(HID_UTILITIES_SOURCE)/build/Default \
      -lHIDUtilities $(patsubst %,-weak_framework %,$(FRAMEWORKS))
  else
    HID_CFLAGS = $(CFLAGS) 
    HID_LIBS = $(LIBS)
  endif
endif

HID_SRC = input_arrays.c hid_$(OS_NAME).c hid.c
HID_OBJECTS := $(patsubst %.c, $(externals_src)/hcs/hid/%.o, $(HID_SRC))
$(HID_OBJECTS) : %.o : %.c
	$(CC) $(HID_CFLAGS) -o "$*.o" -c "$*.c"

$(HID_UTILITIES_SOURCE)/build/Default/libHIDUtilities.a:
# Apple changed the XCode CLI tool's name in xcode2... arg
# if on non-Mac OS X, this target just echos a message
ifeq ($(UNAME),Darwin)
	cd $(HID_UTILITIES_SOURCE) && \
		(test -x /usr/bin/xcodebuild && /usr/bin/xcodebuild) || \
			(test -x /usr/bin/pbxbuild && /usr/bin/pbxbuild) || \
				echo "Not building Apple HID Utilities"
endif

$(externals_src)/hcs/hid/hid.$(EXTENSION): $(HID_OBJECTS) \
$(HID_UTILITIES_SOURCE)/build/Default/libHIDUtilities.a
	$(CC) $(LDFLAGS) -o $(externals_src)/hcs/hid/hid.$(EXTENSION) \
		$(HID_OBJECTS) $(HID_LIBS)
#	$(STRIP) $(externals_src)/hcs/hid/hid.$(EXTENSION)

hid: $(externals_src)/hcs/hid/hid.$(EXTENSION)

hid_install: hid
	install -d $(objectsdir)/$(HID_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(HID_NAME) \
		--author "Hans-Christoph Steiner" \
		--description "" \
		--license "GNU GPL" \
		--version ""
	-install -p $(externals_src)/hcs/hid/hid.$(EXTENSION) $(objectsdir)
	install -p $(externals_src)/hcs/hid/*.pd $(objectsdir)/$(HID_NAME)
	install -d $(helpdir)/$(HID_NAME)
	install -p $(externals_src)/hcs/hid/doc/*.pd       $(helpdir)/$(HID_NAME)
	install -p $(externals_src)/hcs/hid/examples/*.pd $(helpdir)/$(HID_NAME)
	install -d $(examplesdir)/$(HID_NAME)
	install -p $(externals_src)/hcs/hid/examples/*.pd  $(examplesdir)/$(HID_NAME)

hid_clean:
	-rm -f -- $(externals_src)/hcs/hid/*.o
	-rm -f -- $(externals_src)/hcs/hid/hid.$(EXTENSION)
	-rm -f -- $(externals_src)/hcs/hid/*.bak
	-rm -f -- $(externals_src)/hcs/hid/*.*~
	-rm -f -- $(HID_UTILITIES_SOURCE)/build/libHIDUtilities.a
	-rm -f -- $(HID_UTILITIES_SOURCE)/build/Default/libHIDUtilities.a


#------------------------------------------------------------------------------#
# HIDIN
HIDIN_NAME=hidin
HIDIN_OBJECTS := $(wildcard $(externals_src)/olafmatt/hidin/*.c)
HIDIN_DLL := $(externals_src)/olafmatt/hidin/hidin.$(EXTENSION)

$(HIDIN_DLL): $(HIDIN_OBJECTS:.c=.o)
	$(CC) $(LDFLAGS) -o "$(HIDIN_DLL)" $(HIDIN_OBJECTS:.c=.o) $(LIBS) -lhid \
		-lsetupapi
	chmod a-x "$(HIDIN_DLL)"
	$(STRIP) $(HIDIN_DLL)

hidin: $(HIDIN_DLL)

hidin_install: hidin
	install -d $(objectsdir)/$(HIDIN_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(HIDIN_NAME) \
		--author "Olaf Matthes" \
		--description "HID input for Windows HID" \
		--license "GNU GPL"
	install -p $(HIDIN_DLL) $(objectsdir)/$(HIDIN_NAME)
#	install -d $(helpdir)/$(HIDIN_NAME)
#	install -p $(externals_src)/olafmatt/hidin/help/*.pd \
#		$(helpdir)/$(HIDIN_NAME)
#	install -d $(manualsdir)/$(HIDIN_NAME)
#	install -d $(readmesdir)
#	install -p $(externals_src)/olafmatt/hidin/README \
#		$(readmesdir)/$(HIDIN_NAME).txt
#	install -d $(examplesdir)/$(HIDIN_NAME)
#	install -p $(externals_src)/olafmatt/hidin/examples/*.pd \
#		$(examplesdir)/$(HIDIN_NAME)

hidin_clean:
	-rm -f -- $(HIDIN_DLL)
	-rm -f -- $(HIDIN_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/olafmatt/hidin/*.bak
	-rm -f -- $(externals_src)/olafmatt/hidin/*.*~



#------------------------------------------------------------------------------#
# HIDIO
HIDIO_NAME = hidio
ifeq ($(OS_NAME),windows)
  HIDIO_SRC = hidio_windows.c hidio_types.c hidio.c
  HIDIO_CFLAGS = $(CFLAGS) 
  HIDIO_LIBS = $(LIBS) -lhid -lsetupapi
endif
ifeq ($(OS_NAME),darwin)
  HIDIO_SRC = hidio_darwin.c hidio_types.c hidio.c
  FRAMEWORKS = Carbon IOKit ForceFeedback
  HIDIO_CFLAGS = $(CFLAGS) -I$(externals_src)/io/hidio/HID\ Utilities\ Source
  HIDIO_HID_UTILITIES_SOURCE = $(externals_src)/io/hidio/HID\ Utilities\ Source
  HIDIO_LIBS = $(LIBS) -L$(HIDIO_HID_UTILITIES_SOURCE)/build \
    -L$(HIDIO_HID_UTILITIES_SOURCE)/build/Default \
    -lHIDUtilities $(patsubst %,-weak_framework %,$(FRAMEWORKS))
endif
ifeq ($(OS_NAME),linux)
  HIDIO_SRC = input_arrays.c hidio_linux.c hidio_types.c hidio.c
  HIDIO_CFLAGS = $(CFLAGS) 
  HIDIO_LIBS = $(LIBS)
endif

HIDIO_OBJECTS := $(patsubst %.c, $(externals_src)/io/hidio/%.o, $(HIDIO_SRC))
$(HIDIO_OBJECTS) : %.o : %.c
	$(CC) $(HIDIO_CFLAGS) -o "$*.o" -c "$*.c"

$(HIDIO_HID_UTILITIES_SOURCE)/build/Default/libHIDUtilities.a:
# Apple changed the XCode CLI tool's name in xcode2... arg
# if on non-Mac OS X, this target just echos a message
ifeq ($(UNAME),Darwin)
	cd $(HIDIO_HID_UTILITIES_SOURCE) && \
		(test -x /usr/bin/xcodebuild && /usr/bin/xcodebuild) || \
			(test -x /usr/bin/pbxbuild && /usr/bin/pbxbuild) || \
				echo "Not building Apple HID Utilities"
endif

$(externals_src)/io/hidio/hidio.$(EXTENSION): $(HIDIO_OBJECTS) \
$(HIDIO_HID_UTILITIES_SOURCE)/build/Default/libHIDUtilities.a
	$(CC) $(LDFLAGS) -o $(externals_src)/io/hidio/hidio.$(EXTENSION) \
		$(HIDIO_OBJECTS) $(HIDIO_LIBS)
#	$(STRIP) $(externals_src)/io/hidio/hidio.$(EXTENSION)

hidio: $(externals_src)/io/hidio/hidio.$(EXTENSION)

hidio_install: hidio
	install -d $(objectsdir)/$(HIDIO_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(HIDIO_NAME) \
		--author "David Merrill <dmerrill@media.mit.edu>, Hans-Christoph Steiner <hans@at.or.at>, Olaf Matthes <olaf@nullmedium.de>" \
		--description "I/O for USB HID and other supported devices" \
		--license "GNU GPLv2" \
		--version "0.0"
	-install -p $(externals_src)/io/hidio/hidio.$(EXTENSION) $(objectsdir)
#	install -p $(externals_src)/io/hidio/*.pd $(objectsdir)/$(HIDIO_NAME)
	install -d $(helpdir)/$(HIDIO_NAME)
	install -p $(externals_src)/io/hidio/*-help.pd  $(helpdir)/$(HIDIO_NAME)
#	install -p $(externals_src)/io/hidio/examples/*.pd $(helpdir)/$(HIDIO_NAME)
#	install -d $(examplesdir)/$(HIDIO_NAME)
#	install -p $(externals_src)/io/hidio/examples/*.pd  $(examplesdir)/$(HIDIO_NAME)

hidio_clean:
	-rm -f -- $(HIDIO_OBJECTS:.c=.o)
	-rm -f -- $(HIDIO_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(externals_src)/io/hidio/*.bak
	-rm -f -- $(externals_src)/io/hidio/*.*~
	-rm -f -- $(HIDIO_HID_UTILITIES_SOURCE)/build/libHIDUtilities.a
	-rm -f -- $(HIDIO_HID_UTILITIES_SOURCE)/build/Default/libHIDUtilities.a




#------------------------------------------------------------------------------#
# IEM_AMBI
IEM_AMBI_NAME=iem_ambi
# exclude the files for the single-file library format
IEM_AMBI_OBJECTS := $(wildcard $(externals_src)/iem/iem_ambi/src/[a-hj-z]*.c)
iem_ambi: $(IEM_AMBI_OBJECTS:.c=.$(EXTENSION))

iem_ambi_install: iem_ambi
	install -d $(objectsdir)/$(IEM_AMBI_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEM_AMBI_NAME) \
		--author "IEM/KUG, Graz, Austria" \
		--description "calculate ambisonic encoder matrices rotation matrices and decoder matrices from 1st to 4th order in 2D or 3D." \
		--license "GNU GPL" \
		--version ""
	install -p $(IEM_AMBI_OBJECTS:.c=.$(EXTENSION)) \
		$(objectsdir)/$(IEM_AMBI_NAME)
	install -d $(helpdir)/$(IEM_AMBI_NAME)
	install -p $(externals_src)/iem/iem_ambi/help/*.pd \
		$(helpdir)/$(IEM_AMBI_NAME)
#	install -d $(manualsdir)/$(IEM_AMBI_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/iem/iem_ambi/READ_ME.txt \
		$(readmesdir)/$(IEM_AMBI_NAME).txt
#	install -d $(examplesdir)/$(IEM_AMBI_NAME)
#	install -p $(externals_src)/iem/iem_ambi/examples/*.pd \
#		$(examplesdir)/$(IEM_AMBI_NAME)

iem_ambi_clean:
	-rm -f -- $(IEM_AMBI_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(IEM_AMBI_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/iem/iem_ambi/*/*.bak
	-rm -f -- $(externals_src)/iem/iem_ambi/*/*.*~



#------------------------------------------------------------------------------#
# IEM_BIN_AMBI
IEM_BIN_AMBI_NAME=iem_bin_ambi
# exclude the files for the single-file library format
IEM_BIN_AMBI_OBJECTS := $(wildcard $(externals_src)/iem/iem_bin_ambi/src/[a-hj-z]*.c)
iem_bin_ambi: $(IEM_BIN_AMBI_OBJECTS:.c=.$(EXTENSION))

iem_bin_ambi_install: iem_bin_ambi
	install -d $(objectsdir)/$(IEM_BIN_AMBI_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEM_BIN_AMBI_NAME) \
		--author "IEM/KUG, Graz, Austria" \
		--description "calculate the product of an ambisonic decoder-matrix and the binaural HRIR's (in frequency and in time domain)" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEM_BIN_AMBI_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(IEM_BIN_AMBI_NAME)
	install -d $(helpdir)/$(IEM_BIN_AMBI_NAME)
	install -p $(externals_src)/iem/iem_bin_ambi/help/*.pd \
		$(helpdir)/$(IEM_BIN_AMBI_NAME)
	install -d $(manualsdir)/$(IEM_BIN_AMBI_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/iem/iem_bin_ambi/READ_ME.txt \
		$(readmesdir)/$(IEM_BIN_AMBI_NAME).txt
#	install -d $(examplesdir)/$(IEM_BIN_AMBI_NAME)
#	install -p $(externals_src)/iem/iem_bin_ambi/examples/*.pd \
#		$(examplesdir)/$(IEM_BIN_AMBI_NAME)

iem_bin_ambi_clean:
	-rm -f -- $(IEM_BIN_AMBI_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(IEM_BIN_AMBI_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/iem/iem_bin_ambi/*/*.bak
	-rm -f -- $(externals_src)/iem/iem_bin_ambi/*/*.*~



#------------------------------------------------------------------------------#
# IEM16
# more externals in one file - so a little bit complicated ... ;)
IEM16_NAME=iem16
IEM16_DIR := $(externals_src)/iem16/src
IEM16_SRC := $(wildcard $(IEM16_DIR)/*.c)

$(IEM16_DIR)/aclocal.m4: $(IEM16_DIR)/acinclude.m4
	cd $(IEM16_DIR) && aclocal

$(IEM16_DIR)/configure: $(IEM16_DIR)/configure.ac $(IEM16_DIR)/aclocal.m4
	cd $(IEM16_DIR) && autoconf

$(IEM16_DIR)/Make.config: $(IEM16_DIR)/Make.config.in \
$(IEM16_DIR)/configure
	cd $(IEM16_DIR) && ./configure --disable-library \
		--with-pd=$(pd_src)

iem16: $(IEM16_DIR)/Make.config
	$(MAKE) -C $(IEM16_DIR)


iem16_install: iem16
	install -d $(objectsdir)/$(IEM16_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEM16_NAME) \
		--author "IOhannes m zmoelnig" \
		--description "16bit table, array objects for low memory usage" \
		--license "GNU GPL" \
		--version "0.1"
	install -p $(IEM16_DIR)/*.$(EXTENSION) $(objectsdir)/$(IEM16_NAME)
	install -d $(helpdir)/$(IEM16_NAME)
	install -p $(externals_src)/iem16/help/*.pd \
		$(helpdir)/$(IEM16_NAME)

iem16_clean:
	-$(MAKE) -C $(IEM16_DIR) clean
	-rm -f -- $(IEM16_DIR)/*.$(EXTENSION)
	-rm -f -- $(IEM16_DIR)/*.d
	-rm -rf -- $(IEM16_DIR)/autom4ate
	-rm -rf -- $(IEM16_DIR)/conf[0-9][0-9][0-9]*
	-rm -f -- $(IEM16_DIR)/configure
	-rm -f -- $(IEM16_DIR)/Make.config
	-rm -f -- $(IEM16_DIR)/*.o
	-rm -f -- $(IEM16_DIR)/*.bak
	-rm -f -- $(IEM16_DIR)/*.*~



#------------------------------------------------------------------------------#
# IEMLIB
IEMLIB_NAME=iemlib
# omit the lib files and iem_mp3 (for patent reasons :( )
IEMLIB_OBJECTS := $(wildcard $(externals_src)/iemlib/iemlib1/src/*[^1].c) $(wildcard $(externals_src)/iemlib/iemlib2/src/*[^2].c)  $(wildcard $(externals_src)/iemlib/iemlib_t3_lib/src/t3_*.c)  $(wildcard $(externals_src)/iemlib/alias/*.c)
iemlib: $(IEMLIB_OBJECTS:.c=.$(EXTENSION))

iemlib_install: iemlib
	install -d $(objectsdir)/$(IEMLIB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEMLIB_NAME) \
		--description "a collection of objects written at IEM/KUG" \
		--license "GNU GPL"
	install -p $(IEMLIB_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/iemabs/*.pd $(objectsdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/alias/*.pd $(objectsdir)/$(IEMLIB_NAME)
	install -d $(helpdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/*/*-help.pd $(helpdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/*/*.mp3 $(helpdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/*/*.wav $(helpdir)/$(IEMLIB_NAME)
	install -d $(examplesdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/examples/*.* \
		$(examplesdir)/$(IEMLIB_NAME)
	install -d $(manualsdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/*.pdf $(externals_src)/iemlib/*.txt \
		$(externals_src)/iemlib/iemabs/*.txt \
		$(manualsdir)/$(IEMLIB_NAME)


iemlib_clean: 
	-rm -f -- $(IEMLIB_OBJECTS:.c=.$(EXTENSION))
	-rmdir -- $(objectsdir)/$(IEMLIB_NAME)
	-rm -f -- $(helpdir)/$(IEMLIB_NAME)/*.*
	-rmdir -- $(helpdir)/$(IEMLIB_NAME)
	-rm -f -- $(examplesdir)/$(IEMLIB_NAME)/*.*
	-rmdir -- $(examplesdir)/$(IEMLIB_NAME)
	-rm -f -- $(manualsdir)/$(IEMLIB_NAME)/*.*
	-rmdir -- $(manualsdir)/$(IEMLIB_NAME)



#------------------------------------------------------------------------------#
# IEMMATRIX
IEMMATRIX_NAME=iemmatrix
IEMMATRIX_SRC := $(wildcard $(externals_src)/iem/iemmatrix/src/m[at]*.c)
IEMMATRIX_ALIAS := $(wildcard $(externals_src)/iem/iemmatrix/alias/*.c)
IEMMATRIX_OBJ := $(IEMMATRIX_SRC:.c=.o) $(IEMMATRIX_ALIAS:.c=.o)
IEMMATRIX_SHARED := $(wildcard $(externals_src)/iem/iemmatrix/src/iemmatrix_binops.c) \
                    $(wildcard $(externals_src)/iem/iemmatrix/src/iemmatrix_utility.c)

iemmatrix: $(IEMMATRIX_OBJ:.o=.$(EXTENSION))

$(IEMMATRIX_OBJ:.o=.$(EXTENSION)) : %.$(EXTENSION) : $(IEMMATRIX_OBJ) $(IEMMATRIX_SHARED:.c=.o)
	$(CC) $(LDFLAGS) -o $*.$(EXTENSION) "$*.o" $(IEMMATRIX_SHARED:.c=.o) $(LIBS)
	$(STRIP) $*.$(EXTENSION)
	chmod 755 $*.$(EXTENSION)
	#rm -f -- "$*.o"

$(IEMMATRIX_OBJ) $(IEMMATRIX_SHARED:.c=.o) : %.o : %.c
	$(CC) $(CFLAGS) -o "$*.o" -c "$*.c"

iemmatrix_install: iemmatrix
	install -d $(objectsdir)/$(IEMMATRIX_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEMMATRIX_NAME) \
		--author "IOhannes m zmoelnig (zmoelnig AT iem DOT at), thomas musil (musil AT iem DOT at), franz zotter (zotter AT iem DOT at)" \
		--description "objects for matrix operations and math" \
		--license "GNU GPL" \
		--version "$(IEMMATRIX_VERSION)"
	install -p $(IEMMATRIX_OBJ:.o=.$(EXTENSION)) $(objectsdir)/$(IEMMATRIX_NAME)
	install -p $(shell ls -1 $(externals_src)/iem/iemmatrix/abs/*.* | \
		grep -v '\-help.pd') $(objectsdir)/$(IEMMATRIX_NAME)
	install -d $(helpdir)/$(IEMMATRIX_NAME)
	install -p $(externals_src)/iem/iemmatrix/doc/*.pd \
		$(helpdir)/$(IEMMATRIX_NAME)
	install -p $(externals_src)/iem/iemmatrix/abs/*-help.pd \
		$(helpdir)/$(IEMMATRIX_NAME)
	install -d $(manualsdir)/$(IEMMATRIX_NAME)
	install -p $(externals_src)/iem/iemmatrix/*.txt \
		$(manualsdir)/$(IEMMATRIX_NAME)

iemmatrix_clean:
	-rm -f -- $(IEMMATRIX_OBJ:.o=.$(EXTENSION))
	-rm -f -- $(externals_src)/iem/iemmatrix/src/*.o
	-rm -f -- $(externals_src)/iem/iemmatrix/alias/*.o
	-rm -f -- $(externals_src)/iem/iemmatrix/src/*.bak
	-rm -f -- $(externals_src)/iem/iemmatrix/src/*.*~



#------------------------------------------------------------------------------#
# IEM_MATRIX
IEM_MATRIX_NAME=iem_matrix
IEM_MATRIX_SRC := $(wildcard $(externals_src)/iem/iem_matrix/src/matrix_?????*.c)
IEM_MATRIX_OBJ := $(IEM_MATRIX_SRC:.c=.o)

iem_matrix: $(IEM_MATRIX_OBJ:.o=.$(EXTENSION))

iem_matrix_install: iem_matrix
	install -d $(objectsdir)/$(IEM_MATRIX_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEM_MATRIX_NAME) \
		--author "Thomas Musil" \
		--description "audio matrix objects" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEM_MATRIX_OBJ:.o=.$(EXTENSION)) $(objectsdir)/$(IEM_MATRIX_NAME)
	install -d $(helpdir)/$(IEM_MATRIX_NAME)
	install -p $(externals_src)/iem/iem_matrix/help/*.pd \
		$(helpdir)/$(IEM_MATRIX_NAME)

iem_matrix_clean:
	-rm -f -- $(IEM_MATRIX_OBJ:.o=.$(EXTENSION))
	-rm -f -- $(externals_src)/iem/iem_matrix/src/*.o
	-rm -f -- $(externals_src)/iem/iem_matrix/src/*.bak
	-rm -f -- $(externals_src)/iem/iem_matrix/src/*.*~



#------------------------------------------------------------------------------#
# JASCH_LIB
JASCH_LIB_NAME=jasch_lib
JASCH_LIB_OBJECTS := $(wildcard $(externals_src)/jasch_lib/*/*.c)
jasch_lib: $(JASCH_LIB_OBJECTS:.c=.$(EXTENSION))

jasch_lib_install: jasch_lib
	install -d $(objectsdir)/$(JASCH_LIB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(JASCH_LIB_NAME) \
		--author "Jasch" \
		--license "GNU GPLv2" \
		--version ""
	install -p $(JASCH_LIB_OBJECTS:.c=.$(EXTENSION)) \
		$(objectsdir)/$(JASCH_LIB_NAME)
	install -d $(helpdir)/$(JASCH_LIB_NAME)
	install -p $(externals_src)/jasch_lib/*/*-help.pd \
		$(helpdir)/$(JASCH_LIB_NAME)
#	install -d $(manualsdir)/$(JASCH_LIB_NAME)
#	install -p $(externals_src)/jasch_lib/manual.txt \
#		$(manualsdir)/$(JASCH_LIB_NAME)
#	install -d $(readmesdir)
#	install -p $(externals_src)/jasch_lib/README \
#		$(readmesdir)/$(JASCH_LIB_NAME).txt
#	install -d $(examplesdir)/$(JASCH_LIB_NAME)
#	install -p $(externals_src)/jasch_lib/examples/*.pd \
#		$(examplesdir)/$(JASCH_LIB_NAME)

jasch_lib_clean:
	-rm -f -- $(JASCH_LIB_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(JASCH_LIB_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/jasch_lib/*/*.bak
	-rm -f -- $(externals_src)/jasch_lib/*/*.*~



#------------------------------------------------------------------------------#
# LOADERS
LOADERS_NAME=
LOADERS_OBJECTS := $(wildcard $(externals_src)/loaders/*.c)
loaders: $(LOADERS_OBJECTS:.c=.$(EXTENSION))

loaders_install: loaders
	install -d $(objectsdir)/$(LOADERS_NAME)
	install -p $(LOADERS_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(LOADERS_NAME)
	install -d $(helpdir)/$(LOADERS_NAME)
#	install -p $(wildcard $(externals_src)/loaders/help/*.pd) \
#		$(helpdir)/$(LOADERS_NAME)
#	install -d $(manualsdir)/$(LOADERS_NAME)
#	install -p $(externals_src)/loaders/README.txt $(manualsdir)/$(LOADERS_NAME)

loaders_clean:
	-rm -f -- $(externals_src)/loaders/*.$(EXTENSION)
	-rm -f -- $(externals_src)/loaders/*.o
	-rm -f -- $(externals_src)/loaders/*.bak
	-rm -f -- $(externals_src)/loaders/*.*~


#------------------------------------------------------------------------------#
# MAPPING
MAPPING_NAME=mapping
MAPPING_OBJECTS := $(wildcard $(externals_src)/mapping/src/*.c)
mapping: $(MAPPING_OBJECTS:.c=.$(EXTENSION))

mapping_install: mapping
	install -d $(objectsdir)/$(MAPPING_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(MAPPING_NAME) \
		--author "Cyrille Henry and Hans-Christoph Steiner" \
		--description "objects for mapping data to control" \
		--license "GNU GPL" \
		--version ""
# no compiled objects yet...
#	install -p $(MAPPING_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(MAPPING_NAME)
	install -p $(externals_src)/mapping/*.pd $(objectsdir)/$(MAPPING_NAME)
	install -d $(helpdir)/$(MAPPING_NAME)
	install -p $(externals_src)/mapping/help/*.pd \
		$(helpdir)/$(MAPPING_NAME)
#	install -d $(manualsdir)/$(MAPPING_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/mapping/README.txt \
		$(readmesdir)/$(MAPPING_NAME).txt
	install -d $(examplesdir)/$(MAPPING_NAME)
#	install -p $(externals_src)/mapping/examples/*.pd \
#		$(examplesdir)/$(MAPPING_NAME)

mapping_clean:
	-rm -f -- $(externals_src)/mapping/*.$(EXTENSION)
	-rm -f -- $(externals_src)/mapping/src/*.o
	-rm -f -- $(externals_src)/mapping/*.bak
	-rm -f -- $(externals_src)/mapping/*.*~


#------------------------------------------------------------------------------#
# MARKEX
MARKEX_NAME=markex
MARKEX_OBJECTS := $(wildcard $(externals_src)/markex/[a-z]*.c)
markex: $(MARKEX_OBJECTS:.c=.$(EXTENSION))

markex_install: markex
	install -d $(objectsdir)/$(MARKEX_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(MARKEX_NAME) \
		--author "Mark Danks" \
		--license "GNU GPL"
	install -p $(MARKEX_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(MARKEX_NAME)
	install -d $(helpdir)/$(MARKEX_NAME)
	install -p $(wildcard $(externals_src)/markex/*.pd) \
		$(helpdir)/$(MARKEX_NAME)
#	install -d $(manualsdir)/$(MARKEX_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/markex/README \
		$(readmesdir)/$(MARKEX_NAME).txt

markex_clean:
	-rm -f -- $(externals_src)/markex/*.$(EXTENSION)
	-rm -f -- $(externals_src)/markex/*.o
	-rm -f -- $(externals_src)/markex/*.bak
	-rm -f -- $(externals_src)/markex/*.*~


#------------------------------------------------------------------------------#
# MAXLIB
MAXLIB_NAME=maxlib
MAXLIB_OBJECTS := $(wildcard $(externals_src)/maxlib/src/*.c)
maxlib: $(MAXLIB_OBJECTS:.c=.$(EXTENSION))

maxlib_install: maxlib
	install -d $(objectsdir)/$(MAXLIB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(MAXLIB_NAME) \
		--author "Olaf Matthes <olaf.matthes@gmx.de>" \
		--license "GNU GPL" \
		--version "1.5.2-cvs"
	install -p $(MAXLIB_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(MAXLIB_NAME)
	install -d $(helpdir)/$(MAXLIB_NAME)
	install -p $(wildcard $(externals_src)/maxlib/help/*.pd) \
		$(helpdir)/$(MAXLIB_NAME)
#	install -d $(manualsdir)/$(MAXLIB_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/maxlib/README \
		$(readmesdir)/$(MAXLIB_NAME).txt

maxlib_clean:
	-rm -f -- $(externals_src)/maxlib/src/*.$(EXTENSION)
	-rm -f -- $(externals_src)/maxlib/src/*.o
	-rm -f -- $(externals_src)/maxlib/*/*.bak
	-rm -f -- $(externals_src)/maxlib/*/*.*~
	-rm -f -- $(externals_src)/maxlib/*.*~


#------------------------------------------------------------------------------#
# MJLIB
MJLIB_NAME=mjlib
# exclude the library file mjLib.c
MJLIB_OBJECTS := $(wildcard $(externals_src)/mjlib/[a-z]?[a-z]*.c)
mjlib: $(MJLIB_OBJECTS:.c=.$(EXTENSION))

mjlib_install: mjlib
	install -d $(objectsdir)/$(MJLIB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(MJLIB_NAME) \
		--author "mark williamson <mark@junklight.com>" \
		--license "GNU GPL" \
		--version "0.1"
	install -p $(MJLIB_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(MJLIB_NAME)
	install -d $(helpdir)/$(MJLIB_NAME)
	install -p $(wildcard $(externals_src)/mjlib/doc/*.pd) \
		$(helpdir)/$(MJLIB_NAME)
#	install -d $(manualsdir)/$(MJLIB_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/mjlib/readme.txt \
		$(readmesdir)/$(MJLIB_NAME).txt

mjlib_clean:
	-rm -f -- $(externals_src)/mjlib/*.$(EXTENSION)
	-rm -f -- $(externals_src)/mjlib/*.o
	-rm -f -- $(externals_src)/mjlib/*.bak
	-rm -f -- $(externals_src)/mjlib/*.*~



#------------------------------------------------------------------------------#
# MOONLIB
MOONLIB_NAME=moonlib
MOONLIB_OBJECTS := $(wildcard $(externals_src)/moonlib/*.c)
moonlib: $(MOONLIB_OBJECTS:.c=.$(EXTENSION))

moonlib_install: moonlib
	install -d $(objectsdir)/$(MOONLIB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(MOONLIB_NAME) \
		--author "Antoine Rousseau" \
		--version "0.2"
	install -p $(MOONLIB_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(MOONLIB_NAME)
	install -d $(helpdir)/$(MOONLIB_NAME)
	install -p $(externals_src)/moonlib/help/*.pd $(helpdir)/$(MOONLIB_NAME)
	install -p $(externals_src)/moonlib/help/*.gif $(helpdir)/$(MOONLIB_NAME)
	install -d $(helpdir)/$(MOONLIB_NAME)/d
	install -p $(externals_src)/moonlib/help/d/*.gif $(helpdir)/$(MOONLIB_NAME)/d
#	install -d $(manualsdir)/$(MOONLIB_NAME)
#	install -p $(externals_src)/moonlib/manual.txt \
#		$(manualsdir)/$(MOONLIB_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/moonlib/README.txt \
		$(readmesdir)/$(MOONLIB_NAME).txt
	install -d $(examplesdir)/$(MOONLIB_NAME)
#	install -p $(externals_src)/moonlib/examples/*.pd \
#		$(examplesdir)/$(MOONLIB_NAME)

moonlib_clean:
	-rm -f -- $(MOONLIB_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(MOONLIB_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/moonlib/*.bak
	-rm -f -- $(externals_src)/moonlib/*.*~



#------------------------------------------------------------------------------#
# MOTEX
MOTEX_NAME=motex
MOTEX_OBJECTS := $(wildcard $(externals_src)/motex/*.c)
motex: $(MOTEX_OBJECTS:.c=.$(EXTENSION))

motex_install: motex
	install -d $(objectsdir)/$(MOTEX_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(MOTEX_NAME) \
		--author "Iain Mott <iain.mott@bigpond.com>" \
		--license "GNU GPL" \
		--version "1.1.3-cvs"
	install -p $(MOTEX_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(MOTEX_NAME)
	install -p $(externals_src)/motex/noisegate~.pd $(objectsdir)/$(MOTEX_NAME)
	install -d $(helpdir)/$(MOTEX_NAME)
	install -p $(wildcard $(externals_src)/motex/*-help.pd) \
		$(helpdir)/$(MOTEX_NAME)
#	install -d $(manualsdir)/$(MOTEX_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/motex/README \
		$(readmesdir)/$(MOTEX_NAME).txt

motex_clean:
	-rm -f -- $(externals_src)/motex/*.$(EXTENSION)
	-rm -f -- $(externals_src)/motex/*.o
	-rm -f -- $(externals_src)/motex/*.bak
	-rm -f -- $(externals_src)/motex/*.*~




#------------------------------------------------------------------------------#
# MRPEACH
MRPEACH_NAME=mrpeach
MRPEACH_OBJECTS := $(wildcard $(externals_src)/mrpeach/*/*.c)
mrpeach: $(MRPEACH_OBJECTS:.c=.$(EXTENSION))

mrpeach_install: mrpeach
	install -d $(objectsdir)/$(MRPEACH_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(MRPEACH_NAME) \
		--author "Martin Peach <martin.peach@sympatico.ca>" \
		--description "" \
		--license "GNU GPL" \
		--version "0.1"
	install -p $(MRPEACH_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(MRPEACH_NAME)
	install -d $(helpdir)/$(MRPEACH_NAME)
	install -p $(externals_src)/mrpeach/*/*-help.pd \
		$(helpdir)/$(MRPEACH_NAME)
#	install -d $(manualsdir)/$(MRPEACH_NAME)
#	install -p $(externals_src)/mrpeach/manual.txt \
#		$(manualsdir)/$(MRPEACH_NAME)
#	install -d $(readmesdir)
#	install -p $(externals_src)/mrpeach/README \
#		$(readmesdir)/$(MRPEACH_NAME).txt
#	install -d $(examplesdir)/$(MRPEACH_NAME)
#	install -p $(externals_src)/mrpeach/examples/*.pd \
#		$(examplesdir)/$(MRPEACH_NAME)

mrpeach_clean:
	-rm -f -- $(MRPEACH_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(MRPEACH_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/mrpeach/*.bak
	-rm -f -- $(externals_src)/mrpeach/*.*~




#------------------------------------------------------------------------------#
# MSD
MSD_NAME=msd
MSD_OBJECTS := $(wildcard $(externals_src)/nusmuk/msd*/*.cpp)
#msd: $(MSD_OBJECTS:.c=.$(EXTENSION))
msd:

msd_install: msd
	install -d $(objectsdir)/$(MSD_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(MSD_NAME) \
		--author "Nicolas Montgermont, Cyrille Henry, and Frank Barknecht" \
		--description "Mass Spring Damper modeling for Pd" \
		--license "GNU LGPL" \
		--version "0.07"
#	install -p $(MSD_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(MSD_NAME)
	install -d $(helpdir)/$(MSD_NAME)
	install -p $(externals_src)/nusmuk/msd*/*-help.pd \
		$(helpdir)/$(MSD_NAME)
#	install -d $(manualsdir)/$(MSD_NAME)
#	install -p $(externals_src)/msd/manual.txt \
#		$(manualsdir)/$(MSD_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/nusmuk/README.txt \
		$(readmesdir)/$(MSD_NAME).txt
	install -d $(examplesdir)/$(MSD_NAME)
	install -d $(examplesdir)/$(MSD_NAME)/msd
	install -p $(externals_src)/nusmuk/msd/*.pd \
		$(examplesdir)/$(MSD_NAME)/msd
	install -d $(examplesdir)/$(MSD_NAME)/msd2D
	install -p $(externals_src)/nusmuk/msd2D/*.pd \
		$(examplesdir)/$(MSD_NAME)/msd2D
	install -d $(examplesdir)/$(MSD_NAME)/msd3D
	install -p $(externals_src)/nusmuk/msd3D/*.pd \
		$(examplesdir)/$(MSD_NAME)/msd3D
	install -d $(examplesdir)/$(MSD_NAME)/editor
	install -p $(externals_src)/nusmuk/editor/*.* \
		$(examplesdir)/$(MSD_NAME)/editor
	install -p $(externals_src)/nusmuk/editor/README \
		$(examplesdir)/$(MSD_NAME)/editor/README.txt

msd_clean:
	-rm -f -- $(MSD_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(MSD_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/msd/*.bak
	-rm -f -- $(externals_src)/msd/*.*~



#----------------------------------------------------------------------------
# OSCx
OSCX_NAME=oscx
$(externals_src)/OSCx/configure: $(externals_src)/OSCx/configure.ac
	cd $(externals_src)/OSCx && autoconf

$(externals_src)/OSCx/Makefile: $(externals_src)/OSCx/Makefile.in
	cd $(externals_src)/OSCx && ./configure
$(externals_src)/OSCx/libOSC/Makefile: $(externals_src)/OSCx/libOSC/Makefile.in
	cd $(externals_src)/OSCx && ./configure
$(externals_src)/OSCx/src/Makefile: $(externals_src)/OSCx/src/Makefile.in
	cd $(externals_src)/OSCx && ./configure

$(externals_src)/OSCx/src/OSCroute.$(EXTENSION):  $(externals_src)/OSCx/configure \
$(externals_src)/OSCx/Makefile
	$(MAKE) -C $(externals_src)/OSCx
$(externals_src)/OSCx/src/dumpOSC.$(EXTENSION):  $(externals_src)/OSCx/configure \
$(externals_src)/OSCx/Makefile
	$(MAKE) -C $(externals_src)/OSCx
$(externals_src)/OSCx/src/sendOSC.$(EXTENSION):  $(externals_src)/OSCx/configure \
$(externals_src)/OSCx/Makefile
	$(MAKE) -C $(externals_src)/OSCx

oscx: $(externals_src)/OSCx/src/OSCroute.$(EXTENSION) \
$(externals_src)/OSCx/src/dumpOSC.$(EXTENSION) \
$(externals_src)/OSCx/src/sendOSC.$(EXTENSION)

oscx_install: oscx
	install -d $(objectsdir)/$(OSCX_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(OSCX_NAME) \
		--author "<jdl@xdv.org>" \
		--license "BSD" \
		--description "objects for working with OpenSoundControl"
	install -p $(externals_src)/OSCx/src/*.$(EXTENSION) $(objectsdir)/$(OSCX_NAME)
	install -d $(helpdir)/$(OSCX_NAME)
	install -p $(externals_src)/OSCx/doc/*.* $(helpdir)/$(OSCX_NAME)


oscx_clean:
	-$(MAKE) -C $(externals_src)/OSCx $(DEST_PATHS) clean
	-$(MAKE) -C $(externals_src)/OSCx CC=gcc clean
	-rm $(externals_src)/OSCx/Makefile




#------------------------------------------------------------------------------#
# PAN
PAN_NAME=pan
PAN_OBJECTS := $(wildcard $(externals_src)/hcs/pan/*.c)
pan: $(PAN_OBJECTS:.c=.$(EXTENSION))

pan_install: pan
	install -d $(objectsdir)/$(PAN_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(PAN_NAME) \
		--author "Hans-Christoph Steiner <hans@at.or.at>" \
		--description "A library of panning algoritms" \
		--license "GNU GPL" \
		--version ""
# everything is .pd, nothing to compile (yet?)
#	install -p $(PAN_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(PAN_NAME)
	install -p $(wildcard $(externals_src)/hcs/pan/*.pd) \
		$(objectsdir)/$(PAN_NAME)
	install -d $(helpdir)/$(PAN_NAME)
	install -p $(externals_src)/hcs/pan/help/*.pd \
		$(helpdir)/$(PAN_NAME)
#	install -d $(manualsdir)/$(PAN_NAME)
#	install -p $(externals_src)/hcs/pan/manual.txt \
#		$(manualsdir)/$(PAN_NAME)
#	install -d $(readmesdir)
#	install -p $(externals_src)/hcs/pan/README \
#		$(readmesdir)/$(PAN_NAME).txt
#	install -d $(examplesdir)/$(PAN_NAME)
#	install -p $(externals_src)/hcs/pan/examples/*.pd \
#		$(examplesdir)/$(PAN_NAME)


pan_clean:
	-rm -f -- $(PAN_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(PAN_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/hcs/pan/*.bak
	-rm -f -- $(externals_src)/hcs/pan/*.*~




#------------------------------------------------------------------------------#
# PDDP
PDDP_NAME=pddp
pddp:


pddp_install: pddp
	install -d $(objectsdir)/$(PDDP_NAME)
#	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(PDDP_NAME) \
#		--author "PDDP" \
#		--license "BSD" \
#		--description "objects for working with Tcl and Pd's Tk GUI"
# pddp is compiled straight into $(OUT_DIR)
	cd $(externals_src)/miXed/pddp && $(MAKE) \
		OUT_DIR=$(objectsdir)/$(PDDP_NAME)
	install -d $(objectsdir)/$(PDDP_NAME)/pddp
	install -p $(externals_src)/miXed/bin/pddp/*.tcl \
		$(objectsdir)/$(PDDP_NAME)/pddp
#	install -d $(helpdir)/$(PDDP_NAME)
#	install -d $(manualsdir)/$(PDDP_NAME)
	install -d $(examplesdir)/$(PDDP_NAME)
	install -p $(externals_src)/miXed/test/pddp/*.*  \
		$(examplesdir)/$(PDDP_NAME)


pddp_clean:
	-$(MAKE) -C $(externals_src)/mixed/pddp $(DEST_PATHS) clean



#------------------------------------------------------------------------------#
# PDOGG
PDOGG_NAME=pdogg
PDOGG_OBJECTS := $(wildcard $(externals_src)/pdogg/*/*.c)
pdogg: $(PDOGG_OBJECTS:.c=.$(EXTENSION))

pdogg_install: pdogg
	install -d $(objectsdir)/$(PDOGG_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(PDOGG_NAME) \
		--author "Olaf Matthes" \
		--description "objects for reading, writing, and streaming ogg" \
		--license "LGPL" \
		--version "0.2"
	install -p $(PDOGG_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(PDOGG_NAME)
	install -d $(helpdir)/$(PDOGG_NAME)
	install -p $(externals_src)/pdogg/*/*.pd \
		$(helpdir)/$(PDOGG_NAME)
#	install -d $(manualsdir)/$(PDOGG_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/pdogg/readme \
		$(readmesdir)/$(PDOGG_NAME).txt

pdogg_clean:
	-rm -f -- $(externals_src)/pdogg/*/*.$(EXTENSION)
	-rm -f -- $(externals_src)/pdogg/*/*.o
	-rm -f -- $(externals_src)/pdogg/*/*.bak
	-rm -f -- $(externals_src)/pdogg/*/*.*~
	-rm -f -- $(externals_src)/pdogg/*.*~



#------------------------------------------------------------------------------#
# PDP
PDP_NAME=pdp
$(externals_src)/pdp/configure: $(externals_src)/pdp/configure.ac
	cd $(externals_src)/pdp && autoconf

$(externals_src)/pdp/Makefile.config: $(externals_src)/pdp/Makefile.config.in
	cd $(externals_src)/pdp && ./configure PD_CPPFLAGS="-I$(pd_src)/src"
		--disable-quicktime

$(externals_src)/pdp/pdp.$(EXTENSION):  $(externals_src)/pdp/configure \
$(externals_src)/pdp/Makefile.config 
	$(MAKE) -C $(externals_src)/pdp PD_EXECUTABLE="$(pd_src)/bin/pd"

pdp: $(externals_src)/pdp/pdp.$(EXTENSION)

pdp_install: pdp
#	install -d $(objectsdir)/$(PDP_NAME)
#	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(PDP_NAME) \
#		--author "Tom Schouten" \
#		--description "Pure Data Packet" \
#		--license "GNU GPL"
	install -p $(externals_src)/pdp/*.$(EXTENSION) $(objectsdir)
#	install -p $(externals_src)/pdp/abstractions/*.pd $(objectsdir)/$(PDP_NAME)
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

pdp_clean:
	-rm -f -- $(externals_src)/pdp/*.$(EXTENSION)
	-find $(externals_src)/pdp -name '*.o' | xargs rm -f --
	-find $(externals_src)/pdp -name '*.bak' | xargs rm -f --
	-rm -f -- $(externals_src)/pdp/Makefile.config
	-rm -f -- $(externals_src)/pdp/configure



#------------------------------------------------------------------------------#
# PIDIP
PIDIP_NAME=pidip
$(externals_src)/pidip/configure: $(externals_src)/pidip/configure.ac
	cd $(externals_src)/pidip && autoconf

$(externals_src)/pidip/Makefile: $(externals_src)/pidip/Makefile.in
	-cd $(externals_src)/pidip && ./configure --with-pd=$(pd_src) \
		--with-pdp=$(externals_src)/pdp

$(externals_src)/pidip/pidip.$(EXTENSION):  $(externals_src)/pidip/configure \
$(externals_src)/pidip/Makefile
	-$(MAKE) -C $(externals_src)/pidip

pidip: $(externals_src)/pidip/pidip.$(EXTENSION)

pidip_install: pidip
#	install -d $(objectsdir)/$(PIDIP_NAME)
#	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(PIDIP_NAME) \
#		--author "Yves Degoyon" \
#		--description "PiDiP is Definitely in Pieces" \
#		--license "GNU GPLv2"
	install -p $(externals_src)/pidip/*.$(EXTENSION) $(objectsdir)
	install -d $(helpdir)
	install -p $(externals_src)/pidip/doc/*.pd $(helpdir)
	install -d $(examplesdir)/$(PIDIP_NAME)
	install -p $(externals_src)/pidip/patches/*.* $(examplesdir)/$(PIDIP_NAME)
#	install -d $(manualsdir)/$(PIDIP_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/pidip/README \
		$(readmesdir)/$(PIDIP_NAME).txt
	install -d $(examplesdir)/$(PIDIP_NAME)/images
	install -p $(externals_src)/pidip/patches/images/*.* \
		$(examplesdir)/$(PIDIP_NAME)/images
	install -d $(examplesdir)/$(PIDIP_NAME)/morphology
	install -p $(externals_src)/pidip/patches/morphology/*.* \
		$(examplesdir)/$(PIDIP_NAME)/morphology

pidip_clean:
	-rm -f -- $(externals_src)/pidip/*.$(EXTENSION)
	-find $(externals_src)/pidip -name '*.o' | xargs rm -f --
	-rm -f -- $(externals_src)/pidip/*.bak
	-rm -f -- $(externals_src)/pidip/Makefile
	-rm -f -- $(externals_src)/pidip/configure


#------------------------------------------------------------------------------#
# PMPD
PMPD_NAME=pmpd
PMPD_OBJECTS := $(wildcard $(externals_src)/pmpd/src/[a-oq-z]*.c) \
$(externals_src)/pmpd/src/pmpd~.c
pmpd: $(PMPD_OBJECTS:.c=.$(EXTENSION))

pmpd_install: pmpd
	install -d $(objectsdir)/$(PMPD_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(PMPD_NAME) \
		--author "Cyrille Henry" \
		--description "Physical Modelling for Pd"
	-install -p $(externals_src)/pmpd/*.$(EXTENSION) $(objectsdir)/$(PMPD_NAME)
	install -d $(helpdir)/$(PMPD_NAME)
	install -p $(externals_src)/pmpd/help/*.pd $(helpdir)
	install -p $(externals_src)/pmpd/help/*.pd $(helpdir)/$(PMPD_NAME)
	install -d $(manualsdir)/$(PMPD_NAME)
	install -d $(manualsdir)/$(PMPD_NAME)
	install -p $(externals_src)/pmpd/doc/pmpd.pdf/pmpd.pdf \
		$(manualsdir)/$(PMPD_NAME)
	install -d $(examplesdir)/$(PMPD_NAME)
	install -p $(externals_src)/pmpd/exemples/*.pd \
		$(examplesdir)/$(PMPD_NAME)

pmpd_clean:
	-rm -f -- $(PMPD_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(externals_src)/pmpd/*/*.o
	-rm -f -- $(externals_src)/pmpd/*/*.bak
	-rm -f -- $(externals_src)/pmpd/*/*~



#------------------------------------------------------------------------------#
# POSTLUDE
POSTLUDE_NAME=postlude
# flib is separate, so exclude it here
POSTLUDE_OBJECTS := $(wildcard $(externals_src)/postlude/[a-eg-z]*/src/*.c)
postlude: $(POSTLUDE_OBJECTS:.c=.$(EXTENSION))

postlude_install: postlude
	install -d $(objectsdir)/$(POSTLUDE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(POSTLUDE_NAME) \
		--author "Jamie Bullock" \
		--license "GNU GPL"
	install -p $(POSTLUDE_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(POSTLUDE_NAME)
	install -d $(helpdir)/$(POSTLUDE_NAME)
	install -p $(externals_src)/postlude/*/doc/help-*.pd \
		$(helpdir)/$(POSTLUDE_NAME)
#	install -d $(manualsdir)/$(POSTLUDE_NAME)
#	install -p $(externals_src)/postlude/manual.txt \
#		$(manualsdir)/$(POSTLUDE_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/postlude/dssi/README \
		$(readmesdir)/$(POSTLUDE_NAME)-dssi.txt
	install -p $(externals_src)/postlude/getpatchname/README \
		$(readmesdir)/$(POSTLUDE_NAME)-getpatchname.txt
	install -d $(examplesdir)/$(POSTLUDE_NAME)
#	install -p $(externals_src)/postlude/examples/*.pd \
#		$(examplesdir)/$(POSTLUDE_NAME)

postlude_clean:
	-rm -f -- $(POSTLUDE_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(POSTLUDE_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/postlude/*/*.bak
	-rm -f -- $(externals_src)/postlude/*/*.*~



#------------------------------------------------------------------------------#
# SIGPACK
SIGPACK_NAME=sigpack
SIGPACK_OBJECTS := $(wildcard $(externals_src)/sigpack/source/*~.c)
SIGPACK_VERSION := $(shell grep "define VERSION" \
	$(externals_src)/sigpack/source/sIgpAck.c | cut -d '"' -f 2)

sigpack: $(SIGPACK_OBJECTS:.c=.$(EXTENSION))

sigpack_install: sigpack
	install -d $(objectsdir)/$(SIGPACK_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(SIGPACK_NAME) \
		--author "weiss@weiss-archiv.de" \
		--license "GNU GPL 2" \
		--version "$(SIGPACK_VERSION)"
	install -p $(SIGPACK_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(SIGPACK_NAME)
	install -d $(helpdir)/$(SIGPACK_NAME)
	install -p $(externals_src)/sigpack/help/*.pd \
		$(helpdir)/$(SIGPACK_NAME)
#	install -d $(manualsdir)/$(SIGPACK_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/sigpack/readme.txt \
		$(readmesdir)/$(SIGPACK_NAME).txt

sigpack_clean:
	-rm -f -- $(externals_src)/sigpack/source/*.$(EXTENSION)
	-rm -f -- $(externals_src)/sigpack/source/*.o
	-rm -f -- $(externals_src)/sigpack/*/*.bak
	-rm -f -- $(externals_src)/sigpack/*/*.*~


#------------------------------------------------------------------------------#
# SMLIB
SMLIB_NAME=smlib
# exclude SMlib.c since its just for the compiled library
SMLIB_OBJECTS := $(wildcard $(externals_src)/smlib/source/[a-z]*.c)
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
#	install -d $(manualsdir)/$(SMLIB_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/smlib/readme.txt $(readmesdir)/$(SMLIB_NAME).txt
	install -d $(examplesdir)/$(SMLIB_NAME)
	install -p $(wildcard $(externals_src)/smlib/examples/*.pd) \
		$(examplesdir)/$(SMLIB_NAME)

smlib_clean:
	-rm -f -- $(SMLIB_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(externals_src)/smlib/*.o
	-rm -f -- $(externals_src)/smlib/*.bak
	-rm -f -- $(externals_src)/smlib/*.*~


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


toxy_clean:
	-$(MAKE) -C $(externals_src)/mixed/toxy $(DEST_PATHS) clean


#------------------------------------------------------------------------------#
# UNAUTHORIZED
UNAUTHORIZED_NAME=unauthorized
# these need to be created before compiling the C
UNAUTHORIZED_TKFILES = $(wildcard $(externals_src)/unauthorized/*/*.tk)
%.tk2c: %.tk
	bash $(externals_src)/unauthorized/tk2c.bash < $*.tk > $*.tk2c

# a number of objects don't compile under MinGW (yet? they used to, but the
# needed changes were overwritten, so they are in CVS
ifeq ($(OS_NAME),windows)
UNAUTHORIZED_OBJECTS := $(wildcard $(externals_src)/unauthorized/[abdg-oqrw]*/*.c)\
$(wildcard $(externals_src)/unauthorized/c?[a-np-z]*/*.c)
else
 ifeq ($(OS_NAME),darwin)
 # [cooled~] crashes Pd on Mac OS X, [vocoder~] doesn't compile
  UNAUTHORIZED_OBJECTS := $(wildcard $(externals_src)/unauthorized/[abd-uw-z]*/*.c)\
  $(wildcard $(externals_src)/unauthorized/c?[a-np-z]*/*.c)
 else
   # GNU/Linux, BSD, IRIX, etc.
  UNAUTHORIZED_OBJECTS := $(wildcard $(externals_src)/unauthorized/*/*.c)
 endif
endif


# [vocoder~] is built separately since its made from a number of files
UNAUTHORIZED_VOCODER = $(wildcard $(externals_src)/unauthorized/vocoder*/*.c)
 $(externals_src)/unauthorized/vocoder~/vocoder~.$(EXTENSION): $(UNAUTHORIZED_VOCODER:.c=.o)
	$(CC) $(LDFLAGS) -o $(externals_src)/unauthorized/vocoder~/vocoder~.$(EXTENSION) \
		$(UNAUTHORIZED_VOCODER:.c=.o) $(LIBS)

unauthorized: $(externals_src)/unauthorized/vocoder~/vocoder~.$(EXTENSION) \
$(UNAUTHORIZED_TKFILES:.tk=.tk2c) $(UNAUTHORIZED_OBJECTS:.c=.$(EXTENSION))

unauthorized_install: unauthorized
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
	install -p $(externals_src)/unauthorized/*/*.pls \
		$(helpdir)/$(UNAUTHORIZED_NAME)
	install -d $(helpdir)/$(UNAUTHORIZED_NAME)/blm
	install -p $(externals_src)/unauthorized/blinkenlights/blm/*.* \
		$(helpdir)/$(UNAUTHORIZED_NAME)/blm
	install -d $(manualsdir)/$(UNAUTHORIZED_NAME)
	install -p $(externals_src)/unauthorized/*/*.txt \
		$(manualsdir)/$(UNAUTHORIZED_NAME)

unauthorized_clean:
	-rm -f -- $(UNAUTHORIZED_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(UNAUTHORIZED_OBJECTS:.c=.o)
	-rm -f -- $(UNAUTHORIZED_VOCODER:.c=.$(EXTENSION))
	-rm -f -- $(UNAUTHORIZED_VOCODER:.c=.o)
	-rm -f -- $(externals_src)/unauthorized/*/*.bak
	-rm -f -- $(externals_src)/*/*.*~
	-rm -f -- $(externals_src)/*.*~



#------------------------------------------------------------------------------#
# USBHID
USBHID_NAME=usbhid
USBHID_OBJECTS := $(wildcard $(externals_src)/hcs/usbhid/*.c)
usbhid: $(USBHID_OBJECTS:.c=.$(EXTENSION))

usbhid_install: usbhid
	install -d $(objectsdir)/$(USBHID_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(USBHID_NAME) \
		--author "Hans-Christoph Steiner" \
		--description "pure USB HID object for accessing HID devices" \
		--license "GNU GPL" \
		--version ""
	install -p $(USBHID_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(USBHID_NAME)
	install -d $(helpdir)/$(USBHID_NAME)
	install -p $(externals_src)/hcs/usbhid/*-help.pd \
		$(helpdir)/$(USBHID_NAME)
#	install -d $(manualsdir)/$(USBHID_NAME)
#	install -p $(externals_src)/usbhid/manual.txt \
#		$(manualsdir)/$(USBHID_NAME)
#	install -d $(readmesdir)
#	install -p $(externals_src)/usbhid/README \
#		$(readmesdir)/$(USBHID_NAME).txt
#	install -d $(examplesdir)/$(USBHID_NAME)
#	install -p $(externals_src)/usbhid/examples/*.pd \
#		$(examplesdir)/$(USBHID_NAME)

usbhid_clean:
	-rm -f -- $(USBHID_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(USBHID_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/usbhid/*.bak
	-rm -f -- $(externals_src)/usbhid/*.*~



#------------------------------------------------------------------------------#
# VBAP
VBAP_NAME=vbap_ext
VBAP_OBJECTS := $(wildcard $(externals_src)/vbap/*.c)
vbap: $(VBAP_OBJECTS:.c=.$(EXTENSION))

vbap_install: vbap
	install -d $(objectsdir)/$(VBAP_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(VBAP_NAME) \
		--author "Ville Pulkki" \
		--description "Vector Based Amplitude Panning" \
		--version ""
	install -p $(VBAP_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(VBAP_NAME)
	install -p $(shell ls -1 $(externals_src)/vbap/*.pd | grep -v '-help.pd') \
		 $(objectsdir)/$(VBAP_NAME)
	install -d $(helpdir)/$(VBAP_NAME)
	install -p $(externals_src)/vbap/*-help.pd $(helpdir)/$(VBAP_NAME)
#	install -d $(manualsdir)/$(VBAP_NAME)
#	install -p $(externals_src)/vbap/manual.txt \
#		$(manualsdir)/$(VBAP_NAME)
	install -d $(examplesdir)/$(VBAP_NAME)
	install -p $(externals_src)/vbap/*-demo.pd \
		$(examplesdir)/$(VBAP_NAME)

vbap_clean:
	-rm -f -- $(VBAP_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(VBAP_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/vbap/*.bak
	-rm -f -- $(externals_src)/vbap/*.*~



#------------------------------------------------------------------------------#
# WIIREMOTE
# this is installed into the "io" library
WIIREMOTE_NAME=io
WIIREMOTE_SRC := $(wildcard $(externals_src)/io/wiiremote/*.c)

WIIREMOTE_OBJECTS := $(WIIREMOTE_SRC:.c=.o)
$(WIIREMOTE_OBJECTS) : %.o : %.c
	$(CC) $(CFLAGS) -o "$*.o" -c "$*.c"

$(externals_src)/io/wiiremote/wiiremote.$(EXTENSION): $(WIIREMOTE_OBJECTS) 
	$(CC) $(LDFLAGS) -o $(externals_src)/io/wiiremote/wiiremote.$(EXTENSION) \
		$(WIIREMOTE_OBJECTS) -weak_framework IOBluetooth \
		-weak_framework CoreFoundation
#	$(STRIP) $(externals_src)/io/wiiremote/wiiremote.$(EXTENSION)

wiiremote: $(externals_src)/io/wiiremote/wiiremote.$(EXTENSION)

wiiremote_install: wiiremote
	install -d $(objectsdir)/$(WIIREMOTE_NAME)
	install -p $(externals_src)/io/wiiremote/wiiremote.$(EXTENSION) \
		$(objectsdir)/$(WIIREMOTE_NAME)
#	install -d $(helpdir)/$(WIIREMOTE_NAME)
#	install -p $(externals_src)/io/wiiremote/*.pd \
#		$(helpdir)/$(WIIREMOTE_NAME)
#	install -d $(manualsdir)/$(WIIREMOTE_NAME)
#	install -p $(externals_src)/io/wiiremote/manual.txt \
#		$(manualsdir)/$(WIIREMOTE_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/io/wiiremote/COPYRIGHT.txt \
		$(readmesdir)/$(WIIREMOTE_NAME).txt
#	install -d $(examplesdir)/$(WIIREMOTE_NAME)
#	install -p $(externals_src)/io/wiiremote/examples/*.pd \
#		$(examplesdir)/$(WIIREMOTE_NAME)

wiiremote_clean:
	-rm -f -- $(externals_src)/io/wiiremote/wiiremote.$(EXTENSION)
	-rm -f -- $(WIIREMOTE_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/io/wiiremote/*.bak
	-rm -f -- $(externals_src)/io/wiiremote/*.*~



#------------------------------------------------------------------------------#
# Zexy
ZEXY_NAME = zexy
ZEXY_OBJECTS := $(wildcard $(externals_src)/zexy/src/[a-vxy0]*.c) \
$(externals_src)/zexy/src/wrap.c $(externals_src)/zexy/src/z~.c
ZEXY_VERSION := $(shell grep VERSION $(externals_src)/zexy/src/zexy.h | \
	grep -v _VERSION | cut -d ' ' -f 3 | cut -d '"' -f 2)

$(externals_src)/zexy/src/configure: $(externals_src)/zexy/src/configure.ac
	cd $(externals_src)/zexy/src && autoconf

$(externals_src)/zexy/src/zexyconf.h:
	cd $(externals_src)/zexy/src && autoconf

$(externals_src)/zexy/src/Make.config: $(externals_src)/zexy/src/Make.config.in \
$(externals_src)/zexy/src/configure $(externals_src)/zexy/src/zexyconf.h
	cd $(externals_src)/zexy/src/ && ./configure --disable-library \
		--with-pd=$(pd_src)

zexy: $(externals_src)/zexy/src/Make.config
	$(MAKE) -C $(externals_src)/zexy/src/ 

zexy_install: zexy
	install -d $(objectsdir)/$(ZEXY_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(ZEXY_NAME) \
		--author "IOhannes m zmoelnig <zmoelnig@iem.kug.ac.at>" \
		--license "GNU GPL" \
		--description "swiss army knife for pd" \
		--version "$(ZEXY_VERSION)"
	install -p $(ZEXY_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(ZEXY_NAME)
# the below doesn't work for some reason, probably because of some kind of
# strange make execution order <hans@at.or.at>
#	install -p $(shell $(externals_src)/zexy/src/*.$(EXTENSION)) \
#		$(objectsdir)/$(ZEXY_NAME)
	install -p $(shell ls -1 $(externals_src)/zexy/abs/*.pd | \
		grep -v '-help.pd' | sed 's|\(.*\)|"\1"|g') \
		$(objectsdir)/$(ZEXY_NAME)
# an escape pattern for the abstractions with special characters
#		grep -v '-help.pd' | sed 's/\([&<>|~]\)/\\\1/g') 
	install -d $(helpdir)/$(ZEXY_NAME)
	install -p $(shell ls -1 $(externals_src)/zexy/abs/*-help.pd | \
		sed 's|\(.*\)|"\1"|g') $(helpdir)/$(ZEXY_NAME)
	install -p $(externals_src)/zexy/zexy.pd  $(helpdir)
	install -p $(externals_src)/zexy/examples/*.* $(helpdir)/$(ZEXY_NAME)

zexy_clean:
	-$(MAKE) -C $(externals_src)/zexy/src clean
	-rm -f -- $(ZEXY_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(ZEXY_OBJECTS:.c=.d)
	-rm -rf -- $(externals_src)/zexy/src/autom4ate
	-rm -rf -- $(externals_src)/zexy/src/conf[0-9][0-9][0-9]*
	-rm -f -- $(externals_src)/zexy/src/configure
	-rm -f -- $(externals_src)/zexy/src/Make.config
	-rm -f -- $(externals_src)/zexy/src/zexyconf.h
	-rm -f -- $(externals_src)/zexy/src/*.o
	-rm -f -- $(externals_src)/zexy/src/*.bak
	-rm -f -- $(externals_src)/*/*.*~
	-rm -f -- $(externals_src)/*.*~



#------------------------------------------------------------------------------#
# pdcontainer
PDCONTAINER_NAME = pdcontainer
PDCONTAINER_INCLUDE = -I$(externals_src)/grh/PDContainer
PDCONTAINER_SRC = $(wildcard $(externals_src)/grh/PDContainer/src/[hH]*.cpp) \
                  $(wildcard $(externals_src)/grh/PDContainer/tinyxml/*.cpp)
PDCONTAINER_OBJ = $(PDCONTAINER_SRC:.cpp=.o)
PDCONTAINER_TMP = $(wildcard $(externals_src)/grh/PDContainer/src/h_*.cpp)
PDCONTAINER_TARGETS = $(PDCONTAINER_TMP:.cpp=.$(EXTENSION))
PDC_SHARED = $(wildcard $(externals_src)/grh/PDContainer/tinyxml/*.cpp)

pdcontainer: $(PDCONTAINER_TARGETS)

$(PDCONTAINER_TARGETS) : %.$(EXTENSION) : $(PDCONTAINER_OBJ)
	$(CXX) $(LDFLAGS) -o $*.$(EXTENSION) "$*.o" $(shell ls `echo "$*.o" | sed -e s/"h_[a-z]"/"\?\?"/g -e s/"map"/"Map"/g -e s/"set"/"Set"/g -e s/"queue"/"Queue"/g`) $(PDC_SHARED:.cpp=.o) $(LIBS)
	$(STRIP) $*.$(EXTENSION)
	chmod 755 $*.$(EXTENSION)
	#rm -f -- "$*.o"

$(PDCONTAINER_OBJ) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) -DPDCONTAINER_SINGLE_OBJECT $(PDCONTAINER_INCLUDE) -o "$*.o" -c "$*.cpp"

pdcontainer_install: pdcontainer
	install -d $(objectsdir)/$(PDCONTAINER_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(PDCONTAINER_NAME) \
		--author "Georg Holzmann" \
		--description "container objects of the C++ STL for Pd" \
		--license "GPL" \
		--version ""
	install -p $(PDCONTAINER_TARGETS) $(objectsdir)/$(PDCONTAINER_NAME)
	install -d $(helpdir)/$(PDCONTAINER_NAME)
	install -p $(externals_src)/grh/PDContainer/help/*.pd \
		$(helpdir)/$(PDCONTAINER_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/grh/PDContainer/readme.txt \
		$(readmesdir)/$(PDCONTAINER_NAME).txt

pdcontainer_clean:
	-rm -f -- $(PDCONTAINER_TARGETS)
	-rm -f -- $(externals_src)/grh/PDContainer/src/*.o
	-rm -f -- $(externals_src)/grh/PDContainer/src/*.bak
	-rm -f -- $(externals_src)/grh/PDContainer/src/*.*~
	-rm -f -- $(externals_src)/grh/PDContainer/tinyxml/*.o
	-rm -f -- $(externals_src)/grh/PDContainer/tinyxml/*.bak
	-rm -f -- $(externals_src)/grh/PDContainer/tinyxml/*.*~



#------------------------------------------------------------------------------#
# adaptive
ADAPTIVE_NAME=adaptive
ADAPTIVE_SRC := $(wildcard $(externals_src)/grh/adaptive/src/*.c)
ADAPTIVE_OBJECTS := $(ADAPTIVE_SRC:.c=.o)
ADAPTIVE_FLAGS := -DADAPTIVE_SINGLE_OBJ
ADAPTIVE_TMP := $(wildcard $(externals_src)/grh/adaptive/src/*lms*.c)
ADAPTIVE_TARGETS := $(ADAPTIVE_TMP:.c=.$(EXTENSION))

adaptive: $(ADAPTIVE_TARGETS)

$(ADAPTIVE_TARGETS) : %.$(EXTENSION) : %.o $(ADAPTIVE_OBJECTS)
	$(CC) $(LDFLAGS) -o $*.$(EXTENSION) "$*.o" $(externals_src)/grh/adaptive/src/adaptive.o $(LIBS)
	$(STRIP) $*.$(EXTENSION)
	chmod 755 $*.$(EXTENSION)
	rm -f -- "$*.o"

$(ADAPTIVE_OBJECTS) : %.o : %.c
	$(CC) $(CFLAGS) $(ADAPTIVE_FLAGS) -o "$*.o" -c "$*.c"

adaptive_install: adaptive
	install -d $(objectsdir)/$(ADAPTIVE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(ADAPTIVE_NAME) \
		--author "Georg Holzmann, Gerda Strobl" \
		--description "library for adaptive systems and filters" \
		--license "GNU GPL" \
		--version ""
	install -p $(ADAPTIVE_TARGETS) $(objectsdir)/$(ADAPTIVE_NAME)
	install -d $(helpdir)/$(ADAPTIVE_NAME)
	install -p $(externals_src)/grh/adaptive/doc/*.pd \
		$(helpdir)/$(ADAPTIVE_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/grh/adaptive/readme \
		$(readmesdir)/$(ADAPTIVE_NAME).txt
	install -d $(examplesdir)/$(ADAPTIVE_NAME)
	install -p $(externals_src)/grh/adaptive/examples/*.pd \
		$(externals_src)/grh/adaptive/examples/*.dat \
		$(examplesdir)/$(ADAPTIVE_NAME)

adaptive_clean:
	-rm -f -- $(ADAPTIVE_TARGETS)
	-rm -f -- $(ADAPTIVE_OBJECTS)
	-rm -f -- $(externals_src)/grh/adaptive/src/*.bak
	-rm -f -- $(externals_src)/grh/adaptive/src/*.*~



#------------------------------------------------------------------------------#
# iem_adaptfilt
IEMADAPT_NAME=iem_adaptfilt
IEMADAPT_SRC := $(wildcard $(externals_src)/iem/iem_adaptfilt/src/*~.c)

iem_adaptfilt: $(IEMADAPT_SRC:.c=.$(EXTENSION))

iem_adaptfilt_install: iem_adaptfilt
	install -d $(objectsdir)/$(IEMADAPT_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEMADAPT_NAME) \
		--author "Markus Noisternig, Thomas Musil" \
		--description "several algorithms for adaptive filters" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEMADAPT_SRC:.c=.$(EXTENSION)) $(objectsdir)/$(IEMADAPT_NAME)
	install -d $(helpdir)/$(IEMADAPT_NAME)
	install -p $(externals_src)/iem/iem_adaptfilt/help/*.pd \
		$(helpdir)/$(IEMADAPT_NAME)
	install -d $(manualsdir)/$(IEMADAPT_NAME)
	install -p $(externals_src)/iem/iem_adaptfilt/doc/adapt_filt_lib.pdf \
		$(manualsdir)/$(IEMADAPT_NAME)

iem_adaptfilt_clean:
	-rm -f -- $(IEMADAPT_SRC:.c=.$(EXTENSION))
	-rm -f -- $(IEMADAPT_SRC:.c=.o)
	-rm -f -- $(externals_src)/iem/iem_adaptfilt/src/*.bak
	-rm -f -- $(externals_src)/iem/iem_adaptfilt/src/*.*~



#------------------------------------------------------------------------------#
# iem_delay
IEMDELAY_NAME=iem_delay
IEMDELAY_SRC := $(wildcard $(externals_src)/iem/iem_delay/src/*~.c)

iem_delay: $(IEMDELAY_SRC:.c=.$(EXTENSION))

iem_delay_install: iem_delay
	install -d $(objectsdir)/$(IEMDELAY_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEMDELAY_NAME) \
		--author "Thomas Musil" \
		--description "various delay objects" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEMDELAY_SRC:.c=.$(EXTENSION)) $(objectsdir)/$(IEMDELAY_NAME)
	install -d $(helpdir)/$(IEMDELAY_NAME)
	install -p $(externals_src)/iem/iem_delay/help/*.pd \
		$(helpdir)/$(IEMDELAY_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/iem/iem_delay/READ_ME.txt \
		$(readmesdir)/$(IEMDELAY_NAME).txt

iem_delay_clean:
	-rm -f -- $(IEMDELAY_SRC:.c=.$(EXTENSION))
	-rm -f -- $(IEMDELAY_SRC:.c=.o)
	-rm -f -- $(externals_src)/iem/iem_delay/src/*.bak
	-rm -f -- $(externals_src)/iem/iem_delay/src/*.*~



#------------------------------------------------------------------------------#
# iem_roomsim
IEMROOM_NAME=iem_roomsim
IEMROOM_SRC := $(wildcard $(externals_src)/iem/iem_roomsim/src/*d.c)

iem_roomsim: $(IEMROOM_SRC:.c=.$(EXTENSION))

iem_roomsim_install: iem_roomsim
	install -d $(objectsdir)/$(IEMROOM_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEMROOM_NAME) \
		--author "Thomas Musil" \
		--description "objects for room simulation" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEMROOM_SRC:.c=.$(EXTENSION)) $(objectsdir)/$(IEMROOM_NAME)
	install -d $(helpdir)/$(IEMROOM_NAME)
	install -p $(externals_src)/iem/iem_roomsim/help/*.pd \
		$(helpdir)/$(IEMROOM_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/iem/iem_roomsim/READ_ME.txt \
		$(readmesdir)/$(IEMROOM_NAME).txt

iem_roomsim_clean:
	-rm -f -- $(IEMROOM_SRC:.c=.$(EXTENSION))
	-rm -f -- $(IEMROOM_SRC:.c=.o)
	-rm -f -- $(externals_src)/iem/iem_roomsim/src/*.bak
	-rm -f -- $(externals_src)/iem/iem_roomsim/src/*.*~



#------------------------------------------------------------------------------#
# iem_spec2
IEMSPEC2_NAME=iem_spec2
IEMSPEC2_SRC := $(wildcard $(externals_src)/iem/iem_spec2/src/*~.c)

iem_spec2: $(IEMSPEC2_SRC:.c=.$(EXTENSION))

iem_spec2_install: iem_spec2
	install -d $(objectsdir)/$(IEMSPEC2_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEMSPEC2_NAME) \
		--author "Thomas Musil" \
		--description "special spectral processing objects, which only calculates blocksize/2 + 1 samples of a signal" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEMSPEC2_SRC:.c=.$(EXTENSION)) $(objectsdir)/$(IEMSPEC2_NAME)
	install -d $(helpdir)/$(IEMSPEC2_NAME)
	install -p $(externals_src)/iem/iem_spec2/help/*.pd \
		$(helpdir)/$(IEMSPEC2_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/iem/iem_spec2/READ_ME.txt \
		$(readmesdir)/$(IEMSPEC2_NAME).txt

iem_spec2_clean:
	-rm -f -- $(IEMSPEC2_SRC:.c=.$(EXTENSION))
	-rm -f -- $(IEMSPEC2_SRC:.c=.o)
	-rm -f -- $(externals_src)/iem/iem_spec2/src/*.bak
	-rm -f -- $(externals_src)/iem/iem_spec2/src/*.*~



#------------------------------------------------------------------------------#
# iem_tab
IEMTAB_NAME=iem_tab
IEMTAB_SRC := $(wildcard $(externals_src)/iem/iem_tab/src/*.c)
IEMTAB_OBJ := $(IEMTAB_SRC:.c=.o)
IEMTAB_TARTMP := $(wildcard $(externals_src)/iem/iem_tab/src/tab_*.c)
IEMTAB_TARGETS := $(IEMTAB_TARTMP:.c=.$(EXTENSION))
IEMTAB_FLAGS := -DIEMTAB_SINGLE_OBJ

iem_tab: $(IEMTAB_TARGETS)

$(IEMTAB_TARGETS) : %.$(EXTENSION) : %.o $(IEMTAB_OBJ)
	$(CC) $(LDFLAGS) -o $*.$(EXTENSION) "$*.o" $(externals_src)/iem/iem_tab/src/iem_tab.o $(LIBS)
	$(STRIP) $*.$(EXTENSION)
	chmod 755 $*.$(EXTENSION)
	rm -f -- "$*.o"

$(IEMTAB_OBJ) : %.o : %.c
	$(CC) $(CFLAGS) $(IEMTAB_FLAGS) -o "$*.o" -c "$*.c"

iem_tab_install: iem_tab
	install -d $(objectsdir)/$(IEMTAB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEMTAB_NAME) \
		--author "Thomas Musil" \
		--description "library to manipulate tables or arrays" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEMTAB_TARGETS) $(objectsdir)/$(IEMTAB_NAME)
	install -d $(helpdir)/$(IEMTAB_NAME)
	install -p $(externals_src)/iem/iem_tab/help/*.pd \
		$(helpdir)/$(IEMTAB_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/iem/iem_tab/READ_ME.txt \
		$(readmesdir)/$(IEMTAB_NAME).txt

iem_tab_clean:
	-rm -f -- $(IEMTAB_TARGETS)
	-rm -f -- $(IEMTAB_OBJ)
	-rm -f -- $(externals_src)/iem/iem_tab/src/*.bak
	-rm -f -- $(externals_src)/iem/iem_tab/src/*.*~



#------------------------------------------------------------------------------#
# flashserver
FLASHSERVER_NAME=flashserver
FLASHSERVER_OBJECTS := $(wildcard $(externals_src)/olafmatt/flashserver/*.c)
flashserver: $(FLASHSERVER_OBJECTS:.c=.$(EXTENSION))

flashserver_install: flashserver
	install -d $(objectsdir)/$(FLASHSERVER_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(FLASHSERVER_NAME) \
		--author "Olaf Matthes" \
		--description "a flashserver for multiple clients" \
		--license "GNU GPL" \
		--version ""
	install -p $(FLASHSERVER_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(FLASHSERVER_NAME)
	install -d $(helpdir)/$(FLASHSERVER_NAME)
	install -p $(externals_src)/olafmatt/flashserver/*.pd \
		$(helpdir)/$(FLASHSERVER_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/olafmatt/flashserver/README \
		$(readmesdir)/$(FLASHSERVER_NAME).txt
	install -d $(examplesdir)/$(FLASHSERVER_NAME)

flashserver_clean:
	-rm -f -- $(FLASHSERVER_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(FLASHSERVER_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/olafmatt/flashserver/*.bak
	-rm -f -- $(externals_src)/olafmatt/flashserver/*.*~



#------------------------------------------------------------------------------#
# hdspm_mixer
HDSPM_NAME=hdspm_mixer
HDSPM_SRC := $(wildcard $(externals_src)/iem/hdspm_mixer/*.c)
HDSPM_OBJ := $(HDSPM_SRC:.c=.o)
HDSPM_TARGET := $(externals_src)/iem/hdspm_mixer/hdspmmixer.$(EXTENSION)

hdspm_mixer: $(HDSPM_TARGET)

$(HDSPM_TARGET) : $(HDSPM_OBJ)
	$(CC) $(LDFLAGS) -o $(HDSPM_TARGET) $(HDSPM_OBJ) $(LIBS)
	$(STRIP) $(HDSPM_TARGET)
	chmod 755 $(HDSPM_TARGET)
	rm -f -- $(HDSPM_OBJ)

$(HDSPM_OBJ) : %.o : %.c
	$(CC) $(CFLAGS) -o "$*.o" -c "$*.c"

hdspm_mixer_install: hdspm_mixer
	install -d $(objectsdir)/$(HDSPM_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(HDSPM_NAME) \
		--author "Winfried Ritsch" \
		--description "pd-mixer for RME hammerfall devices" \
		--license "GNU GPL" \
		--version ""
	install -p $(HDSPM_TARGET) $(objectsdir)/$(HDSPM_NAME)
	install -d $(helpdir)/$(HDSPM_NAME)
	install -p $(externals_src)/iem/hdspm_mixer/*.pd \
		$(helpdir)/$(HDSPM_NAME)

hdspm_mixer_clean:
	-rm -f -- $(HDSPM_TARGET)
	-rm -f -- $(HDSPM_OBJ)
	-rm -f -- $(externals_src)/iem/hdspm_mixer/*.bak
	-rm -f -- $(externals_src)/iem/hdspm_mixer/*.*~



#------------------------------------------------------------------------------#
# iemgui
IEMGUI_NAME=iemgui
IEMGUI_SRC := $(wildcard $(externals_src)/iem/iemgui/src/*.c)
IEMGUI_OBJ := $(IEMGUI_SRC:.c=.o)
IEMGUI_TARTMP := $(wildcard $(externals_src)/iem/iemgui/src/*_*.c)
IEMGUI_TARGETS := $(IEMGUI_TARTMP:.c=.$(EXTENSION))
IEMGUI_FLAGS := -DIEMGUI_SINGLE_OBJ

iemgui: $(IEMGUI_TARGETS)

$(IEMGUI_TARGETS) : %.$(EXTENSION) : %.o $(IEMGUI_OBJ)
	$(CC) $(LDFLAGS) -o $*.$(EXTENSION) "$*.o" $(externals_src)/iem/iemgui/src/iemgui.o $(LIBS)
	$(STRIP) $*.$(EXTENSION)
	chmod 755 $*.$(EXTENSION)
	rm -f -- "$*.o"

$(IEMGUI_OBJ) : %.o : %.c
	$(CC) $(CFLAGS) $(IEMGUI_FLAGS) -o "$*.o" -c "$*.c"

iemgui_install: iemgui
	install -d $(objectsdir)/$(IEMGUI_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEMGUI_NAME) \
		--author "Thomas Musil" \
		--description "some more GUI objects for PD" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEMGUI_TARGETS) $(objectsdir)/$(IEMGUI_NAME)
	install -d $(helpdir)/$(IEMGUI_NAME)
	install -p $(externals_src)/iem/iemgui/help/*.pd \
		$(helpdir)/$(IEMGUI_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/iem/iemgui/READ_ME.txt \
		$(readmesdir)/$(IEMGUI_NAME).txt

iemgui_clean:
	-rm -f -- $(IEMGUI_TARGETS)
	-rm -f -- $(IEMGUI_OBJ)
	-rm -f -- $(externals_src)/iem/iemgui/src/*.bak
	-rm -f -- $(externals_src)/iem/iemgui/src/*.*~



#------------------------------------------------------------------------------#
# iemxmlrpc
IEMXMLRPC_NAME=iemxmlrpc
IEMXMLRPC_SRC := $(externals_src)/iem/iemxmlrpc/main.cpp
IEMXMLRPC_TARGET := $(externals_src)/iem/iemxmlrpc/xmlrpc.$(EXTENSION)
IEMXMLRPC_FLAGS := -I$(externals_src)/iem/iemxmlrpc/xmlrpc++/src
XMLRPCPP_SRC := $(wildcard $(externals_src)/iem/iemxmlrpc/xmlrpc++/src/*.cpp)

iemxmlrpc: $(IEMXMLRPC_TARGET)

$(IEMXMLRPC_TARGET) : $(IEMXMLRPC_SRC:.cpp=.o) $(XMLRPCPP_SRC:.cpp=.o)
	$(CXX) $(LDFLAGS) -o $(IEMXMLRPC_TARGET) $(IEMXMLRPC_SRC:.cpp=.o) $(XMLRPCPP_SRC:.cpp=.o) $(LIBS)
	$(STRIP) $(IEMXMLRPC_TARGET)
	chmod 755 $(IEMXMLRPC_TARGET)
	#rm -f -- "$*.o"

$(IEMXMLRPC_SRC:.cpp=.o) $(XMLRPCPP_SRC:.cpp=.o) : %.o : %.cpp
	$(CXX) $(CXXFLAGS) $(IEMXMLRPC_FLAGS) -o "$*.o" -c "$*.cpp"

iemxmlrpc_install: iemxmlrpc
	install -d $(objectsdir)/$(IEMXMLRPC_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(IEMXMLRPC_NAME) \
		--author "Thomas Grill, Winfried Ritsch" \
		--description "XMLRPC external for PD" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEMXMLRPC_TARGET) $(objectsdir)/$(IEMXMLRPC_NAME)
	install -d $(helpdir)/$(IEMXMLRPC_NAME)
	install -p $(externals_src)/iem/iemxmlrpc/*.pd \
		$(helpdir)/$(IEMXMLRPC_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/iem/iemxmlrpc/README.txt \
		$(readmesdir)/$(IEMXMLRPC_NAME).txt
	install -d $(examplesdir)/$(IEMXMLRPC_NAME)
	install -p $(externals_src)/iem/iemxmlrpc/*.pd \
                   $(externals_src)/iem/iemxmlrpc/*.py \
		   $(examplesdir)/$(IEMXMLRPC_NAME)

iemxmlrpc_clean:
	-rm -f -- $(IEMXMLRPC_TARGET)
	-rm -f -- $(externals_src)/iem/iemxmlrpc/xmlrpc++/src/*.o
	-rm -f -- $(externals_src)/iem/iemxmlrpc/*.o
	-rm -f -- $(externals_src)/iem/iemxmlrpc/*.bak
	-rm -f -- $(externals_src)/iem/iemxmlrpc/*.*~



#------------------------------------------------------------------------------#
# EARPLUG
EARPLUG_NAME=earplug~
EARPLUG_OBJECTS := $(wildcard $(externals_src)/earplug~/*.c)
earplug: $(EARPLUG_OBJECTS:.c=.$(EXTENSION))

earplug_install: earplug
	install -d $(objectsdir)/$(EARPLUG_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(EARPLUG_NAME) \
		--author "Pei Xiang" \
		--description "filters for binaural audio spatialization, based on KEMAR impulse measurement" \
		--license "" \
		--version ""
	install -p $(EARPLUG_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(EARPLUG_NAME)
	install -p $(externals_src)/earplug~/earplug_data.txt $(objectsdir)/$(EARPLUG_NAME)
	install -d $(helpdir)/$(EARPLUG_NAME)
	install -p $(externals_src)/earplug~/*-help.pd \
		$(helpdir)/$(EARPLUG_NAME)
	install -d $(examplesdir)/$(EARPLUG_NAME)
	install -p $(externals_src)/earplug~/*-help.pd \
		   $(examplesdir)/$(EARPLUG_NAME)

earplug_clean:
	-rm -f -- $(EARPLUG_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(EARPLUG_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/earplug~/*.bak
	-rm -f -- $(externals_src)/earplug~/*.*~



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
clean:  $(patsubst %, %_clean,$(LIB_TARGETS))


distclean: clean cruft_clean


test_locations:
	@echo "PD_VERSION: $(PD_VERSION)"
	@echo "PACKAGE_VERSION: $(PACKAGE_VERSION)"
	@echo "CWD $(CWD)"
	@echo "DESTDIR $(DESTDIR)"
	@echo "PREFIX $(prefix)"
	@echo "BINDIR  $(bindir)"
	@echo "LIBDIR  $(libdir)"
	@echo "OBJECTSDIR  $(objectsdir)"
	@echo "PDDOCDIR  $(pddocdir)"
	@echo "LIBPDDIR  $(libpddir)"
	@echo "LIBPDBINDIR  $(libpdbindir)"
	@echo "HELPDIR  $(helpdir)"
	@echo "MANUALSDIR  $(manualsdir)"
	@echo "EXAMPLESDIR  $(examplesdir)"
	@echo " "
	@echo "Compiling these libs:"
	@echo "$(LIB_TARGETS)"


.PHONY: all install clean distclean test_locations $(LIB_TARGETS) \
$(patsubst %, %_install,$(LIB_TARGETS)) $(patsubst %, %_clean,$(LIB_TARGETS))


