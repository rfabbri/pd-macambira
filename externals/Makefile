
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

.SUFFIXES: .$(EXTENSION) .$(SHARED_LIB)


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

CFLAGS = -DPD -I$(pd_src)/src -Wall -W $(DEBUG_CFLAGS) -I$(gem_src)/src
LDFLAGS =  
LIBS = -lm

ifeq ($(OS_NAME),darwin)
# 10.4 Tiger
  FAT_FLAGS = -arch ppc -arch ppc64 -arch i386
# 10.5 Leopard
#  FAT_FLAGS = -arch ppc -arch ppc7400 -arch ppc64 -arch i386 -arch x86_64
  CFLAGS += -I/sw/include -I$(externals_src)/pdp/include -DMACOSX -DUNIX -Dunix -DDL_OPEN
  LDFLAGS += -bundle -bundle_loader $(pd_src)/bin/pd -undefined dynamic_lookup \
		-L/sw/lib -weak_framework Carbon
  LIBS += -lc 
  DYLIB_LDFLAGS = -dynamiclib -undefined dynamic_lookup -read_only_relocs warning -L/sw/lib
  STRIP = strip -x
endif
ifeq ($(OS_NAME),linux)
  CFLAGS += -I$(externals_src)/pdp/include -DUNIX -Dunix -DDL_OPEN -fPIC
  LDFLAGS += -Wl,--export-dynamic  -shared -fPIC
  LIBS += -lc
  DYLIB_LDFLAGS = $(LDFLAGS)
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
  LDFLAGS += -s -shared
# all of these included libs are part of libc in UNIX platforms.  All except
# libregex are in DLLs, so they get stripped from the external's .dll binary
  LIBS += -L$(pd_src)/src -L$(pd_src)/bin -L$(pd_src)/obj -lpd \
    -lwsock32 -lpthreadGC2 -lkernel32 -luser32 -lgdi32 -lregex
  DYLIB_LDFLAGS = -shared
  STRIP = strip --strip-unneeded -R .note -R .comment
endif

CXXFLAGS = $(CFLAGS)

### C++ files
%.$(EXTENSION): %.cpp
%.$(EXTENSION): %.cc
	$(CXX) $(OPT_CFLAGS) $(CXXFLAGS) -o "$*.o" -c "$<"
	$(CXX) $(LDFLAGS) -o "$*.$(EXTENSION)" "$*.o" $(LIBS)
	chmod a-x "$*.$(EXTENSION)"
#	$(STRIP) $*.$(EXTENSION)
#	rm -f -- $*.o

%.o: %.cpp
%.o: %.cc
	$(CXX) $(OPT_CFLAGS) $(CXXFLAGS) -o "$*.o" -c "$<"


### C files
%.o: %.c
	$(CC) $(OPT_CFLAGS) $(CFLAGS) -o "$*.o" -c "$*.c"

%.$(EXTENSION): %.o
	$(CC) $(LDFLAGS) -o "$*.$(EXTENSION)" "$*.o"  $(LIBS) \
		`test -f $*.libs && cat $*.libs`	\
		`my_dylib=$(patsubst $(externals_src)/%,%,$(@D)); test -f $(@D)/lib$${my_dylib}.$(DYLIB_EXTENSION) && echo -L$(@D) -l$$my_dylib` \
		`my_obj=$(patsubst $(externals_src)/%,%,$(@D)); test -f $(@D)/shared/$${my_obj}.o && echo $(@D)/shared/$${my_obj}.o` \
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
LIB_TARGETS = adaptive bassemu boids bsaylor creb cxc cyclone deprecated earplug ekext ext13 flashserver flatspace flib freeverb ggee hardware hcs iem_ambi iem_bin_ambi iemlib iemgui iemguts iem_adaptfilt iemmatrix iem_matrix iemxmlrpc iem_delay iem_roomsim iem_spec2 iem_tab jasch_lib loaders mapping markex maxlib mjlib moocow moonlib motex mrpeach msd oscx pan pdcontainer pddp pdogg pmpd sigpack smlib tof toxy unauthorized vanilla vbap windowing zexy

# this is for libraries that don't compile (yet) on all platforms
ifeq ($(OS_NAME),windows)
  LIB_TARGETS += 
else
  ifeq ($(OS_NAME),darwin)
    LIB_TARGETS += hid pdp pidip gem2pdp iem16 pdp_opengl apple
  else
    # GNU/Linux, BSD, IRIX, etc.
    LIB_TARGETS += hid pdp pidip gem2pdp iem16 pdp_opengl postlude sixaxis
  endif
endif

#------------------------------------------------------------------------------#

