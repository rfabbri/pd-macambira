TARGET=hid

default: 
	make -C ../../ $(TARGET)

install:
	make -C ../../ $(TARGET)_install

clean:
	make -C ../../ $(TARGET)_clean



# this stuff below probably works, but its not maintained anymore since I use
# externals/Makefile

CWD := $(shell pwd)

# these are setup to be overridden by the packages/Makefile
cvs_root_dir = $(CWD)/../../..
DESTDIR = $(CWD)/build/
BUILDLAYOUT_DIR = $(cvs_root_dir)/packages

-include $(BUILDLAYOUT_DIR)/Makefile.buildlayout



CFLAGS = $(OPT_FLAGS) -Wall -I./ -I../../../pd/src
LDFLAGS = 
LIBS = -lm

ifeq (x$(OS_NAME),x)
default:
	@echo no OS_NAME specified
endif


#SRC = $(wildcard $(externals_src)/hcs/hid/hid*.c)
SRC = $(wildcard *.c)
SRC = input_arrays.c hid_$(OS_NAME).c
OBJ := $(SRC:.c=.o)

# ----------------------- GNU/LINUX i386 -----------------------
ifeq ($(OS_NAME),linux)
  EXTENSION = pd_linux
  LDFLAGS += -export_dynamic -shared
  LIBS += -lc
  STRIP = strip --strip-unneeded
  hid.$(EXTENSION): input_arrays $(OBJ)
endif

# ----------------------- Windows MinGW -----------------------
ifeq ($(OS_NAME),win)
  EXTENSION = dll
  CFLAGS += -mms-bitfields
  LDFLAGS += -shared 
  LIBS += -lhid -lsetupapi -L../../../pd/bin -lpd
  STRIP = strip --strip-unneeded
  hid.$(EXTENSION): input_arrays $(OBJ)
endif

# ----------------------- DARWIN -----------------------
ifeq ($(OS_NAME),darwin)
  EXTENSION = pd_darwin
  CFLAGS +=  -I./HID\ Utilities\ Source
  PDEXECUTABLE = ../../../pd/bin/pd
  FRAMEWORKS = Carbon IOKit ForceFeedback
  LDFLAGS += -bundle  -bundle_loader $(PDEXECUTABLE)
  LIBS += -lc -L/sw/lib -L./HID\ Utilities\ Source/build \
       -lHIDUtilities $(patsubst %,-framework %,$(FRAMEWORKS))
  STRIP = strip -x
  hid.$(EXTENSION): input_arrays hid_utilites $(OBJ)
endif

all: hid.$(EXTENSION)

.SUFFIXES: .$(EXTENSION)

# ----------------------- GENERAL ---------------------------------------------

# generic optimization
OPT_FLAGS = -O3 -ffast-math
# G4 optimization on Mac OS X
#OPT_FLAGS = -O3 -mcpu=7400 -maltivec -ffast-math -fPIC
# faster G4 7450 optimization  (gives errors) on GNU/Linux
#OPT_FLAGS = -O3 -mcpu=7450 -maltivec -ffast-math -fPIC
# G4 optimization on Mac OS X
#OPT_FLAGS = -O3 -mcpu=7400 -faltivec -ffast-math -fPIC
# faster G4 7450 optimization  (gives errors) on Mac OS X
#OPT_FLAGS = -ffast -mcpu=7450 -faltivec -ffast-math -fPIC

%.o: %.c
	$(CC) $(CFLAGS) -o "$*.o" -c "$*.c"

%.$(EXTENSION): %.o
	$(CC) $(LDFLAGS) -o "$*.$(EXTENSION)" "$*.o" $(OBJ) $(LIBS) \
		`test -f $*.libs && cat $*.libs`	\
		`test -f $(dir $*)../$(OS_NAME)/$(notdir $*).libs && \
			cat $(dir $*)../$(OS_NAME)/$(notdir $*).libs`
	chmod a-x "$*.$(EXTENSION)"
	$(STRIP) $*.$(EXTENSION)
	rm -f -- $*.o


input_arrays: input_arrays.c input_arrays.h

input_arrays.c: linux/input.h
	./make-arrays-from-input.h.pl

input_arrays.h: linux/input.h
	./make-arrays-from-input.h.pl


hid_utilities:
	test -f ./HID\ Utilities\ Source/build/libHIDUtilities.a || \
		( cd  ./HID\ Utilities\ Source && pbxbuild )


local_clean: 
	-rm -f -- *.$(EXTENSION) *~ 
	-find . -name '*.o' | xargs rm -f -- 

distclean: local_clean
	 -rm -f -- input_arrays.? doc/ev*-list.pd

.PHONY: all input_arrays hid_utilities clean distclean


test_locations:
	@echo "EXTENSION: $(EXTENSION)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"
	@echo "LIBS: $(LIBS)"
	@echo "STRIP: $(STRIP)"
	@echo " "
	@echo "SRC: $(SRC)"
	@echo "OBJ: $(OBJ)"
	@echo " "
	@echo "OS_NAME: $(OS_NAME)"
	@echo "PD_VERSION: $(PD_VERSION)"
	@echo "PACKAGE_VERSION: $(PACKAGE_VERSION)"
	@echo "CWD: $(CWD)"
	@echo "DESTDIR: $(DESTDIR)"
	@echo "PREFIX: $(prefix)"
	@echo "BINDIR:  $(bindir)"
	@echo "LIBDIR:  $(libdir)"
	@echo "OBJECTSDIR:  $(objectsdir)"
	@echo "PDDOCDIR:  $(pddocdir)"
	@echo "LIBPDDIR:  $(libpddir)"
	@echo "LIBPDBINDIR:  $(libpdbindir)"
	@echo "HELPDIR:  $(helpdir)"
	@echo "MANUALSDIR:  $(manualsdir)"
	@echo "EXAMPLESDIR:  $(examplesdir)"
