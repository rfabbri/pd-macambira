
CWD := $(shell pwd)

# these are setup to be overridden by the packages/Makefile
SRC_ROOT_DIR = $(CWD)/../../..
INSTALL_PREFIX = build
BUILDLAYOUT_DIR = $(CWD)/../..

all: externals

.SUFFIXES: .pd_darwin

include $(BUILDLAYOUT_DIR)/Makefile.buildlayout

BUILD_SRC = $(EXTERNALS_SRC)/build/src
BUILD_DARWIN_SRC = $(EXTERNALS_SRC)/build/darwin

EXTERNALS = $(shell ls -1 $(BUILD_SRC) | grep -e '.*\.c$$')

PDEXECUTABLE = $(PD_SRC)/bin/pd

## These generally need gcc-3.3
# Generic PowerPC
#OPTIM_FLAGS = -mpowerpc-gpopt
# PowerPC 750 (G3)
OPTIM_FLAGS = -mpowerpc-gpopt -mcpu=750
# PowerPC 7400 (G4 <= 700Mhz)
#OPTIM_FLAGS = -fast -fPIC -mcpu=7400 -faltivec
# PowerPC 7450 (G4 >= 533MHz)
#OPTIM_FLAGS = -fast -fPIC -mcpu=7450 -faltivec -ffast-math
# PowerPC G5 (gcc-3.3)
#OPTIM_FLAGS = -fast -fPIC -faltivec

CFLAGS = -DPD -DUNIX -DMACOSX -Dunix $(OPTIM_FLAGS) \
    -Wall -W -Wno-unused -Wno-parentheses -Wno-switch -Wno-shadow 
INCLUDES = -I$(PD_SRC)/src -I/sw/include
LDFLAGS = -bundle  -bundle_loader $(PDEXECUTABLE) -L/sw/lib

externals: $(EXTERNALS:.c=.pd_darwin)

%.pd_darwin: $(BUILD_SRC)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -o "$(BUILD_DARWIN_SRC)/$*.o" \
		-c "$(BUILD_SRC)/$*.c"
	$(CC) $(LDFLAGS) -o "$(BUILD_DARWIN_SRC)/$*.pd_darwin" \
		"$(BUILD_DARWIN_SRC)/$*.o" -lc -lm \
		`test -f $(BUILD_DARWIN_SRC)/$*.libs && \
			cat $(BUILD_DARWIN_SRC)/$*.libs`	\
		`test -f $(BUILD_SRC)/$*.libs && \
			cat $(BUILD_SRC)/$*.libs`
	chmod a-x "$(BUILD_DARWIN_SRC)/$*.pd_darwin"
	rm -f "$(BUILD_DARWIN_SRC)/$*.o" 

#------------------------------------------------------------------------------
# CLEAN TARGETS
#------------------------------------------------------------------------------

clean:
	-rm -rf root *.pkg 
	-rm $(BUILD_DARWIN_SRC)/*~
	rm $(BUILD_DARWIN_SRC)/*.pd_darwin