all: $(LIB_TARGETS)
	@echo " "
	@echo "Compiled externals for $(OS_NAME) aka $(UNAME)"


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
	install -d $(DESTDIR)$(objectsdir)/$(FLATSPACE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(FLATSPACE_NAME) \
		--author "Numerous" \
		--description "This is a collection of externals in a flat namespace" \
		--license "GNU GPL"
	install -p $(FLATSPACE_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(FLATSPACE_NAME)
# all standard objs' help files, it had to be broken up because the list is
# soo long.  They are installed only inside of the libdir since this is so
# messy.  We don't these help patches to come up for other objects
	install -p \
			$(externals_src)/arraysize/*.pd \
			$(externals_src)/beatpipe/*.pd \
			$(externals_src)/bsaylor/help/*.pd \
			$(externals_src)/control/*/*.pd \
			$(externals_src)/bbogart/chaos/tools/*.pd \
			$(externals_src)/bbogart/*/*.pd \
				$(DESTDIR)$(objectsdir)/$(FLATSPACE_NAME)
	install -p \
			$(externals_src)/ff/*.pd \
			$(externals_src)/hcs/folder_list-help.pd \
			$(externals_src)/hcs/split_path-help.pd \
				$(DESTDIR)$(objectsdir)/$(FLATSPACE_NAME)
	install -p \
			$(externals_src)/iem/comport/*/*-help.pd \
			$(externals_src)/nusmuk/line3/*.pd \
			$(externals_src)/plugin~/*.pd \
			$(externals_src)/rhythm_estimator/*.p? \
				$(DESTDIR)$(objectsdir)/$(FLATSPACE_NAME)


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
	install -d $(DESTDIR)$(objectsdir)/$(TEMPLATE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(TEMPLATE_NAME) \
		--author "" \
		--description "" \
		--license "" \
		--version ""
	install -p $(TEMPLATE_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(TEMPLATE_NAME)
	install -d $(DESTDIR)$(helpdir)/$(TEMPLATE_NAME)
	install -p $(externals_src)/template/help/*.pd \
		$(DESTDIR)$(helpdir)/$(TEMPLATE_NAME)
#	install -d $(manualsdir)/$(TEMPLATE_NAME)
#	install -p $(externals_src)/template/manual.txt \
#		$(manualsdir)/$(TEMPLATE_NAME)
	install -d $(readmesdir)
	install -p $(externals_src)/template/README \
		$(readmesdir)/$(TEMPLATE_NAME).txt
	install -d $(DESTDIR)$(examplesdir)/$(TEMPLATE_NAME)
	install -p $(externals_src)/template/examples/*.pd \
		$(DESTDIR)$(examplesdir)/$(TEMPLATE_NAME)

template_clean:
	-rm -f -- $(TEMPLATE_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(TEMPLATE_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/template/*.bak
	-rm -f -- $(externals_src)/template/*.*~




#------------------------------------------------------------------------------#
# AKA.WIIREMOTE
# this is installed into the "io" library
AKA.WIIREMOTE_NAME=io
AKA.WIIREMOTE_SRC := $(wildcard $(externals_src)/io/aka.wiiremote/*.c)

AKA.WIIREMOTE_OBJECTS := $(AKA.WIIREMOTE_SRC:.c=.o)
$(AKA.WIIREMOTE_OBJECTS) : %.o : %.c
	$(CC) $(OPT_CFLAGS) $(CFLAGS) -o "$*.o" -c "$*.c"

$(externals_src)/io/aka.wiiremote/akawiiremote.$(EXTENSION): $(AKA.WIIREMOTE_OBJECTS) 
	$(CC) $(LDFLAGS) -o $(externals_src)/io/aka.wiiremote/akawiiremote.$(EXTENSION) \
		$(AKA.WIIREMOTE_OBJECTS) -weak_framework IOBluetooth \
		-weak_framework CoreFoundation
#	$(STRIP) $(externals_src)/io/aka.wiiremote/aka.wiiremote.$(EXTENSION)

aka.wiiremote: $(externals_src)/io/aka.wiiremote/akawiiremote.$(EXTENSION)

aka.wiiremote_install: aka.wiiremote
	install -d $(DESTDIR)$(objectsdir)/$(AKA.WIIREMOTE_NAME)
	install -p $(externals_src)/io/aka.wiiremote/aka.wiiremote.$(EXTENSION) \
		$(DESTDIR)$(objectsdir)/$(AKA.WIIREMOTE_NAME)
	install -d $(DESTDIR)$(helpdir)/$(AKA.WIIREMOTE_NAME)
	install -p $(externals_src)/io/aka.wiiremote/*.pd \
		$(DESTDIR)$(helpdir)/$(AKA.WIIREMOTE_NAME)
	install -d $(DESTDIR)$(manualsdir)/$(AKA.WIIREMOTE_NAME)
	install -p $(externals_src)/io/aka.wiiremote/*.txt \
		$(DESTDIR)$(manualsdir)/$(AKA.WIIREMOTE_NAME)

aka.wiiremote_clean:
	-rm -f -- $(externals_src)/io/aka.wiiremote/aka.wiiremote.$(EXTENSION)
	-rm -f -- $(AKA.WIIREMOTE_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/io/aka.wiiremote/*.bak
	-rm -f -- $(externals_src)/io/aka.wiiremote/*.*~



#------------------------------------------------------------------------------#
# APPLE
apple:
	make -C $(externals_src)/apple

apple_install:
	make -C $(externals_src)/apple objectsdir="$(objectsdir)" install

apple_clean:
	make -C $(externals_src)/apple clean


#------------------------------------------------------------------------------#
# BASSEMU
BASSEMU_NAME=bassemu
BASSEMU_OBJECTS := $(wildcard $(externals_src)/bassemu/*.c)
bassemu: $(BASSEMU_OBJECTS:.c=.$(EXTENSION))

bassemu_install: bassemu
	install -d $(DESTDIR)$(objectsdir)/$(BASSEMU_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(BASSEMU_NAME) \
		--author "" \
		--description "a synthesizer" \
		--license "" \
		--version ""
	install -p $(BASSEMU_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(BASSEMU_NAME)
	install -d $(DESTDIR)$(helpdir)/$(BASSEMU_NAME)
	install -p $(externals_src)/bassemu/*.pd \
		$(DESTDIR)$(helpdir)/$(BASSEMU_NAME)
	install -p $(externals_src)/bassemu/*.pd \
		$(DESTDIR)$(objectsdir)/$(BASSEMU_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(BASSEMU_NAME)
#	install -p $(externals_src)/bassemu/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(BASSEMU_NAME)
#	install -d $(DESTDIR)$(readmesdir)
#	install -p $(externals_src)/bassemu/README \
		$(DESTDIR)$(readmesdir)/$(BASSEMU_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(BASSEMU_NAME)
#	install -p $(externals_src)/bassemu/examples/*.pd \
		$(DESTDIR)$(examplesdir)/$(BASSEMU_NAME)

bassemu_clean:
	-rm -f -- $(BASSEMU_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(BASSEMU_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/bassemu/*.bak
	-rm -f -- $(externals_src)/bassemu/*.*~



#------------------------------------------------------------------------------#
# BOIDS
BOIDS_NAME=boids
BOIDS_OBJECTS := $(wildcard $(externals_src)/boids/boids*/*.c)
boids: $(BOIDS_OBJECTS:.c=.$(EXTENSION))

boids_install: boids
	install -d $(DESTDIR)$(objectsdir)/$(BOIDS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(BOIDS_NAME) \
		--author "Eric Singer, A. Sier, and Jasch" \
		--description "2D and 3D boids flocking algorithm" \
		--license "GNU GPL 2" \
		--version ""
	install -p $(BOIDS_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(BOIDS_NAME)
	install -d $(DESTDIR)$(helpdir)/$(BOIDS_NAME)
	install -p $(externals_src)/boids/boids*/*-help.pd \
		$(DESTDIR)$(helpdir)/$(BOIDS_NAME)
	install -p $(externals_src)/boids/boids*/*.pd \
		$(DESTDIR)$(objectsdir)/$(BOIDS_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(BOIDS_NAME)
#	install -p $(externals_src)/boids/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(BOIDS_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/boids/boids.readme.txt \
		$(DESTDIR)$(readmesdir)/$(BOIDS_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(BOIDS_NAME)
#	install -p $(externals_src)/boids/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(BOIDS_NAME)

boids_clean:
	-rm -f -- $(BOIDS_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(BOIDS_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/boids/*/*.bak
	-rm -f -- $(externals_src)/boids/*/*.*~


#------------------------------------------------------------------------------#
# BSAYLOR
BSAYLOR_NAME=bsaylor
BSAYLOR_SRC := $(wildcard $(externals_src)/bsaylor/*~.c)
BSAYLOR_OBJECTS := $(BSAYLOR_SRC:.c=.o)
$(BSAYLOR_OBJECTS) : %.o : %.c
	$(CC) $(CFLAGS) -O2 -funroll-loops -fomit-frame-pointer -o "$*.o" -c "$*.c"

bsaylor: $(BSAYLOR_OBJECTS:.o=.$(EXTENSION))

bsaylor_install: bsaylor
	install -d $(DESTDIR)$(objectsdir)/$(BSAYLOR_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(BSAYLOR_NAME) \
		--author "Benjamin R. Saylor <bensaylor@fastmail.fm>" \
		--description "signal objects" \
		--license "GNU GPL 2" \
		--version ""
	install -p $(BSAYLOR_SRC:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(BSAYLOR_NAME)
	install -d $(DESTDIR)$(helpdir)/$(BSAYLOR_NAME)
	install -p $(externals_src)/bsaylor/help/*.pd \
		$(DESTDIR)$(helpdir)/$(BSAYLOR_NAME)
	install -p $(externals_src)/bsaylor/help/*.pd \
		$(DESTDIR)$(objectsdir)/$(BSAYLOR_NAME)
	install -p $(externals_src)/bsaylor/*.wav $(DESTDIR)$(helpdir)/$(BSAYLOR_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(BSAYLOR_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/bsaylor/README.txt \
		$(DESTDIR)$(readmesdir)/$(BSAYLOR_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(BSAYLOR_NAME)
#	install -p $(externals_src)/bsaylor/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(BSAYLOR_NAME)

bsaylor_clean:
	-rm -f -- $(externals_src)/bsaylor/*.$(EXTENSION)
	-rm -f -- $(externals_src)/bsaylor/*.o
	-rm -f -- $(externals_src)/bsaylor/*.bak
	-rm -f -- $(externals_src)/bsaylor/*.*~


#------------------------------------------------------------------------------#
# CREB
CREB_NAME=creb
CREB_OBJECTS := $(wildcard $(externals_src)/creb/modules/*.c)
CREB_CXXOBJECTS := $(wildcard $(externals_src)/creb/modules++/*.cc)

creb: $(CREB_OBJECTS:.c=.$(EXTENSION)) $(CREB_CXXOBJECTS:.cc=.$(EXTENSION))

creb_install: creb
	install -d $(DESTDIR)$(objectsdir)/$(CREB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(CREB_NAME) \
		--author "Tom Schouten <tom@zwizwa.be>" \
		--description "This is a collection of pd externals. My bag of tricks." \
		--license "GNU GPL 2" \
		--version "0.9.2"
	install -p $(externals_src)/creb/abs/*.pd $(DESTDIR)$(objectsdir)/$(CREB_NAME)
	install -p $(CREB_OBJECTS:.c=.$(EXTENSION)) \
		$(CREB_CXXOBJECTS:.cc=.$(EXTENSION)) 		$(DESTDIR)$(objectsdir)/$(CREB_NAME)
	install -d $(DESTDIR)$(helpdir)/$(CREB_NAME)
	install -p $(externals_src)/creb/doc/*.* $(DESTDIR)$(helpdir)/$(CREB_NAME)
	install -p $(externals_src)/creb/doc/*.* $(DESTDIR)$(objectsdir)/$(CREB_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/creb/doc/reference.txt \
		$(DESTDIR)$(readmesdir)/$(CREB_NAME).txt
	install -d $(DESTDIR)$(examplesdir)/$(CREB_NAME)
	install -p $(externals_src)/creb/doc/examples/*.* $(DESTDIR)$(examplesdir)/$(CREB_NAME)

creb_clean:
	-rm -f -- $(CREB_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(CREB_OBJECTS:.c=.o)
	-rm -f -- $(CREB_CXXOBJECTS:.cc=.$(EXTENSION))
	-rm -f -- $(CREB_CXXOBJECTS:.cc=.o)
	-rm -f -- $(externals_src)/creb/*/*.bak
	-rm -f -- $(externals_src)/creb/*/*.*~


#------------------------------------------------------------------------------#
# CXC
CXC_NAME=cxc
CXC_OBJECTS := $(wildcard $(externals_src)/cxc/???*.c)
cxc: $(CXC_OBJECTS:.c=.$(EXTENSION))

cxc_install: cxc
	install -d $(DESTDIR)$(objectsdir)/$(CXC_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(CXC_NAME) \
		--author "jdl@xdv.org" \
		--description "" \
		--license "" \
		--version ""
	install -p $(CXC_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(CXC_NAME)
	install -d $(DESTDIR)$(helpdir)/$(CXC_NAME)
	install -p $(wildcard $(externals_src)/cxc/reference/*.pd) \
		$(DESTDIR)$(helpdir)/$(CXC_NAME)
	install -p $(wildcard $(externals_src)/cxc/reference/*.pd) \
		$(DESTDIR)$(objectsdir)/$(CXC_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(CXC_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/cxc/README \
		 $(DESTDIR)$(readmesdir)/$(CXC_NAME).txt

cxc_clean:
	-rm -f -- $(CXC_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(externals_src)/cxc/*.o
	-rm -f -- $(externals_src)/cxc/*.bak
	-rm -f -- $(externals_src)/cxc/*.*~


#------------------------------------------------------------------------------#
# CYCLONE
CYCLONE_NAME=cyclone
# cyclone is compiled straight into $(OUT_DIR)
CYCLONE_OUT_DIR=$(DESTDIR)$(objectsdir)/$(CYCLONE_NAME)
cyclone: $(bindir)
	$(MAKE) OPT_CFLAGS="-O6 -funroll-loops -fomit-frame-pointer -fno-strict-aliasing -fPIC" \
		-C $(externals_src)/miXed/cyclone

cyclone_install: cyclone
	install -d $(DESTDIR)$(objectsdir)/$(CYCLONE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(CYCLONE_NAME) \
		--author "Kzrysztof Czaja" \
		--license "BSD" \
		--description "a library for porting and running Max/MSP patches in Pd"
# kludge to build as one-file-per-class using the miXed build system
	cd $(externals_src)/miXed/cyclone && $(MAKE) OUT_DIR=$(CYCLONE_OUT_DIR)
# install "maxmode" libraries into subdir, so they don't override the libdir
	install -d $(DESTDIR)$(objectsdir)/$(CYCLONE_NAME)/maxmode
	install -p $(externals_src)/miXed/bin/cyclone.$(EXTENSION) \
		$(externals_src)/miXed/bin/hammer.$(EXTENSION) \
		$(externals_src)/miXed/bin/sickle.$(EXTENSION) \
		$(externals_src)/miXed/bin/maxmode.$(EXTENSION) \
		$(DESTDIR)$(objectsdir)/$(CYCLONE_NAME)/maxmode
# install "cyclist" command line app with pd
	-install -p -m0755 $(externals_src)/miXed/bin/cyclist $(DESTDIR)$(bindir)
	install -d $(DESTDIR)$(helpdir)/$(CYCLONE_NAME)
	install -p $(externals_src)/miXed/doc/help/cyclone/*.* \
		$(DESTDIR)$(helpdir)/$(CYCLONE_NAME)
	install -p $(externals_src)/miXed/doc/help/cyclone/*.* \
		$(DESTDIR)$(objectsdir)/$(CYCLONE_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(CYCLONE_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(CYCLONE_NAME)
	install -p $(externals_src)/miXed/test/cyclone/*.* \
		$(DESTDIR)$(examplesdir)/$(CYCLONE_NAME)


cyclone_clean:
	-$(MAKE) -C $(externals_src)/miXed/cyclone $(DEST_PATHS) clean




#------------------------------------------------------------------------------#
# DEPRECATED
DEPRECATED_NAME=deprecated
DEPRECATED_OBJECTS := $(wildcard $(externals_src)/deprecated/*.c)
deprecated: $(DEPRECATED_OBJECTS:.c=.$(EXTENSION))

deprecated_install: deprecated
	install -d $(DESTDIR)$(objectsdir)/$(DEPRECATED_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(DEPRECATED_NAME) \
		--author "numerous" \
		--description "a collection of deprecated objects that still have some use" \
		--license "" \
		--version ""
	install -p $(DEPRECATED_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(DEPRECATED_NAME)
	install -d $(DESTDIR)$(helpdir)/$(DEPRECATED_NAME)
	install -p $(externals_src)/deprecated/help/*.pd \
		$(DESTDIR)$(helpdir)/$(DEPRECATED_NAME)
	install -d $(DESTDIR)$(manualsdir)/$(DEPRECATED_NAME)
	install -p $(externals_src)/deprecated/doc/*.* \
		$(DESTDIR)$(manualsdir)/$(DEPRECATED_NAME)
#	install -d $(DESTDIR)$(readmesdir)
#	install -p $(externals_src)/deprecated/README \
#		$(DESTDIR)$(readmesdir)/$(DEPRECATED_NAME).txt
	install -d $(DESTDIR)$(examplesdir)/$(DEPRECATED_NAME)
	install -p $(externals_src)/deprecated/examples/*.pd \
		$(DESTDIR)$(examplesdir)/$(DEPRECATED_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(DEVELLIBS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(DEVELLIBS_NAME) \
		--author "Miller Puckette and others" \
		--description "core libraries stripped out of Pd-devel" \
		--license "BSD" \
		--version ""
	install -p $(DEVELLIBS_OBJECTS:.c=.$(EXTENSION)) 
		$(DESTDIR)$(objectsdir)/$(DEVELLIBS_NAME)
	install -d $(DESTDIR)$(helpdir)/$(DEVELLIBS_NAME)
	install -p $(externals_src)/devellibs/help/*.pd \
		$(DESTDIR)$(helpdir)/$(DEVELLIBS_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(DEVELLIBS_NAME)
#	install -p $(externals_src)/devellibs/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(DEVELLIBS_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/devellibs/README \
		$(DESTDIR)$(readmesdir)/$(DEVELLIBS_NAME).txt
	install -d $(DESTDIR)$(examplesdir)/$(DEVELLIBS_NAME)
	install -p $(externals_src)/devellibs/examples/*.pd \
		$(DESTDIR)$(examplesdir)/$(DEVELLIBS_NAME)

devellibs_clean:
	-rm -f -- $(DEVELLIBS_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(DEVELLIBS_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/devellibs/*.bak
	-rm -f -- $(externals_src)/devellibs/*.*~





#------------------------------------------------------------------------------#
# EKEXT
EKEXT_NAME=ekext
EKEXT_OBJECTS := $(wildcard $(externals_src)/ekext/*/*.c) \
	$(wildcard $(externals_src)/ekext/l*/*/*.c)
ekext: $(EKEXT_OBJECTS:.c=.$(EXTENSION))

ekext_install: ekext
	install -d $(DESTDIR)$(objectsdir)/$(EKEXT_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(EKEXT_NAME) \
		--author "Ed Kelly <morph_2016@yahoo.co.uk>" \
		--description "" \
		--license "BSD" \
		--version ""
	install -p $(EKEXT_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(EKEXT_NAME)
	install -d $(DESTDIR)$(helpdir)/$(EKEXT_NAME)
	install -p $(externals_src)/ekext/*/*help*.pd \
		$(DESTDIR)$(helpdir)/$(EKEXT_NAME)
	install -p $(externals_src)/ekext/*/*help*.pd \
		$(DESTDIR)$(objectsdir)/$(EKEXT_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(EKEXT_NAME)
#	install -p $(externals_src)/ekext/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(EKEXT_NAME)
#	install -p $(externals_src)/ekext/README \
#		$(DESTDIR)$(readmesdir)/$(EKEXT_NAME).txt
	install -d $(DESTDIR)$(examplesdir)/$(EKEXT_NAME)
	install -p $(externals_src)/ekext/peakit~/peakit-listmoses.pd \
		$(DESTDIR)$(examplesdir)/$(EKEXT_NAME)

ekext_clean:
	-rm -f -- $(EKEXT_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(EKEXT_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/ekext/*/*.bak
	-rm -f -- $(externals_src)/ekext/*/*.*~




#------------------------------------------------------------------------------#
# EXT13
ext13:
	make -C $(externals_src)/ext13

ext13_install:
	make -C $(externals_src)/ext13 DESTDIR="$(DESTDIR)" objectsdir="$(objectsdir)" install

ext13_clean:
	make -C $(externals_src)/ext13 clean



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
	install -d $(DESTDIR)$(objectsdir)/$(FFTEASE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(FFTEASE_NAME) \
		--author "Eric Lyon and Christopher Penrose" \
		--description "FFTease is a collection of objects implementing various forms of spectral sound processing. These include an additive-synthesis phase vocoder, noise reduction, cross synthesis, and more unusual forms of spectral processing." \
		--license "MIT License" \
		--version "2.0"
	install -p $(FFTEASE_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(FFTEASE_NAME)
	install -d $(DESTDIR)$(helpdir)/$(FFTEASE_NAME)
	install -p $(externals_src)/fftease/help/*.pd \
		$(DESTDIR)$(helpdir)/$(FFTEASE_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(FFTEASE_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/fftease/README \
		$(DESTDIR)$(readmesdir)/$(FFTEASE_NAME).txt
	install -d $(DESTDIR)$(examplesdir)/$(FFTEASE_NAME)
	install -p $(externals_src)/fftease/examples/*.pd \
		$(DESTDIR)$(examplesdir)/$(FFTEASE_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(FILE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(FILE_NAME) \
		--author "" \
		--description "" \
		--license "" \
		--version ""
	install -p $(FILE_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(FILE_NAME)
	install -d $(DESTDIR)$(helpdir)/$(FILE_NAME)
	install -p $(externals_src)/file/help/*.pd \
		$(DESTDIR)$(helpdir)/$(FILE_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(FILE_NAME)
#	install -p $(externals_src)/file/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(FILE_NAME)
#	install -p $(externals_src)/file/README \
#		$(DESTDIR)$(readmesdir)/$(FILE_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(FILE_NAME)
#	install -p $(externals_src)/file/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(FILE_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(FLIB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(FLIB_NAME) \
		--author "Jamie Bullock" \
		--description "library for feature extraction" \
		--license "GNU GPL"
	install -p $(FLIB_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(FLIB_NAME)
	install -d $(DESTDIR)$(helpdir)/$(FLIB_NAME)
	install -p $(externals_src)/postlude/flib/doc/*.pd \
		$(DESTDIR)$(helpdir)/$(FLIB_NAME)
	install -p $(externals_src)/postlude/flib/doc/*.pd \
		$(DESTDIR)$(objectsdir)/$(FLIB_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(FLIB_NAME)
#	install -p $(externals_src)/postlude/flib/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(FLIB_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/postlude/flib/README \
		$(DESTDIR)$(readmesdir)/$(FLIB_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(FLIB_NAME)
#	install -p $(externals_src)/postlude/flib/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(FLIB_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(FRANKENSTEIN_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(FRANKENSTEIN_NAME) \
		--author "" \
		--description "" \
		--license "" \
		--version ""
	install -p $(FRANKENSTEIN_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(FRANKENSTEIN_NAME)
#	install -d $(DESTDIR)$(helpdir)/$(FRANKENSTEIN_NAME)
#	install -p $(externals_src)/frankenstein/help/*.pd \
#		$(DESTDIR)$(helpdir)/$(FRANKENSTEIN_NAME)
	install -d $(DESTDIR)$(manualsdir)/$(FRANKENSTEIN_NAME)
	install -p $(externals_src)/frankenstein/doc/*.* \
		$(DESTDIR)$(manualsdir)/$(FRANKENSTEIN_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(FRANKENSTEIN_NAME)
	install -p $(externals_src)/frankenstein/patches/*.* \
		$(DESTDIR)$(examplesdir)/$(FRANKENSTEIN_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(FREEVERB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(FREEVERB_NAME) \
		--author "Olaf Matthes and Jezar Wakefield" \
		--description "Freeverb is a simple implementation of the standard Schroeder/Moorer reverb model" \
		--license "GNU GPL" \
		--version "1.2"
	install -p $(FREEVERB_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(FREEVERB_NAME)
	install -d $(DESTDIR)$(helpdir)/$(FREEVERB_NAME)
	install -p $(wildcard $(externals_src)/freeverb~/*-help.pd) \
		$(DESTDIR)$(helpdir)/$(FREEVERB_NAME)
	install -p $(wildcard $(externals_src)/freeverb~/*-help.pd) \
		$(DESTDIR)$(objectsdir)/$(FREEVERB_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(FREEVERB_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/freeverb~/README \
		$(DESTDIR)$(readmesdir)/$(FREEVERB_NAME).txt


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
	install -d $(DESTDIR)$(objectsdir)
#	install -d $(DESTDIR)$(objectsdir)/$(GEM2PDP_NAME)
#	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(GEM2PDP_NAME) \
#		--author "Yves Degoyon, Jamie Tittle, Georg Holzmann" \
#		--description "Bridges between Gem and PDP" \
#		--version "0.6"
	install -p $(GEM2PDP_OBJECTS:.cpp=.$(EXTENSION)) $(DESTDIR)$(objectsdir)
#	install -d $(DESTDIR)$(helpdir)/$(GEM2PDP_NAME)
	install -d $(DESTDIR)$(helpdir)
	install -p $(externals_src)/gem2pdp/*.pd  $(DESTDIR)$(helpdir)
#	install -d $(DESTDIR)$(manualsdir)/$(GEM2PDP_NAME)
#	install -p $(externals_src)/gem2pdp/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(GEM2PDP_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/gem2pdp/README \
		$(DESTDIR)$(readmesdir)/$(GEM2PDP_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(GEM2PDP_NAME)
#	install -p $(externals_src)/gem2pdp/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(GEM2PDP_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(GGEE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(GGEE_NAME) \
		--author "Guenter Geiger" \
		--description "" \
		--license "BSD" \
		--version "0.25"
	install -p $(GGEE_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(GGEE_NAME)
	install -p $(shell ls -1 $(externals_src)/vbap/*.pd | grep -v '-help.pd') \
		 $(DESTDIR)$(objectsdir)/$(GGEE_NAME)
	install -d $(DESTDIR)$(helpdir)/$(GGEE_NAME)
	install -p $(externals_src)/ggee/*/*-help.pd \
		$(externals_src)/ggee/*/*.gif $(DESTDIR)$(helpdir)/$(GGEE_NAME)
	install -p $(externals_src)/ggee/*/*-help.pd \
		$(externals_src)/ggee/*/*.gif $(DESTDIR)$(objectsdir)/$(GGEE_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(GGEE_NAME)
#	install -p $(externals_src)/ggee/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(GGEE_NAME)
#	install -p $(externals_src)/ggee/README \
#		$(DESTDIR)$(readmesdir)/$(GGEE_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(GGEE_NAME)
#	install -p $(externals_src)/ggee/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(GGEE_NAME)

ggee_clean:
	-rm -f -- $(GGEE_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(GGEE_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/ggee/*.bak
	-rm -f -- $(externals_src)/ggee/*.*~


#------------------------------------------------------------------------------#
# HARDWARE
HARDWARE_NAME=hardware
# memPIO has some special deps, so don't include it yet.
# uDMX requires libusb, so don't include it yet... 
# multio.c requires libusb, so don't include it yet...  <hans@at.or.at>
#HARDWARE_OBJECTS := $(wildcard $(externals_src)/hardware/*/*.cpp) \
#$(wildcard $(externals_src)/hardware/*/*.c)
#hardware: $(HARDWARE_OBJECTS:.c=.$(EXTENSION)) $(HARDWARE_OBJECTS:.cpp=.$(EXTENSION))
hardware:

hardware_install: hardware
	install -d $(DESTDIR)$(objectsdir)/$(HARDWARE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(HARDWARE_NAME) \
		--description "objects for working with hardware sensor boxes" \
		--license "GNU GPL"
# nothing built from source yet
#	install -p $(HARDWARE_OBJECTS:.c=.$(EXTENSION)) \
#		$(DESTDIR)$(objectsdir)/$(HARDWARE_NAME)
	install -p $(externals_src)/hardware/lanbox/lanbox.pd \
		$(DESTDIR)$(objectsdir)/$(HARDWARE_NAME)
	install -p $(externals_src)/hardware/DMXUSBPro/DMXUSBPro.pd \
		$(DESTDIR)$(objectsdir)/$(HARDWARE_NAME)	
	install -d $(DESTDIR)$(helpdir)/$(HARDWARE_NAME)
	install -p $(externals_src)/hardware/*/*-help.pd \
		$(DESTDIR)$(helpdir)/$(HARDWARE_NAME)
	install -d $(DESTDIR)$(manualsdir)/$(HARDWARE_NAME)
	install -p $(externals_src)/hardware/memPIO/README.txt \
		$(DESTDIR)$(manualsdir)/$(HARDWARE_NAME)/memPIO.txt
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/hardware/README.txt \
		$(DESTDIR)$(readmesdir)/$(HARDWARE_NAME).txt
	install -d $(DESTDIR)$(examplesdir)/$(HARDWARE_NAME)
#	install -p $(externals_src)/hardware/*/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(HARDWARE_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(HCS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(HCS_NAME) \
		--author "Hans-Christoph Steiner <hans@at.or.at>" \
		--description "random grab bag of object prototypes" \
		--license "GNU GPL" \
		--version ""
	install -p $(HCS_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(HCS_NAME)
	install -p $(shell ls -1 $(externals_src)/hcs/*.pd | \
			 grep -v '\-help.pd') $(DESTDIR)$(objectsdir)/$(HCS_NAME)
# hmm, these don't have a home yet...
	install -p $(externals_src)/hcs/general/*.pd $(DESTDIR)$(objectsdir)/$(HCS_NAME)
	install -d $(DESTDIR)$(helpdir)/$(HCS_NAME)
	install -p $(externals_src)/hcs/*-help.pd \
		$(DESTDIR)$(helpdir)/$(HCS_NAME)
	install -p $(externals_src)/hcs/*.gif \
		$(DESTDIR)$(helpdir)/$(HCS_NAME)
	install -p $(externals_src)/hcs/*-help.pd \
		$(DESTDIR)$(objectsdir)/$(HCS_NAME)
	install -p $(externals_src)/hcs/*.gif \
		$(DESTDIR)$(objectsdir)/$(HCS_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(HCS_NAME)
#	install -p $(externals_src)/hcs/README* $(externals_src)/hcs/TODO \
#		$(DESTDIR)$(manualsdir)/$(HCS_NAME)
#	install -d $(DESTDIR)$(readmesdir)
#	install -p $(externals_src)/hcs/README \
#		$(DESTDIR)$(readmesdir)/$(HCS_NAME).txt
	install -d $(DESTDIR)$(examplesdir)/$(HCS_NAME)
	install -p $(externals_src)/hcs/examples/*.pd \
		$(DESTDIR)$(examplesdir)/$(HCS_NAME)

hcs_clean:
	-rm -f -- $(HCS_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(HCS_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/hcs/*.bak
	-rm -f -- $(externals_src)/hcs/*.*~



#------------------------------------------------------------------------------#
# HID
HID_NAME = hid
ifeq ($(OS_NAME),windows)
  HID_CFLAGS = $(OPT_CFLAGS) $(CFLAGS) 
  HID_LIBS = $(LIBS) -lhid -lsetupapi
else
  ifeq ($(OS_NAME),darwin)
    FRAMEWORKS = Carbon IOKit ForceFeedback
    HID_CFLAGS = $(OPT_CFLAGS) $(CFLAGS) -I$(externals_src)/hcs/hid/HID\ Utilities\ Source
    HID_UTILITIES_SOURCE = $(externals_src)/hcs/hid/HID\ Utilities\ Source
    HID_LIBS = $(LIBS) -L$(HID_UTILITIES_SOURCE)/build \
		-L$(HID_UTILITIES_SOURCE)/build/Default \
      -lHIDUtilities $(patsubst %,-weak_framework %,$(FRAMEWORKS))
  else
    HID_CFLAGS = $(OPT_CFLAGS) $(CFLAGS) 
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
	install -d $(DESTDIR)$(objectsdir)/$(HID_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(HID_NAME) \
		--author "Hans-Christoph Steiner" \
		--description "" \
		--license "GNU GPL" \
		--version ""
	-install -p $(externals_src)/hcs/hid/hid.$(EXTENSION) $(DESTDIR)$(objectsdir)
	install -p $(externals_src)/hcs/hid/*.pd $(DESTDIR)$(objectsdir)/$(HID_NAME)
	install -d $(DESTDIR)$(helpdir)/$(HID_NAME)
	install -p $(externals_src)/hcs/hid/doc/*.pd       $(DESTDIR)$(helpdir)/$(HID_NAME)
	install -p $(externals_src)/hcs/hid/doc/*.pd       $(DESTDIR)$(objectsdir)/$(HID_NAME)
	install -p $(externals_src)/hcs/hid/examples/*.pd $(DESTDIR)$(helpdir)/$(HID_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(HID_NAME)
	install -p $(externals_src)/hcs/hid/examples/*.pd  $(DESTDIR)$(examplesdir)/$(HID_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(HIDIN_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(HIDIN_NAME) \
		--author "Olaf Matthes" \
		--description "HID input for Windows HID" \
		--license "GNU GPL"
	install -p $(HIDIN_DLL) $(DESTDIR)$(objectsdir)/$(HIDIN_NAME)
#	install -d $(DESTDIR)$(helpdir)/$(HIDIN_NAME)
#	install -p $(externals_src)/olafmatt/hidin/help/*.pd \
#		$(DESTDIR)$(helpdir)/$(HIDIN_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(HIDIN_NAME)
#	install -d $(DESTDIR)$(readmesdir)
#	install -p $(externals_src)/olafmatt/hidin/README \
#		$(DESTDIR)$(readmesdir)/$(HIDIN_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(HIDIN_NAME)
#	install -p $(externals_src)/olafmatt/hidin/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(HIDIN_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(HIDIO_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(HIDIO_NAME) \
		--author "Hans-Christoph Steiner <hans@at.or.at>, Olaf Matthes <olaf@nullmedium.de>, David Merrill <dmerrill@media.mit.edu>" \
		--description "I/O for USB HID and other supported devices" \
		--license "GNU GPLv2" \
		--version "0.0"
	-install -p $(externals_src)/io/hidio/hidio.$(EXTENSION) $(DESTDIR)$(objectsdir)
#	install -p $(externals_src)/io/hidio/*.pd $(DESTDIR)$(objectsdir)/$(HIDIO_NAME)
	install -d $(DESTDIR)$(helpdir)/$(HIDIO_NAME)
	install -p $(externals_src)/io/hidio/*-help.pd  $(DESTDIR)$(helpdir)/$(HIDIO_NAME)
#	install -p $(externals_src)/io/hidio/examples/*.pd $(DESTDIR)$(helpdir)/$(HIDIO_NAME)
#	install -d $(DESTDIR)$(examplesdir)/$(HIDIO_NAME)
#	install -p $(externals_src)/io/hidio/examples/*.pd  $(DESTDIR)$(examplesdir)/$(HIDIO_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(IEM_AMBI_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(IEM_AMBI_NAME) \
		--author "IEM/KUG, Graz, Austria" \
		--description "calculate ambisonic encoder matrices rotation matrices and decoder matrices from 1st to 4th order in 2D or 3D." \
		--license "GNU GPL" \
		--version ""
	install -p $(IEM_AMBI_OBJECTS:.c=.$(EXTENSION)) \
		$(DESTDIR)$(objectsdir)/$(IEM_AMBI_NAME)
	install -d $(DESTDIR)$(helpdir)/$(IEM_AMBI_NAME)
	install -p $(externals_src)/iem/iem_ambi/*.pd \
		$(DESTDIR)$(helpdir)/$(IEM_AMBI_NAME)
	install -p $(externals_src)/iem/iem_ambi/*.pd \
		$(DESTDIR)$(objectsdir)/$(IEM_AMBI_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(IEM_AMBI_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/iem/iem_ambi/READ_ME.txt \
		$(DESTDIR)$(readmesdir)/$(IEM_AMBI_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(IEM_AMBI_NAME)
#	install -p $(externals_src)/iem/iem_ambi/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(IEM_AMBI_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(IEM_BIN_AMBI_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(IEM_BIN_AMBI_NAME) \
		--author "IEM/KUG, Graz, Austria" \
		--description "calculate the product of an ambisonic decoder-matrix and the binaural HRIR's (in frequency and in time domain)" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEM_BIN_AMBI_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(IEM_BIN_AMBI_NAME)
	install -d $(DESTDIR)$(helpdir)/$(IEM_BIN_AMBI_NAME)
	install -p $(externals_src)/iem/iem_bin_ambi/*.pd \
		$(DESTDIR)$(helpdir)/$(IEM_BIN_AMBI_NAME)
	install -p $(externals_src)/iem/iem_bin_ambi/*.pd \
		$(DESTDIR)$(objectsdir)/$(IEM_BIN_AMBI_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(IEM_BIN_AMBI_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/iem/iem_bin_ambi/READ_ME.txt \
		$(DESTDIR)$(readmesdir)/$(IEM_BIN_AMBI_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(IEM_BIN_AMBI_NAME)
#	install -p $(externals_src)/iem/iem_bin_ambi/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(IEM_BIN_AMBI_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(IEM16_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(IEM16_NAME) \
		--author "IOhannes m zmoelnig" \
		--description "16bit table, array objects for low memory usage" \
		--license "GNU GPL" \
		--version "0.1"
	install -p $(IEM16_DIR)/*.$(EXTENSION) $(DESTDIR)$(objectsdir)/$(IEM16_NAME)
	install -d $(DESTDIR)$(helpdir)/$(IEM16_NAME)
	install -p $(externals_src)/iem16/help/*.pd \
		$(DESTDIR)$(helpdir)/$(IEM16_NAME)
	install -p $(externals_src)/iem16/help/*.pd \
		$(DESTDIR)$(objectsdir)/$(IEM16_NAME)

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
# IEMGUTS
IEMGUTS_NAME=iemguts
IEMGUTS_OBJECTS := $(wildcard $(externals_src)/iem/iemguts/src/*.c)
iemguts: $(IEMGUTS_OBJECTS:.c=.$(EXTENSION))


iemguts_install: iemguts
	install -d $(DESTDIR)$(objectsdir)/$(IEMGUTS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(IEMGUTS_NAME) \
		--author "IOhannes m zmoelnig" \
		--description "IEMguts is a collection of objects that deal with the infrastructure to build better abstractions" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEMGUTS_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(IEMGUTS_NAME)
	install -d $(DESTDIR)$(helpdir)/$(IEMGUTS_NAME)
	install -p $(externals_src)/iem/iemguts/help/*.pd \
		$(DESTDIR)$(helpdir)/$(IEMGUTS_NAME)
	install -p $(externals_src)/iem/iemguts/help/*.pd \
		$(DESTDIR)$(objectsdir)/$(IEMGUTS_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/iem/iemguts/README.txt \
		$(DESTDIR)$(readmesdir)/$(IEMGUTS_NAME).txt
	install -d $(DESTDIR)$(examplesdir)/$(IEMGUTS_NAME)
	install -p $(externals_src)/iem/iemguts/examples/*.pd \
		$(DESTDIR)$(examplesdir)/$(IEMGUTS_NAME)

iemguts_clean:
	-rm -f -- $(IEMGUTS_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(IEMGUTS_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/iem/iemguts/*.bak
	-rm -f -- $(externals_src)/iem/iemguts/*.*~



#------------------------------------------------------------------------------#
# IEMLIB
IEMLIB_NAME=iemlib
# omit the lib files and iem_mp3 (for patent reasons :( )
IEMLIB_SRC := $(wildcard $(externals_src)/iemlib/iemlib1/src/*[^1].c) $(wildcard $(externals_src)/iemlib/iemlib2/src/*[^2].c)  $(wildcard $(externals_src)/iemlib/iem_t3_lib/src/t3_*.c)  $(wildcard $(externals_src)/iemlib/alias/*.c)

IEMLIB_OBJECTS := $(IEMLIB_SRC:.c=.o)
$(IEMLIB_OBJECTS) : %.o : %.c
	$(CC) $(CFLAGS) -O2 -funroll-loops -fomit-frame-pointer -o "$*.o" -c "$*.c"

iemlib: $(IEMLIB_SRC:.c=.$(EXTENSION))

iemlib_install: iemlib
	install -d $(DESTDIR)$(objectsdir)/$(IEMLIB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(IEMLIB_NAME) \
		--description "a collection of objects written at IEM/KUG" \
		--license "GNU GPL"
	install -p $(IEMLIB_SRC:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/iemabs/*.pd $(DESTDIR)$(objectsdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/alias/*.pd $(DESTDIR)$(objectsdir)/$(IEMLIB_NAME)
	install -d $(DESTDIR)$(helpdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/*/*-help.pd $(DESTDIR)$(objectsdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/*/*-help.pd $(DESTDIR)$(helpdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/*/*.mp3 $(DESTDIR)$(helpdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/*/*.wav $(DESTDIR)$(helpdir)/$(IEMLIB_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/examples/*.* \
		$(DESTDIR)$(examplesdir)/$(IEMLIB_NAME)
	install -d $(DESTDIR)$(manualsdir)/$(IEMLIB_NAME)
	install -p $(externals_src)/iemlib/*.pdf $(externals_src)/iemlib/*.txt \
		$(externals_src)/iemlib/iemabs/*.txt \
		$(DESTDIR)$(manualsdir)/$(IEMLIB_NAME)
# kludge to get a single working output~ straight in 'extra'
	-rm -f -- $(DESTDIR)$(objectsdir)/$(IEMLIB_NAME)/output~.pd


iemlib_clean: 
	-rm -f -- $(IEMLIB_OBJECTS:.c=.$(EXTENSION))
	-rmdir -- $(DESTDIR)$(objectsdir)/$(IEMLIB_NAME)
	-rm -f -- $(DESTDIR)$(helpdir)/$(IEMLIB_NAME)/*.*
	-rmdir -- $(DESTDIR)$(helpdir)/$(IEMLIB_NAME)
	-rm -f -- $(DESTDIR)$(examplesdir)/$(IEMLIB_NAME)/*.*
	-rmdir -- $(DESTDIR)$(examplesdir)/$(IEMLIB_NAME)
	-rm -f -- $(DESTDIR)$(manualsdir)/$(IEMLIB_NAME)/*.*
	-rmdir -- $(DESTDIR)$(manualsdir)/$(IEMLIB_NAME)



#------------------------------------------------------------------------------#
# IEMMATRIX
## oh this is all sh!t: please use iemmatrix's build-system instead
IEMMATRIX_NAME=iemmatrix
IEMMATRIX_ROOT := $(externals_src)/iem/iemmatrix
IEMMATRIX_SRC := $(wildcard $(IEMMATRIX_ROOT)/src/m[at]*.c)
IEMMATRIX_ALIAS := $(wildcard $(IEMMATRIX_ROOT)/alias/*.c)
IEMMATRIX_OBJ := $(IEMMATRIX_SRC:.c=.o) $(IEMMATRIX_ALIAS:.c=.o)
IEMMATRIX_SHARED := $(wildcard $(IEMMATRIX_ROOT)/src/iemmatrix_binops.c) \
                    $(wildcard $(IEMMATRIX_ROOT)/src/iemmatrix_utility.c)

iemmatrix: $(IEMMATRIX_OBJ:.o=.$(EXTENSION))

$(IEMMATRIX_OBJ:.o=.$(EXTENSION)) : %.$(EXTENSION) : $(IEMMATRIX_OBJ) $(IEMMATRIX_SHARED:.c=.o)
	$(CC) $(LDFLAGS) -o $*.$(EXTENSION) "$*.o" $(IEMMATRIX_SHARED:.c=.o) $(LIBS)
	$(STRIP) $*.$(EXTENSION)
	chmod 755 $*.$(EXTENSION)
	#rm -f -- "$*.o"

$(IEMMATRIX_OBJ) $(IEMMATRIX_SHARED:.c=.o) : %.o : %.c
	$(CC) $(OPT_CFLAGS) $(CFLAGS) -o "$*.o" -c "$*.c" -I$(IEMMATRIX_ROOT)/src

iemmatrix_install: iemmatrix
	install -d $(DESTDIR)$(objectsdir)/$(IEMMATRIX_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(IEMMATRIX_NAME) \
		--author "IOhannes m zmoelnig (zmoelnig AT iem DOT at), thomas musil (musil AT iem DOT at), franz zotter (zotter AT iem DOT at)" \
		--description "objects for matrix operations and math" \
		--license "GNU GPL" \
		--version "$(IEMMATRIX_VERSION)"
	install -p $(IEMMATRIX_OBJ:.o=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(IEMMATRIX_NAME)
	install -p $(shell ls -1 $(externals_src)/iem/iemmatrix/abs/*.* | \
		grep -v '\-help.pd') $(DESTDIR)$(objectsdir)/$(IEMMATRIX_NAME)
	install -d $(DESTDIR)$(helpdir)/$(IEMMATRIX_NAME)
	install -p $(externals_src)/iem/iemmatrix/doc/*.pd \
		$(DESTDIR)$(helpdir)/$(IEMMATRIX_NAME)
	install -p $(externals_src)/iem/iemmatrix/abs/*-help.pd \
		$(DESTDIR)$(helpdir)/$(IEMMATRIX_NAME)
	install -p $(externals_src)/iem/iemmatrix/abs/*-help.pd \
		$(DESTDIR)$(objectsdir)/$(IEMMATRIX_NAME)
	install -d $(DESTDIR)$(manualsdir)/$(IEMMATRIX_NAME)
	install -p $(externals_src)/iem/iemmatrix/*.txt \
		$(DESTDIR)$(manualsdir)/$(IEMMATRIX_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(IEM_MATRIX_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(IEM_MATRIX_NAME) \
		--author "Thomas Musil" \
		--description "audio matrix objects" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEM_MATRIX_OBJ:.o=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(IEM_MATRIX_NAME)
	install -d $(DESTDIR)$(helpdir)/$(IEM_MATRIX_NAME)
	install -p $(externals_src)/iem/iem_matrix/*.pd \
		$(DESTDIR)$(helpdir)/$(IEM_MATRIX_NAME)
	install -p $(externals_src)/iem/iem_matrix/*.pd \
		$(DESTDIR)$(objectsdir)/$(IEM_MATRIX_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(JASCH_LIB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(JASCH_LIB_NAME) \
		--author "Jasch" \
		--license "GNU GPLv2" \
		--version ""
	install -p $(JASCH_LIB_OBJECTS:.c=.$(EXTENSION)) \
		$(DESTDIR)$(objectsdir)/$(JASCH_LIB_NAME)
	install -d $(DESTDIR)$(helpdir)/$(JASCH_LIB_NAME)
	install -p $(externals_src)/jasch_lib/*/*-help.pd \
		$(DESTDIR)$(helpdir)/$(JASCH_LIB_NAME)
	install -p $(externals_src)/jasch_lib/*/*-help.pd \
		$(DESTDIR)$(objectsdir)/$(JASCH_LIB_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(JASCH_LIB_NAME)
#	install -p $(externals_src)/jasch_lib/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(JASCH_LIB_NAME)
#	install -d $(DESTDIR)$(readmesdir)
#	install -p $(externals_src)/jasch_lib/README \
#		$(DESTDIR)$(readmesdir)/$(JASCH_LIB_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(JASCH_LIB_NAME)
#	install -p $(externals_src)/jasch_lib/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(JASCH_LIB_NAME)

jasch_lib_clean:
	-rm -f -- $(JASCH_LIB_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(JASCH_LIB_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/jasch_lib/*/*.bak
	-rm -f -- $(externals_src)/jasch_lib/*/*.*~



#------------------------------------------------------------------------------#
# LOADERS
LOADERS_NAME=
LOADERS_OBJECTS := $(wildcard $(externals_src)/loaders/*.c)  \
	$(externals_src)/loaders/hexloader/hexloader.c
loaders: $(LOADERS_OBJECTS:.c=.$(EXTENSION))
	echo $(LOADERS_OBJECTS)
loaders_install: loaders
	install -d $(DESTDIR)$(objectsdir)/$(LOADERS_NAME)
	install -p $(LOADERS_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(LOADERS_NAME)
	install -d $(DESTDIR)$(helpdir)/$(LOADERS_NAME)
#	install -p $(wildcard $(externals_src)/loaders/help/*.pd) \
#		$(DESTDIR)$(helpdir)/$(LOADERS_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(LOADERS_NAME)
#	install -p $(externals_src)/loaders/README.txt $(DESTDIR)$(manualsdir)/$(LOADERS_NAME)

loaders_clean:
	-rm -f -- $(LOADERS_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(externals_src)/loaders/*.o
	-rm -f -- $(externals_src)/loaders/*.bak
	-rm -f -- $(externals_src)/loaders/*.*~


#------------------------------------------------------------------------------#
# MAPPING
mapping:
	make -C $(externals_src)/mapping

mapping_install:
	make -C $(externals_src)/mapping DESTDIR="$(DESTDIR)" objectsdir="$(objectsdir)" install

mapping_clean:
	make -C $(externals_src)/mapping clean


#------------------------------------------------------------------------------#
# MARKEX
MARKEX_NAME=markex
MARKEX_OBJECTS := $(wildcard $(externals_src)/markex/[a-z]*.c)
markex: $(MARKEX_OBJECTS:.c=.$(EXTENSION))

markex_install: markex
	install -d $(DESTDIR)$(objectsdir)/$(MARKEX_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(MARKEX_NAME) \
		--author "Mark Danks" \
		--license "GNU GPL"
	install -p $(MARKEX_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(MARKEX_NAME)
	install -d $(DESTDIR)$(helpdir)/$(MARKEX_NAME)
	install -p $(wildcard $(externals_src)/markex/*.pd) \
		$(DESTDIR)$(helpdir)/$(MARKEX_NAME)
	install -p $(wildcard $(externals_src)/markex/*.pd) \
		$(DESTDIR)$(objectsdir)/$(MARKEX_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(MARKEX_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/markex/README \
		$(DESTDIR)$(readmesdir)/$(MARKEX_NAME).txt

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
	install -d $(DESTDIR)$(objectsdir)/$(MAXLIB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(MAXLIB_NAME) \
		--author "Olaf Matthes <olaf.matthes@gmx.de>" \
		--license "GNU GPL" \
		--version "1.5.2-cvs"
	install -p $(MAXLIB_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(MAXLIB_NAME)
	install -d $(DESTDIR)$(helpdir)/$(MAXLIB_NAME)
	install -p $(wildcard $(externals_src)/maxlib/help/*.pd) \
		$(DESTDIR)$(helpdir)/$(MAXLIB_NAME)
	install -p $(wildcard $(externals_src)/maxlib/help/*.pd) \
		$(DESTDIR)$(objectsdir)/$(MAXLIB_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(MAXLIB_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/maxlib/README \
		$(DESTDIR)$(readmesdir)/$(MAXLIB_NAME).txt

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
	install -d $(DESTDIR)$(objectsdir)/$(MJLIB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(MJLIB_NAME) \
		--author "mark williamson <mark@junklight.com>" \
		--license "GNU GPL" \
		--version "0.1"
	install -p $(MJLIB_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(MJLIB_NAME)
	install -d $(DESTDIR)$(helpdir)/$(MJLIB_NAME)
	install -p $(wildcard $(externals_src)/mjlib/doc/*.pd) \
		$(DESTDIR)$(helpdir)/$(MJLIB_NAME)
	install -p $(wildcard $(externals_src)/mjlib/doc/*.pd) \
		$(DESTDIR)$(objectsdir)/$(MJLIB_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(MJLIB_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/mjlib/readme.txt \
		$(DESTDIR)$(readmesdir)/$(MJLIB_NAME).txt

mjlib_clean:
	-rm -f -- $(externals_src)/mjlib/*.$(EXTENSION)
	-rm -f -- $(externals_src)/mjlib/*.o
	-rm -f -- $(externals_src)/mjlib/*.bak
	-rm -f -- $(externals_src)/mjlib/*.*~


#------------------------------------------------------------------------------#
# MOOCOW
MOOCOW_NAME=moocow
#MOOCOW_OBJECTS := $(shell cat $(externals_src)/moocow/extended/objects)
MOOCOW_DIR=$(externals_src)/moocow/extended
MOOCOW_BUILD=$(MOOCOW_DIR)/build.moo

##-- pass some variables on to sub-make
## + we should probably just use make's "export" for this,
##   maybe even exporting all variables by default...
MOOCOW_MAKEFLAGS = \
	CFLAGS="$(CFLAGS)" \
	pd_src="$(pd_src)"

$(externals_src)/moocow/extended/build.stamp:
	$(MAKE) -C $(MOOCOW_DIR) $(MOOCOW_MAKEFLAGS) build.stamp \
	  || echo "moocow: WARNING: build failed"

moocow: $(MOOCOW_DIR)/build.stamp

moocow_install:
	install -d $(DESTDIR)$(objectsdir)/$(MOOCOW_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(MOOCOW_NAME) \
		--author "Bryan Jurish <moocow@bbaw.de>" \
		--description "moocow's externals" \
		--license "GNU GPL" \
		--version "CVS.`date +%Y-%m-%d`"
	install -p $(MOOCOW_BUILD)/ext*/*.$(EXTENSION) \
		$(DESTDIR)$(objectsdir)/$(MOOCOW_NAME) \
	  || echo 'moocow_install: WARNING: no library externals to install!'
	install -p $(MOOCOW_BUILD)/ext*/*.pd \
		$(DESTDIR)$(objectsdir)/$(MOOCOW_NAME) \
	  || echo 'moocow_install: WARNING: no library patches to install!'
	install -d $(DESTDIR)$(helpdir)/$(MOOCOW_NAME)
	install -p $(MOOCOW_BUILD)/doc/5.reference/*.pd \
		$(DESTDIR)$(objectsdir)/$(MOOCOW_NAME) \
	install -p $(MOOCOW_BUILD)/doc/5.reference/*.pd \
		$(DESTDIR)$(helpdir)/$(MOOCOW_NAME) \
	  || echo 'moocow_install: WARNING: no help patches to install!'
#	install -d $(DESTDIR)$(manualsdir)/$(MOOCOW_NAME)
#	install -p $(externals_src)/moocow/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(MOOCOW_NAME) \
#	  || echo 'moocow_install: WARNING: no manuals to install!'
	install -d $(DESTDIR)$(readmesdir)
	install -p $(MOOCOW_DIR)/README.txt \
		$(DESTDIR)$(readmesdir)/$(MOOCOW_NAME).txt \
	  || echo 'moocow_install: WARNING: no README to install!'
#	install -d $(DESTDIR)$(examplesdir)/$(MOOCOW_NAME)
#	install -p $(MOOCOW_BUILD)/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(MOOCOW_NAME) \
#	  || echo "moocow_install: WARNING: no examples patches to install!"

moocow_clean:
	$(MAKE) -C $(externals_src)/moocow/extended distclean
	-rm -f -- $(MOOCOW_DIR)/*.bak
	-rm -f -- $(MOOCOW_DIR)/*.*~


#------------------------------------------------------------------------------#
# MOONLIB
MOONLIB_NAME=moonlib
MOONLIB_OBJECTS := $(wildcard $(externals_src)/moonlib/*.c)
moonlib: $(MOONLIB_OBJECTS:.c=.$(EXTENSION))

moonlib_install: moonlib
	install -d $(DESTDIR)$(objectsdir)/$(MOONLIB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(MOONLIB_NAME) \
		--author "Antoine Rousseau" \
		--version "0.2"
	install -p $(MOONLIB_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(MOONLIB_NAME)
	install -d $(DESTDIR)$(helpdir)/$(MOONLIB_NAME)
	install -p $(externals_src)/moonlib/help/*.pd $(DESTDIR)$(objectsdir)/$(MOONLIB_NAME)
	install -p $(externals_src)/moonlib/help/*.pd $(DESTDIR)$(helpdir)/$(MOONLIB_NAME)
	install -p $(externals_src)/moonlib/help/*.gif $(DESTDIR)$(helpdir)/$(MOONLIB_NAME)
	install -d $(DESTDIR)$(helpdir)/$(MOONLIB_NAME)/d
	install -p $(externals_src)/moonlib/help/d/*.gif $(DESTDIR)$(helpdir)/$(MOONLIB_NAME)/d
#	install -d $(DESTDIR)$(manualsdir)/$(MOONLIB_NAME)
#	install -p $(externals_src)/moonlib/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(MOONLIB_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/moonlib/README.txt \
		$(DESTDIR)$(readmesdir)/$(MOONLIB_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(MOONLIB_NAME)
#	install -p $(externals_src)/moonlib/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(MOONLIB_NAME)

moonlib_clean:
	-rm -f -- $(MOONLIB_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(MOONLIB_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/moonlib/*.bak
	-rm -f -- $(externals_src)/moonlib/*.*~



#------------------------------------------------------------------------------#
# MOTEX
motex:
	make -C $(externals_src)/motex

motex_install:
	make -C $(externals_src)/motex DESTDIR="$(DESTDIR)" objectsdir="$(objectsdir)" install

motex_clean:
	make -C $(externals_src)/motex clean




#------------------------------------------------------------------------------#
# MRPEACH
MRPEACH_NAME=mrpeach
MRPEACH_OBJECTS := $(wildcard $(externals_src)/mrpeach/*/*.c)
mrpeach: $(MRPEACH_OBJECTS:.c=.$(EXTENSION))

mrpeach_install: mrpeach
	install -d $(DESTDIR)$(objectsdir)/$(MRPEACH_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(MRPEACH_NAME) \
		--author "Martin Peach <martin.peach@sympatico.ca>" \
		--description "" \
		--license "GNU GPL" \
		--version "0.1"
	install -p $(MRPEACH_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(MRPEACH_NAME)
	install -d $(DESTDIR)$(helpdir)/$(MRPEACH_NAME)
	install -p $(externals_src)/mrpeach/*/*-help.pd \
		$(DESTDIR)$(helpdir)/$(MRPEACH_NAME)
	install -p $(externals_src)/mrpeach/*/*-help.pd \
		$(DESTDIR)$(objectsdir)/$(MRPEACH_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(MRPEACH_NAME)
#	install -p $(externals_src)/mrpeach/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(MRPEACH_NAME)
#	install -d $(DESTDIR)$(readmesdir)
#	install -p $(externals_src)/mrpeach/README \
#		$(DESTDIR)$(readmesdir)/$(MRPEACH_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(MRPEACH_NAME)
#	install -p $(externals_src)/mrpeach/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(MRPEACH_NAME)

mrpeach_clean:
	-rm -f -- $(MRPEACH_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(MRPEACH_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/mrpeach/*.bak
	-rm -f -- $(externals_src)/mrpeach/*.*~


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
	install -d $(DESTDIR)$(objectsdir)/$(OSCX_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(OSCX_NAME) \
		--author "<jdl@xdv.org>" \
		--license "BSD" \
		--description "objects for working with OpenSoundControl"
	install -p $(externals_src)/OSCx/src/*.$(EXTENSION) $(DESTDIR)$(objectsdir)/$(OSCX_NAME)
	install -d $(DESTDIR)$(helpdir)/$(OSCX_NAME)
	install -p $(externals_src)/OSCx/doc/*.* $(DESTDIR)$(helpdir)/$(OSCX_NAME)
	install -p $(externals_src)/OSCx/doc/*.* $(DESTDIR)$(objectsdir)/$(OSCX_NAME)


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
	install -d $(DESTDIR)$(objectsdir)/$(PAN_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(PAN_NAME) \
		--author "Hans-Christoph Steiner <hans@at.or.at>" \
		--description "A library of panning algoritms" \
		--license "GNU GPL" \
		--version ""
# everything is .pd, nothing to compile (yet?)
#	install -p $(PAN_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(PAN_NAME)
	install -p $(wildcard $(externals_src)/hcs/pan/*.pd) \
		$(DESTDIR)$(objectsdir)/$(PAN_NAME)
	install -d $(DESTDIR)$(helpdir)/$(PAN_NAME)
	install -p $(externals_src)/hcs/pan/help/*.pd \
		$(DESTDIR)$(helpdir)/$(PAN_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(PAN_NAME)
#	install -p $(externals_src)/hcs/pan/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(PAN_NAME)
#	install -d $(DESTDIR)$(readmesdir)
#	install -p $(externals_src)/hcs/pan/README \
#		$(DESTDIR)$(readmesdir)/$(PAN_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(PAN_NAME)
#	install -p $(externals_src)/hcs/pan/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(PAN_NAME)


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
	install -d $(DESTDIR)$(objectsdir)/$(PDDP_NAME)
#	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(PDDP_NAME) \
#		--author "PDDP" \
#		--license "BSD" \
#		--description "objects for working with Tcl and Pd's Tk GUI"
# pddp is compiled straight into $(OUT_DIR)
	cd $(externals_src)/miXed/pddp && $(MAKE) \
		OUT_DIR=$(DESTDIR)$(objectsdir)/$(PDDP_NAME)
	install -d $(DESTDIR)$(objectsdir)/$(PDDP_NAME)/pddp
	install -p $(externals_src)/miXed/bin/pddp/*.tcl \
		$(DESTDIR)$(objectsdir)/$(PDDP_NAME)/pddp
#	install -d $(DESTDIR)$(helpdir)/$(PDDP_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(PDDP_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(PDDP_NAME)
	install -p $(externals_src)/miXed/test/pddp/*.*  \
		$(DESTDIR)$(examplesdir)/$(PDDP_NAME)


pddp_clean:
	-$(MAKE) -C $(externals_src)/mixed/pddp $(DEST_PATHS) clean



#------------------------------------------------------------------------------#
# PDLUA
PDLUA_NAME=pdlua
PDLUA_OBJECTS := $(externals_src)/loaders/pdlua/src/lua.c
pdlua: $(PDLUA_OBJECTS:.c=.$(EXTENSION))

pdlua_install: pdlua
	install -d $(DESTDIR)$(objectsdir)
	install -p $(PDLUA_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)
	install -p $(externals_src)/loaders/pdlua/src/pd.lua $(DESTDIR)$(objectsdir)
	install -d $(DESTDIR)$(manualsdir)/$(PDLUA_NAME)
	install -p $(externals_src)/loaders/pdlua/doc/*.txt \
		$(DESTDIR)$(manualsdir)/$(PDLUA_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/loaders/pdlua/README \
		$(DESTDIR)$(readmesdir)/$(PDLUA_NAME).txt
	install -d $(DESTDIR)$(examplesdir)/$(PDLUA_NAME)
	install -p $(externals_src)/loaders/pdlua/examples/*.* \
		$(DESTDIR)$(examplesdir)/$(PDLUA_NAME)

pdlua_clean:
	-rm -f -- $(PDLUA_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(PDLUA_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/loaders/pdlua/*.bak
	-rm -f -- $(externals_src)/loaders/pdlua/*.*~


#------------------------------------------------------------------------------#
# PDOGG
PDOGG_NAME=pdogg
PDOGG_OBJECTS := $(wildcard $(externals_src)/pdogg/*/*.c)
pdogg: $(PDOGG_OBJECTS:.c=.$(EXTENSION))

pdogg_install: pdogg
	install -d $(DESTDIR)$(objectsdir)/$(PDOGG_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(PDOGG_NAME) \
		--author "Olaf Matthes" \
		--description "objects for reading, writing, and streaming ogg" \
		--license "LGPL" \
		--version "0.2"
	install -p $(PDOGG_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(PDOGG_NAME)
	install -d $(DESTDIR)$(helpdir)/$(PDOGG_NAME)
	install -p $(externals_src)/pdogg/*/*.pd \
		$(DESTDIR)$(helpdir)/$(PDOGG_NAME)
	install -p $(externals_src)/pdogg/*/*.pd \
		$(DESTDIR)$(objectsdir)/$(PDOGG_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(PDOGG_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/pdogg/readme \
		$(DESTDIR)$(readmesdir)/$(PDOGG_NAME).txt

pdogg_clean:
	-rm -f -- $(externals_src)/pdogg/*/*.$(EXTENSION)
	-rm -f -- $(externals_src)/pdogg/*/*.o
	-rm -f -- $(externals_src)/pdogg/*/*.bak
	-rm -f -- $(externals_src)/pdogg/*/*.*~
	-rm -f -- $(externals_src)/pdogg/*.*~



#------------------------------------------------------------------------------#
# PDP
ifeq ($(ARCH),i386)
  PDP_OPTIONS = --enable-quicktime --enable-mmx
else
  PDP_OPTIONS = --enable-quicktime
endif

PDP_NAME=pdp
$(externals_src)/pdp/configure: $(externals_src)/pdp/configure.ac
	cd $(externals_src)/pdp && autoconf

$(externals_src)/pdp/Makefile.config: $(externals_src)/pdp/configure $(externals_src)/pdp/Makefile.config.in
	cd $(externals_src)/pdp && ./configure PD_CPPFLAGS="-I$(pd_src)/src" \
		$(PDP_OPTIONS)

$(externals_src)/pdp/pdp.$(EXTENSION): $(externals_src)/pdp/Makefile.config 
	$(MAKE) -C $(externals_src)/pdp PD_EXECUTABLE="$(pd_src)/bin/pd"

pdp: $(externals_src)/pdp/pdp.$(EXTENSION)

pdp_install: pdp
	install -d $(DESTDIR)$(objectsdir)/$(PDP_NAME)
#	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(PDP_NAME) \
#		--author "Tom Schouten" \
#		--description "Pure Data Packet" \
#		--license "GNU GPL"
	install -d $(DESTDIR)$(objectsdir)
	install -p $(externals_src)/pdp/*.$(EXTENSION) $(DESTDIR)$(objectsdir)/$(PDP_NAME)
#	install -p $(externals_src)/pdp/abstractions/*.pd $(DESTDIR)$(objectsdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/abstractions/*.pd $(DESTDIR)$(objectsdir)
	install -d $(DESTDIR)$(helpdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/doc/objects/*.* $(DESTDIR)$(helpdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/doc/objects/*.* $(DESTDIR)$(objectsdir)/$(PDP_NAME)
	install -d $(DESTDIR)$(manualsdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/doc/reference.txt $(DESTDIR)$(manualsdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/doc/introduction/*.* \
		$(DESTDIR)$(manualsdir)/$(PDP_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/doc/examples/*.* \
		$(DESTDIR)$(examplesdir)/$(PDP_NAME)

pdp_clean:
	-rm -f -- $(externals_src)/pdp/*.$(EXTENSION)
	-find $(externals_src)/pdp -name '*.o' | xargs rm -f --
	-find $(externals_src)/pdp -name '*.bak' | xargs rm -f --
	-rm -f -- $(externals_src)/pdp/Makefile.config
	-rm -f -- $(externals_src)/pdp/configure


#------------------------------------------------------------------------------#
# PDP_OPENGL
PDP_OPENGL_NAME=3dp
PDP_OPENGL_BINARY := $(externals_src)/pdp/opengl/pdp_opengl.$(EXTENSION)
$(PDP_OPENGL_BINARY): $(externals_src)/pdp/Makefile.config
	make -C $(externals_src)/pdp/opengl

pdp_opengl: $(PDP_OPENGL_BINARY)
	echo $(PDP_OPENGL_BINARY)

pdp_opengl_install: pdp_opengl
	install -d $(DESTDIR)$(objectsdir)/$(PDP_OPENGL_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(PDP_OPENGL_NAME) \
		--author "Tom Schouten" \
		--description "opengl extensions for pdp" \
		--license "GPL v2 or later"
	install -p $(PDP_OPENGL_BINARY) $(DESTDIR)$(objectsdir)
	install -p $(externals_src)/pdp/opengl/abstractions/*.pd \
		$(DESTDIR)$(objectsdir)/$(PDP_OPENGL_NAME)
	install -d $(DESTDIR)$(helpdir)/$(PDP_OPENGL_NAME)
	install -p $(externals_src)/pdp/opengl/doc/objects/*.pd $(DESTDIR)$(objectsdir)
	install -p $(externals_src)/pdp/opengl/doc/objects/*.pd $(DESTDIR)$(objectsdir)/$(PDP_OPENGL_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/pdp/opengl/README \
		$(DESTDIR)$(readmesdir)/$(PDP_OPENGL_NAME).txt
	install -d $(DESTDIR)$(examplesdir)/$(PDP_OPENGL_NAME)
	install -p $(externals_src)/pdp/opengl/doc/examples/*.pd \
		$(DESTDIR)$(examplesdir)/$(PDP_OPENGL_NAME)
	install -p $(externals_src)/pdp/opengl/test/*.pd \
		$(DESTDIR)$(examplesdir)/$(PDP_OPENGL_NAME)

pdp_opengl_clean:
	make -C $(externals_src)/pdp/opengl clean
	-rm -f -- $(PDP_OPENGL_BINARY)
	-rm -f -- $(externals_src)/pdp_opengl/*.bak
	-rm -f -- $(externals_src)/pdp_opengl/*.*~


#------------------------------------------------------------------------------#
# PDVJTOOLS
pdvjtools:
	make -C $(externals_src)/pdvjtools

pdvjtools_install:
	make -C $(externals_src)/pdvjtools DESTDIR="$(DESTDIR)" objectsdir="$(objectsdir)" install

pdvjtools_clean:
	make -C $(externals_src)/pdvjtools clean


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
	install -d $(DESTDIR)$(objectsdir)/$(PIDIP_NAME)
#	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(PIDIP_NAME) \
#		--author "Yves Degoyon" \
#		--description "PiDiP is Definitely in Pieces" \
#		--license "GNU GPLv2"
	install -p $(externals_src)/pidip/*.$(EXTENSION) $(DESTDIR)$(objectsdir)/$(PIDIP_NAME)
	install -d $(DESTDIR)$(helpdir)/$(PIDIP_NAME)
	install -p $(externals_src)/pidip/doc/*.pd $(DESTDIR)$(helpdir)/$(PIDIP_NAME)
	install -p $(externals_src)/pidip/doc/*.pd $(DESTDIR)$(objectsdir)/$(PIDIP_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(PIDIP_NAME)
	install -p $(externals_src)/pidip/patches/*.* $(DESTDIR)$(examplesdir)/$(PIDIP_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(PIDIP_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/pidip/README \
		$(DESTDIR)$(readmesdir)/$(PIDIP_NAME).txt
	install -d $(DESTDIR)$(examplesdir)/$(PIDIP_NAME)/images
	install -p $(externals_src)/pidip/patches/images/*.* \
		$(DESTDIR)$(examplesdir)/$(PIDIP_NAME)/images
	install -d $(DESTDIR)$(examplesdir)/$(PIDIP_NAME)/morphology
	install -p $(externals_src)/pidip/patches/morphology/*.* \
		$(DESTDIR)$(examplesdir)/$(PIDIP_NAME)/morphology

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
	install -d $(DESTDIR)$(objectsdir)/$(PMPD_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(PMPD_NAME) \
		--author "Cyrille Henry" \
		--description "Physical Modelling for Pd"
	-install -p $(externals_src)/pmpd/*.$(EXTENSION) $(DESTDIR)$(objectsdir)/$(PMPD_NAME)
	install -d $(DESTDIR)$(helpdir)/$(PMPD_NAME)
	install -p $(externals_src)/pmpd/help/*.pd $(DESTDIR)$(helpdir)/$(PMPD_NAME)
	install -p $(externals_src)/pmpd/help/*.pd $(DESTDIR)$(objectsdir)/$(PMPD_NAME)
	install -d $(DESTDIR)$(manualsdir)/$(PMPD_NAME)
	install -d $(DESTDIR)$(manualsdir)/$(PMPD_NAME)
	install -p $(externals_src)/pmpd/doc/pmpd.pdf/pmpd.pdf \
		$(DESTDIR)$(manualsdir)/$(PMPD_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(PMPD_NAME)
	install -p $(externals_src)/pmpd/exemples/*.pd \
		$(DESTDIR)$(examplesdir)/$(PMPD_NAME)

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
#	$(externals_src)/postlude/psql/psql.c
postlude: $(POSTLUDE_OBJECTS:.c=.$(EXTENSION))

postlude_install: postlude
	install -d $(DESTDIR)$(objectsdir)/$(POSTLUDE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(POSTLUDE_NAME) \
		--author "Jamie Bullock" \
		--license "GNU GPL"
	install -p $(POSTLUDE_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(POSTLUDE_NAME)
	install -p $(externals_src)/postlude/dssi/doc/output~.pd \
		$(DESTDIR)$(objectsdir)/$(POSTLUDE_NAME)
	install -d $(DESTDIR)$(helpdir)/$(POSTLUDE_NAME)
	install -p $(externals_src)/postlude/*/doc/*-help.pd \
		$(DESTDIR)$(helpdir)/$(POSTLUDE_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(POSTLUDE_NAME)
#	install -p $(externals_src)/postlude/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(POSTLUDE_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/postlude/dssi/README \
		$(DESTDIR)$(readmesdir)/$(POSTLUDE_NAME)-dssi.txt
	install -p $(externals_src)/postlude/psql/README \
		$(DESTDIR)$(readmesdir)/$(POSTLUDE_NAME)-psql.txt
	install -d $(DESTDIR)$(examplesdir)/$(POSTLUDE_NAME)
#	install -p $(externals_src)/postlude/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(POSTLUDE_NAME)

postlude_clean:
	-rm -f -- $(POSTLUDE_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(POSTLUDE_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/postlude/*/*.bak
	-rm -f -- $(externals_src)/postlude/*/*.*~



#------------------------------------------------------------------------------#
# SIGPACK
sigpack:
	make -C $(externals_src)/sigpack

sigpack_install:
	make -C $(externals_src)/sigpack DESTDIR="$(DESTDIR)" objectsdir="$(objectsdir)" install

sigpack_clean:
	make -C $(externals_src)/sigpack clean


#------------------------------------------------------------------------------#
# SIXAXIS
SIXAXIS_NAME=io
SIXAXIS_OBJECTS = $(externals_src)/io/sixaxis/sixaxis.c
sixaxis: $(SIXAXIS_OBJECTS:.c=.$(EXTENSION))

sixaxis_install: sixaxis
	install -d $(DESTDIR)$(objectsdir)/$(SIXAXIS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(SIXAXIS_NAME) \
		--author "Hans-Christoph Steiner" \
		--description "supports the Sony SIXAXIS controller's accelerometer" \
		--license "GNU GPL" \
		--version "0.1"
	install -p $(SIXAXIS_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(SIXAXIS_NAME)
	install -d $(DESTDIR)$(helpdir)/$(SIXAXIS_NAME)
	install -p $(externals_src)/io/sixaxis/*.pd \
		$(DESTDIR)$(helpdir)/$(SIXAXIS_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(SIXAXIS_NAME)
#	install -p $(externals_src)/io/sixaxis/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(SIXAXIS_NAME)
#	install -d $(DESTDIR)$(readmesdir)
#	install -p $(externals_src)/io/sixaxis/README \
#		$(DESTDIR)$(readmesdir)/$(SIXAXIS_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(SIXAXIS_NAME)
#	install -p $(externals_src)/io/sixaxis/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(SIXAXIS_NAME)

sixaxis_clean:
	-rm -f -- $(SIXAXIS_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(SIXAXIS_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/io/sixaxis/*.bak
	-rm -f -- $(externals_src)/io/sixaxis/*.*~



#------------------------------------------------------------------------------#
# SMLIB
smlib:
	make -C $(externals_src)/smlib

smlib_install:
	make -C $(externals_src)/smlib DESTDIR="$(DESTDIR)" objectsdir="$(objectsdir)" install

smlib_clean:
	make -C $(externals_src)/smlib clean


#------------------------------------------------------------------------------#
# TCLPD
TCLPD_NAME=tclpd
TCLPD_LIB := $(externals_src)/tclpd/tcl.$(EXTENSION)

$(TCLPD_LIB):
	cd $(externals_src)/tclpd && make

tclpd: $(TCLPD_LIB)

tclpd_install: tclpd
	install -d $(DESTDIR)$(objectsdir)
	install -p $(TCLPD_LIB) $(DESTDIR)$(objectsdir)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/tclpd/README \
		$(DESTDIR)$(readmesdir)/$(TCLPD_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(TCLPD_NAME)
#	install -p $(externals_src)/tclpd/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(TCLPD_NAME)

tclpd_clean:
	make -C $(externals_src)/tclpd clean
	-rm -f -- $(TCLPD_LIB)
	-rm -f -- $(externals_src)/tclpd/*.bak
	-rm -f -- $(externals_src)/tclpd/*.*~


#------------------------------------------------------------------------------#
# TKWIDGETS
TKWIDGETS_NAME=tkwidgets
TKWIDGETS_OBJECTS := $(wildcard $(externals_src)/tkwidgets/*.c)
TKWIDGETS_SHARED := $(wildcard $(externals_src)/tkwidgets/shared/*.c)
tkwidgets_shared: $(TKWIDGETS_SHARED:.c=.o)
#	$(CC) $(DYLIB_LDFLAGS) \
#		-o $(externals_src)/tkwidgets/libtkwidgets.$(DYLIB_EXTENSION) \
#		 $(TKWIDGETS_SHARED:.c=.o)

tkwidgets: tkwidgets_shared $(TKWIDGETS_OBJECTS:.c=.$(EXTENSION))

tkwidgets_install: tkwidgets
	install -d $(DESTDIR)$(objectsdir)/$(TKWIDGETS_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(TKWIDGETS_NAME) \
		--author "Hans-Christoph Steiner and other" \
		--description "a collection of objects based on Tk widgets" \
		--license "GPLv2 or later" 
	install -p $(TKWIDGETS_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(TKWIDGETS_NAME)
	install -d $(DESTDIR)$(helpdir)/$(TKWIDGETS_NAME)
	install -p $(externals_src)/tkwidgets/*.pd \
		$(DESTDIR)$(helpdir)/$(TKWIDGETS_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(TKWIDGETS_NAME)
#	install -p $(externals_src)/tkwidgets/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(TKWIDGETS_NAME)
#	install -d $(DESTDIR)$(readmesdir)
#	install -p $(externals_src)/tkwidgets/README \
#		$(DESTDIR)$(readmesdir)/$(TKWIDGETS_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(TKWIDGETS_NAME)
#	install -p $(externals_src)/tkwidgets/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(TKWIDGETS_NAME)

tkwidgets_clean:
	-rm -f -- $(TKWIDGETS_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(TKWIDGETS_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/tkwidgets/*.bak
	-rm -f -- $(externals_src)/tkwidgets/*.*~

#------------------------------------------------------------------------------#
# TOF
TOF_NAME=tof
TOF_OBJECTS := $(wildcard $(externals_src)/tof/src/*.c)
tof: $(TOF_OBJECTS:.c=.$(EXTENSION))

tof_install: tof
	install -d $(DESTDIR)$(objectsdir)/$(TOF_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(TOF_NAME) \
		--author "Thomas Ouellet Fredericks" \
		--description "Various utilities" \
		--license "Whatever Hans wants" \
		--version "2009-09-22"
	install -p $(TOF_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(TOF_NAME)
	install -d $(DESTDIR)$(helpdir)/$(TOF_NAME)
	install -p $(externals_src)/tof/help/*.pd \
		$(DESTDIR)$(helpdir)/$(TOF_NAME)
	install -p $(externals_src)/tof/help/*.pd \
		$(DESTDIR)$(objectsdir)/$(TOF_NAME)
	install -p $(externals_src)/tof/abs/*.pd \
		$(DESTDIR)$(objectsdir)/$(TOF_NAME)

tof_clean:
	-rm -f -- $(TOF_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(TOF_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/tof/src/*.bak
	-rm -f -- $(externals_src)/tof/src/*.*~

#------------------------------------------------------------------------------#
# TOXY
TOXY_NAME=toxy
# toxy is compiled straight into $(OUT_DIR)
TOXY_OUT_DIR=$(DESTDIR)$(objectsdir)/$(TOXY_NAME)
toxy:


toxy_install: toxy
	install -d $(DESTDIR)$(objectsdir)/$(TOXY_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(TOXY_NAME) \
		--author "Kzrysztof Czaja" \
		--license "BSD" \
		--description "objects for working with Tcl and Pd's Tk GUI"
	cd $(externals_src)/miXed/toxy && $(MAKE) OUT_DIR=$(TOXY_OUT_DIR)
	install -d $(DESTDIR)$(helpdir)/$(TOXY_NAME)
	install -p $(externals_src)/miXed/doc/help/toxy/*.*  \
		$(DESTDIR)$(helpdir)/$(TOXY_NAME)
	install -p $(externals_src)/miXed/doc/help/toxy/*.*  \
		$(DESTDIR)$(objectsdir)/$(TOXY_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(TOXY_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(TOXY_NAME)
	install -p $(externals_src)/miXed/test/toxy/*.*  \
		$(DESTDIR)$(examplesdir)/$(TOXY_NAME)


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
	install -d $(DESTDIR)$(objectsdir)/$(UNAUTHORIZED_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(UNAUTHORIZED_NAME) \
		--author "Yves Degoyon" \
		--license "GNU GPL" \
		--description "GUI and streaming objects"
	install -p $(externals_src)/unauthorized/*/*.$(EXTENSION) \
		$(DESTDIR)$(objectsdir)/$(UNAUTHORIZED_NAME)
	install -d $(DESTDIR)$(helpdir)/$(UNAUTHORIZED_NAME)
	install -p $(externals_src)/unauthorized/*/*.pd \
		$(DESTDIR)$(helpdir)/$(UNAUTHORIZED_NAME)
	install -p $(externals_src)/unauthorized/*/*.pd \
		$(DESTDIR)$(objectsdir)/$(UNAUTHORIZED_NAME)
	install -p $(externals_src)/unauthorized/*/*.pls \
		$(DESTDIR)$(helpdir)/$(UNAUTHORIZED_NAME)
	install -d $(DESTDIR)$(helpdir)/$(UNAUTHORIZED_NAME)/blm
	install -p $(externals_src)/unauthorized/blinkenlights/blm/*.* \
		$(DESTDIR)$(helpdir)/$(UNAUTHORIZED_NAME)/blm
	install -d $(DESTDIR)$(manualsdir)/$(UNAUTHORIZED_NAME)
	install -p $(externals_src)/unauthorized/*/*.txt \
		$(DESTDIR)$(manualsdir)/$(UNAUTHORIZED_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(USBHID_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(USBHID_NAME) \
		--author "Hans-Christoph Steiner" \
		--description "pure USB HID object for accessing HID devices" \
		--license "GNU GPL" \
		--version ""
	install -p $(USBHID_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(USBHID_NAME)
	install -d $(DESTDIR)$(helpdir)/$(USBHID_NAME)
	install -p $(externals_src)/hcs/usbhid/*-help.pd \
		$(DESTDIR)$(helpdir)/$(USBHID_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(USBHID_NAME)
#	install -p $(externals_src)/usbhid/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(USBHID_NAME)
#	install -d $(DESTDIR)$(readmesdir)
#	install -p $(externals_src)/usbhid/README \
#		$(DESTDIR)$(readmesdir)/$(USBHID_NAME).txt
#	install -d $(DESTDIR)$(examplesdir)/$(USBHID_NAME)
#	install -p $(externals_src)/usbhid/examples/*.pd \
#		$(DESTDIR)$(examplesdir)/$(USBHID_NAME)

usbhid_clean:
	-rm -f -- $(USBHID_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(USBHID_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/usbhid/*.bak
	-rm -f -- $(externals_src)/usbhid/*.*~



#------------------------------------------------------------------------------#
# VBAP
VBAP_NAME=vbap
VBAP_OBJECTS := $(wildcard $(externals_src)/vbap/*.c)
vbap: $(VBAP_OBJECTS:.c=.$(EXTENSION))

vbap_install: vbap
	install -d $(DESTDIR)$(objectsdir)/$(VBAP_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(VBAP_NAME) \
		--author "Ville Pulkki" \
		--description "Vector Based Amplitude Panning" \
		--version ""
	install -p $(VBAP_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(VBAP_NAME)
	install -p $(shell ls -1 $(externals_src)/vbap/*.pd | grep -v '-help.pd') \
		 $(DESTDIR)$(objectsdir)/$(VBAP_NAME)
	install -d $(DESTDIR)$(helpdir)/$(VBAP_NAME)
	install -p $(externals_src)/vbap/*-help.pd $(DESTDIR)$(helpdir)/$(VBAP_NAME)
	install -p $(externals_src)/vbap/*-help.pd $(DESTDIR)$(objectsdir)/$(VBAP_NAME)
#	install -d $(DESTDIR)$(manualsdir)/$(VBAP_NAME)
#	install -p $(externals_src)/vbap/manual.txt \
#		$(DESTDIR)$(manualsdir)/$(VBAP_NAME)
	install -d $(DESTDIR)$(examplesdir)/$(VBAP_NAME)
	install -p $(externals_src)/vbap/*-demo.pd \
		$(DESTDIR)$(examplesdir)/$(VBAP_NAME)

vbap_clean:
	-rm -f -- $(VBAP_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(VBAP_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/vbap/*.bak
	-rm -f -- $(externals_src)/vbap/*.*~


#------------------------------------------------------------------------------#
# VANILLA
vanilla:
	make -C $(externals_src)/vanilla

vanilla_install:
	make -C $(externals_src)/vanilla DESTDIR="$(DESTDIR)" objectsdir="$(objectsdir)" install

vanilla_clean:
	make -C $(externals_src)/vanilla clean


#------------------------------------------------------------------------------#
# WINDOWING
windowing:
	make -C $(externals_src)/windowing

windowing_install:
	make -C $(externals_src)/windowing DESTDIR="$(DESTDIR)" objectsdir="$(objectsdir)" install

windowing_clean:
	make -C $(externals_src)/windowing clean


#------------------------------------------------------------------------------#
# XBEE
# this is installed into the "io" library
XBEE_NAME=io
XBEE_SRC := $(wildcard $(externals_src)/io/xbee/*.c)

XBEE_OBJECTS := $(XBEE_SRC:.c=.o)
$(XBEE_OBJECTS) : %.o : %.c
	$(CC) $(CFLAGS) -o "$*.o" -c "$*.c"

$(externals_src)/io/xbee/xbee_test.$(EXTENSION): $(XBEE_OBJECTS) 
	$(CC) $(LDFLAGS) -o $(externals_src)/io/xbee/xbee_test.$(EXTENSION) \
		$(XBEE_OBJECTS)
#	$(STRIP) $(externals_src)/io/xbee/xbee_test.$(EXTENSION)

xbee: $(externals_src)/io/xbee/xbee_test.$(EXTENSION)

xbee_install: xbee
	install -d $(DESTDIR)$(objectsdir)/$(XBEE_NAME)
	install -p $(externals_src)/io/xbee/xbee_test.$(EXTENSION) \
		$(DESTDIR)$(objectsdir)/$(XBEE_NAME)
	install -d $(DESTDIR)$(helpdir)/$(XBEE_NAME)
	install -p $(externals_src)/io/xbee/*.pd \
		$(DESTDIR)$(helpdir)/$(XBEE_NAME)
	install -d $(DESTDIR)$(manualsdir)/$(XBEE_NAME)
	install -p $(externals_src)/io/xbee/*.txt \
		$(DESTDIR)$(manualsdir)/$(XBEE_NAME)

xbee_clean:
	-rm -f -- $(externals_src)/io/xbee/xbee_test.$(EXTENSION)
	-rm -f -- $(XBEE_OBJECTS:.c=.o)
	-rm -f -- $(externals_src)/io/xbee/*.bak
	-rm -f -- $(externals_src)/io/xbee/*.*~


#------------------------------------------------------------------------------#
# Zexy
ZEXY_NAME = zexy
ZEXY_OBJECTS := $(wildcard $(externals_src)/zexy/src/[a-vxy0]*.c) \
$(externals_src)/zexy/src/wrap.c $(externals_src)/zexy/src/z~.c
ZEXY_VERSION := $(shell grep VERSION $(externals_src)/zexy/src/zexy.h | \
	grep -v _VERSION | cut -d ' ' -f 3 | cut -d '"' -f 2)

# ADAPTED FROM IEM16 - previous script broke builds.
ZEXY_DIR := $(externals_src)/zexy/src

$(ZEXY_DIR)/aclocal.m4: $(ZEXY_DIR)/acinclude.m4
	cd $(ZEXY_DIR) && aclocal

$(ZEXY_DIR)/configure: $(ZEXY_DIR)/configure.ac $(ZEXY_DIR)/aclocal.m4
	cd $(ZEXY_DIR) && autoconf

$(ZEXY_DIR)/Make.config: $(ZEXY_DIR)/Make.config.in \
$(ZEXY_DIR)/configure
	cd $(ZEXY_DIR) && ./configure --disable-library \
		--with-pd=$(pd_src)

zexy: $(ZEXY_DIR)/Make.config
	$(MAKE) -C $(ZEXY_DIR)

zexy_install: zexy
	install -d $(DESTDIR)$(objectsdir)/$(ZEXY_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(ZEXY_NAME) \
		--author "IOhannes m zmoelnig <zmoelnig@iem.kug.ac.at>" \
		--license "GNU GPL" \
		--description "swiss army knife for pd" \
		--version "$(ZEXY_VERSION)"
	install -p $(ZEXY_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(ZEXY_NAME)
# the below doesn't work for some reason, probably because of some kind of
# strange make execution order <hans@at.or.at>
#	install -p $(shell $(externals_src)/zexy/src/*.$(EXTENSION)) \
#		$(DESTDIR)$(objectsdir)/$(ZEXY_NAME)
	install -p $(shell ls -1 $(externals_src)/zexy/abs/*.pd | \
		grep -v '-help.pd' | sed 's|\(.*\)|"\1"|g') \
		$(DESTDIR)$(objectsdir)/$(ZEXY_NAME)
# an escape pattern for the abstractions with special characters
#		grep -v '-help.pd' | sed 's/\([&<>|~]\)/\\\1/g') 
	install -d $(DESTDIR)$(helpdir)/$(ZEXY_NAME)
	install -p $(shell ls -1 $(externals_src)/zexy/abs/*-help.pd | \
		sed 's|\(.*\)|"\1"|g') $(DESTDIR)$(helpdir)/$(ZEXY_NAME)
	install -p $(shell ls -1 $(externals_src)/zexy/reference/*-help.pd | \
		sed 's|\(.*\)|"\1"|g') $(DESTDIR)$(helpdir)/$(ZEXY_NAME)
	install -p $(externals_src)/zexy/zexy.pd  $(DESTDIR)$(helpdir)

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
	install -d $(DESTDIR)$(objectsdir)/$(PDCONTAINER_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(PDCONTAINER_NAME) \
		--author "Georg Holzmann" \
		--description "container objects of the C++ STL for Pd" \
		--license "GPL" \
		--version ""
	install -p $(PDCONTAINER_TARGETS) $(DESTDIR)$(objectsdir)/$(PDCONTAINER_NAME)
	install -d $(DESTDIR)$(helpdir)/$(PDCONTAINER_NAME)
	install -p $(externals_src)/grh/PDContainer/help/*.pd \
		$(DESTDIR)$(helpdir)/$(PDCONTAINER_NAME)
	install -p $(externals_src)/grh/PDContainer/help/*.pd \
		$(DESTDIR)$(objectsdir)/$(PDCONTAINER_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/grh/PDContainer/readme.txt \
		$(DESTDIR)$(readmesdir)/$(PDCONTAINER_NAME).txt

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
	install -d $(DESTDIR)$(objectsdir)/$(ADAPTIVE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(ADAPTIVE_NAME) \
		--author "Georg Holzmann, Gerda Strobl" \
		--description "library for adaptive systems and filters" \
		--license "GNU GPL" \
		--version ""
	install -p $(ADAPTIVE_TARGETS) $(DESTDIR)$(objectsdir)/$(ADAPTIVE_NAME)
	install -d $(DESTDIR)$(helpdir)/$(ADAPTIVE_NAME)
	install -p $(externals_src)/grh/adaptive/doc/*.pd \
		$(DESTDIR)$(helpdir)/$(ADAPTIVE_NAME)
	install -p $(externals_src)/grh/adaptive/doc/*.pd \
		$(DESTDIR)$(objectsdir)/$(ADAPTIVE_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/grh/adaptive/readme \
		$(DESTDIR)$(readmesdir)/$(ADAPTIVE_NAME).txt
	install -d $(DESTDIR)$(examplesdir)/$(ADAPTIVE_NAME)
	install -p $(externals_src)/grh/adaptive/examples/*.pd \
		$(externals_src)/grh/adaptive/examples/*.dat \
		$(DESTDIR)$(examplesdir)/$(ADAPTIVE_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(IEMADAPT_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(IEMADAPT_NAME) \
		--author "Markus Noisternig, Thomas Musil" \
		--description "several algorithms for adaptive filters" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEMADAPT_SRC:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(IEMADAPT_NAME)
	install -d $(DESTDIR)$(helpdir)/$(IEMADAPT_NAME)
	install -p $(externals_src)/iem/iem_adaptfilt/help/*.pd \
		$(DESTDIR)$(helpdir)/$(IEMADAPT_NAME)
	install -p $(externals_src)/iem/iem_adaptfilt/help/*.pd \
		$(DESTDIR)$(objectsdir)/$(IEMADAPT_NAME)
	install -d $(DESTDIR)$(manualsdir)/$(IEMADAPT_NAME)
	install -p $(externals_src)/iem/iem_adaptfilt/doc/adapt_filt_lib.pdf \
		$(DESTDIR)$(manualsdir)/$(IEMADAPT_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(IEMDELAY_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(IEMDELAY_NAME) \
		--author "Thomas Musil" \
		--description "various delay objects" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEMDELAY_SRC:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(IEMDELAY_NAME)
	install -d $(DESTDIR)$(helpdir)/$(IEMDELAY_NAME)
	install -p $(externals_src)/iem/iem_delay/*.pd \
		$(DESTDIR)$(helpdir)/$(IEMDELAY_NAME)
	install -p $(externals_src)/iem/iem_delay/*.pd \
		$(DESTDIR)$(objectsdir)/$(IEMDELAY_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/iem/iem_delay/READ_ME.txt \
		$(DESTDIR)$(readmesdir)/$(IEMDELAY_NAME).txt

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
	install -d $(DESTDIR)$(objectsdir)/$(IEMROOM_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(IEMROOM_NAME) \
		--author "Thomas Musil" \
		--description "objects for room simulation" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEMROOM_SRC:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(IEMROOM_NAME)
	install -d $(DESTDIR)$(helpdir)/$(IEMROOM_NAME)
	install -p $(externals_src)/iem/iem_roomsim/*.pd \
		$(DESTDIR)$(helpdir)/$(IEMROOM_NAME)
	install -p $(externals_src)/iem/iem_roomsim/*.pd \
		$(DESTDIR)$(objectsdir)/$(IEMROOM_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/iem/iem_roomsim/READ_ME.txt \
		$(DESTDIR)$(readmesdir)/$(IEMROOM_NAME).txt

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
	install -d $(DESTDIR)$(objectsdir)/$(IEMSPEC2_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(IEMSPEC2_NAME) \
		--author "Thomas Musil" \
		--description "special spectral processing objects, which only calculates blocksize/2 + 1 samples of a signal" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEMSPEC2_SRC:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(IEMSPEC2_NAME)
	install -d $(DESTDIR)$(helpdir)/$(IEMSPEC2_NAME)
	install -p $(externals_src)/iem/iem_spec2/*.pd \
		$(DESTDIR)$(helpdir)/$(IEMSPEC2_NAME)
	install -p $(externals_src)/iem/iem_spec2/*.pd \
		$(DESTDIR)$(objectsdir)/$(IEMSPEC2_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/iem/iem_spec2/READ_ME.txt \
		$(DESTDIR)$(readmesdir)/$(IEMSPEC2_NAME).txt

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
	install -d $(DESTDIR)$(objectsdir)/$(IEMTAB_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(IEMTAB_NAME) \
		--author "Thomas Musil" \
		--description "library to manipulate tables or arrays" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEMTAB_TARGETS) $(DESTDIR)$(objectsdir)/$(IEMTAB_NAME)
	install -d $(DESTDIR)$(helpdir)/$(IEMTAB_NAME)
	install -p $(externals_src)/iem/iem_tab/*.pd \
		$(DESTDIR)$(helpdir)/$(IEMTAB_NAME)
	install -p $(externals_src)/iem/iem_tab/*.pd \
		$(DESTDIR)$(objectsdir)/$(IEMTAB_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/iem/iem_tab/READ_ME.txt \
		$(DESTDIR)$(readmesdir)/$(IEMTAB_NAME).txt

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
	install -d $(DESTDIR)$(objectsdir)/$(FLASHSERVER_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(FLASHSERVER_NAME) \
		--author "Olaf Matthes" \
		--description "a flashserver for multiple clients" \
		--license "GNU GPL" \
		--version ""
	install -p $(FLASHSERVER_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(FLASHSERVER_NAME)
	install -d $(DESTDIR)$(helpdir)/$(FLASHSERVER_NAME)
	install -p $(externals_src)/olafmatt/flashserver/*.pd \
		$(DESTDIR)$(helpdir)/$(FLASHSERVER_NAME)
	install -p $(externals_src)/olafmatt/flashserver/*.pd \
		$(DESTDIR)$(objectsdir)/$(FLASHSERVER_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/olafmatt/flashserver/README \
		$(DESTDIR)$(readmesdir)/$(FLASHSERVER_NAME).txt

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
	install -d $(DESTDIR)$(objectsdir)/$(HDSPM_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(HDSPM_NAME) \
		--author "Winfried Ritsch" \
		--description "pd-mixer for RME hammerfall devices" \
		--license "GNU GPL" \
		--version ""
	install -p $(HDSPM_TARGET) $(DESTDIR)$(objectsdir)/$(HDSPM_NAME)
	install -d $(DESTDIR)$(helpdir)/$(HDSPM_NAME)
	install -p $(externals_src)/iem/hdspm_mixer/*.pd \
		$(DESTDIR)$(helpdir)/$(HDSPM_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(IEMGUI_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(IEMGUI_NAME) \
		--author "Thomas Musil" \
		--description "some more GUI objects for PD" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEMGUI_TARGETS) $(DESTDIR)$(objectsdir)/$(IEMGUI_NAME)
	install -d $(DESTDIR)$(helpdir)/$(IEMGUI_NAME)
	install -p $(externals_src)/iem/iemgui/*.pd \
		$(DESTDIR)$(helpdir)/$(IEMGUI_NAME)
	install -p $(externals_src)/iem/iemgui/*.pd \
		$(DESTDIR)$(objectsdir)/$(IEMGUI_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/iem/iemgui/READ_ME.txt \
		$(DESTDIR)$(readmesdir)/$(IEMGUI_NAME).txt

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
	install -d $(DESTDIR)$(objectsdir)/$(IEMXMLRPC_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(IEMXMLRPC_NAME) \
		--author "Thomas Grill, Winfried Ritsch" \
		--description "XMLRPC external for PD" \
		--license "GNU GPL" \
		--version ""
	install -p $(IEMXMLRPC_TARGET) $(DESTDIR)$(objectsdir)/$(IEMXMLRPC_NAME)
	install -d $(DESTDIR)$(helpdir)/$(IEMXMLRPC_NAME)
	install -p $(externals_src)/iem/iemxmlrpc/*.pd \
		$(DESTDIR)$(helpdir)/$(IEMXMLRPC_NAME)
	install -p $(externals_src)/iem/iemxmlrpc/*.pd \
		$(DESTDIR)$(objectsdir)/$(IEMXMLRPC_NAME)
	install -d $(DESTDIR)$(readmesdir)
	install -p $(externals_src)/iem/iemxmlrpc/README.txt \
		$(DESTDIR)$(readmesdir)/$(IEMXMLRPC_NAME).txt
	install -d $(DESTDIR)$(examplesdir)/$(IEMXMLRPC_NAME)
	install -p $(externals_src)/iem/iemxmlrpc/*.pd \
                   $(externals_src)/iem/iemxmlrpc/*.py \
		   $(DESTDIR)$(examplesdir)/$(IEMXMLRPC_NAME)

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
	install -d $(DESTDIR)$(objectsdir)/$(EARPLUG_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(DESTDIR)$(objectsdir) $(EARPLUG_NAME) \
		--author "Pei Xiang" \
		--description "filters for binaural audio spatialization, based on KEMAR impulse measurement" \
		--license "" \
		--version ""
	install -p $(EARPLUG_OBJECTS:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(EARPLUG_NAME)
	install -p $(externals_src)/earplug~/earplug_data.txt $(DESTDIR)$(objectsdir)/$(EARPLUG_NAME)
	install -d $(DESTDIR)$(helpdir)/$(EARPLUG_NAME)
	install -p $(externals_src)/earplug~/*-help.pd \
		$(externals_src)/earplug~/*.txt \
		$(DESTDIR)$(helpdir)/$(EARPLUG_NAME)
	install -p $(externals_src)/earplug~/*-help.pd \
		$(externals_src)/earplug~/*.txt \
		$(DESTDIR)$(objectsdir)/$(EARPLUG_NAME)

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

TAGS: etags

etags:
	etags $(pd_src)/src/*.[ch] 
	find $(externals_src) -type f -name '*.[ch]' -exec etags -a '{}' \;
	find $(externals_src) -type f -name '*.cc' -exec etags -a '{}' \;
	find $(externals_src) -type f -name '*.cpp' -exec etags -a '{}' \;
	find $(extensions_src) -type f -name '*.[ch]' -exec etags -a '{}' \;
	find /usr/include -type f -name \*.h -exec etags -a '{}' \;
	make etags_`uname -s`

etags_Darwin:
	etags -a $(externals_src)/hcs/hid/HID\ Utilities\ Source/*.[ch]
	find /System/Library/Frameworks  -type f -name \*.h -exec etags -a '{}' \;
	find /Library/Frameworks  -type f -name \*.h -exec etags -a '{}' \;
	find /sw/include -type f -name \*.h -exec etags -a '{}' \;

etags_Linux:

etags_MINGW:
	find /usr/local/include/ -type f -name \*.h -exec etags -a '{}' \;


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


