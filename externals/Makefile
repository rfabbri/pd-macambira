#==============================================================================#
#
# Centralized build system for "externals".  
#
# see README for instructions  <hans@at.or.at>
#
#==============================================================================#

CWD := $(shell pwd)

# these are setup to be overridden by the packages/Makefile
cvs_root_dir = $(CWD)/..
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

OPT_CFLAGS = -O3 -funroll-loops -fomit-frame-pointer

# this variable is to support old "win" directories, rather than "windows"
BUILDSRC_OS_NAME = $(OS_NAME)

CFLAGS = -DPD $(OPT_CFLAGS) -I$(pd_src)/src -Wall -W
LDFLAGS =  
LIBS = -lm

ifeq ($(OS_NAME),darwin)
  CFLAGS += -I/sw/include -DMACOSX -DUNIX -Dunix
  LDFLAGS += -bundle -bundle_loader $(pd_src)/bin/pd -L/sw/lib
  LIBS += -lc 
  DYLIB_LDFLAGS = -dynamiclib -read_only_relocs warning -L/sw/lib
  DYLIB_EXTENSION = dylib
  STRIP = strip -x
else
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
    LIBS += -L$(pd_src)/bin -lpd \
      -lwsock32 -lpthreadGC2 -lkernel32 -luser32 -lgdi32 -lregex
    DYLIB_LDFLAGS = -shared
    DYLIB_EXTENSION = so
    STRIP = strip --strip-unneeded -R .note -R .comment
  else
    CFLAGS +=  -DUNIX -Dunix -fPIC
    LDFLAGS += -Wl,-export_dynamic  -shared
    LIBS += -lc
    DYLIB_LDFLAGS = $(LDFLAGS)
    DYLIB_EXTENSION = dll
    STRIP = strip --strip-unneeded -R .note -R .comment
  endif
endif

CXXFLAGS = $(CFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -o "$*.o" -c "$*.c"

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o "$*.o" -c "$*.cpp"

%.o: %.cc
	$(CXX) $(CXXFLAGS) -o "$*.o" -c "$*.cc"

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
LIB_TARGETS = buildsrc creb cxc cyclone ext13 freeverb hardware iemabs iemlib \
iemmatrix loaders mapping markex maxlib mjlib motex oscx pddp pdogg pmpd \
sigpack smlib toxy unauthorized vbap zexy

# this is for libraries that don't compile (yet) on all platforms
ifeq ($(OS_NAME),windows)
  LIB_TARGETS += 
else
  ifeq ($(OS_NAME),darwin)
    LIB_TARGETS += hid
  else
    # GNU/Linux, BSD, IRIX, etc.
    LIB_TARGETS += hid pdp pidip
  endif
endif


all: pre_all_$(OS_NAME) buildsrc $(LIB_TARGETS)
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
install: $(examplesdir) $(helpdir) $(manualsdir) $(objectsdir) \
all unfiltered_install $(patsubst %, %_install,$(LIB_TARGETS))
	@echo " "
	@echo "externals install succeeded!"


#==============================================================================#
#
# OLD EXTERNALS BUILD SYSTEM TARGETS
#
# this is all stuff related to the externals/build/src/*.c links
#
#==============================================================================#


BUILDSRC_OBJECTS := $(wildcard $(externals_src)/build/src/*.c)

buildsrc: $(BUILDSRC_OBJECTS:.c=.$(EXTENSION))


buildsrc_install: $(helpdir) $(objectsdir)
	install -p $(BUILDSRC_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)
# all standard objs' help files
#    it had to be broken up because the list is soo long
	install -p \
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
			$(externals_src)/hcs/*-help.pd \
				$(helpdir)
	install -p \
			$(externals_src)/hcs/*/doc/*.pd \
			$(externals_src)/hcs/pan/*-help.pd \
			$(externals_src)/iem/comport/*/*-help.pd \
			$(externals_src)/nusmuk/line3/*.pd\
				$(helpdir)
	install -p \
			$(externals_src)/pdogg/*/*.pd \
			$(externals_src)/pdp/doc/objects/*.* \
			$(externals_src)/pidip/doc/*.pd \
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


buildsrc_clean:
	-rm -f -- $(BUILDSRC_OBJECTS:.c=.$(EXTENSION))
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
	install -d $(manualsdir)/$(TEMPLATE_NAME)
	install -p $(externals_src)/template/README \
		$(manualsdir)/$(TEMPLATE_NAME)/README.txt
	install -d $(examplesdir)/$(TEMPLATE_NAME)
	install -p $(externals_src)/template/examples/*.pd \
		$(examplesdir)/$(TEMPLATE_NAME)

template_clean:
	-rm -f -- $(externals_src)/template/*.$(EXTENSION)
	-rm -f -- $(externals_src)/template/*.o
	-rm -f -- $(externals_src)/template/*.bak
	-rm -f -- $(externals_src)/template/*.*~


#------------------------------------------------------------------------------#
# BSAYLOR
BSAYLOR_NAME=bsaylor
BSAYLOR_OBJECTS := $(wildcard $(externals_src)/bsaylor/*~.c)
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
	install -d $(manualsdir)/$(BSAYLOR_NAME)
	install -p $(externals_src)/bsaylor/README.txt \
		$(manualsdir)/$(BSAYLOR_NAME)/README.txt
#	install -d $(examplesdir)/$(BSAYLOR_NAME)
#	install -p $(externals_src)/bsaylor/examples/*.pd \
#		$(examplesdir)/$(BSAYLOR_NAME)

bsaylor_clean:
	-rm -f -- $(externals_src)/bsaylor/*.$(EXTENSION)
	-rm -f -- $(externals_src)/bsaylor/*.o
	-rm -f -- $(externals_src)/bsaylor/*.bak
	-rm -f -- $(externals_src)/bsaylor/*.*~



#------------------------------------------------------------------------------#
# CREB
CREB_NAME=creb
#CREB_OBJECTS := $(wildcard $(externals_src)/creb/modules/*.c)
#CREB_CXXOBJECTS := $(wildcard $(externals_src)/creb/modules++/*.cc)
#creb: $(CREB_CXXOBJECTS:.cc=.$(EXTENSION)) $(CREB_OBJECTS:.c=.$(EXTENSION))
creb:


creb_install: creb
	install -d $(objectsdir)/$(CREB_NAME)
	install -p $(externals_src)/creb/abs/*.pd $(objectsdir)/$(CREB_NAME)
	install -d $(helpdir)/$(CREB_NAME)
	install -p $(externals_src)/creb/doc/examples/*.* \
		$(helpdir)/$(CREB_NAME)
#	install -d $(manualsdir)/$(CREB_NAME)
#	install -d $(examplesdir)/$(CREB_NAME)

creb_clean:
	-rm -f -- $(externals_src)/creb/*.$(EXTENSION)
	-rm -f -- $(externals_src)/creb/*.o
	-rm -f -- $(externals_src)/creb/*.bak
	-rm -f -- $(externals_src)/creb/*.*~


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
	install -d $(manualsdir)/$(CXC_NAME)
	install -p $(externals_src)/cxc/README \
		 $(manualsdir)/$(CXC_NAME)/README.txt

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
	-make -C $(externals_src)/miXed/cyclone $(DEST_PATHS) clean



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
	install -d $(manualsdir)/$(EXT13_NAME)
	install -p $(externals_src)/ext13/README \
		$(manualsdir)/$(EXT13_NAME)/README.txt

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
	install -d $(manualsdir)/$(FFTEASE_NAME)
	install -p $(externals_src)/fftease/README \
		$(manualsdir)/$(FFTEASE_NAME)/README.txt
	install -d $(examplesdir)/$(FFTEASE_NAME)
	install -p $(externals_src)/fftease/examples/*.pd \
		$(examplesdir)/$(FFTEASE_NAME)

fftease_clean:
	-rm -f -- $(externals_src)/fftease/*.$(EXTENSION)
	-rm -f -- $(externals_src)/fftease/*.o
	-rm -f -- $(externals_src)/fftease/*.bak
	-rm -f -- $(externals_src)/fftease/*.*~



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
	install -d $(objectsdir)
#	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(FREEVERB_NAME) \
#		--author "Olaf Matthes and Jezar Wakefield" \
#		--description "Freeverb is a simple implementation of the standard Schroeder/Moorer reverb model" \
#		--license "GNU GPL" \
#		--version "1.2"
	install -p $(FREEVERB_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)
	install -d $(helpdir)
	install -p $(wildcard $(externals_src)/freeverb~/*-help.pd) $(helpdir)
	install -d $(manualsdir)/$(FREEVERB_NAME)
	install -p $(externals_src)/freeverb~/README \
		$(manualsdir)/$(FREEVERB_NAME)/README.txt


freeverb_clean:
	-rm -f -- $(externals_src)/freeverb/*.$(EXTENSION)
	-rm -f -- $(externals_src)/freeverb/*.o
	-rm -f -- $(externals_src)/freeverb/*.bak
	-rm -f -- $(externals_src)/freeverb/*.*~



#------------------------------------------------------------------------------#
# HARDWARE
HARDWARE_NAME=hardware
HARDWARE_OBJECTS := $(wildcard $(externals_src)/hardware/*/*.cpp)
# multio.c requires libusb, so don't include it yet...  <hans@at.or.at>
#$(wildcard $(externals_src)/hardware/*/*.c)
hardware: $(HARDWARE_OBJECTS:.c=.$(EXTENSION)) $(HARDWARE_OBJECTS:.cpp=.$(EXTENSION))

hardware_install: hardware
	install -d $(objectsdir)/$(HARDWARE_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(HARDWARE_NAME) \
		--description "objects for working with hardware sensor boxes" \
		--license "GNU GPL"
	install -p $(HARDWARE_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(HARDWARE_NAME)
	install -d $(helpdir)/$(HARDWARE_NAME)
	install -p $(externals_src)/hardware/*/*-help.pd \
		$(helpdir)/$(HARDWARE_NAME)
	install -d $(manualsdir)/$(HARDWARE_NAME)
	install -p $(externals_src)/hardware/README.txt \
		$(manualsdir)/$(HARDWARE_NAME)/README.txt
	install -p $(externals_src)/hardware/memPIO/README.txt \
		$(manualsdir)/$(HARDWARE_NAME)/memPIO.txt
	install -d $(examplesdir)/$(HARDWARE_NAME)
	install -p $(externals_src)/hardware/*/examples/*.pd \
		$(examplesdir)/$(HARDWARE_NAME)

hardware_clean:
	-rm -f -- $(externals_src)/hardware/*/*.$(EXTENSION)
	-rm -f -- $(externals_src)/hardware/*/*.o
	-rm -f -- $(externals_src)/hardware/*/*.bak
	-rm -f -- $(externals_src)/hardware/*/*.*~



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

$(HID_UTILITIES_SOURCE)/build/libHIDUtilities.a:
# Apple changed the XCode CLI tool's name in xcode2... arg
	cd $(HID_UTILITIES_SOURCE) && \
		(test -x /usr/bin/xcodebuild && /usr/bin/xcodebuild) || pbxbuild


$(externals_src)/hcs/hid/hid.$(EXTENSION): $(HID_OBJECTS) \
$(HID_UTILITIES_SOURCE)/build/libHIDUtilities.a
	$(CC) $(LDFLAGS) -o $(externals_src)/hcs/hid/hid.$(EXTENSION) \
		$(HID_OBJECTS) $(HID_LIBS)
	$(STRIP) $(externals_src)/hcs/hid/hid.$(EXTENSION)

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
	-rm -f -- $(externals_src)/hcs/hid/*.$(EXTENSION)
	-rm -f -- $(externals_src)/hcs/hid/*.bak
	-rm -f -- $(externals_src)/hcs/hid/*.*~


#------------------------------------------------------------------------------#
# HIDIN
HIDIN_NAME=hidin
HIDIN_OBJECTS := $(wildcard $(externals_src)/hidin/*.c)
hidin: $(HIDIN_OBJECTS:.c=.$(EXTENSION))

hidin_install: hidin
	install -d $(objectsdir)/$(HIDIN_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(HIDIN_NAME) \
		--author "" \
		--description "" \
		--license "" \
		--version ""
	install -p $(HIDIN_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(HIDIN_NAME)
	install -d $(helpdir)/$(HIDIN_NAME)
	install -p $(externals_src)/hidin/help/*.pd \
		$(helpdir)/$(HIDIN_NAME)
	install -d $(manualsdir)/$(HIDIN_NAME)
	install -p $(externals_src)/hidin/README \
		$(manualsdir)/$(HIDIN_NAME)/README.txt
	install -d $(examplesdir)/$(HIDIN_NAME)
	install -p $(externals_src)/hidin/examples/*.pd \
		$(examplesdir)/$(HIDIN_NAME)

hidin_clean:
	-rm -f -- $(externals_src)/hidin/*.$(EXTENSION)
	-rm -f -- $(externals_src)/hidin/*.o
	-rm -f -- $(externals_src)/hidin/*.bak
	-rm -f -- $(externals_src)/hidin/*.*~


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
# LOADERS
LOADERS_NAME=
LOADERS_OBJECTS := $(wildcard $(externals_src)/loaders/*.c)
loaders: $(LOADERS_OBJECTS:.c=.$(EXTENSION))

loaders_install: loaders
	install -d $(objectsdir)/$(LOADERS_NAME)
	install -p $(LOADERS_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(LOADERS_NAME)
	install -d $(helpdir)/$(LOADERS_NAME)
	install -p $(wildcard $(externals_src)/loaders/help/*.pd) \
		$(helpdir)/$(LOADERS_NAME)
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
	install -p $(externals_src)/mapping/pd/*.pd $(objectsdir)/$(MAPPING_NAME)
	install -d $(helpdir)/$(MAPPING_NAME)
	install -p $(externals_src)/mapping/help/*.pd \
		$(helpdir)/$(MAPPING_NAME)
	install -d $(manualsdir)/$(MAPPING_NAME)
	install -p $(externals_src)/mapping/README.txt \
		$(manualsdir)/$(MAPPING_NAME)/README.txt
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
	install -d $(manualsdir)/$(MARKEX_NAME)
	install -p $(externals_src)/markex/README \
		$(manualsdir)/$(MARKEX_NAME)/README.txt

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
	install -d $(manualsdir)/$(MAXLIB_NAME)
	install -p $(externals_src)/maxlib/README \
		$(manualsdir)/$(MAXLIB_NAME)/README.txt

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
	install -d $(manualsdir)/$(MJLIB_NAME)
	install -p $(externals_src)/mjlib/readme.txt \
		$(manualsdir)/$(MJLIB_NAME)

mjlib_clean:
	-rm -f -- $(externals_src)/mjlib/*.$(EXTENSION)
	-rm -f -- $(externals_src)/mjlib/*.o
	-rm -f -- $(externals_src)/mjlib/*.bak
	-rm -f -- $(externals_src)/mjlib/*.*~


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
	install -d $(manualsdir)/$(MOTEX_NAME)
	install -p $(externals_src)/motex/README \
		$(manualsdir)/$(MOTEX_NAME)/README.txt

motex_clean:
	-rm -f -- $(externals_src)/motex/*.$(EXTENSION)
	-rm -f -- $(externals_src)/motex/*.o
	-rm -f -- $(externals_src)/motex/*.bak
	-rm -f -- $(externals_src)/motex/*.*~



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
	make -C $(externals_src)/OSCx
$(externals_src)/OSCx/src/dumpOSC.$(EXTENSION):  $(externals_src)/OSCx/configure \
$(externals_src)/OSCx/Makefile
	make -C $(externals_src)/OSCx
$(externals_src)/OSCx/src/sendOSC.$(EXTENSION):  $(externals_src)/OSCx/configure \
$(externals_src)/OSCx/Makefile
	make -C $(externals_src)/OSCx

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
	-cd $(externals_src)/OSCx && make $(DEST_PATHS) clean
	-cd $(externals_src)/OSCx && $(MAKE) CC=gcc clean
	-rm $(externals_src)/OSCx/Makefile



#------------------------------------------------------------------------------#
# PDDP
PDDP_NAME=pddp
# pddp is compiled straight into $(OUT_DIR)
PDDP_OUT_DIR=$(objectsdir)/$(PDDP_NAME)
pddp:


pddp_install: pddp
	install -d $(objectsdir)/$(PDDP_NAME)
#	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(PDDP_NAME) \
#		--author "PDDP" \
#		--license "BSD" \
#		--description "objects for working with Tcl and Pd's Tk GUI"
#	cd $(externals_src)/miXed/pddp && $(MAKE) OUT_DIR=$(PDDP_OUT_DIR)
#	install -d $(helpdir)/$(PDDP_NAME)
#	install -d $(manualsdir)/$(PDDP_NAME)
	install -d $(examplesdir)/$(PDDP_NAME)
	install -p $(externals_src)/miXed/test/pddp/*.*  \
		$(examplesdir)/$(PDDP_NAME)


pddp_clean:
	-make -C $(externals_src)/mixed/pddp $(DEST_PATHS) clean



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
	install -d $(manualsdir)/$(PDOGG_NAME)
	install -p $(externals_src)/pdogg/readme \
		$(manualsdir)/$(PDOGG_NAME)/README.txt

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
	cd $(externals_src)/pdp && ./configure

$(externals_src)/pdp/pdp.$(EXTENSION):  $(externals_src)/pdp/configure \
$(externals_src)/pdp/Makefile.config 
	make -C $(externals_src)/pdp

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
	install -d $(helpdir)/$(PDP_NAME)
	install -p $(externals_src)/pdp/doc/objects/*.* $(helpdir)/$(PDP_NAME)
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
	-cd $(externals_src)/pidip && ./configure --with-pd=../../pd --with-pdp=../pdp

$(externals_src)/pidip/pidip.$(EXTENSION):  $(externals_src)/pidip/configure \
$(externals_src)/pidip/Makefile
	-make -C $(externals_src)/pidip

pidip: $(externals_src)/pidip/pidip.$(EXTENSION)

pidip_install:
	install -d $(objectsdir)/$(PIDIP_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(PIDIP_NAME) \
		--author "Yves Degoyon" \
		--description "PiDiP is Definitely in Pieces"
	-install -p $(externals_src)/pidip/*.$(EXTENSION) $(objectsdir)/$(PIDIP_NAME)
	install -d $(helpdir)/$(PIDIP_NAME)
	install -p $(externals_src)/pidip/doc/*.pd $(helpdir)/$(PIDIP_NAME)
	install -d $(examplesdir)/$(PIDIP_NAME)
	install -p $(externals_src)/pidip/patches/*.* $(examplesdir)/$(PIDIP_NAME)
	install -d $(manualsdir)/$(PIDIP_NAME)
	install -p $(externals_src)/pidip/README \
		$(manualsdir)/$(PIDIP_NAME)/README.txt
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
PMPD_OBJECTS := $(wildcard $(externals_src)/pmpd/src/[a-lq-z]*.c)
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
	-find $(externals_src)/pmpd -name '*.o' | xargs rm -f --
	-find $(externals_src)/pmpd -name '*.bak' | xargs rm -f --
	-find $(externals_src)/pmpd -name '*~' | xargs rm -f --


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
	install -d $(manualsdir)/$(SIGPACK_NAME)
	install -p $(externals_src)/sigpack/readme.txt \
		$(manualsdir)/$(SIGPACK_NAME)/README.txt

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
	install -d $(manualsdir)/$(SMLIB_NAME)
	install -p $(externals_src)/smlib/readme.txt $(manualsdir)/$(SMLIB_NAME)
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
	-make -C $(externals_src)/mixed/toxy $(DEST_PATHS) clean


#------------------------------------------------------------------------------#
# UNAUTHORIZED
UNAUTHORIZED_NAME=unauthorized
# these need to be created before compiling the C
UNAUTHORIZED_TKFILES = $(wildcard $(externals_src)/unauthorized/*/*.tk)
%.tk2c: %.tk
	bash $(externals_src)/unauthorized/tk2c.bash < $*.tk > $*.tk2c

# [cooled~] doesn't compile under MinGW (yet) and it crashes Pd on Mac OS X,
# so its excluded from the build here
UNAUTHORIZED_OBJECTS := $(wildcard $(externals_src)/unauthorized/[a-bd-uw-z]*/*.c) \
$(wildcard $(externals_src)/unauthorized/c?[a-np-z]*/*.c)

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
ZEXY_OBJECTS := $(wildcard $(externals_src)/zexy/src/[a-y0]*.c $(externals_src)/zexy/src/z~.c)
ZEXY_VERSION := $(shell grep VERSION $(externals_src)/zexy/src/zexy.h | \
	grep -v _VERSION | cut -d ' ' -f 3 | cut -d '"' -f 2)

zexy: $(ZEXY_OBJECTS:.c=.$(EXTENSION))

zexy_install: zexy
	install -d $(objectsdir)/$(ZEXY_NAME)
	$(scripts_src)/generate-libdir-metafile.sh $(objectsdir) $(ZEXY_NAME) \
		--author "IOhannes m zmoelnig <zmoelnig@iem.kug.ac.at>" \
		--license "GNU GPL" \
		--description "GUI and streaming objects" \
		--version "$(ZEXY_VERSION)"
	install -p $(ZEXY_OBJECTS:.c=.$(EXTENSION)) $(objectsdir)/$(ZEXY_NAME)
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
	-rm -f -- $(ZEXY_OBJECTS:.c=.$(EXTENSION))
	-rm -f -- $(externals_src)/zexy/src/*.o
	-rm -f -- $(externals_src)/zexy/src/*.bak
	-rm -f -- $(externals_src)/*/*.*~
	-rm -f -- $(externals_src)/*.*~



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
# general
	install -p $(externals_src)/hcs/general/*.pd $(objectsdir)
# pan
	install -p $(shell ls -1 $(externals_src)/hcs/pan/*.pd | \
			 grep -v '\-help.pd') $(objectsdir)


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
	-rm -f -- $(BUILDSRC_OBJECTS:.c=.o) 
	-rm -f -- $(BUILDSRC_OBJECTS:.c=.$(EXTENSION)) 


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
